/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>
   Copyright (C) 1999 Montel Laurent <montell@club-internet.fr>
   Copyright (C) 2002 Norbert Andres <nandres@web.de>

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

#ifndef __kspread_dlg_sort__
#define __kspread_dlg_sort__

#include <kdialogbase.h>
#include <qrect.h>

// #include <qvariant.h>
// #include <qdialog.h>

class KSpreadView;

class QCheckBox;
class QComboBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QRadioButton;
class QTabWidget;
class QWidget;

class KSpreadSortDlg : public QDialog
{ 
  Q_OBJECT

 public:
  KSpreadSortDlg( KSpreadView * parent, const char * name = 0, 
                  bool modal = false, WFlags fl = 0 );
  ~KSpreadSortDlg();

 private slots:
  void sortKey2textChanged( int );
  void useCustomListsStateChanged( int );
  void slotOk();
  void slotOrientationChanged(int id);

 private:
  void init();

  KSpreadView  * m_pView;

  QStringList    m_listColumn;
  QStringList    m_listRow;

  QWidget      * m_page1;
  QWidget      * m_page2;

  QTabWidget   * m_tabWidget;

  QComboBox    * m_sortKey1;
  QComboBox    * m_sortOrder1;
  QComboBox    * m_sortKey2;
  QComboBox    * m_sortOrder2;
  QComboBox    * m_sortKey3;
  QComboBox    * m_sortOrder3;

  QCheckBox    * m_useCustomLists;
  QComboBox    * m_customList;

  QRadioButton * m_sortColumn;
  QRadioButton * m_sortRow;

  QCheckBox    * m_copyLayout;

  QComboBox    * m_outputTable;
  QLineEdit    * m_outputCell;

  QPushButton  * m_buttonOk;
  QPushButton  * m_buttonCancel;
};

#endif 
