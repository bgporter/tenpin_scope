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
    // Seed runtime context and register listener before setSize so the first
    // resized() call uses the persisted sidebar width, not the default.
    runtimeContext.sidebarWidth = persistentContext.sidebarWidth.get ();
    runtimeContext.sidebarWidth.onPropertyChange ([this] (const juce::Identifier&) { resized (); });

    addAndMakeVisible (dataView);
    addAndMakeVisible (deviceView);
    setSize (600, 400);
}

void MainComponent::paint (juce::Graphics&) {}

void MainComponent::resized ()
{
    auto bounds = getLocalBounds ();
    deviceView.setBounds (bounds.removeFromLeft (runtimeContext.sidebarWidth));
    dataView.setBounds (bounds);
}
