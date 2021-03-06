/* This file is part of the KDE project
 * Copyright (C) 2007-2011 Thomas Zander <zander@kde.org>
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
#ifndef STYLESWIDGET_H
#define STYLESWIDGET_H

#include <QWidget>
#include <QList>
#include <QTextBlockFormat>
#include <QTextCharFormat>

#include <KUnit.h>

#include <ui_StylesWidget.h>

class KStyleManager;
class KParagraphStyle;
class KCharacterStyle;
class StylesModel;
class KCanvasBase;

class StylesWidget : public QWidget
{
    Q_OBJECT
public:
    explicit StylesWidget(QWidget *parent = 0);

    void setEmbedded(bool embed);
    void setCanvas(KCanvasBase *canvas) { m_canvasBase = canvas; }

public slots:
    void setStyleManager(KStyleManager *sm);
    void setCurrentFormat(const QTextBlockFormat &format);
    void setCurrentFormat(const QTextCharFormat &format);
    void deleteStyleClicked();

signals:
    void doneWithFocus();
    void paragraphStyleSelected(KParagraphStyle *paragraphStyle, bool canDelete);
    void characterStyleSelected(KCharacterStyle *characterStyle, bool canDelete);

private slots:
    void newStyleClicked();
    void editStyle();
    void applyStyle();
    void applyStyle(const QModelIndex &);
    /// updates button state
    void setCurrent(const QModelIndex &index);
    /// if the display needs to have expand buttons at root
    void setStylesAreNested(bool on);

signals:
    void paragraphStyleSelected(KParagraphStyle *style);
    void characterStyleSelected(KCharacterStyle *style);

private:
    Ui::StylesWidget widget;
    KStyleManager *m_styleManager;

    QTextBlockFormat m_currentBlockFormat;
    QTextCharFormat m_currentCharFormat;
    StylesModel *m_stylesModel;
    bool m_blockSignals;
    bool m_isEmbedded;
    KCanvasBase *m_canvasBase;
};

#endif
