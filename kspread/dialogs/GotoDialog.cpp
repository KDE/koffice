/* This file is part of the KDE project
   Copyright (C) 1999-2003 Laurent Montel <montel@kde.org>
             (C) 2003 Philipp Mueller <philipp.mueller@gmx.de>
             (C) 2003 Ariya Hidayat <ariya@kde.org>
             (C) 2003 Norbert Andres <nandres@web.de>
             (C) 1999 Stephan Kulow <coolo@kde.org>
             (C) 1998-2000 Torben Weis <weis@kde.org>

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
#include "GotoDialog.h"

#include <QLabel>
#include <QLayout>
#include <QVBoxLayout>
#include <QComboBox>

#include <klineedit.h>

#include "Canvas.h"
#include "Doc.h"
#include "Localization.h"
#include "Util.h"
#include "View.h"
#include "Cell.h"
#include "Selection.h"
#include "NamedAreaManager.h"

using namespace KSpread;

GotoDialog::GotoDialog( View* parent, const char* name )
  : KDialog( parent )
{
  setCaption( i18n("Goto Cell") );
  setObjectName( name );
  setModal( true );
  setButtons( Ok|Cancel );

  m_pView = parent;
  QWidget *page = new QWidget();
  setMainWidget( page );
  QVBoxLayout *lay1 = new QVBoxLayout( page );
  lay1->setMargin(KDialog::marginHint());
  lay1->setSpacing(KDialog::spacingHint());

  QLabel *label = new QLabel(i18n("Enter cell:"), page);
  lay1->addWidget(label);

  m_nameCell = new QComboBox( page );
  m_nameCell->setEditable(true);
  lay1->addWidget(m_nameCell);

  const Sheet* sheet = parent->activeSheet();
  Selection* selection = parent->selection();
  if( sheet && selection ) {
    Cell cell(sheet, selection->cursor());
    m_nameCell->addItem( cell.name() );
    m_nameCell->addItem( cell.fullName() );
  }
  Doc *doc = m_pView->doc();
  NamedAreaManager *manager = doc->namedAreaManager();
  m_nameCell->addItems( manager->areaNames() );
  m_nameCell->setFocus();

  connect( this, SIGNAL( okClicked() ), this, SLOT( slotOk() ) );
  connect( m_nameCell, SIGNAL(textChanged ( const QString & )),
           this, SLOT(textChanged ( const QString & )));

  resize( QSize(320,50).expandedTo( minimumSizeHint() ) );
}

void GotoDialog::textChanged ( const QString &_text )
{
    enableButtonOk(!_text.isEmpty());
}

void GotoDialog::slotOk()
{
    m_pView->doc()->emitBeginOperation( false );

    QString tmp_upper = m_nameCell->currentText();
    Region region(tmp_upper, m_pView->doc()->map(), m_pView->activeSheet());
    if ( region.isValid() )
    {
      if ( region.firstSheet() != m_pView->activeSheet() )
          m_pView->setActiveSheet( region.firstSheet() );
      m_pView->selection()->initialize(region);
      accept();
    }
    else
    {
      m_nameCell->setCurrentText("");
    }
    m_pView->slotUpdateView( m_pView->activeSheet() );
}

#include "GotoDialog.moc"
