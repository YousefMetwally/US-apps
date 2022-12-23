#include "ImageProvider2d.hpp"
#include "PluginImpl.hpp"
#include "../shared/QImageSupport.hpp"

using namespace AppAPI;

ImageProvider2d::ImageProvider2d(PluginImpl* impl) :
    QQuickImageProvider(QQuickImageProvider::Pixmap),
    m_impl(impl)
{
}

ImageProvider2d::~ImageProvider2d()
{
}

QPixmap ImageProvider2d::requestPixmap(const QString& /*id*/, QSize* size, const QSize& requestedSize)
{
    if (m_current_image.isNull())
        return QPixmap(1, 1); // avoid "QML Image: Failed to get image from provider" if returning empty pixmap

    int width = requestedSize.width();
    int height = requestedSize.height();

    // move to next frame
    m_current_frame += 1;
    if (m_current_frame >= m_frame_range.end)
        m_current_frame = m_frame_range.begin; // wrap around

    auto pixmap = QPixmap::fromImage(m_current_image).scaled(width, height, Qt::KeepAspectRatio);
    width = pixmap.width();
    height = pixmap.height();

    if (size) {
        size->setWidth(width);
        size->setHeight(height);
    }
    return pixmap;
}

void ImageProvider2d::OnUpdateFrame()
{
    if (!m_cart_movie) {
        if (!Initialize())
            return; // initialization failed
    }

    if (!m_colormap.size())
        m_colormap = m_cart_movie->GetColorMap(TYPE_TISSUE_COLOR);

    IImage2dFramePtr frame = m_image_stream->GetFrame(m_current_frame);
    Image2d frame_data = frame->GetData();
    m_current_image = client::CreateTImage(frame_data, m_colormap);

    emit sourceImageFrameChanged();
}

void ImageProvider2d::Invalidate()
{
    m_cart_movie = nullptr;
    m_image_stream = nullptr;
    m_frame_range = {};
    m_current_frame = -1;
    m_current_image = QImage();
    m_colormap = Image2d();
}

void ImageProvider2d::InvalidateColormap() {
    m_colormap = Image2d();
}

bool ImageProvider2d::Initialize()
{
    if (!m_impl)
        return false;

    m_cart_movie = m_impl->GetMovie2d();
    if (!m_cart_movie)
        return false;

    Interval interval = m_cart_movie->GetRange();
    m_image_stream = nullptr;
    for (int stream_idx = interval.begin; stream_idx < interval.end; ++stream_idx) {
        unsigned int num_samples[]{ 500, 500 };
        Cart2dGeomData geom = m_cart_movie->GetBoundingBox();
        m_image_stream = m_cart_movie->GetStream(interval.begin + stream_idx, geom, num_samples);
        if (m_image_stream)//get first available 2d stream
            break;
    }

    if (!m_image_stream)
        throw std::runtime_error("no 2d stream found");

    m_frame_range = { 0,1 };
    if (m_image_stream) {
        m_frame_range = m_image_stream->GetRange();
        m_current_frame = m_frame_range.begin;
    }

    return true;
}
