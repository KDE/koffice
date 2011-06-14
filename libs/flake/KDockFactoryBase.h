/* This file is part of the KDE project
   Copyright (C) 2006 Peter Simonsson <peter.simonsson@gmail.com>
   Copyright (C) 2011 Thomas Zander <zander@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KODOCKFACTORY_H
#define KODOCKFACTORY_H

#include <QtCore/QString>
#include <QtCore/QObject>

#include "flake_export.h"

class QDockWidget;
class KoDockFactoryBasePrivate;

/**
 * Base class for factories used to create new dock widgets.
 * @see KoDockRegistry
 * @see KCanvasObserverBase
 */
class FLAKE_EXPORT KDockFactoryBase : public QObject
{
    Q_OBJECT
public:
    enum DockPosition {
        DockTornOff, ///< Floating as its own top level window
        DockTop,    ///< Above the central widget
        DockBottom, ///< Below the central widget
        DockRight,  ///< Right of the centra widget
        DockLeft,   ///< Left of the centra widget
        DockMinimized  ///< Not docked, but reachable via the menu
    };

    /**
     * Constuctor for the dockerFactory.
     * @param parent a parent qobject for memory management purposes.
     * @param dockerId a string that identifies the type of docker for purposes like storing
     *  and restoring the state and position between restarts.
     */
    KDockFactoryBase(QObject *parent, const QString &dockerId);
    virtual ~KDockFactoryBase();

    /// @return the id of the dock widget used in docker management.
    QString id() const;

    /// @return the dock widget area the widget should appear in by default
    DockPosition defaultDockPosition() const;

    /// Returns true if the dock widget should get a collapsable header.
    bool isCollapsable() const;

    /**
     * In case the docker is collapsable, returns true if the dock widget
     * will start collapsed by default.
     */
    bool isDefaultCollapsed() const;

    /// Creates the dock widget
    /// @return the created dock widget
    virtual QDockWidget *createDockWidget() = 0;

protected:
    // add setters.
    void setDefaultDockPosition(DockPosition pos);
    void setIsCollapsable(bool collapsable);
    void setDefaultCollapsed(bool collapsed);

    KoDockFactoryBasePrivate *d_ptr;

private:
    Q_DECLARE_PRIVATE(KDockFactoryBase)
};

#endif
