/* This file is part of the KDE project
   Copyright (C) 2006 Stefan Nikolaus <stefan.nikolaus@kdemail.net>
   Copyright (C) 2010 Marijn Kruisselbrink <m.kruisselbrink@student.tue.nl>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
   MA  02110-1301  USA
*/
#ifndef KSPREAD_RTREE_BENCHMARK_H
#define KSPREAD_RTREE_BENCHMARK_H

#include <QObject>
#include "RTree.h"

namespace KSpread
{

class RTreeBenchmark : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void init();
    void cleanup();

    void testInsertionPerformance();
    void testRowInsertionPerformance();
    void testColumnInsertionPerformance();
    void testRowDeletionPerformance();
    void testColumnDeletionPerformance();
    void testLookupPerformance();
private:
    RTree<double> m_tree;
};

} // namepsace KSpread

#endif // KSPREAD_RTREE_BENCHMARK_H
