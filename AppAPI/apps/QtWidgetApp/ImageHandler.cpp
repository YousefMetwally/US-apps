#include "ImageHandler.h"
#include "ui_ImageHandler.h"
#include "PluginImpl.h"
#include "../shared/QImageSupport.hpp"
#include <QComboBox>
#include <QMessageBox>

namespace {
    template <class STREAM, class FRAME>
    FRAME GetCurrentFrame(STREAM * stream) {
        assert(stream);

        FRAME frame;
        int idx = 0;
        HRESULT hr = stream->GetCurrentFrame(&idx, &frame);
        if (hr == E_HANDLE)
            return nullptr; // geometry change

        CHECK(hr);
        return frame;
    }

    std::vector<float> NormalizeECG(const std::deque<std::pair<double, float>>& ecg_data, const float data_range_min = -200.0f, const float data_range_max = 800.0f) {
        if (ecg_data.empty())
            return {};

        const auto data_range_interval = (data_range_min!= data_range_max) ? data_range_max - data_range_min : 250.0f;

        std::vector<float> normilize_ecg(ecg_data.size());

        for (size_t i = 0; i < ecg_data.size(); ++i) {
            normilize_ecg[i] = (ecg_data[i].second - data_range_min) / data_range_interval;
        }
        return normilize_ecg;
    }

    void DrawECG(QPixmap& pixmap, const std::deque<std::pair<double, float>>& ecg_data) {
        QPainter painter(&pixmap);
        const int trace_height = 100;
        QImage ecg_image = client::CreateECGImage(NormalizeECG(ecg_data), trace_height);
        painter.drawPixmap(0, pixmap.height() - trace_height, QPixmap::fromImage(ecg_image));
    }
}


ImageHandler::ImageHandler(QWidget *parent) :
    QWidget(parent),
    m_ui(new Ui::ImageHandler),
    m_movie_loader(),
    m_ecg_queue(500)
{
    m_ui->setupUi(this);
    connect(&m_timer, &QTimer::timeout, this, [this] {NextFrame(false); });
}

ImageHandler::~ImageHandler()
{
}

/** On-screen width of image area. */
unsigned int ImageHandler::ImageWidth () const {
    int borderSize = 2*m_ui->labelImage->lineWidth();
    int viewWidth = m_ui->labelImage->width();
    return viewWidth - borderSize;
}
/** On-screen height of image area. */
unsigned int ImageHandler::ImageHeight () const {
    int borderSize = 2*m_ui->labelImage->lineWidth();
    int viewHeight = m_ui->labelImage->height();
    return viewHeight - borderSize;
}


void ImageHandler::SetPluginImpl(PluginImpl* impl)
{
    m_impl = impl;
}

void ImageHandler::OnCurrentFrameChanged()
{
    NextFrame(true);
}

void ImageHandler::LoadImageSource()
{
    m_movie_loader.LoadImageSource(m_impl->GetSource());

    m_movie_loader.SetOutputSize(ImageWidth(), ImageHeight());
    m_movie_loader.LoadStreams();
    m_ecg_queue.Initialize(m_movie_loader.GetECG(), m_movie_loader.IsLiveMode());

    OpenStreams();
}

void ImageHandler::UpdateFrameUI()
{
    bool enable = !m_ui->pushButton_play->isChecked();

    m_ui->frameScrollControll->setEnabled(enable);
    m_ui->pushButton_acquireMovies->setEnabled(enable);
}

void ImageHandler::NextFrame(bool update_stream)
{
    AppAPI::Interval frame_range = m_movie_loader.GetRange();
    if (frame_range.begin == frame_range.end)
        return; // no frames

    // increment frame
    m_current_frame++;
    if (m_current_frame >= frame_range.end)
        m_current_frame = frame_range.begin; // wrap-around

    client::PosStream& pos_stream = m_movie_loader.GetPosStream();
    if (pos_stream.stream && m_movie_loader.IsLiveMode()) {
        pos_stream.UpdateData();
    }

    SetFrame(m_current_frame);
    m_ecg_queue.Update(m_movie_loader.GetECG(), m_movie_loader.IsLiveMode());
    if (m_timer.isActive() || update_stream) {
        emit FrameChanged(m_current_frame);
    }
}

void ImageHandler::OpenStreams()
{
    if (!m_movie_loader.HasMovie()) {
        emit StreamOpened(false); // disable playback button
        return;
    }

    // default to 1st frame
    AppAPI::Interval frame_range = m_movie_loader.GetRange();
    SetFrame(frame_range.begin);

    emit FrameRangeChanged(frame_range.begin, frame_range.end-1); // [begin, end] interval (http://doc.qt.io/qt-5/qabstractslider.html#setRange)
    emit StreamOpened(true);
}

void ImageHandler::SetFrame(int index) {
    QImage image;
    if (m_movie_loader.Is3d()) {
        AppAPI::IImage3dFramePtr t_frame, cf_frame;
        std::tie(t_frame, cf_frame) = m_movie_loader.GetFrame3d(index);
        image = client::QImageConverter::Frame3d(t_frame, cf_frame, m_movie_loader.GetColorMaps());
    } else {
        AppAPI::IImage2dFramePtr t_frame, cf_frame;
        std::tie(t_frame, cf_frame) = m_movie_loader.GetFrame2d(index);
        image = client::QImageConverter::Frame2d(t_frame, cf_frame, m_movie_loader.GetColorMaps());

        client::PosStream & pos_stream = m_movie_loader.GetPosStream();
        if (pos_stream.stream) {
            // add position-checkerboard grid to image (debugging aid)
            AppAPI::Image2d t_image = t_frame->GetData();

            if (image.format() == QImage::Format_Indexed8) {
                AppAPI::PosSample pos_sample = pos_stream.ClosestSample(t_image.time);
                if (pos_sample.time != 0)
                    client::AddCheckerboardPattern(image, m_movie_loader.Get2dGeom(), pos_sample);
            }
        }
    }

    if(!image.isNull()) {
        QPixmap image_pixmap = QPixmap::fromImage(image.mirrored(m_view.flip_left_right, m_view.flip_up_down));
        DrawECG(image_pixmap, m_ecg_queue.GetData());
        m_ui->labelImage->setPixmap(image_pixmap);
    }
}

void ImageHandler::StartStopPlayback()
{
    if (m_timer.isActive())
        m_timer.stop();
    else
        m_timer.start(100); // trigger every 100ms
}
