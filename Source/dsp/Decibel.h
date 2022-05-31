/*
  ==============================================================================

    Decibel.h
    Created: 31 May 2022 8:15:47am
    Author:  Matt Aiken

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
template<typename FloatType>
struct Decibel
{
    Decibel() { }
    Decibel(const FloatType& val) : floatVal(val) { }
    
    // rule of 3: copy constructor, assignment copy operator, destructor
    Decibel(const Decibel& other) : floatVal(other.floatVal) { }
    Decibel& operator=(const Decibel& other) { floatVal = other.floatVal; return *this; }
    ~Decibel() { }
    
    Decibel& operator+=(const Decibel& other) { floatVal += other.floatVal; return *this; }
    Decibel& operator-=(const Decibel& other) { floatVal -= other.floatVal; return *this; }
    Decibel& operator*=(const Decibel& other) { floatVal *= other.floatVal; return *this; }
    Decibel& operator/=(const Decibel& other)
    {
        if ( other.floatVal != 0 )
            floatVal /= other.floatVal;
        
        return *this;
    }
    
    friend FloatType operator+(const Decibel& d1, const Decibel& d2) { return d1.floatVal + d2.floatVal; }
    friend FloatType operator-(const Decibel& d1, const Decibel& d2) { return d1.floatVal - d2.floatVal; }
    friend FloatType operator*(const Decibel& d1, const Decibel& d2) { return d1.floatVal * d2.floatVal; }
    friend FloatType operator/(const Decibel& d1, const Decibel& d2)
    {
        if ( d2.floatVal == 0 )
            return d1.floatVal;
        
        return d1.floatVal / d2.floatVal;
    }
    
    friend bool operator==(const Decibel& d1, const Decibel& d2) { return d1.floatVal == d2.floatVal; }
    friend bool operator!=(const Decibel& d1, const Decibel& d2) { return d1.floatVal != d2.floatVal; }
    friend bool operator> (const Decibel& d1, const Decibel& d2) { return d1.floatVal  > d2.floatVal; }
    friend bool operator< (const Decibel& d1, const Decibel& d2) { return d1.floatVal  < d2.floatVal; }
    friend bool operator>=(const Decibel& d1, const Decibel& d2) { return d1.floatVal >= d2.floatVal; }
    friend bool operator<=(const Decibel& d1, const Decibel& d2) { return d1.floatVal <= d2.floatVal; }
    
    FloatType getGain() const { return juce::Decibels::decibelsToGain(floatVal); }
    FloatType getDb()   const { return floatVal; }
    
    void setGain(FloatType g) const { floatVal = juce::Decibels::gainToDecibels(g); }
    void setDb(FloatType db)  const { floatVal = db; }
private:
    FloatType floatVal;
};
