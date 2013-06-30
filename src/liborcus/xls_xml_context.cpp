/*************************************************************************
 *
 * Copyright (c) 2013 Kohei Yoshida
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

#include "xls_xml_context.hpp"
#include "xls_xml_namespace_types.hpp"
#include "xls_xml_token_constants.hpp"

#include <iostream>

using namespace std;

namespace orcus {

namespace {

class sheet_attr_parser : public unary_function<xml_token_attr_t, void>
{
    pstring m_name;
public:
    void operator() (const xml_token_attr_t& attr)
    {
        if (attr.ns == NS_xls_xml_ss)
        {
            switch (attr.name)
            {
                case XML_Name:
                    m_name = attr.value;
                break;
                default:
                    ;
            }
        }
    }

    pstring get_name() const { return m_name; }
};

}

xls_xml_context::xls_xml_context(session_context& session_cxt, const tokens& tokens, spreadsheet::iface::import_factory* factory) :
    xml_context_base(session_cxt, tokens),
    mp_factory(factory)
{
}

xls_xml_context::~xls_xml_context()
{
}

bool xls_xml_context::can_handle_element(xmlns_id_t ns, xml_token_t name) const
{
    return true;
}

xml_context_base* xls_xml_context::create_child_context(xmlns_id_t ns, xml_token_t name)
{
    return NULL;
}

void xls_xml_context::end_child_context(xmlns_id_t ns, xml_token_t name, xml_context_base* child)
{
}

void xls_xml_context::start_element(xmlns_id_t ns, xml_token_t name, const xml_attrs_t& attrs)
{
    xml_token_pair_t parent = push_stack(ns, name);
    if (ns == NS_xls_xml_ss)
    {
        switch (name)
        {
            case XML_Workbook:
                // Do nothing.
            break;
            case XML_Worksheet:
            {
                xml_element_expected(parent, NS_xls_xml_ss, XML_Workbook);
                pstring sheet_name = for_each(attrs.begin(), attrs.end(), sheet_attr_parser()).get_name();
                cout << "sheet name: " << sheet_name << endl;
            }
            break;
            case XML_Table:
                xml_element_expected(parent, NS_xls_xml_ss, XML_Worksheet);
            break;
            case XML_Row:
                xml_element_expected(parent, NS_xls_xml_ss, XML_Table);
            break;
            case XML_Cell:
                xml_element_expected(parent, NS_xls_xml_ss, XML_Row);
            break;
            case XML_Data:
                xml_element_expected(parent, NS_xls_xml_ss, XML_Cell);
            break;
            default:
                warn_unhandled();
        }
    }
    else
        warn_unhandled();
}

bool xls_xml_context::end_element(xmlns_id_t ns, xml_token_t name)
{
    return pop_stack(ns, name);
}

void xls_xml_context::characters(const pstring& str)
{
}

}