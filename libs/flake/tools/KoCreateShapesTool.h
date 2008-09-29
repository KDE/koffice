/* This file is part of the KDE project
 *
 * Copyright (C) 2006-2007 Thomas Zander <zander@kde.org>
 * Copyright (C) 2006 Thorsten Zachmann <zachmann@kde.org>
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

#ifndef KOCREATESHAPESTOOL_H
#define KOCREATESHAPESTOOL_H

#include "KoInteractionTool.h"

#include "flake_export.h"

#include <QString>

class KoCanvasBase;
class KoProperties;

#define KoCreateShapesTool_ID "CreateShapesTool"

/**
 * A tool to create shapes with.
 */
class FLAKE_EXPORT KoCreateShapesTool : public KoInteractionTool
{
public:
    /**
     * Create a new tool; typically not called by applications, only by the KoToolManager
     * @param canvas the canvas this tool works for.
     */
    explicit KoCreateShapesTool(KoCanvasBase *canvas);
    /// destructor
    virtual ~KoCreateShapesTool();
    virtual void mouseReleaseEvent(KoPointerEvent *event);
    virtual void activate(bool temporary = false);

    void paint(QPainter &painter, const KoViewConverter &converter);

    /**
     * Each shape-type has an Id; as found in KoShapeFactory::id().id(), to choose which
     * shape this controller should actually create; set the id before the user starts to
     * create the new shape.
     * @param id the SHAPEID of the to be generated shape
     */
    void setShapeId(const QString &id);
    /**
     * return the shape Id that is to be created.
     * @return the shape Id that is to be created.
     */
    const QString &shapeId() const;

    /**
     * Set the shape properties that the create controller will use for the next shape it will
     * create.
     * @param properties the properties or 0 if the default shape should be created.
     */
    void setShapeProperties(KoProperties *properties);
    /**
     * return the properties to be used for creating the next shape
     * @return the properties to be used for creating the next shape
     */
    KoProperties const * shapeProperties();

protected:
    virtual KoInteractionStrategy *createStrategy(KoPointerEvent *event);

private:
    friend class KoCreateShapeStrategy;

    class Private;
    Private * const d;
};

#endif
