/* This file is part of the KDE project
   Copyright 2008 Stefan Nikolaus <stefan.nikolaus@kdemail.net>

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

#ifndef KSPREAD_TABLE_PAGE_MANAGER
#define KSPREAD_TABLE_PAGE_MANAGER

#include "PageManager.h"

namespace KSpread
{
class TableShape;

class TablePageManager : public PageManager
{
public:
    /**
     * Constructor.
     */
    TablePageManager(TableShape* masterShape);

    /**
     * Destructor.
     */
    ~TablePageManager();

    virtual QSizeF size(int page) const;

protected:
    virtual void clearPages();
    virtual void insertPage(int page);
    virtual void preparePage(int page);

private:
    class Private;
    Private * const d;
};

} // namespace KSpread

#endif // KSPREAD_TABLE_PAGE_MANAGER
