/*************************************************************************
 *
 * Copyright (c) 2010-2013 Kohei Yoshida
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

#include "orcus/orcus_xlsx.hpp"

#include "orcus/xml_namespace.hpp"
#include "orcus/global.hpp"
#include "orcus/spreadsheet/import_interface.hpp"

#include "xlsx_types.hpp"
#include "xlsx_handler.hpp"
#include "xlsx_context.hpp"
#include "xlsx_workbook_context.hpp"
#include "ooxml_tokens.hpp"

#include "xml_stream_parser.hpp"
#include "xml_simple_stream_handler.hpp"
#include "opc_reader.hpp"
#include "ooxml_namespace_types.hpp"
#include "session_context.hpp"
#include "opc_context.hpp"

#include <cstdlib>
#include <iostream>
#include <string>
#include <cstring>
#include <sstream>

#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/ptr_container/ptr_vector.hpp>

using namespace std;

namespace orcus {

namespace {

struct print_sheet_info : unary_function<pair<pstring, const opc_rel_extra*>, void>
{
    void operator() (const pair<pstring, const opc_rel_extra*>& v) const
    {
        const xlsx_rel_sheet_info* info = static_cast<const xlsx_rel_sheet_info*>(v.second);
        cout << "sheet name: " << info->name << "  sheet id: " << info->id << "  relationship id: " << v.first << endl;
    }
};

}

class xlsx_opc_handler : public opc_reader::part_handler
{
    orcus_xlsx& m_parent;
public:
    xlsx_opc_handler(orcus_xlsx& parent) : m_parent(parent) {}
    virtual ~xlsx_opc_handler() {}

    virtual bool handle_part(
        schema_t type, const std::string& dir_path, const std::string& file_name, const opc_rel_extra* data)
    {
        if (type == SCH_od_rels_office_doc)
        {
            m_parent.read_workbook(dir_path, file_name);
            return true;
        }
        else if (type == SCH_od_rels_worksheet)
        {
            m_parent.read_sheet(dir_path, file_name, static_cast<const xlsx_rel_sheet_info*>(data));
            return true;
        }
        else if (type == SCH_od_rels_shared_strings)
        {
            m_parent.read_shared_strings(dir_path, file_name);
            return true;
        }
        else if (type == SCH_od_rels_styles)
        {
            m_parent.read_styles(dir_path, file_name);
            return true;
        }

        return false;
    }
};

struct orcus_xlsx_impl
{
    session_context m_cxt;
    xmlns_repository m_ns_repo;
    spreadsheet::iface::import_factory* mp_factory;
    xlsx_opc_handler m_opc_handler;
    opc_reader m_opc_reader;

    orcus_xlsx_impl(spreadsheet::iface::import_factory* factory, orcus_xlsx& parent) :
        mp_factory(factory), m_opc_handler(parent), m_opc_reader(m_ns_repo, m_cxt, m_opc_handler) {}
};

orcus_xlsx::orcus_xlsx(spreadsheet::iface::import_factory* factory) :
    mp_impl(new orcus_xlsx_impl(factory, *this))
{
    mp_impl->m_ns_repo.add_predefined_values(NS_ooxml_all);
    mp_impl->m_ns_repo.add_predefined_values(NS_opc_all);
}

orcus_xlsx::~orcus_xlsx()
{
    delete mp_impl;
}

bool orcus_xlsx::detect(const unsigned char* blob, size_t size)
{
    zip_archive_stream_blob stream(blob, size);
    zip_archive archive(&stream);
    try
    {
        archive.load();
    }
    catch (const zip_error&)
    {
        // Not a valid zip archive.
        return false;
    }

    // Find and parse [Content_Types].xml which is required for OPC package.
    vector<unsigned char> buf;
    if (!archive.read_file_entry("[Content_Types].xml", buf))
        // Failed to read 'app.xml' entry.
        return false;

    if (buf.empty())
        return false;

    xmlns_repository ns_repo;
    ns_repo.add_predefined_values(NS_opc_all);
    session_context session_cxt;
    xml_stream_parser parser(ns_repo, opc_tokens, reinterpret_cast<const char*>(&buf[0]), buf.size(), "[Content_Types].xml");

    xml_simple_stream_handler handler(new opc_content_types_context(session_cxt, opc_tokens));
    parser.set_handler(&handler);
    parser.parse();

    opc_content_types_context& context =
        static_cast<opc_content_types_context&>(handler.get_context());

    std::vector<xml_part_t> parts;
    context.pop_parts(parts);

    if (parts.empty())
        return false;

    // See if we can find the workbook stream.
    xml_part_t workbook_part("/xl/workbook.xml", "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet.main+xml");
    return std::find(parts.begin(), parts.end(), workbook_part) != parts.end();
}

void orcus_xlsx::read_file(const char* fpath)
{
    mp_impl->m_opc_reader.read_file(fpath);
    mp_impl->mp_factory->finalize();
}

void orcus_xlsx::read_workbook(const string& dir_path, const string& file_name)
{
    string filepath = dir_path + file_name;
    cout << "read_workbook: file path = " << filepath << endl;

    vector<unsigned char> buffer;
    if (!mp_impl->m_opc_reader.open_zip_stream(filepath, buffer))
        return;

    if (buffer.empty())
        return;

    ::boost::scoped_ptr<xml_simple_stream_handler> handler(
        new xml_simple_stream_handler(new xlsx_workbook_context(mp_impl->m_cxt, ooxml_tokens)));

    xml_stream_parser parser(mp_impl->m_ns_repo, ooxml_tokens, reinterpret_cast<const char*>(&buffer[0]), buffer.size(), filepath);
    parser.set_handler(handler.get());
    parser.parse();

    // Get sheet info from the context instance.
    xlsx_workbook_context& context =
        static_cast<xlsx_workbook_context&>(handler->get_context());
    opc_rel_extras_t sheet_data;
    context.pop_sheet_info(sheet_data);
    for_each(sheet_data.begin(), sheet_data.end(), print_sheet_info());

    mp_impl->m_opc_reader.check_relation_part(file_name, &sheet_data);
}

void orcus_xlsx::read_sheet(const string& dir_path, const string& file_name, const xlsx_rel_sheet_info* data)
{
    cout << "---" << endl;
    string filepath = dir_path + file_name;
    cout << "read_sheet: file path = " << filepath << endl;

    vector<unsigned char> buffer;
    if (!mp_impl->m_opc_reader.open_zip_stream(filepath, buffer))
        return;

    if (buffer.empty())
        return;

    if (data)
    {
        cout << "relationship sheet data: " << endl;
        cout << "  sheet name: " << data->name << "  sheet ID: " << data->id << endl;
    }

    xml_stream_parser parser(mp_impl->m_ns_repo, ooxml_tokens, reinterpret_cast<const char*>(&buffer[0]), buffer.size(), file_name);
    spreadsheet::iface::import_sheet* sheet = mp_impl->mp_factory->append_sheet(data->name.get(), data->name.size());
    ::boost::scoped_ptr<xlsx_sheet_xml_handler> handler(new xlsx_sheet_xml_handler(mp_impl->m_cxt, ooxml_tokens, sheet));
    parser.set_handler(handler.get());
    parser.parse();

    mp_impl->m_opc_reader.check_relation_part(file_name, NULL);
}

void orcus_xlsx::read_shared_strings(const string& dir_path, const string& file_name)
{
    cout << "---" << endl;
    string filepath = dir_path + file_name;
    cout << "read_shared_strings: file path = " << filepath << endl;

    vector<unsigned char> buffer;
    if (!mp_impl->m_opc_reader.open_zip_stream(filepath, buffer))
        return;

    if (buffer.empty())
        return;

    xml_stream_parser parser(mp_impl->m_ns_repo, ooxml_tokens, reinterpret_cast<const char*>(&buffer[0]), buffer.size(), file_name);
    ::boost::scoped_ptr<xml_simple_stream_handler> handler(
        new xml_simple_stream_handler(
            new xlsx_shared_strings_context(mp_impl->m_cxt, ooxml_tokens, mp_impl->mp_factory->get_shared_strings())));
    parser.set_handler(handler.get());
    parser.parse();
}

void orcus_xlsx::read_styles(const string& dir_path, const string& file_name)
{
    cout << "---" << endl;
    string filepath = dir_path + file_name;
    cout << "read_styles: file path = " << filepath << endl;

    spreadsheet::iface::import_styles* styles = mp_impl->mp_factory->get_styles();
    if (!styles)
        // Client code doesn't support styles.
        return;

    vector<unsigned char> buffer;
    if (!mp_impl->m_opc_reader.open_zip_stream(filepath, buffer))
        return;

    if (buffer.empty())
        return;

    xml_stream_parser parser(mp_impl->m_ns_repo, ooxml_tokens, reinterpret_cast<const char*>(&buffer[0]), buffer.size(), file_name);
    ::boost::scoped_ptr<xml_simple_stream_handler> handler(
        new xml_simple_stream_handler(
            new xlsx_styles_context(mp_impl->m_cxt, ooxml_tokens, mp_impl->mp_factory->get_styles())));
//      xlsx_styles_context& context =
//          static_cast<xlsx_styles_context&>(handler->get_context());
    parser.set_handler(handler.get());
    parser.parse();
}

}
