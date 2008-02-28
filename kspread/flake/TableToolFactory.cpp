/* This file is part of the KDE project
   Copyright 2006 Stefan Nikolaus <stefan.nikolaus@kdemail.net>

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
#include "TableToolFactory.h"

#include <klocale.h>

#include "TableShape.h"
#include "TableTool.h"

using namespace KSpread;

TableToolFactory::TableToolFactory( QObject* parent )
    : KoToolFactory( parent, "TableToolFactoryId", i18n( "Table tool" ) )
{
    setToolTip( i18n( "Table editing tool" ) );
    setIcon( "spreadsheetshape" );
    setToolType( dynamicToolType() );
    setPriority( 1 );
    setActivationShapeId( TableShapeId );
}

TableToolFactory::~TableToolFactory()
{
}

KoTool* TableToolFactory::createTool( KoCanvasBase* canvas )
{
    return new TableTool( canvas );
}

#include "TableToolFactory.moc"
