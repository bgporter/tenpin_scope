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

/**
 * @brief Base class for handlers that process MIDI-CI assembled messages.
 * Mirrors MessageHandler in structure: dispatches by type identifier,
 * with a catch-all fallback and pre/post hooks.
 */
class CiHandler
{
public:
    CiHandler ();
    virtual ~CiHandler ();

    Handler::Result handle (const Event& e);
    virtual Handler::Result handle (const Event& e, void* ctx);

protected:
    Handler::Result defaultResult { Handler::Result::notHandled };

private:
    virtual Handler::Result preDispatch  (const Event& e) { return Handler::Result::ok; }
    virtual Handler::Result postDispatch (const Event& e, Handler::Result pendingResult) { return pendingResult; }

    virtual Handler::Result onCiPeCapabilitiesInquiry      (const Event& e) { return defaultResult; }
    virtual Handler::Result onCiPeCapabilitiesReply        (const Event& e) { return defaultResult; }
    virtual Handler::Result onCiPeGetPropertyDataInquiry   (const Event& e) { return defaultResult; }
    virtual Handler::Result onCiPeGetPropertyDataReply     (const Event& e) { return defaultResult; }
    virtual Handler::Result onCiPeSetPropertyDataInquiry   (const Event& e) { return defaultResult; }
    virtual Handler::Result onCiPeSetPropertyDataReply     (const Event& e) { return defaultResult; }
    virtual Handler::Result onCiPeSubscriptionInquiry      (const Event& e) { return defaultResult; }
    virtual Handler::Result onCiPeSubscriptionReply        (const Event& e) { return defaultResult; }
    virtual Handler::Result onCiPeNotify                   (const Event& e) { return defaultResult; }
    virtual Handler::Result onCiProfileInquiry        (const Event& e) { return defaultResult; }
    virtual Handler::Result onCiProfileInquiryReply (const Event& e) { return defaultResult; }
    virtual Handler::Result onCiProfileSetOn         (const Event& e) { return defaultResult; }
    virtual Handler::Result onCiProfileSetOff        (const Event& e) { return defaultResult; }
    virtual Handler::Result onCiProfileEnabled       (const Event& e) { return defaultResult; }
    virtual Handler::Result onCiProfileDisabled      (const Event& e) { return defaultResult; }
    virtual Handler::Result onCiProfileAdded         (const Event& e) { return defaultResult; }
    virtual Handler::Result onCiProfileRemoved             (const Event& e) { return defaultResult; }
    virtual Handler::Result onCiProfileDetailsInquiry      (const Event& e) { return defaultResult; }
    virtual Handler::Result onCiProfileDetailsInquiryReply (const Event& e) { return defaultResult; }
    virtual Handler::Result onCiProfileSpecificData        (const Event& e) { return defaultResult; }
    virtual Handler::Result onCiDiscoveryInquiry    (const Event& e) { return defaultResult; }
    virtual Handler::Result onCiDiscoveryReply      (const Event& e) { return defaultResult; }
    virtual Handler::Result onCiEndpointInquiry   (const Event& e) { return defaultResult; }
    virtual Handler::Result onCiEndpointReply     (const Event& e) { return defaultResult; }
    virtual Handler::Result onCiInvalidateMuid    (const Event& e) { return defaultResult; }
    virtual Handler::Result onCiNak              (const Event& e) { return defaultResult; }
    virtual Handler::Result onCiAck              (const Event& e) { return defaultResult; }
    virtual Handler::Result onCiMessage          (const Event& e) { return defaultResult; }
};
