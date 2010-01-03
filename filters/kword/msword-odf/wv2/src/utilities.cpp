/* This file is part of the wvWare 2 project
   Copyright (C) 2002-2003 Werner Trobin <trobin@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the Library GNU General Public
   version 2 of the License, or (at your option) version 3 or,
   at the discretion of KDE e.V (which shall act as a proxy as in
   section 14 of the GPLv3), any later version..

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "utilities.h"
#include <stdio.h>

namespace wvWare
{

    std::string int2string( int i )
    {
        char buf[ 40 ];
        snprintf( buf, 40, "%d", i );
        return std::string( buf );
    }

    std::string uint2string( unsigned int i )
    {
        char buf[ 40 ];
        snprintf( buf, 40, "%u", i );
        return std::string( buf );
    }
} // namespace wvWare
