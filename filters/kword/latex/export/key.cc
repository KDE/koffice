/*
** A program to convert the XML rendered by KWord into LATEX.
**
** Copyright (C) 2002 Robert JACOLIN
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

#include <kdebug.h>		/* for kdDebug() stream */
#include "key.h"

/*******************************************/
/* Constructor                             */
/*******************************************/
Key::Key(eKeyType type): _type(type)
{
	_name = "";
	_filename = "";
}

/*******************************************/
/* Destructor                              */
/*******************************************/
Key::~Key()
{
	kdDebug() << "Destruction of a key." << endl;
}

/*******************************************/
/* Analyse                                 */
/*******************************************/
void Key::analyse(const QDomNode balise)
{
	/* MARKUP TYPE :  PARAGRAPH */

	kdDebug() << "**** KEY ****" << endl;
	kdDebug() << getAttr(balise, "name") << endl;
	setName(getAttr(balise, "name"));
	setFilename(getAttr(balise, "filename"));
	setHour(getAttr(balise, "hour").toInt());
	setMSec(getAttr(balise, "msec").toInt());
	setDay(getAttr(balise, "day").toInt());
	setMinute(getAttr(balise, "minute").toInt());
	setSecond(getAttr(balise, "second").toInt());
	setMonth(getAttr(balise, "month").toInt());
	setYear(getAttr(balise, "year").toInt());
	kdDebug() << "**** END KEY ****" << endl;
}

/*******************************************/
/* Generate                                */
/*******************************************/
/* Generate each text zone with the parag. */
/* markup.                                 */
/*******************************************/
void Key::generate(QTextStream &out)
{

	kdDebug() << "  GENERATION KEY" << endl;

	kdDebug() << "PARA KEY" << endl;
}
