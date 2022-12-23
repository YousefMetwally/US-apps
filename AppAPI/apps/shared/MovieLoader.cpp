#include "MovieLoader.hpp"

using namespace AppAPI;
using namespace client;

namespace {
    template <class STREAM, class FRAME>
    FRAME GetCurrentFrame(STREAM * stream) {
        assert(stream);

        FRAME frame;
        int idx = 0;
        HRESULT hr = stream->raw_GetCurrentFrame(&idx, &frame);
        if (hr == E_HANDLE)
            return nullptr; // geometry change

        CHECK(hr);
        return frame;
    }
}

bool MovieLoader::LoadImageSource(AppAPI::IImageSource * source) {
    // update source
    m_source = source;

    assert(m_source);

    Interval mg_range = m_source->GetRange();

    if (mg_range.end == mg_range.begin)
        return false;

    IMovieGroupPtr mg = m_source->GetMovieGroup(mg_range.begin);  // Load only the first mg.

    m_live_mode = (mg->IsLiveMode() != 0);

    Interval interval = mg->GetRange();

    for (int index = interval.begin; index < interval.end; ++index) {
        ImageMovieType type = IMAGE_MOVIE_INVALID;
        IUnknownPtr movie;
        HRESULT res = mg->raw_GetMovie(index, PROCESSING_CARTESIAN, &type, &movie);
        if (E_BOUNDS == res)
            return false;
        CHECK(res);

        if (type == IMAGE_MOVIE_2D) {
            m_3d = {}; // Invalidate 3D movie because possible mode switching from 3D to 2D.
            m_2d.movie = movie;
        } else if (type == IMAGE_MOVIE_3D) {
            m_2d = {}; // Invalidate 2D movie because possible mode switching from 2D to 3D.
            m_3d.movie = movie;
        } else if (type == IMAGE_MOVIE_TIME_SERIES) {
            m_time_series.movie = movie;
        } else {
            return false;
        }
    }

    if (!m_2d.movie && !m_3d.movie)
        return false;

    return true;
}

bool MovieLoader::LoadStreams() {
    m_2d.Invalidate(); // release any prior object
    m_3d.Invalidate(); // release any prior object
    InvalidateColormaps();
    m_time_series.Invalidate(); // release any prior object

    // update stream
    if (m_time_series.movie) {
        m_time_series.Parse(!m_live_mode);
    }

    if (Is3d()) {
        if (!m_3d.movie)
            return false;

        // let img. res match on-screen resolution
        unsigned int resolution[] = { m_output_res[0], m_output_res[1], 32 };

        Cart3dGeomData geom = m_3d.movie->GetBoundingBox();
        geom = GrowViewToFitAspectRatio(geom, resolution);

        return m_3d.Parse(geom, resolution);
    } else {
        if (!m_2d.movie)
            return false;

        // let img. res match on-screen resolution
        unsigned int resolution[] = { m_output_res[0], m_output_res[1] };

        Cart2dGeomData geom = m_2d.movie->GetBoundingBox();
        geom = GrowViewToFitAspectRatio(geom, resolution);

        return m_2d.Parse(geom, resolution);
    }
}

std::tuple<IImage2dFramePtr,IImage2dFramePtr> MovieLoader::GetFrame2d(int index) {
    if (!m_2d.t.stream)
        return {};

    IImage2dFramePtr t_frame;
    IImage2dFramePtr cf_frame;
    if (m_live_mode) {
        t_frame = GetCurrentFrame<IImage2dStream, IImage2dFramePtr>(m_2d.t.stream);
        if (!t_frame) {
            InvalidateStream(); // rescan stream after geometry change
            return {};
        }
        if (m_2d.cf.stream) {
            cf_frame = GetCurrentFrame<IImage2dStream, IImage2dFramePtr>(m_2d.cf.stream);
            if (cf_frame) {
            } else {
                InvalidateStream(); // rescan stream after geometry change
                return {};
            }
        }
    } else {
        t_frame = m_2d.t.stream->GetFrame(index);

        if (m_2d.cf.stream) {
            double time = m_2d.t.FrameTime(index);
            int cf_idx = m_2d.cf.ClosestFrame(time);
            cf_frame = m_2d.cf.stream->GetFrame(cf_idx);
        }
    }

    m_colormaps.Load(*m_2d.movie, m_2d.cf.stream != nullptr); // load colormaps after loading frames to prevent invalidation

    return {t_frame, cf_frame};
}

std::tuple<IImage3dFramePtr,IImage3dFramePtr> MovieLoader::GetFrame3d(int index) {
    if (!m_3d.t.stream)
        return {};

    IImage3dFramePtr t_frame;
    IImage3dFramePtr cf_frame;
    if (m_live_mode) {
        t_frame = GetCurrentFrame<IImage3dStream, IImage3dFramePtr>(m_3d.t.stream);
        if (!t_frame) {
            InvalidateStream(); // rescan stream after geometry change
            return {};
        }
        if (m_3d.cf.stream) {
            cf_frame = GetCurrentFrame<IImage3dStream, IImage3dFramePtr>(m_3d.cf.stream);
            if (cf_frame) {
            } else {
                InvalidateStream(); // rescan stream after geometry change
                return {};
            }
        }
    } else {
        t_frame = m_3d.t.stream->GetFrame(index);

        if (m_3d.cf.stream) {
            double time = m_3d.t.FrameTime(index);
            int cf_idx = m_3d.cf.ClosestFrame(time);

            cf_frame = m_3d.cf.stream->GetFrame(cf_idx);
        }
    }

    m_colormaps.Load(*m_3d.movie, m_3d.cf.stream != nullptr); // load colormaps after loading frames to prevent invalidation

    return {t_frame, cf_frame};
}
