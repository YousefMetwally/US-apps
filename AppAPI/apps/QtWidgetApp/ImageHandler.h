#pragma once
#include <QWidget>
#include <memory>
#include <QTimer>
#include <deque>

#include "../shared/MovieLoader.hpp"
#include "EcgQueue.hpp"
#ifndef Q_MOC_RUN
#include <AppAPI/AppAPI.tlh>
#endif

namespace Ui {
class ImageHandler;
}

class PluginImpl;


class ImageHandler : public QWidget
{
    Q_OBJECT

    public:
        explicit ImageHandler(QWidget *parent = 0);
        ~ImageHandler();

        void SetPluginImpl(PluginImpl* impl);

    public Q_SLOTS:
        void InvalidateStream() {
            m_movie_loader.InvalidateStream();
        }

        void InvalidateColormaps() {
            m_movie_loader.InvalidateColormaps();
        }

        void OnCurrentFrameChanged();
        void LoadImageSource();
        void UpdateFrameUI();
        void NextFrame(bool update_stream);
        /** Set current movie based on movie index (zero-based). */
        void OpenStreams();
        void SetFrame(int index);
        void StartStopPlayback();

        void SetViewSettings(AppAPI::ViewSettings view) {
            m_view = view;
        }

    Q_SIGNALS:
        void StreamsNumberChanged(int number);
        void FrameRangeChanged(int min, int max);
        void StreamOpened(bool opened);
        void FrameChanged(int index);

    private:
        /** On-screen width of image area. */
        unsigned int ImageWidth () const;
        /** On-screen height of image area. */
        unsigned int ImageHeight () const;

        std::unique_ptr<Ui::ImageHandler> m_ui;
        PluginImpl*  m_impl;

        QTimer       m_timer;
        AppAPI::ViewSettings m_view = {};

        client::MovieLoader m_movie_loader;
        EcgQueue            m_ecg_queue;
        int                 m_current_frame = 0;
};
