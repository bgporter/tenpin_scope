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

#include "ciHandler.h"

#include "model/ci/profile.h"
#include "model/ci/discovery.h"
#include "model/ci/endpointInfo.h"
#include "model/ci/invalidateMuid.h"
#include "model/ci/ack.h"

CiHandler::CiHandler () {}

CiHandler::~CiHandler () {}

Handler::Result CiHandler::handle (const Event& e)
{
    Handler::Result result { preDispatch (e) };
    if (result == Handler::Result::ok)
    {
        const auto type = e.getType ();
        if (type == CiProfileInquiry::type)
            result = onCiProfileInquiry (e);
        else if (type == CiProfileInquiryReply::type)
            result = onCiProfileInquiryReply (e);
        else if (type == CiDiscoveryInquiry::type)
            result = onCiDiscoveryInquiry (e);
        else if (type == CiDiscoveryReply::type)
            result = onCiDiscoveryReply (e);
        else if (type == CiEndpointInquiry::type)
            result = onCiEndpointInquiry (e);
        else if (type == CiEndpointReply::type)
            result = onCiEndpointReply (e);
        else if (type == CiInvalidateMuid::type)
            result = onCiInvalidateMuid (e);
        else if (type == CiNak::type)
            result = onCiNak (e);
        else if (type == CiAck::type)
            result = onCiAck (e);
        else
            result = onCiMessage (e);
    }
    return postDispatch (e, result);
}

Handler::Result CiHandler::handle (const Event& e, void* /*ctx*/)
{
    return handle (e);
}
