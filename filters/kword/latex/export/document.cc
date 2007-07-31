/*
** A program to convert the XML rendered by KWord into LATEX.
**
** Copyright (C) 2000, 2001, 2002 Robert JACOLIN
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
** Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
**
*/

#include <stdlib.h>		/* for atoi function    */

#include <kdebug.h>
#include <ktemporaryfile.h>
#include <KoStore.h>

#include <QDir>
//Added by qt3to4:
#include <QTextStream>

#include "fileheader.h"
#include "document.h"
#include "textFrame.h"
#include "formula.h"
#include "pixmapFrame.h"

/*******************************************/
/* Constructor                             */
/*******************************************/
Document::Document()
{
}

/*******************************************/
/* Destructor                              */
/*******************************************/
Document::~Document()
{
	kDebug(30522) <<"Document destructor";
}

/*******************************************/
/* Analyze                                 */
/*******************************************/
void Document::analyze(const QDomNode node)
{
	kDebug(30522) << getChildName(node, 0);
	for(int index= 0; index < getNbChild(node); index++)
	{
		Element *elt = 0;
		kDebug(30522) <<"--------------------------------------------------";

		kDebug(30522) << getChildName(node, index);
		switch(getTypeFrameset(getChild(node, index)))
		{
			case ST_NONE:
				kDebug(30522) <<"NONE";
				break;
			case ST_TEXT:
				kDebug(30522) <<"TEXT";
				elt = new TextFrame;
				elt->analyze(getChild(node, index));
				break;
			case ST_PICTURE:
				kDebug(30522) <<"PICTURE";
				elt = new PixmapFrame();
				elt->analyze(getChild(node, index));
				break;
			case ST_PART:
				kDebug(30522) <<"PART";
				//elt = new Part;
				//elt->analyze(getChild(node, index));
				break;
			case ST_FORMULA:
				/* Just save the frameset in a QString input
				 * call the formula latex export filter
				 * save in output
				 * generate: write the output
				 */
				kDebug(30522) <<"FORMULA";
				elt = new Formula;
				elt->analyze(getChild(node, index));
				break;
			case ST_HLINE:
				kDebug(30522) <<"HLINE";
				break;
			default:
				kDebug(30522) <<"error" << elt->getType() <<"" << ST_TEXT;
		}

		/* 3. Add the Element in one of the lists */
		if(elt != 0)
		{
			kDebug(30522) <<"INFO :" << elt->getSection();
			switch(elt->getSection())
			{
				case SS_FOOTERS: kDebug(30522) <<" FOOTER";
					       _footers.append(elt);
					       break;
				case SS_HEADERS: kDebug(30522) <<" HEADER";
						_headers.append(elt);
					break;
				case SS_BODY:
					if(!elt->isTable())
					{
						switch(elt->getType())
						{
							case ST_TEXT:
									_corps.append(elt);
									kDebug(30522) <<" BODY";
								break;
							case ST_PART:
									kDebug(30522) <<" PART";
									//_parts.append(elt);
								break;
							case ST_FORMULA:
									kDebug(30522) <<" FORMULA";
									_formulas.append(elt);
								break;
							case ST_PICTURE:
									kDebug(30522) <<" PIXMAP";
									_pixmaps.append(elt);
								break;
							default:
									kError(30522) << "Element frame type no supported or type unexpected." << endl;
						}
					}
					break;
				case SS_TABLE:
					kDebug(30522) <<" TABLE";
					/* Don't add simplely the cell */
					/* heriter ListTable de ListElement et surcharger
					 * la methode add. Une cellule est un element.
					 */
					_tables.add(elt);
					FileHeader::instance()->useTable();
					break;
				case SS_FOOTNOTES: /* Just for the new kwd file version */
						_footnotes.append(elt);
				break;
				default: kDebug(30522) <<"UNKNOWN";
					break;
			}
		}
		kDebug(30522) <<"END OF ANALYSIS OF A FRAMESET";
	}
}

/*******************************************/
/* AnalyzePixmaps                          */
/*******************************************/
void Document::analyzePixmaps(const QDomNode node)
{
	for(int index= 0; index < getNbChild(node); index++)
	{
		Key *key = 0;
		kDebug(30522) <<"NEW PIXMAP";

		key = new Key(Key::PIXMAP);
		key->analyze(getChild(node, "KEY"));
		_keys.append(key);
	}
}

/*******************************************/
/* getTypeFrameset                         */
/*******************************************/
SType Document::getTypeFrameset(const QDomNode node)
{
	SType type = ST_NONE;
	type = (SType) getAttr(node, "frameType").toInt();
	return type;
}

/*******************************************/
/* Generate                                */
/*******************************************/
void Document::generate(QTextStream &out, bool hasPreamble)
{
	kDebug(30522) <<"DOC. GENERATION.";

	if(hasPreamble)
		generatePreamble(out);
	kDebug(30522) <<"preamble :" << hasPreamble;

	/* Body */
	kDebug(30522) << endl <<"body :" << _corps.count();

	if(hasPreamble)
	{
		out << "\\begin{document}" << endl;
		Config::instance()->indent();
	}
	QString dir = "";
	if( !Config::instance()->getPicturesDir().isEmpty() &&
			Config::instance()->getPicturesDir() != NULL &&
			FileHeader::instance()->hasGraphics())
	{
		out << endl << "\\graphicspath{{" << Config::instance()->getPicturesDir() << "}}" << endl;
	}

	if(_corps.getFirst() != 0)
		_corps.getFirst()->generate(out);

	/* Just for test */
	/*if(_tables.getFirst() != 0)
		_tables.getFirst()->generate(out);
	if(_formulas.getFirst() != 0)
		_formulas.getFirst()->generate(out);*/
	if(hasPreamble)
		out << "\\end{document}" << endl;
	Config::instance()->desindent();
	if(Config::instance()->getIndentation() != 0)
			kError(30522) << "Error : indent != 0 at the end ! " << endl;
}

/*******************************************/
/* GeneratePreamble                        */
/*******************************************/
void Document::generatePreamble(QTextStream &out)
{
	Element* header;
	Element* footer;

	/* For each header */
	if(FileHeader::instance()->hasHeader())
	{
		kDebug(30522) <<"header :" << _headers.count();

		/* default : no rule */
		out << "\\renewcommand{\\headrulewidth}{0pt}" << endl;
		for(header = _headers.first(); header != 0; header = _headers.next())
		{
			generateTypeHeader(out, header);
		}
	}

	/* For each footer */
	if(FileHeader::instance()->hasFooter())
	{
		kDebug(30522) <<"footer :" << _footers.count();

		/* default : no rule */
		out << "\\renewcommand{\\footrulewidth}{0pt}" << endl;
		for(footer = _footers.first(); footer != 0; footer = _footers.next())
		{
			generateTypeFooter(out, footer);
		}
	}
	/* Specify what header/footer style to use */
	if(FileHeader::instance()->hasHeader() || FileHeader::instance()->hasFooter())
		out << "\\pagestyle{fancy}" << endl;
	else
	{
		out << "\\pagestyle{empty}" << endl;
	}
}

/*******************************************/
/* GenerateTypeHeader                      */
/*******************************************/
void Document::generateTypeHeader(QTextStream &out, Element *header)
{
	kDebug(30522) <<"generate header";
	if((FileHeader::instance()->getHeadType() == FileHeader::TH_ALL ||
		FileHeader::instance()->getHeadType() == FileHeader::TH_FIRST) && header->getInfo() == SI_EVEN)
	{
		out << "\\fancyhead[L]{}" << endl;
		out << "\\fancyhead[C]{";
		header->generate(out);
		out << "}" << endl;
		out << "\\fancyhead[R]{}" << endl;
	}

	switch(header->getInfo())
	{
		case SI_NONE:
		case SI_FIRST:
			break;
		case SI_ODD:
			out << "\\fancyhead[RO]{}" << endl;
			out << "\\fancyhead[CO]{";
			header->generate(out);
			out << "}" << endl;
			out << "\\fancyhead[LO]{}" << endl;
			break;
		case SI_EVEN:
			out << "\\fancyhead[RE]{}" << endl;
			out << "\\fancyhead[CE]{";
			header->generate(out);
			out << "}" << endl;
			out << "\\fancyhead[LE]{}" << endl;
			break;
	}

	if(header->getInfo() == SI_FIRST)
	{
		out << "\\fancyhead{";
		header->generate(out);
		out << "}" << endl;
		out << "\\thispagestyle{fancy}" << endl;
	}
}

/*******************************************/
/* GenerateTypeFooter                      */
/*******************************************/
void Document::generateTypeFooter(QTextStream &out, Element *footer)
{
	if(FileHeader::instance()->getFootType() == FileHeader::TH_ALL &&
			footer->getInfo() == SI_EVEN)
	{
		out << "\\fancyfoot[L]{}" << endl;
		out << "\\fancyfoot[C]{";
		footer->generate(out);
		out << "}" << endl;
		out << "\\fancyfoot[R]{}" << endl;
	}
	else if(FileHeader::instance()->getFootType() == FileHeader::TH_EVODD)
	{
		switch(footer->getInfo())
		{
			case SI_NONE:
			case SI_FIRST:
				break;
			case SI_ODD:
				out << "\\fancyfoot[CO]{";
				footer->generate(out);
				out << "}";
				break;
			case SI_EVEN:
				out << "\\fancyfoot[CE]{";
				footer->generate(out);
				out << "}";
				break;
		}
	}
	else if(FileHeader::instance()->getFootType() == FileHeader::TH_FIRST &&
			footer->getInfo() == SI_FIRST)
	{
		out << "\\fanycfoot{";
		footer->generate(out);
		out << "}" << endl;
		out << "\\thispagestyle{fancy}" << endl;
	}
}

Element* Document::searchAnchor(const QString& anchor)
{
	Element *elt = _tables.first();
	while(elt != 0)
	{
		kDebug(30522) << elt->getGrpMgr();
		if(elt->getGrpMgr() == anchor)
			return elt;
		elt = _tables.next();
	}
	kDebug(30522) <<"Not in table, search in formula list.";
	elt = _formulas.first();
	while(elt != 0)
	{
		if(elt->getName() == anchor)
			return elt;
		elt = _formulas.next();
	}
	kDebug(30522) <<"Not in table and formula list, search in pictures.";
	elt = _pixmaps.first();
	while(elt != 0)
	{
		if(elt->getName() == anchor)
			return elt;
		elt = _pixmaps.next();
	}
	return NULL;

}

Element* Document::searchFootnote(const QString& footnote)
{
	Element* elt = _footnotes.first();
	while(elt != 0)
	{
		if(elt->getName() == footnote)
			return elt;
		elt = _footnotes.next();
	}
	return NULL;

}

Key* Document::searchKey(const QString& keyName)
{
	Key* key = _keys.first();
	while(key != 0)
	{
		kDebug(30522) <<"key" << key->getFilename();
		if(key->getFilename() == keyName)
			return key;
		key = _keys.next();
	}
	return NULL;

}

QString Document::extractData(const QString& key)
{
	QString data = searchKey(key)->getName();
	kDebug(30522) <<"Opening" << data;
	if(!getStorage()->isOpen())
	{
		if(!getStorage()->open(data))
		{
			kError(30522) << "Unable to open " << data << endl;
			return QString("");
		}
	}

	/* Temp file with the default name in the default temp dir */
	KTemporaryFile tempFile;
	tempFile.setAutoRemove(false);
	tempFile.open();

	const Q_LONG buflen = 4096;
	char buffer[ buflen ];
	Q_LONG readBytes = getStorage()->read( buffer, buflen );

	while ( readBytes > 0 )
	{
		tempFile.write( buffer, readBytes );
		readBytes = getStorage()->read( buffer, buflen );
	}
	tempFile.close();
	if(!getStorage()->close())
	{
		kError(30522) << "Unable to close " << data << endl;
		return QString("");
	}
	kDebug(30522) <<"temp filename :" << tempFile.fileName();
	return tempFile.fileName();
}
