/* This file is part of the KDE project
   Copyright (C) 2002 Laurent Montel <lmontel@mandrakesoft.com>

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

#ifndef KARBON_PART_IFACE_H
#define KARBON_PART_IFACE_H

#include <KoDocumentIface.h>
#include <dcopref.h>

#include <qstring.h>

class KarbonPart;

class KarbonPartIface : virtual public KoDocumentIface
{
	K_DCOP
public:
	KarbonPartIface( KarbonPart *part_ );
k_dcop:
	void selectAllObjects();
	void deselectAllObjects();

	bool showStatusBar () const;
	void setShowStatusBar (bool b);
	void setUndoRedoLimit( int _undo );
	void initConfig();
	int maxRecentFiles() const;
	void clearHistory();
	QString unitName() const;

    void setBackupFile( bool _b );
    bool backupFile()const;

private:
	KarbonPart *m_part;
};

#endif
