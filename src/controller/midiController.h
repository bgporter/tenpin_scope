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

#include "model/appContext.h"
#include "model/midiProperties.h"
#include "model/persistentContext.h"
#include "model/runtimeContext.h"

class EndpointController;
class MidiController : public juce::ump::EndpointsListener
{
public:
    MidiController (juce::StringRef sessionName, const AppContext& appContext);

    ~MidiController () override;

    /**
     * @brief This controller owns the one Session object that
     * manages all MIDI connections for the lifetime of the app.
     *
     * @return juce::ump::Session*
     */
    juce::ump::Session* getSession () { return &session; }

    void endpointsChanged () override;

    void virtualMidiServiceActiveChanged () override {}

private:
    void addEndpointController (juce::ump::EndpointId endpointId);
    void updateEndpointController (juce::ump::EndpointId endpointId);

    RuntimeContext runtimeContext;
    MidiProperties midiProperties;
    juce::ump::Endpoints* endpoints;
    juce::ump::Session session;
    std::vector<std::unique_ptr<EndpointController>> endpointControllers;
};