/*
  ==============================================================================

    RotaryControl.cpp
    Created: 1 Jun 2022 12:30:34am
    Author:  Matt Aiken

  ==============================================================================
*/

#include "RotaryControl.h"
#include "../ColourPalette.h"

//==============================================================================
RotaryControl::RotaryControl(juce::RangedAudioParameter& rap, const juce::String& unitSuffix, const juce::String& title)
    : juce::Slider(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag, juce::Slider::TextEntryBoxPosition::NoTextBox),
      param(&rap),
      suffix(unitSuffix)
{
    setLookAndFeel(&lnf);
    setLabels();
    setName(title);
}

void RotaryControl::setLabels()
{
    labels.clear();
    if (auto* choiceParam = dynamic_cast<juce::AudioParameterChoice*>(param))
    {
        auto choices = choiceParam->choices;

        for ( auto& choice : choices )
        {
            if ( choice.substring(choice.indexOf(".")) != ".5" )
                choice = choice.substring(0, choice.indexOf("."));
        }
        
        labels.add({0.f, choices[0] + suffix});
        labels.add({1.f, choices[choices.size() - 1] + suffix});
    }
    else
    {
        auto range = param->getNormalisableRange();
        labels.add({0.f, juce::String(range.start) + suffix});
        labels.add({1.f, juce::String(range.end) + suffix});
    }
}

void RotaryControl::paint(juce::Graphics& g)
{
    auto startAngle = juce::degreesToRadians(180.f + 45.f);
    auto endAngle = juce::degreesToRadians(180.f - 45.f) + juce::MathConstants<float>::twoPi;

    auto range = param->getNormalisableRange();
    auto bounds = getLocalBounds();
    
    g.setColour(ColourPalette::getColour(ColourPalette::Text));
    g.drawFittedText(getName(),
                     bounds.removeFromTop(getTextHeight() + 3),
                     juce::Justification::centred,
                     1);
    
    auto valueToDraw = juce::jmap<float>(getValue(),
                                         range.start,
                                         range.end,
                                         startAngle,
                                         endAngle);
    
    auto rotaryBounds = getRotaryBounds();
    getLookAndFeel().drawRotarySlider(g,
                                      rotaryBounds.getX(),      // x
                                      rotaryBounds.getY(),      // y
                                      rotaryBounds.getWidth(),  // width
                                      rotaryBounds.getHeight(), // height
                                      valueToDraw,              // position
                                      startAngle,               // start angle
                                      endAngle,                 // end angle
                                      *this);                   // slider
    
    auto center = rotaryBounds.toFloat().getCentre();
    auto radius = rotaryBounds.getWidth() * 0.5f;
    
    g.setFont(getTextHeight());
    
    // current value string
    g.setColour(ColourPalette::getColour(ColourPalette::Text));
    juce::Rectangle<float> r;
    auto str = getDisplayString();
    r.setSize(g.getCurrentFont().getStringWidth(str), getTextHeight());
    r.setCentre(center);
    g.drawFittedText(str, r.toNearestInt(), juce::Justification::centred, 1);
    
    // value range labels
    for ( auto i = 0; i < labels.size(); ++i )
    {
        auto pos = labels[i].pos;
        jassert(0.f <= pos);
        jassert(pos <= 1.f);
        
        auto angle = juce::jmap<float>(pos, 0.f, 1.f, startAngle, endAngle);
        auto c = center.getPointOnCircumference(radius + getTextHeight() * 0.5f + 1, angle);
        
        juce::Rectangle<float> r;
        auto str = labels[i].label;
        r.setSize(g.getCurrentFont().getStringWidth(str), getTextHeight());
        r.setCentre(c);
        r.setY(r.getY() + getTextHeight());
        
        g.drawFittedText(str, r.toNearestInt(), juce::Justification::centred, 1);
    }
}

juce::Rectangle<int> RotaryControl::getRotaryBounds() const
{
    auto bounds = getLocalBounds();

    auto size = juce::jmin(bounds.getWidth(), bounds.getHeight());

    size -= getTextHeight() * 2;

    juce::Rectangle<int> r;
    r.setSize(size, size);
    r.setCentre(bounds.getCentreX(), 0);
    r.setY(bounds.getCentreY() - (size * 0.5));

    return r;
}

juce::String RotaryControl::getDisplayString() const
{
    juce::String str;
    
    if (auto* choiceParam = dynamic_cast<juce::AudioParameterChoice*>(param))
    {
        auto currentChoice = choiceParam->getCurrentChoiceName();
        str = ( currentChoice.substring(currentChoice.indexOf(".")) == ".5" )
            ? currentChoice
            : currentChoice.substring(0, currentChoice.indexOf("."));
    }
    else if (auto* floatParam = dynamic_cast<juce::AudioParameterFloat*>(param))
        str = juce::String(getValue());
    else
        jassertfalse;

    str << suffix;

    return str;
}
