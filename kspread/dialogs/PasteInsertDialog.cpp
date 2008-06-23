/* This file is part of the KDE project
   Copyright (C) 2003 Norbert Andres <nandres@web.de>
             (C) 2002 Philipp Mueller <philipp.mueller@gmx.de>
             (C) 2002 John Dailey <dailey@vt.edu>
             (C) 2000-2002 Laurent Montel <montel@kde.org>

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
   Boston, MA 02110-1301, USA.
*/

// Local
#include "PasteInsertDialog.h"

#include <q3buttongroup.h>

#include <QVBoxLayout>
#include <klocale.h>

#include <kdebug.h>
#include <QRadioButton>
#include <QCheckBox>

#include "Selection.h"
#include "Sheet.h"

using namespace KSpread;

PasteInsertDialog::PasteInsertDialog(QWidget* parent, Selection* selection)
  : KDialog( parent )
{
  setCaption( i18n("Paste Inserting Cells") );
  setObjectName("PasteInsertDialog");
  setModal( true );
  setButtons( Ok|Cancel );
  m_selection = selection;
  rect = selection->lastRange();

  QWidget *page = new QWidget();
  setMainWidget( page );
  QVBoxLayout *lay1 = new QVBoxLayout( page );
  lay1->setMargin(KDialog::marginHint());
  lay1->setSpacing(KDialog::spacingHint());

  Q3ButtonGroup *grp = new Q3ButtonGroup( 1, Qt::Horizontal, i18n("Insert"),page);
  grp->setRadioButtonExclusive( true );
  grp->layout();
  lay1->addWidget(grp);
  rb1 = new QRadioButton( i18n("Move towards right"), grp );
  rb2 = new QRadioButton( i18n("Move towards bottom"), grp );
  rb1->setChecked(true);

  connect( this, SIGNAL( okClicked() ), this, SLOT( slotOk() ) );
}

void PasteInsertDialog::slotOk()
{
    if( rb1->isChecked() )
      m_selection->activeSheet()->paste( m_selection->lastRange(),
                                     true, Paste::Normal, Paste::OverWrite,
                                     true, -1 );
    else if( rb2->isChecked() )
      m_selection->activeSheet()->paste( m_selection->lastRange(),
                                     true, Paste::Normal, Paste::OverWrite,
                                     true, +1 );

    accept();
}

#include "PasteInsertDialog.moc"
