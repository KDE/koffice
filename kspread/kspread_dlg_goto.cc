/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>
   Copyright (C) 1999, 2000,2001  Montel Laurent <lmontel@mandrakesoft.com>
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


#include "kspread_dlg_goto.h"
#include "kspread_canvas.h"
#include "kspread_doc.h"
#include "kspread_util.h"

#include <qlayout.h>


KSpreadGotoDlg::KSpreadGotoDlg( KSpreadView* parent, const char* name )
	: KDialogBase( parent, name, TRUE,i18n("Goto cell"),Ok|Cancel )
{
  m_pView = parent;
  QWidget *page = new QWidget( this );
  setMainWidget(page);
  QVBoxLayout *lay1 = new QVBoxLayout( page, 0, spacingHint() );

  m_nameCell = new QLineEdit( page );
  lay1->addWidget(m_nameCell);

  m_nameCell->setFocus();
  enableButtonOK( false );

  connect( this, SIGNAL( okClicked() ), this, SLOT( slotOk() ) );
  connect( m_nameCell, SIGNAL(textChanged ( const QString & )),
           this, SLOT(textChanged ( const QString & )));
}

void KSpreadGotoDlg::textChanged ( const QString &_text )
{
    enableButtonOK(!_text.isEmpty());
}

void KSpreadGotoDlg::slotOk()
{
    QString tmp_upper;
    tmp_upper=m_nameCell->text().upper();

    if ( tmp_upper.contains( ':' ) ) //Selection entered in location widget
        m_pView->canvasWidget()->gotoLocation( KSpreadRange( tmp_upper, m_pView->doc()->map() ) );
    else //Location entered in location widget
        m_pView->canvasWidget()->gotoLocation( KSpreadPoint( tmp_upper, m_pView->doc()->map() ) );
    accept();
}

#include "kspread_dlg_goto.moc"
