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

#include "KoDeleteChangeMarker.h"

//KOffice includes
#include <KoTextDocument.h>
#include <KXmlReader.h>
#include <KXmlWriter.h>
#include <KoTextShapeSavingContext.h>
#include <KoShapeLoadingContext.h>
#include <opendocument/KoTextSharedSavingData.h>
#include "KoChangeTrackerElement.h"
#include "KoChangeTracker.h"

//KDE includes
#include <kdebug.h>

//Qt includes
#include <QFontMetrics>
#include <QTextDocument>
#include <QTextInlineObject>
#include <QPainter>

/*********************************** ODF Bug Work-around code **********************************************/
const QString KoDeleteChangeMarker::RDFListName("http://www.koffice.org/list#");
const QString KoDeleteChangeMarker::RDFListItemName("http://www.koffice.org/list-item#");
const QString KoDeleteChangeMarker::RDFListValidity("http://www.kofficde.org/list-status#valid");
const QString KoDeleteChangeMarker::RDFListItemValidity("http://www.koffice.org/list-item-status#valid");
const QString KoDeleteChangeMarker::RDFListLevel("http://www.koffice.org/list-status#level");
const QString KoDeleteChangeMarker::RDFDeleteChangeContext("http://www.koffice.org/deleteChangeMetadata");
/***********************************************************************************************************/

class KoDeleteChangeMarker::Private
{
public:
    Private() {}

    KoChangeTracker *changeTracker;
    QString text;
    int id;
    QString deleteChangeXml;
    QHash<KListStyle::ListIdType, KListStyle *> deletedListStyles;
};

KoDeleteChangeMarker::KoDeleteChangeMarker(KoChangeTracker* changeTracker)
        : d(new Private())
{
    d->changeTracker = changeTracker;
}

KoDeleteChangeMarker::~KoDeleteChangeMarker()
{
    delete d;
}
/*
void KoDeleteChangeMarker::setText (const QString& text)
{
    d->text = text;
}

QString KoDeleteChangeMarker::text() const
{
    return d->text;
}
*/
void KoDeleteChangeMarker::setChangeId (int id)
{
    d->id = id;
}

int KoDeleteChangeMarker::changeId() const
{
    return d->id;
}

int KoDeleteChangeMarker::position() const
{
    return textPosition();
}

bool KoDeleteChangeMarker::loadOdf(const KXmlElement &element, KoShapeLoadingContext &context)
{
    Q_UNUSED(element)
    Q_UNUSED(context);
    return false;
}

void KoDeleteChangeMarker::paint(QPainter& painter, QPaintDevice *pd, const QRectF &rect, QTextInlineObject object, const QTextCharFormat &format)
{
    Q_UNUSED(painter);
    Q_UNUSED(pd);
    Q_UNUSED(rect);
    Q_UNUSED(object);
    Q_UNUSED(format);
}

void KoDeleteChangeMarker::resize(QTextInlineObject object, const QTextCharFormat &format, QPaintDevice *pd)
{
    Q_UNUSED(object);
    Q_UNUSED(format);
    Q_UNUSED(pd);

    object.setWidth(0);
}

void KoDeleteChangeMarker::updatePosition(QTextInlineObject object, const QTextCharFormat &format)
{
    Q_UNUSED(object);
    Q_UNUSED(format);
}

void KoDeleteChangeMarker::saveOdf(KoShapeSavingContext &context)
{
    KOdfGenericChange change;
    if (d->changeTracker->saveFormat() == KoChangeTracker::ODF_1_2) {
        change.setChangeFormat(KOdfGenericChange::ODF_1_2);
    } else {
        change.setChangeFormat(KOdfGenericChange::DELTAXML);
    }

    QString changeName;
    KoTextSharedSavingData *sharedData = 0;
    if (context.sharedData(KOTEXT_SHARED_SAVING_ID)) {
        sharedData = dynamic_cast<KoTextSharedSavingData*>(context.sharedData(KOTEXT_SHARED_SAVING_ID));
        if (!sharedData) {
            kWarning(32500) << "There is no KoTextSharedSavingData in the context. This should not be the case";
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

void KoDeleteChangeMarker::setDeleteChangeXml(QString &deleteChangeXml)
{
    d->deleteChangeXml = deleteChangeXml;
}

void KoDeleteChangeMarker::setDeletedListStyle(KListStyle::ListIdType id, KListStyle *style)
{
    d->deletedListStyles.insert(id, style);
}

KListStyle *KoDeleteChangeMarker::deletedListStyle(KListStyle::ListIdType id)
{
    return d->deletedListStyles.value(id);
}

