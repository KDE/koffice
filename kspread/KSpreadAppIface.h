/* This file is part of the KDE project
   Copyright
          � 2004 Ariya Hidayat <ariya@kde.org>
          � 2003 David Faure <faure@kde.org>
	  � 2001 Philipp Mueller
	  � 2001 Laurent Montel <montel@kde.org>
	  � 2002 Werner Trobin <trobin@kde.org>
          � 1999 Torben Weis <weis@kde.org>


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
*/#ifndef KSPREAD_APP_IFACE_H
#define KSPREAD_APP_IFACE_H

#include <dcopobject.h>
#include <dcopref.h>

#include <qmap.h>
#include <qstring.h>

class KSpreadAppIface : public DCOPObject
{
    K_DCOP
public:
    KSpreadAppIface();

k_dcop:
    virtual DCOPRef createDoc();
    virtual DCOPRef createDoc( const QString& name );
    virtual QMap<QString,DCOPRef> documents();
    virtual DCOPRef document( const QString& name );
};

#endif
