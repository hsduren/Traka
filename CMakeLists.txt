cmake_minimum_required(VERSION 3.22)
if(NOT CMAKE_OSX_DEPLOYMENT_TARGET)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "13.0" CACHE STRING "Minimum macOS" FORCE)
endif()
project(Traka VERSION 0.1.1 LANGUAGES C CXX)
set(TE_ADD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(vendor/tracktion_engine)
juce_add_gui_app(Traka PRODUCT_NAME "Traka" BUNDLE_ID "com.yxf.traka"
    COMPANY_NAME "Traka"
    COMPANY_COPYRIGHT "Copyright (C) 2026 hsduren and Traka contributors"
    MICROPHONE_PERMISSION_ENABLED TRUE
    MICROPHONE_PERMISSION_TEXT "Traka records audio onto the track you select."
    NEEDS_CURL FALSE NEEDS_WEB_BROWSER FALSE)
juce_generate_juce_header(Traka)
set_source_files_properties(Resources/AppIcon.icon PROPERTIES
    MACOSX_PACKAGE_LOCATION Resources XCODE_EXPLICIT_FILE_TYPE folder.iconcomposer.icon)
target_sources(Traka PRIVATE Resources/AppIcon.icon Source/Main.cpp Source/Session.cpp Source/Studio.cpp Source/SelfTest.cpp Source/UISmoke.cpp)
set_target_properties(Traka PROPERTIES XCODE_ATTRIBUTE_ASSETCATALOG_COMPILER_APPICON_NAME AppIcon)
set_property(TARGET Traka APPEND PROPERTY LINK_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/Resources/AppIcon.icon/icon.json"
    "${CMAKE_CURRENT_SOURCE_DIR}/Resources/AppIcon.icon/Assets/TrakaConsole.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/scripts/compile_icon.py")
target_compile_features(Traka PRIVATE cxx_std_20)
target_compile_definitions(Traka PRIVATE JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_MODAL_LOOPS_PERMITTED=1
    TRAKA_VERSION="${PROJECT_VERSION}" TRAKA_SOURCE_URL="https://github.com/hsduren/Traka"
    JUCE_DISPLAY_SPLASH_SCREEN=0 JUCE_REPORT_APP_USAGE=0 TRACKTION_ENABLE_TIMESTRETCH_SOUNDTOUCH=1)
target_compile_options(Traka PRIVATE -fno-aligned-allocation)
target_link_libraries(Traka PRIVATE tracktion::tracktion_engine tracktion::tracktion_core tracktion::tracktion_graph
    juce::juce_audio_utils juce::juce_audio_processors juce::juce_recommended_config_flags)
enable_testing()
add_test(NAME audio_workflow COMMAND "$<TARGET_FILE:Traka>" --self-test)
set_tests_properties(audio_workflow PROPERTIES TIMEOUT 90)

configure_file(Resources/SOURCE.txt.in generated/SOURCE.txt @ONLY)

# Carry the actual license texts and corresponding-source URL with every app build.
add_custom_command(TARGET Traka POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_BUNDLE_DIR:Traka>/Contents/Resources/OpenSource"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/Resources/OpenSource"
        "$<TARGET_BUNDLE_DIR:Traka>/Contents/Resources/OpenSource"
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_BINARY_DIR}/generated/SOURCE.txt"
        "$<TARGET_BUNDLE_DIR:Traka>/Contents/Resources/OpenSource/SOURCE.txt"
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE"
        "$<TARGET_BUNDLE_DIR:Traka>/Contents/Resources/OpenSource/AGPL-3.0.txt"
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/THIRD_PARTY_NOTICES.md"
        "$<TARGET_BUNDLE_DIR:Traka>/Contents/Resources/OpenSource/THIRD_PARTY_NOTICES.md")

find_package(Python3 REQUIRED COMPONENTS Interpreter)
add_custom_command(TARGET Traka POST_BUILD
    COMMAND ${Python3_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/scripts/compile_icon.py"
        "$<TARGET_BUNDLE_DIR:Traka>" "${CMAKE_CURRENT_SOURCE_DIR}/Resources/AppIcon.icon"
    VERBATIM)
