/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Reginald Stadlbauer <reggie@kde.org>

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

#include "kwdoc.h"
#include "kwframe.h"
#include "kwcanvas.h"
#include "insdia.h"
#include "insdia.moc"
#include "kwtableframeset.h"
#include "kwcommand.h"

#include <klocale.h>

#include <qwidget.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qstring.h>
#include <qspinbox.h>
#include <qradiobutton.h>
#include <qbuttongroup.h>


/******************************************************************/
/* Class: KWInsertDia                                             */
/******************************************************************/

KWInsertDia::KWInsertDia( QWidget *parent, const char *name, KWTableFrameSet *_table, KWDocument *_doc, InsertType _type, KWCanvas *_canvas )
    : KDialogBase( Tabbed, QString::null, Ok | Cancel, Ok, parent, name, true )
{
    type = _type;
    table = _table;
    doc = _doc;
    canvas = _canvas;

    setupTab1();

    setInitialSize( QSize(300, 250) );
}

void KWInsertDia::setupTab1()
{
    tab1 = addPage( type == ROW ? i18n( "Insert Row" ) : i18n( "Insert Column" ) );

    grid1 = new QGridLayout( tab1, 3, 1, KDialog::marginHint(), KDialog::spacingHint() );

    QButtonGroup *grp = new QButtonGroup( type == ROW ? i18n( "Insert New Row" ) : i18n( "Insert New Column" ), tab1 );
    grp->setExclusive( true );

    grid2 = new QGridLayout( grp, 3, 1, KDialog::marginHint(), KDialog::spacingHint() );

    rBefore = new QRadioButton( i18n( "Before" ), grp );
    rBefore->resize( rBefore->sizeHint() );
    grp->insert( rBefore );
    grid2->addWidget( rBefore, 1, 0 );

    rAfter = new QRadioButton( i18n( "After" ), grp );
    rAfter->resize( rAfter->sizeHint() );
    grp->insert( rAfter );
    grid2->addWidget( rAfter, 2, 0 );
    rAfter->setChecked( true );

    grid2->addRowSpacing( 0, 7 );
    grid2->addRowSpacing( 1, rBefore->height() );
    grid2->addRowSpacing( 2, rAfter->height() );
    grid2->setRowStretch( 0, 0 );
    grid2->setRowStretch( 1, 0 );
    grid2->setRowStretch( 1, 0 );

    grid2->addColSpacing( 0, rBefore->width() );
    grid2->addColSpacing( 0, rAfter->width() );
    grid2->setColStretch( 0, 1 );

    grid1->addWidget( grp, 0, 0 );

    rc = new QLabel( type == ROW ? i18n( "Row:" ) : i18n( "Column:" ), tab1 );
    rc->resize( rc->sizeHint() );
    rc->setAlignment( AlignLeft | AlignBottom );
    grid1->addWidget( rc, 1, 0 );

    value = new QSpinBox( 1, type == ROW ? table->getRows() : table->getCols(), 1, tab1 );
    value->resize( value->sizeHint() );
    value->setValue( type == ROW ? table->getRows() : table->getCols() );
    grid1->addWidget( value, 2, 0 );

    grid1->addRowSpacing( 0, grp->height() );
    grid1->addRowSpacing( 1, rc->height() );
    grid1->addRowSpacing( 2, value->height() );
    grid1->setRowStretch( 0, 0 );
    grid1->setRowStretch( 1, 1 );
    grid1->setRowStretch( 2, 0 );

    grid1->addColSpacing( 0, grp->width() );
    grid1->addColSpacing( 0, rc->width() );
    grid1->addColSpacing( 0, value->width() );
    grid1->setColStretch( 0, 1 );
}

bool KWInsertDia::doInsert()
{
    unsigned int insert= value->value() - ( rBefore->isChecked() ? 1 : 0 );
    if ( type == ROW )
    {
         KWInsertRowCommand *cmd = new KWInsertRowCommand( i18n("Insert row"), table, insert);
        //table->insertCol( value->value() - ( rBefore->isChecked() ? 1 : 0 ) );
         cmd->execute();
         doc->addCommand(cmd);
    }
    else
    {
        KWInsertColumnCommand *cmd = new KWInsertColumnCommand( i18n("Insert column"), table, insert);
        //table->insertCol( value->value() - ( rBefore->isChecked() ? 1 : 0 ) );
        cmd->execute();
        doc->addCommand(cmd);
    }

    return true;
}

void KWInsertDia::slotOk()
{
   if (doInsert())
   {
      KDialogBase::slotOk();
   }
}
