/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>

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


#include "kspread_dlg_cons.h"
#include "kspread_canvas.h"
#include "kspread_doc.h"
#include "kspread_util.h"
#include "kspread_table.h"
#include "kspread_global.h"
#include "kspread_interpreter.h"

#include <koscript.h>

#include <kmessagebox.h>
#include <qlayout.h>
#include <assert.h>
#include <qcombobox.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <kdebug.h>
#include <qpushbutton.h>

KSpreadConsolidate::KSpreadConsolidate( KSpreadView* parent, const char* name )
	: QDialog( parent, name )
{
  m_pView = parent;

  setCaption( i18n("Consolidate") );

  QGridLayout *grid1 = new QGridLayout(this,12,2,15,7);

  QLabel* tmpQLabel;
  tmpQLabel = new QLabel( this, "Label_1" );
  grid1->addWidget(tmpQLabel,0,0);
  tmpQLabel->setText( i18n("&Function:") );  
  
  m_pFunction = new QComboBox( this );
  grid1->addWidget(m_pFunction,1,0);
  tmpQLabel->setBuddy(m_pFunction);

  m_pFunction->insertItem( i18n("Sum"), Sum );
  m_pFunction->insertItem( i18n("Average"), Average );
  m_pFunction->insertItem( i18n("Count"), Count );
  m_pFunction->insertItem( i18n("Max"), Max );
  m_pFunction->insertItem( i18n("Min"), Min );
  m_pFunction->insertItem( i18n("Product"), Product );
  m_pFunction->insertItem( i18n("Standard Deviation"), StdDev );
  m_pFunction->insertItem( i18n("Variance"), Var );

  tmpQLabel = new QLabel( this, "Label_1" );
  tmpQLabel->setText( i18n("Re&ference:") );
  grid1->addWidget(tmpQLabel,2,0);

  m_pRef = new QLineEdit( this );
  grid1->addWidget(m_pRef,3,0);
  tmpQLabel->setBuddy(m_pRef);
  
  tmpQLabel = new QLabel( this, "Label_1" );
  grid1->addWidget(tmpQLabel,4,0);
  tmpQLabel->setText( i18n("&Entered references:") );

  m_pRefs = new QListBox( this );
  grid1->addMultiCellWidget( m_pRefs,5,8,0,0);
  tmpQLabel->setBuddy(m_pRefs);
  
  m_pRow = new QCheckBox( i18n("&Description in row"), this );
  grid1->addWidget( m_pRow,9,0);
  m_pCol = new QCheckBox( i18n("De&scription in column"), this );
  grid1->addWidget(m_pCol,10,0);
  m_pCopy = new QCheckBox( i18n("Co&py data"), this );
  grid1->addWidget(m_pCopy,11,0);

  m_pOk = new QPushButton( i18n("&OK"), this );
  grid1->addWidget(m_pOk,0,1);
  m_pOk->setEnabled( false );
  m_pCancel= new QPushButton( i18n("&Cancel"), this );
  grid1->addWidget(m_pCancel,1,1);

  m_pAdd = new QPushButton( i18n("&Add"), this );
  grid1->addWidget(m_pAdd,2,1);
  m_pRemove = new QPushButton( i18n("&Remove"), this );
  grid1->addWidget(m_pRemove,3,1);


  connect( m_pOk, SIGNAL( clicked() ), this, SLOT( slotOk() ) );
  connect( m_pCancel, SIGNAL( clicked() ), this, SLOT( slotCancel() ) );
  connect( m_pAdd, SIGNAL( clicked() ), this, SLOT( slotAdd() ) );
  connect( m_pRemove, SIGNAL( clicked() ), this, SLOT( slotRemove() ) );
  connect( m_pRef, SIGNAL( returnPressed() ), this, SLOT( slotReturnPressed() ) );

  connect( m_pView, SIGNAL( sig_selectionChanged( KSpreadTable*, const QRect& ) ),
	   this, SLOT( slotSelectionChanged( KSpreadTable*, const QRect& ) ) );
}

KSpreadConsolidate::~KSpreadConsolidate()
{
    kdDebug(36001)<<"KSpreadConsolidate::~KSpreadConsolidate()\n";
}

enum Description { D_ROW, D_COL, D_NONE, D_BOTH };

struct st_cell
{
  QString xdesc;
  QString ydesc;
  KSpreadCell* cell;
  QString table;
  int x;
  int y;
};

void KSpreadConsolidate::slotOk()
{
  KSpreadMap *map = m_pView->doc()->map();

  KSpreadTable* table = m_pView->activeTable();
  int dx = m_pView->canvasWidget()->markerColumn();
  int dy = m_pView->canvasWidget()->markerRow();

  QString function;

  switch( m_pFunction->currentItem() )
  {
    case Sum:      function = "SUM"; break;
    case Average:  function = "AVERAGE"; break;
    case Count:    function = "COUNT"; break;
    case Max:      function = "MAX"; break;
    case Min:      function = "MIN"; break;
    case Product:  function = "PRODUCT"; break;
    case StdDev:   function = "STDDEV"; break;
    case Var:      function = "VARIANCE"; break;
    default: break; // bad bad !
  }

  QStringList r = refs();
  QValueList<KSpreadRange> ranges;
  QStringList::Iterator s = r.begin();
  for( ; s != r.end(); ++s )
  {
    KSpreadRange r( *s, map );
    // TODO: Check for valid
    Q_ASSERT( r.isValid() );

    if ( r.table == 0 )
    {
      r.table = table;
      r.tableName = table->tableName();
    }
    ranges.append( r  );
  }

  Description desc;
  if ( m_pRow->isChecked() && m_pCol->isChecked() )
    desc = D_BOTH;
  else if ( m_pRow->isChecked() )
    desc = D_ROW;
  else if ( m_pCol->isChecked() )
    desc = D_COL;
  else
    desc = D_NONE;

  // Check whether all ranges have same size
  Q_ASSERT( ranges.count() > 0 );
  QValueList<KSpreadRange>::Iterator it = ranges.begin();
  int w = (*it).range.right() - (*it).range.left() + 1;
  int h = (*it).range.bottom() - (*it).range.top() + 1;
  if ( w <= ( ( desc == D_BOTH || desc == D_COL ) ? 1 : 0 ) ||
       h <= ( ( desc == D_BOTH || desc == D_ROW ) ? 1 : 0 ) )
  {
    KMessageBox::error( this, i18n( "The range\n%1\nis too small" ).arg( *( r.begin() ) ));
    return;
  }

  if( (*it).range.bottom()==KS_rowMax || (*it).range.right()== KS_colMax )
  {
    KMessageBox::error( this, i18n( "The range\n%1\nis too large" ).arg( *( r.begin() ) ));
    return;
  }

  ++it;
  int i = 1;
  for( ; it != ranges.end(); ++it, i++ )
  {
    int w2 = (*it).range.right() - (*it).range.left() + 1;
    int h2 = (*it).range.bottom() - (*it).range.top() + 1;
    if((*it).range.bottom()==KS_rowMax || (*it).range.right()== KS_colMax)
    {
      KMessageBox::error( this, i18n( "The range\n%1\nis too large" ).arg( r[i]));
      return;
    }
    if ( ( desc == D_NONE && ( w != w2 || h != h2 ) ) ||
	 ( desc == D_ROW && h != h2 ) ||
	 ( desc == D_COL && w != w2 ) )
    {
      QString tmp = i18n( "The ranges\n%1\nand\n%2\nhave different size").arg( *( r.begin() ) ).arg( r[i] );
      KMessageBox::error( this, tmp);
      return;
    }
  }

  // Create the consolidation table
  if ( desc == D_NONE )
  {
    // Check whether the destination is part of the source ...
    QRect dest;
    dest.setCoords( dx, dy, dx + w - 1, dy + h - 1 );
    it = ranges.begin();
    for( ; it != ranges.end(); ++it )
    {
      KSpreadTable *t = (*it).table;
      Q_ASSERT( t );
      QRect r;
      r.setCoords( (*it).range.left(), (*it).range.top(), (*it).range.right(), (*it).range.bottom() );
      if ( t == table && r.intersects( dest ) )
      {
	QString tmp( i18n("The source tables intersect with the destination table") );
	KMessageBox::error( this, tmp);
	return;
      }
    }

    for( int x = 0; x < w; x++ )
    {
      for( int y = 0; y < h; y++ )
      {
        bool novalue=true;
        QString formula = "=" + function + "(";
	it = ranges.begin();
	for( ; it != ranges.end(); ++it )
        {
	  KSpreadTable *t = (*it).table;
	  assert( t );
	  KSpreadCell *c = t->cellAt( x + (*it).range.left(), y + (*it).range.top() );
          if(!c->isDefault())
                novalue=false;
	  if ( it != ranges.begin() ) formula += ";";
	  formula += (*it).tableName + "!";
	  formula += util_cellName( x + (*it).range.left(), y + (*it).range.top() );
	}
	formula += ")";

        if(!novalue)
	  table->setText( dy + y, dx + x, 
            m_pCopy->isChecked() ? evaluate( formula, table ) : formula );
      }
    }
  }
  else if ( desc == D_ROW )
  {
    // Get list of all descriptions in the rows
    QStringList lst;
    it = ranges.begin();
    for( ; it != ranges.end(); ++it )
    {
      KSpreadTable *t = (*it).table;
      assert( t );
      kdDebug(36001) << "FROM " << (*it).range.left() << " to " << (*it).range.right() << endl;
      for( int x = (*it).range.left(); x <= (*it).range.right() ; ++x )
      {
	KSpreadCell *c = t->cellAt( x, (*it).range.top() );
	if ( c )
	{
	  QString s = c->valueString();
	  if ( !lst.contains( s ) )
	    lst.append( s );
	}
      }
    }
    lst.sort();

    // Check whether the destination is part of the source ...
    QRect dest;
    dest.setCoords( dx, dy, dx + lst.count() - 1, dy + h - 1 );
    it = ranges.begin();
    for( ; it != ranges.end(); ++it )
    {
      KSpreadTable *t = (*it).table;
      assert( t );
      QRect r;
      r.setCoords( (*it).range.left(), (*it).range.top(), (*it).range.right(), (*it).range.bottom() );
      if ( t == table && r.intersects( dest ) )
      {
	QString tmp( i18n("The source tables intersect with the destination table") );
	KMessageBox::error( this, tmp);
	return;
      }
    }

    // Now create the consolidation table
    int x = 0;
    QStringList::Iterator s = lst.begin();
    for( ; s != lst.end(); ++s, ++x )
    {
      table->setText( dy, dx + x, *s );

      for( int y = 1; y < h; ++y )
      {
	int count = 0;
        QString formula = "=" + function + "(";
	it = ranges.begin();
	for( ; it != ranges.end(); ++it )
        {
	  for( int i = (*it).range.left(); i <= (*it).range.right(); ++i )
	  {
	    KSpreadTable *t = (*it).table;
	    assert( t );
	    KSpreadCell *c = t->cellAt( i, (*it).range.top() );
	    if ( c )
	    {
	      if ( c->valueString() == *s )
	      {
//		KSpreadCell *c2 = t->cellAt( i, y + (*it).range.top() );
		count++;
		if ( it != ranges.begin() ) formula += ";";
		formula += (*it).tableName + "!";
		formula += util_cellName( i, y + (*it).range.top() );
	      }
	    }
	  }
	}
	formula += ")";

	table->setText( dy + y, dx + x, 
          m_pCopy->isChecked() ? evaluate( formula, table ) : formula );
      }
    }
  }
  else if ( desc == D_COL )
  {
    // Get list of all descriptions in the columns
    QStringList lst;
    it = ranges.begin();
    for( ; it != ranges.end(); ++it )
    {
      KSpreadTable *t = (*it).table;
      assert( t );
      for( int y = (*it).range.top(); y <= (*it).range.bottom() ; ++y )
      {
	KSpreadCell *c = t->cellAt( (*it).range.left(), y );
	if ( c )
	{
	  QString s = c->valueString();
	  if ( !s.isEmpty() && lst.find( s ) == lst.end() )
	    lst.append( s );
	}
      }
    }
    lst.sort();

    // Check whether the destination is part of the source ...
    QRect dest;
    dest.setCoords( dx, dy, dx + w - 1, dy + lst.count() - 1 );
    it = ranges.begin();
    for( ; it != ranges.end(); ++it )
    {
      KSpreadTable *t = (*it).table;
      assert( t );
      QRect r;
      r.setCoords( (*it).range.left(), (*it).range.top(), (*it).range.right(), (*it).range.bottom() );
      if ( t == table && r.intersects( dest ) )
      {
	QString tmp( i18n("The source tables intersect with the destination table") );
	KMessageBox::error( this, tmp);
	return;
      }
    }

    // Now create the consolidation table
    int y = 0;
    QStringList::Iterator s = lst.begin();
    for( ; s != lst.end(); ++s, ++y )
    {
      table->setText( dy + y, dx, *s );

      for( int x = 1; x < w; ++x )
      {
	int count = 0;
        QString formula = "=" + function + "(";
	it = ranges.begin();
	for( ; it != ranges.end(); ++it )
        {
	  for( int i = (*it).range.top(); i <= (*it).range.bottom(); i++ )
	  {
	    KSpreadTable *t = (*it).table;
	    assert( t );
	    KSpreadCell *c = t->cellAt( (*it).range.left(), i );
	    if ( c )
	    {
	      QString v = c->valueString();
	      if ( !v.isEmpty() && *s == v )
	      {
//		KSpreadCell *c2 = t->cellAt( x + (*it).range.left(), i );
		count++;
		if ( it != ranges.begin() ) formula += ";";
		formula += (*it).tableName + "!";
		formula += util_cellName( i, y + (*it).range.top() );
	      }
	    }
	  }
	}

	formula += ")";

	table->setText( dy + y, dx + x, 
          m_pCopy->isChecked() ? evaluate( formula, table ) : formula );
      }
    }
  }
  else if ( desc == D_BOTH )
  {
    // Get list of all descriptions in the columns
    QStringList cols;
    it = ranges.begin();
    for( ; it != ranges.end(); ++it )
    {
      KSpreadTable *t = (*it).table;
      assert( t );
      for( int y = (*it).range.top() + 1; y <= (*it).range.bottom() ; ++y )
      {
	KSpreadCell *c = t->cellAt( (*it).range.left(), y );
	if ( c )
	{
	  QString s = c->valueString();
	  if ( !s.isEmpty() && cols.find( s ) == cols.end() )
	    cols.append( s );
	}
      }
    }
    cols.sort();

    // Get list of all descriptions in the rows
    QStringList rows;
    it = ranges.begin();
    for( ; it != ranges.end(); ++it )
    {
      KSpreadTable *t = (*it).table;
      assert( t );
      for( int x = (*it).range.left() + 1; x <= (*it).range.right() ; ++x )
      {
	KSpreadCell *c = t->cellAt( x, (*it).range.top() );
	if ( c )
	{
	  QString s = c->valueString();
	  if ( !s.isEmpty() && rows.find( s ) == rows.end() )
	    rows.append( s );
	}
      }
    }
    rows.sort();

    // Check whether the destination is part of the source ...
    QRect dest;
    dest.setCoords( dx, dy, dx + cols.count(), dy + rows.count() );
    it = ranges.begin();
    for( ; it != ranges.end(); ++it )
    {
      KSpreadTable *t = (*it).table;
      assert( t );
      QRect r;
      r.setCoords( (*it).range.left(), (*it).range.top(), (*it).range.right(), (*it).range.bottom() );
      if ( t == table && r.intersects( dest ) )
      {
	QString tmp( i18n("The source tables intersect with the destination table") );
	KMessageBox::error( this, tmp);
	return;
      }
    }

    // Fill the list with all interesting cells
    QValueList<st_cell> lst;
    it = ranges.begin();
    for( ; it != ranges.end(); ++it )
    {
      KSpreadTable *t = (*it).table;
      assert( t );
      for( int x = (*it).range.left() + 1; x <= (*it).range.right() ; ++x )
      {
	KSpreadCell *c = t->cellAt( x, (*it).range.top() );
	if ( c )
	{
	  QString ydesc = c->valueString();
	  for( int y = (*it).range.top() + 1; y <= (*it).range.bottom() ; ++y )
	  {
	    KSpreadCell *c2 = t->cellAt( (*it).range.left(), y );
	    if ( c2 )
	    {
	      QString xdesc = c2->valueString();
	      KSpreadCell *c3 = t->cellAt( x, y );
	      if ( c3 && c3->isNumeric() )
	      {
		st_cell k;
		k.xdesc = xdesc;
		k.ydesc = ydesc;
		k.cell = c3;
		k.table = (*it).tableName;
		k.x = x;
		k.y = y;
		lst.append( k );
	      }
	    }
	  }
	}
      }
    }

    // Draw the row description
    int i = 1;
    QStringList::Iterator s = rows.begin();
    for( ; s != rows.end(); ++s, ++i )
      table->setText( dy, dx + i, *s );

    // Draw the column description
    i = 1;
    s = cols.begin();
    for( ; s != cols.end(); ++s, ++i )
      table->setText( dy + i, dx, *s );

    // Draw the data
    int x = 1;
    QStringList::Iterator ydesc = rows.begin();
    for( ; ydesc != rows.end(); ++ydesc, x++ )
    {
      int y = 1;
      QStringList::Iterator xdesc = cols.begin();
      for( ; xdesc != cols.end(); ++xdesc, y++ )
      {
	int count = 0;
        QString formula = "=" + function + "(";
	QValueList<st_cell>::Iterator lit = lst.begin();
	for( ; lit != lst.end(); ++lit )
	{
	  if ( (*lit).xdesc == *xdesc && (*lit).ydesc == *ydesc )
	  {
	    count++;
  	    if ( it != ranges.begin() ) formula += ";";
	    formula += (*it).tableName + "!";
	    formula += util_cellName( i, y + (*it).range.top() );
	  }
	}
	formula += ")";

	table->setText( dy + y, dx + x, 
          m_pCopy->isChecked() ? evaluate( formula, table ) : formula );
      }
    }
  }
  m_pView->updateEditWidget();
  accept();
  delete this;
}

void KSpreadConsolidate::slotCancel()
{
  reject();
  delete this;
}

void KSpreadConsolidate::slotAdd()
{
  slotReturnPressed();
}

void KSpreadConsolidate::slotRemove()
{
  int i = m_pRefs->currentItem();
  if ( i < 0 )
    return;

  m_pRefs->removeItem( i );

  if ( m_pRefs->count() == 0 )
    m_pOk->setEnabled( false );
}

QStringList KSpreadConsolidate::refs()
{
  QStringList list;
  int c = m_pRefs->count();

  for( int i = 0; i < c; i++ )
    list.append( m_pRefs->text( i ) );

  return list;
}

void KSpreadConsolidate::slotSelectionChanged( KSpreadTable* _table, const QRect& _selection )
{
  if ( _selection.left() == 0 || _selection.top() == 0 ||
       _selection.right() == 0 || _selection.bottom() == 0 )
  {
    m_pRef->setText( "" );
    return;
  }

  QString area = util_rangeName( _table, _selection );
  m_pRef->setText( area );
  m_pRef->setSelection( 0, area.length() );
}

void KSpreadConsolidate::slotReturnPressed()
{
  QString txt = m_pRef->text();

  KSpreadRange r( txt, m_pView->doc()->map() );
  if ( !r.isValid() )
  {
    KMessageBox::error( this, i18n("The range\n%1\n is malformed").arg( txt ));
    return;
  }

  if ( !txt.isEmpty() )
  {
    m_pRefs->insertItem( txt );
    m_pOk->setEnabled( true );
  }
}

void KSpreadConsolidate::closeEvent ( QCloseEvent * )
{
    delete this;
}

QString KSpreadConsolidate::evaluate( const QString& formula, KSpreadTable* table )
{
  QString result = "###";

  kdDebug(36001)<<"KSpreadConsolidate::evaluate " << formula << endl;
 
  KSContext context;
  QPtrList<KSpreadDependency> lst;

  // parse and evaluate formula
  KSParseNode* code = table->doc()->interpreter()->parse( context, 
    table, formula, lst );
  if( !code ) return result;

  context = table->doc()->context();
  if ( !table->doc()->interpreter()->evaluate( context, code, table ) )
    return result;

  if ( context.value()->type() == KSValue::DoubleType )
     return QString::number( context.value()->doubleValue() );

  if ( context.value()->type() == KSValue::IntType )
     return QString::number( context.value()->intValue() );

  return result;
}

#include "kspread_dlg_cons.moc"
