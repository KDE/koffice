/* This file is part of the KDE project
   Copyright (C) 2004 Ariya Hidayat <ariya@kde.org>

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

#include "commands.h"

#include <kspread_doc.h>
#include <kspread_map.h>
#include <kspread_sheet.h>
#include <kspread_undo.h>

// ----- UndoWrapperCommand -----

UndoWrapperCommand::UndoWrapperCommand( KSpreadUndoAction* ua )
{
  undoAction = ua;
}

void UndoWrapperCommand::execute()
{
  // This is not really safe and functional, but UndoWrapperCommand
  // is a workaround anyway. So leave it as it it (Ariya)
  undoAction->redo();
}

void UndoWrapperCommand::unexecute()
{
  undoAction->undo();
}

QString UndoWrapperCommand::name() const
{
  return undoAction->getName();
}

// ----- RenameSheetCommand -----

RenameSheetCommand::RenameSheetCommand( KSpreadSheet* s, QString name )
{
  sheet = s;
  if( s ) oldName = s->tableName();
  newName = name;
}

QString RenameSheetCommand::name() const
{
  return i18n("Rename Sheet");
}

void RenameSheetCommand::execute()
{
  if( sheet )
    sheet->setTableName( newName );
}

void RenameSheetCommand::unexecute()
{
  if( sheet )
    sheet->setTableName( oldName );
}

// ----- HideSheetCommand -----

HideSheetCommand::HideSheetCommand( KSpreadSheet* sheet )
{
  doc = sheet->doc();
  sheetName = sheet->tableName();
}

void HideSheetCommand::execute()
{
  KSpreadSheet* sheet = doc->map()->findTable( sheetName );
  if( !sheet ) return;

  sheet->hideTable( true );
}

void HideSheetCommand::unexecute()
{
  KSpreadSheet* sheet = doc->map()->findTable( sheetName );
  if( !sheet ) return;

  sheet->hideTable( false );
}

QString HideSheetCommand::name() const
{
    QString n = QString( i18n("Hide Sheet %1") ).arg( sheetName );
    if( n.length() > 64 ) n = i18n("Hide Sheet");
    return n;
}

// ----- ShowSheetCommand -----

ShowSheetCommand::ShowSheetCommand( KSpreadSheet* sheet )
{
  doc = sheet->doc();
  sheetName = sheet->tableName();
}

void ShowSheetCommand::execute()
{
  KSpreadSheet* sheet = doc->map()->findTable( sheetName );
  if( !sheet ) return;

  sheet->hideTable( false );
}

void ShowSheetCommand::unexecute()
{
  KSpreadSheet* sheet = doc->map()->findTable( sheetName );
  if( !sheet ) return;

  sheet->hideTable( true );
}

QString ShowSheetCommand::name() const
{
    QString n = QString( i18n("Show Sheet %1") ).arg( sheetName );
    if( n.length() > 64 ) n = i18n("Show Sheet");
    return n;
}
