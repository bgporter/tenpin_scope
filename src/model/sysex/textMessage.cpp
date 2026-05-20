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
: MessageBase (e.getTypeName (), juce::ValueTree { e })
{
}

TextMessage::TextMessage (juce::ValueTree vt)
: MessageBase (vt.getType ().toString (), vt)
{
}

TextMessage::TextMessage (const juce::String& typeName, int theGroup, int theChannel,
                          int theAddress, int theStatusBank, int theStatus,
                          const juce::String& theText)
: MessageBase (typeName)
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
: MessageBase (typeName)
{
    functionBlockNumber = theFunctionBlockNumber;
    text                = theText;
}

bool TextMessage::isTextMessage (const Event& e)
{
    const auto type = e.getType ();
    return isFlexDataTextMessage (type) || isStreamTextMessage (type);
}

bool TextMessage::isFlexDataTextMessage (const juce::Identifier& type)
{
    return type == typeProjectName           ||
           type == typeCompositionName       ||
           type == typeMidiClipName          ||
           type == typeCopyrightNotice       ||
           type == typeComposerName          ||
           type == typeLyricistName          ||
           type == typeArrangerName          ||
           type == typePublisherName         ||
           type == typePrimaryPerformer      ||
           type == typeAccompanyingPerformer ||
           type == typeRecordingDate         ||
           type == typeRecordingLocation     ||
           type == typeLyrics               ||
           type == typeLyricsLanguage        ||
           type == typeRuby                  ||
           type == typeRubyLanguage;
}

bool TextMessage::isStreamTextMessage (const juce::Identifier& type)
{
    return type == typeEndpointName      ||
           type == typeProductInstanceId ||
           type == typeFunctionBlockName;
}

juce::String TextMessage::displayName (const juce::Identifier& type)
{
    if (type == typeProjectName)           return "Project Name";
    if (type == typeCompositionName)       return "Composition Name";
    if (type == typeMidiClipName)          return "MIDI Clip Name";
    if (type == typeCopyrightNotice)       return "Copyright Notice";
    if (type == typeComposerName)          return "Composer Name";
    if (type == typeLyricistName)          return "Lyricist Name";
    if (type == typeArrangerName)          return "Arranger Name";
    if (type == typePublisherName)         return "Publisher Name";
    if (type == typePrimaryPerformer)      return "Primary Performer";
    if (type == typeAccompanyingPerformer) return "Accompanying Performer";
    if (type == typeRecordingDate)         return "Recording Date";
    if (type == typeRecordingLocation)     return "Recording Location";
    if (type == typeLyrics)               return "Lyrics";
    if (type == typeLyricsLanguage)        return "Lyrics Language";
    if (type == typeRuby)                  return "Ruby";
    if (type == typeRubyLanguage)          return "Ruby Language";
    if (type == typeEndpointName)          return "Endpoint Name";
    if (type == typeProductInstanceId)     return "Product Instance ID";
    if (type == typeFunctionBlockName)     return "Function Block Name";
    return type.toString ();
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
