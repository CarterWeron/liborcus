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

#ifndef __ORCUS_OPC_READER_HPP__
#define __ORCUS_OPC_READER_HPP__

#include "orcus/ooxml/schemas.hpp"
#include "orcus/xml_simple_handler.hpp"
#include "orcus/env.hpp"

#include <vector>
#include <string>
#include <boost/noncopyable.hpp>

struct zip;

namespace orcus {

class pstring;
struct opc_rel_extra;

class opc_reader : boost::noncopyable
{
    typedef std::vector<std::string> dir_stack_type;

public:
    class part_handler
    {
    public:
        virtual ~part_handler() = 0;
        virtual bool handle_part(
            schema_t type, const std::string& dir_path, const std::string& file_name, const opc_rel_extra* data) = 0;
    };

    opc_reader(part_handler& handler);

    void read_file(const char* fpath);

    /**
     * Read an xml part inside package.  The path is relative to the relation
     * file.
     *
     * @param path the path to the xml part.
     * @param type schema type.
     */
    void read_part(const pstring& path, const schema_t type, const opc_rel_extra* data);
    void check_relation_part(const char* file_name, const opc_rel_extras_t* extras);

private:

    void list_content() const;
    void read_content();
    void read_content_types();
    void read_relations(const char* path, std::vector<opc_rel_t>& rels);

    std::string get_current_dir() const;

private:
    part_handler& m_handler;

    struct zip* m_archive;

    xml_simple_stream_handler m_opc_rel_handler;

    std::vector<xml_part_t> m_parts;
    std::vector<xml_part_t> m_ext_defaults;
    dir_stack_type m_dir_stack;
};

}

#endif
