/* This file is part of the KDE project
   Copyright (C) 2002 Norbert Andres, nandres@web.de

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

#ifndef __kspread_dlg_changes__
#define __kspread_dlg_changes__

#include <kdialogbase.h>
#include <qwidget.h>

class KSpreadChanges;
class KSpreadView;

class KComboBox;
class KLineEdit;
class KListView;
class KPushButton;

class QCheckBox;
class QDateTimeEdit;

class FilterMain : public QWidget
{
  Q_OBJECT
 public:
  FilterMain( QWidget * parent = 0, const char * name = "FilterMain", WFlags fl = 0 );
  ~FilterMain();

  QCheckBox     * m_dateBox;
  QCheckBox     * m_authorBox;
  QCheckBox     * m_rangeBox;
  QCheckBox     * m_commentBox;
  KLineEdit     * m_authorEdit;
  KLineEdit     * m_rangeEdit;
  KLineEdit     * m_commentEdit;
  KComboBox     * m_dateUsage;
  QDateTimeEdit * m_timeFirst;
  QDateTimeEdit * m_timeSecond;
};

class AcceptRejectWidget : public QWidget
{
  Q_OBJECT

 public:
  AcceptRejectWidget( QWidget * parent = 0, const char * name = 0, WFlags fl = 0 );
  ~AcceptRejectWidget();
  
  KPushButton * m_acceptButton;
  KPushButton * m_rejectButton;
  KPushButton * m_acceptAllButton;
  KPushButton * m_rejectAllButton;
  KListView   * m_listView;
  FilterMain  * m_filter;
};


class KSpreadAcceptDlg : public KDialogBase
{  
  Q_OBJECT
 public:
  KSpreadAcceptDlg( KSpreadView * parent, KSpreadChanges * changes,
                    const char * name = "KSpreadAcceptDlg" );
  ~KSpreadAcceptDlg();

 private:
  KSpreadView        * m_view;
  KSpreadChanges     * m_changes;  
  AcceptRejectWidget * m_dialog;
};


#endif


