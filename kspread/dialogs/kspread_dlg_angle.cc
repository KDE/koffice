/* This file is part of the KDE project
   Copyright (C) 2003 Norbert Andres<nandres@web.de>
             (C) 2002 Philipp Mueller <philipp.mueller@gmx.de>
             (C) 2002 Ariya Hidayat <ariya@kde.org>
             (C) 1999-2002 Laurent Montel <montel@kde.org>
             (C) 1998-1999 Torben Weis <weis@kde.org> 

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

#include <kspread_dlg_angle.h>
#include <kspread_cell.h>
#include <kspread_view.h>
#include <kspread_doc.h>
#include <kspread_sheet.h>

#include <qlayout.h>
#include <qpushbutton.h>

#include <kbuttonbox.h>
#include <kdebug.h>
#include <klocale.h>
#include <knuminput.h>

KSpreadAngle::KSpreadAngle( KSpreadView* parent, const char* name,const QPoint &_marker)
	: KDialogBase( parent, name,TRUE,i18n("Change Angle" ), Ok|Cancel|Default )
{
  m_pView=parent;
  marker=_marker;

  QWidget *page = new QWidget( this );
  setMainWidget(page);

  QVBoxLayout *lay = new QVBoxLayout( page, 0, spacingHint() );
  m_pAngle = new KIntNumInput( page );
  m_pAngle->setRange( -90, 90, 1 );
  m_pAngle->setLabel( i18n("Angle:") );
  m_pAngle->setSuffix(" �");
  lay->addWidget( m_pAngle );

  QWidget* spacer = new QWidget( page );
  spacer->setSizePolicy( QSizePolicy( QSizePolicy::Preferred, QSizePolicy::Expanding ) );
  lay->addWidget( spacer );

  m_pAngle->setFocus();

  connect( this, SIGNAL( okClicked() ), this, SLOT( slotOk() ) );

  KSpreadCell *cell = m_pView->activeTable()->cellAt( marker.x(), marker.y() );
  int angle=-(cell->getAngle(marker.x(), marker.y()));
  m_pAngle->setValue( angle );
}

void KSpreadAngle::slotOk()
{
    m_pView->doc()->emitBeginOperation( false );
    m_pView->activeTable()->setSelectionAngle(m_pView->selectionInfo(), -m_pAngle->value());
    m_pView->slotUpdateView( m_pView->activeTable() );
    // m_pView->doc()->emitEndOperation();

    accept();
}

void KSpreadAngle::slotDefault()
{
    m_pAngle->setValue( 0 );
}


#include "kspread_dlg_angle.moc"
