/* TODO : Manage File problems !
 */
/*
** A program to convert the XML rendered by KWord into LATEX.
**
** Copyright (C) 2000 Robert JACOLIN
**
** This library is free software; you can redistribute it and/or
** modify it under the terms of the GNU Library General Public
** License as published by the Free Software Foundation; either
** version 2 of the License, or (at your option) any later version.
**
** This library is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
** Library General Public License for more details.
**
** To receive a copy of the GNU Library General Public License, write to the
** Free Software Foundation, Inc., 59 Temple Place - Suite 330,
** Boston, MA  02111-1307, USA.
**
*/

#include <kdebug.h>

#include "xml2latexparser.h"

Xml2LatexParser::Xml2LatexParser(const KoStore* in, QString fileOut, Config* config):
		XmlParser(config, in), _file(fileOut), _in( in )
{
	//kdDebug() << fileIn.latin1() << endl;
	kdDebug() << fileOut.latin1() << endl;
	_filename = fileOut;
	//setFileHeader(_fileHeader);
	setRoot(&_document);
}

void Xml2LatexParser::analyse()
{
	QDomNode balise;
	balise = init();
	//balise = getChild(balise, "DOC");
	kdDebug() <<"HEADER -> PAPER" << endl;
	FileHeader::instance()->analysePaper(getChild(balise, "PAPER"));
	kdDebug() <<"HEADER -> ATTRIBUTES" << endl;
	FileHeader::instance()->analyseAttributs(getChild(balise, "ATTRIBUTES"));
	kdDebug() <<"HEADER -> FRAMESETS" << endl;
	_document.analyse(getChild(balise, "FRAMESETS"));
	kdDebug() <<"HEADER -> END FRAMESETS" << endl;
	//kdDebug() <<"HEADER -> STYLES" << endl;
	//
	kdDebug() <<"HEADER -> PICTURES" << endl;
	_document.analysePixmaps(getChild(balise, "PICTURES"));
	//kdDebug() <<"HEADER -> SERIALL" << endl;
	kdDebug() << "END ANALYSE" << endl;
}

void Xml2LatexParser::generate()
{
	if(_file.open(IO_WriteOnly))
	{
		kdDebug() << "GENERATION" << endl;
		_out.setDevice(&_file);
		if(!Config::instance()->isEmbeded())
			FileHeader::instance()->generate(_out);
		_document.generate(_out, !Config::instance()->isEmbeded());
		//_out << getDocument();
		_file.close();
	}
	else
		kdDebug() << "Can't use the file ..." << endl;
}
