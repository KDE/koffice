
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
** Free Software Foundation, Inc., 59 Temple Place - Suite 330,
** Boston, MA  02111-1307, USA.
**
*/

#ifndef __KSPREAD_LATEX_DOCUMENT_H__
#define __KSPREAD_LATEX_DOCUMENT_H__

#include <qfile.h>			/* for QFile class */
#include <qtextstream.h>	/* for QTextStream class */
#include <qstring.h>		/* for QString class */

#include "xmlparser.h"
#include "config.h"
#include "spreadsheet.h"

class Document : public XmlParser, Config
{
	QFile       _file;
	QTextStream _out;
	QString     _filename;
	const KoStore*    _in;

	//FileHeader  _header;
	Spreadsheet _document;

	/* Options */
	

	public:
		/**
		 * @param in tar file.
		 * @param fileOut Output latex filename.
		 */
		Document(const KoStore*, QString);

		virtual ~Document();

		/* Accesors */
		
		void analyse();
		void generate();

	private:

};

#endif /* __KSPREAD_LATEX_DOCUMENT_H__ */
