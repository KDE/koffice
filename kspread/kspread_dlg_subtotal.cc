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

#include "kspread_dlg_subtotal.h"
#include "kspreadsubtotal.h"
#include "kspread_sheet.h"
#include "kspread_view.h"
#include "kspread_util.h"

#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>

#include <qcheckbox.h>
#include <qcombobox.h>
#include <qlistview.h>
#include <qmemarray.h>


KSpreadSubtotalDlg::KSpreadSubtotalDlg( KSpreadView * parent, QRect const & selection, const char * name )
  : KDialogBase(parent, name, true, "Subtotals", Ok | Cancel | User1, Ok, true, KGuiItem("Remove All") ),
    m_pView( parent ),
    m_pTable( m_pView->activeTable() ),
    m_selection( selection ),
    m_dialog( new KSpreadSubtotal( this ) )
{
  setButtonBoxOrientation( Vertical );
  setMainWidget( m_dialog );

  fillColumnBoxes();
  fillFunctionBox();
}

KSpreadSubtotalDlg::~KSpreadSubtotalDlg()
{
}

void KSpreadSubtotalDlg::slotOk()
{
  int numOfCols = m_selection.width();
  QMemArray<int> columns( numOfCols );

  int n = 0;
  bool empty = true;
  int left = m_selection.left();
  for ( QListViewItem * item = m_dialog->m_columnList->firstChild(); item; item = item->nextSibling() )
  {
    if ( ((QCheckListItem * ) item)->isOn() )
    {
      columns[n] = left + n;
      empty = false;
    }
    else
      columns[n] = -1;
    ++n;
  }

  if ( empty )
  {
    KMessageBox::sorry( this, i18n("You need to select at least one column for adding subtotals.") );
    return;
  }

  if ( m_dialog->m_replaceSubtotals->isChecked() )
    removeSubtotalLines();

  int mainCol = left + m_dialog->m_columnBox->currentItem();
  int bottom = m_selection.bottom();
  int top    = m_selection.top();
  int right  = m_selection.right();
  left       = m_selection.left();
  QString oldText = m_pTable->cellAt( mainCol, top )->strOutText();
  QString newText;
  QString result( " " + i18n("Result") );
  int lastChangedRow = top;

  bool ignoreEmptyCells = m_dialog->m_IgnoreBox->isChecked();
  bool addRow;
  if ( !m_dialog->m_summaryOnly->isChecked() )
  {
    int y = top + 1;
    kdDebug() << "Starting in row " << y << endl;
    while ( y <= bottom )
    {
      addRow = true;
      newText = m_pTable->cellAt( mainCol, y )->strOutText();

      if ( ignoreEmptyCells && (newText.length() == 0) )
      {
        ++y;
        kdDebug() << "Still the same -> " << y << endl;
        continue;
      }

      if (newText != oldText)
      {
        int saveY = y;
        for (int x = 0; x < numOfCols; ++x)
        {
          kdDebug() << "Column: " << x << ", " << columns[x] << endl;
          if (columns[x] != -1)
          {
            if (!addSubtotal( mainCol, columns[x], y - 1, lastChangedRow, addRow, oldText + result))
              reject();

            if ( addRow )
            {
              ++saveY;
              ++bottom;
            }

            addRow = false;
          }
        }
        y = saveY;
        lastChangedRow = y;
      }
      oldText = newText;
      ++y;
    }

    addRow = true;
    for ( int x = 0; x < numOfCols; ++x )
    {
      if ( columns[x] != -1 )
      {
        if ( !addSubtotal( mainCol, columns[x], y - 1, lastChangedRow, addRow, oldText + result ) )
          reject();
        addRow = false;
      }
    }
    ++y;
  }

  if ( m_dialog->m_summaryBelow->isChecked() )
  {
    addRow = true;
    int bottom = m_selection.bottom();
    for (int x = 0; x < numOfCols; ++x)
    {
      if (columns[x] != -1)
      {
        addSubtotal( mainCol, columns[x], bottom, top, addRow, i18n("Grand Total") );
        addRow = false;
      }
    }
  }

  accept();
}

void KSpreadSubtotalDlg::slotCancel()
{
  reject();
}

void KSpreadSubtotalDlg::slotUser1()
{
  removeSubtotalLines();
  accept();
}

void KSpreadSubtotalDlg::removeSubtotalLines()
{
  kdDebug() << "Removing subtotal lines" << endl;

  int r = m_selection.right();
  int l = m_selection.left();
  int t = m_selection.top();

  KSpreadCell * cell;
  QString text;

  for ( int y = m_selection.bottom(); y >= t; --y )
  {
    kdDebug() << "Checking row: " << y << endl;
    bool containsSubtotal = false;
    for (int x = l; x <= r; ++x )
    {
      cell = m_pTable->cellAt( x, y );
      if ( cell->isDefault() || !cell->isFormula() )
        continue;

      text = cell->text();
      if ( text.find( "SUBTOTAL" ) != -1 )
      {
        containsSubtotal = true;
        break;
      }
    }

    if ( containsSubtotal )
    {
      kdDebug() << "Line " << y << " contains a subtotal " << endl;
      QRect rect( l, y, m_selection.width(), 1 );
      m_pTable->unshiftColumn( rect );

      m_selection.setHeight( m_selection.height() - 1 );
    }
  }
  kdDebug() << "Done removing subtotals" << endl;
}

void KSpreadSubtotalDlg::fillColumnBoxes()
{
  int r = m_selection.right();
  int row = m_selection.top();

  KSpreadCell    * cell;
  QCheckListItem * item;

  QString text;
  QString col( i18n( "Column '%1' ") );

  for ( int i = m_selection.left(); i <= r; ++i )
  {
    cell = m_pTable->cellAt( i, row );
    text = cell->strOutText();

    if ( text.length() > 0 )
    {
      text = col.arg( util_encodeColumnLabelText( i ) );
    }

    m_dialog->m_columnBox->insertItem( text );

    item = new QCheckListItem( m_dialog->m_columnList,
                               text,
                               QCheckListItem::CheckBox );
    item->setOn(false);
    m_dialog->m_columnList->insertItem( item );
  }
}

void KSpreadSubtotalDlg::fillFunctionBox()
{
    QStringList lst;
    lst <<i18n( "Average" );
    lst <<i18n( "Count" );
    lst <<i18n( "CountA" );
    lst <<i18n( "Max" );
    lst << i18n( "Min" );
    lst << i18n( "Product" );
    lst << i18n( "StDev" );
    lst << i18n( "StDevP" );
    lst <<i18n( "Sum" );
    lst << i18n( "Var" );
    lst << i18n( "VarP" );
    m_dialog->m_functionBox->insertStringList(lst);
}

bool KSpreadSubtotalDlg::addSubtotal( int mainCol, int column, int row, int topRow,
                                      bool addRow, QString const & text )
{
  kdDebug() << "Adding subtotal: " << mainCol << ", " << column << ", Rows: " << row << ", " << topRow
            << ": addRow: " << addRow << ", Text: " << text << endl;
  if ( addRow )
  {
    QRect rect(m_selection.left(), row + 1, m_selection.width(), 1);
    if ( !m_pTable->shiftColumn( rect ) )
      return false;

    m_selection.setHeight( m_selection.height() + 1 );

    KSpreadCell * cell = m_pTable->nonDefaultCell( mainCol, row + 1 );
    cell->setCellText( text );
    cell->setTextFontBold( true );
    cell->setTextFontItalic( true );
    cell->setTextFontUnderline( true );
  }

  QString colName = util_encodeColumnLabelText( column );

  QString formula("=SUBTOTAL(");
  formula += QString::number( m_dialog->m_functionBox->currentItem() + 1 );
  formula += "; ";
  formula += colName;
  formula += QString::number( topRow );
  // if ( topRow != row )
  {
    formula += ":";
    formula += colName;
    formula += QString::number( row );
  }
  formula += ")";

  KSpreadCell * cell = m_pTable->nonDefaultCell( column, row + 1 );
  cell->setCellText( formula );
  cell->setTextFontBold( true );
  cell->setTextFontItalic( true );
  cell->setTextFontUnderline( true );

  return true;
}

#include "kspread_dlg_subtotal.moc"

