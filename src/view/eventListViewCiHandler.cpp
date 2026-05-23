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

#include "eventListViewCiHandler.h"

#include "model/ci/ciConstants.h"
#include "model/ci/discovery.h"
#include "palette.h"
#include "view/dispatchContext.h"
#include "view/eventNameUtils.h"
#include "view/eventView.h"

namespace
{
juce::String formatMuid (int muid, ValueFormatType fmt)
{
    if (muid == broadcastMuid)
        return "broadcast";
    return formatValue (static_cast<uint32_t> (muid), 28, fmt);
}

juce::String formatDeviceId (int deviceId)
{
    if (deviceId == CiDeviceId::functionBlock)
        return "function block";
    if (deviceId == CiDeviceId::group)
        return "group";
    if (deviceId >= 0 && deviceId <= 0x0F)
        return "ch " + juce::String (deviceId + 1);
    return juce::String::toHexString (deviceId);
}

juce::String formatCiCategories (int bitmap)
{
    juce::StringArray names;
    if (bitmap & CiCategory::protocolNegotiation)
        names.add ("Protocol");
    if (bitmap & CiCategory::profileConfiguration)
        names.add ("Profile");
    if (bitmap & CiCategory::propertyExchange)
        names.add ("Property");
    if (bitmap & CiCategory::processInquiry)
        names.add ("Process");
    return names.isEmpty () ? juce::String ("none") : names.joinIntoString (", ");
}

// Adds the fields shared by both Discovery Inquiry and Discovery Reply.
// Works with any struct that has the discovery field members.
template <typename T> void addDiscoveryCommonFields (EventView* view, const T& m, ValueFormatType fmt)
{
    view->addValue ("device ID", formatDeviceId (m.deviceId.get ()));
    view->addValue ("grp", juce::String (static_cast<int> (m.userGroup)));
    view->addLine ();
    view->addValue ("src MUID", formatMuid (m.sourceMuid.get (), fmt));
    view->addValue ("dst MUID", formatMuid (m.destMuid.get (), fmt));
    view->addLine ();
    view->addValue ("manufacturer ID",
                    formatValue (static_cast<uint32_t> (m.manufacturerByte0.get ()), 8, ValueFormatType::Hex) + " " +
                        formatValue (static_cast<uint32_t> (m.manufacturerByte1.get ()), 8, ValueFormatType::Hex) +
                        " " +
                        formatValue (static_cast<uint32_t> (m.manufacturerByte2.get ()), 8, ValueFormatType::Hex));
    view->addLine ();
    view->addValue (
        "device family",
        formatValue (static_cast<uint32_t> (m.deviceFamilyLsb.get () | (m.deviceFamilyMsb.get () << 7)), 14, fmt));
    view->addValue ("device model", formatValue (static_cast<uint32_t> (m.deviceFamilyModelLsb.get () |
                                                                        (m.deviceFamilyModelMsb.get () << 7)),
                                                 14, fmt));
    view->addValue ("sw revision", juce::String (m.revisionByte0.get ()) + "." + juce::String (m.revisionByte1.get ()) +
                                       "." + juce::String (m.revisionByte2.get ()) + "." +
                                       juce::String (m.revisionByte3.get ()));
    view->addLine ();
    view->addValue ("CI categories", formatCiCategories (m.ciCategoriesSupported.get ()));
    view->addValue ("maximum size", formatValue (static_cast<uint32_t> (m.maxSysexSize.get ()), 28, fmt));
}

template <typename T> void addExtraData (EventView* view, const T& m)
{
    if (auto extra = m.extraData.get (); extra && !extra->empty ())
    {
        view->addValue ("extra", "");
        for (size_t i = 0; i < extra->size (); ++i)
            view->addValue ("", formatValue ((*extra)[i], 8, ValueFormatType::Hex));
    }
}
} // namespace

// ============================================================================

EventListViewCiHandler::EventListViewCiHandler (AppContext& theAppContext)
: appContext { theAppContext }
, pc { theAppContext }
{
}

EventListViewCiHandler::~EventListViewCiHandler () {}

Handler::Result EventListViewCiHandler::handle (const Event& e, void* ctx)
{
    if (ctx)
    {
        auto* dispCtx = static_cast<DispatchContext*> (ctx);
        eventView     = dispCtx->view;
        currentWidth  = dispCtx->width;
    }

    if (eventView)
    {
        Palette pal { pc };
        eventView->setColors (static_cast<juce::Colour> (pal.umpBackground.get ()).brighter (0.1f),
                              pal.midiCiLabel.get (), pal.midiCiValue.get (), pal.outline.get ());
    }

    return CiHandler::handle (e, ctx);
}

Handler::Result EventListViewCiHandler::preDispatch (const Event& e)
{
    if (!eventView)
        return Handler::Result::notHandled;
    eventView->setTime (formatTime (e.timestamp));
    eventView->setEndpoint (formatEndpoint (e));
    return Handler::Result::ok;
}

Handler::Result EventListViewCiHandler::postDispatch (const Event& /*e*/, Handler::Result pendingResult)
{
    if (pendingResult == Handler::Result::ok)
        eventView->sizeToWidth (currentWidth);
    return pendingResult;
}

Handler::Result EventListViewCiHandler::onCiDiscoveryInquiry (const Event& e)
{
    CiDiscoveryInquiry m { e };
    const auto rawFmt = pc.eventViewContext.valueFormatType.get ();
    const auto fmt = (rawFmt == ValueFormatType::Integer) ? ValueFormatType::Integer : ValueFormatType::Hex;

    eventView->setEvent ("CI Discovery Inquiry");
    addDiscoveryCommonFields (eventView, m, fmt);

    if (m.messageFormat.get () >= 2)
        eventView->addValue ("path", juce::String (m.outputPathId.get ()));

    addExtraData (eventView, m);
    return Handler::Result::ok;
}

Handler::Result EventListViewCiHandler::onCiDiscoveryReply (const Event& e)
{
    CiDiscoveryReply m { e };
    const auto rawFmt = pc.eventViewContext.valueFormatType.get ();
    const auto fmt = (rawFmt == ValueFormatType::Integer) ? ValueFormatType::Integer : ValueFormatType::Hex;

    eventView->setEvent ("CI Discovery Reply");
    addDiscoveryCommonFields (eventView, m, fmt);

    if (m.messageFormat.get () >= 2)
    {
        eventView->addValue ("path", juce::String (m.outputPathId.get ()));
        eventView->addValue ("fb", juce::String (m.functionBlock.get ()));
    }

    addExtraData (eventView, m);
    return Handler::Result::ok;
}
