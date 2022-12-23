#include "QtQuickApp.hpp"
#include <qapplication.h>

#ifdef QT_AXSERVER_LIB
#include "../../common/QAxClassAppContainer.hpp"

// REF: http://doc.qt.io/qt-5/activeqt-server.html#serving-controls
QAXFACTORY_BEGIN("{ADFFA069-3C72-4296-9F3C-2DDE1192C098}", // type library ID (MUST be unique for each app)
                 "{5C343A81-7F27-433B-B8AB-975B1A1FE046}") // application ID (MUST be unique for each app)
    QAXCLASS_APPCONTAINER(QtQuickApp)
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
    
    QtQuickApp mainWin;
    mainWin.resize(640, 480);
    std::unique_ptr<PluginImpl> impl(mainWin.createAggregate());
#ifndef __EMSCRIPTEN__
    impl->Initialize(AppAPI::STARTUP_NORMAL, {}, nullptr, basic_parent, pat_parent);
#endif
    mainWin.show();
    
    return app.exec();
}
#endif
