/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>
   Copyright (C) 1999 Montel Laurent <montell@club-internet.fr>

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

#ifndef __kspread_dlg_angle__
#define __kspread_dlg_angle__

#include <kdialogbase.h>

class KSpreadView;
class KIntNumInput;

class KSpreadAngle : public KDialogBase
{
  Q_OBJECT
public:
  KSpreadAngle( KSpreadView* parent,const char* name,const QPoint &_marker);

public slots:
  void slotOk();
  void slotDefault();
protected:
  KSpreadView* m_pView;
  QPoint  marker;
  KIntNumInput *m_pAngle;
};

#endif
