/* This file is part of the KDE project
   Copyright (C) 2002-2003 Norbert Andres <nandres@web.de>
             (C) 2002 Ariya Hidayat <ariya@kde.org>
             (C) 2002 Laurent Montel <montel@kde.org>

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

#ifndef __kspread_dlg_database__
#define __kspread_dlg_database__

#include <qdialog.h>
#include <qrect.h>

#include <kwizard.h>

class KSpreadView;

class QCheckBox;
class QComboBox;
class QFrame;
class QGridLayout;
class QLabel;
class QLineEdit;
class QListViewItem;
class QHBoxLayout;
class QRadioButton;
class QSqlDatabase;
class QTextEdit;
class QVBoxLayout;
class QWidget;

class KListView;
class KPushButton;

class KSpreadDatabaseDlg : public KWizard
{
  Q_OBJECT

 public:
  enum PageId { eDatabase = 0, eTables = 1, eColumns = 2, eOptions = 3, eResult = 4 };

  KSpreadDatabaseDlg( KSpreadView * parent, QRect const & rect, const char * name = 0, bool modal = FALSE, WFlags fl = 0 );
  virtual ~KSpreadDatabaseDlg();

 private slots:
  void orBox_clicked();
  void andBox_clicked();
  void startingCell_clicked();
  void startingRegion_clicked();
  void connectButton_clicked();
  void databaseNameChanged( const QString & s );
  void databaseHostChanged( const QString & s );
  void databaseDriverChanged( int );
  void popupTableViewMenu( QListViewItem *, const QPoint &, int );
  void tableViewClicked( QListViewItem * );
  void accept();

 protected:
  void next();
  void back();

  QGridLayout  * m_databaseLayout;
  QGridLayout  * m_tableLayout;
  QGridLayout  * m_columnsLayout;
  QGridLayout  * m_optionsLayout;
  QGridLayout  * m_resultLayout;

 private:
  int            m_currentPage;
  KSpreadView  * m_pView;
  QRect          m_targetRect;
  QSqlDatabase * m_dbConnection;

  QWidget      * m_database;
  QLabel       * m_databaseStatus;
  QLineEdit    * m_username;
  QLineEdit    * m_port;
  QLineEdit    * m_databaseName;
  QComboBox    * m_driver;
  QLineEdit    * m_password;
  QLineEdit    * m_host;
  QLabel       * m_Type;
  QWidget      * m_table;
  QComboBox    * m_databaseList;
  KPushButton  * m_connectButton;
  QLabel       * m_tableStatus;
  QLabel       * m_SelectTableLabel;
  KListView    * m_tableView;
  QWidget      * m_columns;
  KListView    * m_columnView;
  QLabel       * m_columnsStatus;
  QWidget      * m_options;
  QComboBox    * m_columns_1;
  QComboBox    * m_columns_2;
  QComboBox    * m_columns_3;
  QComboBox    * m_operator_1;
  QComboBox    * m_operator_2;
  QComboBox    * m_operator_3;
  QLineEdit    * m_operatorValue_1;
  QLineEdit    * m_operatorValue_2;
  QLineEdit    * m_operatorValue_3;
  QRadioButton * m_andBox;
  QRadioButton * m_orBox;
  QComboBox    * m_columnsSort_1;
  QComboBox    * m_columnsSort_2;
  QComboBox    * m_sortMode_1;
  QComboBox    * m_sortMode_2;
  QCheckBox    * m_distinct;
  QWidget      * m_result;
  QTextEdit    * m_sqlQuery;
  QRadioButton * m_startingRegion;
  QLineEdit    * m_cell;
  QLineEdit    * m_region;
  QRadioButton * m_startingCell;

  void switchPage( int id );
  bool databaseDoNext();
  bool tablesDoNext();
  bool columnsDoNext();
  bool optionsDoNext();

  QString exchangeWildcards(QString const & value);
  QString getWhereCondition( QString const &, QString const &, int );
};

#endif
