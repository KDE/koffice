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

#include <stdlib.h>		/* for atoi function */
#include <kdebug.h>		/* for kdDebug() stream */
#include <qptrstack.h>		/* for getFormula() */
#include <qdom.h>
#include <kformulamimesource.h>
#include "formula.h"

/*******************************************/
/* Constructor                             */
/*******************************************/
Formula::Formula()
{
	_left              = 0;
	_right             = 0;
	_top               = 0;
	_bottom            = 0;
	_runaround         = TA_NONE;
	_runaroundGap      = 0;
	_autoCreate        = TC_EXTEND;
	_newFrameBehaviour = TF_RECONNECT;

}

/*******************************************/
/* analyse                                 */
/*******************************************/
void Formula::analyse(const QDomNode balise)
{

	/* MARKUP TYPE : FRAMESET INFO = TEXTE, ENTETE CONNUE */

	/* Parameters Analyse */
	Element::analyse(balise);

	kdDebug() << "FRAME ANALYSE (Formula)" << endl;

	/* Chlidren markups Analyse */
	for(int index= 0; index < getNbChild(balise); index++)
	{
		if(getChildName(balise, index).compare("FRAME")== 0)
		{
			analyseParamFrame(balise);
		}
		else if(getChildName(balise, index).compare("FORMULA")== 0)
		{
			getFormula(getChild(getChild(balise, "FORMULA"), "FORMULA"), 0);
			kdDebug() << _formula << endl;
		}

	}
	kdDebug() << "END OF A FRAME" << endl;
}

/*******************************************/
/* getFormula                              */
/*******************************************/
/* Get back the xml markup tree.           */
/*******************************************/
void Formula::getFormula(QDomNode p, int indent)
{
/*	while( p.)
	{*/
		switch( p.nodeType() )
		{
			case QDomNode::TextNode:
				_formula = _formula + QString(p.toText().data()) + " ";
				break;
		/*	case TT_Space:
				_formula = _formula + p->zText;
				//printf("%*s\"%s\"\n", indent, "", p->zText);
				break;
			case TT_EOL:
				_formula = _formula + "\n";
				//printf("%*s\n", indent, "");
				break;*/
			case QDomNode::ElementNode:
				_formula = _formula + "<" + p.nodeName();
				QDomNamedNodeMap attr = p.attributes();
				for(int index = 0; index < attr.length(); index++)
				{ // The attributes
					_formula = _formula + " " + attr.item(index).nodeName();
					_formula = _formula + "=\"" + attr.item(index).nodeValue() + "\"";
				}
				if(p.childNodes().length() == 0)
					_formula = _formula + "/>\n";
				else
				{
					_formula = _formula + ">\n";
					QDomNodeList child = p.childNodes();
					for(int index = 0; index < child.length(); index++)
					{
						getFormula(child.item(index), indent+3); // The child elements
					}
					_formula = _formula + "</" + p.nodeName() + ">\n";
				}
				break;
			/*default:
				kdError() << "Can't happen" << endl;
				break;*/
		}
	/*	p = p.nextSibling();
	}*/
}

/*******************************************/
/* analyseParamFrame                       */
/*******************************************/
void Formula::analyseParamFrame(const QDomNode balise)
{
	/*<FRAME left="28" top="42" right="566" bottom="798" runaround="1" />*/

	_left = getAttr(balise, "left").toInt();
	_top = getAttr(balise, "top").toInt();
	_right = getAttr(balise, "right").toInt();
	_bottom = getAttr(balise, "bottom").toInt();
	setRunAround(getAttr(balise, "runaround").toInt());
	setAroundGap(getAttr(balise, "runaroundGap").toInt());
	setAutoCreate(getAttr(balise, "autoCreateNewFrame").toInt());
	setNewFrame(getAttr(balise, "newFrameBehaviour").toInt());
	setSheetSide(getAttr(balise, "sheetside").toInt());
}

/*******************************************/
/* generate                                */
/*******************************************/
void Formula::generate(QTextStream &out)
{
	kdDebug() << "FORMULA GENERATION" << endl;
	QDomDocument *doc = new QDomDocument();
	doc->setContent(_formula);
	KFormula::MimeSource *formula =
		new KFormula::MimeSource(*doc);
	kdDebug() << QString(formula->encodedData("text/x-tex")) << endl;
	out << "$" << QString(formula->encodedData("text/x-tex")) << "$";
}

