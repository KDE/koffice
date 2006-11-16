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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include <QFrame>
#include <QLayout>
#include <QPushButton>
//Added by qt3to4:
#include <QVBoxLayout>

#include <kbuttonbox.h>
#include <kdebug.h>
#include <klocale.h>
#include <knuminput.h>

#include <Cell.h>
#include <Doc.h>
#include <Sheet.h>
#include <Format.h>
#include <View.h>

#include "AngleDialog.h"

using namespace KSpread;

AngleDialog::AngleDialog(View* parent, const char* name, const QPoint &_marker)
  : KDialog( parent )
{
  setCaption( i18n("Change Angle") );
  setObjectName( name );
  setModal( true );
  setButtons( Ok|Cancel|Default );

  m_pView=parent;
  marker=_marker;

  QWidget *page = new QWidget();
  setMainWidget( page );

  QVBoxLayout *lay = new QVBoxLayout( page );
  lay->setMargin(0);
  lay->setSpacing(spacingHint());
  m_pAngle = new KIntNumInput( page );
  m_pAngle->setRange( -90, 90, 1 );
  m_pAngle->setLabel( i18n("Angle:") );
  m_pAngle->setSuffix(" ");
  lay->addWidget( m_pAngle );

  QWidget* spacer = new QWidget( page );
  spacer->setSizePolicy( QSizePolicy( QSizePolicy::Preferred, QSizePolicy::Expanding ) );
  lay->addWidget( spacer );

  m_pAngle->setFocus();

  connect( this, SIGNAL( okClicked() ), this, SLOT( slotOk() ) );

  Cell *cell = m_pView->activeSheet()->cellAt( marker.x(), marker.y() );
  int angle = - cell->style().angle();
  m_pAngle->setValue( angle );
}

void AngleDialog::slotOk()
{
    m_pView->setSelectionAngle( -m_pAngle->value() );
    accept();
}

void AngleDialog::slotDefault()
{
    m_pAngle->setValue( 0 );
}


#include "AngleDialog.moc"
