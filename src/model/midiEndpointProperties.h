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

#include "utility/variantConverters.h"

namespace
{
juce::String makeEndpointIdString (juce::ump::EndpointId endpointId)
{
    return endpointId.get (juce::ump::IOKind::src) + "-" + endpointId.get (juce::ump::IOKind::dst);
}
} // namespace

class MidiEndpointProperties : public cello::Object
{
public:
    static const inline juce::Identifier type { "MidiEndpointProperties" };
    MidiEndpointProperties (juce::ump::EndpointId theEndpointId)
    : cello::Object { type.toString (), nullptr }
    {
        endpointId = theEndpointId;
    }

    MidiEndpointProperties (const juce::ValueTree& valueTree)
    : cello::Object { type.toString (), valueTree }
    {
    }

    MidiEndpointProperties (const MidiEndpointProperties& other)
    : cello::Object { type.toString (), other }
    {
    }

    ~MidiEndpointProperties () override {}

    /// The name of the endpoint.
    MAKE_VALUE_MEMBER (juce::String, name, "");
    /// The endpoint ID.
    MAKE_VALUE_MEMBER (juce::ump::EndpointId, endpointId, {});
    /// The endpoint ID as a string.
    MAKE_COMPUTED_VALUE_MEMBER (juce::String, endpointIdString,
                                [this] () -> juce::String { return makeEndpointIdString (endpointId.get ()); })
    /// Whether the input is alive.
    MAKE_VALUE_MEMBER (bool, isInputAlive, false);
    /// Whether the output is alive.
    MAKE_VALUE_MEMBER (bool, isOutputAlive, false);
    /// true when either the input or output is alive.
    MAKE_COMPUTED_VALUE_MEMBER (bool, isAlive,
                                [this] () -> bool { return isInputAlive.get () || isOutputAlive.get (); });
    /// This is used to track the number of messages received from this endpoint.
    MAKE_VALUE_MEMBER (int, rxCount, 0);
    /// keep track of the number of messages transmitted to this endpoint.
    MAKE_VALUE_MEMBER (int, txCount, 0);
    /// TEMPORARY DEBUG: UUID string for debugging purposes.
    MAKE_VALUE_MEMBER (juce::String, debugUuid, juce::Uuid ().toString ());
};
