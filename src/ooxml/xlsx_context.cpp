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

#include "ooxml/xlsx_context.hpp"
#include "global.hpp"
#include "ooxml/ooxml_token_constants.hpp"
#include "ooxml/ooxml_types.hpp"
#include "ooxml/schemas.hpp"
#include "model/sheet.hpp"

#include <iostream>
#include <fstream>
#include <cstdlib>

using namespace std;

namespace orcus {

namespace {

class root_element_attr_parser: ::std::unary_function<void, xml_attr_t>
{
public:
    root_element_attr_parser(
        const schema_t expected_schema, const xmlns_token_t expected_ns);

    virtual ~root_element_attr_parser();

    virtual void handle_other_attrs(const xml_attr_t& attr) = 0;

    root_element_attr_parser& operator= (const root_element_attr_parser& r)
    {
        m_default_ns = r.m_default_ns;
        m_expected_schema = r.m_expected_schema;
        m_expected_ns = r.m_expected_ns;
        return *this;
    }

    void operator() (const xml_attr_t& attr);
    xmlns_token_t get_default_ns() const;
private:
    xmlns_token_t m_default_ns;
    schema_t m_expected_schema;
    xmlns_token_t m_expected_ns;
};

root_element_attr_parser::root_element_attr_parser(
    const schema_t expected_schema, const xmlns_token_t expected_ns) :
    m_default_ns(XMLNS_UNKNOWN_TOKEN),
    m_expected_schema(expected_schema),
    m_expected_ns(expected_ns) {}

root_element_attr_parser::~root_element_attr_parser() {}

void root_element_attr_parser::operator() (const xml_attr_t& attr)
{
    if (attr.ns == XMLNS_UNKNOWN_TOKEN && attr.name == XML_xmlns)
    {
        if (attr.value == m_expected_schema)
            m_default_ns = m_expected_ns;
    }
    else
        handle_other_attrs(attr);
}

xmlns_token_t root_element_attr_parser::get_default_ns() const
{
    return m_default_ns;
}

class workbook_attr_parser : public root_element_attr_parser
{
public:
    workbook_attr_parser() :
        root_element_attr_parser(SCH_xlsx_main, XMLNS_xlsx) {}
    virtual ~workbook_attr_parser() {}
    virtual void handle_other_attrs(const xml_attr_t &attr) {}
};

class workbook_sheet_attr_parser : unary_function<void, xml_attr_t>
{
public:
    void operator() (const xml_attr_t& attr)
    {
        if (attr.ns == XMLNS_UNKNOWN_TOKEN && attr.name == XML_name)
            m_sheet.name = attr.value.intern();
        else if (attr.ns == XMLNS_UNKNOWN_TOKEN && attr.name == XML_sheetId)
        {
            const pstring& val = attr.value;
            if (!val.empty())
                m_sheet.id = strtoul(val.str().c_str(), NULL, 10);
        }
        else if (attr.ns == XMLNS_r && attr.name == XML_id)
        {
            m_sheet.rid = attr.value.intern();
        }
    }

    const xlsx_workbook_context::sheet get_sheet() const { return m_sheet; }

private:
    xlsx_workbook_context::sheet m_sheet;
};

}

xlsx_workbook_context::sheet::sheet() : id(0) {}

xlsx_workbook_context::xlsx_workbook_context(const tokens& tokens) :
    xml_context_base(tokens) {}

xlsx_workbook_context::~xlsx_workbook_context() {}

bool xlsx_workbook_context::can_handle_element(xmlns_token_t /*ns*/, xml_token_t /*name*/) const
{
    return true;
}

xml_context_base* xlsx_workbook_context::create_child_context(xmlns_token_t /*ns*/, xml_token_t /*name*/) const
{
    return NULL;
}

void xlsx_workbook_context::end_child_context(xmlns_token_t /*ns*/, xml_token_t /*name*/, xml_context_base* /*child*/)
{
}

void xlsx_workbook_context::start_element(xmlns_token_t ns, xml_token_t name, const xml_attrs_t& attrs)
{
    xml_token_pair_t parent = push_stack(ns, name);
    switch (name)
    {
        case XML_workbook:
        {
            xml_element_expected(parent, XMLNS_UNKNOWN_TOKEN, XML_UNKNOWN_TOKEN);
            print_attrs(get_tokens(), attrs);

            xmlns_token_t default_ns =
                for_each(attrs.begin(), attrs.end(), workbook_attr_parser()).get_default_ns();

            // the namespace for worksheet element comes from its own 'xmlns' attribute.
            get_current_element().first = default_ns;
            set_default_ns(default_ns);
        }
        break;
        case XML_sheets:
            xml_element_expected(parent, XMLNS_xlsx, XML_workbook);
        break;
        case XML_sheet:
        {
            xml_element_expected(parent, XMLNS_xlsx, XML_sheets);
            const sheet& sheet = for_each(
                attrs.begin(), attrs.end(), workbook_sheet_attr_parser()).get_sheet();
            m_sheets.push_back(sheet);
        }
        break;
        default:
            warn_unhandled();
    }
}

bool xlsx_workbook_context::end_element(xmlns_token_t ns, xml_token_t name)
{
    return pop_stack(ns, name);
}

void xlsx_workbook_context::characters(const pstring& str) {}

void xlsx_workbook_context::pop_sheet_info(vector<sheet>& sheets)
{
    m_sheets.swap(sheets);
}

namespace {

class worksheet_attr_parser : public root_element_attr_parser
{
public:
    worksheet_attr_parser() :
        root_element_attr_parser(SCH_xlsx_main, XMLNS_xlsx) {}
    virtual ~worksheet_attr_parser() {}
    virtual void handle_other_attrs(const xml_attr_t &attr) {}
};

const char* dummy_sheet_name = "Test Sheet";

}

xlsx_sheet_xml_context::xlsx_sheet_xml_context(const tokens& tokens) :
    xml_context_base(tokens),
    mp_sheet(NULL)
{
}

xlsx_sheet_xml_context::~xlsx_sheet_xml_context()
{
    delete mp_sheet;
}

bool xlsx_sheet_xml_context::can_handle_element(xmlns_token_t ns, xml_token_t name) const
{
    return true;
}

xml_context_base* xlsx_sheet_xml_context::create_child_context(xmlns_token_t ns, xml_token_t name) const
{
    return NULL;
}

void xlsx_sheet_xml_context::end_child_context(xmlns_token_t ns, xml_token_t name, xml_context_base* child)
{
}

void xlsx_sheet_xml_context::start_element(xmlns_token_t ns, xml_token_t name, const xml_attrs_t& attrs)
{
    xml_token_pair_t parent = push_stack(ns, name);

    switch (name)
    {
        case XML_worksheet:
        {
            print_attrs(get_tokens(), attrs);

            xmlns_token_t default_ns = 
                for_each(attrs.begin(), attrs.end(), worksheet_attr_parser()).get_default_ns();

            // the namespace for worksheet element comes from its own 'xmlns' attribute.
            get_current_element().first = default_ns;
            set_default_ns(default_ns);

            mp_sheet = new model::sheet(pstring(dummy_sheet_name));
        }
        break;
        case XML_sheetData:
            xml_element_expected(parent, XMLNS_xlsx, XML_worksheet);
        break;
        case XML_row:
            xml_element_expected(parent, XMLNS_xlsx, XML_sheetData);
        break;
        case XML_c:
            xml_element_expected(parent, XMLNS_xlsx, XML_row);
        break;
        case XML_v:
            xml_element_expected(parent, XMLNS_xlsx, XML_c);
        break;
        default:
            warn_unhandled();
    }

}

bool xlsx_sheet_xml_context::end_element(xmlns_token_t ns, xml_token_t name)
{
    switch (name)
    {
        case XML_worksheet:
        break;
    }

    return pop_stack(ns, name);
}

void xlsx_sheet_xml_context::characters(const pstring& str)
{
}

// ============================================================================

namespace {

class shared_strings_root_attr_parser : public root_element_attr_parser
{
public:
    shared_strings_root_attr_parser() :
        root_element_attr_parser(SCH_xlsx_main, XMLNS_xlsx),
        m_count(0), m_unique_count(0) {}
    virtual ~shared_strings_root_attr_parser() {}

    virtual void handle_other_attrs(const xml_attr_t &attr)
    {
        switch (attr.name)
        {
            case XML_count:
                m_count = strtoul(attr.value.str().c_str(), NULL, 10);
            break;
            case XML_uniqueCount:
                m_unique_count = strtoul(attr.value.str().c_str(), NULL, 10);
            break;
        }
    }

    shared_strings_root_attr_parser& operator= (const shared_strings_root_attr_parser& r)
    {
        root_element_attr_parser::operator= (r);
        m_count = r.m_count;
        m_unique_count = r.m_unique_count;
        return *this;
    }

    size_t get_count() const { return m_count; }
    size_t get_unique_count() const { return m_unique_count; }
private:
    size_t m_count;
    size_t m_unique_count;
};

}

xlsx_shared_strings_context::xlsx_shared_strings_context(const tokens& tokens) :
    xml_context_base(tokens) {}

xlsx_shared_strings_context::~xlsx_shared_strings_context() {}

bool xlsx_shared_strings_context::can_handle_element(xmlns_token_t ns, xml_token_t name) const
{
    return true;
}

xml_context_base* xlsx_shared_strings_context::create_child_context(xmlns_token_t ns, xml_token_t name) const
{
    return NULL;
}

void xlsx_shared_strings_context::end_child_context(xmlns_token_t ns, xml_token_t name, xml_context_base* child)
{
}

void xlsx_shared_strings_context::start_element(xmlns_token_t ns, xml_token_t name, const xml_attrs_t& attrs)
{
    xml_token_pair_t parent = push_stack(ns, name);
    switch (name)
    {
        case XML_sst:
        {
            xml_element_expected(parent, XMLNS_UNKNOWN_TOKEN, XML_UNKNOWN_TOKEN);
            print_attrs(get_tokens(), attrs);

            shared_strings_root_attr_parser func;
            func = for_each(attrs.begin(), attrs.end(), shared_strings_root_attr_parser());
            xmlns_token_t default_ns = func.get_default_ns();

            // the namespace for worksheet element comes from its own 'xmlns' attribute.
            get_current_element().first = default_ns;
            set_default_ns(default_ns);

            cout << "count: " << func.get_count() << "  unique count: " << func.get_unique_count() << endl;
        }
        break;
        case XML_si:
            xml_element_expected(parent, XMLNS_xlsx, XML_sst);
        break;
        case XML_t:
            xml_element_expected(parent, XMLNS_xlsx, XML_si);
        break;
        default:
            warn_unhandled();
    }
}

bool xlsx_shared_strings_context::end_element(xmlns_token_t ns, xml_token_t name)
{
    switch (name)
    {
        case XML_t:
            cout << m_current_str << endl;
        break;
    }
    return pop_stack(ns, name);
}

void xlsx_shared_strings_context::characters(const pstring& str)
{
    xml_token_pair_t& cur_token = get_current_element();
    if (cur_token.first == XMLNS_xlsx && cur_token.second == XML_t)
        m_current_str = str;
}

}
