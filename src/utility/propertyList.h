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

#include <vector>

/**
    @class PropertyList
    @brief manages a list of (key, value) pairs used to send analytics properties.
 */
class PropertyList
{
public:
    using Property = std::pair<juce::Identifier, juce::var>;

    PropertyList (std::initializer_list<Property> props)
    : properties { props }
    {
    }

    using dataType       = std::vector<Property>;
    using iterator       = dataType::iterator;
    using const_iterator = dataType::const_iterator;

    iterator begin () { return properties.begin (); }
    iterator end () { return properties.end (); }
    const_iterator begin () const { return properties.begin (); }
    const_iterator end () const { return properties.end (); }

    /**
     *  @brief Append a new property to the end of the list
        @return false if we're trying to add more properties than MixPanel supports (255 total)
     */
    bool append (const Property& property)
    {
        if (properties.size () >= 255)
            return false;
        properties.push_back (property);
        return true;
    }

    bool append (const PropertyList& other)
    {
        if (properties.size () + other.properties.size () >= 255)
            return false;

        properties.insert (properties.end (), other.properties.begin (), other.properties.end ());
        return true;
    }

private:
    std::vector<Property> properties;
};