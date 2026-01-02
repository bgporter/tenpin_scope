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

#include "logger.h"

namespace
{
const juce::Identifier fileId { "file" };
const juce::Identifier functionId { "fn" };
const juce::Identifier lineId { "line" };
const juce::Identifier messageId { "msg" };

juce::int64 getCurrentTimestamp ()
{
    return juce::Time::getCurrentTime ().toMilliseconds ();
}

juce::String cleanEventName (const juce::String& eventName)
{
    // analytics events can (and will) contain internal spaces; this will
    // convert those back into valid identifiers. We *shouldn't* need any
    // more stringent cleanup than this.
    return eventName.replace (" ", "_");
}

juce::String getLevelName (LogWriter::Level level)
{
    switch (level)
    {
        case LogWriter::Level::error:
            return "ERROR";
        case LogWriter::Level::warn:
            return "WARN";
        case LogWriter::Level::info:
            return "INFO";
        case LogWriter::Level::debug:
            return "DEBUG";
        case LogWriter::Level::trace:
            return "TRACE";
    }
}

void trimLogFile (juce::File logFile, int maxLines)
{
    juce::StringArray lines;
    logFile.readLines (lines);
    const auto lineCount { lines.size () };
    if (lineCount <= maxLines)
        return;

    // trim excess lines from the beginning of the list
    lines.removeRange (0, lineCount - maxLines);
    logFile.replaceWithText (lines.joinIntoString (juce::newLine));
}
class LogEvent
{
public:
    LogEvent (const juce::String& name, const PropertyList& eventProps)
    : timestamp { getCurrentTimestamp () }
    , event { name }
    , properties { eventProps }
    {
    }
    void appendProperties (const PropertyList& moreProps) { properties.append (moreProps); }

    juce::String toString () const
    {
        juce::String s;
        s << timestamp << " " << cleanEventName (event);
        for (const auto& [key, value] : properties)
        {
            // strings are written inside quotes so that each line may be parsed
            // easily. Numeric (etc) values are not quoted.
            const auto optionalQuote { value.isString () ? "\"" : "" };
            s << " " << key << "=" << optionalQuote << value.toString () << optionalQuote;
        }
        return s;
    }

private:
    juce::int64 timestamp;
    juce::String event;
    PropertyList properties;
};
} // namespace

class LogWriter::Impl : public juce::Timer
{
public:
    Impl () = default;
    ~Impl () override {}

    void enableLogging (bool isEnabled)
    {
        const juce::ScopedLock lock { mutex };
        loggingEnabled = isEnabled;
    }

    bool isLoggingEnabled () const
    {
        const juce::ScopedLock lock { mutex };
        return loggingEnabled;
    }

    void addEvent (const juce::String& event)
    {
        const juce::ScopedLock lock { mutex };
        if (!loggingEnabled)
            return;

        DBG (event);
        pendingEvents.add (event);
        if (pendingEvents.size () >= maxEventCount)
            flush ();
    }

    void init (const juce::File& theLogFile, int flushInterval, int maxPendingEvents)
    {
        // if the log file doesn't already exist, create an empty one.
        theLogFile.create ();
        // ...and if it did exist, keep it to a reasonable size
        trimLogFile (theLogFile, 20000);

        // we only store the path, opening/closing the file each time we write to it.
        logFilePath   = theLogFile.getFullPathName ();
        maxEventCount = std::max (1, maxPendingEvents);
        startTimer (flushInterval * 1000);
    }

    void setLogLevel (Level theLogLevel)
    {
        const juce::ScopedLock lock { mutex };
        logLevel = theLogLevel;
    }

    LogWriter::Level getLogLevel () const
    {
        const juce::ScopedLock lock { mutex };
        return logLevel;
    }

    void shutdown ()
    {
        enableLogging (false);
        stopTimer ();
        flush ();
    }

    void flush ()
    {
        auto logFile { juce::File { logFilePath } };
        if (!logFile.existsAsFile ())
        {
            // !!! get this error to analytics!
            jassertfalse;
            return;
        }

        const juce::ScopedLock lock { mutex };
        for (const auto& line : pendingEvents)
        {
            logFile.appendText (line + "\n");
        }
        pendingEvents.clear ();
    }

    void timerCallback () override { flush (); }

private:
    bool loggingEnabled { true };
    juce::CriticalSection mutex;
    juce::String logFilePath;
    Level logLevel { Level::info };
    int maxEventCount { 10 };
    juce::StringArray pendingEvents;
};

//
////////////////////////////////////////////////////////////////////////////////
//

LogWriter::LogWriter ()
{
#if JUCE_DEBUG
    // debug builds always log everything.
    setLogLevel (Level::trace);
#endif
}

LogWriter::Delegate& LogWriter::getDelegate ()
{
    static LogWriter::Delegate delegate;
    return delegate;
}

void LogWriter::init (const juce::File& theLogFile, int flushInterval, int maxPendingEvents)
{
    // if the log file doesn't already exist, create an empty one.
    theLogFile.create ();
    // ...and if it did exist, keep it to a reasonable size
    trimLogFile (theLogFile, 20000);

    auto& delegate { getDelegate () };
    delegate.init (theLogFile, flushInterval, maxPendingEvents);
    TRACE_ ("Logger is initialized!");
}

void LogWriter::setLogLevel (Level theLogLevel)
{
    auto& delegate { getDelegate () };
    delegate.setLogLevel (theLogLevel);
}

void LogWriter::setErrorCallback (ErrorLogCallback cb)
{
    auto& delegate { getDelegate () };
    delegate.setErrorCallback (cb);
}

void LogWriter::flush ()
{
    auto& delegate { getDelegate () };
    delegate.flush ();
}

void LogWriter::shutdown ()
{
    auto& delegate { getDelegate () };
    delegate.shutdown ();
}

void LogWriter::log (juce::StringRef eventName, const PropertyList& properties)
{
    LogEvent e { eventName, properties };
    auto& delegate { getDelegate () };
    delegate.addEvent (e.toString ());
}

void LogWriter::log (const PropertyList& logProps, const PropertyList& analyticsProps)
{
    LogEvent e { getLevelName (LogWriter::Level::info), logProps };
    e.appendProperties (analyticsProps);
    auto& delegate { getDelegate () };
    delegate.addEvent (e.toString ());
}

void LogWriter::log (Level level, const PropertyList& properties, const juce::String& file, const juce::String& fn,
                     int line)
{
    auto& delegate { getDelegate () };
    if (level > delegate.getLogLevel ())
        return;

    PropertyList logProperties { properties };
    if (file.isNotEmpty ())
    {
        const auto f { juce::File::createFileWithoutCheckingPath (file) };
        logProperties.append ({ fileId, f.getFileNameWithoutExtension () });
    }
    if (fn.isNotEmpty ())
        logProperties.append ({ functionId, fn });
    if (line > 0)
        logProperties.append ({ lineId, line });

    log (getLevelName (level), logProperties);
    // if an error callback has been set, the delegate will invoke it.
    if (level == LogWriter::Level::error)
        delegate.handleError (properties);
}

void LogWriter::log (Level level, const juce::String& msg, const juce::String& file, const juce::String& fn, int line)
{
    // clang-format off
    log (level, { { messageId, msg } }, file, fn, line);
    // clang-format on
}
void LogWriter::enableLogging (bool enable)
{
    auto& delegate { getDelegate () };
    delegate.enableLogging (enable);
}

LogWriter::Delegate::Delegate ()
: pImpl { std::make_unique<LogWriter::Impl> () }
{
}

LogWriter::Delegate::~Delegate ()
{
    if (pImpl != nullptr)
    {
        // You didn't call shutdown() before existing the app!
        jassertfalse;
        pImpl->shutdown ();
        pImpl = nullptr;
    }
}

void LogWriter::Delegate::enableLogging (bool isEnabled)
{
    if (pImpl != nullptr)
        pImpl->enableLogging (isEnabled);
}

bool LogWriter::Delegate::isLoggingEnabled () const
{
    return (pImpl != nullptr && pImpl->isLoggingEnabled ());
}

void LogWriter::Delegate::addEvent (const juce::String& event)
{
    if (isLoggingEnabled ())
        pImpl->addEvent (event);
}

void LogWriter::Delegate::init (const juce::File& theLogFile, int flushInterval, int maxPendingEvents)
{
    if (pImpl == nullptr)
    {
        jassertfalse;
        return;
    }
    pImpl->init (theLogFile, flushInterval, maxPendingEvents);
}

void LogWriter::Delegate::setLogLevel (Level theLogLevel)
{
    if (pImpl == nullptr)
    {
        jassertfalse;
        return;
    }
    pImpl->setLogLevel (theLogLevel);
}

LogWriter::Level LogWriter::Delegate::getLogLevel () const
{
    if (pImpl == nullptr)
    {
        jassertfalse;
        return Level::error;
    }
    return pImpl->getLogLevel ();
}

void LogWriter::Delegate::setErrorCallback (ErrorLogCallback cb)
{
    errorCallback = cb;
}

void LogWriter::Delegate::handleError (const PropertyList& props)
{
    if (errorCallback != nullptr)
        errorCallback (props);
}

void LogWriter::Delegate::flush ()
{
    if (isLoggingEnabled ())
        pImpl->flush ();
}
void LogWriter::Delegate::shutdown ()
{
    if (pImpl == nullptr)
    {
        // shutting down twice?
        jassertfalse;
        return;
    }
    pImpl->shutdown ();
    pImpl = nullptr;
}
