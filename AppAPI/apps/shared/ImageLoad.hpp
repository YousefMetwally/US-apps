/** Common image-data functions intended for sharing with 3rd parties. */
#pragma once
#include <cmath>
#include <vector>
#include <AppAPI/AppAPI.tlh>


namespace client {

/** Color-map support struct. */
struct ColorMaps {
    AppAPI::Image2d tissue;
    AppAPI::Image2d cf;
    AppAPI::Image2d cf_arb;

    void Invalidate() {
        tissue = AppAPI::Image2d();
        cf     = AppAPI::Image2d();
        cf_arb = AppAPI::Image2d();
    }

    template <typename MOVIE>
    void Load(MOVIE & m, bool load_cf) {
        if (tissue.size())
            return; // already initialized

        tissue = m.GetColorMap(AppAPI::TYPE_TISSUE_COLOR);
        assert(tissue.format == AppAPI::IMAGE_FORMAT_R8G8B8A8);

        if (load_cf) {
            cf = m.GetColorMap(AppAPI::TYPE_FLOW_COLOR);
            assert(cf.format == AppAPI::IMAGE_FORMAT_R8G8B8A8);

            cf_arb = m.GetColorMap(AppAPI::TYPE_FLOW_ARB);
            assert(cf_arb.format == AppAPI::IMAGE_FORMAT_U8);
        }
    }
};


/** 2D/3D image stream support class. */
template <typename STREAM>
class Stream {
public:
    STREAM              stream;
private:
    AppAPI::Interval    range = {}; ///< frame index range
    std::vector<double> times;      ///< frame times

public:
    void Invalidate() {
        stream = nullptr;
        range = {};
        times.clear();
    }

    void Initialize() {
        assert(stream);

        // Get the number of frames
        range = stream->GetRange();

        // Get the frame times
        int base_idx = 0;
        SAFEARRAY* tmp = stream->GetFrameTimes(&base_idx);
        CComSafeArray<double> frame_times_com;
        frame_times_com.Attach(tmp);

        if (frame_times_com.GetCount() != static_cast<unsigned int>(range.end - range.begin))
            throw std::runtime_error("Number of frames reported by the stream's GetRange and GetFrameTimes do not match");

        times.clear();
        for (unsigned int i = 0; i < frame_times_com.GetCount(); i++)
            times.push_back(frame_times_com.GetAt(i));
    }

    AppAPI::Interval Range() const {
        return range;
    }

    double FrameTime(int idx) const {
        assert(times.size() > 0);
        unsigned int offset = idx - range.begin; // must be unsigned
        assert(offset < times.size());
        return times[offset];
    }

    /** Return index of closest frame with frame-time <= time. */
    int ClosestFrame(double time) const {
        assert(times.size() > 0);

        int idx = range.begin; // default to 1st frame
        for (int i = 0; i < static_cast<int>(times.size()); ++i) {
            if (times[i] <= time)
                idx = range.begin+i; // update idx
            else
                break;

        }
        return idx;
    }
};

/** 2D/3D image movie support struct. */
template <typename MOVIE, typename STREAM, typename GEOM>
struct Movie {
    MOVIE           movie;
    GEOM            geom = {};

    Stream<STREAM>  t;  ///< tissue stream
    Stream<STREAM>  cf; ///< color-flow stream (optional)

    void Invalidate() {
        t.Invalidate();
        cf.Invalidate();
    }

    bool Parse(GEOM & _geom, unsigned int num_samples[]) {
        geom = _geom;

        AppAPI::Interval interval = movie->GetRange();

        for (int i = interval.begin; i < interval.end; ++i) {
            decltype(t.stream) stream = movie->GetStream(i, geom, num_samples);
            AppAPI::ImageType image_type = stream->GetImageType();
            switch (image_type) {
            case AppAPI::IMAGE_TYPE_TISSUE:
                t.stream = stream;
                assert(t.stream);
                t.Initialize();
                break;
            case AppAPI::IMAGE_TYPE_BLOOD_VEL:
                cf.stream = stream;
                assert(cf.stream);
                cf.Initialize();
                break;
            default:
                break;
            }
        }

        return t.stream || cf.stream;
    }
};

/** Time series stream support class. */
class TsStream {
public:
    AppAPI::IScalarSeriesStreamPtr stream;

private:
    std::vector<std::pair<double, float>> data;

public:
    void Invalidate() {
        stream = nullptr;
        data.clear();
    }

    void LoadData() {
        assert(stream);

        AppAPI::Interval range = stream->GetRange();

        // only display first ECG "packet"
        AppAPI::ScalarSeries ts = stream->GetData(range.begin);
        assert(ts.type == AppAPI::SAMPLE_ECG);

        for (size_t i = 0; i < ts.size(); ++i)
            data.push_back(std::make_pair(ts.Times()[i], ts.Samples()[i]));
    }

    std::vector<std::pair<double, float>> GetData() const {
        return data;
    }
};

/** Position sensor stream support class. */
class PosStream {
public:
    AppAPI::IPosSeriesStreamPtr stream;

private:
    std::vector<AppAPI::PosSample>    data;

public:
    void Invalidate() {
        stream = nullptr;
        data.clear();
    }

    void LoadData() {
        assert(stream);

        AppAPI::Interval range = stream->GetRange();

        for (int i = range.begin; i < range.end; ++i) {
            AppAPI::PosSample sample = stream->GetData(i);
            data.push_back(sample);
        }
    }

    void UpdateData() {
        assert(stream);
        int index = 0;
        AppAPI::PosSample sample = stream->GetCurrentData(&index);
        data.clear();
        data.push_back(sample);
    }

    /** Return index of closest sample with time-stamp <= time. */
    AppAPI::PosSample ClosestSample(double time) const {
        assert(data.size() > 0);

        size_t idx = 0; // default to 1st frame
        for (size_t i = 0; i < data.size(); ++i) {
            if (data[i].time <= time)
                idx = i; // update idx
            else
                break;

        }
        return data[idx];
    }
};

/** Time series movie support struct. */
template <typename MOVIE>
struct TsMovie {
    MOVIE           movie;

    TsStream        ecg;  ///< ecg stream
    PosStream       pos;  ///< position sensor samples

    void Invalidate() {
        ecg.Invalidate();
        pos.Invalidate();
    }

    void Parse(bool load_data) {
        AppAPI::Interval interval = movie->GetRange();

        for (int i = interval.begin; i < interval.end; ++i) {
            AppAPI::TimeSeriesStreamType type = {};
            IUnknownPtr tmp = movie->GetStream(i, &type);

            if (type == AppAPI::TIME_SERIES_SCALAR) {
                AppAPI::IScalarSeriesStreamPtr stream;
                stream = tmp; // cast

                AppAPI::SampleType sample_type = stream->GetSampleType();
                switch (sample_type) {
                case AppAPI::SAMPLE_ECG:
                    ecg.stream = stream;
                    assert(ecg.stream);
                    if (load_data)
                        ecg.LoadData();
                    break;
                default:
                    break;
                }
            } else if (type == AppAPI::TIME_SERIES_POS) {
                pos.stream = tmp; // cast
                pos.LoadData();
            }
        }
    }
};


/** Grow a "view" to match the aspect ratio of the on-screen resolution.
    The impl. can be simplified by using a lin.alg. library. */
template <class GEOM>
static GEOM GrowViewToFitAspectRatio (GEOM view, const unsigned int resolution[]) {
    assert(resolution[0] > 0);
    assert(resolution[1] > 0);

    const float screen_aspect = resolution[0]*1.0f/resolution[1]; // >1 mean wide; <1 means narrow
    // view vector lengths
    const float len1 = std::sqrt(view.dir1[0]*view.dir1[0] + view.dir1[1]*view.dir1[1] + view.dir1[2]*view.dir1[2]);
    const float len2 = std::sqrt(view.dir2[0]*view.dir2[0] + view.dir2[1]*view.dir2[1] + view.dir2[2]*view.dir2[2]);

    if (len1/len2 < screen_aspect) {
        // grow view along len1
        float new_len1 = screen_aspect*len2;
        // shift origin
        view.origin[0] -= view.dir1[0]*(new_len1-len1)/2/len1;
        view.origin[1] -= view.dir1[1]*(new_len1-len1)/2/len1;
        view.origin[2] -= view.dir1[2]*(new_len1-len1)/2/len1;
        // grow dir1
        view.dir1[0] *= new_len1/len1;
        view.dir1[1] *= new_len1/len1;
        view.dir1[2] *= new_len1/len1;
    } else {
        // grow view along len2
        float new_len2 = (1/screen_aspect)*len1;
        // shift origin
        view.origin[0] -= view.dir2[0]*(new_len2-len2)/2/len2;
        view.origin[1] -= view.dir2[1]*(new_len2-len2)/2/len2;
        view.origin[2] -= view.dir2[2]*(new_len2-len2)/2/len2;
        // grow dir2
        view.dir2[0] *= new_len2/len2;
        view.dir2[1] *= new_len2/len2;
        view.dir2[2] *= new_len2/len2;
    }

    return view;
}

} // namespace client
