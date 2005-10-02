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

#ifndef __KSPREAD_LATEX_MAP_H__
#define __KSPREAD_LATEX_MAP_H__

#include <qstring.h>
#include <qptrstack.h>		/* historic list */
#include <qptrlist.h>		/* for list of format */

#include "xmlparser.h"
#include "config.h"
#include "table.h"

/***********************************************************************/
/* Class: Map                                                         */
/***********************************************************************/

/**
 * This class hold a real paragraph. It tells about the text in this
 * paragraph, its format, etc. The complete text is a list of Map instances.
 * A footnote is a list of paragraph instances (now but not in the "futur").
 */
class Map: public XmlParser, Config
{
	QPtrList<Table> _tables;

	public:
		/**
		 * Constructors
		 *
		 * Creates a new instance of Map.
		 *
		 * @param Texte the text this paragraph is belonging to.
		 */
		Map();

		/* 
		 * Destructor
		 *
		 * The destructor must remove the list of little zones.
		 */
		virtual ~Map();

		/**
		 * Accessors
		 */

		/**
		 * Modifiers
		 */

		/**
		 * Helpfull functions
		 */

		/**
		 * Get informations from a markup tree.
		 */
		void analyse         (const QDomNode);

		/**
		 * Write the paragraph in a file.
		 */
		void generate        (QTextStream&);

	
	private:

};

#endif /* __KSPREAD_LATEX_MAP_H__ */
