// -*- Mode: c++; c-basic-offset: 4; indent-tabs-mode: nil; tab-width: 4; -*-
/* This file is part of the KDE project
   Copyright (C) 2004 Thorsten Zachmann <zachmann@kde.orgReginald Stadlbauer <reggie@kde.org>

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
 * Boston, MA 02110-1301, USA.
*/

#ifndef PENSTYLEWIDGET_H
#define PENSTYLEWIDGET_H

#include "global.h"

#include <qwidget.h>

#include "KPrCommand.h"
#include "KPrPen.h"

class PenStyleUI;


class PenStyleWidget : public QWidget
{
    Q_OBJECT
public:
    PenStyleWidget( QWidget *parent, const char *name, const PenCmd::Pen &pen, bool configureLineEnds = true );
    ~PenStyleWidget();

    int getPenConfigChange() const;
    PenCmd::Pen getPen() const;

    void setPen( const PenCmd::Pen &pen );
    void apply();

private:
    KPPen getKPPen() const;
    LineEnd getLineBegin() const;
    LineEnd getLineEnd() const;

    void setPen( const KPPen &pen );
    void setLineBegin( LineEnd lb );
    void setLineEnd( LineEnd le );

    PenCmd::Pen m_pen;

    PenStyleUI *m_ui;

private slots:
    void slotReset();
    void slotPenChanged();
    void slotLineBeginChanged();
    void slotLineEndChanged();
};

#endif /* PENSTYLEWIDGET_H */
