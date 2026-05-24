
// Shared base class for CI unit test suites. Each CI test .inl file includes
// this file and inherits from CiTestHelpers instead of TestSuite directly.

class CiTestHelpers : public TestSuite
{
public:
    CiTestHelpers (const juce::String& name, const juce::String& category)
    : TestSuite (name, category)
    {
    }

protected:
    bool noF0inBuf (const Buffer& buf) const
    {
        for (size_t i = 0; i < buf.size (); ++i)
            if (buf[i] == 0xF0) return false;
        return true;
    }

    bool noF7inBuf (const Buffer& buf) const
    {
        for (size_t i = 0; i < buf.size (); ++i)
            if (buf[i] == 0xF7) return false;
        return true;
    }

    // Reconstruct a 28-bit MUID from 4 × 7-bit bytes at buf[offset..offset+3].
    int muidFromBuf (const Buffer& buf, size_t offset) const
    {
        return static_cast<int> (buf[offset])
             | (static_cast<int> (buf[offset + 1]) << 7)
             | (static_cast<int> (buf[offset + 2]) << 14)
             | (static_cast<int> (buf[offset + 3]) << 21);
    }

    // Reconstruct a 14-bit word from 2 × 7-bit bytes at buf[offset..offset+1].
    int wordFromBuf (const Buffer& buf, size_t offset) const
    {
        return static_cast<int> (buf[offset]) | (static_cast<int> (buf[offset + 1]) << 7);
    }

    // Validates the 5-byte common CI SysEx header (buf[0..4]).
    // expectedDeviceId: CiDeviceId::functionBlock (0x7F) for all current CI messages.
    // expectedFormat:   the message format version byte (1 or 2).
    void checkCommonCiHeader (const Buffer& buf, int expectedDeviceId,
                               int expectedType, int expectedFormat)
    {
        expectEquals (static_cast<int> (buf[0]), 0x7E, "buf[0] must be 0x7E, not 0xF0");
        expectEquals (static_cast<int> (buf[1]), expectedDeviceId, "device ID");
        expectEquals (static_cast<int> (buf[2]), 0x0D, "MIDI-CI sub-ID");
        expectEquals (static_cast<int> (buf[3]), expectedType, "message type");
        expect (static_cast<int> (buf[4]) >= messageFormatMin, "format must be >= 1");
        expectEquals (static_cast<int> (buf[4]), expectedFormat, "format byte");
        expect (noF0inBuf (buf), "Buffer must not contain 0xF0");
        expect (noF7inBuf (buf), "Buffer must not contain 0xF7");
    }

    // Build a Buffer::Ptr from a null-terminated ASCII/JSON string.
    Buffer::Ptr makeHeader (const char* json) const
    {
        Buffer::Ptr hdr = new Buffer ();
        for (const char* p = json; *p; ++p)
            hdr->append (static_cast<uint8_t> (*p));
        return hdr;
    }

    // Build a Buffer::Ptr from a brace-initializer list of bytes.
    Buffer::Ptr makeData (std::initializer_list<uint8_t> bytes) const
    {
        Buffer::Ptr buf = new Buffer ();
        for (auto b : bytes)
            buf->append (b);
        return buf;
    }
};
