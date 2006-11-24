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

#ifndef KOCommand_h
#define KOCommand_h

#include <kcommand.h>
#include <koffice_export.h>

#include <KoSelection.h>

#include <QList>
#include <QPointF>

class KoShape;
class KoShapeGroup;
class KoShapeContainer;
class KoShapeControllerBase;
class QString;

/// The undo / redo command for shape moving.
class FLAKE_EXPORT KoShapeMoveCommand : public KCommand {
public:
    /**
     * Constructor.
     * @param shapes the set of objects that are moved.
     * @param previousPositions the known set of previous positions for each of the objects.
     *  this list naturally must have the same amount of items as the shapes set.
     * @param newPositions the new positions for the shapes.
     *  this list naturally must have the same amount of items as the shapes set.
     */
    KoShapeMoveCommand(const KoSelectionSet &shapes, QList<QPointF> &previousPositions, QList<QPointF> &newPositions);
    /**
     * Constructor.
     * @param shapes the set of objects that are moved.
     * @param previousPositions the known set of previous positions for each of the objects.
     *  this list naturally must have the same amount of items as the shapes set.
     * @param newPositions the new positions for the shapes.
     *  this list naturally must have the same amount of items as the shapes set.
     */
    KoShapeMoveCommand(const QList<KoShape*> &shapes, QList<QPointF> &previousPositions, QList<QPointF> &newPositions);
    /// execute the command
    void execute ();
    /// revert the actions done in execute
    void unexecute ();
    /// return the name of this command
    QString name () const;

    /// update newPositions list with new postions.
    void setNewPositions(QList<QPointF> newPositions) { m_newPositions = newPositions; }

private:
    QList<KoShape*> m_shapes;
    QList<QPointF> m_previousPositions, m_newPositions;
};

/// The undo / redo command for shape rotating.
class FLAKE_EXPORT KoShapeRotateCommand : public KCommand {
public:
    /**
     * Comand to rotate a selection of shapes.  Note that it just alters the rotated
     * property of those shapes, and nothing more.
     * @param shapes all the shapes that should be rotated
     * @param previousAngles a list with the same amount of items as shapes with the
     *        old rotation angles
     * @param newAngles a list with the same amount of items as shapes with the new angles.
     */
    KoShapeRotateCommand(const KoSelectionSet &shapes, QList<double> &previousAngles, QList<double> &newAngles);
    /**
     * Comand to rotate a selection of shapes.  Note that it just alters the rotated
     * property of those shapes, and nothing more.
     * @param shapes all the shapes that should be rotated
     * @param previousAngles a list with the same amount of items as shapes with the
     *        old rotation angles
     * @param newAngles a list with the same amount of items as shapes with the new angles.
     */
    KoShapeRotateCommand(const QList<KoShape*> &shapes, QList<double> &previousAngles, QList<double> &newAngles);
    /// execute the command
    void execute ();
    /// revert the actions done in execute
    void unexecute ();
    /// return the name of this command
    QString name () const;
private:
    QList<KoShape*> m_shapes;
    QList<double> m_previousAngles, m_newAngles;
};

/// The undo / redo command for shape shearing.
class FLAKE_EXPORT KoShapeShearCommand : public KCommand {
public:
    /**
     * Comand to rotate a selection of shapes.  Note that it just alters the rotated
     * property of those shapes, and nothing more.
     * @param shapes all the shapes that should be rotated
     * @param previousShearXs a list with the same amount of items as shapes with the
     *        old shearX values
     * @param previousShearYs a list with the same amount of items as shapes with the
     *        old shearY values
     * @param newShearXs a list with the same amount of items as shapes with the new values.
     * @param newShearYs a list with the same amount of items as shapes with the new values.
     */
    KoShapeShearCommand(const KoSelectionSet &shapes, QList<double> &previousShearXs, QList<double> &previousShearYs, QList<double> &newShearXs, QList<double> &newShearYs);
    /// execute the command
    void execute ();
    /// revert the actions done in execute
    void unexecute ();
    /// return the name of this command
    QString name () const;
private:
    QList<KoShape*> m_shapes;
    QList<double> m_previousShearXs, m_previousShearYs, m_newShearXs, m_newShearYs;
};

/// The undo / redo command for shape sizing.
class FLAKE_EXPORT KoShapeSizeCommand : public KCommand {
public:
    /**
     * The undo / redo command for shape sizing.
     * @param shapes all the shapes that will be rezised at the same time
     * @param previousSizes the old sizes; in a list with a member for each shape
     * @param newSizes the new sizes; in a list with a member for each shape
     */
    KoShapeSizeCommand(const KoSelectionSet &shapes, QList<QSizeF> &previousSizes, QList<QSizeF> &newSizes);
    /**
     * The undo / redo command for shape sizing.
     * @param shapes all the shapes that will be rezised at the same time
     * @param previousSizes the old sizes; in a list with a member for each shape
     * @param newSizes the new sizes; in a list with a member for each shape
     */
    KoShapeSizeCommand(const QList<KoShape*> &shapes, QList<QSizeF> &previousSizes, QList<QSizeF> &newSizes);
    /// execute the command
    void execute ();
    /// revert the actions done in execute
    void unexecute ();
    /// return the name of this command
    QString name () const;
private:
    QList<KoShape*> m_shapes;
    QList<QSizeF> m_previousSizes, m_newSizes;
};

/// The undo / redo command for grouping shapes
class FLAKE_EXPORT KoGroupShapesCommand : public KCommand {
public:
    /**
     * Command to group a set of shapes into a predefined container.
     * @param container the container to group the shapes under.
     * @param shapes a list of all the shapes that should be grouped.
     * @param clipped a list of the same length as the shapes list with one bool for each shape.
     *      See KoShapeContainer::childClipped()
     */
    KoGroupShapesCommand(KoShapeContainer *container, QList<KoShape *> shapes, QList<bool> clipped);
    /**
     * Command to group a set of shapes into a predefined container.
     * Convenience constructor since KoShapeGroup does not allow clipping.
     * @param container the group to group the shapes under.
     * @param shapes a list of all the shapes that should be grouped.
     */
    KoGroupShapesCommand(KoShapeGroup *container, QList<KoShape *> shapes);
    virtual ~KoGroupShapesCommand() { };
    /// execute the command
    virtual void execute ();
    /// revert the actions done in execute
    virtual void unexecute ();
    /// return the name of this command
    virtual QString name () const;

protected:
    KoGroupShapesCommand(); ///< protected constructor for child classes
    QList<KoShape*> m_shapes; ///<list of shapes to be grouped
    QList<bool> m_clipped; ///< list of booleas to specify the shape of the same index to eb clipped
    KoShapeContainer *m_container; ///< the container where the grouping should be for.
    QList<KoShapeContainer*> m_oldParents; ///< the old parents of the shapes
};

/// The undo / redo command for ungrouping shapes
class FLAKE_EXPORT KoUngroupShapesCommand : public KoGroupShapesCommand {
public:
    /**
     * Command to ungroup a set of shapes from one parent container.
     * @param container the group to ungroup the shapes from.
     * @param shapes a list of all the shapes that should be ungrouped.
     */
    KoUngroupShapesCommand(KoShapeContainer *container, QList<KoShape *> shapes);
    /// execute the command
    void execute ();
    /// revert the actions done in execute
    void unexecute ();
    /// return the name of this command
    QString name () const;
};

/// The undo / redo command for creating shapes
class FLAKE_EXPORT KoShapeCreateCommand : public KCommand {
public:
    /**
     * Command used on creation of new shapes
     * @param controller the controller used to add/remove the shape from
     * @param shape the shape thats just been created.
     */
    KoShapeCreateCommand( KoShapeControllerBase *controller, KoShape *shape );
    virtual ~KoShapeCreateCommand();
    /// execute the command
    void execute ();
    /// revert the actions done in execute
    void unexecute ();
    /// return the name of this command
    virtual QString name () const;
private:
    enum AddRemove { Add, Remove };
    void recurse(KoShape *shape, const AddRemove ar);

    KoShapeControllerBase *m_controller;
    KoShape *m_shape;
    bool m_deleteShape;
};

/// The undo / redo command for deleting shapes
class FLAKE_EXPORT KoShapeDeleteCommand : public KCommand {
public:
    /**
     * Command to delete a single shape by means of a shape controller.
     * @param controller the controller to used for deleting.
     * @param shape a single shape that should be deleted.
     */
    KoShapeDeleteCommand( KoShapeControllerBase *controller, KoShape *shape );
    /**
     * Command to delete a set of shapes by means of a shape controller.
     * @param controller the controller to used for deleting.
     * @param shapes a set of all the shapes that should be deleted.
     */
    KoShapeDeleteCommand( KoShapeControllerBase *controller, const KoSelectionSet &shapes );
    virtual ~KoShapeDeleteCommand();
    /// execute the command
    void execute ();
    /// revert the actions done in execute
    void unexecute ();
    /// return the name of this command
    virtual QString name () const;
private:
    KoShapeControllerBase *m_controller;
    QList<KoShape*> m_shapes;
    bool m_deleteShapes;
};

/// The undo / redo command for setting the shape background
class FLAKE_EXPORT KoShapeBackgroundCommand : public KCommand {
public:
    /**
     * Command to set a new shape background.
     * @param shapes a set of all the shapes that should get the new background.
     * @param brush the new background brush
     */
    KoShapeBackgroundCommand( const KoSelectionSet &shapes, const QBrush &brush );
    virtual ~KoShapeBackgroundCommand();
    /// execute the command
    void execute ();
    /// revert the actions done in execute
    void unexecute ();
    /// return the name of this command
    virtual QString name () const;
private:
    QList<KoShape*> m_shapes;    ///< the shapes to set background for
    QList<QBrush> m_oldBrushes; ///< the old background brushes, one for each shape
    QBrush m_newBrush;           ///< the new background brush to set
};

/// The undo / redo command for aligning shapes
class KoShapeAlignCommand : public KCommand {
public:
    /// The different alignment options for this command
    enum Align
    {
        HorizontalLeftAlignment,    ///< Align left
        HorizontalCenterAlignment,  ///< Align Centered horizontally
        HorizontalRightAlignment,   ///< Align Right
        VerticalBottomAlignment,    ///< Align bottom
        VerticalCenterAlignment,    ///< Align centered vertically
        VerticalTopAlignment        ///< Align top
    };
    /**
     * Command to align a set of shapes in a rect
     * @param shapes a set of all the shapes that should be aligned
     * @param align the aligment type
     * @param boundingRect the rect the shape will be aligned in
     */
    KoShapeAlignCommand( const KoSelectionSet &shapes, Align align, QRectF boundingRect );
    virtual ~KoShapeAlignCommand();
    /// execute the command
    virtual void execute();
    /// revert the actions done in execute
    virtual void unexecute();
    /// return the name of this command
    virtual QString name () const;
private:
    KoShapeMoveCommand *m_command;
};

/// The undo / redo command for distributing shapes
class KoShapeDistributeCommand : public KCommand
{
public:
    /// The different options to ditribute with this command
    enum Distribute
    {
        HorizontalCenterDistribution,   ///< Horizontal centered
        HorizontalGapsDistribution,     ///< Horizontal Gaps
        HorizontalLeftDistribution,     ///< Horizontal Left
        HorizontalRightDistribution,    ///< Horizontal Right
        VerticalCenterDistribution,     ///< Vertical centered
        VerticalGapsDistribution,       ///< Vertical Gaps
        VerticalBottomDistribution,     ///< Vertical bottom
        VerticalTopDistribution         ///< Vertical top
    };
    /**
     * Command to align a set of shapes in a rect
     * @param shapes a set of all the shapes that should be distributed
     * @param distribute the distribution type
     * @param boundingRect the rect the shapes will be distributed in
     */
    KoShapeDistributeCommand( const KoSelectionSet &shapes, Distribute distribute, QRectF boundingRect );
    virtual ~KoShapeDistributeCommand();
    /// execute the command
    virtual void execute();
    /// revert the actions done in execute
    virtual void unexecute();
    /// return the name of this command
    virtual QString name () const;
private:
    double getAvailableSpace( KoShape *first, KoShape *last, double extent, QRectF boundingRect );
    Distribute m_distribute;
    KoShapeMoveCommand *m_command;
};

/// The undo / redo command to lock a set of shapes position and size
class KoShapeLockCommand : public KCommand
{
public:
    /**
     * Command to lock a set of shapes position and size
     * @param shapes a set of shapes that should change lock state
     * @param oldLock list of old lock states the same length as @p shapes
     * @param newLock list of new lock states the same length as @p shapes
     */
    KoShapeLockCommand(const KoSelectionSet &shapes, const QList<bool> &oldLock, const QList<bool> &newLock);
    /**
     * Command to lock a set of shapes position and size
     * @param shapes a set of shapes that should change lock state
     * @param oldLock list of old lock states the same length as @p shapes
     * @param newLock list of new lock states the same length as @p shapes
     */
    KoShapeLockCommand(const QList<KoShape*> &shapes, const QList<bool> &oldLock, const QList<bool> &newLock);
    ~KoShapeLockCommand();

    /// execute the command
    virtual void execute();
    /// revert the actions done in execute
    virtual void unexecute();
    /// return the name of this command
    virtual QString name () const;

private:
    QList<KoShape*> m_shapes;    /// the shapes to set background for
    QList<bool> m_oldLock;       /// old lock states
    QList<bool> m_newLock;       /// new lock states
};

#endif
