#include "QtWidgetApp.hpp"
#include <iostream>
#include <QFileDialog>
#include <qapplication.h>

#ifdef QT_AXSERVER_LIB
#include "../../common/QAxClassAppContainer.hpp"


// REF: http://doc.qt.io/qt-5/activeqt-server.html#serving-controls
QAXFACTORY_BEGIN("{B58EAFA5-C53A-4D67-9698-55D8C2EA47D6}", // type library ID (MUST be unique for each app)
                 "{3ECBF94A-21FE-40EF-B60A-B0BCC514ABBC}") // application ID (MUST be unique for each app)
    QAXCLASS_APPCONTAINER(QtWidgetApp)
QAXFACTORY_END()

#else

extern size_t HostSupportForceLink();

#include <iostream>
#include <QOpenGLContext>
#include <HostSupport/HostSupport_i.c> // for PluginHostBasic CLSID
#include "../shared/LoadFile.hpp"
#include "../../common/OffscreenGlContext.hpp"


ComInitialize com(COINIT_MULTITHREADED);

int main(int argc, char *argv[]) {
    // trigger HostSupport/HostLoader linking (needed for static builds)
    HostSupportForceLink();

    QApplication  app(argc, argv);

    // set default OpenGL settings before starting app
    QSurfaceFormat glFormat;
    if (QOpenGLContext::openGLModuleType() == QOpenGLContext::LibGL) {
        glFormat.setVersion(3, 3); // use OpenGL 3.3 on desktop
        glFormat.setProfile(QSurfaceFormat::CoreProfile);
    } else {
        glFormat.setVersion(3, 0); // use OpenGL 3.0 on embedded (ES)
    }
    QSurfaceFormat::setDefaultFormat(glFormat);

    OffscreenGlContext gl_ctx;
    auto lock_gl = gl_ctx.Activate();

    using namespace AppAPI;

#ifndef __EMSCRIPTEN__
    IPluginHostBasicPtr basic_parent;
    IPluginHostPatientPtr pat_parent;
    try {
        CHECK(basic_parent.CreateInstance(CLSID_PluginHostBasic, NULL, CLSCTX_ALL));
        CHECK(pat_parent.CreateInstance(CLSID_PluginHostPatient, NULL, CLSCTX_ALL));
        IImageSourcePtr source = pat_parent->GetImageSource();
#if TARGET_OS_IPHONE
        // use simulator until we get suitable file selection UI
        GenerateSyntheticIntoSource(*source);
#else
        QString filename = QFileDialog::getOpenFileName(nullptr, "Select file", "", "Image files (*.*)");
  #ifdef __ANDROID__
        CopyContentURIToTempFile(filename); // file auto-deleted at scope end
  #endif
        LoadFileIntoSource(filename.toStdWString(), *source);
#endif
    } catch (std::exception & e) {
        std::cerr << "ERROR: " << e.what() << std::endl;
        return -1;
    }
#endif

    QtWidgetApp mainWin;
    mainWin.resize(640, 480);
    std::unique_ptr<PluginImpl> impl(mainWin.createAggregate());
#ifndef __EMSCRIPTEN__
    impl->Initialize(AppAPI::STARTUP_NORMAL, {}, nullptr, basic_parent, pat_parent);
#endif
    mainWin.show();
    
    return app.exec();
}
#endif
