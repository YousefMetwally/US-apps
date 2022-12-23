/** Common AppAPI-Qt conversion functions intended for sharing with 3rd parties. */
#pragma once
#include "ImageLoad.hpp"
#include <QImage>
#include <QPainter>
#include <vector>

namespace client {

static unsigned int RGBA_to_BGRA(unsigned int val) {
    // channel access is reverse relative to byte ordering since we're running on little-endian
    unsigned char r = (val)& 0xFF;
    unsigned char g = (val >> 8) & 0xFF;
    unsigned char b = (val >> 16) & 0xFF;
    unsigned char a = (val >> 24) & 0xFF;
    return (a << 24) | (r << 16) | (g << 8) | b;
};

/** Convert an AppAPI colormap (in QImage::Format_RGBA8888 format) to Qt colormap format (in QImage::Format_ARGB32 format).*/
static QVector<QRgb> ConvertRgbColorMap(const AppAPI::Image2d & colormap) {
    assert(colormap.format == AppAPI::IMAGE_FORMAT_R8G8B8A8);
    assert(colormap.dims[0] == 256);
    assert(colormap.dims[1] == 1);

    const auto * cb_buf = reinterpret_cast<const unsigned int*>(colormap.data_ptr());
    QVector<QRgb> colors(256);
    for (int i = 0; i < 256; ++i)
        colors[i] = RGBA_to_BGRA(cb_buf[i]);

    return colors;
}

static QImage CreateTImage(const AppAPI::Image2d& frame_data, const AppAPI::Image2d & colormap) {
    // Translate format to QImage format
    if (frame_data.format == AppAPI::IMAGE_FORMAT_R8G8B8A8) {
        // tmp_ptr must be multiple of 32bit, or else scan-lines will not be aligned (REF: http://doc.qt.io/qt-5.6/qimage.html#scanLine)
        assert(reinterpret_cast<std::uintptr_t>(frame_data.data_ptr()) % 4 == 0);
        return QImage(frame_data.data_ptr(), frame_data.dims[0], frame_data.dims[1], frame_data.stride0, QImage::Format_RGBA8888).copy();
    }

    if (frame_data.format == AppAPI::IMAGE_FORMAT_U8) {
        // must apply colormap
        QImage image(frame_data.data_ptr(), frame_data.dims[0], frame_data.dims[1], frame_data.stride0, QImage::Format_Indexed8);
        image.setColorTable(ConvertRgbColorMap(colormap)); // expects QImage::Format_ARGB32
        return image.copy();
    }

    throw std::runtime_error("unsupported image type");
}

static QImage CreateTImage(const AppAPI::Image3d& frame_data, const AppAPI::Image2d & colormap, unsigned int plane) {
    assert(plane < frame_data.dims[2]);

    // Translate format to QImage format
    if (frame_data.format == AppAPI::IMAGE_FORMAT_R8G8B8A8) {
        // tmp_ptr must be multiple of 32bit, or else scan-lines will not be aligned (REF: http://doc.qt.io/qt-5.6/qimage.html#scanLine)
        assert(reinterpret_cast<std::uintptr_t>(frame_data.data_ptr()) % 4 == 0);
        return QImage(frame_data.data_ptr() + plane*frame_data.stride1, frame_data.dims[0], frame_data.dims[1], frame_data.stride0, QImage::Format_RGBA8888).copy();
    }

    if (frame_data.format == AppAPI::IMAGE_FORMAT_U8) {
        // must apply colormap
        QImage image(frame_data.data_ptr() + plane*frame_data.stride1, frame_data.dims[0], frame_data.dims[1], frame_data.stride0, QImage::Format_Indexed8);
        image.setColorTable(ConvertRgbColorMap(colormap)); // expects QImage::Format_ARGB32
        return image.copy();
    }

    throw std::runtime_error("unsupported image type");
}

static QImage CreateCFImage(const AppAPI::Image2d& t_frame, const AppAPI::Image2d& cf_frame, const ColorMaps& colormaps) {
    assert((t_frame.dims[0] == cf_frame.dims[0]) && (t_frame.dims[1] == cf_frame.dims[1]));
    assert(t_frame.format == AppAPI::IMAGE_FORMAT_U8);
    assert(cf_frame.format == AppAPI::IMAGE_FORMAT_FREQ8BW8);

    std::vector<unsigned int> img_buf(t_frame.dims[0] * t_frame.dims[1]);

    for (unsigned int j = 0; j < t_frame.dims[1]; ++j) {
        for (unsigned int i = 0; i < t_frame.dims[0]; ++i) {
            auto cf_val = reinterpret_cast<const unsigned short*>(cf_frame.data_ptr())[j*cf_frame.stride0/sizeof(unsigned short) + i];
            auto t_val = t_frame.data_ptr()[j*t_frame.stride0 + i];

            auto arb_val = colormaps.cf_arb.data_ptr()[cf_val];
            if (arb_val > t_val)
                img_buf[j*t_frame.dims[0] + i] = RGBA_to_BGRA(reinterpret_cast<const unsigned int*>(colormaps.cf.data_ptr())[cf_val]);
            else
                img_buf[j*t_frame.dims[0] + i] = RGBA_to_BGRA(reinterpret_cast<const unsigned int*>(colormaps.tissue.data_ptr())[t_val]);
        }
    }

    return QImage(reinterpret_cast<unsigned char*>(img_buf.data()), t_frame.dims[0], t_frame.dims[1], sizeof(unsigned int) * t_frame.dims[0], QImage::Format_ARGB32).copy();
}

static QImage CreateCFImage(const AppAPI::Image3d& t_frame, const AppAPI::Image3d& cf_frame, const ColorMaps& colormaps, unsigned int plane) {
    assert((t_frame.dims[0] == cf_frame.dims[0]) && (t_frame.dims[1] == cf_frame.dims[1]) && (t_frame.dims[2] == cf_frame.dims[2]));
    assert(plane < t_frame.dims[2]);
    assert(t_frame.format == AppAPI::IMAGE_FORMAT_U8);
    assert(cf_frame.format == AppAPI::IMAGE_FORMAT_FREQ8BW8);

    std::vector<unsigned int> img_buf(t_frame.dims[0] * t_frame.dims[1]);

    for (unsigned int j = 0; j < t_frame.dims[1]; ++j) {
        for (unsigned int i = 0; i < t_frame.dims[0]; ++i) {
            auto cf_val = reinterpret_cast<const unsigned short*>(cf_frame.data_ptr())[j*cf_frame.stride0/sizeof(unsigned short) + i + plane*cf_frame.stride1/sizeof(unsigned short)];
            auto t_val = t_frame.data_ptr()[j*t_frame.stride0 + i + plane*t_frame.stride1];

            auto arb_val = colormaps.cf_arb.data_ptr()[cf_val];
            if (arb_val > t_val)
                img_buf[j*t_frame.dims[0] + i] = RGBA_to_BGRA(reinterpret_cast<const unsigned int*>(colormaps.cf.data_ptr())[cf_val]);
            else
                img_buf[j*t_frame.dims[0] + i] = RGBA_to_BGRA(reinterpret_cast<const unsigned int*>(colormaps.tissue.data_ptr())[t_val]);
        }
    }

    return QImage(reinterpret_cast<unsigned char*>(img_buf.data()), t_frame.dims[0], t_frame.dims[1], sizeof(unsigned int) * t_frame.dims[0], QImage::Format_ARGB32).copy();
}

inline void AddCheckerboardPattern(QImage & image, const AppAPI::Cart2dGeomData img_geom, const AppAPI::PosSample ref_frame) {
    assert(image.format() == QImage::Format_Indexed8);
    const float GRID = 2*0.050f; // 5cm grid size (2x for bright+dark cycle)

    // image-to-global coordinate rotation matrix
    // Based on https://en.wikipedia.org/wiki/Quaternions_and_spatial_rotation#Conversion_to_and_from_the_matrix_representation with w=a, x=b, y=c & z=d
    float ref_rot[3][3] = {};
    {
        // quaternion elements
        float x = ref_frame.orient[0]; // axis_x * sin(angle/2)
        float y = ref_frame.orient[1]; // axis_y * sin(angle/2)
        float z = ref_frame.orient[2]; // axis_z * sin(angle/2)
        float w = std::sqrt(1.0f - x*x - y*y - z*z); // cos(angle/2) [normalize to unit norm]

        // rotation matrix for the extenal reference frame
        ref_rot[0][0] = w*w + x*x - y*y - z*z; // 1st row
        ref_rot[0][1] = 2*x*y - 2*w*z;
        ref_rot[0][2] = 2*w*y + 2*x*z;

        ref_rot[1][0] = 2*w*z + 2*x*y; // 2nd row
        ref_rot[1][1] = w*w - x*x + y*y - z*z;
        ref_rot[1][2] = 2*y*z - 2*w*x;

        ref_rot[2][0] = 2*x*z - 2*w*y; // 3rd row
        ref_rot[2][1] = 2*w*x + 2*y*z;
        ref_rot[2][2] = w*w - x*x - y*y + z*z;
    }

    const int W = image.width();
    const int H = image.height();
    for (int h = 0; h < H; ++h) {
        const float dh = (h+0.5f)/H; // in (0,1)
        BYTE * line = reinterpret_cast<BYTE*>(image.scanLine(h));

        for (int w = 0; w < W; ++w) {
            const float dw = (w+0.5f)/W; // in (0,1)
            // image pixel coordinate
            float pix_pos[3] = {};
            pix_pos[0] = img_geom.origin[0] + dw*img_geom.dir1[0] + dh*img_geom.dir2[0];
            pix_pos[1] = img_geom.origin[1] + dw*img_geom.dir1[1] + dh*img_geom.dir2[1];
            pix_pos[2] = img_geom.origin[2] + dw*img_geom.dir1[2] + dh*img_geom.dir2[2];

            // convert to global coordinates
            float pos[3] = {};
            pos[0] = ref_frame.pos[0] + ref_rot[0][0]*pix_pos[0] + ref_rot[0][1]*pix_pos[1] + ref_rot[0][2]*pix_pos[2];
            pos[1] = ref_frame.pos[1] + ref_rot[1][0]*pix_pos[0] + ref_rot[1][1]*pix_pos[1] + ref_rot[1][2]*pix_pos[2];
            pos[2] = ref_frame.pos[2] + ref_rot[2][0]*pix_pos[0] + ref_rot[2][1]*pix_pos[1] + ref_rot[2][2]*pix_pos[2];

            // add checkerboard pattern based on global coordinates
            bool grid_x = fabs(fmod(pos[0]/GRID, 1)) > 0.5f;
            bool grid_y = fabs(fmod(pos[1]/GRID, 1)) > 0.5f;
            bool grid_z = fabs(fmod(pos[2]/GRID, 1)) > 0.5f;
            BYTE & pixel = line[w];
            if (pixel != 0) { // 0-valued pixels are transparent and not to be changed
                if (grid_x ^ grid_y ^ grid_z)
                    pixel = std::min(pixel + 16, 255); // slightly brighter
                else
                    pixel = std::max(pixel - 16, 1); // slightly darker but never transparent (== 0)
            }
        }
    }
}

inline QImage CreateECGImage(const std::vector<float>& ecg_data, const int trace_height) {
    const int trace_width = static_cast<int>(ecg_data.size());
    QImage ecg_image(trace_width, trace_height, QImage::Format_ARGB32);
    ecg_image.fill(QColor(0, 0, 0, 0));
    QPainter ecg_painter(&ecg_image);
    ecg_painter.setRenderHints(QPainter::Antialiasing, true);

    QPen ecg_pen(QColor("#118679"));
    ecg_pen.setWidth(2);
    ecg_pen.setStyle(Qt::SolidLine);
    ecg_painter.setPen(ecg_pen);

    const int y_space = 5; //Add some space below and above plot to have room for drawing QRS markers etc.
    const float adjusted_height = trace_height - 2 * y_space;

    QPolygonF ecg_trace(static_cast<int>(ecg_data.size()));
    for (size_t index = 0; index < ecg_data.size(); index++) {
        const float y_polarity_inverted = 1.0f - ecg_data[index];
        float y_scaled = y_polarity_inverted * adjusted_height;
        y_scaled = y_scaled + y_space;

        ecg_trace[static_cast<int>(index)] = QPointF(static_cast<float>(index), y_scaled);
    }
    ecg_painter.drawPolyline(ecg_trace);

    return ecg_image;
}


class QImageConverter {
public:
    /* Convert tissue & color-flow (optional) 2D frames into a QImage of RGB format. */
    static QImage Frame2d(AppAPI::IImage2dFrame * t_frame, AppAPI::IImage2dFrame * cf_frame, const ColorMaps & colormaps) {
        if (!t_frame)
            return QImage(); // no image data available

        AppAPI::Image2d t_image = t_frame->GetData();
        AppAPI::Image2d cf_image; // optional
        if (cf_frame)
            cf_image = cf_frame->GetData();

        QImage image;
        if (cf_frame) {
            image = CreateCFImage(t_image, cf_image, colormaps);
        } else {
            image = CreateTImage(t_image, colormaps.tissue);
        }

        return image;
    }

    /* Convert tissue & color-flow (optional) 3D frames into a QImage of RGB format. */
    static QImage Frame3d(AppAPI::IImage3dFrame * t_frame, AppAPI::IImage3dFrame * cf_frame, const ColorMaps & colormaps) {
        if (!t_frame)
            return QImage(); // no image data available

        AppAPI::Image3d t_image = t_frame->GetData();
        AppAPI::Image3d cf_image; // optional
        if (cf_frame)
            cf_image = cf_frame->GetData();

        QImage image;
        if (cf_frame) {
            image = CreateCFImage(t_image, cf_image, colormaps, t_image.dims[2] / 2);
        } else {
            image = CreateTImage(t_image, colormaps.tissue, t_image.dims[2] / 2); // extract mid-volume slice
        }

        return image;
    }
};

} //namespace client
