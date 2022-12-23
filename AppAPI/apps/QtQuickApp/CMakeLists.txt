option(BUILD_AS_LIB "Build plug-in as in-process" OFF)

if(WIN32)
    include(../../buildscripts/qtext.cmake)
    include(../../buildscripts/version.cmake)
endif()

set(QtQuickApp_SOURCES
    main.cpp
    PluginImpl.cpp
    QtQuickApp.cpp
    ImageProvider2d.cpp
    )

set(QtQuickApp_HEADERS
    PluginImpl.hpp
    QtQuickApp.hpp
    ImageProvider2d.hpp
    ../shared/ImageLoad.hpp
    ../shared/QImageSupport.hpp
    )
    
set(QtQuickApp_QRC
    test2d.qrc
    )
    
set(QtQuickApp_QML
    main.qml
    )
source_group(qml FILES ${QtQuickApp_QML})


# Run moc, uic & rcc automatically
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Find Qt libraries
find_package(Qt6 COMPONENTS Quick Core Gui QuickWidgets QUIET)
if(NOT Qt6_FOUND)
    find_package(Qt5 5.15 REQUIRED Quick Core Gui QuickWidgets)
endif()

if(WIN32)
    if(Qt6_FOUND)
        find_package(Qt6 REQUIRED AxServer)
    else()
        find_package(Qt5 REQUIRED AxServer)
    endif()
endif()
if(IOS OR EMSCRIPTEN)
    if(Qt6_FOUND)
        find_package(Qt6 REQUIRED QmlImportScanner)
    else()
        find_package(Qt5 REQUIRED QmlImportScanner)
    endif()
endif()


if(ANDROID OR BUILD_AS_LIB)
    add_library(QtQuickApp SHARED ${QtQuickApp_SOURCES} ${QtQuickApp_HEADERS} ${QtQuickApp_QRC} ${QtQuickApp_QML})
else()
    add_executable(QtQuickApp MACOSX_BUNDLE ${QtQuickApp_SOURCES} ${QtQuickApp_HEADERS} ${QtQuickApp_QRC} ${QtQuickApp_QML})
    
    set_target_properties(QtQuickApp PROPERTIES
        MACOSX_BUNDLE_GUI_IDENTIFIER QtQuickApp
    )
    if(CMAKE_DEBUG_POSTFIX)
        set_target_properties(QtQuickApp PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})
    endif()
endif()

set_property(TARGET QtQuickApp PROPERTY WIN32_EXECUTABLE true) # hide console

# Link to Qt
target_link_libraries(QtQuickApp PUBLIC Qt::Gui Qt::Quick Qt::QuickWidgets)

if(WIN32)
    target_link_libraries(QtQuickApp PUBLIC Qt::AxServer)
    
    set_target_properties(QtQuickApp PROPERTIES LINK_FLAGS ${CMAKE_LINK_DEF_FILE_FLAG}"${CMAKE_CURRENT_SOURCE_DIR}/module.def")

    add_nuget_dependencies(QtQuickApp packages.config)
    # Explicitly add AppAPI to include dirs (NuGet target not sufficient due to CMake/IDC limitation)
    include_nuget_directory(QtQuickApp AppAPI)
    qtext_register_target(QtQuickApp)
    add_resource_file(QtQuickApp "Empty QtQuick based app" "GE Healthcare Ultrasound" "${VERSION}")
    add_resource_icon(QtQuickApp "${CMAKE_CURRENT_SOURCE_DIR}/module.ico")

    if(ENABLE_COPY_QT_DEPS)
        add_custom_command(TARGET QtQuickApp POST_BUILD
                           COMMAND "${Qt_DIR}/../../../bin/windeployqt.exe" ARGS --qmldir ${CMAKE_CURRENT_SOURCE_DIR} "$(TargetPath)"
                           COMMENT "Copying Qt dependencies")
    endif()
else()
    target_link_libraries(QtQuickApp PRIVATE HostSupport::HostSupport) # host lib. in non-embeded mode
    
    if(ANDROID)
        # APK packaging
        add_qt_android_apk(QtQuickApp_apk QtQuickApp
                           DEPENDS HostSupport::HostSupport HostLoader::HostLoader
        )
    elseif(IOS OR EMSCRIPTEN)
        # link to missing Qt libs
        link_qt_static(QtQuickApp)
        qt_import_qml_plugins(QtQuickApp)
    endif()
endif()
