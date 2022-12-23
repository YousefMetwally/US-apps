#pragma once
#include <memory>
#include <iostream>
#include <QFileDialog>
#include <QMainWindow>
#ifdef QT_AXSERVER_LIB
  #include <QAxBindable>
#endif
#include <QQuickView>

#ifndef Q_MOC_RUN
#include <AppAPI/AppAPI.tlh>
#endif
#include <AppAPI/Version.hpp>
#include <AppAPI/MessageFilter.hpp>
#include "PluginImpl.hpp"
#include <QTimer>

#if QT_VERSION < QT_VERSION_CHECK(6, 3, 2)
  #error Qt 6.3.2 or newer required to avoid proxy problems (https://bugreports.qt.io/browse/QTBUG-104436)
#endif

class QtWebBrowserApp :
        public QMainWindow
#ifdef QT_AXSERVER_LIB
        , public QAxBindable
#endif
{
    Q_OBJECT
    Q_CLASSINFO("Version",     APPAPI_VERSION_SHORT)
    Q_CLASSINFO("ClassID",     "{6C063252-8BA4-42B1-B0C9-D72C89130EA7}")
    Q_CLASSINFO("InterfaceID", "{3E46EB5F-9E51-4D42-A018-9C538D4D9060}")
    Q_CLASSINFO("EventsID",    "{227BD063-9720-48A3-8D70-4EEE45026167}")
    Q_CLASSINFO("Implemented Categories", "{CDD621FE-F252-4BA1-BC87-15D19CAFCEAE}") // preview image
    
    public:
        explicit QtWebBrowserApp(QWidget *parent = 0);
        ~QtWebBrowserApp() override;

#ifdef QT_AXSERVER_LIB
        QAxAggregated* createAggregate() override;
#else
        PluginImpl* createAggregate();
#endif

    public slots:
        void OnParentSet();

    private:
        void CreateUI();

        PluginImpl                  * m_impl = nullptr; ///< weak ptr.
        QTimer                        m_timer;
        std::unique_ptr<QQuickView>   m_ui;
        appapi::AutoMessageFilter     m_filter{new appapi::HostMessageFilter};
};
