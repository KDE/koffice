/* This file is part of the KDE project
   Copyright (C) 2010 KO GmbH <ben.martin@kogmbh.com>

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

#include "RdfPrefixMapping.h"
#include <KoTextRdfCore.h>
#include <Soprano/Soprano>
#include <kdebug.h>
using namespace Soprano;

RdfPrefixMapping::RdfPrefixMapping(KoDocumentRdf *rdf)
        : m_rdf(rdf)
{
    insert("pkg", "http://docs.oasis-open.org/opendocument/meta/package/common#");
    insert("odf", "http://docs.oasis-open.org/opendocument/meta/package/odf#");
    insert("rdf", "http://www.w3.org/1999/02/22-rdf-syntax-ns#");
    insert("xhtml", "http://www.w3.org/1999/xhtml");
    insert("xsd", "http://www.w3.org/2001/XMLSchema");
    insert("xsi", "http://www.w3.org/2001/XMLSchema-instance");
    insert("foaf", "http://xmlns.com/foaf/0.1/");
    insert("geo84", "http://www.w3.org/2003/01/geo/wgs84_pos#");
    insert("dcterms", "http://dublincore.org/documents/dcmi-terms/#");
    insert("cite", "http://docs.oasis-open.org/prototype/opendocument/citation#");
    insert("cal", "http://www.w3.org/2002/12/cal/icaltzd#");
}

RdfPrefixMapping::~RdfPrefixMapping()
{
}

QString RdfPrefixMapping::canonPrefix(const QString &pname) const
{
    QString name = pname;
    int idx = name.indexOf(':');
    if (idx >= 0) {
        name = name.left(idx + 1);
    }
    if (!name.endsWith(':')) {
        name += ':';
    }
    return name;
}


QString RdfPrefixMapping::URItoPrefexedLocalname(const QString &uri) const
{
    for (QMap<QString,QString>::const_iterator mi = m_mappings.begin(); mi != m_mappings.end(); ++mi) {
        if (uri.startsWith(mi.value())) {
            QString ret = mi.key() + uri.mid(mi.value().length());
            return ret;
        }
    }
    return uri;
}

QString RdfPrefixMapping::PrefexedLocalnameToURI(const QString &pname) const
{
    QString pfx = canonPrefix(pname);
    if (pfx.isEmpty()) {
        return pname;
    }
    QMap<QString,QString>::const_iterator mi = m_mappings.find(pfx);
    if (mi == m_mappings.end())
        return pname;
    return mi.value() + pname.mid(mi.key().length());
}

QString RdfPrefixMapping::prefexToURI(const QString &pfx) const
{
    QString prefix = canonPrefix(pfx);
    QMap<QString,QString>::const_iterator mi = m_mappings.find(prefix);
    if (mi == m_mappings.end()) {
        return QString();
    }
    return mi.value();
}

void RdfPrefixMapping::dump() const
{
    kDebug(30015) << m_mappings;
}

void RdfPrefixMapping::insert(const QString &prefix, const QString &url)
{
    QString fixedPrefix = canonPrefix(prefix);
    kDebug(30015) << " prefix:" << fixedPrefix << " url:" << url;
    m_mappings.insert(fixedPrefix, url);
}

void RdfPrefixMapping::remove(const QString &prefix)
{
    QString fixedPrefix = canonPrefix(prefix);
    kDebug(30015) << " prefix:" << fixedPrefix;
    m_mappings.remove(fixedPrefix);
}

void RdfPrefixMapping::load(Soprano::Model *model)
{
    QString nodePrefix = "http://kogmbh.net/rdf/prefixmapping/";
    Node rdfNil = Node::createResourceNode(QUrl("http://www.w3.org/1999/02/22-rdf-syntax-ns#nil"));
    Node rdfFirst = Node::createResourceNode(QUrl("http://www.w3.org/1999/02/22-rdf-syntax-ns#first"));
    Node rdfRest = Node::createResourceNode(QUrl("http://www.w3.org/1999/02/22-rdf-syntax-ns#rest"));
    Soprano::Node ListHeadSubject = Node::createResourceNode(QUrl(nodePrefix + "list"));
    QList<Statement> listNodes = KoTextRdfCore::loadList(model, ListHeadSubject);
    kDebug(30015) << "found count:" << listNodes.size();
    foreach (Soprano::Statement s, listNodes) {
        Soprano::Node dataBNode = s.object();
        QString prefix = KoTextRdfCore::getObject(model, dataBNode,
                         Node::createResourceNode(QUrl(nodePrefix + "prefix"))).toString();
        QString url = KoTextRdfCore::getObject(model, dataBNode,
                                               Node::createResourceNode(QUrl(nodePrefix + "url"))).toString();
        kDebug(30015) << "found prefix:" << prefix << " url:" << url;
        insert(prefix, url);
    }
}

void RdfPrefixMapping::save(Soprano::Model *model, Soprano::Node context) const
{
    QString nodePrefix = "http://kogmbh.net/rdf/prefixmapping/";
    Node rdfNil = Node::createResourceNode(QUrl("http://www.w3.org/1999/02/22-rdf-syntax-ns#nil"));
    Node rdfFirst = Node::createResourceNode(QUrl("http://www.w3.org/1999/02/22-rdf-syntax-ns#first"));
    Node rdfRest = Node::createResourceNode(QUrl("http://www.w3.org/1999/02/22-rdf-syntax-ns#rest"));
    Soprano::Node dataBNode = model->createBlankNode();
    QList< Soprano::Node > dataBNodeList;
    QMap<QString,QString>::const_iterator mi = m_mappings.begin();
    QMap<QString,QString>::const_iterator me = m_mappings.end();
    for (; mi != me; ++mi) {
        kDebug(30015) << "saving prefix:" << mi.key() << " url:" << mi.value();
        dataBNode = model->createBlankNode();
        model->addStatement(dataBNode,Node::createResourceNode(QUrl(nodePrefix + "prefix")),
            Node::createLiteralNode(mi.key()), context);
        model->addStatement(dataBNode, Node::createResourceNode(QUrl(nodePrefix + "url")),
            Node::createLiteralNode(mi.value()), context);
        dataBNodeList << dataBNode;
    }
    Soprano::Node ListHeadSubject = Node::createResourceNode(QUrl(nodePrefix + "list"));
    KoTextRdfCore::saveList(model, ListHeadSubject, dataBNodeList, context);
}
