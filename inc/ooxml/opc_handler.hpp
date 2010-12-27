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

#ifndef __ORCUS_OPC_HANDLER_HPP__
#define __ORCUS_OPC_HANDLER_HPP__

#include "xmlhandler.hpp"
#include "ooxml/types.hpp"

namespace orcus {

class tokens;
class opc_content_types_context;
class opc_relations_context;

/**
 * XML stream handler for [Content_Types].xml part.
 */
class opc_content_types_handler : public xml_stream_handler
{
public:
    opc_content_types_handler(const tokens& _tokens);
    virtual ~opc_content_types_handler();

    virtual void start_document();
    virtual void end_document();
    virtual void start_element(xmlns_token_t ns, xml_token_t name, const xml_attrs_t& attrs);
    virtual void end_element(xmlns_token_t ns, xml_token_t name);
    virtual void characters(const pstring& str);

    void pop_parts(::std::vector<xml_part_t>& parts);
    void pop_ext_defaluts(::std::vector<xml_part_t>& ext_defaults);
private:
    opc_content_types_context* mp_context;
};

/**
 * XML Stream handler for relations parts.
 */
class opc_relations_handler : public xml_stream_handler
{
public:
    opc_relations_handler(const tokens& _tokens);
    virtual ~opc_relations_handler();

    virtual void start_document();
    virtual void end_document();
    virtual void start_element(xmlns_token_t ns, xml_token_t name, const xml_attrs_t& attrs);
    virtual void end_element(xmlns_token_t ns, xml_token_t name);
    virtual void characters(const pstring& str);

    void init();
    void pop_rels(::std::vector<opc_rel_t>& rels);

private:
    opc_relations_context* mp_context;
};

}

#endif
