/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>
   Copyright (C) 1999,2000,2001 Montel Laurent <lmontel@mandrakesoft.com>

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



#include "kspread_dlg_area.h"
#include "kspread_view.h"
#include "kspread_sheet.h"
#include "kspread_doc.h"

#include <qlayout.h>
#include <qlineedit.h>
#include <kmessagebox.h>

KSpreadarea::KSpreadarea( KSpreadView * parent, const char * name, const QPoint & _marker )
  : KDialogBase( parent, name, TRUE, i18n("Area Name"), Ok | Cancel )
{
  m_pView  = parent;
  m_marker = _marker;

  QWidget * page = new QWidget( this );
  setMainWidget(page);
  QVBoxLayout * lay1 = new QVBoxLayout( page, 0, spacingHint() );

  m_areaName = new QLineEdit(page);
  m_areaName->setMinimumWidth( m_areaName->sizeHint().width() * 3 );

  lay1->addWidget( m_areaName );
  m_areaName->setFocus();
  connect ( m_areaName, SIGNAL(textChanged ( const QString & )), this, SLOT(slotAreaNamechanged( const QString &)));
  connect( this, SIGNAL( okClicked() ), this, SLOT( slotOk() ) );
  enableButtonOK(!m_areaName->text().isEmpty());

}

void KSpreadarea::slotAreaNamechanged( const QString & text)
{
  enableButtonOK(!text.isEmpty());
}

void KSpreadarea::slotOk()
{
  QString tmp(m_areaName->text());
  if( !tmp.isEmpty() )
  {
    tmp = tmp.lower();

    QRect rect( m_pView->selection() );
    bool newName = true;
    QValueList<Reference>::Iterator it;
    QValueList<Reference> area = m_pView->doc()->listArea();
    for ( it = area.begin(); it != area.end(); ++it )
    {
      if(tmp == (*it).ref_name)
        newName = false;
    }
    if (newName)
    {
      m_pView->doc()->addAreaName(rect, tmp, m_pView->activeTable()->tableName());
      accept();
    }
    else
      KMessageBox::error( this, i18n("This name is already used."));
  }
  else
  {
    KMessageBox::error( this, i18n("Area text is empty!") );
  }
}

#include "kspread_dlg_area.moc"
