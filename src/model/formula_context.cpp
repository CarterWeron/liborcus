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

#include "orcus/model/formula_context.hpp"
#include "orcus/model/document.hpp"
#include "orcus/global.hpp"

#include <ixion/config.hpp>
#include <ixion/formula_name_resolver.hpp>
#include <ixion/matrix.hpp>

#include <sstream>

namespace orcus { namespace model {

formula_context::formula_context(document& doc) :
    m_doc(doc),
    mp_config(new ixion::config),
    mp_name_resolver(new ixion::formula_name_resolver_a1) {}

formula_context::~formula_context()
{
    delete mp_config;
    delete mp_name_resolver;
}

const ixion::config& formula_context::get_config() const
{
    return *mp_config;
}

const ixion::formula_name_resolver& formula_context::get_name_resolver() const
{
    return *mp_name_resolver;
}

const ixion::base_cell* formula_context::get_cell(const ixion::abs_address_t& addr) const
{
    return m_doc.get_cell(addr);
}

ixion::base_cell* formula_context::get_cell(const ixion::abs_address_t& addr)
{
    return m_doc.get_cell(addr);
}

ixion::interface::cells_in_range*
formula_context::get_cells_in_range(const ixion::abs_range_t& range)
{
    return new cells_in_range(range, m_doc);
}

ixion::interface::const_cells_in_range*
formula_context::get_cells_in_range(const ixion::abs_range_t& range) const
{
    return new const_cells_in_range(range, m_doc);
}

std::string formula_context::get_cell_name(const ixion::base_cell* p) const
{
    ixion::abs_address_t pos = get_cell_position(p);
    return get_name_resolver().get_name(pos);
}

ixion::abs_address_t formula_context::get_cell_position(const ixion::base_cell* p) const
{
    return m_doc.get_cell_position(p);
}

const ixion::formula_cell* formula_context::get_named_expression(const std::string& name) const
{
    std::ostringstream os;
    os << "formula_context::get_named_expression not implemented!";
    os << " (name: " << name << ")";
    throw general_error(os.str());
    return NULL;
}

const std::string* formula_context::get_named_expression_name(const ixion::formula_cell* expr) const
{
    throw general_error("formula_context::get_named_expression_name not implemented!");
    return NULL;
}

ixion::matrix formula_context::get_range_value(const ixion::abs_range_t& range) const
{
    return m_doc.get_range_value(range);
}

ixion::interface::session_handler* formula_context::get_session_handler() const
{
    return NULL;
}

const ixion::formula_tokens_t* formula_context::get_formula_tokens(ixion::sheet_t sheet, size_t identifier) const
{
    return m_doc.get_formula_tokens(sheet, identifier, false);
}

size_t formula_context::add_formula_tokens(ixion::sheet_t sheet, ixion::formula_tokens_t* p)
{
    throw general_error("formula_context::add_formula_tokens not implemented!");
    return 0;
}

void formula_context::remove_formula_tokens(ixion::sheet_t sheet, size_t identifier)
{
    throw general_error("formula_context::remove_formula_tokens not implemented!");
}

const ixion::formula_tokens_t* formula_context::get_shared_formula_tokens(ixion::sheet_t sheet, size_t identifier) const
{
    return m_doc.get_formula_tokens(sheet, identifier, true);
}

size_t formula_context::set_formula_tokens_shared(ixion::sheet_t sheet, size_t identifier)
{
    throw general_error("formula_context::set_formula_tokens_shared not implemented!");
}

ixion::abs_range_t formula_context::get_shared_formula_range(ixion::sheet_t sheet, size_t identifier) const
{
    throw general_error("formula_context::get_shared_formula_range not implemented!");
}

void formula_context::set_shared_formula_range(ixion::sheet_t sheet, size_t identifier, const ixion::abs_range_t& range)
{
    throw general_error("formula_context::set_shared_formula_range not implemented!");
}

size_t formula_context::add_string(const char* p, size_t n)
{
    throw general_error("formula_context::add_string not implemented!");
    return 0;
}

const std::string* formula_context::get_string(size_t identifier) const
{
    throw general_error("formula_context::get_string not implemented!");
    return NULL;
}

class cells_in_range_impl
{
    typedef std::vector<const ixion::base_cell*> cells_type;
    cells_type m_cells;
    cells_type::const_iterator m_beg;
    cells_type::const_iterator m_end;
    cells_type::const_iterator m_cur;
public:
    cells_in_range_impl(const ixion::abs_range_t& range, const document& doc)
    {
        doc.get_cells(range, m_cells);
        m_beg = m_cells.begin();
        m_end = m_cells.end();
    }

    const ixion::base_cell* first()
    {
        m_cur = m_beg;
        return m_cur == m_end ? NULL : *m_cur;
    }

    const ixion::base_cell* next()
    {
        ++m_cur;
        return m_cur == m_end ? NULL : *m_cur;
    }
};

cells_in_range::cells_in_range(const ixion::abs_range_t& range, const document& doc) :
    mp_impl(new cells_in_range_impl(range, doc)) {}

cells_in_range::~cells_in_range()
{
    delete mp_impl;
}

ixion::base_cell* cells_in_range::first()
{
    return const_cast<ixion::base_cell*>(mp_impl->first());
}

ixion::base_cell* cells_in_range::next()
{
    return const_cast<ixion::base_cell*>(mp_impl->next());
}

const_cells_in_range::const_cells_in_range(const ixion::abs_range_t& range, const document& doc) :
    mp_impl(new cells_in_range_impl(range, doc)) {}

const_cells_in_range::~const_cells_in_range()
{
    delete mp_impl;
}

const ixion::base_cell* const_cells_in_range::first()
{
    return mp_impl->first();
}

const ixion::base_cell* const_cells_in_range::next()
{
    return mp_impl->next();
}

}}

