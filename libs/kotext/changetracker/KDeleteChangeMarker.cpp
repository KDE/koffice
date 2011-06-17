/* This file is part of the KDE project
 * Copyright (C) 2009 Pierre Stirnweiss <pstirnweiss@googlemail.com>
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

#include "KDeleteChangeMarker.h"

//KOffice includes
#include <KTextDocument.h>
#include <KXmlReader.h>
#include <KXmlWriter.h>
#include <KTextShapeSavingContext.h>
#include <KShapeLoadingContext.h>
#include <opendocument/KTextSharedSavingData.h>
#include "KChangeTrackerElement.h"
#include "KChangeTracker.h"

//KDE includes
#include <kdebug.h>

//Qt includes
#include <QFontMetrics>
#include <QTextDocument>
#include <QTextInlineObject>
#include <QPainter>

/*********************************** ODF Bug Work-around code **********************************************/
const QString KDeleteChangeMarker::RDFListName("http://www.koffice.org/list#");
const QString KDeleteChangeMarker::RDFListItemName("http://www.koffice.org/list-item#");
const QString KDeleteChangeMarker::RDFListValidity("http://www.kofficde.org/list-status#valid");
const QString KDeleteChangeMarker::RDFListItemValidity("http://www.koffice.org/list-item-status#valid");
const QString KDeleteChangeMarker::RDFListLevel("http://www.koffice.org/list-status#level");
const QString KDeleteChangeMarker::RDFDeleteChangeContext("http://www.koffice.org/deleteChangeMetadata");
/***********************************************************************************************************/

class KDeleteChangeMarker::Private
{
public:
    Private() {}

    KChangeTracker *changeTracker;
    QString text;
    int id;
    QString deleteChangeXml;
    QHash<KListStyle::ListIdType, KListStyle *> deletedListStyles;
};

KDeleteChangeMarker::KDeleteChangeMarker(KChangeTracker* changeTracker)
        : d(new Private())
{
    d->changeTracker = changeTracker;
}

KDeleteChangeMarker::~KDeleteChangeMarker()
{
    delete d;
}
/*
void KDeleteChangeMarker::setText (const QString& text)
{
    d->text = text;
}

QString KDeleteChangeMarker::text() const
{
    return d->text;
}
*/
void KDeleteChangeMarker::setChangeId (int id)
{
    d->id = id;
}

int KDeleteChangeMarker::changeId() const
{
    return d->id;
}

int KDeleteChangeMarker::position() const
{
    return textPosition();
}

bool KDeleteChangeMarker::loadOdf(const KXmlElement &element, KShapeLoadingContext &context)
{
    Q_UNUSED(element)
    Q_UNUSED(context);
    return false;
}

void KDeleteChangeMarker::paint(QPainter& painter, QPaintDevice *pd, const QRectF &rect, QTextInlineObject object, const QTextCharFormat &format)
{
    Q_UNUSED(painter);
    Q_UNUSED(pd);
    Q_UNUSED(rect);
    Q_UNUSED(object);
    Q_UNUSED(format);
}

void KDeleteChangeMarker::resize(QTextInlineObject object, const QTextCharFormat &format, QPaintDevice *pd)
{
    Q_UNUSED(object);
    Q_UNUSED(format);
    Q_UNUSED(pd);

    object.setWidth(0);
}

void KDeleteChangeMarker::updatePosition(QTextInlineObject object, const QTextCharFormat &format)
{
    Q_UNUSED(object);
    Q_UNUSED(format);
}

void KDeleteChangeMarker::saveOdf(KShapeSavingContext &context)
{
    KOdfGenericChange change;
    if (d->changeTracker->saveFormat() == KChangeTracker::ODF_1_2) {
        change.setChangeFormat(KOdfGenericChange::ODF_1_2);
    } else {
        change.setChangeFormat(KOdfGenericChange::DELTAXML);
    }

    QString changeName;
    KTextSharedSavingData *sharedData = 0;
    if (context.sharedData(KOTEXT_SHARED_SAVING_ID)) {
        sharedData = dynamic_cast<KTextSharedSavingData*>(context.sharedData(KOTEXT_SHARED_SAVING_ID));
        if (!sharedData) {
            kWarning(32500) << "There is no KTextSharedSavingData in the context. This should not be the case";
            return;
        }
    }
    d->changeTracker->saveInlineChange(d->id, change);
    change.addChildElement("deleteChangeXml", d->deleteChangeXml);
    changeName = sharedData->genChanges().insert(change);

    context.xmlWriter().startElement("text:change", false);
    context.xmlWriter().addAttribute("text:change-id", changeName);
    context.xmlWriter().endElement();
}

void KDeleteChangeMarker::setDeleteChangeXml(QString &deleteChangeXml)
{
    d->deleteChangeXml = deleteChangeXml;
}

void KDeleteChangeMarker::setDeletedListStyle(KListStyle::ListIdType id, KListStyle *style)
{
    d->deletedListStyles.insert(id, style);
}

KListStyle *KDeleteChangeMarker::deletedListStyle(KListStyle::ListIdType id)
{
    return d->deletedListStyles.value(id);
}

