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

#ifndef __ORCUS_MODEL_DOCUMENT_HPP__
#define __ORCUS_MODEL_DOCUMENT_HPP__

#include "orcus/model/interface.hpp"
#include "orcus/model/sheet.hpp"
#include "orcus/pstring.hpp"

#include <ixion/types.hpp>

#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/noncopyable.hpp>

namespace orcus { namespace model {

class shared_strings;
class styles;
class formula_context;

/**
 * Internal document representation used only for testing the filters.
 */
class document : private ::boost::noncopyable
{
    friend class sheet;

    /**
     * Single sheet entry which consists of a sheet name and a sheet data.
     * Use the printer function object to print sheet content with for_each
     * function.
     */
    struct sheet_item : private ::boost::noncopyable
    {
        pstring name;
        sheet   data;
        sheet_item(document& doc, const pstring& _name, sheet_t sheet);

        struct printer : public ::std::unary_function<sheet_item, void>
        {
            void operator() (const sheet_item& item) const;
        };

        struct html_printer : public ::std::unary_function<sheet_item, void>
        {
            html_printer(const ::std::string& filepath);
            void operator() (const sheet_item& item) const;
        private:
            const ::std::string& m_filepath;
        };
    };

public:
    document();
    ~document();

    ixion::base_cell* get_cell(const ixion::abs_address_t& addr);
    const ixion::base_cell* get_cell(const ixion::abs_address_t& addr) const;
    void get_cells(const ixion::abs_range_t& range, std::vector<const ixion::base_cell*>& cells) const;

    ixion::abs_address_t get_cell_position(const ixion::base_cell* p) const;
    const ixion::formula_tokens_t* get_formula_tokens(sheet_t sheet_id, size_t identifier) const;

    shared_strings* get_shared_strings();
    const shared_strings* get_shared_strings() const;

    styles* get_styles();
    const styles* get_styles() const;

    formula_context& get_formula_context();
    const formula_context& get_formula_context() const;

    sheet* append_sheet(const pstring& sheet_name);

    void calc_formulas();

    /**
     * Dump document content to stdout for debugging.
     */
    void dump() const;

    /**
     * File name should not contain an extension.  The final name will be
     * [filename] + _ + [sheet name] + .html.
     *
     * @param filename base file name
     */
    void dump_html(const ::std::string& filename) const;

private:
    const ixion::base_cell* get_cell_from_sheets(const ixion::abs_address_t& addr) const;
    void insert_dirty_cell(ixion::formula_cell* cell);

private:
    ::boost::ptr_vector<sheet_item> m_sheets;
    shared_strings* mp_strings;
    styles* mp_styles;
    formula_context* mp_formula_cxt;
    ixion::dirty_cells_t m_dirty_cells;
};

}}

#endif
