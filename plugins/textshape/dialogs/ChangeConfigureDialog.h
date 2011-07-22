/* This file is part of the KDE project
 * Copyright (C) 2009 Ganesh Paramasivam <ganesh@crystalfab.com>
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
#ifndef CHANGE_CONFIGURE_DIALOG_H
#define CHANGE_CONFIGURE_DIALOG_H

#include <QtGui/QLabel>
#include <KDE/KDialog>
#include <KChangeTracker.h>

class ColorDisplayLabel : public QLabel
{
public:
    ColorDisplayLabel(QWidget *parent = 0);
    ~ColorDisplayLabel();
    void paintEvent(QPaintEvent *event);
    QColor color() const;
    void setColor(const QColor &color);

private:
    QColor labelColor;
};

#include <ui_ChangeConfigureDialog.h>

class ChangeConfigureDialog:public KDialog
{
    Q_OBJECT

    typedef enum
    {
        eInsert,
        eDelete,
        eFormatChange,
        eChangeTypeNone
    } ChangeType;

public:
    ChangeConfigureDialog(const QColor &insertionColor, const QColor &deletionColor, const QColor &formatChangeColor, const QString &authorName, KChangeTracker::ChangeSaveFormat changeSaveFormat, QWidget *parent = 0);
    ~ChangeConfigureDialog();

    QColor insertionBgColor() const;
    QColor deletionBgColor() const;
    QColor formatChangeBgColor() const;
    QString authorName() const;
    KChangeTracker::ChangeSaveFormat saveFormat() const;

private:
    Ui::ChangeConfigureDialog ui;
    void updatePreviewText();
    void colorSelect(ChangeType type);

private slots:
    void insertionColorSelect();
    void deletionColorSelect();
    void formatChangeColorSelect();
};

#endif
