/* This file is part of the KDE project
   Copyright (C) 2001,2002,2003,2004 Laurent Montel <montel@kde.org>

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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef __KCHARTFONTCONFIGPAGE_H__
#define __KCHARTFONTCONFIGPAGE_H__

#include <qwidget.h>
#include <qbutton.h>

#include "kchartcolorarray.h"
#include "kchartDataEditor.h"
class QLineEdit;
class QListBox;
class QPushButton;

class KChartParams;

class KChartFontConfigPage : public QWidget
{
    Q_OBJECT

public:
    KChartFontConfigPage( KChartParams* params,QWidget* parent, KoChart::Data *dat);
    void init();
    void apply();
    void initList();

public slots:
    void changeLabelFont();
private:
    KChartParams* _params;
    QLineEdit *font;
    QListBox *list;
    QPushButton *fontButton;
    QFont xTitle;
    QFont yTitle;
    QFont label;
    QFont yAxis;
    QFont xAxis;
    QFont legend;
    QButton::ToggleState xTitleIsRelative;
    QButton::ToggleState yTitleIsRelative;
    QButton::ToggleState labelIsRelative;
    QButton::ToggleState yAxisIsRelative;
    QButton::ToggleState xAxisIsRelative;
    QButton::ToggleState legendIsRelative;
    KoChart::Data *data;
};
#endif
