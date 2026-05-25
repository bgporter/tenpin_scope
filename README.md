# 10-Pin Scope
Debug/monitor tool for MIDI 2.0 data

## About

This is my third (at least) attempt at a MIDI 2.0 debugger/monitor tool.  

The original one (2018/19) focused on generating and parsing UMP messages, and was used for a 'plugfest' at NAMM 2019, and [shown to the public](https://www.youtube.com/watch?v=K2dAIvrI8zg) at the Audio Developer Conference (ADC) in London in 2019.

The second attempt (abandoned) was built around a pure C++ library for working with MIDI 2.0 data (including CI and Property Exchange), but collided with a job change, and the further away from it I got, the less I agreed with some of the design choices. 

Now here in late 2025, JUCE has added support for working with UMP data streams, and there's no rich debug/monitoring tool for MIDI 2.0 data, so I'm building the one that I wish existed. 

