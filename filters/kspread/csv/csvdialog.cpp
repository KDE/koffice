/* This file is part of the KDE project
   Copyright (C) 1999 David Faure <faure@kde.org>

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

#include <dialogui.h>
#include <csvdialog.h>

#include <qtable.h>
#include <qcheckbox.h>
#include <qcursor.h>
#include <qlineedit.h>
#include <qcombobox.h>
#include <qspinbox.h>
#include <qtextstream.h>
#include <qbuttongroup.h>
#include <qpushbutton.h>
#include <qradiobutton.h>

#include <kapplication.h>
#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>

CSVDialog::CSVDialog(QWidget* parent, QByteArray& fileArray, const QString /*seperator*/)
    : KDialogBase(parent, 0, true, QString::null, Ok|Cancel, No, true),
      m_adjustRows(false),
      m_adjustCols(false),
      m_startRow(0),
      m_startCol(0),
      m_endRow(-1),
      m_endCol(-1),
      m_textquote('"'),
      m_delimiter(","),
      m_ignoreDups(false),
      m_fileArray(fileArray),
      m_dialog(new DialogUI(this))
{
    setCaption( i18n( "Import" ) );
    kapp->restoreOverrideCursor();

    fillTable();

    resize(sizeHint());
    setMainWidget(m_dialog);

    m_dialog->m_table->setSelectionMode(QTable::NoSelection);

    connect(m_dialog->m_formatBox, SIGNAL(clicked(int)),
            this, SLOT(formatClicked(int)));
    connect(m_dialog->m_delimiterBox, SIGNAL(clicked(int)),
            this, SLOT(delimiterClicked(int)));
    connect(m_dialog->m_delimiterEdit, SIGNAL(returnPressed()),
            this, SLOT(returnPressed()));
    connect(m_dialog->m_delimiterEdit, SIGNAL(textChanged ( const QString & )),
            this, SLOT(textChanged ( const QString & ) ));
    connect(m_dialog->m_comboQuote, SIGNAL(activated(const QString &)),
            this, SLOT(textquoteSelected(const QString &)));
    connect(m_dialog->m_table, SIGNAL(currentChanged(int, int)),
            this, SLOT(currentCellChanged(int, int)));
    connect(m_dialog->m_ignoreDuplicates, SIGNAL(stateChanged(int)),
            this, SLOT(ignoreDuplicatesChanged(int)));
    connect(m_dialog->m_updateButton, SIGNAL(clicked()),
            this, SLOT(updateClicked()));
}

CSVDialog::~CSVDialog()
{
    kapp->setOverrideCursor(Qt::waitCursor);
}

void CSVDialog::fillTable( )
{
    int row, column;
    bool lastCharDelimiter = false;
    enum { S_START, S_QUOTED_FIELD, S_MAYBE_END_OF_QUOTED_FIELD, S_END_OF_QUOTED_FIELD,
           S_MAYBE_NORMAL_FIELD, S_NORMAL_FIELD } state = S_START;

    QChar x;
    QString field = "";

    kapp->setOverrideCursor(Qt::waitCursor);

    for (row = 0; row < m_dialog->m_table->numRows(); ++row)
        for (column = 0; column < m_dialog->m_table->numCols(); ++column)
            m_dialog->m_table->clearCell(row, column);

    int maxColumn = 1;
    row = column = 1;
    QTextStream inputStream(m_fileArray, IO_ReadOnly);
    inputStream.setEncoding(QTextStream::Locale);

    while (!inputStream.atEnd()) 
    {
        inputStream >> x; // read one char

        if (x == '\r') 
            inputStream >> x; // eat '\r', to handle DOS/LOSEDOWS files correctly

        if ( column > maxColumn )
          maxColumn = column;

        switch (state)
        {
         case S_START :
            if (x == m_textquote)
            {
                state = S_QUOTED_FIELD;
            }
            else if (x == m_delimiter)
            {
                if ((m_ignoreDups == false) || (lastCharDelimiter == false))
                    ++column;
                lastCharDelimiter = true;
            }
            else if (x == '\n')
            {
                ++row;
                column = 1;
                if ( row > ( m_endRow - m_startRow ) && m_endRow >= 0 )
                  break;
            }
            else
            {
                field += x;
                state = S_MAYBE_NORMAL_FIELD;
            }
            break;
         case S_QUOTED_FIELD :
            if (x == m_textquote)
            {
                state = S_MAYBE_END_OF_QUOTED_FIELD;
            }
            else if (x == '\n')
            {
                setText(row - m_startRow, column - m_startCol, field);
                field = "";

                ++row;
                column = 1;
                if ( row > ( m_endRow - m_startRow ) && m_endRow >= 0 )
                  break;

                state = S_START;
            }
            else
            {
                field += x;
            }
            break;
         case S_MAYBE_END_OF_QUOTED_FIELD :
            if (x == m_textquote)
            {
                field += x;
                state = S_QUOTED_FIELD;
            }
            else if (x == m_delimiter || x == '\n')
            {
                setText(row - m_startRow, column - m_startCol, field);
                field = "";
                if (x == '\n')
                {
                    ++row;
                    column = 1;
                    if ( row > ( m_endRow - m_startRow ) && m_endRow >= 0 )
                      break;
                }
                else
                {
                    if ((m_ignoreDups == false) || (lastCharDelimiter == false))
                        ++column;
                    lastCharDelimiter = true;
                }
                state = S_START;
            }
            else
            {
                state = S_END_OF_QUOTED_FIELD;
            }
            break;
         case S_END_OF_QUOTED_FIELD :
            if (x == m_delimiter || x == '\n')
            {
                setText(row - m_startRow, column - m_startCol, field);
                field = "";
                if (x == '\n')
                {
                    ++row;
                    column = 1;
                    if ( row > ( m_endRow - m_startRow ) && m_endRow >= 0 )
                      break;
                }
                else
                {
                    if ((m_ignoreDups == false) || (lastCharDelimiter == false))
                        ++column;
                    lastCharDelimiter = true;
                }
                state = S_START;
            }
            else
            {
                state = S_END_OF_QUOTED_FIELD;
            }
            break;
         case S_MAYBE_NORMAL_FIELD :
            if (x == m_textquote)
            {
                field = "";
                state = S_QUOTED_FIELD;
                break;
            }
         case S_NORMAL_FIELD :
            if (x == m_delimiter || x == '\n')
            {
                setText(row - m_startRow, column - m_startCol, field);
                field = "";
                if (x == '\n')
                {
                    ++row;
                    column = 1;
                    if ( row > ( m_endRow - m_startRow ) && m_endRow >= 0 )
                      break;
                }
                else
                {
                    if ((m_ignoreDups == false) || (lastCharDelimiter == false))
                        ++column;
                    lastCharDelimiter = true;
                }
                state = S_START;
            }
            else
            {
                field += x;
            }
        }
        if (x != m_delimiter)
          lastCharDelimiter = false;
    }

    // file with only one line without '\n'
    if (field.length() > 0)
    {
      setText(row - m_startRow, column - m_startCol, field);
      ++row;
      field = "";
    }
    
    m_adjustCols = true;
    adjustRows( row - m_startRow );
    adjustCols( maxColumn - m_startCol );
    m_dialog->m_colEnd->setMaxValue( maxColumn );
    if ( m_endCol == -1 )
      m_dialog->m_colEnd->setValue( maxColumn );
    

    for (column = 0; column < m_dialog->m_table->numCols(); ++column)
    {
        QString header = m_dialog->m_table->horizontalHeader()->label(column);
        if (header != i18n("Text") && header != i18n("Number") &&
            header != i18n("Date") && header != i18n("Currency"))
            m_dialog->m_table->horizontalHeader()->setLabel(column, i18n("Text"));

        m_dialog->m_table->adjustColumn(column);
    }
    fillComboBox();

    kapp->restoreOverrideCursor();
}

void CSVDialog::fillComboBox()
{
  if ( m_endRow == -1 )
    m_dialog->m_rowEnd->setValue( m_dialog->m_table->numRows() );  
  else
    m_dialog->m_rowEnd->setValue( m_endRow );

  if ( m_endCol == -1 )
    m_dialog->m_colEnd->setValue( m_dialog->m_table->numCols() );
  else
    m_dialog->m_colEnd->setValue( m_endCol );  

  m_dialog->m_rowEnd->setMinValue( 1 );
  m_dialog->m_colEnd->setMinValue( 1 );
  m_dialog->m_rowEnd->setMaxValue( m_dialog->m_table->numRows() );
  m_dialog->m_colEnd->setMaxValue( m_dialog->m_table->numCols() );

  m_dialog->m_rowStart->setMinValue( 1 );
  m_dialog->m_colStart->setMinValue( 1 );
  m_dialog->m_rowStart->setMaxValue( m_dialog->m_table->numRows() );
  m_dialog->m_colStart->setMaxValue( m_dialog->m_table->numCols() );
}

int CSVDialog::getRows()
{
  int rows = m_dialog->m_table->numRows();
  if ( m_endRow >= 0 )
  {
    if ( rows > ( m_startRow + m_endRow ) )
      rows = m_startRow + m_endRow;
  }

  return rows;
}

int CSVDialog::getCols()
{
  int cols = m_dialog->m_table->numCols();
  if ( m_endCol >= 0 )
  {
    if ( cols > ( m_startCol + m_endCol ) )
      cols = m_startCol + m_endCol;
  }

  return cols;
}

int CSVDialog::getHeader(int col)
{
    QString header = m_dialog->m_table->horizontalHeader()->label(col);

    if (header == i18n("Text"))
        return TEXT;
    else if (header == i18n("Number"))
        return NUMBER;
    else if (header == i18n("Currency"))
        return CURRENCY;
    else
        return DATE;
}

QString CSVDialog::getText(int row, int col)
{
    return m_dialog->m_table->text( row, col );
}

void CSVDialog::setText(int row, int col, const QString& text)
{
    if ( row < 1 || col < 1 ) // skipped by the user
        return;

    if ( ( row > ( m_endRow - m_startRow ) && m_endRow > 0 ) || ( col > ( m_endCol - m_startCol ) && m_endCol > 0 ) )
      return;

    if ( m_dialog->m_table->numRows() < row ) 
    {
        m_dialog->m_table->setNumRows( row + 5000 ); /* We add 5000 at a time to limit recalculations */
        m_adjustRows = true;
    }

    if ( m_dialog->m_table->numCols() < col )
    {
        m_dialog->m_table->setNumCols( col );
        m_adjustCols = true;
    }

    m_dialog->m_table->setText( row - 1, col - 1, text );
}

/*
 * Called after the first fillTable() when number of rows are unknown.
 */
void CSVDialog::adjustRows(int iRows)
{
    if (m_adjustRows) 
    {
        m_dialog->m_table->setNumRows( iRows );
        m_adjustRows = false;
    }
}

void CSVDialog::adjustCols(int iCols)
{
    if (m_adjustCols) 
    {  
        m_dialog->m_table->setNumCols( iCols );
        m_adjustCols = false;

        if ( m_endCol == -1 )
        {
          if ( iCols > ( m_endCol - m_startCol ) )
            iCols = m_endCol - m_startCol;

          m_dialog->m_table->setNumCols( iCols );
        }
    }
}

void CSVDialog::returnPressed()
{
    if (m_dialog->m_delimiterBox->id(m_dialog->m_delimiterBox->selected()) != 4)
        return;

    m_delimiter = m_dialog->m_delimiterEdit->text();
    fillTable();
}

void CSVDialog::textChanged ( const QString & )
{
    m_dialog->m_radioOther->setChecked ( true );
    delimiterClicked(4); // other
}

void CSVDialog::formatClicked(int id)
{
    QString header;

    switch (id)
    {
    case 1: // text
        header = i18n("Text");
        break;
    case 0: // number
        header = i18n("Number");
        break;
    case 3: // date
        header = i18n("Date");
        break;
    case 2: // currency
        header = i18n("Currency");
        break;
    }

    m_dialog->m_table->horizontalHeader()->setLabel(m_dialog->m_table->currentColumn(), header);
}

void CSVDialog::delimiterClicked(int id)
{
    switch (id)
    {
    case 0: // comma
        m_delimiter = ",";
        break;
    case 4: // other
        m_delimiter = m_dialog->m_delimiterEdit->text();
        break;
    case 2: // tab
        m_delimiter = "\t";
        break;
    case 3: // space
        m_delimiter = " ";
        break;
    case 1: // semicolon
        m_delimiter = ";";
        break;
    }

    fillTable();
}

void CSVDialog::textquoteSelected(const QString& mark)
{
    if (mark == i18n("none"))
        m_textquote = 0;
    else
        m_textquote = mark[0];

    fillTable();
}

void CSVDialog::updateClicked()
{
  if ( !checkUpdateRange() )
    return;

  m_startRow = m_dialog->m_rowStart->value() - 1;
  m_endRow   = m_dialog->m_rowEnd->value();

  m_startCol  = m_dialog->m_colStart->value() - 1;
  m_endCol    = m_dialog->m_colEnd->value();

  fillTable();
}

bool CSVDialog::checkUpdateRange()
{
  if ( ( m_dialog->m_rowStart->value() > m_dialog->m_rowEnd->value() ) 
       || ( m_dialog->m_colStart->value() > m_dialog->m_colEnd->value() ) )
  {
    KMessageBox::error( this, i18n( "Please check the ranges you specified. The start value must be lower than the end value." ) );
    return false;
  }

  return true;
}

void CSVDialog::currentCellChanged(int, int col)
{
    int id;
    QString header = m_dialog->m_table->horizontalHeader()->label(col);

    if (header == i18n("Text"))
        id = 1;
    else if (header == i18n("Number"))
        id = 0;
    else if (header == i18n("Date"))
        id = 3;
    else
        id = 2;

    m_dialog->m_formatBox->setButton(id);
}

void CSVDialog::ignoreDuplicatesChanged(int)
{
  if (m_dialog->m_ignoreDuplicates->isChecked())
    m_ignoreDups = true;
  else
    m_ignoreDups = false;
  fillTable();
}

#include <csvdialog.moc>
