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

#include "msgHandler.h"

#include "model/sysex/mdsMessage.h"
#include "model/sysex/sysex7Message.h"
#include "model/sysex/sysex8Message.h"
#include "model/sysex/textMessage.h"

MessageHandler::MessageHandler () {}

MessageHandler::~MessageHandler () {}

Handler::Result MessageHandler::handle (const Event& e)
{
    Handler::Result result { preDispatch (e) };
    if (result == Handler::Result::ok)
    {
        const auto type = e.getType ();
        if (type == Sysex7Message::type)
            result = onSysex7Message (e);
        else if (type == Sysex8Message::type)
            result = onSysex8Message (e);
        else if (type == MdsMessage::type)
            result = onMdsMessage (e);
        else if (TextMessage::isTextMessage (e))
        {
            if (TextMessage::isFlexDataTextMessage (type))
                result = onFlexDataTextMessage (e);
            else
                result = onStreamTextMessage (e);

            if (result == Handler::Result::notHandled)
                result = onTextMessage (e);
            if (result == Handler::Result::notHandled)
                result = onMessage (e);
        }
        else
            result = onMessage (e);
    }
    return postDispatch (e, result);
}

Handler::Result MessageHandler::handle (const Event& e, void* /*ctx*/)
{
    return handle (e);
}
