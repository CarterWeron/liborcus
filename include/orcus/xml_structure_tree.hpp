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

#ifndef __ORCUS_XML_STRUCTURE_TREE_HPP__
#define __ORCUS_XML_STRUCTURE_TREE_HPP__

#include "env.hpp"
#include "types.hpp"

#include <boost/noncopyable.hpp>
#include <boost/ptr_container/ptr_vector.hpp>

namespace orcus {

struct xml_structure_tree_impl;

/**
 * Tree representing the structure of elements in XML content.  Recurring
 * elements under the same parent are represented by a single element
 * instance.  This tree only includes elements; no attributes and content
 * nodes appear in this tree.
 */
class ORCUS_DLLPUBLIC xml_structure_tree
{
    xml_structure_tree(const xml_structure_tree&); // disabled;
    xml_structure_tree& operator= (const xml_structure_tree&); // disabled

public:
    struct element;
    typedef boost::ptr_vector<element> element_store_type;

    struct element : boost::noncopyable
    {
        xmlns_id_t ns;
        pstring name;
        element_store_type child_elements;

        bool repeat:1;

        element();
    };

    xml_structure_tree();
    ~xml_structure_tree();

    void read_file(const char* filepath);

private:
    xml_structure_tree_impl* mp_impl;
};

}



#endif
