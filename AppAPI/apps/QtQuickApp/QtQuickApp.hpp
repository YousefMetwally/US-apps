#pragma once
#include <memory>
#include <iostream>
#include <QFileDialog>
#include <QMainWindow>
#ifdef QT_AXSERVER_LIB
  #include <QAxBindable>
#endif
#include <QQuickWidget>
#include <QQuickView>

#ifndef Q_MOC_RUN
#include <AppAPI/AppAPI.tlh>
#endif
#include <AppAPI/Version.hpp>
#include <AppAPI/MessageFilter.hpp>
#include "PluginImpl.hpp"
#include <QTimer>


class ImageProvider2d; // forward decl.

class QtQuickApp :
        public QMainWindow
#ifdef QT_AXSERVER_LIB
        , public QAxBindable
#endif
{
    Q_OBJECT
    Q_CLASSINFO("Version",     APPAPI_VERSION_SHORT)
    Q_CLASSINFO("ClassID",     "{F2439C70-97A2-48B7-A9F7-6A95F5563084}") // MUST be unique for each app
    Q_CLASSINFO("InterfaceID", "{E70F7F0D-1562-4563-95EA-3154217951C0}") // MUST be unique for each app
    Q_CLASSINFO("EventsID",    "{F117F234-676B-4437-84A7-BDC7C2C92116}") // MUST be unique for each app
    Q_CLASSINFO("Implemented Categories", "{9173BC66-7961-4A2D-A732-095928D00ADC},{F69D2153-5B23-4902-A5F0-63D2586D41A6}") // 2D image support, persistent clipboard

    Q_PROPERTY(ImageProvider2d* image2dprovider MEMBER m_imageProvider CONSTANT)
    
    public:
        explicit QtQuickApp (QWidget *parent = 0);
        ~QtQuickApp () override;

#ifdef QT_AXSERVER_LIB
        QAxAggregated* createAggregate() override;
#else
        PluginImpl* createAggregate();
#endif

    public slots:
        void OnParentSet(); /// Called by the PluginImpl when the parent has been set
        void OnExit();
        void OnStopPlay();
#ifdef __EMSCRIPTEN__
        void OnFileLoad();
        void OnFileSelected(const QString & filename, const QByteArray & content);
#endif
    private:
        void CreateUI();

        PluginImpl              * m_impl = nullptr; ///< weak ptr.
        QTimer                    m_timer;
#if defined __EMSCRIPTEN__ || defined _WIN32
        using QuickUI = QQuickView; ///< QQuickWidget not compatible with WASM
#else
        using QuickUI = QQuickWidget; ///< cannot use QQuickView due to Android problems (https://bugreports.qt.io/browse/QTBUG-39454)
#endif
        std::unique_ptr<QuickUI>  m_ui;
        ImageProvider2d         * m_imageProvider = nullptr; ///< weak ptr.
        appapi::AutoMessageFilter m_filter{new appapi::HostMessageFilter};
};
