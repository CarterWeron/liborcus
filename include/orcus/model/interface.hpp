/*************************************************************************
 *
 * Copyright (c) 2011 Kohei Yoshida
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

#ifndef __ORCUS_MODEL_INTERFACE_HPP__
#define __ORCUS_MODEL_INTERFACE_HPP__

#include <cstdlib>

#include "orcus/model/global.hpp"

namespace orcus { namespace model {

/**
 * Interface class designed to be derived by the implementor. 
 */
class shared_strings_base
{
public:
    virtual ~shared_strings_base() = 0;

    /**
     * Append new string to the string list.  Order of insertion is important 
     * since that determines the numerical ID values of inserted strings. 
     * Note that this method assumes that the caller knows the string being 
     * appended is not yet in the pool.
     *  
     * @param s pointer to the first character of the string array.  The 
     *          string array doesn't necessary have to be null-terminated.
     * @param n length of the string. 
     *  
     * @return ID of the string just inserted. 
     */
    virtual size_t append(const char* s, size_t n) = 0;

    /**
     * Similar to the append method, it adds new string to the string pool; 
     * however, this method checks if the string being added is already in the 
     * pool before each insertion, to avoid duplicated strings. 
     * 
     * @param s pointer to the first character of the string array.  The 
     *          string array doesn't necessary have to be null-terminated.
     * @param n length of the string. 
     *  
     * @return ID of the string just inserted. 
     */
    virtual size_t add(const char* s, size_t n) = 0;
};

inline shared_strings_base::~shared_strings_base() {}

class sheet_base
{
public:
    virtual ~sheet_base() = 0;

    virtual void set_string(row_t row, col_t col, size_t sindex) = 0;
};

inline sheet_base::~sheet_base() {}

/**
 * This interface provides the filters a means to instantiate concrete
 * classes that implement the above interfaces.
 */
class factory_base
{
public:
    virtual ~factory_base() = 0;

    virtual shared_strings_base* get_shared_strings() = 0;
    virtual sheet_base* append_sheet() = 0;
};

inline factory_base::~factory_base() {}

}}

#endif
