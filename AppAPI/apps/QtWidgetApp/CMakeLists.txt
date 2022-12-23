option(BUILD_AS_LIB "Build plug-in as in-process" OFF)

if(WIN32)
    include(../../buildscripts/qtext.cmake)
    include(../../buildscripts/version.cmake)
endif()

set(QtWidgetApp_SOURCES
    main.cpp
    PluginImpl.cpp
    QtWidgetApp.cpp
    MeasurementsHandler.cpp
    PatientHandler.cpp
    LogHandler.cpp
    ImageHandler.cpp
    ../shared/MovieLoader.cpp
    )

set(QtWidgetApp_HEADERS
    EcgQueue.hpp
    PluginImpl.h
    QtWidgetApp.hpp
    MeasurementsHandler.h
    PatientHandler.h
    LogHandler.h
    ImageHandler.h
    ../shared/ImageLoad.hpp
    ../shared/MovieLoader.hpp
    ../shared/QImageSupport.hpp
    )
    
set(QtWidgetApp_UI
    pluginmainwnd.ui
    MeasurementsHandler.ui
    PatientHandler.ui
    LogHandler.ui
    ImageHandler.ui
    )


# Run moc & uic automatically
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

# Find Qt libraries
find_package(Qt6 COMPONENTS Core Gui Widgets QUIET)
if(NOT Qt6_FOUND)
    find_package(Qt5 5.15 REQUIRED Core Gui Widgets)
endif()

if(WIN32)
    if(Qt6_FOUND)
        find_package(Qt6 REQUIRED AxServer)
    else()
        find_package(Qt5 REQUIRED AxServer)
    endif()
endif()

# add current dir to incl. path
set(CMAKE_INCLUDE_CURRENT_DIR ON)


if(ANDROID OR BUILD_AS_LIB)
    add_library(QtWidgetApp SHARED ${QtWidgetApp_SOURCES} ${QtWidgetApp_HEADERS} ${QtWidgetApp_UI})
else()
    add_executable(QtWidgetApp MACOSX_BUNDLE ${QtWidgetApp_SOURCES} ${QtWidgetApp_HEADERS} ${QtWidgetApp_UI})
    
    set_target_properties(QtWidgetApp PROPERTIES
        MACOSX_BUNDLE_GUI_IDENTIFIER QtWidgetApp
    )
endif()

set_property(TARGET QtWidgetApp PROPERTY WIN32_EXECUTABLE true) # hide console

# Link to Qt
target_link_libraries(QtWidgetApp PUBLIC Qt::Gui Qt::Widgets)

if(WIN32)
    target_link_libraries(QtWidgetApp PUBLIC Qt::AxServer)

    set_target_properties(QtWidgetApp PROPERTIES LINK_FLAGS ${CMAKE_LINK_DEF_FILE_FLAG}"${CMAKE_CURRENT_SOURCE_DIR}/module.def")

    add_nuget_dependencies(QtWidgetApp packages.config)
    # Explicitly add AppAPI to include dirs (NuGet target not sufficient due to CMake/IDC limitation)
    include_nuget_directory(QtWidgetApp AppAPI)
    add_resource_file(QtWidgetApp "2D/3D test application for AppAPI" "GE Healthcare Ultrasound" "${VERSION}")
    add_resource_icon(QtWidgetApp "${CMAKE_CURRENT_SOURCE_DIR}/module.ico")
    qtext_register_target(QtWidgetApp)

    # Hardcode Visual Studio project GUID to match QtWidgetAppInstaller reference
    set(QtWidgetApp_GUID_CMAKE "21BE382A-E0A2-36A6-92A3-7F547B315910" CACHE INTERNAL "Project GUID")

    if(ENABLE_COPY_QT_DEPS OR ENABLE_INSTALLERS)
        add_custom_command(TARGET QtWidgetApp POST_BUILD
                           COMMAND "${Qt_DIR}/../../../bin/windeployqt.exe" ARGS "$(TargetPath)"
                           COMMENT "Copying Qt dependencies")
    endif()
else()
    target_link_libraries(QtWidgetApp PRIVATE HostSupport::HostSupport) # host lib. in non-embeded mode
    
    if(ANDROID)
        # APK packaging
        add_qt_android_apk(QtWidgetApp_apk QtWidgetApp
                           DEPENDS HostSupport::HostSupport HostLoader::HostLoader
        )
    elseif(EMSCRIPTEN)
        link_qt_static(QtWidgetApp)
    elseif(IOS)
        link_qt_static(QtWidgetApp)
    endif()
endif()
