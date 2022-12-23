#pragma once
#include <memory>
#include <deque>
#include <iostream>
#include <string>
#include <codecvt>
#include <locale>
#include <tuple>

#include <AppAPI/AppAPI.tlh>

#include "ImageLoad.hpp"

namespace client {

class MovieLoader {
public:
    void InvalidateStream() {
        m_2d.Invalidate();
        m_3d.Invalidate();
        InvalidateColormaps();
    }

    void InvalidateColormaps() {
        m_colormaps.Invalidate();
    }

    bool LoadImageSource(AppAPI::IImageSource * source);
    /** Set current movie based on movie index (zero-based). */
    bool LoadStreams();

    bool Is3d() const { return m_3d.movie; }

    void SetOutputSize(unsigned int width, unsigned int height) {
        m_output_res[0] = width;
        m_output_res[1] = height;
    }

    bool HasMovie() const {
        return (m_2d.movie || m_3d.movie);
    }

    TsStream& GetECG () {
        return m_time_series.ecg;
    }

    PosStream& GetPosStream () {
        return m_time_series.pos;
    }

    AppAPI::Interval GetRange() const {
        return Is3d() ? m_3d.t.Range() : m_2d.t.Range();
    }

    const ColorMaps & GetColorMaps() const {
        return m_colormaps;
    }

    AppAPI::Cart2dGeomData Get2dGeom () const {
        return m_2d.geom;
    }

    bool IsLiveMode() const {
        return m_live_mode;
    }

    /** Returns a (tissue,cf) tuple where cf is optional. */
    std::tuple<AppAPI::IImage2dFramePtr, AppAPI::IImage2dFramePtr> GetFrame2d(int index);
    /** Returns a (tissue,cf) tuple where cf is optional. */
    std::tuple<AppAPI::IImage3dFramePtr, AppAPI::IImage3dFramePtr> GetFrame3d(int index);

private:
    AppAPI::IImageSourcePtr              m_source;
    Movie<AppAPI::IImage2dMoviePtr, AppAPI::IImage2dStreamPtr, AppAPI::Cart2dGeomData> m_2d;
    Movie<AppAPI::IImage3dMoviePtr, AppAPI::IImage3dStreamPtr, AppAPI::Cart3dGeomData> m_3d;
    TsMovie<AppAPI::ITimeSeriesMoviePtr> m_time_series;
    ColorMaps                            m_colormaps;
    bool                                 m_live_mode = false;
    unsigned int                         m_output_res[2] = {256, 256};
};

} // namespace client
