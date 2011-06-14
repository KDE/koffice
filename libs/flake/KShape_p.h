/* This file is part of the KDE project
 * Copyright (C) 2009 Thomas Zander <zander@kde.org>
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
#ifndef KOSHAPEPRIVATE_H
#define KOSHAPEPRIVATE_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Flake API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//


#include "KShape.h"
#include "KoShapeConnectionPolicy.h"

class KShapePrivate
{
public:
    KShapePrivate(KShape *shape);
    virtual ~KShapePrivate();
    /**
     * Notify the shape that a change was done. To be used by inheriting shapes.
     * @param type the change type
     */
    void shapeChanged(KShape::ChangeType type);

    void addShapeManager(KoShapeManager *manager);
    void removeShapeManager(KoShapeManager *manager);

    /**
     * Add a connection to the list of connections of this shape.
     * This is typically called only from the constructor of the KoShapeConnection class.
     */
    void addConnection(KoShapeConnection *connection);
    /**
     * Remove a connection to the list of connections of this shape.
     * This is typically called only from the destructor of the KoShapeConnection class.
     */
    void removeConnection(KoShapeConnection *connection);

    void loadOdfGluePoints(const KXmlElement &gluePoints);

    /**
     * Fills the style stack and returns the value of the given style property (e.g fill, stroke).
     */
    static QString getStyleProperty(const char *property, KoShapeLoadingContext &context);

    /// Loads the shadow style
    KoShapeShadow *loadOdfShadow(KoShapeLoadingContext &context) const;

    /// calls update on the shape where the border is.
    void updateBorder();

    QSizeF size; // size in pt
    QString shapeId;
    QString name; ///< the shapes names

    QTransform localMatrix; ///< the shapes local transformation matrix

    QVector<QPointF> connectors; ///< glue points in percent of size [0..1]
    QVector<KoShapeConnectionPolicy> connectorPolicies;

    KoShapeContainer *parent;
    QSet<KoShapeManager *> shapeManagers;
    QSet<KShape *> toolDelegates;
    KoShapeUserData *userData;
    KShapeApplicationData *appData;
    KoShapeBackground * fill; ///< Stands for the background color / fill etc.
    KoShapeBorderBase *border; ///< points to a border, or 0 if there is no border
    KShape *q_ptr;
    QSet<KShape*> dependees; ///< set of shapes dependent on this shape
    KoShapeShadow * shadow; ///< the current shape shadow
    QMap<QString, QString> additionalAttributes;
    QMap<QString, QString> additionalStyleAttributes;
    QSet<KEventAction *> eventActions; ///< list of event actions the shape has
    KFilterEffectStack *filterEffectStack; ///< stack of filter effects applied to the shape
    qreal transparency; ///< the shapes transparency

    QList<KoShapeConnection*> connections;

    static const int MaxZIndex = 32767;
    int zIndex : 16; // keep maxZIndex in sync!
    int visible : 1;
    int printable : 1;
    int geometryProtected : 1;
    int keepAspect : 1;
    int selectable : 1;
    int detectCollision : 1;
    int protectContent : 1;
    int editBlockDepth : 4;
    int editBlockEndShouldEmit : 1;
    int dummy : 4; // filler till 32 bits, adjust whenever altering the set!

    Q_DECLARE_PUBLIC(KShape)
};

#endif
