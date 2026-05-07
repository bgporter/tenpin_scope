
#include <juce_core/juce_core.h>

class Test_Buffer : public TestSuite
{
public:
    Test_Buffer ()
    : TestSuite ("Buffer", "utility")
    {
    }

    void runTest () override
    {
        beginTest ("Buffer");

        test ("default constructed is empty",
              [this] ()
              {
                  Buffer::Ptr buf { new Buffer () };
                  expect (buf->empty ());
                  expectEquals (static_cast<int> (buf->size ()), 0);
              });

        test ("single-byte append",
              [this] ()
              {
                  Buffer::Ptr buf { new Buffer () };
                  buf->append (0xAB);
                  expect (!buf->empty ());
                  expectEquals (static_cast<int> (buf->size ()), 1);
                  expectEquals (static_cast<int> ((*buf)[0]), static_cast<int> (0xAB));
              });

        test ("multi-byte append",
              [this] ()
              {
                  const uint8_t data[] = { 0x01, 0x02, 0x03, 0x04 };
                  Buffer::Ptr buf { new Buffer () };
                  buf->append (data, std::size (data));
                  expectEquals (static_cast<int> (buf->size ()), 4);
                  for (int i = 0; i < 4; ++i)
                      expectEquals (static_cast<int> ((*buf)[i]), i + 1);
              });

        test ("iterators",
              [this] ()
              {
                  const uint8_t data[] = { 10, 20, 30 };
                  Buffer::Ptr buf { new Buffer () };
                  buf->append (data, std::size (data));

                  int sum = 0;
                  for (auto b : *buf)
                      sum += b;
                  expectEquals (sum, 60);

                  sum = 0;
                  for (auto it = buf->cbegin (); it != buf->cend (); ++it)
                      sum += *it;
                  expectEquals (sum, 60);
              });

        test ("VariantConverter round-trip",
              [this] ()
              {
                  Buffer::Ptr original { new Buffer () };
                  original->append (0x11);
                  original->append (0x22);
                  original->append (0x33);

                  const juce::var v { juce::VariantConverter<Buffer::Ptr>::toVar (original) };
                  Buffer::Ptr recovered { juce::VariantConverter<Buffer::Ptr>::fromVar (v) };

                  expect (recovered != nullptr);
                  expectEquals (static_cast<int> (recovered->size ()), 3);
                  expectEquals (static_cast<int> ((*recovered)[0]), static_cast<int> (0x11));
                  expectEquals (static_cast<int> ((*recovered)[1]), static_cast<int> (0x22));
                  expectEquals (static_cast<int> ((*recovered)[2]), static_cast<int> (0x33));
              });

        test ("null Ptr round-trips cleanly",
              [this] ()
              {
                  Buffer::Ptr nullBuf;
                  const juce::var v { juce::VariantConverter<Buffer::Ptr>::toVar (nullBuf) };
                  Buffer::Ptr recovered { juce::VariantConverter<Buffer::Ptr>::fromVar (v) };
                  expect (recovered == nullptr);
              });
    }
};

static Test_Buffer testBuffer;
