/* This file is part of the KDE project
   Copyright (C) 1998, 1999, 2000 Torben Weis <weis@kde.org>

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

#ifndef __koml_parser_wrapper_h__
#define __koml_parser_wrapper_h__

#include <qdom.h>
#include <qvaluestack.h>

#include <string>
#include <vector>

using std::string;

struct KOMLAttrib
{
    string m_strName;
    string m_strValue;
};

class KOMLParser
{
public:
    KOMLParser( const QDomDocument& doc );
    virtual ~KOMLParser();

    /**
     * @param _search is the opening tag we are searching for. If this is 0L
     *                every opening tag is matched.
     * @param _tag holds the matched tag.
     */
    bool open( const char *_search, string& unused );
    bool close( string& unused );

    bool readText( string& text );

    bool parseTag( const char *_tag, string& tagname, std::vector<KOMLAttrib>& _attribs );

    /**
     * @return the node the parser is currently on.
     */
    QDomNode currentNode();
    /**
     * @return the node the parser is currently on.
     */
    QDomElement currentElement();
    
protected:
    QDomDocument m_doc;
    QDomNode m_node;
    QValueStack<QDomNode> m_stack;
};

#endif
