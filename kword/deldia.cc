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
#include "kwtableframeset.h"
#include "deldia.h"
#include "deldia.moc"
#include "kwcommand.h"

#include <klocale.h>

#include <qlabel.h>
#include <qspinbox.h>

#include <stdlib.h>

/******************************************************************/
/* Class: KWDeleteDia                                             */
/******************************************************************/

KWDeleteDia::KWDeleteDia( QWidget *parent, const char *name, KWTableFrameSet *_table, KWDocument *_doc, DeleteType _type, KWCanvas *_canvas )
    : KDialogBase( Plain, QString::null, Ok | Cancel, Ok, parent, name, true )
{
    type = _type;
    table = _table;
    doc = _doc;
    canvas = _canvas;
    m_toRemove.clear();

    setupTab1();
    setButtonOKText(i18n("&Delete"), type == ROW ?
    	i18n("Delete the row from the table.") :
    	i18n("Delete the column from the table."));

    setInitialSize( QSize(300, 150) );
}

void KWDeleteDia::setupTab1()
{
    tab1 = plainPage();
    grid1 = new QGridLayout( tab1, 4, 1, 0, spacingHint() );
    QString message =type == ROW ? i18n( "Delete Rows:" ) : i18n( "Delete Columns:" );
    bool firstSelectedCell = true; // used to know whether to add a ", " to the message string.

    uint max = (type == ROW) ? table->getRows() : table->getCols(); // max row/col to loop up to
    for (uint i=0;i<max; i++)
    {
        if ( ( (type == ROW) && table->isRowSelected(i)) ||
                ( (type == COL) && table->isColSelected(i) ) )
        {
            if (!firstSelectedCell)
                message += ", "; // i18n??
            message += QString::number(i +1);
            m_toRemove.push_back(i);
            firstSelectedCell = false;
       }
    }
    Q_ASSERT(m_toRemove.count() > 0);

    if (m_toRemove.count() == ( (type == ROW) ? table->getRows() : table->getCols() ) )
        // all the columns are selected and the user asked to remove columns or the same with rows
        // => we want to delete the whole table
        message = i18n("Delete the whole table");

    // do not display hugely long dialogs if many rows/cells are selected
    if (m_toRemove.count() > 10)
        message = ROW ? i18n("Delete all selected rows") : i18n("Delete all selected cells");

    rc = new QLabel( message , tab1 );
    rc->resize( rc->sizeHint() );
    rc->setAlignment( AlignLeft | AlignBottom );
    grid1->addWidget( rc, 1, 0 );
}

bool KWDeleteDia::doDelete()
{
    KCommand *globalCommand;

    if (m_toRemove.count() == ( (type == ROW) ? table->getRows() : table->getCols() ) )
    {   // we have to delete the whole table
        //globalCommand = new KWDeleteTableCommand(i18n("Remove table"), table);
        doc->deleteTable(table);
    }
    else
    {   // we will just delete some row/cols
        if ( type == ROW )
        {
            globalCommand = new KMacroCommand(i18n("Remove rows"));
            for (uint i=0;i<m_toRemove.count();i++)
            {
                KWRemoveRowCommand *cmd = new KWRemoveRowCommand( i18n("Remove row"), table, m_toRemove[i] );
                static_cast<KMacroCommand*>(globalCommand)->addCommand(cmd);
            }
        }
        else
        {
            globalCommand = new KMacroCommand(i18n("Remove columns"));
            for (uint i=0;i<m_toRemove.count();i++)
            {
                KWRemoveColumnCommand *cmd = new KWRemoveColumnCommand( i18n("Remove column"), table, m_toRemove[i] );
                static_cast<KMacroCommand*>(globalCommand)->addCommand(cmd);
            }
        }
        globalCommand->execute();
        doc->addCommand(globalCommand);
    }

    return true;
}

void KWDeleteDia::slotOk()
{
   if (doDelete())
   {
      KDialogBase::slotOk();
   }
}
