# Example GUI App CMakeLists.txt

# To get started on a new GUI app, copy this entire folder (containing this file and C++ sources) to
# a convenient location, and then start making modifications. For other examples of CMakeLists for
# GUI apps, check `extras/Projucer` and `examples/DemoRunner` in the JUCE repo.

# The first line of any CMake project should be a call to `cmake_minimum_required`, which checks
# that the installed CMake will be able to understand the following CMakeLists, and ensures that
# CMake's behaviour is compatible with the named version. This is a standard CMake command, so more
# information can be found in the CMake docs.

cmake_minimum_required(VERSION 3.22)

# The top-level CMakeLists.txt file for a project must contain a literal, direct call to the
# `project()` command. `project()` sets up some helpful variables that describe source/binary
# directories, and the current project version. This is a standard CMake command.

project(TenpinScope VERSION 0.0.1)
set (TargetName ${PROJECT_NAME})

set (CMAKE_CXX_STANDARD 20)
set (CMAKE_CXX_STANDARD_REQUIRED ON)
set (CMAKE_EXPORT_COMPILE_COMMANDS ON)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
option(JUCE_ENABLE_MODULE_SOURCE_GROUPS "Show all module sources in IDE projects" ON)

# If you've installed JUCE somehow (via a package manager, or directly using the CMake install
# target), you'll need to tell this project that it depends on the installed copy of JUCE. If you've
# included JUCE directly in your source tree (perhaps as a submodule), you'll need to tell CMake to
# include that subdirectory as part of the build.

add_subdirectory(modules)                    # If you've put JUCE in a subdirectory called JUCE

# If your app depends the VST2 SDK, perhaps to host VST2 plugins, CMake needs to be told where
# to find the SDK on your system. This setup should be done before calling `juce_add_gui_app`.

# juce_set_vst2_sdk_path(...)

# `juce_add_gui_app` adds an executable target with the name passed as the first argument
# (GuiAppExample here). This target is a normal CMake target, but has a lot of extra properties set
# up by default. This function accepts many optional arguments. Check the readme at
# `docs/CMake API.md` in the JUCE repo for the full list.

juce_add_gui_app(${TargetName}
    # VERSION ...                       # Set this if the app version is different to the project version
    # ICON_BIG ...                      # ICON_* arguments specify a path to an image file to use as an icon
    # ICON_SMALL ...
    # DOCUMENT_EXTENSIONS ...           # Specify file extensions that should be associated with this app
    # COMPANY_NAME ...                  # Specify the name of the app's author
    PRODUCT_NAME "${TargetName}")     # The name of the final executable, which can differ from the target name

# `juce_generate_juce_header` will create a JuceHeader.h for a given target, which will be generated
# into your build tree. This should be included with `#include <JuceHeader.h>`. The include path for
# this header will be automatically added to the target. The main function of the JuceHeader is to
# include all your JUCE module headers; if you're happy to include module headers directly, you
# probably don't need to call this.


# `target_sources` adds source files to a target. We pass the target that needs the sources as the
# first argument, then a visibility parameter for the sources which should normally be PRIVATE.
# Finally, we supply a list of source files that will be built into the target. This is a standard
# CMake command.

set(SOURCES_MAIN
    src/Main.cpp
    src/MainComponent.h
    src/MainComponent.cpp
    src/mainWindow.h
    src/mainWindow.cpp
)

set (SOURCES_CONTROLLER
    src/controller/endpointController.h
    src/controller/endpointController.cpp
    src/controller/eventFilter.h
    src/controller/eventFilter.cpp
    src/controller/midiController.h
    src/controller/midiController.cpp
    src/controller/syntheticEndpointController.h
    src/controller/syntheticEndpointController.cpp)

set (SOURCES_HANDLER
    src/handler/handlerResult.h
    src/handler/msgHandler.h
    src/handler/msgHandler.cpp
    src/handler/eventDispatcher.h
    src/handler/eventDispatcher.cpp
    src/handler/umpHandler.h
    src/handler/umpHandler.cpp)

set (SOURCES_MODEL
    src/model/appContext.h
    src/model/event.h
    src/model/eventList.h
    src/model/eventList.cpp
    src/model/midiProperties.h
    src/model/midiEndpointProperties.h
    src/model/midiTypes.h
    src/model/midiTypes.cpp
    src/model/persistentContext.h
    src/model/runtimeContext.h
    src/model/ump/bitField.h
    src/model/ump/channelVoice1.h
    src/model/ump/channelVoice1.cpp
    src/model/ump/channelVoice2.h
    src/model/ump/channelVoice2.cpp
    src/model/ump/umpEvent.h
    src/model/ump/systemCommon.h
    src/model/ump/systemCommon.cpp
    src/model/ump/sysex7.h
    src/model/ump/sysex7.cpp
    src/model/ump/sysex8.h
    src/model/ump/sysex8.cpp
    src/model/ump/flexData.h
    src/model/ump/flexData.cpp
    src/model/ump/stream.h
    src/model/ump/stream.cpp
    src/model/ump/utility.h
    src/model/ump/utility.cpp)


set(SOURCES_SYSEX
    src/model/sysex/sysex7Message.h
    src/model/sysex/sysex7Message.cpp
    src/model/sysex/sysex7Builder.h
    src/model/sysex/sysex7Builder.cpp
    src/model/sysex/sysex8Message.h
    src/model/sysex/sysex8Message.cpp
    src/model/sysex/sysex8Builder.h
    src/model/sysex/sysex8Builder.cpp
)

set(SOURCES_UTILITY
    src/utility/logger.h
    src/utility/logger.cpp
    src/utility/propertyList.h
    src/utility/variantConverters.h
    src/utility/buffer.h
    src/utility/buffer.cpp
    src/utility/paletteColor.h
    src/utility/paletteColor.cpp
    src/utility/versionNumber.h
    src/utility/versionNumber.cpp
)

set(SOURCES_VIEW
    src/view/dataView.h
    src/view/dataView.cpp
    src/view/dataViewHeader.h
    src/view/dataViewHeader.cpp
    src/view/deviceView.h
    src/view/deviceView.cpp
    src/view/endpointView.h
    src/view/endpointView.cpp
    src/view/eventNameUtils.h
    src/view/eventNameUtils.cpp
    src/view/eventView.h
    src/view/eventView.cpp
    src/view/eventListView.h
    src/view/eventListView.cpp
    src/view/eventListSettingsView.h
    src/view/eventListSettingsView.cpp
    src/view/dispatchContext.h
    src/view/eventListViewHandler.h
    src/view/eventListViewHandler.cpp
    src/view/eventListViewMsgHandler.h
    src/view/eventListViewMsgHandler.cpp
    src/view/labeledValue.h
    src/view/labeledValue.cpp
    src/view/palette.h
    src/view/palette.cpp
    src/view/resizeHandle.h
    src/view/resizeHandle.cpp
    src/view/headerView.h
    src/view/headerView.cpp
    src/view/labeledComboBox.h
    src/view/labeledComboBox.cpp
    src/view/lookAndFeel.h
    src/view/lookAndFeel.cpp
)

set(APP_SOURCES
    ${SOURCES_MAIN}
    ${SOURCES_CONTROLLER}
    ${SOURCES_HANDLER}
    ${SOURCES_MODEL}
    ${SOURCES_SYSEX}
    ${SOURCES_UTILITY}
    ${SOURCES_VIEW}
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/src PREFIX "src" FILES ${APP_SOURCES})

target_sources(${TargetName}
    PRIVATE
        ${APP_SOURCES})

# `target_compile_definitions` adds some preprocessor definitions to our target. In a Projucer
# project, these might be passed in the 'Preprocessor Definitions' field. JUCE modules also make use
# of compile definitions to switch certain features on/off, so if there's a particular feature you
# need that's not on by default, check the module header for the correct flag to set here. These
# definitions will be visible both to your code, and also the JUCE module code, so for new
# definitions, pick unique names that are unlikely to collide! This is a standard CMake command.

target_compile_definitions(${TargetName}
    PRIVATE
        RUN_UNIT_TESTS=1
        # JUCE_WEB_BROWSER and JUCE_USE_CURL would be on by default, but you might not need them.
        JUCE_WEB_BROWSER=0  # If you remove this, add `NEEDS_WEB_BROWSER TRUE` to the `juce_add_gui_app` call
        JUCE_USE_CURL=0     # If you remove this, add `NEEDS_CURL TRUE` to the `juce_add_gui_app` call
        JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:${TargetName},JUCE_PRODUCT_NAME>"
        JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:${TargetName},JUCE_VERSION>")

# If your target needs extra binary assets, you can add them here. The first argument is the name of
# a new static library target that will include all the binary resources. There is an optional
# `NAMESPACE` argument that can specify the namespace of the generated binary data class. Finally,
# the SOURCES argument should be followed by a list of source files that should be built into the
# static library. These source files can be of any kind (wav data, images, fonts, icons etc.).
# Conversion to binary-data will happen when your target is built.

juce_add_binary_data(FontData NAMESPACE FontData
    SOURCES
        assets/fonts/Fira_Code/static/FiraCode-Bold.ttf
        assets/fonts/IBM_Plex_Sans/static/IBMPlexSans_Condensed-Bold.ttf
    )

file(GLOB IMAGE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/assets/images/*.svg")
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/assets/images")

juce_add_binary_data(ImageData NAMESPACE ImageData HEADER_NAME ImageData.h
    SOURCES
        ${IMAGE_FILES}
    )

target_include_directories(${TargetName} PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/modules/
    ${CMAKE_CURRENT_SOURCE_DIR}/src/
    )
# `target_link_libraries` links libraries and JUCE modules to other libraries or executables. Here,
# we're linking our executable target to the `juce::juce_gui_extra` module. Inter-module
# dependencies are resolved automatically, so `juce_core`, `juce_events` and so on will also be
# linked automatically. If we'd generated a binary data target above, we would need to link to it
# here too. This is a standard CMake command.

target_link_libraries(${TargetName}  
    PRIVATE
        FontData
        ImageData
        juce::juce_gui_extra
        juce::juce_audio_basics
        juce::juce_audio_devices
        cello::cello
        friz::friz
        testSuite::testSuite 
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)

juce_generate_juce_header(${TargetName})

