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

#ifndef __ORCUS_OOXML_CONTENT_TYPES_HPP__
#define __ORCUS_OOXML_CONTENT_TYPES_HPP__

#include "types.hpp"

namespace orcus {

extern content_type_t CT_ooxml_extended_properties;
extern content_type_t CT_ooxml_drawing;
extern content_type_t CT_ooxml_vml_drawing;
extern content_type_t CT_ooxml_xlsx_calc_chain;
extern content_type_t CT_ooxml_xlsx_comments;
extern content_type_t CT_ooxml_xlsx_connections;
extern content_type_t CT_ooxml_xlsx_external_link;
extern content_type_t CT_ooxml_xlsx_pivot_cache_def;
extern content_type_t CT_ooxml_xlsx_pivot_cache_rec;
extern content_type_t CT_ooxml_xlsx_pivot_table;
extern content_type_t CT_ooxml_xlsx_printer_settings;
extern content_type_t CT_ooxml_xlsx_query_table;
extern content_type_t CT_ooxml_xlsx_shared_strings;
extern content_type_t CT_ooxml_xlsx_sheet_main;
extern content_type_t CT_ooxml_xlsx_styles;
extern content_type_t CT_ooxml_xlsx_table;
extern content_type_t CT_ooxml_xlsx_worksheet;
extern content_type_t CT_ooxml_theme;
extern content_type_t CT_opc_core_properties;
extern content_type_t CT_opc_relationships;
extern content_type_t CT_xml;

/**
 * Null-terminated array of all content types.
 */
extern content_type_t* CT_all;

}

#endif
