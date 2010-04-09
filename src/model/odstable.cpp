/*************************************************************************
 *
 * Copyright (c) 2010 Kohei Yoshida
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

#include "model/odstable.hpp"
#include "global.hpp"

#include <iostream>
#include <algorithm>

using namespace std;

namespace orcus { namespace model {

namespace {

struct row_deleter : public unary_function<pair<row_t, ods_table::row_type*>, void>
{
    void operator() (const pair<row_t, ods_table::row_type*>& r)
    {
        delete r.second;
    }
};

struct colsize_checker : public unary_function<pair<row_t, ods_table::row_type*>, void>
{
    colsize_checker() : m_colsize(0) {}
    colsize_checker(const colsize_checker& r) : m_colsize(r.m_colsize) {}

    void operator() (const pair<row_t, ods_table::row_type*>& r)
    {
        size_t colsize = r.second->size();
        if (colsize > m_colsize)
            m_colsize = colsize;
    }

    size_t get_colsize() const { return m_colsize; }

private:
    size_t m_colsize;
};

}

ods_table::ods_table(const string& name) :
    m_name(name)
{
}

ods_table::~ods_table()
{
    for_each(m_sheet.begin(), m_sheet.end(), row_deleter());
}

const string& ods_table::get_name() const
{
    return m_name;
}

void ods_table::set_cell(row_t row, col_t col, const string& val)
{
    sheet_type::iterator itr = m_sheet.find(row);
    if (itr == m_sheet.end())
    {
        // This row doesn't exist yet.  Create it.
        pair<sheet_type::iterator, bool> r = m_sheet.insert(sheet_type::value_type(row, new row_type));
        if (!r.second)
            throw general_error("failed to insert a new row instance.");
        itr = r.first;
    }

    row_type* p = itr->second;
    p->insert(row_type::value_type(col, val));
}

string ods_table::get_cell(row_t row, col_t col) const
{
    sheet_type::const_iterator itr = m_sheet.find(row);
    if (itr == m_sheet.end())
        return string();

    row_type* p = itr->second;
    row_type::const_iterator itr_cell = p->find(col);
    if (itr_cell == p->end())
        return string();

    return itr_cell->second;
}

size_t ods_table::row_size() const
{
    return m_sheet.size();
}

size_t ods_table::col_size() const
{
    return for_each(m_sheet.begin(), m_sheet.end(), colsize_checker()).get_colsize();
}

}}
