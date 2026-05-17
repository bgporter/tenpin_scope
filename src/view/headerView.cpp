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

#include "headerView.h"

#include "palette.h"
#include <ImageData.h>


HeaderView::HeaderView (AppContext& context)
: appContext { context }
, persistentContext { appContext }
, runtimeContext { appContext }
{
    Palette palette { persistentContext };
    const juce::Colour iconColor = palette.defaultText.get ();
    // 0xF3F3F3 is the fill color baked into all SVGs in assets/images
    const auto svgFillColor = juce::Colour (0xFFF3F3F3);

    const auto iconImage = renderSvgWithColor (
        ImageData::filter_list_24dp_F3F3F3_FILL0_wght300_GRAD0_opsz24_svg,
        ImageData::filter_list_24dp_F3F3F3_FILL0_wght300_GRAD0_opsz24_svgSize,
        svgFillColor, iconColor, kHeight);

    filterButton.setImages (
        false, true, true,
        iconImage, 0.7f, juce::Colours::transparentBlack,
        iconImage, 1.0f, juce::Colours::transparentBlack,
        iconImage, 1.0f, iconColor.withAlpha (0.2f));
    filterButton.setClickingTogglesState (true);
    filterButton.setButtonText ("filter");
    filterButton.onClick = [this] ()
    {
        if (onSettingsToggled)
            onSettingsToggled (filterButton.getToggleState ());
    };
    addAndMakeVisible (filterButton);

    const auto clearIconImage = renderSvgWithColor (
        ImageData::delete_sweep_24dp_F3F3F3_FILL0_wght300_GRAD0_opsz24_svg,
        ImageData::delete_sweep_24dp_F3F3F3_FILL0_wght300_GRAD0_opsz24_svgSize,
        svgFillColor, iconColor, kHeight);

    clearButton.setImages (
        false, true, true,
        clearIconImage, 0.7f, juce::Colours::transparentBlack,
        clearIconImage, 1.0f, juce::Colours::transparentBlack,
        clearIconImage, 1.0f, iconColor.withAlpha (0.2f));
    clearButton.setButtonText ("clear");
    clearButton.onClick = [this] () { ++runtimeContext.clearEvents; };
    addAndMakeVisible (clearButton);

    const juce::Image pauseImg = renderSvgWithColor (
        ImageData::pause_circle_24dp_F3F3F3_FILL0_wght400_GRAD0_opsz24_svg,
        ImageData::pause_circle_24dp_F3F3F3_FILL0_wght400_GRAD0_opsz24_svgSize,
        svgFillColor, iconColor, kHeight);
    const juce::Image playImg = renderSvgWithColor (
        ImageData::play_circle_24dp_F3F3F3_FILL0_wght400_GRAD0_opsz24_svg,
        ImageData::play_circle_24dp_F3F3F3_FILL0_wght400_GRAD0_opsz24_svgSize,
        svgFillColor, iconColor, kHeight);

    auto setPauseButtonImage = [this, pauseImg, playImg, iconColor] ()
    {
        const auto& img = pauseButton.getToggleState () ? playImg : pauseImg;
        pauseButton.setImages (
            false, true, true,
            img, 0.7f, juce::Colours::transparentBlack,
            img, 1.0f, juce::Colours::transparentBlack,
            img, 1.0f, iconColor.withAlpha (0.2f));
    };
    setPauseButtonImage ();
    pauseButton.setClickingTogglesState (true);
    pauseButton.setButtonText ("pause");
    pauseButton.onClick = [this, setPauseButtonImage] ()
    {
        runtimeContext.midiProperties.pause = pauseButton.getToggleState ();
        setPauseButtonImage ();
    };
    addAndMakeVisible (pauseButton);
}

void HeaderView::dismissSettings ()
{
    if (filterButton.getToggleState ())
        filterButton.setToggleState (false, juce::sendNotification);
}

void HeaderView::paint (juce::Graphics& g)
{
    Palette palette { PersistentContext { appContext } };
    g.fillAll (palette.windowBackground.get ());

    g.setColour (palette.defaultText.get ());
    g.drawHorizontalLine (getHeight () - 1, 0.0f, static_cast<float> (getWidth ()));
}

void HeaderView::sidebarWidthChanged ()
{
    resized ();
}

void HeaderView::resized ()
{
    const int buttonSize = kHeight - 8;
    const int yOffset    = (getHeight () - buttonSize) / 2;
    const int sidebarW   = runtimeContext.sidebarWidth.get ();

    clearButton.setBounds  (4, yOffset, buttonSize, buttonSize);
    filterButton.setBounds (getWidth () - buttonSize - 4, yOffset, buttonSize, buttonSize);

    pauseButton.setBounds (sidebarW - buttonSize - 4, yOffset, buttonSize, buttonSize);
}
