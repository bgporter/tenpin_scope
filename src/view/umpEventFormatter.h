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

#include "model/ump/channelVoice2.h"
#include "model/ump/umpEvent.h"

/**
 * @brief Format a UmpEvent into a human-readable string description.
 * This function extracts formatting logic that was previously in DataViewHandler
 * so it can be reused for both real-time event display and virtual scrolling.
 */
inline juce::String formatUmpEvent (const UmpEvent& event)
{
    juce::String description;
    description << event.timestamp << " ";

    // Determine message type and format accordingly
    int messageType = event.messageType.get ();

    if (messageType == MessageTypes::midi2ChannelVoice)
    {
        // Get the status from the first word
        uint32_t data0 = event.getattr<uint32_t> (UmpWords::data0Id, 0);
        int status     = (data0 >> 20) & 0xF;

        switch (status)
        {
            case UmpValues::noteOff:
            {
                Midi2NoteOffEvent midi2Event (event);
                description << "Group: " << midi2Event.userGroup << " Channel: " << midi2Event.userChannel
                            << " -- Note Off";
                break;
            }
            case UmpValues::noteOn:
            {
                Midi2NoteOnEvent midi2Event (event);
                description << "Group: " << midi2Event.userGroup << " Channel: " << midi2Event.userChannel
                            << " -- Note On";
                break;
            }
            case UmpValues::polyPressure:
            {
                Midi2PolyPressureEvent midi2Event (event);
                description << "Group: " << midi2Event.userGroup << " Channel: " << midi2Event.userChannel
                            << " -- Poly Pressure";
                break;
            }
            case UmpValues::controlChange:
            {
                Midi2ControlChangeEvent midi2Event (event);
                description << "Group: " << midi2Event.userGroup << " Channel: " << midi2Event.userChannel
                            << " -- Control Change";
                break;
            }
            case UmpValues::programChange:
            {
                Midi2ProgramChangeEvent midi2Event (event);
                description << "Group: " << midi2Event.userGroup << " Channel: " << midi2Event.userChannel
                            << " -- Program Change";
                break;
            }
            case UmpValues::channelPressure:
            {
                Midi2ChannelPressureEvent midi2Event (event);
                description << "Group: " << midi2Event.userGroup << " Channel: " << midi2Event.userChannel
                            << " -- Channel Pressure";
                break;
            }
            case UmpValues::pitchBend:
            {
                Midi2PitchBendEvent midi2Event (event);
                description << "Group: " << midi2Event.userGroup << " Channel: " << midi2Event.userChannel
                            << " -- Pitch Bend";
                break;
            }
            case UmpValues::perNoteManagement:
            {
                Midi2PerNoteManagementEvent midi2Event (event);
                description << "Group: " << midi2Event.userGroup << " Channel: " << midi2Event.userChannel
                            << " -- Per Note Management";
                break;
            }
            case UmpValues::registeredPerNoteController:
            {
                Midi2RegisteredPerNoteControllerEvent midi2Event (event);
                description << "Group: " << midi2Event.userGroup << " Channel: " << midi2Event.userChannel
                            << " -- Registered Per Note Controller";
                break;
            }
            case UmpValues::assignablePerNoteController:
            {
                Midi2AssignablePerNoteControllerEvent midi2Event (event);
                description << "Group: " << midi2Event.userGroup << " Channel: " << midi2Event.userChannel
                            << " -- Assignable Per Note Controller";
                break;
            }
            case UmpValues::registeredController:
            {
                Midi2RegisteredControllerEvent midi2Event (event);
                description << "Group: " << midi2Event.userGroup << " Channel: " << midi2Event.userChannel
                            << " -- Registered Controller";
                break;
            }
            case UmpValues::assignableController:
            {
                Midi2AssignableControllerEvent midi2Event (event);
                description << "Group: " << midi2Event.userGroup << " Channel: " << midi2Event.userChannel
                            << " -- Assignable Controller";
                break;
            }
            case UmpValues::relativeRegisteredController:
            {
                Midi2RelativeRegisteredControllerEvent midi2Event (event);
                description << "Group: " << midi2Event.userGroup << " Channel: " << midi2Event.userChannel
                            << " -- Relative Registered Controller";
                break;
            }
            case UmpValues::relativeAssignableController:
            {
                Midi2RelativeAssignableControllerEvent midi2Event (event);
                description << "Group: " << midi2Event.userGroup << " Channel: " << midi2Event.userChannel
                            << " -- Relative Assignable Controller";
                break;
            }
            case UmpValues::perNotePitchBend:
            {
                Midi2PerNotePitchBendEvent midi2Event (event);
                description << "Group: " << midi2Event.userGroup << " Channel: " << midi2Event.userChannel
                            << " -- Per Note Pitch Bend";
                break;
            }
            default:
            {
                Midi2ChannelVoiceEvent midi2Event (event);
                description << "Group: " << midi2Event.userGroup << " Channel: " << midi2Event.userChannel
                            << " -- MIDI2 Channel Voice";
                break;
            }
        }
    }
    else
    {
        // Generic UMP event formatting
        description << "UMP DATA: ";
        description << juce::String::formatted ("%08X ", event.getattr<uint32_t> (UmpWords::data0Id, 0));
        description << juce::String::formatted ("%08X", event.getattr<uint32_t> (UmpWords::data1Id, 0));
    }

    return description;
}
