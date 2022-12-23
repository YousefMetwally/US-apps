#include "QtWebBrowserApp.hpp"
#include <qapplication.h>
#include <QtWebEngineQuick>

#include "../../common/QAxClassAppContainer.hpp"

// REF: http://doc.qt.io/qt-5/activeqt-server.html#serving-controls
QAXFACTORY_BEGIN("{401B111E-19B5-495D-AD8D-EE66E9F0AE5A}", // type library ID
                 "{A1D606A6-13FF-482B-8DA8-650BB6DE9EB6}") // application ID
    QAXCLASS_APPCONTAINER(QtWebBrowserApp)
QAXFACTORY_END()


// custom main function based on qtactiveqt\src\activeqt\control\qaxmain.cpp
int main(int argc, char** argv) {
    // Run web engine in-proc. Required for correct firewall whitelisting of the app.
    // Also seem to fix initialization failure when running under low integrity level (low IL) on Qt6.
    qputenv("QTWEBENGINE_CHROMIUM_FLAGS", "--single-process");

    QtWebEngineQuick::initialize();

    QAxFactory::startServer();
    QApplication app(argc, argv);
    app.setQuitOnLastWindowClosed(false);

    return app.exec();
}
