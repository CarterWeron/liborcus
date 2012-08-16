/*************************************************************************
 *
 * Copyright (c) 2012 Kohei Yoshida
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 ************************************************************************/

#ifndef __ORCUS_STRING_POOL_HPP__
#define __ORCUS_STRING_POOL_HPP__

#include <string>
#include <boost/unordered_set.hpp>
#include <boost/functional/hash.hpp>

namespace orcus {

class pstring;

/**
 * Implements string hash map.
 */
class string_pool
{
    struct string_hash
    {
        size_t operator() (const std::string* p) const;
    private:
        boost::hash<std::string> m_hash;
    };

    struct string_equal_to
    {
        bool operator() (const std::string* p1, const std::string* p2) const;
    private:
        std::equal_to<std::string> m_equal_to;
    };

    typedef boost::unordered_set<std::string*, string_hash, string_equal_to> string_store_type;

public:
    string_pool();
    ~string_pool();

    pstring intern(const char* str);
    pstring intern(const char* str, size_t n);
    void dump() const;
    void clear();
    size_t size() const;

private:
    string_store_type m_store;
};

}

#endif
