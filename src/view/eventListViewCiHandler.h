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

#include "handler/ciHandler.h"
#include "model/appContext.h"
#include "model/persistentContext.h"

class EventView;

class EventListViewCiHandler : public CiHandler
{
public:
    EventListViewCiHandler (AppContext& theAppContext);
    ~EventListViewCiHandler () override;

    Handler::Result handle (const Event& e, void* ctx) override;

private:
    Handler::Result preDispatch  (const Event& e) override;
    Handler::Result postDispatch (const Event& e, Handler::Result pendingResult) override;

    Handler::Result onCiDiscoveryInquiry  (const Event& e) override;
    Handler::Result onCiDiscoveryReply    (const Event& e) override;
    Handler::Result onCiEndpointInquiry   (const Event& e) override;
    Handler::Result onCiEndpointReply     (const Event& e) override;

    AppContext appContext;
    PersistentContext pc;
    EventView* eventView  { nullptr };
    int        currentWidth { 0 };
};
