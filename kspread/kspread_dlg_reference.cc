/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>
   Copyright (C) 1999, 2000, 2001  Montel Laurent <lmontel@mandrakesoft.com>

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



#include "kspread_dlg_reference.h"
#include "kspread_canvas.h"
#include "kspread_doc.h"
#include "kspread_util.h"
#include "kspread_map.h"
#include <qlayout.h>
#include <kbuttonbox.h>
#include <kmessagebox.h>
#include <kdebug.h>

KSpreadreference::KSpreadreference( KSpreadView* parent, const char* name )
  : QDialog( parent, name,TRUE )
{
  m_pView = parent;
  QVBoxLayout *lay1 = new QVBoxLayout( this );
  lay1->setMargin( 5 );
  lay1->setSpacing( 10 );
  m_list = new QListBox(this);
  lay1->addWidget( m_list );

  setCaption( i18n("Area Name") );

  m_rangeName = new QLabel(this);
  lay1->addWidget(m_rangeName);

  m_pRemove = new QPushButton(i18n("&Remove..."), this);
  lay1->addWidget( m_pRemove );
  //m_pRemove->setEnabled(false);
  KButtonBox *bb = new KButtonBox( this );
  bb->addStretch();
  m_pOk = bb->addButton( i18n("&OK") );
  m_pOk->setDefault( TRUE );
  m_pCancel = bb->addButton( i18n( "&Cancel" ) );
  bb->layout();
  lay1->addWidget( bb );

  QString text;
  QValueList<Reference>::Iterator it;
  QValueList<Reference> area = m_pView->doc()->listArea();
  for ( it = area.begin(); it != area.end(); ++it )
  {
    text = (*it).ref_name;
    m_list->insertItem(text);
  }

  if (!m_list->count())
  {
    m_pOk->setEnabled(false);
    m_pRemove->setEnabled(false);
  }

  connect( m_pOk, SIGNAL( clicked() ), this, SLOT( slotOk() ) );
  connect( m_pCancel, SIGNAL( clicked() ), this, SLOT( slotCancel() ) );
  connect( m_pRemove, SIGNAL( clicked() ), this, SLOT( slotRemove() ) );
  connect( m_list, SIGNAL(doubleClicked(QListBoxItem *)), this,
           SLOT(slotDoubleClicked(QListBoxItem *)));
  connect( m_list, SIGNAL(highlighted ( QListBoxItem * ) ), this,
           SLOT(slotHighlighted(QListBoxItem * )));
  m_rangeName->setText(i18n("Area: %1").arg(""));

  resize( 250, 200 );
}

void KSpreadreference::slotHighlighted(QListBoxItem * )
{
  QString tmp = m_list->text(m_list->currentItem());
  QString tmpName;
  QValueList<Reference>::Iterator it;
  QValueList<Reference> area(m_pView->doc()->listArea());
  for ( it = area.begin(); it != area.end(); ++it )
  {
    if ((*it).ref_name == tmp)
    {
      if (!m_pView->doc()->map()->findTable( (*it).table_name))
        kdDebug(36001) << "(*it).table_name '" << (*it).table_name
                       << "' not found!*********" << endl;
      else
        tmpName = util_rangeName(m_pView->doc()->map()->findTable( (*it).table_name),
                                 (*it).rect);
      break;
    }
  }

  tmpName = i18n("Area: %1").arg(tmpName);
  m_rangeName->setText(tmpName);
}

void KSpreadreference::slotDoubleClicked(QListBoxItem *)
{
  slotOk();
}

void KSpreadreference::slotRemove()
{
  if (m_list->currentItem() == -1)
    return;

  int ret = KMessageBox::warningYesNo( this, i18n("Do you really want to remove this area name?"));
  if (ret == 4) // reponse = No
    return;

  QString textRemove;
  if ( m_list->currentItem() != -1)
  {
    QString textRemove = m_list->text(m_list->currentItem());
    m_pView->doc()->removeArea(textRemove );

    /*
      m_list->clear();
      QString text;
      QValueList<Reference>::Iterator it;
      QValueList<Reference> area=m_pView->doc()->listArea();
      for ( it = area.begin(); it != area.end(); ++it )
      {
      text=(*it).ref_name;
      m_list->insertItem(text);
      }
    */

    m_list->removeItem(m_list->currentItem());

    KSpreadTable *tbl;
    
    for ( tbl = m_pView->doc()->map()->firstTable(); tbl != 0L; tbl = m_pView->doc()->map()->nextTable() )
    {
      tbl->refreshRemoveAreaName(textRemove);
    }
  }

  if (!m_list->count())
  {
    m_pOk->setEnabled(false);
    m_pRemove->setEnabled(false);
  }
}

void KSpreadreference::slotOk()
{
  QString text;
  if (m_list->currentItem() != -1)
  {
    int index = m_list->currentItem();
    text = m_list->text(index);
    QValueList<Reference> area = m_pView->doc()->listArea();

    if (m_pView->activeTable()->tableName() != area[ index ].table_name)
    {
      KSpreadTable *table = m_pView->doc()->map()->findTable(area[ index ].table_name);
      if (table)
        table->setActiveTable();
    }

    m_pView->canvasWidget()->gotoLocation(KSpreadPoint( m_pView->activeTable()->tableName()
                                                        + "!"+util_cellName(area[ index ].rect.left(), area[ index ].rect.top()  ), m_pView->doc()->map() ) );
    m_pView->activeTable()->setSelection(area[ index ].rect ,
                                         m_pView->canvasWidget() );
  }
  accept();
}

void KSpreadreference::slotCancel()
{
  reject();
}


#include "kspread_dlg_reference.moc"
