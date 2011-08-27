/*
   Copyright (c) 2006 Boudewijn Rempt (boud@valdyas.org)
   Copyright (C) 2007, 2010-2011 Thomas Zander <zander@kde.org>
   Copyright (c) 2008 Carlos Licea <carlos.licea@kdemail.net>

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
#include "KResourceManager.h"
#include "KShape.h"
#include "KLineBorder.h"

#include <QVariant>
#include <QMetaObject>
#include <KUndoStack>
#include <KDebug>

class KResourceManager::Private
{
public:
    void fetchLazy(int key, const KResourceManager *parent) {
        Q_ASSERT(lazyResources.contains(key));
        Slot slot = lazyResources.value(key);
        lazyResources.remove(key);

        KResourceManager *rm = const_cast<KResourceManager*>(parent);
        slot.object->metaObject()->invokeMethod(slot.object, slot.slot,
            Qt::DirectConnection, Q_ARG(KResourceManager *, rm));
    }


    QHash<int, QVariant> resources;
    struct Slot {
        QObject *object;
        const char *slot;
    };
    QHash<int, Slot> lazyResources;
};

KResourceManager::KResourceManager(QObject *parent)
        : QObject(parent),
        d(new Private())
{
    setGrabSensitivity(3);
}

KResourceManager::~KResourceManager()
{
    delete d;
}

void KResourceManager::setResource(int key, const QVariant &value)
{
    if (d->resources.contains(key)) {
        if (d->resources.value(key) == value)
            return;
        d->resources[key] = value;
    } else {
        d->resources.insert(key, value);
        d->lazyResources.remove(key);
    }
    emit resourceChanged(key, value);
}

QVariant KResourceManager::resource(int key) const
{
    if (d->lazyResources.contains(key))
        d->fetchLazy(key, this);
    if (!d->resources.contains(key)) {
        QVariant empty;
        return empty;
    } else
        return d->resources.value(key);
}

void KResourceManager::setResource(int key, KShape *shape)
{
    QVariant v;
    v.setValue(shape);
    setResource(key, v);
}

void KResourceManager::setResource(int key, const KUnit &unit)
{
    QVariant v;
    v.setValue(unit);
    setResource(key, v);
}

QColor KResourceManager::colorResource(int key) const
{
    if (d->lazyResources.contains(key))
        d->fetchLazy(key, this);
    if (! d->resources.contains(key))
        return QColor(0, 0, 0); // lets have a sane default
    return resource(key).value<QColor>();
}

void KResourceManager::setForegroundColor(const QColor &color)
{
    setResource(KCanvasResource::ForegroundColor, color);
}

QColor KResourceManager::foregroundColor() const
{
    return colorResource(KCanvasResource::ForegroundColor);
}

void KResourceManager::setBackgroundColor(const QColor &color)
{
    setResource(KCanvasResource::BackgroundColor, color);
}

QColor KResourceManager::backgroundColor() const
{
    return colorResource(KCanvasResource::BackgroundColor);
}

KShape *KResourceManager::koShapeResource(int key) const
{
    if (d->lazyResources.contains(key))
        d->fetchLazy(key, this);
    if (! d->resources.contains(key))
        return 0;

    return resource(key).value<KShape *>();
}

void KResourceManager::setHandleRadius(int handleRadius)
{
    // do not allow arbitrary small handles
    if (handleRadius < 3)
        handleRadius = 3;
    setResource(KCanvasResource::HandleRadius, QVariant(handleRadius));
}

int KResourceManager::handleRadius() const
{
    if (d->resources.contains(KCanvasResource::HandleRadius))
        return d->resources.value(KCanvasResource::HandleRadius).toInt();
    return 3; // default value.
}

KUnit KResourceManager::unitResource(int key) const
{
    if (d->lazyResources.contains(key))
        d->fetchLazy(key, this);
    return resource(key).value<KUnit>();
}

void KResourceManager::setGrabSensitivity(int grabSensitivity)
{
    // do not allow arbitrary small handles
    if (grabSensitivity < 1)
        grabSensitivity = 1;
    setResource(KCanvasResource::GrabSensitivity, QVariant(grabSensitivity));
}

int KResourceManager::grabSensitivity() const
{
    return resource(KCanvasResource::GrabSensitivity).toInt();
}

bool KResourceManager::boolResource(int key) const
{
    if (d->lazyResources.contains(key))
        d->fetchLazy(key, this);
    if (! d->resources.contains(key))
        return false;
    return d->resources[key].toBool();
}

int KResourceManager::intResource(int key) const
{
    if (d->lazyResources.contains(key))
        d->fetchLazy(key, this);
    if (! d->resources.contains(key))
        return 0;
    return d->resources[key].toInt();
}

QString KResourceManager::stringResource(int key) const
{
    if (d->lazyResources.contains(key))
        d->fetchLazy(key, this);
    if (! d->resources.contains(key)) {
        QString empty;
        return empty;
    }
    return qvariant_cast<QString>(resource(key));
}

QSizeF KResourceManager::sizeResource(int key) const
{
    if (d->lazyResources.contains(key))
        d->fetchLazy(key, this);
    if (! d->resources.contains(key)) {
        QSizeF empty;
        return empty;
    }
    return qvariant_cast<QSizeF>(resource(key));
}

bool KResourceManager::hasResource(int key) const
{
    return d->resources.contains(key) || d->lazyResources.contains(key);
}

void KResourceManager::clearResource(int key)
{
    if (! d->resources.contains(key))
        return;
    d->resources.remove(key);
    QVariant empty;
    emit resourceChanged(key, empty);
}

void KResourceManager::setLazyResourceSlot(int key, QObject *object, const char *slot)
{
    if (d->resources.contains(key) || d->lazyResources.contains(key))
        return;
    KResourceManager::Private::Slot target;
    target.object = object;
    target.slot = slot;
    d->lazyResources.insert(key, target);
}

KUndoStack *KResourceManager::undoStack() const
{
    if (!hasResource(KoDocumentResource::UndoStack))
        return 0;
    return static_cast<KUndoStack*>(resource(KoDocumentResource::UndoStack).value<void*>());
}

void KResourceManager::setUndoStack(KUndoStack *undoStack)
{
    QVariant variant;
    variant.setValue<void*>(undoStack);
    setResource(KoDocumentResource::UndoStack, variant);
}

KImageCollection *KResourceManager::imageCollection() const
{
    if (!hasResource(KoDocumentResource::ImageCollection))
        return 0;
    return static_cast<KImageCollection*>(resource(KoDocumentResource::ImageCollection).value<void*>());
}

void KResourceManager::setImageCollection(KImageCollection *ic)
{
    QVariant variant;
    variant.setValue<void*>(ic);
    setResource(KoDocumentResource::ImageCollection, variant);
}

KOdfDocumentBase *KResourceManager::odfDocument() const
{
    if (!hasResource(KoDocumentResource::OdfDocument))
        return 0;
    return static_cast<KOdfDocumentBase*>(resource(KoDocumentResource::OdfDocument).value<void*>());
}

void KResourceManager::setOdfDocument(KOdfDocumentBase *currentDocument)
{
    QVariant variant;
    variant.setValue<void*>(currentDocument);
    setResource(KoDocumentResource::OdfDocument, variant);
}

void KResourceManager::setTextDocumentList(const QList<QTextDocument *> &allDocuments)
{
    QList<QVariant> list;
    foreach (QTextDocument *doc, allDocuments) {
        QVariant v;
        v.setValue<void*>(doc);
        list << v;
    }
    setResource(KoDocumentResource::TextDocuments, list);
}

QList<QTextDocument *> KResourceManager::textDocumentList() const
{
    QList<QTextDocument*> answer;
    QVariant variant = resource(KoDocumentResource::TextDocuments);
    if (variant.isNull())
        return answer;
    QList<QVariant> list = qvariant_cast<QList<QVariant> >(variant);
    foreach (const QVariant &variant, list) {
        answer << static_cast<QTextDocument*>(variant.value<void*>());
    }
    return answer;
}

#include <KResourceManager.moc>
