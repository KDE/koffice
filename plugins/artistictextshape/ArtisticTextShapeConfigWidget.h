/* This file is part of the KDE project
 * Copyright (C) 2007 Jan Hambrecht <jaham@gmx.net>
 * Copyright (C) 2008 Rob Buis <buis@kde.org>
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

#ifndef ARTISTICTEXTSHAPECONFIGWIDGET_H
#define ARTISTICTEXTSHAPECONFIGWIDGET_H

#include "ui_ArtisticTextShapeConfigWidget.h"

#include "ArtisticTextShape.h"

#include <KoShapeConfigWidgetBase.h>

#include <QUndoCommand>
#include <KLocale>

class ArtisticTextShape;

class ArtisticTextShapeConfigWidget : public KoShapeConfigWidgetBase
{
    Q_OBJECT
public:
    ArtisticTextShapeConfigWidget();
    /// reimplemented
    virtual void open(KoShape *shape);
    /// reimplemented
    virtual void save();
    /// reimplemented
    virtual bool showOnShapeCreate() { return false; }
    /// reimplemented
    virtual QUndoCommand * createCommand();

private:
    class ChangeFont : public QUndoCommand
    {
    public:
        ChangeFont( ArtisticTextShapeConfigWidget *widget, const QFont &font )
            : m_widget( widget ), m_font( font )
        {
            m_shape = widget->m_shape;
            setText( i18n("Change font") );
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
        ArtisticTextShapeConfigWidget *m_widget;
        ArtisticTextShape *m_shape;
        QFont m_font;
        QFont m_oldFont;
    };
    class ChangeText : public QUndoCommand
    {
    public:
        ChangeText( ArtisticTextShapeConfigWidget *widget, const QString &text )
            : m_widget( widget ), m_text( text )
        {
            m_shape = widget->m_shape;
            setText( i18n("Change text") );
        }
        virtual void undo()
        {
            if ( m_shape ) {
                m_shape->setText( m_oldText );
                m_widget->open( m_shape );
            }
        }
        virtual void redo()
        {
            if ( m_shape ) {
                m_oldText = m_shape->text();
                m_shape->setText( m_text );
                m_widget->open( m_shape );
            }
        }
    private:
        ArtisticTextShapeConfigWidget *m_widget;
        ArtisticTextShape *m_shape;
        QString m_text;
        QString m_oldText;
    };
    class ChangeAnchor : public QUndoCommand
    {
    public:
        ChangeAnchor( ArtisticTextShapeConfigWidget *widget, ArtisticTextShape::TextAnchor anchor )
            : m_widget( widget ), m_anchor( anchor )
        {
            m_shape = widget->m_shape;
            setText( i18n("Change text anchor") );
        }
        virtual void undo()
        {
            if ( m_shape ) {
                m_shape->setTextAnchor( m_oldAnchor );
                m_widget->open( m_shape );
            }
        }
        virtual void redo()
        {
            if ( m_shape ) {
                m_oldAnchor = m_shape->textAnchor();
                m_shape->setTextAnchor( m_anchor );
                m_widget->open( m_shape );
            }
        }
    private:
        ArtisticTextShapeConfigWidget *m_widget;
        ArtisticTextShape *m_shape;
        ArtisticTextShape::TextAnchor m_anchor;
        ArtisticTextShape::TextAnchor m_oldAnchor;
    };

private:
    void blockChildSignals( bool block );
    Ui::ArtisticTextShapeConfigWidget widget;
    ArtisticTextShape * m_shape;
    QButtonGroup * m_anchorGroup;
};

#endif // ARTISTICTEXTSHAPECONFIGWIDGET_H
