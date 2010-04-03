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

#include "odscontext.hpp"

#include <iostream>
#include <fstream>

using namespace std;

namespace orcus {

ods_content_xml_context::ods_content_xml_context()
{
}

ods_content_xml_context::~ods_content_xml_context()
{
}

void ods_content_xml_context::start_content()
{
    cout << "start content" << endl;
}

void ods_content_xml_context::end_content()
{
    cout << "end content" << endl;
}

void ods_content_xml_context::start_table(const xml_attrs_t& attrs)
{
    cout << "start table: " << endl;
}

void ods_content_xml_context::end_table()
{
    cout << "end table" << endl;
}

void ods_content_xml_context::start_column(const xml_attrs_t& attrs)
{
}

void ods_content_xml_context::end_column()
{
}

void ods_content_xml_context::start_row(const xml_attrs_t& attrs)
{
}

void ods_content_xml_context::end_row()
{
}

void ods_content_xml_context::start_cell(const xml_attrs_t& attrs)
{
}

void ods_content_xml_context::end_cell()
{
}

void ods_content_xml_context::print_html(const string& filepath) const
{
    ofstream file(filepath.c_str());
    file << "<html>" << endl;
    file << "<title>content.xml</title>" << endl;
    file << "<body>" << endl;
    file << "</body>" << endl;
    file << "</html>" << endl;
}

}
