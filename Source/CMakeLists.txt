target_sources("${PROJECT_NAME}"
    PRIVATE
        PluginProcessor.cpp
        PluginProcessor.h
        PluginEditor.cpp
        PluginEditor.h
        DSP/SaturationStage.cpp
        DSP/SaturationStage.h
        DSP/CompressorStage.cpp
        DSP/CompressorStage.h
        DSP/TiltFilter.cpp
        DSP/TiltFilter.h
        UI/KnobComponent.cpp
        UI/KnobComponent.h)

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES
    PluginProcessor.cpp
    PluginProcessor.h
    PluginEditor.cpp
    PluginEditor.h
    DSP/SaturationStage.cpp
    DSP/SaturationStage.h
    DSP/CompressorStage.cpp
    DSP/CompressorStage.h
    DSP/TiltFilter.cpp
    DSP/TiltFilter.h
    UI/KnobComponent.cpp
    UI/KnobComponent.h)
