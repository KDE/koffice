/*
 * Copyright (C) 2007, 2010 Thomas Zander <zander@kde.org>
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
#ifndef FOLDERSHAPEMODEL_H
#define FOLDERSHAPEMODEL_H

#include <KShapeContainerModel.h>

class FolderShape;

class FolderShapeModel : public KShapeContainerModel
{
public:
    FolderShapeModel(FolderShape *parent);

    /// add a content item to the folder
    virtual void add(KShape *child);
    /// remove a content item from the folder
    virtual void remove(KShape *child);
    /// ignored.
    virtual void setClipped(const KShape *child, bool clipping);
    /// always returns true clipping any child to the folder outline
    virtual bool isClipped(const KShape *child) const;
    virtual bool isChildLocked(const KShape *child) const;
    /// always returns true, but thats irrelevant since folders don't rotate.
    virtual bool inheritsTransform(const KShape *shape) const;
    /// ignored
    virtual void setInheritsTransform(const KShape *shape, bool inherit);
    /// return content item count
    virtual int count() const;
    /// returns content items added earlier
    virtual QList<KShape *> shapes() const;
    /// reimplemented from KShapeContainerModel
    virtual void containerChanged(KShapeContainer *container, KShape::ChangeType type);
    /// reimplemented from KShapeContainerModel
    virtual void childChanged(KShape *child, KShape::ChangeType type);

    /// called by the folder shape to allow us to reorganize the items in the folder
    void folderResized();

private:
    QList<KShape*> m_icons;
    FolderShape *m_parent;
};

#endif
