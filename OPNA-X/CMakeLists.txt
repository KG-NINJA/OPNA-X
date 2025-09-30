cmake_minimum_required(VERSION 3.21)

project(OPNAX VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set_property(GLOBAL PROPERTY USE_FOLDERS YES)

if(NOT DEFINED JUCE_DIR)
    message(FATAL_ERROR "JUCE_DIR must point to a JUCE checkout or install")
endif()

add_subdirectory(${JUCE_DIR} JUCE)

juce_add_plugin(OPNAX
    COMPANY_NAME "OPNA" 
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD TRUE
    PLUGIN_MANUFACTURER_CODE OPNA
    PLUGIN_CODE OPX1
    FORMATS VST3
    PRODUCT_NAME "OPNA-X"
    SOURCES
        Source/FMEngine.h
        Source/FMEngine.cpp
        Source/PCMEngine.h
        Source/PCMEngine.cpp
        Source/SSGEngine.h
        Source/SSGEngine.cpp
        Source/PluginProcessor.h
        Source/PluginProcessor.cpp
        Source/PluginEditor.h
        Source/PluginEditor.cpp)

juce_add_binary_data(OPNAXData
    SOURCES
        Resources/pcm/demo_sample.wav)

juce_generate_juce_header(OPNAX)

target_link_libraries(OPNAX
    PRIVATE
        juce::juce_audio_basics
        juce::juce_audio_formats
        juce::juce_audio_processors
        juce::juce_audio_utils
        juce::juce_gui_basics
        juce::juce_gui_extra
        juce::juce_dsp
        OPNAXData)

target_compile_definitions(OPNAX
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0)
