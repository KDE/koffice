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

#include "KoRdfSemanticItemViewSite.h"
#include "KoDocumentRdf.h"
#include "KoDocumentRdf_p.h"
#include "KCanvasBase.h"
#include "KToolProxy.h"
#include "KResourceManager.h"
#include "KOdfText.h"
#include "KoTextEditor.h"
#include <kdebug.h>

using namespace Soprano;

class KoRdfSemanticItemViewSitePrivate
{
public:
    QString m_xmlid;
    KoRdfSemanticItem *m_semItem;
    KoRdfSemanticItemViewSitePrivate(KoRdfSemanticItem *si, const QString &xmlid)
        : m_xmlid(xmlid)
        , m_semItem(si)
        {
        }
};


KoRdfSemanticItemViewSite::KoRdfSemanticItemViewSite(KoRdfSemanticItem *si, const QString &xmlid)
    :
    d (new KoRdfSemanticItemViewSitePrivate(si,xmlid))
{
}

KoRdfSemanticItemViewSite::~KoRdfSemanticItemViewSite()
{
    delete d;
}

Soprano::Node KoRdfSemanticItemViewSite::linkingSubject() const
{
    KoDocumentRdf *rdf = d->m_semItem->documentRdf();
    Soprano::Model *m = rdf->model();
    Node pred(QUrl("http://koffice.org/rdf/site/package/common#idref"));
    Node obj = Node::createLiteralNode(d->m_xmlid);
    Node context = rdf->manifestRdfNode();
    // try to find it if it already exists
    StatementIterator it = m->listStatements(Node(), pred, obj, context);
    QList<Statement> allStatements = it.allElements();
    foreach (const Soprano::Statement &s, allStatements) {
        return s.subject();
    }
    Node ret = m->createBlankNode();
    m->addStatement(ret, pred, obj, context);
    return ret;
}

QString KoRdfSemanticItemViewSite::getProperty(const QString &prop, const QString &defval) const
{
    Soprano::Node ls = linkingSubject();
    QString fqprop = "http://koffice.org/rdf/site#" + prop;
    KoDocumentRdf *rdf = d->m_semItem->documentRdf();
    Soprano::Model *m = rdf->model();
    StatementIterator it = m->listStatements(ls, Node::createResourceNode(QUrl(fqprop)),
                               Node(), rdf->manifestRdfNode());
    QList<Statement> allStatements = it.allElements();
    foreach (const Soprano::Statement &s, allStatements) {
        return s.object().toString();
    }
    return defval;
}

void KoRdfSemanticItemViewSite::setProperty(const QString &prop, const QString &v)
{
    QString fqprop = "http://koffice.org/rdf/site#" + prop;
    KoDocumentRdf *rdf = d->m_semItem->documentRdf();
    Soprano::Model *m = rdf->model();
    Soprano::Node ls = linkingSubject();
    Soprano::Node pred = Node::createResourceNode(QUrl(fqprop));
    m->removeAllStatements(Statement(ls, pred, Node()));
    m->addStatement(ls, pred,Node::createLiteralNode(v), rdf->manifestRdfNode());
}

KoSemanticStylesheet *KoRdfSemanticItemViewSite::stylesheet() const
{
    QString name = getProperty("stylesheet", "name");
    QString type = getProperty("stylesheet-type", KoSemanticStylesheet::stylesheetTypeSystem());
    QString uuid = getProperty("stylesheet-uuid", "");
    kDebug(30015) << "stylesheet at site, format(), xmlid:" << d->m_xmlid;
    kDebug(30015) << " sheet:" << name << " type:" << type;
    KoSemanticStylesheet *ret(0);
    if (!uuid.isEmpty()) {
        ret = d->m_semItem->findStylesheetByUuid(uuid);
    }
    if (!ret) {
        ret = d->m_semItem->findStylesheetByName(type, name);
    }
    if (!ret) {
        // safety first, there will always be a default stylesheet
        ret = d->m_semItem->defaultStylesheet();
    }
    Q_ASSERT(ret);
    return ret;
}

void KoRdfSemanticItemViewSite::applyStylesheet(KoTextEditor *editor, KoSemanticStylesheet *ss)
{
    // Save the stylesheet property and cause a reflow.
    kDebug(30015) << "apply stylesheet at site. format(), xmlid:" << d->m_xmlid << " sheet:" << ss->name();
    setStylesheetWithoutReflow(ss);
    reflowUsingCurrentStylesheet(editor);
}

void KoRdfSemanticItemViewSite::disassociateStylesheet()
{
    kDebug(30015) << "stylesheet at site. xmlid:" << d->m_xmlid;
    setProperty("stylesheet", "");
    setProperty("stylesheet-type", "");
    setProperty("stylesheet-uuid", "");
}

void KoRdfSemanticItemViewSite::setStylesheetWithoutReflow(KoSemanticStylesheet *ss)
{
    // Save the stylesheet property
    kDebug(30015) << "apply stylesheet at site. format(), xmlid:" << d->m_xmlid << " sheet:" << ss->name();
    setProperty("stylesheet", ss->name());
    setProperty("stylesheet-type", ss->type());
    setProperty("stylesheet-uuid", ss->uuid());
}

void KoRdfSemanticItemViewSite::reflowUsingCurrentStylesheet(KoTextEditor *editor)
{
    KoSemanticStylesheet *ss = stylesheet();
    ss->format(d->m_semItem, editor, d->m_xmlid);
}

void KoRdfSemanticItemViewSite::selectRange(KResourceManager *provider, int startpos, int endpos)
{
    kDebug(30015) << " startpos:" << startpos << " endpos:" << endpos;
    if (endpos > startpos) {
        provider->setResource(KOdfText::CurrentTextPosition, startpos);
        provider->setResource(KOdfText::CurrentTextAnchor, endpos + 1);
        provider->clearResource(KOdfText::SelectedTextPosition);
        provider->clearResource(KOdfText::SelectedTextAnchor);
    } else {
        provider->setResource(KOdfText::CurrentTextPosition, startpos);
    }
}

void KoRdfSemanticItemViewSite::select(KCanvasBase *host)
{
    Q_ASSERT(d->m_semItem);
    Q_ASSERT(d->m_semItem->documentRdf());
    Q_ASSERT(host);
    KoTextEditor *editor = KoDocumentRdf::ensureTextTool(host);
    KResourceManager *provider = host->resourceManager();
    KoDocumentRdf *rdf = d->m_semItem->documentRdf();
    QPair<int, int> p = p = rdf->findExtent(d->m_xmlid);
    int startpos = p.first;
    int endpos = p.second + 1;
    if (!endpos) {
        kDebug(30015) << "No end point found for semantic item:" << d->m_semItem->name();
        kDebug(30015) << "xmlid:" << d->m_xmlid;
        return;
    }
    kDebug(30015) << "xmlid:" << d->m_xmlid;
    kDebug(30015) << "start:" << startpos << " endpos:" << endpos;
    selectRange(provider, startpos, endpos);
    kDebug(30015) << "selected text:" << editor->selectedText();
}
