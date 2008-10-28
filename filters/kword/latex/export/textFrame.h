/* A TEXT IS A SET OF TITLES, A (SET OF) PARAGRAPHS OR LISTS */
/*
** Header file for inclusion with kword_xml2latex.c
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
** Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
**
*/

#ifndef __KWORD_LATEX_FRAMETEXT_H__
#define __KWORD_LATEX_FRAMETEXT_H__

#include <q3ptrlist.h>
//Added by qt3to4:
#include <QTextStream>
#include "element.h"
//#include "border.h"
#include "para.h"

/***********************************************************************/
/* Class: TextFrame                                                    */
/***********************************************************************/

/**
 * This class holds 2 lists of paragraphs for the text and for the footnotes.
 * NOTE: The frame analysis can be (must be) in an Element class.
 */
class TextFrame: public Element
{
	/* DATA MARKUP */
	double  _left,
		 	_top,
		 	_right,
		 	_bottom;
	TAround _runaround;
	double  _runaroundGap;
	TCreate _autoCreate;
	TNFrame _newFrameBehaviour;
	TSide   _sheetSide;

	/* CHILD MARKUP */
	Q3PtrList<Para> _parags;
	//QPtrList<Para> _footnotes;

	/* USEFUL DATA */
	EEnv      _lastEnv;
	EType     _lastTypeEnum;

	public:
		/**
		 * Constructors
		 *
		 * Creates a new instances of a Text frame.
		 *
		 */
		TextFrame();

		/* 
		 * Destructor
		 *
		 * The destructor must remove the list of parag and footnotes.
		 */
		virtual ~TextFrame() 
		{
			//delete _footnotes;
			kDebug(30522) <<"Destruction of a txt frame";
		}

		/**
		 * Accessors
		 */
		/*bool    hasColor      () const;
		bool    hasUline      () const;*/
		double  getLeft       () const { return _left;              }
		double  getRight      () const { return _right;             }
		double  getTop        () const { return _top;               }
		double  getBottom     () const { return _bottom;            }
		TAround getRunAround  () const { return _runaround;         }
		double  getAroundGap  () const { return _runaroundGap;      }
		TCreate getAutoCreate () const { return _autoCreate;        }
		TNFrame getNewFrame   () const { return _newFrameBehaviour; }
		TSide   getSheetSide  () const { return _sheetSide;         }
		Para*   getFirstPara  () const { return _parags.getFirst(); }
		EEnv    getNextEnv    (Q3PtrList<Para>, const int);
		bool    isBeginEnum   (Para*, Para*);
		bool    isCloseEnum   (Para*, Para*);

		/**
		 * Modifiors
		 */
		void setLeft      (const double l)    { _left   = l;               }
		void setRight     (const double r)    { _right  = r;               }
		void setTop       (const double t)    { _top    = t;               }
		void setBottom    (const double b)    { _bottom = b;               }

		void setRunAround (const int a)    { _runaround = (TAround) a;  }
		void setAroundGap (const double r) { _runaroundGap = r;         }
		void setAutoCreate(const int a)    { _autoCreate = (TCreate) a; }
		void setNewFrame  (const int n)    { _newFrameBehaviour = (TNFrame) n; }
		void setSheetSide (const int s)    { _sheetSide = (TSide) s;    }

		//Para* searchFootnote(const QString);

		/**
		 * Get information from a markup tree.
		 */
		void analyze(const QDomNode);

		/**
		 * Write the text in a file.
		 */
		void generate(QTextStream&);

	private:
		/**
		 * Get information from a markup tree (only parameters
		 * in a frame).
		 */
		void analyzeParamFrame(const QDomNode);

};

#endif /* __KWORD_LATEX_FRAMETEXT_H__ */

