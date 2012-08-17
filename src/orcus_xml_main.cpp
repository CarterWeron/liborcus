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

#include "orcus/orcus_xml.hpp"
#include "orcus/global.hpp"
#include "orcus/sax_parser.hpp"

#include "model/factory.hpp"
#include "model/document.hpp"

#include <boost/scoped_ptr.hpp>

#include <cstdlib>

using namespace orcus;
using namespace std;

namespace {

void print_help()
{
    cout << "Usage: orcus-xml [map file] [data file]" << endl;
}

class xml_map_sax_handler
{
    struct attr
    {
        pstring ns;
        pstring name;
        pstring val;

        attr(const pstring& _ns, const pstring& _name, const pstring& _val) :
            ns(_ns), name(_name), val(_val) {}
    };

    struct scope
    {
        pstring ns;
        pstring name;

        scope(const pstring& _ns, const pstring& _name) :
            ns(_ns), name(_name) {}
    };

    vector<attr> m_attrs;
    vector<scope> m_scopes;
    orcus_xml& m_app;

public:
    xml_map_sax_handler(orcus_xml& app) : m_app(app) {}

    void declaration()
    {
        m_attrs.clear();
    }

    void start_element(const pstring& ns, const pstring& name)
    {
        pstring xpath, sheet;
        model::row_t row = -1;
        model::col_t col = -1;
        vector<attr>::const_iterator it = m_attrs.begin(), it_end = m_attrs.end();

        if (name == "cell")
        {
            for (; it != it_end; ++it)
            {
                if (it->name == "xpath")
                    xpath = it->val;
                else if (it->name == "sheet")
                    sheet = it->val;
                else if (it->name == "row")
                    row = strtol(it->val.get(), NULL, 10);
                else if (it->name == "column")
                    col = strtol(it->val.get(), NULL, 10);
            }

            m_app.set_cell_link(xpath, sheet, row, col);
        }
        else if (name == "range")
        {
            for (; it != it_end; ++it)
            {
                if (it->name == "sheet")
                    sheet = it->val;
                else if (it->name == "row")
                    row = strtol(it->val.get(), NULL, 10);
                else if (it->name == "column")
                    col = strtol(it->val.get(), NULL, 10);
            }

            m_app.start_range(sheet, row, col);
        }
        else if (name == "field")
        {
            for (; it != it_end; ++it)
            {
                if (it->name == "xpath")
                {
                    xpath = it->val;
                    break;
                }
            }

            m_app.append_field_link(xpath);
        }
        else if (name == "sheet")
        {
            pstring sheet_name;
            for (; it != it_end; ++it)
            {
                if (it->name == "name")
                {
                    sheet_name = it->val;
                    break;
                }
            }

            if (!sheet_name.empty())
                m_app.append_sheet(sheet_name);
        }

        m_scopes.push_back(scope(ns, name));
        m_attrs.clear();
    }

    void end_element(const pstring& ns, const pstring& name)
    {
        if (name == "range")
            m_app.commit_range();

        m_scopes.pop_back();
    }

    void characters(const pstring&) {}

    void attribute(const pstring& ns, const pstring& name, const pstring& val)
    {
        m_attrs.push_back(attr(ns, name, val));
    }
};

void read_map_file(orcus_xml& app, const char* filepath)
{
    cout << "reading map file " << filepath << endl;
    string strm;
    load_file_content(filepath, strm);
    if (strm.empty())
        return;

    xml_map_sax_handler handler(app);
    sax_parser<xml_map_sax_handler> parser(strm.c_str(), strm.size(), handler);
    parser.parse();
}

}

int main(int argc, char** argv)
{
    if (argc < 3)
    {
        print_help();
        return EXIT_FAILURE;
    }

    boost::scoped_ptr<model::document> doc(new model::document);
    boost::scoped_ptr<model::factory> fact(new model::factory(doc.get()));

    orcus_xml app(fact.get());
    read_map_file(app, argv[1]);
    app.read_file(argv[2]);
    doc->dump();
    pstring::intern::dispose();

    return EXIT_SUCCESS;
}
