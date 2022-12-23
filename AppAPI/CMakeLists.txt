cmake_minimum_required(VERSION 3.16)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

project(AppAPI)

include(buildscripts/nuget.cmake)
if(IOS)
    include(buildscripts/iOS.cmake)
elseif(ANDROID)
    include(buildscripts/qt-android-cmake/AddQtAndroidApk.cmake)
endif()

set(VERSION "0.0.0" CACHE STRING "Build version")

if(WIN32)
    # build options
    option(ENABLE_COPY_QT_DEPS "Copy Qt dependencies as post-build event" ON)
    option(ENABLE_INSTALLERS "Include installer projects" OFF) # require WiX installed (https://wixtoolset.org/)

    # Don't use windows min/max, use STL instead
    add_definitions(-DNOMINMAX)
else()
    # also search for precompiled packages in "packages" subfolder
    list(APPEND CMAKE_FIND_ROOT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/packages")

    # work-around for incomplete handling of transitive dependencies
    # Issue: https://gitlab.kitware.com/cmake/cmake/-/issues/20511
    find_package(AppAPI REQUIRED AppAPI)
    find_package(Threads REQUIRED)
    find_package(HostLoader REQUIRED HostLoader)
    find_package(HostSupport REQUIRED HostSupport)
endif()

# enable unicode for all projects
add_definitions(-DUNICODE -D_UNICODE)

# check if Qt SDK is found
find_package(Qt6 COMPONENTS Widgets QUIET)
if(Qt6_FOUND)
    set(Qt_DIR ${Qt6_DIR})
else()
    find_package(Qt5 5.15 COMPONENTS Widgets)
    if(Qt5_FOUND)
        set(Qt_DIR ${Qt5_DIR})
    endif()
endif()

# projects to build
if(Qt5Widgets_FOUND OR Qt6Widgets_FOUND)
    # Qt apps
    add_subdirectory(apps/QtWidgetApp)
    if(ENABLE_INSTALLERS AND WIN32)
        include_external_msproject(QtWidgetAppInstaller apps/QtWidgetAppInstaller/QtWidgetAppInstaller.wixproj PLATFORM x64)
        add_dependencies(QtWidgetAppInstaller QtWidgetApp)
    endif()
    add_subdirectory(apps/QtQuickApp)
endif()
if(Qt6Widgets_FOUND)
    if(WIN32)
        # Qt WebEngine sample app
        add_subdirectory(apps/QtWebBrowserApp)
    endif()
endif()

if(NOT WIN32)
    # add mobile apps
    add_subdirectory(apps/QmlTestApp)
endif()

if(WIN32)
    # C# apps
    include_external_msproject(WinFormsApp apps/WinFormsApp/WinFormsApp.csproj PLATFORM x64)
    if(ENABLE_INSTALLERS)
        include_external_msproject(WinFormsAppInstaller apps/WinFormsAppInstaller/WinFormsAppInstaller.wixproj PLATFORM x64)
        add_dependencies(WinFormsAppInstaller WinFormsApp)
    endif()
endif()

# standalone examples
if(WIN32)
    add_subdirectory(host/LaunchTest)
endif()
add_subdirectory(host/StandaloneSample)
if(WIN32)
    include_external_msproject(PySamples host/PySamples/PySamples.pyproj PLATFORM "Any CPU")
    include_external_msproject(StandaloneCsSample host/StandaloneCsSample/StandaloneCsSample.csproj PLATFORM x64)
endif()
