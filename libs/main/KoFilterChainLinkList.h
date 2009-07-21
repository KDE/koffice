/* This file is part of the KOffice libraries
   Copyright (C) 2009 Boudewijn Rempt <boud@valdyas.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/
#ifndef KOFILTERCHAINLINKLIST_H
#define KOFILTERCHAINLINKLIST_H

#include <QList>

namespace KOfficeFilter {

    class ChainLink;


    class ChainLinkList
    {
    public:
        ChainLinkList();
        ~ChainLinkList();

        void deleteAll();
        int count() const;

        ChainLink* current() const;

        ChainLink* first();

        ChainLink* next();

        void prepend(ChainLink* link);

        void append(ChainLink* link);

    private:

        QList<ChainLink*> m_chainLinks;
        int m_current;

    };

};

#endif // KOFILTERCHAINLINKLIST_H
