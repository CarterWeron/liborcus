/*************************************************************************
 *
 * Copyright (c) 2010, 2011 Kohei Yoshida
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

#include "sheet.hpp"

#include "styles.hpp"
#include "shared_strings.hpp"
#include "document.hpp"

#include "orcus/global.hpp"

#include <iostream>
#include <fstream>
#include <algorithm>
#include <sstream>
#include <vector>
#include <cassert>
#include <memory>
#include <cstdlib>

#include <mdds/mixed_type_matrix.hpp>
#include <ixion/formula.hpp>
#include <ixion/formula_result.hpp>
#include <ixion/matrix.hpp>
#include <ixion/formula_name_resolver.hpp>

using namespace std;

namespace orcus { namespace spreadsheet {

namespace {

#if 0
struct colsize_checker : public unary_function<pair<row_t, import_sheet::row_type*>, void>
{
    colsize_checker() : m_colsize(0) {}
    colsize_checker(const colsize_checker& r) : m_colsize(r.m_colsize) {}

    void operator() (const pair<row_t, import_sheet::row_type*>& r)
    {
        size_t colsize = r.second->size();
        if (colsize > m_colsize)
            m_colsize = colsize;
    }

    size_t get_colsize() const { return m_colsize; }

private:
    size_t m_colsize;
};
#endif

}

const row_t import_sheet::max_row_limit = 1048575;
const col_t import_sheet::max_col_limit = 1023;

import_sheet::import_sheet(document& doc, sheet_t sheet) :
    m_doc(doc), m_max_row(0), m_max_col(0), m_sheet(sheet)
{
}

import_sheet::~import_sheet()
{
    for_each(m_cell_formats.begin(), m_cell_formats.end(),
             map_object_deleter<cell_format_type>());
}

void import_sheet::set_auto(row_t row, col_t col, const char* p, size_t n)
{
    if (!p || !n)
        return;

    ixion::model_context& cxt = m_doc.get_model_context();

    // First, see if this can be parsed as a number.
    char* endptr = NULL;
    double val = strtod(p, &endptr);
    const char* endptr_check = p + n;
    if (endptr == endptr_check)
        // Treat this as a numeric value.
        cxt.set_numeric_cell(ixion::abs_address_t(m_sheet,row,col), val);
    else
        // Treat this as a string value.
        cxt.set_string_cell(ixion::abs_address_t(m_sheet,row,col), p, n);
}

void import_sheet::set_string(row_t row, col_t col, size_t sindex)
{
    ixion::model_context& cxt = m_doc.get_model_context();
    cxt.set_string_cell(ixion::abs_address_t(m_sheet,row,col), sindex);
}

void import_sheet::set_value(row_t row, col_t col, double value)
{
    ixion::model_context& cxt = m_doc.get_model_context();
    cxt.set_numeric_cell(ixion::abs_address_t(m_sheet,row,col), value);
}

void import_sheet::set_format(row_t row, col_t col, size_t index)
{
    cell_format_type::iterator itr = m_cell_formats.find(row);
    if (itr == m_cell_formats.end())
    {
        pair<cell_format_type::iterator, bool> r =
            m_cell_formats.insert(
                cell_format_type::value_type(
                    row, new segment_col_index_type(0, max_col_limit, 0)));

        if (!r.second)
        {
            cerr << "insertion of new cell format container failed!" << endl;
            return;
        }
        itr = r.first;
    }

    segment_col_index_type& con = *itr->second;
    con.insert_back(col, col+1, index);

    update_size(row, col);
}

void import_sheet::set_formula(row_t row, col_t col, formula_grammar_t grammar,
                        const char* p, size_t n)
{
    // Tokenize the formula string and store it.
    ixion::model_context& cxt = m_doc.get_model_context();
    ixion::abs_address_t pos(m_sheet, row, col);
    cxt.set_formula_cell(pos, p, n);
    ixion::register_formula_cell(cxt, pos);
    m_doc.insert_dirty_cell(pos);
}

void import_sheet::set_shared_formula(
    row_t row, col_t col, formula_grammar_t grammar, size_t sindex,
    const char* p_formula, size_t n_formula, const char* p_range, size_t n_range)
{
    ixion::model_context& cxt = m_doc.get_model_context();
    ixion::abs_address_t pos(m_sheet, row, col);
    cxt.set_shared_formula(pos, sindex, p_formula, n_formula, p_range, n_range);
    set_shared_formula(row, col, sindex);
}

void import_sheet::set_shared_formula(row_t row, col_t col, size_t sindex)
{
    ixion::model_context& cxt = m_doc.get_model_context();
    ixion::abs_address_t pos(m_sheet, row, col);
    cxt.set_formula_cell(pos, sindex, true);
    ixion::register_formula_cell(cxt, pos);
    m_doc.insert_dirty_cell(pos);
}

void import_sheet::set_formula_result(row_t row, col_t col, const char* p, size_t n)
{
}

row_t import_sheet::row_size() const
{
    return 0;
#if 0
    if (m_rows.empty())
        return 0;

    return m_max_row + 1;
#endif
}

col_t import_sheet::col_size() const
{
    return 0;
#if 0
    if (m_rows.empty())
        return 0;

    return m_max_col + 1;
#endif
}

ixion::matrix import_sheet::get_range_value(row_t row1, col_t col1, row_t row2, col_t col2) const
{
    const ixion::model_context& cxt = m_doc.get_model_context();
    ixion::abs_range_t range;
    range.first = ixion::abs_address_t(m_sheet,row1,col1);
    range.last  = ixion::abs_address_t(m_sheet,row2,col2);
    return cxt.get_range_value(range);
}

void import_sheet::dump() const
{
    const ixion::model_context& cxt = m_doc.get_model_context();
    ixion::abs_range_t range = cxt.get_data_range(m_sheet);
    if (!range.valid())
        // Sheet is empty.  Nothing to print.
        return;

    size_t row_count = range.last.row + 1;
    size_t col_count = range.last.column + 1;
    cout << "rows: " << row_count << "  cols: " << col_count << endl;

    typedef mdds::mixed_type_matrix<string, bool> mx_type;
    mx_type mx(row_count, col_count, mdds::matrix_density_sparse_empty);

    // Put all cell values into matrix as string elements first.
    for (size_t row = 0; row < row_count; ++row)
    {
        for (size_t col = 0; col < col_count; ++col)
        {
            ixion::abs_address_t pos(m_sheet,row,col);
            switch (cxt.get_celltype(pos))
            {
                case ixion::celltype_string:
                {
                    size_t sindex = cxt.get_string_identifier(pos);
                    const string* p = cxt.get_string(sindex);
                    assert(p);
                    mx.set_string(row, col, new string(*p));
                }
                break;
                case ixion::celltype_numeric:
                {
                    ostringstream os;
                    os << cxt.get_numeric_value(pos) << " [v]";
                    mx.set_string(row, col, new string(os.str()));
                }
                break;
                case ixion::celltype_formula:
                {
                    // print the formula and the formula result.
                    const ixion::formula_cell* cell = cxt.get_formula_cell(pos);
                    assert(cell);
                    size_t index = cell->get_identifier();
                    const ixion::formula_tokens_t* t = cxt.get_formula_tokens(m_sheet, index);
                    if (t)
                    {
                        ostringstream os;
                        string formula;
                        ixion::print_formula_tokens(
                            m_doc.get_model_context(), pos, *t, formula);
                        os << formula;

                        const ixion::formula_result* res = cell->get_result_cache();
                        if (res)
                            os << " (" << res->str(m_doc.get_model_context()) << ")";

                        mx.set_string(row, col, new string(os.str()));
                    }
                }
                break;
                default:
                    ;
            }
        }
    }

    // Calculate column widths first.
    mx_type::size_pair_type sp = mx.size();
    vector<size_t> col_widths(sp.second, 0);

    for (size_t r = 0; r < sp.first; ++r)
    {
        for (size_t c = 0; c < sp.second; ++c)
        {
            if (mx.get_type(r, c) == ::mdds::element_empty)
                continue;

            const string* p = mx.get_string(r, c);
            if (col_widths[c] < p->size())
                col_widths[c] = p->size();
        }
    }

    // Create a row separator string;
    ostringstream os;
    os << '+';
    for (size_t i = 0; i < col_widths.size(); ++i)
    {
        os << '-';
        size_t cw = col_widths[i];
        for (size_t i = 0; i < cw; ++i)
            os << '-';
        os << "-+";
    }

    string sep = os.str();

    // Now print to stdout.
    cout << sep << endl;
    for (size_t r = 0; r < row_count; ++r)
    {
        cout << "|";
        for (size_t c = 0; c < col_count; ++c)
        {
            size_t cw = col_widths[c]; // column width
            if (mx.get_type(r, c) == ::mdds::element_empty)
            {
                for (size_t i = 0; i < cw; ++i)
                    cout << ' ';
                cout << "  |";
            }
            else
            {
                const string* s = mx.get_string(r, c);
                cout << ' ' << *s;
                cw -= s->size();
                for (size_t i = 0; i < cw; ++i)
                    cout << ' ';
                cout << " |";
            }
        }
        cout << endl;
        cout << sep << endl;
    }
}

void import_sheet::dump_check(ostream& os) const
{
    const ixion::model_context& cxt = m_doc.get_model_context();
    ixion::abs_range_t range = cxt.get_data_range(m_sheet);
    if (!range.valid())
        // Sheet is empty.  Nothing to print.
        return;

    size_t row_count = range.last.row + 1;
    size_t col_count = range.last.column + 1;

    for (size_t row = 0; row < row_count; ++row)
    {
        for (size_t col = 0; col < col_count; ++col)
        {
            ixion::abs_address_t pos(m_sheet, row, col);
            switch (cxt.get_celltype(pos))
            {
                case ixion::celltype_string:
                {
                    os << "row: " << row << "; column: " << col << endl;
                    os << "type: string" << endl;
                    size_t sindex = cxt.get_string_identifier(pos);
                    const string* p = cxt.get_string(sindex);
                    assert(p);
                    os << "value: '" << *p << "'" << endl;
                    os << endl;
                }
                break;
                case ixion::celltype_numeric:
                {
                    os << "row: " << row << "; column: " << col << endl;
                    os << "type: numeric" << endl;
                    os << "value: " << cxt.get_numeric_value(pos) << endl;
                    os << endl;
                }
                break;
                case ixion::celltype_formula:
                {
                    os << "row: " << row << "; column: " << col << endl;
                    os << "type: formula" << endl;
                    // print the formula and the formula result.
                    const ixion::formula_cell* cell = cxt.get_formula_cell(pos);
                    assert(cell);
                    size_t index = cell->get_identifier();
                    const ixion::formula_tokens_t* t = cxt.get_formula_tokens(m_sheet, index);
                    if (t)
                    {
                        string formula;
                        ixion::print_formula_tokens(
                            m_doc.get_model_context(), pos, *t, formula);
                        os << "expression: " << formula << endl;

                        const ixion::formula_result* res = cell->get_result_cache();
                        if (res)
                            os << "result: " << res->str(m_doc.get_model_context()) << endl;
                    }
                    os << endl;
                }
                break;
                default:
                    ;
            }
        }
    }
}

namespace {

template<typename _OSTREAM>
class html_elem
{
public:
    html_elem(_OSTREAM& strm, const char* name, const char* attr = NULL) :
        m_strm(strm), m_name(name)
    {
        if (attr)
            m_strm << '<' << m_name << " style=\"" << attr << "\">";
        else
            m_strm << '<' << m_name << '>';
    }

    ~html_elem()
    {
        m_strm << "</" << m_name << '>';
    }

private:
    _OSTREAM& m_strm;
    const char* m_name;
};

template<typename _OSTREAM>
void print_formatted_text(_OSTREAM& strm, const string& text, const import_shared_strings::format_runs_type& formats)
{
    typedef html_elem<_OSTREAM> elem;

    const char* p_span = "span";

    size_t pos = 0;
    import_shared_strings::format_runs_type::const_iterator itr = formats.begin(), itr_end = formats.end();
    for (; itr != itr_end; ++itr)
    {
        const import_shared_strings::format_run& run = *itr;
        if (pos < run.pos)
        {
            // flush unformatted text.
            strm << text;
            pos = run.pos;
        }

        if (!run.size)
            continue;

        string style = "";
        if (run.bold)
            style += "font-weight: bold;";
        if (run.italic)
            style += "font-style: italic;";

        if (!run.font.empty())
            style += "font-family: " + run.font.str() + ";";

        if (run.font_size)
        {
            ostringstream os;
            os << "font-size: " << run.font_size << "pt;";
            style += os.str();
        }

        if (style.empty())
            strm << string(&text[pos], run.size);
        else
        {
            elem span(strm, p_span, style.c_str());
            strm << string(&text[pos], run.size);
        }

        pos += run.size;
    }

    if (pos < text.size())
    {
        // flush the remaining unformatted text.
        strm << string(&text[pos], text.size() - pos);
    }
}

void build_style_string(string& str, const import_styles& styles, const import_styles::xf& fmt)
{
    ostringstream os;
    if (fmt.font)
    {
        const import_styles::font* p = styles.get_font(fmt.font);
        if (p)
        {
            if (!p->name.empty())
                os << "font-family: " << p->name << ";";
            if (p->size)
                os << "font-size: " << p->size << "pt;";
            if (p->bold)
                os << "font-weight: bold;";
            if (p->italic)
                os << "font-style: italic;";
        }
    }
    if (fmt.fill)
    {
        const import_styles::fill* p = styles.get_fill(fmt.fill);
        if (p)
        {
            if (p->pattern_type == "solid")
            {
                const import_styles::color& r = p->fg_color;
                os << "background-color: rgb(" << r.red << "," << r.green << "," << r.blue << ");";
            }
        }
    }
    str += os.str();
}

}

void import_sheet::dump_html(const string& filepath) const
{
    typedef html_elem<ofstream> elem;

    ofstream file(filepath.c_str());
    if (!file)
    {
        cerr << "failed to create file: " << filepath << endl;
        return;
    }

    const char* p_html  = "html";
    const char* p_table = "table";
    const char* p_tr    = "tr";
    const char* p_td    = "td";
    const char* p_table_attrs      = "border:1px solid rgb(220,220,220); border-collapse:collapse; ";
    const char* p_empty_cell_attrs = "border:1px solid rgb(220,220,220); color:white; ";

    const ixion::model_context& cxt = m_doc.get_model_context();
    ixion::abs_range_t range = cxt.get_data_range(m_sheet);

    {
        elem root(file, p_html);

        if (!range.valid())
            // Sheet is empty.  Nothing to print.
            return;

        const import_shared_strings* sstrings = m_doc.get_shared_strings();

        elem table(file, p_table, p_table_attrs);

        row_t row_count = range.last.row + 1;
        col_t col_count = range.last.column + 1;
        for (row_t row = 0; row < row_count; ++row)
        {
            elem tr(file, p_tr, p_table_attrs);
            for (col_t col = 0; col < col_count; ++col)
            {
                ixion::abs_address_t pos(m_sheet,row,col);

                size_t xf = get_cell_format(row, col);
                string style = p_table_attrs;
                if (xf)
                {
                    // Apply cell format.
                    import_styles* p_styles = m_doc.get_styles();
                    const import_styles::xf* fmt = p_styles->get_cell_xf(xf);
                    if (fmt)
                        build_style_string(style, *p_styles, *fmt);
                }

                ixion::celltype_t ct = cxt.get_celltype(pos);
                if (ct == ixion::celltype_empty)
                {
                    string style;
                    style += p_empty_cell_attrs;
                    elem td(file, p_td, style.c_str());
                    file << '-'; // empty cell.
                    continue;
                }

                elem td(file, p_td, style.c_str());
                ostringstream os;
                switch (ct)
                {
                    case ixion::celltype_string:
                    {
                        size_t sindex = cxt.get_string_identifier(pos);
                        const string* p = cxt.get_string(sindex);
                        assert(p);
                        const import_shared_strings::format_runs_type* pformat = sstrings->get_format_runs(sindex);
                        if (pformat)
                            print_formatted_text<ostringstream>(os, *p, *pformat);
                        else
                            os << *p;
                    }
                    break;
                    case ixion::celltype_numeric:
                        os << cxt.get_numeric_value(pos);
                    break;
                    case ixion::celltype_formula:
                    {
                        // print the formula and the formula result.
                        const ixion::formula_cell* cell = cxt.get_formula_cell(pos);
                        assert(cell);
                        size_t index = cell->get_identifier();
                        const ixion::formula_tokens_t* t = cxt.get_formula_tokens(m_sheet, index);
                        if (t)
                        {
                            ostringstream os;
                            string formula;
                            ixion::print_formula_tokens(
                                m_doc.get_model_context(), pos, *t, formula);
                            os << formula;

                            const ixion::formula_result* res = cell->get_result_cache();
                            if (res)
                                os << " (" << res->str(m_doc.get_model_context()) << ")";
                        }
                    }
                    break;
                    default:
                        ;
                }

                file << os.str();
            }
        }
    }
}

void import_sheet::update_size(row_t row, col_t col)
{
    if (m_max_row < row)
        m_max_row = row;
    if (m_max_col < col)
        m_max_col = col;
}

size_t import_sheet::get_cell_format(row_t row, col_t col) const
{
    cell_format_type::const_iterator itr = m_cell_formats.find(row);
    if (itr == m_cell_formats.end())
        return 0;

    segment_col_index_type& con = *itr->second;
    if (!con.is_tree_valid())
        con.build_tree();

    size_t index;
    if (!con.search_tree(col, index))
        return 0;

    return index;
}

}}
