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
#include <kdebug.h>		/* for kDebug() stream */
#include "para.h"
#include "textFrame.h"		/* father class.        */
#include "format.h"		/* children classes.    */
//#include "picturezone.h"
#include "fileheader.h"
#include "textzone.h"
#include "variablezone.h"
#include "footnote.h"
#include "anchor.h"
//Added by qt3to4:
#include <QTextStream>
#include <Q3PtrList>

/* static data */
Q3PtrStack<EType> Para::_historicList;
int Para::_tabulation = 0;

/*******************************************/
/* Constructor                             */
/*******************************************/
Para::Para(TextFrame* textFrame)
{
	_element    = textFrame;
	_lines      = 0;
	_name       = 0;
	_info       = EP_NONE;	/* the parag is not a footnote */
	//_hardbrk   = EP_FLOW;	/* and it's not a new page */
	_currentPos = 0;		/* At the beginning of the paragraph */
	_tabulation = 0;
	_text = "";
}

/*******************************************/
/* Destructor                              */
/*******************************************/
Para::~Para()
{
	kDebug(30522) <<"Destruction of a parag.";
	if(_lines != 0)
		delete _lines;
}

/*******************************************/
/* GetFrameType                            */
/*******************************************/
/* To know if it's the text or it's a      */
/* header or a footer.                     */
/*******************************************/
SSect Para::getFrameType() const
{
	return _element->getSection();
}

/*******************************************/
/* getTypeFormat                           */
/*******************************************/
/* To know if the zone is a textzone, a    */
/* footnote, a picture, a variable.        */
/*******************************************/
EFormat Para::getTypeFormat(const QDomNode node) const
{
	//<FORMAT id="1" ...>

	return (EFormat) getAttr(node, "id").toInt();
}

/*******************************************/
/* getNbCharPara                           */
/*******************************************/
/* To know the size of a paragraph.        */
/*******************************************/
int Para::getNbCharPara() const
{
	int nb = 0;
	Format* zone = 0;

	if(_lines != 0)
	{
		kDebug(30522) <<"  NB ZONE :" << _lines->count();

		for(zone = _lines->first(); zone != 0; zone = _lines->next())
		{
			switch(zone->getId())
			{
				case EF_TEXTZONE:
						nb = nb + ((TextZone*) zone)->getSize();
					break;
				case EF_PICTURE:
					break;
				case EF_TABULATOR:
					break;
				case EF_VARIABLE:
					break;
				case EF_FOOTNOTE:
					break;
				case EF_ANCHOR:
					break;
				case EF_ERROR:
					break;
			}
		}
	}
	return nb;
}

/*******************************************/
/* Analyze                                 */
/*******************************************/
void Para::analyze(const QDomNode node)
{
	/* markup type: paragraph */

	kDebug(30522) <<"**** PARAGRAPH ****";

	/* Analysis of the child markups */
	for(int index = 0; index < getNbChild(node); index++)
	{
		if(getChildName(node, index).compare("TEXT")== 0)
		{
			_text =  getData(node, index);
			kDebug(30522) <<"TEXT :" << _text;
		}
		else if(getChildName(node, index).compare("NAME")== 0)
		{
			analyzeName(getChild(node, index));
		}
		else if(getChildName(node, index).compare("INFO")== 0)
		{
			analyzeInfo(getChild(node, index));
		}
		/*else if(getChildName(node, index).compare("HARDBRK")== 0)
		{
			analyzeBrk(getChild(node, index));
		}*/
		else if(getChildName(node, index).compare("FORMATS")== 0)
		{
			// IMPORTANT ==> font and style
			kDebug(30522) <<"FORMATS";
			analyzeFormats(getChild(node, index));

		}
		else if(getChildName(node, index).compare("LAYOUT")== 0)
		{
			kDebug(30522) <<"LAYOUT";
			analyzeLayoutPara(getChild(node, index));
		}
	}
	kDebug(30522) <<" **** END PARAGRAPH ****";
}

/*******************************************/
/* AnalyzeName                             */
/*******************************************/
/* If a footnote has a name it is a        */
/* footnote/endnote.                       */
/*******************************************/
void Para::analyzeName(const QDomNode node)
{
	/* <NAME name="Footnote/Endnote_1"> */

	_name = new QString(getAttr(node, "NAME"));
}

/*******************************************/
/* AnalyzeInfo                             */
/*******************************************/
/* Type of the parag.: If info is 1, it is */
/* a footnote/endnote (so it has a name).  */
/*******************************************/
void Para::analyzeInfo(const QDomNode node)
{
	/* <INFO info="1"> */

	_info = (EP_INFO) getAttr(node, "INFO").toInt();
}

/*******************************************/
/* AnalyzeBrk                              */
/*******************************************/
/* There is a new page before this         */
/* paragraph.                              */
/*******************************************/
/*void Para::analyzeBrk(const QDomNode node)
{
	//<NAME name="Footnote/Endnote_1">

	_hardbrk = (EP_HARDBRK) getAttr(node, "FRAME").toInt();
}*/

/*******************************************/
/* AnalyzeLayoutPara                       */
/*******************************************/
/* Analyze the layout of a para.           */
/* For each format, keep the type (picture,*/
/* text, variable, footnote) and put the   */
/* zone in a list.                         */
/*******************************************/
void Para::analyzeLayoutPara(const QDomNode node)
{
	Format* zone = 0;

	analyzeLayout(node);
	for(int index= 0; index < getNbChild(node); index++)
	{
		if(getChildName(node, index).compare("FORMAT")== 0)
		{
			//analyzeFormat(node);
			/* No more format: verify if all the text zone has been formatted */
			if(_currentPos != _text.length())
			{
				zone = new TextZone(_text, this);
				((TextZone*) zone)->setPos(_currentPos);
				((TextZone*) zone)->setLength(_currentPos - _text.length());
				((TextZone*) zone)->analyze();
				if(_lines == 0)
					_lines = new Q3PtrList<Format>;
				/* add the text */
				_lines->append(zone);
				_currentPos = _currentPos + ((TextZone*) zone)->getLength();

			}
		}
		/*else
			kDebug(30522) <<" FORMAT FIELD UNKNOWN";*/
	}
}

/*******************************************/
/* AnalyzeFormats                          */
/*******************************************/
/* Analyze several formats.                */
/* keep the type (picture, text, variable, */
/* footnote) and put the zone in a list.   */
/*******************************************/
void Para::analyzeFormats(const QDomNode node)
{
	for(int index= 0; index < getNbChild(node, "FORMAT"); index++)
	{
		if(getChildName(node, index).compare("FORMAT")== 0)
		{
			kDebug(30522) <<"A FORMAT !!!";
			analyzeFormat(getChild(node, index));
		}
		else
			kDebug(30522) <<" FORMAT UNUSEFULL HERE";
	}
}

/*******************************************/
/* AnalyzeFormat                           */
/*******************************************/
/* Analyze one format.                     */
/* keep the type (picture, text, variable, */
/* footnote) and put the zone in a list.   */
/*******************************************/
void Para::analyzeFormat(const QDomNode node)
{
	Format *zone      = 0;
	Format *zoneFirst = 0;

	kDebug(30522) <<"ANALYZE FORMAT BODY";
	switch(getTypeFormat(node))
	{
		case EF_ERROR: kDebug(30522) <<"Id format error";
			break;
		case EF_TEXTZONE: /* It's a text line (1) */
				zone = new TextZone(_text, this);
				if(_currentPos != _text.length())
				{
					zone->analyze(node);
					if(zone->getPos() != _currentPos)
					{
						if(_lines == 0)
							_lines = new Q3PtrList<Format>;
							/* Create first a default format */
						zoneFirst = new TextZone(_text, this);
						zoneFirst->setPos(_currentPos);
						zoneFirst->setLength(zone->getPos() - _currentPos);
						((TextZone*) zoneFirst)->analyze();

						/* Add the text without format */
						_lines->append(zoneFirst);
						_currentPos = _currentPos + ((TextZone*) zoneFirst)->getLength();
					}
				}
			break;
		case EF_PICTURE: /* It's a picture (2) */
				/*zone = new PictureZone(this);
				zone->analyze(node);*/
			break;
		case EF_VARIABLE: /* It's a variable (4) */
				zone = new VariableZone(this);
				zone->analyze(node);
			break;
		case EF_FOOTNOTE: /* It's a footnote (5) */
				zone = new Footnote(this);
				zone->analyze(node);
			break;
		case EF_ANCHOR: /* It's an anchor (6) */
				zone = new Anchor(this);
				zone->analyze(node);
			break;
		default: /* Unknown */
				kDebug(30522) <<"Format not yet supported";
	}

	if(zone->getPos() != _currentPos)
	{
		if(_lines == 0)
			_lines = new Q3PtrList<Format>;
			/* Create first a default format */
		zoneFirst = new TextZone(_text, this);
		zoneFirst->setPos(_currentPos);
		zoneFirst->setLength(zone->getPos() - _currentPos);
		((TextZone*) zoneFirst)->analyze();
		kDebug(30522) <<"current position:" << _currentPos;
		/* Add the text without format */
		_lines->append(zoneFirst);
		_currentPos = _currentPos + zoneFirst->getLength();
	}

	if(zone != 0)
	{
		if(_lines == 0)
			_lines = new Q3PtrList<Format>;

		/* add the text */
		_lines->append(zone);
		_currentPos = _currentPos + zone->getLength();
	}
}

/*******************************************/
/* Generate                                */
/*******************************************/
/* Generate each text zone with the parag. */
/* markup.                                 */
/*******************************************/
void Para::generate(QTextStream &out)
{

	kDebug(30522) <<"  GENERATION PARA";

	if(getInfo() != EP_FOOTNOTE && getFrameType() != SS_HEADERS &&
	   getFrameType() != SS_FOOTERS)
	{
		/* We generate center, itemize tag and new page only for
		 * parag not for footnote
		 * If a parag. have a special format (beginning)
		 */
		if(isHardBreak())
			out << "\\newpage" << endl;
		generateDebut(out);
	}

	/* If text is a \n, then it's a break line. */
	if(_text == "\n")
		out << "\\\\" << endl;
	else if(_lines != 0)
	{
		Format* zone = 0;
		kDebug(30522) <<"  NB ZONE :" << _lines->count();

		for(zone = _lines->first(); zone != 0; zone = _lines->next())
		{
			zone->generate(out);
		}
		/* To separate the text zones. */
	}

	if(getInfo() != EP_FOOTNOTE && getFrameType() != SS_HEADERS &&
	   getFrameType() != SS_FOOTERS)
	{
		/* id than above : a parag. have a special format. (end)
		 * only it's not a header, nor a footer nor a footnote/endnote
		 */
		generateFin(out);
		if(isHardBreakAfter())
			out << "\\newpage" << endl;
	}
	kDebug(30522) <<"PARA GENERATED";
}

/*******************************************/
/* GenerateDebut                           */
/*******************************************/
/* Generate the beginning paragraph markup.*/
/*******************************************/
void Para::generateDebut(QTextStream &out)
{
	/* Be careful we are in a table !
	 * You can't use directly environment, ...
	 */
	if(getFrameType() == SS_TABLE)
	{
		//int sizeCell = 5;
		/* first number depends with the cell size (next number}
		 * and with the number of characters in the para.
		 * It can be 20 char. / 5 cm  = 4 char / cm so */
		/* nbLines = nb_char_para / (4 * cell size) + 1 */
		//sizeCell = (_element->getRight() - _element->getLeft()) / 27;
		//kDebug(30522) <<"SIZE OF CELL :" << sizeCell;
		// TODO : arrondir au superieur avec tgmath.h ??
		//_nbLines = ((_element->getBottom() - _element->getTop()) / 27) + 1;
		//kDebug(30522) <<"NB OF LINES :" << _nbLines;
		/* 2 at least, 1 for the line, 1 for the space line */
		/*if(_nbLines < 2)
			_nbLines = 2;
		out << "\\multirow{" << _nbLines << "}{"<< sizeCell << "cm}{" << endl;*/
	}
	/* if it's a chapter */
	if(isChapter())
	{
		/* switch the type, the depth do */
		generateTitle(out);
		Config::instance()->indent();
	}
	else if(isEnum())
	{
		Config::instance()->writeIndent(out);
		out << "\\item ";
	}
	else
		Config::instance()->writeIndent(out);
}

void Para::generateBeginEnv(QTextStream &out)
{
	kDebug(30522) <<"Begin new Env :" << getEnv();

	Config::instance()->writeIndent(out);

	switch(getEnv())
	{
		case ENV_LEFT: out << "\\begin{flushleft}" << endl;
			break;
		case ENV_RIGHT: out << "\\begin{flushright}" << endl;
			break;
		case ENV_CENTER: out << "\\begin{center}" << endl;
			break;
		case ENV_JUSTIFY: out << endl;
			break;
		case ENV_NONE:
			break;
	}
	
	Config::instance()->indent();
}

/*******************************************/
/* openList                                */
/*******************************************/
/* Generate the markup to begin a list and */
/* push the type in the historic stack.    */
/*******************************************/
void Para::openList(QTextStream &out)
{
	EType *type_temp = 0;

	Config::instance()->writeIndent(out);

	switch(getCounterType())
	{
		case TL_NONE:
			break;
		case TL_ARABIC:
			out << "\\begin{enumerate}" << endl;
			break;
		case TL_LLETTER:	/* a, b, ... */
			out << "\\begin{enumerate}[a]" << endl;
			break;
		case TL_CLETTER:	/* A, B, ... */
			out << "\\begin{enumerate}[A]" << endl;
			break;
		case TL_LLNUMBER:	/* i, ii, ... */
			out << "\\begin{enumerate}[i]" << endl;
			break;
		case TL_CLNUMBER: 	/* I, II, ... */
			out << "\\begin{enumerate}[I]" << endl;
			break;
		case TL_CUSTOM_SIMPLE: /* - */
			out << "\\begin{enumerate}[" << convertSpecialChar(getCounterBullet()) << "]" << endl;
			break;
		case TL_CUSTOM_COMPLEX: /* - */
			out << "\\begin{enumerate}[" << convertSpecialChar(getCounterBullet()) << "]" << endl;
			break;
		case TL_CIRCLE_BULLET:
			out << "\\begin{itemize}" << endl;
			break;
		case TL_SQUARE_BULLET:
			out << "\\begin{itemize}" << endl;
			break;
		case TL_DISC_BULLET:
			out << "\\begin{itemize}" << endl;
			break;
		default:
			out << "\\begin{itemize}[SPECIAL]" << endl;
	}

	Config::instance()->indent();

	/* Keep the list type */
	type_temp = new EType(getCounterType());
	kDebug(30522) <<" type list to open :" << *type_temp;
	_historicList.push(type_temp);
}

/*******************************************/
/* GenerateFin                             */
/*******************************************/
/* Generate the closing paragraph markup.  */
/*******************************************/
void Para::generateFin(QTextStream &out)
{
	/* Close a title of chapter */
	if(isChapter())
		out << "}";
}

/*******************************************/
/* GenerateEndEnv                          */
/*******************************************/
/* Generate the closing environment markup.*/
/*******************************************/
void Para::generateEndEnv(QTextStream &out)
{
	kDebug(30522) <<"end of an environment :" << getEnv();
	
	Config::instance()->desindent();
	
	switch(getEnv())
	{
		case ENV_LEFT:
				out << endl;
				Config::instance()->writeIndent(out);
				out << "\\end{flushleft}";
			break;
		case ENV_RIGHT:
				out << endl;
				Config::instance()->writeIndent(out);
				out << "\\end{flushright}";
			break;
		case ENV_CENTER:
				out << endl;
				Config::instance()->writeIndent(out);
				out << "\\end{center}";
			break;
		case ENV_JUSTIFY:
			break;
		case ENV_NONE:
			break;
	}

	Config::instance()->desindent();
}

/*******************************************/
/* closeList                               */
/*******************************************/
/* Generate the closing list markup for a  */
/* list type (letter, custom, ...) and     */
/* remove the last list saved.             */
/*******************************************/
void Para::closeList(QTextStream &out, Para* next)
{
	closeList(getCounterType(), out);

	if(((getCounterDepth() - 1) >= 0) && ((next!= 0 && !next->isEnum()) || next == 0))
	{
		/* We must close all the lists since
		 * after this paragraph it's a normal paragraph.
		 */
		kDebug(30522) <<"lists to close";
		while(!_historicList.isEmpty())
		{
			EType *type_temp = 0;
			type_temp = _historicList.pop();
			if(type_temp != 0)
				closeList(*type_temp, out);
		}
	}
}

/*******************************************/
/* closeList                               */
/*******************************************/
/* Generate the closing list markup for a  */
/* list type (letter, custom, ...) and     */
/* remove the last list saved.             */
/*******************************************/
void Para::closeList(EType type, QTextStream &out)
{
	//out << endl;
	kDebug(30522) <<" type list to close :" << type;

	/* Because of a new markup, we need a new line. */
	out << endl;

	Config::instance()->desindent();
	Config::instance()->writeIndent(out);
	
	/* but the next parag is not a same list */
	switch(type)
	{
		case TL_NONE: //out << endl;
			break;
		case TL_ARABIC:
		case TL_LLETTER:  /* a, b, ... */
		case TL_CLETTER:  /* A, B, ... P. 250*/
		case TL_LLNUMBER: /* i, ii, ... */
		case TL_CLNUMBER: /* I, II, ... */
		case TL_CUSTOM_SIMPLE: /* - */
		case TL_CUSTOM_COMPLEX: /* - */
			       out << "\\end{enumerate}" << endl;
			break;
		case TL_CIRCLE_BULLET:
				out << "\\end{itemize}" << endl;
			break;
		case TL_SQUARE_BULLET:
		case TL_DISC_BULLET:
				out << "\\end{itemize}" << endl;
			break;
		default:
				out << "no suported" << endl;
	}

	Config::instance()->writeIndent(out);

	/* Pop the list which has been closed */
	_historicList.remove();
	kDebug(30522) <<"removed";
}

/*******************************************/
/* GenerateTitle                           */
/*******************************************/
void Para::generateTitle(QTextStream &out)
{
	switch(getCounterDepth())
	{
		case 0:
			out << "\\section{";
			break;
		case 1:
			out << "\\subsection{";
			break;
		case 2:
			out << "\\subsubsection{";
			break;
		case 3:
			out << "\\paragraph{";
			break;
		case 4:
			out << "\\subparagraph{";
			break;
		case 5:
			out << "% section too deep" << endl;
			out << "\\textbf{";
	}
}
