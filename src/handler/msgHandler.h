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

#include "handler/handlerResult.h"
#include "model/event.h"

class MessageHandler
{
public:
    MessageHandler ();
    virtual ~MessageHandler ();

    Handler::Result handle (const Event& e);
    virtual Handler::Result handle (const Event& e, void* ctx);

protected:
    Handler::Result defaultResult { Handler::Result::notHandled };

private:
    virtual Handler::Result preDispatch (const Event& e) { return Handler::Result::ok; }
    virtual Handler::Result postDispatch (const Event& e, Handler::Result pendingResult) { return pendingResult; }

    virtual Handler::Result onSysex7Message        (const Event& e) { return defaultResult; }
    virtual Handler::Result onSysex8Message        (const Event& e) { return defaultResult; }
    virtual Handler::Result onMdsMessage           (const Event& e) { return defaultResult; }
    virtual Handler::Result onFlexDataTextMessage  (const Event& e) { return defaultResult; }
    virtual Handler::Result onStreamTextMessage    (const Event& e) { return defaultResult; }
    virtual Handler::Result onTextMessage          (const Event& e) { return defaultResult; }
    virtual Handler::Result onMessage              (const Event& e) { return defaultResult; }
};
