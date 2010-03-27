#!/usr/bin/env python
#************************************************************************
#
#  Copyright (c) 2010 Kohei Yoshida
#  
#  Permission is hereby granted, free of charge, to any person
#  obtaining a copy of this software and associated documentation
#  files (the "Software"), to deal in the Software without
#  restriction, including without limitation the rights to use,
#  copy, modify, merge, publish, distribute, sublicense, and/or sell
#  copies of the Software, and to permit persons to whom the
#  Software is furnished to do so, subject to the following
#  conditions:
#  
#  The above copyright notice and this permission notice shall be
#  included in all copies or substantial portions of the Software.
#  
#  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
#  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
#  OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
#  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
#  HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
#  WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
#  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
#  OTHER DEALINGS IN THE SOFTWARE.
#
#***********************************************************************

import xml.parsers.expat, sys

class xml_parser:

    def __init__ (self, strm):
        self.__strm = strm
        self.__elem = None
        self.tokens = {}

    def start_element(self, name, attrs):
        self.__elem = name
        if name in ['element', 'attribute'] and attrs.has_key('name'):
            tokens = attrs['name'].split(':')
            for token in tokens:
                self.tokens[token] = True

    def end_element(self, name):
        pass

    def character(self, data):
        if self.__elem == 'value':
            s = data.strip()
            if len(s) > 0:
                self.tokens[s] = True

    def parse (self):
        p = xml.parsers.expat.ParserCreate()
        p.StartElementHandler = self.start_element
        p.EndElementHandler = self.end_element
        p.CharacterDataHandler = self.character
        p.Parse(self.__strm, 1)

def normalize_name (old):
    new = ''
    for c in old:
        if c in '.-': # '.' nad '-' are not allowed in C++ symbols.
            c = '_'
        new += c
    return new

def main (args):

    file = open(sys.argv[1], 'r')
    chars = file.read()
    file.close()
    
    parser = xml_parser(chars)
    parser.parse()
    tokens = parser.tokens.keys()
    tokens.sort()
    token_id = 0
    for token in tokens:
        token = normalize_name(token)
        print ("const uint32_t XML_%s = %d;"%(token, token_id))
        token_id += 1
    
    print ("const char** token_names[] = {")
    token_id = 0
    token_size = len(tokens)
    for i in xrange(0, token_size):
        token = tokens[i]
        s = ','
        if i == token_size-1:
            s = ' '
        print ("    \"%s\"%s // %d"%(token, s, token_id))
        token_id += 1
    print ("};")

if __name__ == '__main__':
    main(sys.argv)
