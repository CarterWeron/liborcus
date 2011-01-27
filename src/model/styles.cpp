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

#include "orcus/model/styles.hpp"

namespace orcus { namespace model {

styles::font::font() :
    size(0.0), bold(false), italic(false)
{
}

void styles::font::reset()
{
    name.clear();
    size = 0.0;
    bold = false;
    italic = false;
}

styles::styles()
{
}

styles::~styles()
{
}

void styles::set_font_count(size_t n)
{
    m_fonts.reserve(n);
}

void styles::commit_font()
{
    m_fonts.push_back(m_cur_font);
    m_cur_font.reset();
}

void styles::set_font_bold(bool b)
{
    m_cur_font.bold = b;
}

void styles::set_font_italic(bool b)
{
    m_cur_font.italic = b;
}

void styles::set_font_name(const char* s, size_t n)
{
    m_cur_font.name = pstring(s, n).intern();
}

void styles::set_font_size(double point)
{
    m_cur_font.size = point;
}

}}
