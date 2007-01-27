/* This file is part of the KDE project
 * Copyright (C) 2006 Isaac Clerencia <isaac@warp.es>
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

#include <KoShape.h>
#include <KoProperties.h>

#include "regularpolygon/KoRegularPolygonShapeFactory.h"
#include "regularpolygon/KoRegularPolygonShape.h"

#include <klocale.h>

KoRegularPolygonShapeFactory::KoRegularPolygonShapeFactory(QObject *parent)
: KoShapeFactory(parent, "KoRegularPolygonShape", i18n("A regular polygon shape"))
{
	setToolTip(i18n("A regular polygon shape"));

	KoShapeTemplate t;
	t.name = i18n("Nice pentagon");
	t.toolTip = i18n("A nice pentagon");
	t.icon = "redSquare";
	KoProperties *props = new KoProperties();
	props->setProperty("points", 5);
	t.properties = props;
	addTemplate(t);

	t.name = i18n("Nice hexagon");
	t.toolTip = i18n("A nice hexagon");
	t.icon = "redSquare";
	props = new KoProperties();
	props->setProperty("points", 6);
	t.properties = props;
	addTemplate(t);
}

KoShape *KoRegularPolygonShapeFactory::createDefaultShape() const
{
    KoRegularPolygonShape *s = new KoRegularPolygonShape();
    s->setPoints(5);
    s->resize(QSizeF(100,100));
    s->setBackground(QBrush(Qt::red));
    return s;
}

KoShape *KoRegularPolygonShapeFactory::createShape(const KoProperties* props) const
{
    KoRegularPolygonShape *s = new KoRegularPolygonShape();
    s->setPoints(props->intProperty("points", 5));
    s->resize(QSizeF(100,100));
    s->setBackground(QBrush(Qt::red));
    return s;
}

#include <KoRegularPolygonShapeFactory.moc>
