/*
  ==============================================================================

    Fifo.h
    Created: 22 Jun 2022 1:29:22am
    Author:  Matt Aiken

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
template<typename T>
struct IsRefCountedObjectPtr : std::false_type { };

template<typename T>
struct IsRefCountedObjectPtr<juce::ReferenceCountedObjectPtr<T>> : std::true_type { };

template<typename T>
struct IsRefCountedArray : std::false_type { };

template<typename T>
struct IsRefCountedArray<juce::ReferenceCountedArray<T>> : std::true_type { };

template<typename T, size_t Size>
struct Fifo
{
    size_t getSize() const noexcept
    {
        return Size;
    }
    
    // used when T is AudioBuffer<float>
    void prepare(int numSamples, int numChannels)
    {
        static_assert( std::is_same<T, juce::AudioBuffer<float>>::value == true );
        for ( auto& buffer : buffers)
        {
            buffer.setSize(numChannels, numSamples, false, true, true);
            buffer.clear();
        }
    }
    
    // used when T is std::vector<float>
    void prepare(size_t numElements)
    {
        static_assert( std::is_same<T, std::vector<float>>::value == true );
        for ( auto& buffer : buffers)
        {
            buffer.clear();
            buffer.resize(numElements, 0);
        }
    }
    
    bool push(const T& t)
    {
        auto write = fifo.write(1);
        if ( write.blockSize1 > 0 )
        {
            size_t index = static_cast<size_t>(write.startIndex1);
            if constexpr( IsRefCountedObjectPtr<T>::value )
            {
                T old = buffers[index];
                buffers[index] = t;
                jassert( old.get() == nullptr || old->getReferenceCount() > 1 );
            }
            else
            {
                buffers[index] = t;
            }
            return true;
        }
        return false;
    }
    
    bool pull(T& t)
    {
        auto read = fifo.read(1);
        if ( read.blockSize1 > 0 )
        {
            t = buffers[read.startIndex1];
            return true;
        }
        return false;
    }
    
    int getNumAvailableForReading() const
    {
        return fifo.getNumReady();
    }
    
    int getAvailableSpace() const
    {
        return fifo.getFreeSpace();
    }
    
    void exchange(T&& t)
    {
        auto read = fifo.read(1);
        if ( read.blockSize1 > 0 )
        {
            auto idx = read.startIndex1;
            if constexpr( IsRefCountedObjectPtr<T>::value )
            {
                std::swap(t, buffers[idx]);
                jassert( buffers[idx] == nullptr );
            }
            else if constexpr( IsRefCountedArray<T>::value )
            {
                std::swap(t, buffers[idx]);
                jassert( buffers[idx].size() == 0 );
            }
            else if constexpr( std::is_same<T, std::vector<float>>::value == true )
            {
                if ( t.size() < buffers[idx].size() )
                {
                    buffers[idx] = t;
                }
                else
                {
                    std::swap(t, buffers[idx]);
                }
            }
            else if constexpr( std::is_same<T, juce::AudioBuffer<float>>::value == true )
            {
                if ( t.getNumSamples() < buffers[idx].getNumSamples() )
                {
                    buffers[idx] = t;
                }
                else
                {
                    std::swap(t, buffers[idx]);
                }
            }
            else
            {
                std::swap(t, buffers[idx]);
            }
        }
    }
    
private:
    juce::AbstractFifo fifo { Size };
    std::array<T, Size> buffers;
};
