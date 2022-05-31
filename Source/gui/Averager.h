/*
  ==============================================================================

    Averager.h
    Created: 31 May 2022 12:54:20am
    Author:  Matt Aiken

  ==============================================================================
*/

#pragma once

#include "../Globals.h"

//==============================================================================
template<typename T>
struct Averager
{
    Averager(size_t numElements, T initialValue)
    {
        resize(numElements, initialValue);
    }
    
    void resize(size_t numElements, T initialValue)
    {
        elements.resize(numElements);
        clear(initialValue);
    }
    
    void clear(T initialValue)
    {
        auto elementsSize = getSize();
        elements.assign(elementsSize, initialValue);
        writeIndex.store(0);
        sum.store(static_cast<T>(std::accumulate(elements.begin(), elements.end(), 0)));
        avg.store(sum.load() / elementsSize);
    }
    
    size_t getSize() const
    {
        return elements.size();
    }
    
    void add(T t)
    {
        auto elementsSize = getSize();
        auto idx = writeIndex.load();
        auto runningTotal = sum.load();
        
        runningTotal -= elements[idx];
        runningTotal += t;
        
        elements[idx] = t;
        
        idx++;
        if ( idx >= elementsSize )
            idx = 0;
        
        sum.store(runningTotal);
        writeIndex.store(idx);
        avg.store(sum.load() / elementsSize);
    }
    
    float getAvg() const
    {
        return avg.load();
    }
    
private:
    std::vector<T> elements;
    std::atomic<float> avg { Globals::getNegativeInf() };
    std::atomic<size_t> writeIndex = 0;
    std::atomic<T> sum { 0 };
};
