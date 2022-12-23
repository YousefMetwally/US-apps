option(BUILD_AS_LIB "Build plug-in as in-process" OFF)

if(WIN32)
    include(../../buildscripts/qtext.cmake)
    include(../../buildscripts/version.cmake)
endif()

set(QtWebBrowserApp_SOURCES
    main.cpp
    PluginImpl.cpp
    QtWebBrowserApp.cpp
    )

set(QtWebBrowserApp_HEADERS
    PluginImpl.hpp
    QtWebBrowserApp.hpp
    )
    
set(QtWebBrowserApp_QRC
    QtWebBrowserApp.qrc
    )
    
set(QtWebBrowserApp_QML
    main.qml
    )
source_group(qml FILES ${QtWebBrowserApp_QML})


# Run moc & uic automatically
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Find Qt libraries
find_package(Qt6 REQUIRED Quick Core Gui WebEngineQuick)
if(WIN32)
    find_package(Qt6 REQUIRED AxServer)
endif()


if(ANDROID OR BUILD_AS_LIB)
    add_library(QtWebBrowserApp SHARED ${QtWebBrowserApp_SOURCES} ${QtWebBrowserApp_HEADERS} ${QtWebBrowserApp_QRC} ${QtWebBrowserApp_QML})
else()
    add_executable(QtWebBrowserApp ${QtWebBrowserApp_SOURCES} ${QtWebBrowserApp_HEADERS} ${QtWebBrowserApp_QRC} ${QtWebBrowserApp_QML})
endif()

set_property(TARGET QtWebBrowserApp PROPERTY WIN32_EXECUTABLE true) # hide console

# Link to Qt
target_link_libraries(QtWebBrowserApp PUBLIC Qt::Gui Qt::Quick Qt::WebEngineQuick)

if(WIN32)
    target_link_libraries(QtWebBrowserApp PUBLIC Qt::AxServer)
    
    set_target_properties(QtWebBrowserApp PROPERTIES LINK_FLAGS ${CMAKE_LINK_DEF_FILE_FLAG}"${CMAKE_CURRENT_SOURCE_DIR}/module.def")

    add_nuget_dependencies(QtWebBrowserApp packages.config)
    # Explicitly add AppAPI to include dirs (NuGet target not sufficient due to CMake/IDC limitation)
    include_nuget_directory(QtWebBrowserApp AppAPI)
    qtext_register_target(QtWebBrowserApp)
    add_resource_file(QtWebBrowserApp "Web browser app" "GE Healthcare Ultrasound" "${VERSION}")
    add_resource_icon(QtWebBrowserApp "${CMAKE_CURRENT_SOURCE_DIR}/module.ico")

    if(ENABLE_COPY_QT_DEPS)
        add_custom_command(TARGET QtWebBrowserApp POST_BUILD
                           COMMAND "${Qt_DIR}/../../../bin/windeployqt.exe" ARGS --qmldir ${CMAKE_CURRENT_SOURCE_DIR} "$(TargetPath)"
                           COMMENT "Copying Qt dependencies")
    endif()
else()
    target_link_libraries(QtWebBrowserApp PRIVATE HostSupport::HostSupport) # host lib. in non-embeded mode
    
    if(ANDROID)
        # APK packaging
        add_qt_android_apk(QtWebBrowserApp_apk QtWebBrowserApp
                           DEPENDS HostSupport::HostSupport HostLoader::HostLoader
        )
    endif()
endif()
