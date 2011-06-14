/* This file is part of the KDE project
 * Copyright (C) 2006-2011 Thomas Zander <zander@kde.org>
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
#ifndef __KOVARIABLE_H__
#define __KOVARIABLE_H__

// KOffice libs
#include "KInlineObject.h"
#include "kotext_export.h"

// Qt + kde
#include <QHash>
#include <QString>

class QTextInlineObject;
class QTextDocument;
class KShape;
class KoShapeLoadingContext;
class KProperties;
class QWidget;
class KoVariableManager;
class KoVariablePrivate;

/**
 * Base class for in-text variables.
 *
 * A variable is a field inserted into the text and the content is set to a specific value that
 * is used as text.  This class is pretty boring in that it has just a setValue() to alter the
 * text shown; we depend on plugin writers to create more exciting ways to update variables.
 *
 * The inheriting class can choose from a number of callbacks to reimplement, see the
 *  KInlineObject API. The most important one for variable authors is the positionChanged()
 * virtual method. That hook is called whenever the variable gets a new position.
 * For variables that needs to change their value based on position in the document,
 * you should implement that hook.  Useful methods.
 */
class KOTEXT_EXPORT KoVariable : public KInlineObject
{
public:
    /**
     * Constructor.
     */
    explicit KoVariable(bool propertyChangeListener = false);
    virtual ~KoVariable();

    /**
     * The new value this variable will show.
     * Will be used at the next repaint.
     * @param value the new value this variable shows.
     */
    void setValue(const QString &value);

    /// @return the current value of this variable.
    QString value() const;

    /**
     * Shortly after instantiating this variable the factory should set the
     * properties using this method.
     * Note that the loading mechanism will fill this properties object with the
     * attributes from the ODF file (if applicable), so it would be useful to synchronize
     * the property names based on that.
     */
    virtual void setProperties(const KProperties *props) {
        Q_UNUSED(props);
    }

    /**
     * If this variable has user-editable options it should provide a widget that is capable
     * of manipulating these options so the text-tool can use it to show that to the user.
     * Note that all manipulations should have a direct effect on the variable itself.
     */
    virtual QWidget *createOptionsWidget() {
        return 0;
    }

private:
    void updatePosition(QTextInlineObject object, const QTextCharFormat &format);
    void resize(QTextInlineObject object, const QTextCharFormat &format, QPaintDevice *pd);
    void paint(QPainter &painter, QPaintDevice *pd, const QRectF &rect,
            QTextInlineObject object, const QTextCharFormat &format);

private:
    Q_DECLARE_PRIVATE(KoVariable)
};

#endif
