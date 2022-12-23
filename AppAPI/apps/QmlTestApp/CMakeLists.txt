if(WIN32)
    include(../../buildscripts/qtext.cmake)
    include(../../buildscripts/version.cmake)
else()
    find_package(MobileSandbox)
endif()

# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# Find the Qt libraries for Qt Quick/QML
find_package(Qt6 COMPONENTS Core Qml Quick QUIET)
if(NOT Qt6_FOUND)
    find_package(Qt5 5.15 REQUIRED Core Qml Quick)
endif()

if(IOS OR EMSCRIPTEN)
    if(Qt6_FOUND)
        find_package(Qt6 REQUIRED QmlImportScanner)
    else()
        find_package(Qt5 REQUIRED QmlImportScanner)
    endif()
endif()

# add source files
set(SOURCE
    ImageItem.hpp
    ImageItem.cpp
    QmlTestApp.hpp
    QmlTestApp.cpp
    ../shared/MovieLoader.cpp
    ../shared/MovieLoader.hpp
    app.qrc
)

set(RESOURCE_FILES
)

# Create QmlTestApp library
add_library(QmlTestApp ${SOURCE})

# Use the Qml/Quick modules
target_link_libraries(QmlTestApp PRIVATE Qt::Qml Qt::Quick)

if(WIN32)
    add_nuget_dependencies(QmlTestApp packages.config)
else()
    target_link_libraries(QmlTestApp PRIVATE AppAPI::AppAPI)
endif()

# Create QmlTestAppStandalone executable.
if(ANDROID)
    add_library(QmlTestAppStandalone SHARED QmlTestAppStandalone.cpp ${RESOURCE_FILES})
else()
    add_executable(QmlTestAppStandalone QmlTestAppStandalone.cpp ${RESOURCE_FILES})
endif()

target_link_libraries(QmlTestAppStandalone PRIVATE MobileSandbox::MobileSandbox QmlTestApp)
target_link_libraries(QmlTestAppStandalone PRIVATE Qt::Qml Qt::Core)

if(WIN32)
    add_nuget_dependencies(QmlTestAppStandalone packages.config)

    add_resource_file(QmlTestAppStandalone "QML-based AppAPI test app" "GE Healthcare Ultrasound" "${VERSION}")
    add_resource_icon(QmlTestAppStandalone "${CMAKE_CURRENT_SOURCE_DIR}/module.ico")
else()
    if(APPLE)
        set_target_properties(QmlTestAppStandalone PROPERTIES
            MACOSX_BUNDLE TRUE
            MACOSX_BUNDLE_GUI_IDENTIFIER QmlTestAppStandalone
        )

        if(IOS)
            set_target_properties(QmlTestAppStandalone PROPERTIES
                MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/../../buildscripts/IOSBundleInfo.plist.in
            )
        else()
            set_target_properties(QmlTestAppStandalone PROPERTIES
                MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/../../buildscripts/MacOSXBundleInfo.plist.in
            )
        endif()
        
        # Setting MACOSX_PACKAGE_LOCATION to "Resources/QmlTestApp". If resources
        # exist, this will create a folder "QmlTestApp" inside the Contents/Resources/
        # folder on MacOS. On iOS, the "Resources/" folder is removed by default
        # and the resources will be available directly in the "QmlTestApp/" folder
        # inside the .app package.
        set_source_files_properties(${RESOURCE_FILES} PROPERTIES MACOSX_PACKAGE_LOCATION Resources/QmlTestApp)
    elseif(ANDROID)
        # APK packaging
        add_qt_android_apk(QmlTestAppStandalone_apk QmlTestAppStandalone
                           DEPENDS HostSupport::HostSupport HostLoader::HostLoader
        )
    endif()

    if(IOS OR EMSCRIPTEN)
        qt_import_qml_plugins(QmlTestAppStandalone)
        link_qt_static(QmlTestAppStandalone)
    endif()
endif()

