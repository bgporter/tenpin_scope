/*
 MIT License

 Copyright (c) 2026 Brett g Porter

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to so, subject to the following conditions:

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
#include <array>
#include <queue>

#include "model/appContext.h"
#include "model/midiEndpointProperties.h"
#include "model/midiProperties.h"
#include "model/ump/umpEvent.h"
#include "model/sysex/mdsBuilder.h"
#include "model/sysex/sysex7Builder.h"
#include "model/sysex/sysex8Builder.h"
#include "model/sysex/textBuilder.h"

/// @brief Raw packet data to queue from MIDI thread for processing on message thread
struct RawPacketData
{
    std::array<uint32_t, 4> data;
    size_t size;
    double timestamp;
    int endpointIndex;
};

class EndpointController : public juce::ump::DisconnectionListener,
                           private juce::ump::Consumer
{
public:
    /**
     * @brief Construct a new Endpoint Controller object.
     *
     * @param index The index of this endpoint.
     * @param id -- NOTE that we don't pass in an endpoint object, because
     * its state is ephemeral; when you need updated endpoint information, fetch a
     * fresh Endpoint instance using the ID.
     * @param midiProperties The MidiProperties object to append this endpoint's properties to.
     */
    EndpointController (int index, juce::ump::EndpointId id, const MidiProperties& midiProperties,
                        AppContext& appContext);

    /**
     * @brief Destructor for the Endpoint Controller object.
     */
    ~EndpointController () override;

    juce::ValueTree getEndpointProperties () const { return midiEndpointProperties; }
    MidiEndpointProperties& getMidiEndpointProperties () { return midiEndpointProperties; }

    /**
     * @brief attempt to make fresh input/output connections.
     *
     * @param session The session to use for connecting the endpoint.
     */
    void connectEndpoint (juce::ump::Session* session);

    void disconnected () override;

    juce::ump::EndpointId getEndpointId () const { return midiEndpointProperties.endpointId; }

    void processUmpEvents ();

private:
    void consume (juce::ump::Iterator b, juce::ump::Iterator e, double time) override;
    void createUmpEvent (const juce::ump::View& packet, double time);

private:
    int endpointIndex { -1 };
    juce::ump::Input input;
    juce::ump::Output output;
    MidiProperties midiProperties;
    MidiEndpointProperties midiEndpointProperties;
    RuntimeContext runtimeContext;
    std::optional<Sysex7Builder> sysex7Builder;
    std::optional<Sysex8Builder> sysex8Builder;
    std::optional<MdsBuilder>    mdsBuilder;
    std::optional<TextBuilder>   textBuilder;
    /// @brief The time when the first packet was received from any endpoint.
    static inline double startTime { -1 };
    std::queue<RawPacketData> eventQueue;
    juce::CriticalSection queueLock;
};
