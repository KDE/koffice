/****************************************************************************
** $Id$
**
** Copyright (C) 1992-1998 Troll Tech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

/****************************************************
 * modified by Reginald Stadlbauer <reggie@kde.org> *
 ****************************************************/


#include <stdlib.h>

#include <klocale.h>

#include <qscrollbar.h>

#include <qtooltip.h>
#include <qwhatsthis.h>

#include "sheet.h"
#include "sheet.moc"


Sheet::Sheet( QWidget *parent, const char *name, int _tableSize )
    :QWidget(parent,name)
{
    tableSize = _tableSize;

    table = new ParsedArray(tableSize, tableSize);
    _head = new ParsedArray(1,         tableSize);
    _side = new ParsedArray(tableSize, 1);

    tableView = new SheetTable(tableSize, tableSize, this, Tbl_clipCellPainting );
    head      = new SheetTable(tableSize, 1,         this, Tbl_clipCellPainting);
    side      = new SheetTable(1,         tableSize, this, Tbl_clipCellPainting, "Side");
    
    head->setPaletteBackgroundColor(cyan);
    side->setPaletteBackgroundColor(cyan);
    
    
    QToolTip::add(head,i18n("Names for all data values at each position"));
    QWhatsThis::add(head,i18n("<p><b>Set names for data values.</b><br><br>"
        "Each column represents one data value of each data set (row),"
        " you can change the name of the data values by entering text in"
        " each column header. These names are represented along the"
        " x-axis of the chart.<br> These names could for example"
        " represent years/dates or elapsed time.</p>"));

    QToolTip::add(side,i18n("Names of the data sets"));
    QWhatsThis::add(side,i18n("<p><b>Set names for data sets.</b><br><br>"
      "Each data set that is represented by a row can have its own name."
      " This name is also shown in the Legend of the chart. You can set"
      " the name of each data set by entering text in this first column.</p>"));
                  
    QToolTip::add(tableView,i18n("Chart data, each row is a data set."));
    QWhatsThis::add(tableView,i18n("<p>This table represents the complete data"
    " for the chart.<br><br> Each row is one data set of values."
    " The name of such a data set can be changed in the first column (on the left)"
    " of the table. In a line diagram each row is one line. In a ring diagram each row"
    " is one slice. <br><br> Each column represents one value of each data set."
    " Just like rows you can also change the name of each value in the"
    " first row (at the top) of the table. In a bar diagram the number of columns"
    " defines the number of value sets. In a ring diagram each column is one ring.</p>"));
    
                  
    extraH = head->tHeight() + 2;
    extraW = side->tWidth()  + 2;

    head->move(extraW,0);
    side->move(0,extraH);
    tableView->move(extraW, extraH);

    connect( tableView, SIGNAL(selected(int, int)),
             this, SLOT(exportText(int, int)) );
    connect( tableView, SIGNAL(newText(int, int, QString)),
             this, SLOT(importText(int, int, QString)) );

    connect( head, SIGNAL(selected(int, int)),
             this, SLOT(exportTextHead(int, int)) );
    connect( head, SIGNAL(newText(int, int, QString)),
             this, SLOT(importTextHead(int, int, QString)) );

    connect( side, SIGNAL(selected(int, int)),
             this, SLOT(exportTextSide(int, int)) );
    connect( side, SIGNAL(newText(int,int, QString)),
             this, SLOT(importTextSide(int, int, QString)) );

    // Horizontal scrollbar
    horz = new QScrollBar( QScrollBar::Horizontal, this, "scrollBar" );
    horz->resize( tableView->width(), 16 );
    horz->setRange( 0, tableView->numCols() - tableView->numColsVisible() );
    horz->setSteps( 1, tableView->numColsVisible() );

    connect( tableView, SIGNAL(newCol(int)), head, SLOT(scrollHorz(int)));
    connect( tableView, SIGNAL(newCol(int)), this, SLOT(setHorzBar(int)) );
    connect( head, SIGNAL(newCol(int)), this, SLOT(setHorzBar(int)) );
    connect( horz, SIGNAL(valueChanged(int)),
             tableView, SLOT(scrollHorz(int)));
    connect( horz, SIGNAL(valueChanged(int)),
             head, SLOT(scrollHorz(int)));
    extraH += horz->height();

    // Vertical scrollbar
    vert = new QScrollBar( QScrollBar::Vertical, this, "scrollBar" );
    vert->resize( 16, tableView->width() );
    vert->setRange( 0, tableView->numRows() - tableView->numRowsVisible() );
    vert->setSteps( 1, tableView->numRowsVisible() );

    connect( tableView, SIGNAL(newRow(int)), side, SLOT(scrollVert(int)) );
    connect( tableView, SIGNAL(newRow(int)), this, SLOT(setVertBar(int)) );
    connect( side, SIGNAL(newRow(int)), this, SLOT(setVertBar(int)) );
    connect( vert, SIGNAL(valueChanged(int)),
             tableView, SLOT(scrollVert(int)));
    connect( vert, SIGNAL(valueChanged(int)),
             side, SLOT(scrollVert(int)));
    extraW += vert->width();

    // Set row and column labels to useful default values.
    QString str;
    for (int i = 0;i <tableSize;i++) {
        str.sprintf("Data Set %d",i+1);
        importTextSide(i,0,str);
        str.sprintf("Value %d",i+1);
        importTextHead(0,i,str);
    }

}

Sheet::~Sheet()
{
  delete tableView;
  delete side;
  delete head;

  delete table;
  delete _side;
  delete _head;
}


int Sheet::rows()
{
  int   row, col, rows;
  bool  colValid;

  rows = 0;
  for (row = 0; row < tableSize; row++) {
      colValid = false;
      for (col = 0; col < tableSize; col++) {
          if (tableView->hasValue(row, col))
	      colValid = true;
      }
      if (!colValid) 
	  return rows;
      else
	  rows++;
  }

  return rows;
}


int Sheet::cols()
{
  int   row, col, cols;
  bool  rowValid;

  cols = 0;
  for (col = 0;col < tableSize;col++) {
      rowValid = false;
      for (row = 0; row < tableSize; row++) {
          if (tableView->hasValue(row, col))
	      rowValid = true;
      }
      if (!rowValid)
	  return cols;
      else
	  cols++;
  }

  return cols;
}


QString Sheet::getX(int col)
{
    return _head->rawText( 0, col );
}


QString Sheet::getY(int row)
{
    return _side->rawText( row, 0 );
}


double Sheet::getCell(int row, int col)
{
    if (table->rawText(row,col).isEmpty())
	return 0.0;
    else
	return table->rawText(row,col).toDouble();
}


void Sheet::setHorzBar(int val)
{
    horz->setValue(val);
}


void Sheet::setVertBar(int val)
{
    vert->setValue(val);
}


// Set the contents of cell at (row, col) to 'str', calculate the
// value and set the corresponding cellview to that value.
//

void Sheet::importText( int row, int col, QString str )
{
    table->setText( row, col, str.copy() );
    tableView->showText( row, col, table->calc( row, col ) );
}


// Set the input field of the cellview to the text in cell at
// (row, col) in the document.
//

void Sheet::exportText( int row, int col )
{
    tableView->setInputText( table->rawText(row, col) );
}


void Sheet::importTextHead( int row, int col, QString s )
{
    _head->setText( row, col, s.copy() );
    head->showText( row, col, _head->calc( row, col ) );
}


void Sheet::exportTextHead( int row, int col )
{
    head->setInputText( _head->rawText(row,col) );
}


void Sheet::importTextSide( int row, int col, QString s )
{
    _side->setText( row, col, s.copy() );
    side->showText( row, col, _side->calc( row, col ) );
}


void Sheet::exportTextSide( int row, int col )
{
    side->setInputText( _side->rawText(row,col) );
}


// ----------------------------------------------------------------
//                       Protected methods


void Sheet::resizeEvent( QResizeEvent * e )
{
    // Calculate width and height that we have available for the table.
    int w = e->size().width() - extraW;
    int h = e->size().height() - extraH;

    // Calculate the number of columns and rows that we have room for.
    int c = w / tableView->cellWidth(); //### TODO: variable width
    int r = h / tableView->cellHeight();

    // Now calculate width and height that this number of cells take up.
    w = c * tableView->cellWidth();
    h = r * tableView->cellHeight();

    // Resize the table, header and side views to the new sizes.
    //side->setNumRows( r );
    side->resize( side->tWidth(), h );
    //head->setNumCols( c );
    head->resize( w, head->height() );
    tableView->resize( w + tableView->extraW, h + tableView->extraH );

    // Resize the scrollbars
    QRect cr = tableView->geometry();
    horz->setGeometry( cr.left(), cr.bottom() + 1,
                       cr.width(), horz->height() );
    horz->setRange( 0, tableView->numCols() - tableView->numColsVisible() );
    horz->setSteps( 1, tableView->numColsVisible() );

    vert->setGeometry( cr.right() + 1, cr.top(),
                       vert->width(), cr.height() );
    vert->setRange( 0, tableView->numRows() - tableView->numRowsVisible() );
    vert->setSteps( 1, tableView->numRowsVisible() );
}
