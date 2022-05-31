/*
  ==============================================================================

    DbScale.h
    Created: 31 May 2022 1:01:45am
    Author:  Matt Aiken

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
struct Tick
{
    float db { 0.f };
    int y { 0 };
};

struct DbScale : juce::Component
{
    ~DbScale() override = default;
    void paint(juce::Graphics& g) override;
    void buildBackgroundImage(int dbDivision, juce::Rectangle<int> meterBounds, int minDb, int maxDb);
    static std::vector<Tick> getTicks(int dbDivision, juce::Rectangle<int> meterBounds, int minDb, int maxDb);
private:
    juce::Image bkgd;
};
