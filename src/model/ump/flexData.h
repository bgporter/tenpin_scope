/*
 MIT License

 Copyright (c) 2026 Brett g Porter

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 SOFTWARE.
 */

#pragma once

#include <JuceHeader.h>

#include "../midiTypes.h"
#include "umpEvent.h"

enum class FlexDataFormat
{
    complete  = 0,
    start     = 1,
    continue_ = 2,
    end       = 3,
};

enum class FlexDataAddress
{
    channel = 0,
    group   = 1,
};

enum class FlexDataStatusBank
{
    setupAndPerformance = 0x00,
    metadataText        = 0x01,
    performanceText     = 0x02,
};

enum class SetupAndPerformanceStatus
{
    setTempo         = 0x00,
    setTimeSignature = 0x01,
    setMetronome     = 0x02,
    setKeySignature  = 0x05,
};

enum class TonicNote
{
    unknown = 0x0,
    a       = 0x1,
    b       = 0x2,
    c       = 0x3,
    d       = 0x4,
    e       = 0x5,
    f       = 0x6,
    g       = 0x7,
};

namespace juce
{
template <> struct VariantConverter<FlexDataFormat>
{
    static FlexDataFormat fromVar (const juce::var& v)
    {
        return static_cast<FlexDataFormat> (std::clamp (static_cast<int> (v), 0, 3));
    }
    static juce::var toVar (FlexDataFormat val) { return static_cast<int> (val); }
};

template <> struct VariantConverter<FlexDataAddress>
{
    static FlexDataAddress fromVar (const juce::var& v)
    {
        return static_cast<FlexDataAddress> (std::clamp (static_cast<int> (v), 0, 3));
    }
    static juce::var toVar (FlexDataAddress val) { return static_cast<int> (val); }
};

template <> struct VariantConverter<FlexDataStatusBank>
{
    static FlexDataStatusBank fromVar (const juce::var& v)
    {
        return static_cast<FlexDataStatusBank> (static_cast<int> (v));
    }
    static juce::var toVar (FlexDataStatusBank val) { return static_cast<int> (val); }
};

template <> struct VariantConverter<TonicNote>
{
    static TonicNote fromVar (const juce::var& v)
    {
        return static_cast<TonicNote> (std::clamp (static_cast<int> (v), 0, 7));
    }
    static juce::var toVar (TonicNote val) { return static_cast<int> (val); }
};
} // namespace juce

struct FlexDataEvent : public UmpEvent
{
    FlexDataEvent (const UmpEvent& event)
    : UmpEvent (event)
    {
    }

    // Word 0
    MAKE_BITFIELD (int,              group,      0, 4, 24);
    MAKE_BITFIELD (FlexDataFormat,   format,     0, 2, 22);
    MAKE_BITFIELD (FlexDataAddress,  address,    0, 2, 20);
    MAKE_BITFIELD (int,              channel,    0, 4, 16);
    MAKE_BITFIELD (FlexDataStatusBank, statusBank, 0, 8, 8);
    MAKE_BITFIELD (int,              status,     0, 8,  0);

    MAKE_COMPUTED_VALUE_MEMBER (
        int, userGroup,
        [this] () -> int { return group.get () + 1; },
        [this] (const int& val) { group = val - 1; });

    MAKE_COMPUTED_VALUE_MEMBER (
        int, userChannel,
        [this] () -> int { return channel.get () + 1; },
        [this] (const int& val) { channel = val - 1; });

protected:
    template <typename StatusEnum>
    FlexDataEvent (MidiGroup theGroup, FlexDataFormat theFormat, FlexDataAddress theAddress,
                   int theChannel, FlexDataStatusBank theStatusBank, StatusEnum theStatus)
    : UmpEvent ()
    {
        setattr<uint32_t> (UmpWords::data0Id, 0);
        setattr<uint32_t> (UmpWords::data1Id, 0);
        setattr<uint32_t> (UmpWords::data2Id, 0);
        setattr<uint32_t> (UmpWords::data3Id, 0);
        messageType = MessageTypes::flexData;
        userGroup   = theGroup;
        format      = theFormat;
        address     = theAddress;
        channel     = theChannel;
        statusBank  = theStatusBank;
        status      = static_cast<int> (theStatus);
    }
};

// ---------------------------------------------------------------------------

inline uint32_t bpmToTenNs (double bpm)
{
    return static_cast<uint32_t> (6'000'000'000.0 / bpm);
}

inline double tenNsToBpm (uint32_t tenNsPerQN)
{
    return 6'000'000'000.0 / static_cast<double> (tenNsPerQN);
}

// ---------------------------------------------------------------------------

struct SetTempoEvent : public FlexDataEvent
{
    SetTempoEvent (const UmpEvent& event)
    : FlexDataEvent (event)
    {
        init ();
    }

    SetTempoEvent (MidiGroup theGroup, double bpm)
    : FlexDataEvent (theGroup,
                     FlexDataFormat::complete,
                     FlexDataAddress::group,
                     0,
                     FlexDataStatusBank::setupAndPerformance,
                     SetupAndPerformanceStatus::setTempo)
    {
        init ();
        tenNsPerQuarterNote = bpmToTenNs (bpm);
    }

    // Word 1: full 32-bit tempo value (10-nanosecond units per quarter note)
    MAKE_BITFIELD (uint32_t, tenNsPerQuarterNote, 1, 32, 0);

private:
    void init () { eventName = "Flex Data: Set Tempo"; }
};

// ---------------------------------------------------------------------------

struct SetTimeSignatureEvent : public FlexDataEvent
{
    SetTimeSignatureEvent (const UmpEvent& event)
    : FlexDataEvent (event)
    {
        init ();
    }

    // Construct from musical time signature values.
    // denominatorPower: the power-of-2 exponent (2=quarter, 3=eighth, etc.; 0=non-standard)
    // num32ndNotes: number of 1/32 notes per 24 MIDI clocks (typically 8)
    SetTimeSignatureEvent (MidiGroup theGroup, int theNumerator, int theDenominatorPower,
                           int theNum32ndNotes)
    : FlexDataEvent (theGroup,
                     FlexDataFormat::complete,
                     FlexDataAddress::group,
                     0,
                     FlexDataStatusBank::setupAndPerformance,
                     SetupAndPerformanceStatus::setTimeSignature)
    {
        init ();
        numerator        = theNumerator;
        denominatorPower = theDenominatorPower;
        num32ndNotes     = theNum32ndNotes;
    }

    // Word 1
    MAKE_BITFIELD (int, numerator,        1, 8, 24);
    MAKE_BITFIELD (int, denominatorPower, 1, 8, 16);
    MAKE_BITFIELD (int, num32ndNotes,     1, 8,  8);

private:
    void init () { eventName = "Flex Data: Set Time Signature"; }
};

// ---------------------------------------------------------------------------

struct SetMetronomeEvent : public FlexDataEvent
{
    SetMetronomeEvent (const UmpEvent& event)
    : FlexDataEvent (event)
    {
        init ();
    }

    SetMetronomeEvent (MidiGroup theGroup, int theNumClocksPerPrimaryClick,
                       int theBarAccentPart1, int theBarAccentPart2, int theBarAccentPart3,
                       int theNumSubdivisionClicks1, int theNumSubdivisionClicks2)
    : FlexDataEvent (theGroup,
                     FlexDataFormat::complete,
                     FlexDataAddress::group,
                     0,
                     FlexDataStatusBank::setupAndPerformance,
                     SetupAndPerformanceStatus::setMetronome)
    {
        init ();
        numClocksPerPrimaryClick = theNumClocksPerPrimaryClick;
        barAccentPart1           = theBarAccentPart1;
        barAccentPart2           = theBarAccentPart2;
        barAccentPart3           = theBarAccentPart3;
        numSubdivisionClicks1    = theNumSubdivisionClicks1;
        numSubdivisionClicks2    = theNumSubdivisionClicks2;
    }

    // Word 1
    MAKE_BITFIELD (int, numClocksPerPrimaryClick, 1, 8, 24);
    MAKE_BITFIELD (int, barAccentPart1,           1, 8, 16);
    MAKE_BITFIELD (int, barAccentPart2,           1, 8,  8);
    MAKE_BITFIELD (int, barAccentPart3,           1, 8,  0);

    // Word 2
    MAKE_BITFIELD (int, numSubdivisionClicks1, 2, 8, 24);
    MAKE_BITFIELD (int, numSubdivisionClicks2, 2, 8, 16);

private:
    void init () { eventName = "Flex Data: Set Metronome"; }
};

// ---------------------------------------------------------------------------

struct SetKeySignatureEvent : public FlexDataEvent
{
    SetKeySignatureEvent (const UmpEvent& event)
    : FlexDataEvent (event)
    {
        init ();
    }

    // theSharpsFlats: +1..+7 = sharps, 0 = none, -1..-7 = flats, -8 = unknown/non-standard
    SetKeySignatureEvent (MidiGroup theGroup, int theSharpsFlats, TonicNote theTonicNote)
    : FlexDataEvent (theGroup,
                     FlexDataFormat::complete,
                     FlexDataAddress::group,
                     0,
                     FlexDataStatusBank::setupAndPerformance,
                     SetupAndPerformanceStatus::setKeySignature)
    {
        init ();
        sharpsFlats = theSharpsFlats;
        tonicNote   = theTonicNote;
    }

    // Signed view of the 4-bit two's complement sharps/flats field.
    MAKE_COMPUTED_VALUE_MEMBER (
        int, sharpsFlats,
        [this] () -> int {
            const int raw = sharpsFlatsRaw_.get ();
            return (raw >= 8) ? (raw - 16) : raw;
        },
        [this] (const int& val) { sharpsFlatsRaw_ = val & 0xF; });

    MAKE_BITFIELD (TonicNote, tonicNote, 1, 4, 24);

private:
    MAKE_BITFIELD (int, sharpsFlatsRaw_, 1, 4, 28);
    void init () { eventName = "Flex Data: Set Key Signature"; }
};
