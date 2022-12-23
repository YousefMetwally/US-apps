#pragma once
#ifndef Q_MOC_RUN
#include <AppAPI/AppAPI.tlh>
#endif

#include <QQuickImageProvider>

class PluginImpl;

class ImageProvider2d :
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    public QObject, // QQuickImageProvider doesn't inherit from QObject in Qt5
#endif
    public QQuickImageProvider
{
    Q_OBJECT
    Q_PROPERTY(int sourceImageFrame READ GetSourceImageFrame NOTIFY sourceImageFrameChanged)

    public:
        ImageProvider2d(PluginImpl* impl = nullptr);
        ~ImageProvider2d() override;

        QPixmap requestPixmap(const QString& id, QSize* size, const QSize& requestedSize) override;

        int GetSourceImageFrame() const { return m_current_frame; }

    public slots:
        void OnUpdateFrame();
        void Invalidate();
        void InvalidateColormap();

    signals:
        void sourceImageFrameChanged();

    private:
        bool Initialize();

        PluginImpl*               m_impl = nullptr;
        AppAPI::IImage2dMoviePtr  m_cart_movie;
        AppAPI::IImage2dStreamPtr m_image_stream;
        AppAPI::Interval          m_frame_range = {};
        int                       m_current_frame = -1;
        QImage                    m_current_image;
        AppAPI::Image2d           m_colormap;
};
