if (NOT DEFINED CMAKE_XCODE_ATTRIBUTE_DEVELOPMENT_TEAM) 
    message(WARNING "CMAKE_XCODE_ATTRIBUTE_DEVELOPMENT_TEAM not set. Bundle identifiers might need to be updated before code signing and deployment to an iOS device.")
endif()


find_package(Qt5Widgets REQUIRED)

# Link to missing Qt libraries on iOS.
# Temporary solution until https://bugreports.qt.io/browse/QTBUG-87060 is released
function(link_qt_static target)
    if (Qt5Widgets_VERSION VERSION_LESS 6.0.0)
        target_link_options(${target} PRIVATE
            "-Wl,-e,_qt_main_wrapper"
        )
    endif()
endfunction()
