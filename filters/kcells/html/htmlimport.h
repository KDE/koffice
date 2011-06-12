/* This file is part of the KDE project
   Copyright (C) 2001 Eva Brucherseifer <eva@kde.org>
   Copyright (C) 2005 Bram Schoenmakers <bramschoenmakers@kde.nl>
   based on kcells csv export filter by David Faure

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
 * Boston, MA 02110-1301, USA.
*/

#ifndef HTMLIMPORT_H
#define HTMLIMPORT_H

#include <QObject>
#include <KoFilter.h>
#include <QDir>
#include <KUrl>
#include <QByteArray>
#include <QStack>
#include <QVariantList>

// namespace KCells
// {
// class Sheet;
// }

namespace DOM
{
    class Node;
    class Element;
}

class KoOdfWriteStore;
class KoXmlWriter;
class KOdfGenericStyles;

class HTMLImport : public KoFilter
{
    Q_OBJECT
public:
    HTMLImport(QObject* parent, const QVariantList&);
    virtual ~HTMLImport();

    virtual KoFilter::ConversionStatus convert(const QByteArray& from, const QByteArray& to);

Q_SIGNALS:
    void completed();

private:
    KoFilter::ConversionStatus loadUrl(const KUrl &url);
    void parseNode(DOM::Node n);
    bool parseTag(DOM::Element e);
    bool createStyle();
    bool createMeta();
private:
    enum State { InNone, InFrameset, InBody, InTable, InRow, InCell };
    QStack<State> m_states;
    
    QDir m_inputDir;    
    KoOdfWriteStore* m_store;
    KoXmlWriter* m_manifestWriter;
    KOdfGenericStyles* m_mainStyles;
};

#endif

