/* This file is part of the KDE project
 * Copyright (C) 2010 Thomas Zander <zander@kde.org>
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

#include "KoTextOnShapeContainer.h"
#include "KShapeContainer_p.h"
#include "SimpleShapeContainerModel_p.h"
#include "KoShapeRegistry.h"
#include "KShapeFactoryBase.h"
#include "KoTextShapeDataBase.h"

#include <KOdfXmlNS.h>
#include <KOdfLoadingContext.h>
#include <KShapeLoadingContext.h>

#include <KDebug>
#include <QTextCursor>

class KoTextOnShapeContainerPrivate : public KShapeContainerPrivate
{
public:
    KoTextOnShapeContainerPrivate(KShapeContainer *q);
    virtual ~KoTextOnShapeContainerPrivate();

    KShape *content; // the original shape
    KShape *textShape;
    KoTextOnShapeContainer::ResizeBehavior resizeBehavior;
};

class KoTextOnShapeContainerModel : public SimpleShapeContainerModel
{
public:
    KoTextOnShapeContainerModel(KoTextOnShapeContainer *qq, KoTextOnShapeContainerPrivate *containerData);
    virtual void containerChanged(KShapeContainer *container, KShape::ChangeType type);
    virtual void proposeMove(KShape *child, QPointF &move);
    virtual void childChanged(KShape *child, KShape::ChangeType type);
    bool inheritsTransform(const KShape *) const {
        return true;
    }

    KoTextOnShapeContainer *q;
    KoTextOnShapeContainerPrivate *containerData;
    bool lock;
};

// KoTextOnShapeContainerPrivate
KoTextOnShapeContainerPrivate::KoTextOnShapeContainerPrivate(KShapeContainer *q)
    : KShapeContainerPrivate(q),
    content(0),
    textShape(0),
    resizeBehavior(KoTextOnShapeContainer::IndependendSizes)
{
}

KoTextOnShapeContainerPrivate::~KoTextOnShapeContainerPrivate()
{
    // the 'content' object is not owned by us
    delete textShape;
}

/// KoTextOnShapeContainerModel
KoTextOnShapeContainerModel::KoTextOnShapeContainerModel(KoTextOnShapeContainer *qq, KoTextOnShapeContainerPrivate *data)
    : q(qq),
    containerData(data),
    lock(false)
{
}

void KoTextOnShapeContainerModel::containerChanged(KShapeContainer *container, KShape::ChangeType type)
{
#ifdef QT_NO_DEBUG
    Q_UNUSED(container);
#endif
    if (lock || type != KShape::SizeChanged) {
        return;
    }
    lock = true;
    Q_ASSERT(container == q);
    containerData->content->setSize(q->size());
    KShape *text = containerData->textShape;
    if (text) {
        text->setSize(q->size());
    }
    lock = false;
}

void KoTextOnShapeContainerModel::proposeMove(KShape *child, QPointF &move)
{
    if (child == containerData->textShape) { // not user movable
        move.setX(0);
        move.setY(0);
    }
}

void KoTextOnShapeContainerModel::childChanged(KShape *child, KShape::ChangeType type)
{
    if (lock) {
        return;
    }
    lock = true;
    // the container is leading in size, so the only reason we get here is when
    // one of the child shapes decided to resize itself. This would probably be
    // the text shape deciding it needs more space.
    KShape *text = containerData->textShape;
    if (child == text) {
        switch (type) {
        case KShape::SizeChanged:
            q->setSize(text->size()); // should have a policy to decide what to do
            break;
        default: // the others are not interesting for us
            break;
        }
    }
    lock = false;
}

/// KoTextOnShapeContainer
KoTextOnShapeContainer::KoTextOnShapeContainer(KShape *childShape, KResourceManager *documentResources)
    : KShapeContainer(*(new KoTextOnShapeContainerPrivate(this)))
{
    Q_D(KoTextOnShapeContainer);
    Q_ASSERT(childShape);
    d->content = childShape;

    setSize(childShape->size());
    setZIndex(childShape->zIndex());
    setTransformation(childShape->transformation());
    if (childShape->parent()) {
        childShape->parent()->addShape(this);
        childShape->setParent(0);
    }

    childShape->setPosition(QPointF()); // since its relative to my position, this won't move it
    childShape->setSelectable(false);

    d->model = new KoTextOnShapeContainerModel(this, d);
    addShape(childShape);

    QSet<KShape*> delegates;
    delegates << childShape;
    KShapeFactoryBase *factory = KoShapeRegistry::instance()->get("TextShapeID");
    if (factory) { // not installed, thats too bad, but allowed
        d->textShape = factory->createDefaultShape(documentResources);
        Q_ASSERT(d->textShape); // would be a bug in the text shape;
        d->textShape->setSize(size());
        d->textShape->setTransformation(childShape->transformation());
        KoTextShapeDataBase *shapeData = qobject_cast<KoTextShapeDataBase*>(d->textShape->userData());
        Q_ASSERT(shapeData); // would be a bug in kotext
        shapeData->setVerticalAlignment(Qt::AlignVCenter);
        addShape(d->textShape);
        d->textShape->setZIndex(childShape->zIndex() + 1);
        d->textShape->setSelectable(false);
        delegates << d->textShape;
    } else {
        kWarning(30006) << "Text shape factory not found";
    }
    setToolDelegates(delegates);
}

KoTextOnShapeContainer::~KoTextOnShapeContainer()
{
    Q_D(KoTextOnShapeContainer);
    // can't do this in the destructor of the Private class as by the time that destructor gets called there is no ShapeContainer anymore to check the parent
    if (d->content && d->content->parent() == this) delete d->content;
}

bool KoTextOnShapeContainer::loadOdf(const KXmlElement &element, KShapeLoadingContext &context)
{
    Q_D(KoTextOnShapeContainer);
    if (d->textShape == 0)
        return false; // probably because the factory was not found.

    KoTextShapeDataBase *shapeData = qobject_cast<KoTextShapeDataBase*>(d->textShape->userData());
    Q_ASSERT(shapeData); // would be a bug in kotext

    QString styleName = element.attributeNS(KOdfXmlNS::draw, "style-name");
    if (!styleName.isEmpty()) {
        KOdfStyleStack &styleStack = context.odfLoadingContext().styleStack();
        styleStack.save();
        context.odfLoadingContext().fillStyleStack(element, KOdfXmlNS::draw, "style-name", "graphic");
        styleStack.setTypeProperties("graphic");
        QString valign = styleStack.property(KOdfXmlNS::draw, "textarea-vertical-align");
        if (valign == "top") {
            shapeData->setVerticalAlignment(Qt::AlignTop);
        } else if (valign == "middle") {
            shapeData->setVerticalAlignment(Qt::AlignVCenter);
        } else if (valign == "bottom") {
            shapeData->setVerticalAlignment(Qt::AlignBottom);
        }
        styleStack.restore();
    }

    return shapeData->loadOdf(element, context);
}

void KoTextOnShapeContainer::saveOdf(KoShapeSavingContext &context) const
{
    Q_D(const KoTextOnShapeContainer);
    if (d->content)
        d->content->saveOdf(context);
}

void KoTextOnShapeContainer::setPlainText(const QString &text)
{
    Q_D(KoTextOnShapeContainer);
    if (d->textShape == 0) {
        kWarning(30006) << "No text shape present in KoTextOnShapeContainer";
        return;
    }
    KoTextShapeDataBase *shapeData = qobject_cast<KoTextShapeDataBase*>(d->textShape->userData());
    Q_ASSERT(shapeData); // would be a bug in kotext
    Q_ASSERT(shapeData->document());
    shapeData->document()->setPlainText(text);
}

void KoTextOnShapeContainer::setResizeBehavior(ResizeBehavior resizeBehavior)
{
    Q_D(KoTextOnShapeContainer);
    if (d->resizeBehavior == resizeBehavior) {
        return;
    }
    d->resizeBehavior = resizeBehavior;
    d->model->containerChanged(this, KShape::SizeChanged);
}

KoTextOnShapeContainer::ResizeBehavior KoTextOnShapeContainer::resizeBehavior() const
{
    Q_D(const KoTextOnShapeContainer);
    return d->resizeBehavior;
}

void KoTextOnShapeContainer::setTextAlignment(Qt::Alignment alignment)
{
    Q_D(KoTextOnShapeContainer);
    if (d->textShape == 0) {
        kWarning(30006) << "No text shape present in KoTextOnShapeContainer";
        return;
    }

    // vertical
    KoTextShapeDataBase *shapeData = qobject_cast<KoTextShapeDataBase*>(d->textShape->userData());
    shapeData->setVerticalAlignment(alignment);

    // horizontal
    Q_ASSERT(shapeData->document());
    QTextBlockFormat bf;
    bf.setAlignment(alignment & Qt::AlignHorizontal_Mask);

    QTextCursor cursor(shapeData->document());
    cursor.setPosition(QTextCursor::End, QTextCursor::KeepAnchor);
    cursor.mergeBlockFormat(bf);
}

Qt::Alignment KoTextOnShapeContainer::textAlignment() const
{
    Q_D(const KoTextOnShapeContainer);
    if (d->textShape == 0) {
        kWarning(30006) << "No text shape present in KoTextOnShapeContainer";
        return Qt::AlignTop;
    }

    // vertical
    KoTextShapeDataBase *shapeData = qobject_cast<KoTextShapeDataBase*>(d->textShape->userData());
    Qt::Alignment answer = shapeData->verticalAlignment() & Qt::AlignVertical_Mask;

    // horizontal
    Q_ASSERT(shapeData->document());
    QTextCursor cursor(shapeData->document());
    answer = answer | (cursor.blockFormat().alignment() & Qt::AlignHorizontal_Mask);

    return answer;
}

void KoTextOnShapeContainer::saveOdfChildElements(KoShapeSavingContext &context) const
{
    Q_D(const KoTextOnShapeContainer);
    if (d->textShape == 0) {
        return;
    }
    KoTextShapeDataBase *shapeData = qobject_cast<KoTextShapeDataBase*>(d->textShape->userData());
    Q_ASSERT(shapeData); // would be a bug in kotext
    if (!shapeData->document()->isEmpty()) {
        shapeData->saveOdf(context);
    }
}

// static
void KoTextOnShapeContainer::tryWrapShape(KShape *shape, const KXmlElement &element, KShapeLoadingContext &context)
{
    KXmlElement text = KoXml::namedItemNS(element, KOdfXmlNS::text, "p");
    if (!text.isNull()) {
        KShapeContainer *oldParent = shape->parent();
        KoTextOnShapeContainer *tos = new KoTextOnShapeContainer(shape,
                context.documentResourceManager());
        if (!tos->loadOdf(element, context)) {
            // failed, delete it again.
            shape->setParent(oldParent);
            delete tos;
        }
    }
}
