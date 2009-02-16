/* This file is part of the KDE project
   Copyright (C) 2005-2006 Ariya Hidayat <ariya@kde.org>
   Copyright (C) 2007 Thorsten Zachmann <zachmann@kde.org>

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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KOXMLREADERFORWARD_H
#define KOXMLREADERFORWARD_H

// use standard QDom, useful to test KoXml classes against Qt's QDom
//#define KOXML_USE_QDOM

#ifdef KOXML_USE_QDOM

#include <QtXml/qdom.h>

typedef QDomNode KoXmlNode;
typedef QDomElement KoXmlElement;
typedef QDomText KoXmlText;
typedef QDomCDATASection KoXmlCDATASection;
typedef QDomDocumentType KoXmlDocumentType;
typedef QDomDocument KoXmlDocument;

#else

class KoXmlElement;
class KoXmlNode;
class KoXmlText;
class KoXmlCDATASection;
class KoXmlDocumentType;
class KoXmlDocument;

#endif

#endif // KOXMLREADERFORWARD_H
