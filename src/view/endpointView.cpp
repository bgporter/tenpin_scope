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

EndpointView::EndpointView (AppContext& theAppContext, juce::ValueTree tree)
: appContext { theAppContext }
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
    endpointProperties.isInputAlive.onPropertyChange ([this] (const juce::Identifier&) { repaint (); });
    endpointProperties.isOutputAlive.onPropertyChange ([this] (const juce::Identifier&) { repaint (); });
}

void EndpointView::paint (juce::Graphics& g)
{
    juce::Colour backgroundColor = juce::Colours::darkgrey;
    if (endpointProperties.isAlive)
    {
        backgroundColor = endpointProperties.isInputAlive.get () ? juce::Colours::green : juce::Colours::red;
    }
    g.fillAll (backgroundColor);

    if (auto* laf = TenpinLookAndFeel::getFrom (*this))
        g.setColour (laf->getPalette ().divider.get ());
    const auto bounds = getLocalBounds ();
    const auto y      = static_cast<float> (bounds.getBottom ()) - 1.0f;
    g.drawLine (0.0f, y, static_cast<float> (bounds.getWidth ()), y, 2.0f);
}

void EndpointView::resized ()
{
    auto bounds       = getLocalBounds ();
    const auto topRow = bounds.removeFromTop (bounds.getHeight () / 2);
    nameLabel.setBounds (topRow);

    auto bottomRow    = bounds;
    const auto width  = bottomRow.getWidth ();
    const auto labelW = width / 4;

    rxTitleLabel.setBounds (bottomRow.removeFromLeft (labelW));
    rxValueLabel.setBounds (bottomRow.removeFromLeft (labelW));
    txTitleLabel.setBounds (bottomRow.removeFromLeft (labelW));
    txValueLabel.setBounds (bottomRow.removeFromLeft (labelW));
}
