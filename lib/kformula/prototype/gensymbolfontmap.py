#!/usr/bin/env python

import time
from xml.sax import saxutils, handler, make_parser

unicodetable = {}
fonttable = {}

class ContentGenerator(handler.ContentHandler):  

    def __init__(self):
        handler.ContentHandler.__init__(self)
        self.font = None
        
    def startElement(self, name, attrs):
        if name == 'unicodetable':
            self.font = None
            for (name, value) in attrs.items():
                if name == "font" and value:
                    self.font = value
                    if value not in fonttable:
                        fonttable[value] = []
        elif self.font and name == 'entry':
            number = ''
            for (name, value) in attrs.items():
                if name == "key": key = int(value)
                elif name == "number": number = value
                elif name == "name": latexName = value
                elif name == "class": charClass = value

            if number != '':
                unicodetable[number] = (latexName, charClass)
                fonttable[self.font].append((key, number))

def fontkey(font, number):
    for mapping in fonttable[font]:
        k, n = mapping
        if n == number:
            return k


def writeFontTable(fontname, f):
    f.write('\n\nstatic InternFontTable ' + fontname + 'Map[] = {\n')
    for key in unicodetable:
        latexName, charClass = unicodetable[key]
        pos = fontkey(fontname, key)
        if pos:
            f.write('    { ' + key + ', ' + hex(pos) + ', ' + charClass + ' },\n')
    f.write('    { 0, 0, ORDINARY }\n};\n\n')

    
def main():
    f = open('../symbolfontmapping.cc', 'w')
    f.write('''//
// Created: ''' + time.ctime(time.time()) + '''
//      by: gensymbolfontmap.py
//    from: symbol.xml
//
// WARNING! All changes made in this file will be lost!

struct InternFontTable { short unicode; uchar pos; CharClass cl; };
''')
    fontnames = [ "symbol",
                  "esstixnine", 
                  "esstixthirteen", 
                  "esstixeleven", 
                  "esstixfourteen", 
                  "esstixfive", 
                  "esstixfifteen", 
                  "esstixeight", 
                  "esstixthree", 
                  "esstixten", 
                  "esstixsixteen", 
                  "esstixone", 
                  "esstixtwo", 
                  "esstixsix", 
                  "esstixseven", 
                  "esstixtwelve", 
                  "esstixseventeen", 
                  "esstixfour" ]
    for fn in fontnames:
        writeFontTable(fn, f)

    print >>f, 'struct UnicodeNameTable { short unicode; const char* name; };'
    print >>f, 'static UnicodeNameTable nameTable[] = {'
    nameDir = {}
    for key in unicodetable:
        latexName, charClass = unicodetable[key]
        if len(latexName) > 0:
            for fn in fontnames:
                if fontkey(fn, key):
                    print >>f, '    { ' + key + ', "' + latexName + '" },'
                    break
    print >>f, '    { 0, 0 }\n};'
    f.close()
    

   
def make_unicode_table():
    header = []
    codes = {}
    f = open('../config/unicode.tbl', 'r')
    for line in f.xreadlines():
        if line[0] == '#':
            header.append(line.strip())
        else:
            break
    for line in f.xreadlines():
        if len(line) > 0:
            codes[line.split(',')[0].strip()] = line
    f.close()
    
    for key in unicodetable:
        latexName, charClass = unicodetable[key]
        if len(latexName) > 0:
            codes[key] = key + ', ' + charClass + ', ' + latexName.replace('\\', '')
        else:
            codes[key] = key + ', ' + charClass
            
    f = open('../config/unicode.tbl', 'w')
    for line in header:
        print >> f, line
    for key in codes:
        print >> f, codes[key]
    f.close()

def make_font_table(font):
    header = []
    try:
        f = open('../config/' + font + '.font', 'r')
        for line in f.xreadlines():
            if line[0] == '#':
                header.append(line.strip())
            else:
                break
        f.close()
    except IOError:
        pass
    
    f = open('../config/' + font + '.font', 'w')
    for line in header:
        print >> f, line
    #print >> f, "name = " + font
    for key in unicodetable:
        latexName, charClass = unicodetable[key]
        pos = fontkey(font, key)
        if pos:
            print >> f, str(pos) + ', ' + key
    f.close()

def make_all_font_tables():
    for font in fonttable:
        make_font_table(font)
        
if __name__ == '__main__':
    parser = make_parser()
    parser.setContentHandler(ContentGenerator())
    parser.parse("symbol.xml")

    main()
    #make_unicode_table()
    #make_all_font_tables()
