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

#include "textMessage.h"

#include "model/ump/flexData.h"
#include "model/ump/stream.h"

TextMessage::TextMessage (const Event& e)
: Event (e.getTypeName (), juce::ValueTree { e })
{
}

TextMessage::TextMessage (juce::ValueTree vt)
: Event (vt.getType ().toString (), vt)
{
}

TextMessage::TextMessage (const juce::String& typeName, int theGroup, int theChannel,
                          int theAddress, int theStatusBank, int theStatus,
                          const juce::String& theText)
: Event (typeName)
{
    group      = theGroup;
    channel    = theChannel;
    address    = theAddress;
    statusBank = theStatusBank;
    status     = theStatus;
    text       = theText;
}

TextMessage::TextMessage (const juce::String& typeName, int theFunctionBlockNumber,
                          const juce::String& theText)
: Event (typeName)
{
    functionBlockNumber = theFunctionBlockNumber;
    text                = theText;
}

bool TextMessage::isTextMessage (const Event& e)
{
    const auto name = e.getTypeName ();
    return isFlexDataTextMessage (name) || isStreamTextMessage (name);
}

bool TextMessage::isFlexDataTextMessage (const juce::String& typeName)
{
    return typeName == typeProjectName.toString ()           ||
           typeName == typeCompositionName.toString ()       ||
           typeName == typeMidiClipName.toString ()          ||
           typeName == typeCopyrightNotice.toString ()       ||
           typeName == typeComposerName.toString ()          ||
           typeName == typeLyricistName.toString ()          ||
           typeName == typeArrangerName.toString ()          ||
           typeName == typePublisherName.toString ()         ||
           typeName == typePrimaryPerformer.toString ()      ||
           typeName == typeAccompanyingPerformer.toString () ||
           typeName == typeRecordingDate.toString ()         ||
           typeName == typeRecordingLocation.toString ()     ||
           typeName == typeLyrics.toString ()                ||
           typeName == typeLyricsLanguage.toString ()        ||
           typeName == typeRuby.toString ()                  ||
           typeName == typeRubyLanguage.toString ();
}

bool TextMessage::isStreamTextMessage (const juce::String& typeName)
{
    return typeName == typeEndpointName.toString ()      ||
           typeName == typeProductInstanceId.toString () ||
           typeName == typeFunctionBlockName.toString ();
}

juce::String TextMessage::displayName (const juce::String& typeName)
{
    if (typeName == typeProjectName.toString ())           return "Project Name";
    if (typeName == typeCompositionName.toString ())       return "Composition Name";
    if (typeName == typeMidiClipName.toString ())          return "MIDI Clip Name";
    if (typeName == typeCopyrightNotice.toString ())       return "Copyright Notice";
    if (typeName == typeComposerName.toString ())          return "Composer Name";
    if (typeName == typeLyricistName.toString ())          return "Lyricist Name";
    if (typeName == typeArrangerName.toString ())          return "Arranger Name";
    if (typeName == typePublisherName.toString ())         return "Publisher Name";
    if (typeName == typePrimaryPerformer.toString ())      return "Primary Performer";
    if (typeName == typeAccompanyingPerformer.toString ()) return "Accompanying Performer";
    if (typeName == typeRecordingDate.toString ())         return "Recording Date";
    if (typeName == typeRecordingLocation.toString ())     return "Recording Location";
    if (typeName == typeLyrics.toString ())                return "Lyrics";
    if (typeName == typeLyricsLanguage.toString ())        return "Lyrics Language";
    if (typeName == typeRuby.toString ())                  return "Ruby";
    if (typeName == typeRubyLanguage.toString ())          return "Ruby Language";
    if (typeName == typeEndpointName.toString ())          return "Endpoint Name";
    if (typeName == typeProductInstanceId.toString ())     return "Product Instance ID";
    if (typeName == typeFunctionBlockName.toString ())     return "Function Block Name";
    return typeName;
}

juce::String TextMessage::typeForFlexStatus (int statusBank, int status)
{
    if (statusBank == static_cast<int> (FlexDataStatusBank::metadataText))
    {
        switch (static_cast<MetadataTextStatus> (status))
        {
            case MetadataTextStatus::projectName:           return typeProjectName.toString ();
            case MetadataTextStatus::compositionName:       return typeCompositionName.toString ();
            case MetadataTextStatus::midiClipName:          return typeMidiClipName.toString ();
            case MetadataTextStatus::copyrightNotice:       return typeCopyrightNotice.toString ();
            case MetadataTextStatus::composerName:          return typeComposerName.toString ();
            case MetadataTextStatus::lyricistName:          return typeLyricistName.toString ();
            case MetadataTextStatus::arrangerName:          return typeArrangerName.toString ();
            case MetadataTextStatus::publisherName:         return typePublisherName.toString ();
            case MetadataTextStatus::primaryPerformer:      return typePrimaryPerformer.toString ();
            case MetadataTextStatus::accompanyingPerformer: return typeAccompanyingPerformer.toString ();
            case MetadataTextStatus::recordingDate:         return typeRecordingDate.toString ();
            case MetadataTextStatus::recordingLocation:     return typeRecordingLocation.toString ();
            default:                                        break;
        }
    }
    if (statusBank == static_cast<int> (FlexDataStatusBank::performanceText))
    {
        switch (static_cast<PerformanceTextStatus> (status))
        {
            case PerformanceTextStatus::lyrics:         return typeLyrics.toString ();
            case PerformanceTextStatus::lyricsLanguage: return typeLyricsLanguage.toString ();
            case PerformanceTextStatus::ruby:           return typeRuby.toString ();
            case PerformanceTextStatus::rubyLanguage:   return typeRubyLanguage.toString ();
            default:                                    break;
        }
    }
    return {};
}

juce::String TextMessage::typeForStreamStatus (int status)
{
    switch (status)
    {
        case StreamStatus::endpointNameNotification:      return typeEndpointName.toString ();
        case StreamStatus::productInstanceId:             return typeProductInstanceId.toString ();
        case StreamStatus::functionBlockNameNotification: return typeFunctionBlockName.toString ();
        default:                                          return {};
    }
}

#if RUN_UNIT_TESTS
#include "test/test_TextMessage.inl"
#endif
