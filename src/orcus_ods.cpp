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

#include <zip.h>

#include "orcus/xml_parser.hpp"
#include "orcus/odf/ods_handler.hpp"
#include "orcus/odf/odf_tokens.hpp"
#include "orcus/model/document.hpp"
#include "orcus/model/factory.hpp"

#include <cstdlib>
#include <iostream>
#include <vector>

#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>

using namespace std;
using namespace orcus;

class orcus_ods : private ::boost::noncopyable
{
public:
    orcus_ods(model::factory_base* factory);
    ~orcus_ods();

    void read_file(const char* fpath, const char* outpath);

private:
    void list_content(struct zip* archive) const;
    void read_content(struct zip* archive);
    void read_content_xml(const uint8_t* p, size_t size);

private:
    ::boost::shared_ptr<model::factory_base> mp_factory;
};

orcus_ods::orcus_ods(model::factory_base* factory) :
    mp_factory(factory)
{
}

orcus_ods::~orcus_ods()
{
}

void orcus_ods::list_content(struct zip* archive) const
{
    zip_uint64_t num = zip_get_num_entries(archive, 0);
    cout << "number of files this archive contains: " << num << endl;

    for (zip_uint64_t i = 0; i < num; ++i)
    {
        const char* filename = zip_get_name(archive, i, 0);
        cout << filename << endl;
    }
}

void orcus_ods::read_content(struct zip* archive)
{
    if (!archive)
        return;

    struct zip_stat file_stat;
    if (zip_stat(archive, "content.xml", 0, &file_stat))
    {
        cout << "failed to get stat on content.xml" << endl;
        return;
    }

    cout << "name: " << file_stat.name << "  size: " << file_stat.size << endl;
    struct zip_file* zfd = zip_fopen(archive, file_stat.name, 0);
    if (zfd)
    {
        vector<uint8_t> buf(file_stat.size, 0);
        int buf_read = zip_fread(zfd, &buf[0], file_stat.size);
        cout << "actual buffer read: " << buf_read << endl;
        if (buf_read > 0)
        read_content_xml(&buf[0], buf_read);
        zip_fclose(zfd);
    }
}

void orcus_ods::read_content_xml(const uint8_t* p, size_t size)
{
    xml_stream_parser parser(odf_tokens, p, size, "content.xml");
    ::boost::scoped_ptr<ods_content_xml_handler> handler(
        new ods_content_xml_handler(odf_tokens, mp_factory.get()));
    parser.set_handler(handler.get());
    parser.parse();
}

void orcus_ods::read_file(const char* fpath, const char* outpath)
{
    cout << "reading " << fpath << endl;
    int error;
    struct zip* archive = zip_open(fpath, 0, &error);
    if (!archive)
    {
        cout << "failed to open " << fpath << endl;
        return;
    }

    list_content(archive);
    read_content(archive);
    zip_close(archive);
}

int main(int argc, char** argv)
{
    if (argc != 2)
        return EXIT_FAILURE;

    ::boost::scoped_ptr<model::document> doc(new model::document);

    orcus_ods app(new model::factory(doc.get()));
    app.read_file(argv[1], "out.html");
//  doc->dump();
    pstring::intern::dispose();

    return EXIT_SUCCESS;
}
