#include "MainComponent.h"
#include "model/persistentContext.h"

//==============================================================================
MainComponent::MainComponent (AppContext& theAppContext)
: appContext (theAppContext)
, persistentContext { theAppContext }
, runtimeContext { theAppContext }
, dataView { theAppContext }
, deviceView { theAppContext }
{
    addAndMakeVisible (dataView);
    addAndMakeVisible (deviceView);
    setSize (600, 400);

    // Seed runtime context with persisted value so initial layout is correct.
    runtimeContext.setattr<int> ("sidebarWidth", persistentContext.sidebarWidth.get ());
    runtimeContext.sidebarWidth.onPropertyChange ([this] (const juce::Identifier&) { resized (); });
}

void MainComponent::paint (juce::Graphics&) {}

void MainComponent::resized ()
{
    auto bounds = getLocalBounds ();
    deviceView.setBounds (bounds.removeFromLeft (runtimeContext.sidebarWidth));
    dataView.setBounds (bounds);
}
