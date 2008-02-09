/* This file is part of the KDE project
 * Copyright (C) 2007 Jan Hambrecht <jaham@gmx.net>
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

#ifndef SIMPLETEXTSHAPECONFIGWIDGET_H
#define SIMPLETEXTSHAPECONFIGWIDGET_H

#include "ui_SimpleTextShapeConfigWidget.h"

#include "SimpleTextShape.h"

#include <KoShapeConfigWidgetBase.h>

#include <QUndoCommand>

class SimpleTextShape;

class SimpleTextShapeConfigWidget : public KoShapeConfigWidgetBase
{
    Q_OBJECT
public:
    SimpleTextShapeConfigWidget();
    /// reimplemented
    virtual void open(KoShape *shape);
    /// reimplemented
    virtual void save();
    /// reimplemented
    virtual bool showOnShapeCreate() { return false; }
    /// reimplemented
    virtual QUndoCommand * createCommand();
private slots:
    void slotTextChanged();

private:
    class ChangeFont : public QUndoCommand
    {
    public:
        ChangeFont( SimpleTextShapeConfigWidget *widget, const QFont &font )
            : m_widget( widget ), m_font( font )
        {
            m_shape = widget->m_shape;
            setText( "Change font" );
        }
        virtual void undo()
        {
            if ( m_shape ) {
                m_shape->setFont( m_oldFont );
                m_widget->open( m_shape );
            }
        }
        virtual void redo()
        {
            if ( m_shape ) {
                m_oldFont = m_shape->font();
                m_shape->setFont( m_font );
                m_widget->open( m_shape );
            }
        }
    private:
        SimpleTextShapeConfigWidget *m_widget;
        SimpleTextShape *m_shape;
        QFont m_font;
        QFont m_oldFont;
    };

private:
    void blockChildSignals( bool block );
    Ui::SimpleTextShapeConfigWidget widget;
    SimpleTextShape * m_shape;
    QButtonGroup * m_anchorGroup;
};

#endif // SIMPLETEXTSHAPECONFIGWIDGET_H
