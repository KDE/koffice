/* This file is part of the KDE libraries
    Copyright (C) 2000, 2001, 2002, 2003, 2004 Laurent Montel <montel@kde.org>

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



#ifndef __KCHARTPIECONFIGPAGE_H__
#define __KCHARTPIECONFIGPAGE_H__

#include <qwidget.h>
#include <qlistview.h>

class QSpinBox;
class QLineEdit;
class QCheckBox;
class QPushButton;
class QFont;
class QRadioButton;

class KChartParams;

class KChartPieConfigPage : public QWidget
{
    Q_OBJECT

public:
    KChartPieConfigPage( KChartParams* params, QWidget* parent );
    void init();
    void apply();
    void initList();
    
public slots:
    void changeValue(int);
    void slotselected(QListViewItem *);
    
private:
    int col;
    KChartParams* _params;
    QSpinBox *dist;
    QSpinBox *column;
    QListView *list;
    QSpinBox *explose;
    QMemArray<int> value;
    int pos;
};

#endif
