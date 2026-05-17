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

#include "endpointView.h"
#include "lookAndFeel.h"
#include <ImageData.h>

EndpointView::EndpointView (AppContext& theAppContext, juce::ValueTree tree)
: appContext { theAppContext }
, runtimeContext { appContext }
, endpointProperties { tree }
{
    jassert (endpointProperties.wasWrapped ());

    nameLabel.setJustificationType (juce::Justification::centredLeft);
    rxTitleLabel.setJustificationType (juce::Justification::centredLeft);
    rxValueLabel.setJustificationType (juce::Justification::centredLeft);
    txTitleLabel.setJustificationType (juce::Justification::centredLeft);
    txValueLabel.setJustificationType (juce::Justification::centredLeft);

    if (auto* laf = TenpinLookAndFeel::getFrom (*this))
    {
        nameLabel.setFont (laf->getTenpinLabelFont ());
        rxTitleLabel.setFont (laf->getTenpinLabelFont ());
        txTitleLabel.setFont (laf->getTenpinLabelFont ());
        rxValueLabel.setFont (laf->getTenpinValueFont ());
        txValueLabel.setFont (laf->getTenpinValueFont ());
    }
    addAndMakeVisible (nameLabel);
    addAndMakeVisible (rxTitleLabel);
    addAndMakeVisible (rxValueLabel);
    addAndMakeVisible (txTitleLabel);
    addAndMakeVisible (txValueLabel);

    rxTitleLabel.setText ("Rx:", juce::dontSendNotification);
    txTitleLabel.setText ("Tx:", juce::dontSendNotification);

    nameLabel.setText (endpointProperties.name.get (), juce::dontSendNotification);
    endpointProperties.name.onPropertyChange (
        [this] (const juce::Identifier&)
        {
            nameLabel.setText (endpointProperties.name.get (), juce::dontSendNotification);
            repaint ();
        });

    auto updateRx = [this] ()
    { rxValueLabel.setText (juce::String (endpointProperties.received.count.get ()), juce::dontSendNotification); };

    auto updateTx = [this] ()
    { txValueLabel.setText (juce::String (endpointProperties.transmitted.count.get ()), juce::dontSendNotification); };

    // execute the callbacks to set the initial values.
    updateRx ();
    updateTx ();

    endpointProperties.received.count.onPropertyChange ([updateRx] (const juce::Identifier&) { updateRx (); });
    endpointProperties.transmitted.count.onPropertyChange ([updateTx] (const juce::Identifier&) { updateTx (); });
    endpointProperties.isInputAlive.onPropertyChange ([this] (const juce::Identifier&) { updateColors (); });
    endpointProperties.isOutputAlive.onPropertyChange ([this] (const juce::Identifier&) { updateColors (); });

    if (endpointProperties.isSynthetic.get ())
    {
        playButton.onClick = [this] { endpointProperties.playRequested = true; };
        addAndMakeVisible (playButton);
    }

    pauseButton.setClickingTogglesState (true);
    pauseButton.setButtonText ("pause");
    pauseButton.onClick = [this] ()
    {
        endpointProperties.pause = pauseButton.getToggleState ();
        applyPauseButtonImage ();
        updateColors ();
    };
    addAndMakeVisible (pauseButton);

    updateColors ();

    auto updatePauseEnabled = [this] ()
    {
        pauseButton.setEnabled (!runtimeContext.midiProperties.pause.get ());
    };
    updatePauseEnabled ();

    runtimeContext.midiProperties.pause.onPropertyChange (
        [updatePauseEnabled, this] (const juce::Identifier&)
        {
            updatePauseEnabled ();
            updateColors ();
        });
}

void EndpointView::updateColors ()
{
    const bool rxAlive = endpointProperties.isInputAlive.get ();
    const bool txAlive = endpointProperties.isOutputAlive.get ();
    const bool alive   = rxAlive || txAlive;

    seenRx |= rxAlive;
    seenTx |= txAlive;

    auto* laf = TenpinLookAndFeel::getFrom (*this);
    if (laf == nullptr)
        return;

    const auto& pal = laf->getPalette ();

    const bool effectiveRx = rxAlive || (!alive && seenRx);
    const bool effectiveTx = txAlive || (!alive && seenTx);

    juce::Colour bg, data;
    if (effectiveRx && effectiveTx)
    {
        bg   = pal.endpointBidirBackground.get ();
        data = pal.endpointBidirData.get ();
    }
    else if (effectiveRx)
    {
        bg   = pal.endpointRxBackground.get ();
        data = pal.endpointRxData.get ();
    }
    else if (effectiveTx)
    {
        bg   = pal.endpointTxBackground.get ();
        data = pal.endpointTxData.get ();
    }
    else
    {
        bg   = pal.surfaceHigh.get ();
        data = pal.defaultText.get ();
    }

    if (!alive)
    {
        bg   = bg.withAlpha (0.4f);
        data = data.withAlpha (0.5f);
    }

    bgColor = bg;
    nameLabel.setColour (juce::Label::textColourId, data);
    rxTitleLabel.setColour (juce::Label::textColourId, data);
    rxValueLabel.setColour (juce::Label::textColourId, data);
    txTitleLabel.setColour (juce::Label::textColourId, data);
    txValueLabel.setColour (juce::Label::textColourId, data);

    const auto svgFillColor = juce::Colour (0xFFF3F3F3);
    pauseImg = renderSvgWithColor (
        ImageData::pause_circle_24dp_F3F3F3_FILL0_wght400_GRAD0_opsz24_svg,
        ImageData::pause_circle_24dp_F3F3F3_FILL0_wght400_GRAD0_opsz24_svgSize,
        svgFillColor, data, 24);
    playImg = renderSvgWithColor (
        ImageData::play_circle_24dp_F3F3F3_FILL0_wght400_GRAD0_opsz24_svg,
        ImageData::play_circle_24dp_F3F3F3_FILL0_wght400_GRAD0_opsz24_svgSize,
        svgFillColor, data, 24);
    applyPauseButtonImage ();

    const bool paused = runtimeContext.midiProperties.pause.get () || endpointProperties.pause.get ();
    setAlpha (paused ? 0.5f : 1.0f);

    repaint ();
}

void EndpointView::applyPauseButtonImage ()
{
    const auto& img = pauseButton.getToggleState () ? playImg : pauseImg;
    pauseButton.setImages (
        false, true, true,
        img, 0.7f, juce::Colours::transparentBlack,
        img, 1.0f, juce::Colours::transparentBlack,
        img, 1.0f, juce::Colours::transparentBlack);
}

void EndpointView::paint (juce::Graphics& g)
{
    g.fillAll (bgColor);

    if (auto* laf = TenpinLookAndFeel::getFrom (*this))
        g.setColour (laf->getPalette ().divider.get ());
    const auto bounds = getLocalBounds ();
    const auto y      = static_cast<float> (bounds.getBottom ()) - 1.0f;
    g.drawLine (0.0f, y, static_cast<float> (bounds.getWidth ()), y, 2.0f);
}

void EndpointView::resized ()
{
    auto bounds  = getLocalBounds ();
    auto topRow  = bounds.removeFromTop (bounds.getHeight () / 2);

    const int buttonSize = topRow.getHeight () - 8;
    const int slotW      = buttonSize + 6;

    if (endpointProperties.isSynthetic.get ())
    {
        auto slot = topRow.removeFromRight (slotW);
        playButton.setBounds (slot.withSizeKeepingCentre (buttonSize, buttonSize));
    }
    {
        auto slot = topRow.removeFromRight (slotW);
        pauseButton.setBounds (slot.withSizeKeepingCentre (buttonSize, buttonSize));
    }
    nameLabel.setBounds (topRow);

    auto bottomRow    = bounds;
    const auto width  = bottomRow.getWidth ();
    const auto labelW = width / 4;

    rxTitleLabel.setBounds (bottomRow.removeFromLeft (labelW));
    rxValueLabel.setBounds (bottomRow.removeFromLeft (labelW));
    txTitleLabel.setBounds (bottomRow.removeFromLeft (labelW));
    txValueLabel.setBounds (bottomRow.removeFromLeft (labelW));
}
