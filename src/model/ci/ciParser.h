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

#pragma once

#include <functional>

#include "model/event.h"
#include "model/sysex/sysex7Message.h"

/**
 * @brief Inspects assembled Sysex7Messages and emits typed CiMessage
 * objects for recognized MIDI-CI message types.
 *
 * Sits after Sysex7Builder in the processing pipeline. The caller keeps
 * the original Sysex7Message in the EventList; CiParser creates an
 * additional CI-typed event for each recognized CI message.
 */
class CiParser
{
public:
    using DeferFn = std::function<void (Event&)>;

    explicit CiParser (DeferFn deferFn);

    /**
     * @brief Try to parse a Sysex7Message as a MIDI-CI message.
     * If the message is a recognized CI type, constructs the appropriate
     * typed struct and calls deferFn with it. Non-CI messages and unknown
     * CI types are silently ignored.
     */
    void parse (const Sysex7Message& msg);

private:
    DeferFn defer;
};
