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
#include <stdlib.h>

#include "document.h"

/*******************************************/
/* Constructor                             */
/*******************************************/
Document::Document(QString fileIn, QString fileOut): XmlParser(fileIn), _file(fileOut)
{
	kdDebug() << fileOut.latin1() << endl;
	_filename = fileOut;
	setLatexType(LT_PSTRICKS);
}

/*******************************************/
/* Constructor                             */
/*******************************************/
Document::Document(QByteArray in, QString fileOut): XmlParser(in), _file(fileOut)
{
	kdDebug() << fileOut.latin1() << endl;
	_filename = fileOut;
	setLatexType(LT_PSTRICKS);
}

/*******************************************/
/* Destructor                              */
/*******************************************/
Document::~Document()
{
	kdDebug() << "Corps Destructor" << endl;
}

/*******************************************/
/* Analyse                                 */
/*******************************************/
void Document::analyse()
{
	QDomNode balise;

	kdDebug() << "BEGIN THE ANALYSE OF A DOCUMENT" << endl;
	balise = init();
	balise = getChild(balise, "killustrator");
	analysePreambule(balise);
	analyseDocument(balise);
	kdDebug() << "END OF ANALYSE OF A DOCUMENT" << endl;
}

void Document::analysePreambule(const QDomNode balise)
{
	/* Get parameters */
	//balise.getAttr(balise, "mime");
	_version= getAttr(balise, "version").toInt();
	//balise.getAttr(balise, "editor");
}

/*******************************************/
/* AnalyseDocument                         */
/*******************************************/
void Document::analyseDocument(const QDomNode balise)
{
	_header.analyse(getChild(balise, "head"));
	
	for(int index = 1; index < getNbChild(balise); index++)
	{
		Page* page = 0;
		kdDebug() << "balise : " << getChildName(balise, index) << endl;
		if(getChildName(balise, index).compare("page") == 0)
		{
			page = new Page();
			page->analyse(getChild(balise, index));
			_pages.append(page);
		}
	}
}

/*******************************************/
/* Generate                                */
/*******************************************/
void Document::generate() //QTextStream &out)
{
	if(_file.open(IO_WriteOnly))
	{
		kdDebug() << "GENERATION" << endl;
		_out.setDevice(&_file);
		/* Generation */
		_header.generate(_out);

		if(_pages.first()->getFormat() == TF_CUSTOM)
		{
			_out << "\\setlength{\\paperwidth}{"  << _pages.first()->getWidth()  << "mm}" << endl;
			_out << "\\setlength{\\paperheight}{" << _pages.first()->getHeight() << "mm}" << endl;
		}

		_out << "\\setlength{\\textwidth}{"  << (_pages.first()->getWidth() - _pages.first()->getLeftMargin() - _pages.first()->getRightMargin())  << "mm}" << endl;
		_out << "\\setlength{\\textheight}{" << (_pages.first()->getHeight()) << "mm}" << endl;

		/* Margin */
		_out << "\\setlength{\\topmargin}{" << _pages.first()->getTopMargin() << "mm}" << endl;
		_out << "\\addtolength{\\leftmargin}{" << _pages.first()->getLeftMargin() << "mm}" << endl;
		_out << endl;
		
		_out << "\\begin{document}" << endl;
		for(Page* page = _pages.first(); page != 0; page = _pages.next())
		{
			if(getLatexType() == LT_PSTRICKS)
			{
				_out << "\\begin{pspicture}(";
				_out << page->getWidth() << "mm,";
				_out << page->getHeight() << "mm)" << endl;
				page->generatePSTRICKS(_out);
				_out << "\\end{pspicture}" << endl;
			}
		}
		_out << "\\end{document}" << endl;
		_out << getDocument();
	}
	else
		kdDebug() << "Can't use the file ..." << endl;
	_file.close();
}

