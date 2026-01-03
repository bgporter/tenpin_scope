#include "MainComponent.h"
#include "model/persistentContext.h"

//==============================================================================
MainComponent::MainComponent (AppContext& theAppContext)
: appContext (theAppContext)
, persistentContext { theAppContext }
, dataView { theAppContext }
, deviceView { theAppContext }
{
    addAndMakeVisible (dataView);
    addAndMakeVisible (deviceView);
    setSize (600, 400);

    persistentContext.sidebarWidth.onPropertyChange ([this] (const juce::Identifier&) { resized (); });
}

void MainComponent::paint (juce::Graphics& ) {}

void MainComponent::resized ()
{
    auto bounds = getLocalBounds ();
    deviceView.setBounds (bounds.removeFromLeft (persistentContext.sidebarWidth));
    dataView.setBounds (bounds);
}
