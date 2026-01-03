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

#include <JuceHeader.h>

#include "propertyList.h"
#include "variantConverters.h"

class LogWriter
{
public:
    enum class Level
    {
        error, ///< error should always be logged.
        warn,
        info,
        debug,
        trace ///< trace is super-low level, probably only used in development
    };

    /**
     * @brief Initialize the logger. Your code should do this before executing
     * anything that might want to log something, so we know where to write the
     * log file!
     *
     * @param theLogFile
     * @param flushInterval number of seconds to wait before flushing pending events.
     * @param maxPendingEvents maximum number of events to accumulate before flushing.
     */
    static void init (const juce::File& theLogFile, int flushInterval = 2, int maxPendingEvents = 20);
    static void setLogLevel (Level theLogLevel);

    using ErrorLogCallback = std::function<void (const PropertyList&)>;
    /**
     * @brief We'd like to be able to e.g. ensure that errors reported in the app
     * code are also sent to the analytics back end for early detection. The Analytics
     * module (or any other code, but only 1 callback can be registered) can pass
     * us a callback function that will be invoked when we log an error.
     *
     * @param cb callback function.
     */
    static void setErrorCallback (ErrorLogCallback cb);

    /**
     * @brief write any pending events into the log file.
     *
     */
    static void flush ();

    /**
     * @brief stop the logger's operation; because we use a static object that
     * outlives the rest of the application, we need to do this manually as
     * part of the app's shutdown code.
     */
    static void shutdown ();

    /**
     * @brief do-nothing methods used to disable TRACE logging, see the
     * macros at the bottom of this file.
     */
    static void noOp (const juce::String&) {}
    static void noOp (const PropertyList&) {}

    /**
     * @brief function to add a log event to the queue.
     *
     * @param eventName
     * @param properties
     */
    static void log (juce::StringRef eventName, const PropertyList& properties);

    /**
     * @brief log analytics data to the file, at INFO level.
     *
     * @param logProps
     * @param analyticsProps
     */
    static void log (const PropertyList& logProps, const PropertyList& analyticsProps);

    static void log (Level level, const PropertyList& properties, const juce::String& file = {},
                     const juce::String& fn = {}, int line = 0);

    static void log (Level level, const juce::String& msg, const juce::String& file = {}, const juce::String& fn = {},
                     int line = 0);

    /**
     * @brief (temporarily) disable logging. This is useful for unit tests, where
     * the logger has not been initialized, so attempting to log anything will
     * result in assertions/failures. Make sure to re-enable logging when you're done!
     *
     * @param enable
     */
    static void enableLogging (bool enable);

private:
    LogWriter ();
    ~LogWriter ()
    {
        // make sure that the delegate/impl are shut down?
    }
    LogWriter (const LogWriter&)            = delete;
    LogWriter& operator= (const LogWriter&) = delete;
    LogWriter (LogWriter&&)                 = delete;
    LogWriter& operator= (LogWriter&&)      = delete;

    class Impl;
    class Delegate
    {
    public:
        /**
         * @brief Construct a new Delegate object, creating the pointer to the
         * implementation object.
         *
         */
        Delegate ();

        ~Delegate ();

        void enableLogging (bool isEnabled);
        bool isLoggingEnabled () const;
        void addEvent (const juce::String& event);
        void init (const juce::File& theLogFile, int flushInterval = 2, int maxPendingEvents = 20);
        void setLogLevel (Level theLogLevel);
        Level getLogLevel () const;
        void setErrorCallback (ErrorLogCallback cb);
        void handleError (const PropertyList& props);
        void flush ();
        void shutdown ();

    private:
        std::unique_ptr<LogWriter::Impl> pImpl;
        ErrorLogCallback errorCallback { nullptr };
    };

    /**
     * @brief The LogWriter has a single instance of the Delegate class, created
     * as a Meyers singleton.
     *
     * @return Delegate&
     */
    static Delegate& getDelegate ();
};

// Macros to simplify calls to the logger, inserting file/fn/line number of the call.
// These are variadic macros and can be used to invoke LogWriter::log either with
// a single string argument, or with a property list to carry multiple pieces of
// data into the log call.

/**
 * @brief Log an error. Use this (at least!) any time you would use an assertion
 * in the code. Eventually we'll be sure that logged errors are also pushed
 * to analytics.
 * NOTE that this one is enclosed in {curly braces} so that it will work correctly if
 * the macro is in a simple if() statement like
 * ```
 * if (someCondition)
 *     ERROR("Error message!");
 * ```
 */
#define ERROR_(...)                                                                          \
    {                                                                                        \
        jassertfalse;                                                                        \
        LogWriter::log (LogWriter::Level::error, __VA_ARGS__, __FILE__, __func__, __LINE__); \
    }

/**
 * @brief Log a warning -- the app can continue, but something is not as expected
 * and it would be useful to see in a log file in the event of problems.
 */
#define WARN_(...) LogWriter::log (LogWriter::Level::warn, __VA_ARGS__, __FILE__, __func__, __LINE__)

/**
 * @brief Log a warning only once. This should help in situations where logging every instance
 * of a warning would spam the log file for no good reason.
 */
#define WARN_ONCE_(...)                                                                         \
    {                                                                                           \
        static bool hasLogged { false };                                                        \
        if (!hasLogged)                                                                         \
        {                                                                                       \
            LogWriter::log (LogWriter::Level::warn, __VA_ARGS__, __FILE__, __func__, __LINE__); \
            hasLogged = true;                                                                   \
        }                                                                                       \
    }

/**
 * @brief The default logging level to use: this is something you want to be sure that
 * we include in the log files.
 */
#define INFO_(...) LogWriter::log (LogWriter::Level::info, __VA_ARGS__, __FILE__, __func__, __LINE__)

/**
 * @brief Log useful information for development/debug purposes that is too low-level
 * to be included by default in users' log files.
 */
#define DEBUG_(...) LogWriter::log (LogWriter::Level::debug, __VA_ARGS__, __FILE__, __func__, __LINE__)

/**
 * @brief super low-level logging during development. To leave TRACE_() calls in place
 * but suppress them from actually doing anything, you can (on a per-file basis)
 * control this by including a line like
 * `#define DISABLE_TRACE 1`
 * before including this file, which will turn off all TRACE_() calls in that file.
 */
#ifndef DISABLE_TRACE
#define DISABLE_TRACE 0
#endif

#undef TRACE_

#if DISABLE_TRACE > 0
#define TRACE_(...) LogWriter::noOp (__VA_ARGS__)
#else
#define TRACE_(...) LogWriter::log (LogWriter::Level::trace, __VA_ARGS__, __FILE__, __func__, __LINE__)
#endif

#undef DISABLE_TRACE
