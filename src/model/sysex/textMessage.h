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

#include "model/event.h"

/**
 * @brief Assembled message for all FlexData metadata/performance text events and
 * UMP stream text events (endpoint name, product instance ID, function block name).
 *
 * The ValueTree type name identifies the specific text type (e.g. "MsgCompositionName").
 * All 19 variants share the same fields; unused fields are zero.
 *
 * FlexData fields: group, channel, address, statusBank, status, text.
 * StreamText fields: status, functionBlockNumber (only for MsgFunctionBlockName), text.
 */
struct TextMessage : public Event
{
    // -------------------------------------------------------------------------
    // FlexData metadata text type identifiers
    static const inline juce::Identifier typeProjectName           { "MsgProjectName" };
    static const inline juce::Identifier typeCompositionName       { "MsgCompositionName" };
    static const inline juce::Identifier typeMidiClipName          { "MsgMidiClipName" };
    static const inline juce::Identifier typeCopyrightNotice       { "MsgCopyrightNotice" };
    static const inline juce::Identifier typeComposerName          { "MsgComposerName" };
    static const inline juce::Identifier typeLyricistName          { "MsgLyricistName" };
    static const inline juce::Identifier typeArrangerName          { "MsgArrangerName" };
    static const inline juce::Identifier typePublisherName         { "MsgPublisherName" };
    static const inline juce::Identifier typePrimaryPerformer      { "MsgPrimaryPerformer" };
    static const inline juce::Identifier typeAccompanyingPerformer { "MsgAccompanyingPerformer" };
    static const inline juce::Identifier typeRecordingDate         { "MsgRecordingDate" };
    static const inline juce::Identifier typeRecordingLocation     { "MsgRecordingLocation" };

    // FlexData performance text type identifiers
    static const inline juce::Identifier typeLyrics                { "MsgLyrics" };
    static const inline juce::Identifier typeLyricsLanguage        { "MsgLyricsLanguage" };
    static const inline juce::Identifier typeRuby                  { "MsgRuby" };
    static const inline juce::Identifier typeRubyLanguage          { "MsgRubyLanguage" };

    // Stream text type identifiers
    static const inline juce::Identifier typeEndpointName          { "MsgEndpointName" };
    static const inline juce::Identifier typeProductInstanceId     { "MsgProductInstanceId" };
    static const inline juce::Identifier typeFunctionBlockName     { "MsgFunctionBlockName" };

    // -------------------------------------------------------------------------
    // Re-wrap an existing Event whose ValueTree is already a known text message type
    TextMessage (const Event& e);

    // Reconstruct from a stored ValueTree
    TextMessage (juce::ValueTree vt);

    // Build a fresh FlexData text message
    TextMessage (const juce::String& typeName, int theGroup, int theChannel,
                 int theAddress, int theStatusBank, int theStatus, const juce::String& theText);

    // Build a fresh stream text message
    TextMessage (const juce::String& typeName, int theFunctionBlockNumber, const juce::String& theText);

    // -------------------------------------------------------------------------
    MAKE_VALUE_MEMBER (juce::String, text,                {});
    MAKE_VALUE_MEMBER (int,          group,               {});
    MAKE_VALUE_MEMBER (int,          channel,             {});
    MAKE_VALUE_MEMBER (int,          address,             {});
    MAKE_VALUE_MEMBER (int,          statusBank,          {});
    MAKE_VALUE_MEMBER (int,          status,              {});
    MAKE_VALUE_MEMBER (int,          functionBlockNumber, {});

    // -------------------------------------------------------------------------
    // Returns true if e's type name is one of the 19 known text message types.
    static bool isTextMessage (const Event& e);

    // Classify the specific text message type.
    static bool isFlexDataTextMessage  (const juce::String& typeName);
    static bool isStreamTextMessage    (const juce::String& typeName);

    // Human-readable label for display in the event view.
    static juce::String displayName (const juce::String& typeName);

    // Map FlexData (statusBank, status) to the corresponding type identifier string.
    static juce::String typeForFlexStatus (int statusBank, int status);

    // Map StreamText status code to the corresponding type identifier string.
    static juce::String typeForStreamStatus (int status);
};
