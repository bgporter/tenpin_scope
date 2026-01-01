
cmake_minimum_required(VERSION 3.22)
add_subdirectory(JUCE)           # If you've put JUCE in a subdirectory called JUCE
juce_add_module(cello ALIAS_NAMESPACE cello)           # add the cello module
juce_add_module(testSuite ALIAS_NAMESPACE testSuite)   # add testSuite
juce_add_module(animator/Source/friz ALIAS_NAMESPACE friz)