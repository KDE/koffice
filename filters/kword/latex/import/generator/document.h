/* This file is part of the KDE project
 * Copyright (C) 2002, 2003 Robert JACOLIN <rjacolin@ifrance.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef __KWORD_LATEX_IMPORT_DOCUMENT__
#define __KWORD_LATEX_IMPORT_DOCUMENT__

#include <element.h>
#include <q3ptrlist.h>
#include "body.h"

class KoStore;
class QDomNode;
class Command;

class Document
{
	public:
		Document()
		{
		}

		~Document()
		{
		}

		/**
		 * Analyze the latex tree and keep data.
		 * 
		 * @param elt The root tree which contaisn an entire latex document.
		 * 
		 * @return <code>false</code> if the analysis failed.
		 */
		bool analyze(Q3PtrList<Element>* elt);

		/**
		 * Analyze and get all information in a documentclass command.
		 */
		bool analyzeDocumentClass(Command* documentclass);

		/**
		 * Generate a kword document.
		 *
		 * @param store The kword document zip file.
		 *
		 * @return <code>true</code> if the generation succeeded.
		 */
		bool generate(KoStore* store);

		/**
		 * Copy the DOM tree in a file.
		 *
		 * @param tree The tree to copy.
		 * @param store The KoStore that this doc is from
		 */
		void serialize(KoStore* store, QDomDocument tree);

	private:
		Body _body;
};

#endif /* __KWORD_LATEX_IMPORT_DOCUMENT__ */
