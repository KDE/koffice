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
#ifndef KO_TOOL_MANAGER_P
#define KO_TOOL_MANAGER_P

#include <QList>
#include <QObject>
#include <QString>

#include <kshortcut.h>

class KoToolFactory;
class KoToolBox;
class KoShapeManager;
class KoCanvasBase;
class KoTool;
class KoShape;

class QAbstractButton;

/// \internal
class ToolHelper : public QObject {
    Q_OBJECT
public:
    explicit ToolHelper(KoToolFactory *tool);
    QAbstractButton *createButton();
    /// wrapper around KoToolFactory::id();
    const QString &id() const;
    /// wrapper around KoToolFactory::name();
    const QString &name() const;
    /// wrapper around KoToolFactory::toolType();
    const QString &toolType() const;
    /// wrapper around KoToolFactory::activationShapeId();
    const QString &activationShapeId() const;
    /// wrapper around KoToolFactory::priority();
    int priority() const;
    KoTool *createTool(KoCanvasBase *canvas) const;
    int uniqueId() const { return m_uniqueId; }
    /// wrapper around KoToolFactory::shortcut()
    const KShortcut& shortcut() const;

signals:
    /// emitted when one of the generated buttons was pressed.
    void toolActivated(ToolHelper *tool);

private slots:
    void buttonPressed();

private:
    KoToolFactory *m_toolFactory;
    int m_uniqueId;
};

/// \internal
/// Helper class to transform a simple signal selection changed into a signal with a parameter
class Connector : public QObject {
    Q_OBJECT
public:
    Connector(KoShapeManager *parent);

public slots:
    void selectionChanged();

signals:
    void selectionChanged(QList<KoShape*> shape);

private:
    KoShapeManager *m_shapeManager;
};

#endif
