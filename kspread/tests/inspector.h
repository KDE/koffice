/* This file is part of the KDE project
   Copyright 2005 Ariya Hidayat <ariya@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; only
   version 2 of the License.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KSPREAD_INSPECTOR
#define KSPREAD_INSPECTOR

#include <kpagedialog.h>

namespace KSpread
{
class Cell;

class Inspector : public KPageDialog
{
    Q_OBJECT
public:
    Inspector( const Cell& cell );
    ~Inspector();

private:
    Q_DISABLE_COPY( Inspector )

    class Private;
    Private * const d;
};

} // namespace KSpread

#endif // KSPREAD_INSPECTOR
