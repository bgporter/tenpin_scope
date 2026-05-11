#include "MainComponent.h"
#include "model/persistentContext.h"

static constexpr int kSettingsAnimId { 1 };

//==============================================================================
MainComponent::MainComponent (AppContext& theAppContext)
: appContext (theAppContext)
, persistentContext { theAppContext }
, runtimeContext { theAppContext }
, animator { std::make_unique<friz::DisplaySyncController> (this) }
, headerView { theAppContext }
, dataView { theAppContext }
, deviceView { theAppContext }
{
    // Seed runtime context and register listener before setSize so the first
    // resized() call uses the persisted sidebar width, not the default.
    runtimeContext.sidebarWidth = persistentContext.sidebarWidth.get ();
    runtimeContext.sidebarWidth.onPropertyChange ([this] (const juce::Identifier&) { resized (); });

    headerView.onSettingsToggled = [this] (bool show)
    {
        const float startVal = runtimeContext.settingsPos;
        const float endVal   = show ? 1.f : 0.f;

        animator.cancelAnimation (kSettingsAnimId, false);

        auto anim = friz::makeAnimation<friz::Parametric> (
            kSettingsAnimId, startVal, endVal, 250,
            friz::Parametric::CurveType::kEaseInOutCubic);

        anim->onUpdate ([this] (int, const std::array<float, 1>& vals)
        {
            runtimeContext.settingsPos = vals[0];
        });

        animator.addAnimation (std::move (anim));
    };

    dataView.onDismissSettings = [this] () { headerView.dismissSettings (); };

    addAndMakeVisible (headerView);
    addAndMakeVisible (dataView);
    addAndMakeVisible (deviceView);
    setSize (600, 400);
}

void MainComponent::paint (juce::Graphics&) {}

void MainComponent::resized ()
{
    auto bounds = getLocalBounds ();
    headerView.setBounds (bounds.removeFromTop (HeaderView::kHeight));
    deviceView.setBounds (bounds.removeFromLeft (runtimeContext.sidebarWidth));
    dataView.setBounds (bounds);
}
