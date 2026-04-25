#pragma once

// CMake builds don't use an AppConfig.h, so it's safe to include juce module headers
// directly. If you need to remain compatible with Projucer-generated builds, and
// have called `juce_generate_juce_header(<thisTarget>)` in your CMakeLists.txt,
// you could `#include <JuceHeader.h>` here instead, to make all your module headers visible.
#include <juce_gui_extra/juce_gui_extra.h>

#include "model/appContext.h"
#include "model/persistentContext.h"
#include "model/runtimeContext.h"
#include "view/dataView.h"
#include "view/deviceView.h"
#include "view/headerView.h"

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainComponent final : public juce::Component
{
public:
    //==============================================================================
    MainComponent (AppContext& theAppContext);

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized () override;

private:
    //==============================================================================
    // Your private member variables go here...
    AppContext appContext;
    PersistentContext persistentContext;
    RuntimeContext runtimeContext;
    friz::Animator animator;
    HeaderView headerView;
    DataView dataView;
    DeviceView deviceView;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
