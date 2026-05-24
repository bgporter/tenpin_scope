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

#include "model/ci/ack.h"
#include "model/ci/ciConstants.h"
#include "model/ci/discovery.h"
#include "model/ci/endpointInfo.h"
#include "model/ci/invalidateMuid.h"
#include "model/ci/profile.h"
#include "model/ci/propertyExchange.h"
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

juce::String formatManufacturerId (int byte0, int byte1, int byte2)
{
    if (byte0 != 0)
        return formatValue (static_cast<uint32_t> (byte0), 8, ValueFormatType::Hex);
    return formatValue (0u, 8, ValueFormatType::Hex) + " " +
           formatValue (static_cast<uint32_t> (byte1), 8, ValueFormatType::Hex) + " " +
           formatValue (static_cast<uint32_t> (byte2), 8, ValueFormatType::Hex);
}

juce::String formatProfileId (const ProfileId& p)
{
    auto hex = [] (uint8_t b) { return formatValue (static_cast<uint32_t> (b), 8, ValueFormatType::Hex); };
    if (p.isStandardDefined ())
        return "(std) bank: " + hex (p.byte2) + " number: " + hex (p.byte3) + " version: " + hex (p.byte4) +
               " level: " + hex (p.byte5);
    return "(mfr) id: " + hex (p.byte1) + " " + hex (p.byte2) + " " + hex (p.byte3) + " info: " + hex (p.byte4) + " " +
           hex (p.byte5);
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
    view->addValue ("manufacturer ID", formatManufacturerId (m.manufacturerByte0.get (), m.manufacturerByte1.get (),
                                                             m.manufacturerByte2.get ()));
    view->addLine ();
    view->addValue ("device family", formatValue (static_cast<uint32_t> (m.deviceFamilyMsb.get ()), 7, fmt) + " " +
                                         formatValue (static_cast<uint32_t> (m.deviceFamilyLsb.get ()), 7, fmt));
    view->addValue ("device model", formatValue (static_cast<uint32_t> (m.deviceFamilyModelMsb.get ()), 7, fmt) + " " +
                                        formatValue (static_cast<uint32_t> (m.deviceFamilyModelLsb.get ()), 7, fmt));
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

template <typename T>
void displayAckNak (EventView* view, const juce::String& title, const T& m, ValueFormatType fmt)
{
    view->setEvent (title);
    view->addValue ("device ID", formatDeviceId (m.deviceId.get ()));
    view->addValue ("grp", juce::String (static_cast<int> (m.userGroup)));
    view->addLine ();
    view->addValue ("src MUID", formatMuid (m.sourceMuid.get (), fmt));
    view->addValue ("dst MUID", formatMuid (m.destMuid.get (), fmt));
    view->addLine ();
    view->addValue ("orig sub-ID",
                    formatValue (static_cast<uint32_t> (m.originalSubId.get ()), 8, ValueFormatType::Hex));
    view->addValue ("status", formatValue (static_cast<uint32_t> (m.statusCode.get ()), 8, fmt));
    view->addValue ("status data", formatValue (static_cast<uint32_t> (m.statusData.get ()), 8, fmt));
    view->addLine ();
    view->addValue ("details",
                    formatValue (static_cast<uint32_t> (m.detail0.get ()), 8, ValueFormatType::Hex) + " " +
                        formatValue (static_cast<uint32_t> (m.detail1.get ()), 8, ValueFormatType::Hex) + " " +
                        formatValue (static_cast<uint32_t> (m.detail2.get ()), 8, ValueFormatType::Hex) + " " +
                        formatValue (static_cast<uint32_t> (m.detail3.get ()), 8, ValueFormatType::Hex) + " " +
                        formatValue (static_cast<uint32_t> (m.detail4.get ()), 8, ValueFormatType::Hex));
    const auto text = static_cast<juce::String> (m.messageText);
    if (text.isNotEmpty ())
    {
        view->addLine ();
        view->addValue ("message", text);
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

ValueFormatType EventListViewCiHandler::valueFormat () const
{
    const auto fmt = pc.eventViewContext.valueFormatType.get ();
    return (fmt == ValueFormatType::Decimal) ? ValueFormatType::Decimal : ValueFormatType::Hex;
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

Handler::Result EventListViewCiHandler::onCiEndpointInquiry (const Event& e)
{
    CiEndpointInquiry m { e };
    const auto fmt = valueFormat ();

    eventView->setEvent ("CI Endpoint Inquiry");
    eventView->addValue ("device ID", formatDeviceId (m.deviceId.get ()));
    eventView->addValue ("grp", juce::String (static_cast<int> (m.userGroup)));
    eventView->addLine ();
    eventView->addValue ("src MUID", formatMuid (m.sourceMuid.get (), fmt));
    eventView->addValue ("dst MUID", formatMuid (m.destMuid.get (), fmt));
    eventView->addLine ();
    eventView->addValue ("status", formatValue (static_cast<uint32_t> (m.status.get ()), 8, fmt));
    return Handler::Result::ok;
}

Handler::Result EventListViewCiHandler::onCiEndpointReply (const Event& e)
{
    CiEndpointReply m { e };
    const auto fmt = valueFormat ();

    eventView->setEvent ("CI Endpoint Reply");
    eventView->addValue ("device ID", formatDeviceId (m.deviceId.get ()));
    eventView->addValue ("grp", juce::String (static_cast<int> (m.userGroup)));
    eventView->addLine ();
    eventView->addValue ("src MUID", formatMuid (m.sourceMuid.get (), fmt));
    eventView->addValue ("dst MUID", formatMuid (m.destMuid.get (), fmt));
    eventView->addLine ();
    eventView->addValue ("status", formatValue (static_cast<uint32_t> (m.status.get ()), 8, fmt));
    eventView->addValue ("prod ID", static_cast<juce::String> (m.productInstanceId));
    return Handler::Result::ok;
}

Handler::Result EventListViewCiHandler::onCiInvalidateMuid (const Event& e)
{
    CiInvalidateMuid m { e };
    const auto fmt = valueFormat ();

    eventView->setEvent ("CI Invalidate MUID");
    eventView->addValue ("device ID", formatDeviceId (m.deviceId.get ()));
    eventView->addValue ("grp", juce::String (static_cast<int> (m.userGroup)));
    eventView->addLine ();
    eventView->addValue ("src MUID", formatMuid (m.sourceMuid.get (), fmt));
    eventView->addValue ("target MUID", formatMuid (m.targetMuid.get (), fmt));
    return Handler::Result::ok;
}

Handler::Result EventListViewCiHandler::onCiNak (const Event& e)
{
    CiNak m { e };
    displayAckNak (eventView, "CI NAK", m, valueFormat ());
    return Handler::Result::ok;
}

Handler::Result EventListViewCiHandler::onCiAck (const Event& e)
{
    CiAck m { e };
    displayAckNak (eventView, "CI ACK", m, valueFormat ());
    return Handler::Result::ok;
}

Handler::Result EventListViewCiHandler::onCiPeCapabilitiesInquiry (const Event& e)
{
    CiPeCapabilitiesInquiry m { e };
    const auto fmt = valueFormat ();

    eventView->setEvent ("CI PE Capabilities Inquiry");
    eventView->addValue ("device ID", formatDeviceId (m.deviceId.get ()));
    eventView->addValue ("grp", juce::String (static_cast<int> (m.userGroup)));
    eventView->addLine ();
    eventView->addValue ("src MUID", formatMuid (m.sourceMuid.get (), fmt));
    eventView->addValue ("dst MUID", formatMuid (m.destMuid.get (), fmt));
    eventView->addLine ();
    eventView->addValue ("simultaneous requests", juce::String (m.simultaneousRequests.get ()));
    if (m.messageFormat.get () >= 2)
    {
        eventView->addValue ("PE major version", juce::String (m.majorVersion.get ()));
        eventView->addValue ("PE minor version", juce::String (m.minorVersion.get ()));
    }
    return Handler::Result::ok;
}

Handler::Result EventListViewCiHandler::onCiPeCapabilitiesReply (const Event& e)
{
    CiPeCapabilitiesReply m { e };
    const auto fmt = valueFormat ();

    eventView->setEvent ("CI PE Capabilities Reply");
    eventView->addValue ("device ID", formatDeviceId (m.deviceId.get ()));
    eventView->addValue ("grp", juce::String (static_cast<int> (m.userGroup)));
    eventView->addLine ();
    eventView->addValue ("src MUID", formatMuid (m.sourceMuid.get (), fmt));
    eventView->addValue ("dst MUID", formatMuid (m.destMuid.get (), fmt));
    eventView->addLine ();
    eventView->addValue ("simultaneous requests", juce::String (m.simultaneousRequests.get ()));
    if (m.messageFormat.get () >= 2)
    {
        eventView->addValue ("PE major version", juce::String (m.majorVersion.get ()));
        eventView->addValue ("PE minor version", juce::String (m.minorVersion.get ()));
    }
    return Handler::Result::ok;
}

Handler::Result EventListViewCiHandler::onCiPeGetPropertyDataInquiry (const Event& e)
{
    CiPeGetPropertyDataInquiry m { e };
    const auto fmt = valueFormat ();

    eventView->setEvent ("CI PE Get Property Data");
    eventView->addValue ("device ID", formatDeviceId (m.deviceId.get ()));
    eventView->addValue ("grp", juce::String (static_cast<int> (m.userGroup)));
    eventView->addLine ();
    eventView->addValue ("src MUID",   formatMuid (m.sourceMuid.get (), fmt));
    eventView->addValue ("dst MUID",   formatMuid (m.destMuid.get (), fmt));
    eventView->addValue ("request ID", formatValue (static_cast<uint32_t> (m.requestId.get ()), 8, fmt));
    eventView->addLine ();
    if (auto hdr { m.headerData.get () }; hdr != nullptr && hdr->size () > 0)
        eventView->addValue ("header", juce::String::fromUTF8 (
            reinterpret_cast<const char*> (&(*hdr)[0]), static_cast<int> (hdr->size ())));
    else
        eventView->addValue ("header", "(empty)");
    return Handler::Result::ok;
}

Handler::Result EventListViewCiHandler::onCiPeGetPropertyDataReply (const Event& e)
{
    CiPeGetPropertyDataReply m { e };
    const auto fmt = valueFormat ();

    eventView->setEvent ("CI PE Get Property Data Reply");
    eventView->addValue ("device ID", formatDeviceId (m.deviceId.get ()));
    eventView->addValue ("grp", juce::String (static_cast<int> (m.userGroup)));
    eventView->addLine ();
    eventView->addValue ("src MUID",   formatMuid (m.sourceMuid.get (), fmt));
    eventView->addValue ("dst MUID",   formatMuid (m.destMuid.get (), fmt));
    eventView->addValue ("request ID", formatValue (static_cast<uint32_t> (m.requestId.get ()), 8, fmt));
    eventView->addLine ();
    if (auto hdr { m.headerData.get () }; hdr != nullptr && hdr->size () > 0)
        eventView->addValue ("header", juce::String::fromUTF8 (
            reinterpret_cast<const char*> (&(*hdr)[0]), static_cast<int> (hdr->size ())));
    else
        eventView->addValue ("header", "(empty)");
    eventView->addLine ();
    const int nc = m.numberOfChunks.get ();
    eventView->addValue ("chunk", juce::String (m.chunkNumber.get ()) + " of "
                                  + (nc == 0 ? "?" : juce::String (nc)));
    if (auto prop { m.propertyData.get () }; prop != nullptr && prop->size () > 0)
    {
        eventView->addValue ("data", juce::String (static_cast<int> (prop->size ())) + " bytes");
        eventView->addLine ();
        eventView->addValue ("  value", juce::String::fromUTF8 (
            reinterpret_cast<const char*> (&(*prop)[0]), static_cast<int> (prop->size ())));
    }
    else
    {
        eventView->addValue ("data", "(empty)");
    }
    return Handler::Result::ok;
}

Handler::Result EventListViewCiHandler::onCiPeSetPropertyDataInquiry (const Event& e)
{
    CiPeSetPropertyDataInquiry m { e };
    const auto fmt = valueFormat ();

    eventView->setEvent ("CI PE Set Property Data");
    eventView->addValue ("device ID", formatDeviceId (m.deviceId.get ()));
    eventView->addValue ("grp", juce::String (static_cast<int> (m.userGroup)));
    eventView->addLine ();
    eventView->addValue ("src MUID",   formatMuid (m.sourceMuid.get (), fmt));
    eventView->addValue ("dst MUID",   formatMuid (m.destMuid.get (), fmt));
    eventView->addValue ("request ID", formatValue (static_cast<uint32_t> (m.requestId.get ()), 8, fmt));
    eventView->addLine ();
    if (auto hdr { m.headerData.get () }; hdr != nullptr && hdr->size () > 0)
        eventView->addValue ("header", juce::String::fromUTF8 (
            reinterpret_cast<const char*> (&(*hdr)[0]), static_cast<int> (hdr->size ())));
    else
        eventView->addValue ("header", "(empty)");
    eventView->addLine ();
    const int nc = m.numberOfChunks.get ();
    eventView->addValue ("chunk", juce::String (m.chunkNumber.get ()) + " of "
                                  + (nc == 0 ? "?" : juce::String (nc)));
    if (auto prop { m.propertyData.get () }; prop != nullptr && prop->size () > 0)
    {
        eventView->addValue ("data", juce::String (static_cast<int> (prop->size ())) + " bytes");
        eventView->addLine ();
        eventView->addValue ("  value", juce::String::fromUTF8 (
            reinterpret_cast<const char*> (&(*prop)[0]), static_cast<int> (prop->size ())));
    }
    else
    {
        eventView->addValue ("data", "(empty)");
    }
    return Handler::Result::ok;
}

Handler::Result EventListViewCiHandler::onCiPeSetPropertyDataReply (const Event& e)
{
    CiPeSetPropertyDataReply m { e };
    const auto fmt = valueFormat ();

    eventView->setEvent ("CI PE Set Property Data Reply");
    eventView->addValue ("device ID", formatDeviceId (m.deviceId.get ()));
    eventView->addValue ("grp", juce::String (static_cast<int> (m.userGroup)));
    eventView->addLine ();
    eventView->addValue ("src MUID",   formatMuid (m.sourceMuid.get (), fmt));
    eventView->addValue ("dst MUID",   formatMuid (m.destMuid.get (), fmt));
    eventView->addValue ("request ID", formatValue (static_cast<uint32_t> (m.requestId.get ()), 8, fmt));
    eventView->addLine ();
    if (auto hdr { m.headerData.get () }; hdr != nullptr && hdr->size () > 0)
        eventView->addValue ("header", juce::String::fromUTF8 (
            reinterpret_cast<const char*> (&(*hdr)[0]), static_cast<int> (hdr->size ())));
    else
        eventView->addValue ("header", "(empty)");
    return Handler::Result::ok;
}

static Handler::Result displayPeSubscription (EventView* eventView, const juce::String& eventName,
                                              int deviceId, int userGroup,
                                              int sourceMuid, int destMuid, int reqId,
                                              Buffer::Ptr headerData, int numChunks, int chunkNum,
                                              Buffer::Ptr propData, ValueFormatType fmt)
{
    eventView->setEvent (eventName);
    eventView->addValue ("device ID", formatDeviceId (deviceId));
    eventView->addValue ("grp", juce::String (userGroup));
    eventView->addLine ();
    eventView->addValue ("src MUID",   formatMuid (sourceMuid, fmt));
    eventView->addValue ("dst MUID",   formatMuid (destMuid, fmt));
    eventView->addValue ("request ID", formatValue (static_cast<uint32_t> (reqId), 8, fmt));
    eventView->addLine ();
    if (auto hdr { headerData.get () }; hdr != nullptr && hdr->size () > 0)
        eventView->addValue ("header", juce::String::fromUTF8 (
            reinterpret_cast<const char*> (&(*hdr)[0]), static_cast<int> (hdr->size ())));
    else
        eventView->addValue ("header", "(empty)");
    eventView->addLine ();
    eventView->addValue ("chunk", juce::String (chunkNum) + " of "
                                  + (numChunks == 0 ? "?" : juce::String (numChunks)));
    if (auto prop { propData.get () }; prop != nullptr && prop->size () > 0)
    {
        eventView->addValue ("data", juce::String (static_cast<int> (prop->size ())) + " bytes");
        eventView->addLine ();
        eventView->addValue ("  value", juce::String::fromUTF8 (
            reinterpret_cast<const char*> (&(*prop)[0]), static_cast<int> (prop->size ())));
    }
    else
    {
        eventView->addValue ("data", "(empty)");
    }
    return Handler::Result::ok;
}

Handler::Result EventListViewCiHandler::onCiPeSubscriptionInquiry (const Event& e)
{
    CiPeSubscriptionInquiry m { e };
    const auto fmt = valueFormat ();
    return displayPeSubscription (eventView, "CI PE Subscription",
                                  m.deviceId.get (), static_cast<int> (m.userGroup),
                                  m.sourceMuid.get (), m.destMuid.get (), m.requestId.get (),
                                  m.headerData.get (), m.numberOfChunks.get (),
                                  m.chunkNumber.get (), m.propertyData.get (), fmt);
}

Handler::Result EventListViewCiHandler::onCiPeSubscriptionReply (const Event& e)
{
    CiPeSubscriptionReply m { e };
    const auto fmt = valueFormat ();
    return displayPeSubscription (eventView, "CI PE Subscription Reply",
                                  m.deviceId.get (), static_cast<int> (m.userGroup),
                                  m.sourceMuid.get (), m.destMuid.get (), m.requestId.get (),
                                  m.headerData.get (), m.numberOfChunks.get (),
                                  m.chunkNumber.get (), m.propertyData.get (), fmt);
}

Handler::Result EventListViewCiHandler::onCiPeNotify (const Event& e)
{
    // Notify (0x3F) is deprecated in MIDI-CI v1.1; ACK/NAK should be used instead.
    // Displayed for backward compatibility with legacy devices.
    CiPeNotify m { e };
    const auto fmt = valueFormat ();
    return displayPeSubscription (eventView, "CI PE Notify (deprecated)",
                                  m.deviceId.get (), static_cast<int> (m.userGroup),
                                  m.sourceMuid.get (), m.destMuid.get (), m.requestId.get (),
                                  m.headerData.get (), m.numberOfChunks.get (),
                                  m.chunkNumber.get (), m.propertyData.get (), fmt);
}

Handler::Result EventListViewCiHandler::onCiProfileInquiry (const Event& e)
{
    CiProfileInquiry m { e };
    const auto fmt = valueFormat ();

    eventView->setEvent ("CI Profile Inquiry");
    eventView->addValue ("device ID", formatDeviceId (m.deviceId.get ()));
    eventView->addValue ("grp", juce::String (static_cast<int> (m.userGroup)));
    eventView->addLine ();
    eventView->addValue ("src MUID", formatMuid (m.sourceMuid.get (), fmt));
    eventView->addValue ("dst MUID", formatMuid (m.destMuid.get (), fmt));
    return Handler::Result::ok;
}

Handler::Result EventListViewCiHandler::onCiProfileInquiryReply (const Event& e)
{
    CiProfileInquiryReply m { e };
    const auto fmt = valueFormat ();

    eventView->setEvent ("CI Profile Inquiry Reply");
    eventView->addValue ("device ID", formatDeviceId (m.deviceId.get ()));
    eventView->addValue ("grp", juce::String (static_cast<int> (m.userGroup)));
    eventView->addLine ();
    eventView->addValue ("src MUID", formatMuid (m.sourceMuid.get (), fmt));
    eventView->addValue ("dst MUID", formatMuid (m.destMuid.get (), fmt));
    eventView->addLine ();

    const int cep = m.enabledProfileCount ();
    eventView->addValue ("enabled", juce::String (cep));
    const juce::String indent ("   ");
    for (int i = 0; i < cep; ++i)
    {
        eventView->addLine ();
        eventView->addValue (indent + juce::String (i + 1) + ".", formatProfileId (m.enabledProfileAt (i)));
    }

    eventView->addLine ();
    const int cdp = m.disabledProfileCount ();
    eventView->addValue ("disabled", juce::String (cdp));
    for (int i = 0; i < cdp; ++i)
    {
        eventView->addLine ();
        eventView->addValue (indent + juce::String (i + 1) + ".", formatProfileId (m.disabledProfileAt (i)));
    }

    return Handler::Result::ok;
}

Handler::Result EventListViewCiHandler::onCiProfileSetOn (const Event& e)
{
    CiProfileSetOn m { e };
    const auto fmt = valueFormat ();

    eventView->setEvent ("CI Set Profile On");
    eventView->addValue ("device ID", formatDeviceId (m.deviceId.get ()));
    eventView->addValue ("grp",       juce::String (static_cast<int> (m.userGroup)));
    eventView->addLine ();
    eventView->addValue ("src MUID", formatMuid (m.sourceMuid.get (), fmt));
    eventView->addValue ("dst MUID", formatMuid (m.destMuid.get (), fmt));
    eventView->addLine ();
    eventView->addValue ("profile",   formatProfileId (m.profileId ()));
    if (m.messageFormat.get () >= 2)
        eventView->addValue ("channels", formatValue (static_cast<uint32_t> (m.channelsRequested.get ()), 14, fmt));
    return Handler::Result::ok;
}

Handler::Result EventListViewCiHandler::onCiProfileSetOff (const Event& e)
{
    CiProfileSetOff m { e };
    const auto fmt = valueFormat ();

    eventView->setEvent ("CI Set Profile Off");
    eventView->addValue ("device ID", formatDeviceId (m.deviceId.get ()));
    eventView->addValue ("grp",       juce::String (static_cast<int> (m.userGroup)));
    eventView->addLine ();
    eventView->addValue ("src MUID", formatMuid (m.sourceMuid.get (), fmt));
    eventView->addValue ("dst MUID", formatMuid (m.destMuid.get (), fmt));
    eventView->addLine ();
    eventView->addValue ("profile", formatProfileId (m.profileId ()));
    return Handler::Result::ok;
}

Handler::Result EventListViewCiHandler::onCiProfileEnabled (const Event& e)
{
    CiProfileEnabled m { e };
    const auto fmt = valueFormat ();

    eventView->setEvent ("CI Profile Enabled");
    eventView->addValue ("device ID", formatDeviceId (m.deviceId.get ()));
    eventView->addValue ("grp",       juce::String (static_cast<int> (m.userGroup)));
    eventView->addLine ();
    eventView->addValue ("src MUID", formatMuid (m.sourceMuid.get (), fmt));
    eventView->addValue ("dst MUID", formatMuid (m.destMuid.get (), fmt));
    eventView->addLine ();
    eventView->addValue ("profile", formatProfileId (m.profileId ()));
    if (m.messageFormat.get () >= 2)
        eventView->addValue ("channels", formatValue (static_cast<uint32_t> (m.channelsEnabled.get ()), 14, fmt));
    return Handler::Result::ok;
}

Handler::Result EventListViewCiHandler::onCiProfileDisabled (const Event& e)
{
    CiProfileDisabled m { e };
    const auto fmt = valueFormat ();

    eventView->setEvent ("CI Profile Disabled");
    eventView->addValue ("device ID", formatDeviceId (m.deviceId.get ()));
    eventView->addValue ("grp",       juce::String (static_cast<int> (m.userGroup)));
    eventView->addLine ();
    eventView->addValue ("src MUID", formatMuid (m.sourceMuid.get (), fmt));
    eventView->addValue ("dst MUID", formatMuid (m.destMuid.get (), fmt));
    eventView->addLine ();
    eventView->addValue ("profile",  formatProfileId (m.profileId ()));
    if (m.messageFormat.get () >= 2)
        eventView->addValue ("channels", formatValue (static_cast<uint32_t> (m.channelsDisabled.get ()), 14, fmt));
    return Handler::Result::ok;
}

Handler::Result EventListViewCiHandler::onCiProfileAdded (const Event& e)
{
    CiProfileAdded m { e };
    const auto fmt = valueFormat ();

    eventView->setEvent ("CI Profile Added");
    eventView->addValue ("device ID", formatDeviceId (m.deviceId.get ()));
    eventView->addValue ("grp",       juce::String (static_cast<int> (m.userGroup)));
    eventView->addLine ();
    eventView->addValue ("src MUID", formatMuid (m.sourceMuid.get (), fmt));
    eventView->addValue ("dst MUID", formatMuid (m.destMuid.get (), fmt));
    eventView->addLine ();
    eventView->addValue ("profile", formatProfileId (m.profileId ()));
    return Handler::Result::ok;
}

Handler::Result EventListViewCiHandler::onCiProfileRemoved (const Event& e)
{
    CiProfileRemoved m { e };
    const auto fmt = valueFormat ();

    eventView->setEvent ("CI Profile Removed");
    eventView->addValue ("device ID", formatDeviceId (m.deviceId.get ()));
    eventView->addValue ("grp",       juce::String (static_cast<int> (m.userGroup)));
    eventView->addLine ();
    eventView->addValue ("src MUID", formatMuid (m.sourceMuid.get (), fmt));
    eventView->addValue ("dst MUID", formatMuid (m.destMuid.get (), fmt));
    eventView->addLine ();
    eventView->addValue ("profile", formatProfileId (m.profileId ()));
    return Handler::Result::ok;
}

Handler::Result EventListViewCiHandler::onCiProfileDetailsInquiry (const Event& e)
{
    CiProfileDetailsInquiry m { e };
    const auto fmt = valueFormat ();

    eventView->setEvent ("CI Profile Details Inquiry");
    eventView->addValue ("device ID", formatDeviceId (m.deviceId.get ()));
    eventView->addValue ("grp",       juce::String (static_cast<int> (m.userGroup)));
    eventView->addLine ();
    eventView->addValue ("src MUID", formatMuid (m.sourceMuid.get (), fmt));
    eventView->addValue ("dst MUID", formatMuid (m.destMuid.get (), fmt));
    eventView->addLine ();
    eventView->addValue ("profile", formatProfileId (m.profileId ()));
    eventView->addValue ("target",  formatValue (static_cast<uint32_t> (m.inquiryTarget.get ()), 8, ValueFormatType::Hex) +
                                        (m.isRegisteredTarget () ? "  (registered)" : "  (profile-specific)"));
    return Handler::Result::ok;
}

Handler::Result EventListViewCiHandler::onCiProfileDetailsInquiryReply (const Event& e)
{
    CiProfileDetailsInquiryReply m { e };
    const auto fmt = valueFormat ();

    eventView->setEvent ("CI Profile Details Reply");
    eventView->addValue ("device ID", formatDeviceId (m.deviceId.get ()));
    eventView->addValue ("grp",       juce::String (static_cast<int> (m.userGroup)));
    eventView->addLine ();
    eventView->addValue ("src MUID", formatMuid (m.sourceMuid.get (), fmt));
    eventView->addValue ("dst MUID", formatMuid (m.destMuid.get (), fmt));
    eventView->addLine ();
    eventView->addValue ("profile", formatProfileId (m.profileId ()));
    eventView->addValue ("target",  formatValue (static_cast<uint32_t> (m.inquiryTarget.get ()), 8, ValueFormatType::Hex) +
                                        (m.isRegisteredTarget () ? "  (registered)" : "  (profile-specific)"));

    if (auto data = m.targetData.get (); data && !data->empty ())
    {
        eventView->addLine ();
        eventView->addValue ("data", "");
        for (size_t i = 0; i < data->size (); ++i)
            eventView->addValue ("", formatValue ((*data)[i], 8, ValueFormatType::Hex));
    }
    return Handler::Result::ok;
}

Handler::Result EventListViewCiHandler::onCiProfileSpecificData (const Event& e)
{
    CiProfileSpecificData m { e };
    const auto fmt = valueFormat ();

    eventView->setEvent ("CI Profile Specific Data");
    eventView->addValue ("device ID", formatDeviceId (m.deviceId.get ()));
    eventView->addValue ("grp",       juce::String (static_cast<int> (m.userGroup)));
    eventView->addLine ();
    eventView->addValue ("src MUID", formatMuid (m.sourceMuid.get (), fmt));
    eventView->addValue ("dst MUID", formatMuid (m.destMuid.get (), fmt));
    eventView->addLine ();
    eventView->addValue ("profile", formatProfileId (m.profileId ()));

    if (auto data = m.profileData.get (); data && !data->empty ())
    {
        eventView->addLine ();
        eventView->addValue ("data", juce::String (static_cast<int> (data->size ())) + " bytes");
        for (size_t i = 0; i < data->size (); ++i)
            eventView->addValue ("", formatValue ((*data)[i], 8, ValueFormatType::Hex));
    }
    return Handler::Result::ok;
}

Handler::Result EventListViewCiHandler::onCiDiscoveryInquiry (const Event& e)
{
    CiDiscoveryInquiry m { e };
    const auto fmt = valueFormat ();

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
    const auto fmt = valueFormat ();

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
