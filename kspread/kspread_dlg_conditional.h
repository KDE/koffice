/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>
   Copyright (C) 1999, 2000 Montel Laurent <montell@club-internet.fr>

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

#ifndef __kspread_dlg_conditional__
#define __kspread_dlg_conditional__

#include <qdialog.h>
#include <qcombobox.h>
#include <qlineedit.h>
#include <kcolorbtn.h>
#include <qpushbutton.h>
#include <qfont.h>
#include <qlabel.h>
#include "kspread_cell.h"
class KSpreadView;
class KSpreadTable;


class KSpreadWidgetconditional : public QWidget
{
  Q_OBJECT
public:
  KSpreadWidgetconditional(QWidget *_parent,const char* name);
  double getBackFirstValue();
  double getBackSecondValue();
  QFont getFont(){return font;}
  QColor getColor();
  Conditional typeOfCondition();
  void init(KSpreadConditional *tmp);
public slots:
  void changeLabelFont();
  void changeIndex(const QString &text);
  void refreshPreview();
  void disabled();
signals:
  void fontSelected();
protected:
  QComboBox *choose;
  QLineEdit *edit1;
  QLineEdit *edit2;
  KColorButton* color;
  QPushButton *fontButton;
  QFont font;
  KSpreadConditional tmpCond;
  QLineEdit *preview;
};

class KSpreadconditional : public QDialog
{
  Q_OBJECT
public:
KSpreadconditional(KSpreadView* parent, const char* name,const QRect &_marker );
void init();
public slots:
  void slotOk();
  void slotClose();

protected:
  KSpreadView* m_pView;
  QPushButton* m_pOk;
  QPushButton* m_pClose;
  QRect  marker;
  KSpreadWidgetconditional *firstCond;
  KSpreadWidgetconditional *secondCond;
  KSpreadWidgetconditional *thirdCond;
  KSpreadConditional result;
};



#endif
