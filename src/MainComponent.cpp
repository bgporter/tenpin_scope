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
, dataViewHeader { theAppContext }
, dataView { theAppContext }
, deviceView { theAppContext }
, settingsView { theAppContext }
, sidebarResizer { runtimeContext.sidebarWidth, persistentContext.sidebarWidth, persistentContext.dragging,
                   ResizeHandle::LinePosition::right }
, col1Resizer { runtimeContext.col1Width, persistentContext.eventViewContext.col1Width, persistentContext.dragging }
, col2Resizer { runtimeContext.col2Width, persistentContext.eventViewContext.col2Width, persistentContext.dragging }
, col3Resizer { runtimeContext.col3Width, persistentContext.eventViewContext.col3Width, persistentContext.dragging }
{
    // Seed runtime context and register listener before setSize so the first
    // resized() call uses the persisted sidebar width, not the default.
    runtimeContext.sidebarWidth = persistentContext.sidebarWidth.get ();
    runtimeContext.sidebarWidth.onPropertyChange ([this] (const juce::Identifier&)
    {
        resized ();
        headerView.sidebarWidthChanged ();
    });

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

    settingsViewport.setViewedComponent (&settingsView, false);
    settingsViewport.setScrollBarsShown (true, false);
    runtimeContext.settingsPos.onPropertyChange ([this] (const juce::Identifier&) { resized (); });

    addAndMakeVisible (headerView);
    addAndMakeVisible (dataViewHeader);
    addAndMakeVisible (deviceView);
    addAndMakeVisible (dataView);
    // Resizers added last so they sit above all content including the header row.
    sidebarResizer.setHeaderHeight (DataViewHeader::kHeaderHeight);
    col1Resizer.setHeaderHeight (DataViewHeader::kHeaderHeight);
    col2Resizer.setHeaderHeight (DataViewHeader::kHeaderHeight);
    col3Resizer.setHeaderHeight (DataViewHeader::kHeaderHeight);
    addAndMakeVisible (sidebarResizer);
    addAndMakeVisible (col1Resizer);
    addAndMakeVisible (col2Resizer);
    addAndMakeVisible (col3Resizer);
    addAndMakeVisible (settingsViewport);

    runtimeContext.col1Width.onPropertyChange ([this] (const juce::Identifier&) { resized (); });
    runtimeContext.col2Width.onPropertyChange ([this] (const juce::Identifier&) { resized (); });
    runtimeContext.col3Width.onPropertyChange ([this] (const juce::Identifier&) { resized (); });
    persistentContext.eventViewContext.textHeight.onPropertyChange (
        [this] (const juce::Identifier&) { sendLookAndFeelChange (); });
    setSize (600, 400);
}

void MainComponent::paint (juce::Graphics&) {}

void MainComponent::resized ()
{
    auto bounds = getLocalBounds ();
    headerView.setBounds (bounds.removeFromTop (HeaderView::kHeight));
    dataViewHeader.setBounds (bounds.removeFromTop (DataViewHeader::kHeaderHeight));
    deviceView.setBounds (bounds.removeFromLeft (runtimeContext.sidebarWidth));
    dataView.setBounds (bounds);

    // Position resizers spanning from the top of the DataViewHeader to the
    // bottom of the window, centred on each column boundary.
    constexpr int kDividerWidth = 5;
    const int sidebarW          = runtimeContext.sidebarWidth;
    const int col1W             = runtimeContext.col1Width;
    const int col2W             = runtimeContext.col2Width;
    const int col3W             = runtimeContext.col3Width;
    const int half              = kDividerWidth / 2;
    const int handleY           = HeaderView::kHeight;
    const int handleH           = getHeight () - handleY;
    sidebarResizer.setBounds (sidebarW - kDividerWidth, handleY, kDividerWidth, handleH);

    // Settings panel: centered over the DataView area, slides in from above.
    const float settingsPos  = runtimeContext.settingsPos;
    const int dataViewY      = HeaderView::kHeight + DataViewHeader::kHeaderHeight;
    const int dataViewW      = getWidth () - sidebarW;
    const int dataViewH      = getHeight () - dataViewY;
    const int settingsW      = settingsView.getWidth ();
    const int settingsH      = juce::jmin (settingsView.getHeight (), dataViewH);
    const int settingsX      = sidebarW + (dataViewW - settingsW) / 2;
    const int settingsY      = juce::roundToInt (settingsPos * (float) (settingsH + dataViewY)) - settingsH;
    settingsViewport.setBounds (settingsX, settingsY, settingsW, settingsH);
    col1Resizer.setBounds (sidebarW + col1W - half,                                         handleY, kDividerWidth, handleH);
    col2Resizer.setBounds (sidebarW + col1W + kDividerWidth + col2W - half,                 handleY, kDividerWidth, handleH);
    col3Resizer.setBounds (sidebarW + col1W + kDividerWidth + col2W + kDividerWidth + col3W - half, handleY, kDividerWidth, handleH);
}
