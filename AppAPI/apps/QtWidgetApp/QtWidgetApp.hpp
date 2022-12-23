#pragma once
#include <memory>
#include <QMainWindow>
#ifdef QT_AXSERVER_LIB
#  include <QAxBindable>
#endif
#include "PluginImpl.h"
#ifndef Q_MOC_RUN
#include <AppAPI/AppAPI.tlh>
#endif
#include <AppAPI/Version.hpp>
#include <AppAPI/MessageFilter.hpp>

#include "ui_pluginmainwnd.h"
#include "MeasurementsHandler.h"


class QtWidgetApp :
        public QWidget
#ifdef QT_AXSERVER_LIB
        , public QAxBindable
#endif
{
    Q_OBJECT
    Q_CLASSINFO("Version",     APPAPI_VERSION_SHORT)
    Q_CLASSINFO("ClassID",     "{27F51D13-CD40-4390-98D2-9AE79BA9216B}") // MUST be unique for each app
    Q_CLASSINFO("InterfaceID", "{08E0BF35-A529-44CE-A2BF-E404C4B93E53}") // MUST be unique for each app
    Q_CLASSINFO("EventsID",    "{8BE39C99-67C5-437D-A55C-B3FDF7534240}") // MUST be unique for each app
    Q_CLASSINFO("Implemented Categories",  "{9173BC66-7961-4A2D-A732-095928D00ADC},{3A1D3339-0CA9-43BA-9AA3-3D621F835C2C}") // 2D+3D image support
    Q_CLASSINFO("CoClassAlias", "WidgetApp_Qt") // class name in registry

    ///
    public:
        explicit QtWidgetApp (QWidget *parent = 0);
        ~QtWidgetApp () override;

#ifdef QT_AXSERVER_LIB
        QAxAggregated* createAggregate() override;
#else
        PluginImpl* createAggregate();
#endif

    public Q_SLOTS:
        void RequestLoad();
#ifdef __EMSCRIPTEN__
        void OnFileSelected(const QString & /*filename*/, const QByteArray & content);
#endif
        void RequestQuit();

    private:
        std::unique_ptr<Ui::QtWidgetApp> m_ui;
        PluginImpl *                     m_impl = nullptr; ///< weak ptr.
        appapi::AutoMessageFilter        m_filter{new appapi::HostMessageFilter};
};
