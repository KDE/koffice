/* This file is part of the KDE project
   Copyright (C) 2003 Robert JACOLIN <rjacolin@ifrance.com>

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
   This file use code from koTemplateOpenDia for the method chooseSlot.
*/

#include <latexexportIface.h>
#include "kwordlatexexportdia.h"

/*
 *  Constructs a KWordLatexExportDia which is a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
LatexExportIface::LatexExportIface(KWordLatexExportDia* dia)
    :	DCOPObject("FilterConfigDia")
{
	_dialog = dia;
}

LatexExportIface::~LatexExportIface()
{
}

void LatexExportIface::useDefaultConfig()
{
	_dialog->accept();
}

