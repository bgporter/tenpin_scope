

#include <juce_core/juce_core.h>

#include "model/appContext.h"
#include "model/ump/channelVoice2.h"

class Test_EventFilter : public TestSuite
{
public:
    Test_EventFilter ()
    : TestSuite ("EventFilter", "!!! category !!!")
    {
    }

    void runTest () override
    {
        setup (
            [this] ()
            {
                context = std::make_unique<AppContext> (juce::File ());
                filter  = std::make_unique<EventFilter> (*context);
                PersistentContext pc { *context };
                evc = std::make_unique<EventViewContext> (pc);
            });

        tearDown (
            [this] ()
            {
                evc     = nullptr;
                filter  = nullptr;
                context = nullptr;
            });

        test ("note on/off - passes by default",
              [&] ()
              {
                  expect (filter->filterMidiEvent (Midi2NoteOnEvent (1, 1, 60, uint16_t (32768))));
                  expect (filter->filterMidiEvent (Midi2NoteOffEvent (1, 1, 60, uint16_t (32768))));
              });

        test ("note on/off - showNoteOnOff blocks notes",
              [&] ()
              {
                  evc->showNoteOnOff = false;
                  expect (!filter->filterMidiEvent (Midi2NoteOnEvent (1, 1, 60, uint16_t (32768))));
                  expect (!filter->filterMidiEvent (Midi2NoteOffEvent (1, 1, 60, uint16_t (32768))));
              });

        test ("showChannelVoice blocks all channel voice events",
              [&] ()
              {
                  evc->showChannelVoice = false;
                  expect (!filter->filterMidiEvent (Midi2NoteOnEvent (1, 1, 60, uint16_t (32768))));
                  expect (!filter->filterMidiEvent (Midi2NoteOffEvent (1, 1, 60, uint16_t (32768))));
                  expect (!filter->filterMidiEvent (Midi2PolyPressureEvent (1, 1, 60, uint32_t (0))));
                  expect (!filter->filterMidiEvent (Midi2ControlChangeEvent (1, 1, 7, uint32_t (0))));
                  expect (!filter->filterMidiEvent (Midi2ProgramChangeEvent (1, 1, 0)));
                  expect (!filter->filterMidiEvent (Midi2ChannelPressureEvent (1, 1, uint32_t (0))));
                  expect (!filter->filterMidiEvent (Midi2PitchBendEvent (1, 1, int32_t (0))));
              });

        test ("per-note events - passes by default",
              [&] ()
              {
                  expect (filter->filterMidiEvent (Midi2PolyPressureEvent (1, 1, 60, uint32_t (0))));
                  expect (filter->filterMidiEvent (Midi2PerNotePitchBendEvent (1, 1, 60, int32_t (0))));
                  expect (filter->filterMidiEvent (Midi2PerNoteManagementEvent (1, 1, 60, true, false)));
                  expect (filter->filterMidiEvent (Midi2RegisteredPerNoteControllerEvent (1, 1, 60, 0, uint32_t (0))));
                  expect (filter->filterMidiEvent (Midi2AssignablePerNoteControllerEvent (1, 1, 60, 0, uint32_t (0))));
              });

        test ("per-note events - showPerNoteEvents blocks all per-note types",
              [&] ()
              {
                  evc->showPerNoteEvents = false;
                  expect (!filter->filterMidiEvent (Midi2PolyPressureEvent (1, 1, 60, uint32_t (0))));
                  expect (!filter->filterMidiEvent (Midi2PerNotePitchBendEvent (1, 1, 60, int32_t (0))));
                  expect (!filter->filterMidiEvent (Midi2PerNoteManagementEvent (1, 1, 60, true, false)));
                  expect (!filter->filterMidiEvent (Midi2RegisteredPerNoteControllerEvent (1, 1, 60, 0, uint32_t (0))));
                  expect (!filter->filterMidiEvent (Midi2AssignablePerNoteControllerEvent (1, 1, 60, 0, uint32_t (0))));
              });

        test ("control change - passes by default",
              [&] ()
              {
                  expect (filter->filterMidiEvent (Midi2ControlChangeEvent (1, 1, 7, uint32_t (0))));
                  expect (filter->filterMidiEvent (Midi2RegisteredControllerEvent (1, 1, 0, 7, uint32_t (0))));
                  expect (filter->filterMidiEvent (Midi2AssignableControllerEvent (1, 1, 0, 7, uint32_t (0))));
                  expect (filter->filterMidiEvent (Midi2RelativeRegisteredControllerEvent (1, 1, 0, 7, int32_t (0))));
                  expect (filter->filterMidiEvent (Midi2RelativeAssignableControllerEvent (1, 1, 0, 7, int32_t (0))));
              });

        test ("control change - showControlChange blocks all controller types",
              [&] ()
              {
                  evc->showControlChange = false;
                  expect (!filter->filterMidiEvent (Midi2ControlChangeEvent (1, 1, 7, uint32_t (0))));
                  expect (!filter->filterMidiEvent (Midi2RegisteredControllerEvent (1, 1, 0, 7, uint32_t (0))));
                  expect (!filter->filterMidiEvent (Midi2AssignableControllerEvent (1, 1, 0, 7, uint32_t (0))));
                  expect (!filter->filterMidiEvent (Midi2RelativeRegisteredControllerEvent (1, 1, 0, 7, int32_t (0))));
                  expect (!filter->filterMidiEvent (Midi2RelativeAssignableControllerEvent (1, 1, 0, 7, int32_t (0))));
              });

        test ("program change - passes by default",
              [&] () { expect (filter->filterMidiEvent (Midi2ProgramChangeEvent (1, 1, 0))); });

        test ("program change - showProgramChange blocks program change",
              [&] ()
              {
                  evc->showProgramChange = false;
                  expect (!filter->filterMidiEvent (Midi2ProgramChangeEvent (1, 1, 0)));
              });

        test ("channel pressure - passes by default",
              [&] () { expect (filter->filterMidiEvent (Midi2ChannelPressureEvent (1, 1, uint32_t (0)))); });

        test ("channel pressure - showChannelPressure blocks channel pressure",
              [&] ()
              {
                  evc->showChannelPressure = false;
                  expect (!filter->filterMidiEvent (Midi2ChannelPressureEvent (1, 1, uint32_t (0))));
              });

        test ("pitch bend - passes by default",
              [&] () { expect (filter->filterMidiEvent (Midi2PitchBendEvent (1, 1, int32_t (0)))); });

        test ("pitch bend - showPitchBend blocks pitch bend",
              [&] ()
              {
                  evc->showPitchBend = false;
                  expect (!filter->filterMidiEvent (Midi2PitchBendEvent (1, 1, int32_t (0))));
              });
    }

private:
    std::unique_ptr<AppContext> context;
    std::unique_ptr<EventFilter> filter;
    std::unique_ptr<EventViewContext> evc;
};

static Test_EventFilter testEventFilter;
