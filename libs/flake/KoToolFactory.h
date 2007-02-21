/* This file is part of the KDE project
 * Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
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

#ifndef KO_TOOL_FACTORY_H
#define KO_TOOL_FACTORY_H

#include "KoTool.h"
#include <KoID.h>
#include <flake_export.h>

#include <klocale.h>
#include <kshortcut.h>
#include <QObject>

/**
 * A factory for KoTool objects.
 * The baseclass for all tool plugins. Each plugin that ships a KoTool should also
 * ship a factory. That factory will extend this class and set variable data like
 * a toolTip and icon in the constructor of that extending class.
 *
 * An example usage would be:<pre>
class MyToolFactory : public KoToolFactory {
public:
    MyToolFactory(QObject *parent, const QStringList&)
        : KoToolFactory(parent, "MyTool", i18n("My Tool")) {
        setToolTip(i18n("Create object"));
        setToolType("dynamic");
        setPriority(5);
    }
    ~MyToolFactory() {}
    KoTool* createTool(KoCanvasBase *canvas);
};
K_EXPORT_COMPONENT_FACTORY(myLibrary,
     KGenericFactory<MyToolFactory>( "MyTool" ) )
</pre>

 */
class FLAKE_EXPORT KoToolFactory : public QObject {
    Q_OBJECT

public:
    /**
     * Create the new factory
     * @param parent the parent QObject for memory management usage.
     * @param id a string that will be used internally for referencing the tool, for
     *   example for use by the KoTool::sigActivateTemporary.
     * @param name the user visible name of the tool this factory creates.
     */
    KoToolFactory(QObject *parent, const QString &id, const QString &name);
    virtual ~KoToolFactory();

    /**
     *  instanciate a new tool
     * @param canvas the canvas that the new tool will work on. Should be passed
     *    to the constructor of the tool.
     * @return a new KoTool instance
     */
    virtual KoTool * createTool(KoCanvasBase *canvas) = 0;
    /**
     * return the id for the tool this factory creates.
     * @return the id for the tool this factory creates.
     */
    const QString &toolId() const;
    /**
     * return the user visible (and translated) name to be seen by the user.
     * @return the user visible (and translated) name to be seen by the user.
     */
    const QString &name() const;
    /**
     * Create a KoID for the tool this factory creates.
     */
    const KoID id() const;
    /**
     * Returns The priority of this tool in its section in the toolbox
     * @return The priority of this tool.
     */
    int priority() const;
    /**
     * returns the type of tool, used to group tools in the toolbox
     * @return the type of tool
     */
    const QString &toolType() const;
    /**
     * return a translated tooltip Text
     * @return a translated tooltip Text
     */
    const QString &toolTip() const;
    /**
     * return the basename of the icon for this tool
     * @return the basename of the icon for this tool
     */
    const QString& icon() const;
    /**
     * Return the id of the shape we can process.
     * This is the shape ID the tool we create is associated with.  So a TextTool for a TextShape.
     * @return the id of a shape, or an empty string for all shapes.
     */
    const QString &activationShapeId() const;

    /**
     * Return the default keyboard shortcut for activation of this tool (if
     * the shape this tool belongs to is active).
     *
     * @return the shortcut
     */
    const KShortcut& shortcut() const;

    bool inputDeviceAgnostic() const;

    /**
     * Returns the main toolType
     * Each tool has a toolType which it uses to be grouped in the toolbox.
     * The predefined areas are main and dynamic. "main" tools are always
     * shown.
     *
     * @see toolType()
     * @see setToolType()
     */
    static QString mainToolType() { return "main"; }
    /**
     * Returns the dynamic toolType
     * Each tool has a toolType which it uses to be grouped in the toolbox.
     * The predefined areas are main and dynamic. Dynamic tools are hidden
     * until the shape they belong to is activated. XXX: hiding and showing
     * buttons is a no-no from a usability pov, so we should discuss this
     * with KDE's usability group.
     *
     * @see toolType()
     * @see setToolType()
     */
    static QString dynamicToolType() { return "dynamic"; }

protected:
    /**
     * Set the tooltip to be used for this tool
     * @param tooltip the tooltip
     */
    void setToolTip(const QString & tooltip);
    /**
     * Set the toolType. used to group tools in the toolbox
     * @param toolType the toolType
     */
    void setToolType(const QString & toolType);
    /**
     * Set an icon to be used in the toolBox.
     * @param iconName the basename (without extension) of the icon
     * @see KIconLoader
     */
    void setIcon(const QString & iconName);
    /**
     * Set the priority of this tool, as it is shown in the toolBox; lower number means
     * it will be show more to the front of the list.
     * @param newPriority the priority
     */
    void setPriority(int newPriority);
    /**
     * Set the id of the shape we can process.
     * This is the ID, as passed to the constructor of a KoShapeFactory, that the tool
     * we create is associated with. This means that if a KoTextShape is selected, then
     * all tools that have its id set here will be added to the dynamic part of the toolbox.
     * @param activationShapeId the ID of the shape
     */
    void setActivationShapeID(const QString &activationShapeId);

    /**
     * Set the default shortcut for activation of this tool.
     */
    void setShortcut(const KShortcut & shortcut);

private:
    QString m_toolType;
    QString m_tooltip;
    QString m_activationId;
    QString m_icon;
    const QString m_name, m_id;
    int m_priority;
    KShortcut m_shortcut;
};

#endif
