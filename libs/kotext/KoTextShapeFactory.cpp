/* This file is part of the KDE project
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
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
#include <KoTextShapeFactory.h>

#include <klocale.h>
#include <kgenericfactory.h>

#include "KoProperties.h"
#include "KoShapeRegistry.h"
#include "KoToolRegistry.h"
#include <KoShapeGeometry.h>
#include <KoShape.h>

#include "KoTextShapeData.h"
#include "KoTextShape.h"
#include "KoTextToolFactory.h"

typedef KGenericFactory<KoTextPlugin> KoTextPluginFactory;
K_EXPORT_COMPONENT_FACTORY(kotext2, KoTextPluginFactory( "TextShape" ) )

KoTextPlugin::KoTextPlugin(QObject * parent, const QStringList & l)
    : QObject(parent)
{
    KoToolRegistry::instance()->add(new KoTextToolFactory(parent, l));
    KoShapeRegistry::instance()->add(new KoTextShapeFactory(parent));
}

KoTextShapeFactory::KoTextShapeFactory(QObject *parent)
    : KoShapeFactory(parent, KoTextShape_SHAPEID, i18n("Text"))
{
    setToolTip(i18n("A Shape That Shows Text"));

    KoShapeTemplate t;
    t.name = "Simple text";
    t.toolTip = "Text Shape With Some Text";
    KoProperties *props = new KoProperties();
    t.properties = props;
    props->setProperty("text", "<b>Koffie</b>, koffie... Querelanten\ndrinken geen KOffice maar groene thee.");
    addTemplate(t);

}

KoShape *KoTextShapeFactory::createDefaultShape() {
    KoTextShape *text = new KoTextShape();
    text->setShapeId(shapeId());
    return text;
}

KoShape *KoTextShapeFactory::createShape(const KoProperties * params) const {
    KoTextShape *shape = new KoTextShape();
    shape->setShapeId(shapeId());
    QTextDocument *doc = new QTextDocument();
    doc->setDefaultFont(QFont("Sans", 10, QFont::Normal, false));
    doc->setHtml( params->getProperty("text").toString() );
    KoTextShapeData *data = static_cast<KoTextShapeData*> (shape->userData());
    data->setDocument(doc);
    return shape;
}

QList<KoShapeConfigWidgetBase*> KoTextShapeFactory::createShapeOptionPanels() {
    QList<KoShapeConfigWidgetBase*> answer;
    answer.append(new KoShapeGeometry());
    return answer;
}

#include "KoTextShapeFactory.moc"
