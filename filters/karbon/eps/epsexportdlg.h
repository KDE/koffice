/* This file is part of the KDE project
   Copyright (C) 2002, The Karbon Developers

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

#ifndef __VEPSEXPORTDLG_H__
#define __VEPSEXPORTDLG_H__

#include <kdialogbase.h>


class QButtonGroup;
class QCheckBox;

class EpsExportDlg : public KDialogBase
{
	Q_OBJECT

public:
	EpsExportDlg( QWidget* parent = 0L, const char* name = 0L );

	uint psLevel() const;
	bool exportHidden() const;
private:
	QButtonGroup* m_psLevelButtons;
	QCheckBox* m_hiddenExport;
};

#endif

