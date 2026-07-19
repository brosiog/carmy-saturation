#pragma once

#include <juce_dsp/juce_dsp.h>

class SaturationStage
{
public:
    void prepare (const juce::dsp::ProcessSpec& spec);
    void reset();

    void setGirth (float girth01);

    void process (const juce::dsp::ProcessContextReplacing<float>& context);

private:
    float currentDriveGain  { 1.0f };
    float currentMakeupGain { 1.0f };

    float targetDriveGain   { 1.0f };
    float targetMakeupGain  { 1.0f };

    static constexpr float maxDriveDb = 36.0f;

    struct DcBlocker
    {
        float x1 { 0.0f };
        float y1 { 0.0f };

        float process (float x) noexcept
        {
            const float y = x - x1 + 0.995f * y1;
            x1 = x;
            y1 = y;
            return y;
        }

        void reset() noexcept { x1 = 0.0f; y1 = 0.0f; }
    };

    std::vector<DcBlocker> dcBlockers;
};
