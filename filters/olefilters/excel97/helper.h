/* This file is part of the KDE project

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
 */

#ifndef HELPER_H
#define HELPER_H

#include <qdatastream.h>
#include <qptrlist.h>
#include <qintdict.h>
#include <qdom.h>

#include <klocale.h>

#include "definitions.h"

class MergeInfo
{
public:
	MergeInfo(int fr, int lr, int fc, int lc)
	{ m_fr = fr; m_lr = lr; m_fc = fc; m_lc = lc; }
	
	int row() { return m_fr + 1; }
	int col() { return m_fc + 1; }

	int rowspan() { return m_lr - m_fr; }
	int colspan() { return m_lc - m_fc; }

private:
	int m_fr, m_lr, m_fc, m_lc;
};

class PenFormat
{
public:
	PenFormat() { }

	void setWidth(int width) { m_width = width; }
	void setStyle(int style) { m_style = style; }

	int width() { return m_width; }
	int style() { return m_style; }

private:
	int m_width, m_style;
};

class Helper
{
public:
	Helper(QDomDocument *root, QPtrList<QDomElement> *tables);
	~Helper();

	void addDict(Dictionary dict, int index, void *obj);
	void *queryDict(Dictionary dict, int index);
	
	void getTime(double time, int &hour,int  &min, int &second);
	void getDate(int date, int &year, int &month, int &day, Q_UINT16 date1904);
	void getFont(Q_UINT16, QDomElement &f, Q_UINT16 fontid);
	void getPen(Q_UINT16 xf, QDomElement &f, Q_UINT16 fontid);
	
	const QString getFormula(Q_UINT16 row, Q_UINT16 column, QDataStream &rgce, Q_UINT16 biff);
	const QDomElement getFormat(Q_UINT16 xf);

	PenFormat borderStyleToQtStyle(int penStyle);

	KLocale locale() { return m_locale; }
	
private:
	QDomDocument *m_root;
	QPtrList<QDomElement> *m_tables;
	
	QIntDict<xfrec> m_xfrec;
	QIntDict<QString> m_sstrec;
	QIntDict<fontrec> m_fontrec;
	QIntDict<formatrec> m_formatrec;	

	KLocale m_locale;
};

#endif
