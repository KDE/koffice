/* This file is part of the KDE project
 * Copyright (C) 2006-2007,2009,2010 Thomas Zander <zander@kde.org>
 * Copyright (C) 2007 Jan Hambrecht <jaham@gmx.net>
 * Copyright (C) 2008 Thorsten Zachmann <zachmann@kde.org>
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
#include "TextShapeFactory.h"
#include "TextShape.h"
#include "TextEditingPluginContainer.h"

#include <KProperties.h>
#include <KoShape.h>
#include <KoTextDocument.h>
#include <KoTextShapeData.h>
#include <KOdfXmlNS.h>
#include <KoStyleManager.h>
#include <KoResourceManager.h>
#include <KInlineTextObjectManager.h>
#include <changetracker/KChangeTracker.h>
#include <KImageCollection.h>
#include <KoShapeLoadingContext.h>

#include <klocale.h>
#include <KUndoStack>
#include <QTextCursor>

TextShapeFactory::TextShapeFactory(QObject *parent)
        : KoShapeFactoryBase(parent, TextShape_SHAPEID, i18n("Text"))
{
    setToolTip(i18n("A shape that shows text"));
    QList<QPair<QString, QStringList> > odfElements;
    odfElements.append(QPair<QString, QStringList>(KOdfXmlNS::draw, QStringList("text-box")));
    odfElements.append(QPair<QString, QStringList>(KOdfXmlNS::table, QStringList("table")));
    setOdfElements(odfElements);
    setLoadingPriority(1);

    KoShapeTemplate t;
    t.name = i18n("Text");
    t.icon = "x-shape-text";
    t.toolTip = i18n("Text Shape");
    KProperties *props = new KProperties();
    t.properties = props;
    props->setProperty("demo", true);
    addTemplate(t);
}

KoShape *TextShapeFactory::createDefaultShape(KoResourceManager *documentResources) const
{
    TextShape *text = new TextShape();
    if (documentResources) {
        KoTextDocument document(text->textShapeData()->document());
        document.setUndoStack(documentResources->undoStack());

        KInlineTextObjectManager *itom = documentResources->resource(KoText::InlineTextObjectManager).value<KInlineTextObjectManager*>();
        if (itom)
            document.setInlineTextObjectManager(itom);

        KoStyleManager *styleManager = documentResources->resource(KoText::StyleManager).value<KoStyleManager*>();
        if (styleManager)
            document.setStyleManager(styleManager);
        KPageProvider *pp = static_cast<KPageProvider *>(documentResources->resource(KoText::PageProvider).value<void*>());
        if (pp)
            text->setPageProvider(pp);
            KChangeTracker *changeTracker = documentResources->resource(KoText::ChangeTracker).value<KChangeTracker*>();
        if (changeTracker)
            document.setChangeTracker(changeTracker);

        text->setImageCollection(documentResources->imageCollection());
    }

    return text;
}

KoShape *TextShapeFactory::createShape(const KProperties *params, KoResourceManager *documentResources) const
{
    TextShape *shape = static_cast<TextShape*>(createDefaultShape(documentResources));
    shape->textShapeData()->document()->setUndoRedoEnabled(false);
    shape->setSize(QSizeF(300, 200));
    shape->setDemoText(params->boolProperty("demo"));
    QString text("text");
    if (params->contains(text)) {
        KoTextShapeData *shapeData = qobject_cast<KoTextShapeData*>(shape->userData());
        QTextCursor cursor(shapeData->document());
        cursor.insertText(params->stringProperty(text));
    }
    if (documentResources) {
        shape->setImageCollection(documentResources->imageCollection());
    }
    shape->textShapeData()->document()->setUndoRedoEnabled(true);
    return shape;
}

bool TextShapeFactory::supports(const KXmlElement & e, KoShapeLoadingContext &context) const
{
    Q_UNUSED(context);
    return (e.localName() == "text-box" && e.namespaceURI() == KOdfXmlNS::draw) ||
        (e.localName() == "table" && e.namespaceURI() == KOdfXmlNS::table);
}

void TextShapeFactory::newDocumentResourceManager(KoResourceManager *manager)
{
    manager->setLazyResourceSlot(KoDocumentResource::ImageCollection,
            this, "createImageCollection");
    manager->setLazyResourceSlot(KoText::InlineTextObjectManager,
            this, "createTextObjectManager");
    manager->setLazyResourceSlot(KoText::StyleManager,
            this, "createStylemanager");
    manager->setLazyResourceSlot(KoDocumentResource::UndoStack,
            this, "createUndoStack");
    manager->setLazyResourceSlot(TextEditingPluginContainer::ResourceId,
            this, "createEditingPluginContainer");
}

void TextShapeFactory::createStylemanager(KoResourceManager *manager)
{
    QVariant variant;
    variant.setValue(new KoStyleManager(manager));
    manager->setResource(KoText::StyleManager, variant);
}

void TextShapeFactory::createTextObjectManager(KoResourceManager *manager)
{
    QVariant variant;
    variant.setValue<KInlineTextObjectManager*>(new KInlineTextObjectManager(manager));
    manager->setResource(KoText::InlineTextObjectManager, variant);
}

void TextShapeFactory::createImageCollection(KoResourceManager *manager)
{
    manager->setImageCollection(new KImageCollection(manager));
}

void TextShapeFactory::createUndoStack(KoResourceManager *manager)
{
    kWarning(32500) << "No KUndoStack found in the document resource manager, creating a new one";
    manager->setUndoStack(new KUndoStack(manager));
}

void TextShapeFactory::createEditingPluginContainer(KoResourceManager *manager)
{
    TextEditingPluginContainer *container = TextEditingPluginContainer::create(manager);
    QVariant variant;
    variant.setValue<TextEditingPluginContainer*>(container);
    manager->setResource(TextEditingPluginContainer::ResourceId, variant);
}

#include <TextShapeFactory.moc>
