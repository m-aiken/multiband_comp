/*
  ==============================================================================

    Averager.h
    Created: 31 May 2022 12:54:20am
    Author:  Matt Aiken

  ==============================================================================
*/

#pragma once

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
        elements.assign(getSize(), initialValue);
        writeIndex.store(0);
        sum.store(static_cast<T>(std::accumulate(elements.begin(), elements.end(), 0)));
        avg.store(sum.load() / getSize());
    }
    
    size_t getSize() const
    {
        return elements.size();
    }
    
    void add(T t)
    {
        auto idx = writeIndex.load();
        auto runningTotal = sum.load();
        
        runningTotal -= elements[idx];
        runningTotal += t;
        
        elements[idx] = t;
        
        idx = (idx + 1) % getSize();
        
        sum.store(runningTotal);
        writeIndex.store(idx);
        avg.store(sum.load() / getSize());
    }
    
    T getAvg() const
    {
        return avg.load();
    }
    
private:
    std::vector<T> elements;
    std::atomic<T> avg { T() };
    std::atomic<size_t> writeIndex = 0;
    std::atomic<T> sum { 0 };
};
