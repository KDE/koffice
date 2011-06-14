/* This file is part of the KDE project
 * Copyright (C) 2006-2010 Thomas Zander <zander@kde.org>
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

#ifndef TEXTSHAPEFACTORY_H
#define TEXTSHAPEFACTORY_H

#include <KShapeFactoryBase.h>

class KShape;
class KInlineTextObjectManager;

class TextShapeFactory : public KShapeFactoryBase
{
    Q_OBJECT

public:
    /// constructor
    explicit TextShapeFactory(QObject *parent);
    ~TextShapeFactory() {}

    virtual KShape *createShape(const KProperties *params, KResourceManager *documentResources = 0) const;
    virtual KShape *createDefaultShape(KResourceManager *documentResources = 0) const;
    virtual bool supports(const KXmlElement & e, KoShapeLoadingContext &context) const;

    virtual void newDocumentResourceManager(KResourceManager *manager);

public slots:
    void createStylemanager(KResourceManager *manager);
    void createTextObjectManager(KResourceManager *manager);
    void createImageCollection(KResourceManager *manager);
    void createUndoStack(KResourceManager *manager);
    void createEditingPluginContainer(KResourceManager *manager);
};

#endif
