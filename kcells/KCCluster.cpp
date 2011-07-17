/* This file is part of the KDE project
   Copyright (C) 2005 Tomas Mecir <mecirt@gmail.com>
   Copyright (C) 2000 Torben Weis <weis@kde.org>

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

// Local
#include "KCCluster.h"

#include <stdlib.h>

#include <kdebug.h>

#include "KCCell.h"
#include "RowColumnFormat.h"

#if 0
/****************************************************
 *
 * KCCluster
 *
 ****************************************************/

/* Generate a matrix LEVEL1 with the size LEVEL1*LEVEL1 */
KCCluster::KCCluster()
        : m_first(0), m_autoDelete(false), m_biggestX(0), m_biggestY(0)
{
    m_cluster = (KCCell***)malloc(KCELLS_CLUSTER_LEVEL1 * KCELLS_CLUSTER_LEVEL1 * sizeof(KCCell**));

    for (int x = 0; x < KCELLS_CLUSTER_LEVEL1; ++x)
        for (int y = 0; y < KCELLS_CLUSTER_LEVEL1; ++y)
            m_cluster[ y * KCELLS_CLUSTER_LEVEL1 + x ] = 0;
}

/* Delete the matrix LEVEL1 and all existing LEVEL2 matrizes */
KCCluster::~KCCluster()
{
// Can't we use clear(), to remove double code - Philipp?
    for (int x = 0; x < KCELLS_CLUSTER_LEVEL1; ++x)
        for (int y = 0; y < KCELLS_CLUSTER_LEVEL1; ++y) {
            KCCell** cl = m_cluster[ y * KCELLS_CLUSTER_LEVEL1 + x ];
            if (cl) {
                free(cl);
                m_cluster[ y * KCELLS_CLUSTER_LEVEL1 + x ] = 0;
            }
        }

    if (m_autoDelete) {
        KCCell* cell = m_first;
        while (cell) {
            KCCell* n = cell->nextCell();
            delete cell;
            cell = n;
        }
    }

    free(m_cluster);
}

void KCCluster::clear()
{
    for (int x = 0; x < KCELLS_CLUSTER_LEVEL1; ++x)
        for (int y = 0; y < KCELLS_CLUSTER_LEVEL1; ++y) {
            KCCell** cl = m_cluster[ y * KCELLS_CLUSTER_LEVEL1 + x ];
            if (cl) {
                free(cl);
                m_cluster[ y * KCELLS_CLUSTER_LEVEL1 + x ] = 0;
            }
        }

    if (m_autoDelete) {
        KCCell* cell = m_first;
        while (cell) {
            KCCell* n = cell->nextCell();
            delete cell;
            cell = n;
        }
    }

    m_first = 0;
    m_biggestX = m_biggestY = 0;
}

KCCell* KCCluster::lookup(int x, int y) const
{
    if (x >= KCELLS_CLUSTER_MAX || x < 0 || y >= KCELLS_CLUSTER_MAX || y < 0) {
        kDebug(36001) << "KCCluster::lookup: invalid column or row value (col:"
        << x << "  | row: " << y << ")" << endl;
        return 0;
    }
    int cx = x / KCELLS_CLUSTER_LEVEL2;
    int cy = y / KCELLS_CLUSTER_LEVEL2;
    int dx = x % KCELLS_CLUSTER_LEVEL2;
    int dy = y % KCELLS_CLUSTER_LEVEL2;

    KCCell** cl = m_cluster[ cy * KCELLS_CLUSTER_LEVEL1 + cx ];
    if (!cl)
        return 0;

    return cl[ dy * KCELLS_CLUSTER_LEVEL2 + dx ];
}

/* Paste a cell in LEVEL2 (it's more paste than insert) */
void KCCluster::insert(KCCell* cell, int x, int y)
{
    if (x >= KCELLS_CLUSTER_MAX || x < 0 || y >= KCELLS_CLUSTER_MAX || y < 0) {
        kDebug(36001) << "KCCluster::insert: invalid column or row value (col:"
        << x << "  | row: " << y << ")" << endl;
        return;
    }

    int cx = x / KCELLS_CLUSTER_LEVEL2;
    int cy = y / KCELLS_CLUSTER_LEVEL2;
    int dx = x % KCELLS_CLUSTER_LEVEL2;
    int dy = y % KCELLS_CLUSTER_LEVEL2;

    KCCell** cl = m_cluster[ cy * KCELLS_CLUSTER_LEVEL1 + cx ];
    if (!cl) {
        cl = (KCCell**)malloc(KCELLS_CLUSTER_LEVEL2 * KCELLS_CLUSTER_LEVEL2 * sizeof(KCCell*));
        m_cluster[ cy * KCELLS_CLUSTER_LEVEL1 + cx ] = cl;

        for (int a = 0; a < KCELLS_CLUSTER_LEVEL2; ++a)
            for (int b = 0; b < KCELLS_CLUSTER_LEVEL2; ++b)
                cl[ b * KCELLS_CLUSTER_LEVEL2 + a ] = 0;
    }

    if (cl[ dy * KCELLS_CLUSTER_LEVEL2 + dx ])
        remove(x, y);

    cl[ dy * KCELLS_CLUSTER_LEVEL2 + dx ] = cell;

    if (m_first) {
        cell->setNextCell(m_first);
        m_first->setPreviousCell(cell);
    }
    m_first = cell;

    if (x > m_biggestX) m_biggestX = x;
    if (y > m_biggestY) m_biggestY = y;
}

/* Removes the cell of a matrix, the matrix itself keeps unchanged */
void KCCluster::remove(int x, int y)
{
    if (x >= KCELLS_CLUSTER_MAX || x < 0 || y >= KCELLS_CLUSTER_MAX || y < 0) {
        kDebug(36001) << "KCCluster::remove: invalid column or row value (col:"
        << x << "  | row: " << y << ")" << endl;
        return;
    }

    int cx = x / KCELLS_CLUSTER_LEVEL2;
    int cy = y / KCELLS_CLUSTER_LEVEL2;
    int dx = x % KCELLS_CLUSTER_LEVEL2;
    int dy = y % KCELLS_CLUSTER_LEVEL2;

    KCCell** cl = m_cluster[ cy * KCELLS_CLUSTER_LEVEL1 + cx ];
    if (!cl)
        return;

    KCCell* c = cl[ dy * KCELLS_CLUSTER_LEVEL2 + dx ];
    if (!c)
        return;

    cl[ dy * KCELLS_CLUSTER_LEVEL2 + dx ] = 0;

    if (m_autoDelete) {
        if (m_first == c)
            m_first = c->nextCell();
        if (c->doesMergeCells()) {
            c->mergeCells(c->column(), c->row(), 0, 0);
        }
        delete c;
    } else {
        if (m_first == c)
            m_first = c->nextCell();
        if (c->previousCell())
            c->previousCell()->setNextCell(c->nextCell());
        if (c->nextCell())
            c->nextCell()->setPreviousCell(c->previousCell());
        c->setNextCell(0);
        c->setPreviousCell(0);
    }
}

bool KCCluster::insertShiftRight(const QPoint& marker)
{
    bool dummy;
    return insertShiftRight(marker, dummy);
}

bool KCCluster::insertShiftDown(const QPoint& marker)
{
    bool dummy;
    return insertShiftDown(marker, dummy);
}

void KCCluster::removeShiftUp(const QPoint& marker)
{
    bool dummy;
    removeShiftUp(marker, dummy);
}

void KCCluster::removeShiftLeft(const QPoint& marker)
{
    bool dummy;
    removeShiftLeft(marker, dummy);
}

void KCCluster::setAutoDelete(bool b)
{
    m_autoDelete = b;
}

bool KCCluster::autoDelete() const
{
    return m_autoDelete;
}

KCCell* KCCluster::firstCell() const
{
    return m_first;
}

bool KCCluster::insertShiftRight(const QPoint& marker, bool& work)
{
    work = false;

    if (marker.x() >= KCELLS_CLUSTER_MAX || marker.x() < 0 ||
            marker.y() >= KCELLS_CLUSTER_MAX || marker.y() < 0) {
        kDebug(36001) << "KCCluster::insertShiftRight: invalid column or row value (col:"
        << marker.x() << "  | row: " << marker.y() << ")" << endl;
        return false;
    }

    int cx = marker.x() / KCELLS_CLUSTER_LEVEL2;
    int cy = marker.y() / KCELLS_CLUSTER_LEVEL2;
    int dx = marker.x() % KCELLS_CLUSTER_LEVEL2;
    int dy = marker.y() % KCELLS_CLUSTER_LEVEL2;

    // Is there a cell at the bottom most position ?
    // In this case the shift is impossible.
    KCCell** cl = m_cluster[ cy * KCELLS_CLUSTER_LEVEL1 + KCELLS_CLUSTER_LEVEL1 - 1 ];
    if (cl && cl[ dy * KCELLS_CLUSTER_LEVEL2 + KCELLS_CLUSTER_LEVEL2 - 1 ])
        return false;

    bool a = autoDelete();
    setAutoDelete(false);

    // Move cells in this row one down.
    for (int i = KCELLS_CLUSTER_LEVEL1 - 1; i >= cx ; --i) {
        KCCell** cl = m_cluster[ cy * KCELLS_CLUSTER_LEVEL1 + i ];
        if (cl) {
            work = true;
            int left = 0;
            if (i == cx)
                left = dx;
            int right = KCELLS_CLUSTER_LEVEL2 - 1;
            if (i == KCELLS_CLUSTER_LEVEL1 - 1)
                right = KCELLS_CLUSTER_LEVEL2 - 2;
            for (int k = right; k >= left; --k) {
                KCCell* c = cl[ dy * KCELLS_CLUSTER_LEVEL2 + k ];
                if (c) {
                    remove(c->column(), c->row());
                    c->move(c->column() + 1, c->row());
                    insert(c, c->column(), c->row());
                }
            }
        }
    }

    setAutoDelete(a);

    return true;
}

bool KCCluster::insertShiftDown(const QPoint& marker, bool& work)
{
    work = false;

    if (marker.x() >= KCELLS_CLUSTER_MAX || marker.x() < 0 ||
            marker.y() >= KCELLS_CLUSTER_MAX || marker.y() < 0) {
        kDebug(36001) << "KCCluster::insertShiftDown: invalid column or row value (col:"
        << marker.x() << "  | row: " << marker.y() << ")" << endl;
        return false;
    }

    int cx = marker.x() / KCELLS_CLUSTER_LEVEL2;
    int cy = marker.y() / KCELLS_CLUSTER_LEVEL2;
    int dx = marker.x() % KCELLS_CLUSTER_LEVEL2;
    int dy = marker.y() % KCELLS_CLUSTER_LEVEL2;

    // Is there a cell at the right most position ?
    // In this case the shift is impossible.
    KCCell** cl = m_cluster[ KCELLS_CLUSTER_LEVEL1 * (KCELLS_CLUSTER_LEVEL1 - 1) + cx ];
    if (cl && cl[ KCELLS_CLUSTER_LEVEL2 *(KCELLS_CLUSTER_LEVEL2 - 1) + dx ])
        return false;

    bool a = autoDelete();
    setAutoDelete(false);

    // Move cells in this column one right.
    for (int i = KCELLS_CLUSTER_LEVEL1 - 1; i >= cy ; --i) {
        KCCell** cl = m_cluster[ i * KCELLS_CLUSTER_LEVEL1 + cx ];
        if (cl) {
            work = true;

            int top = 0;
            if (i == cy)
                top = dy;
            int bottom = KCELLS_CLUSTER_LEVEL2 - 1;
            if (i == KCELLS_CLUSTER_LEVEL1 - 1)
                bottom = KCELLS_CLUSTER_LEVEL2 - 2;
            for (int k = bottom; k >= top; --k) {
                KCCell* c = cl[ k * KCELLS_CLUSTER_LEVEL2 + dx ];
                if (c) {
                    remove(c->column(), c->row());
                    c->move(c->column(), c->row() + 1);
                    insert(c, c->column(), c->row());
                }
            }
        }
    }

    setAutoDelete(a);

    return true;
}

bool KCCluster::insertColumn(int col)
{
    if (col >= KCELLS_CLUSTER_MAX || col < 0) {
        kDebug(36001) << "KCCluster::insertColumn: invalid column value (col:"
        << col << ")" << endl;
        return false;
    }

    // Is there a cell at the right most position ?
    // In this case the shift is impossible.
    for (int t1 = 0; t1 < KCELLS_CLUSTER_LEVEL1; ++t1) {
        KCCell** cl = m_cluster[ t1 * KCELLS_CLUSTER_LEVEL1 + KCELLS_CLUSTER_LEVEL1 - 1 ];
        if (cl)
            for (int t2 = 0; t2 < KCELLS_CLUSTER_LEVEL2; ++t2)
                if (cl[ t2 * KCELLS_CLUSTER_LEVEL2 + KCELLS_CLUSTER_LEVEL2 - 1 ])
                    return false;
    }

    for (int t1 = 0; t1 < KCELLS_CLUSTER_LEVEL1; ++t1) {
        bool work = true;
        for (int t2 = 0; work && t2 < KCELLS_CLUSTER_LEVEL2; ++t2)
            insertShiftRight(QPoint(col, t1 * KCELLS_CLUSTER_LEVEL2 + t2), work);
    }

    return true;
}

bool KCCluster::insertRow(int row)
{
    if (row >= KCELLS_CLUSTER_MAX || row < 0) {
        kDebug(36001) << "KCCluster::insertRow: invalid row value (row:"
        << row << ")" << endl;
        return false;
    }

    // Is there a cell at the bottom most position ?
    // In this case the shift is impossible.
    for (int t1 = 0; t1 < KCELLS_CLUSTER_LEVEL1; ++t1) {
        KCCell** cl = m_cluster[ KCELLS_CLUSTER_LEVEL1 * (KCELLS_CLUSTER_LEVEL1 - 1) + t1 ];
        if (cl)
            for (int t2 = 0; t2 < KCELLS_CLUSTER_LEVEL2; ++t2)
                if (cl[ KCELLS_CLUSTER_LEVEL2 *(KCELLS_CLUSTER_LEVEL2 - 1) + t2 ])
                    return false;
    }

    for (int t1 = 0; t1 < KCELLS_CLUSTER_LEVEL1; ++t1) {
        bool work = true;
        for (int t2 = 0; work && t2 < KCELLS_CLUSTER_LEVEL2; ++t2)
            insertShiftDown(QPoint(t1 * KCELLS_CLUSTER_LEVEL2 + t2, row), work);
    }

    return true;
}

void KCCluster::removeShiftUp(const QPoint& marker, bool& work)
{
    work = false;

    if (marker.x() >= KCELLS_CLUSTER_MAX || marker.x() < 0 ||
            marker.y() >= KCELLS_CLUSTER_MAX || marker.y() < 0) {
        kDebug(36001) << "KCCluster::removeShiftUp: invalid column or row value (col:"
        << marker.x() << "  | row: " << marker.y() << ")" << endl;
        return;
    }

    int cx = marker.x() / KCELLS_CLUSTER_LEVEL2;
    int cy = marker.y() / KCELLS_CLUSTER_LEVEL2;
    int dx = marker.x() % KCELLS_CLUSTER_LEVEL2;
    int dy = marker.y() % KCELLS_CLUSTER_LEVEL2;

    bool a = autoDelete();
    setAutoDelete(false);

    // Move cells in this column one column to the left.
    for (int i = cy; i < KCELLS_CLUSTER_LEVEL1; ++i) {
        KCCell** cl = m_cluster[ i * KCELLS_CLUSTER_LEVEL1 + cx ];
        if (cl) {
            work = true;

            int top = 0;
            if (i == cy)
                top = dy + 1;
            int bottom = KCELLS_CLUSTER_LEVEL2 - 1;
            for (int k = top; k <= bottom; ++k) {
                KCCell* c = cl[ k * KCELLS_CLUSTER_LEVEL2 + dx ];
                if (c) {
                    remove(c->column(), c->row());
                    c->move(c->column(), c->row() - 1);
                    insert(c, c->column(), c->row());
                }
            }
        }
    }

    setAutoDelete(a);
}

void KCCluster::removeShiftLeft(const QPoint& marker, bool& work)
{
    work = false;

    if (marker.x() >= KCELLS_CLUSTER_MAX || marker.x() < 0 ||
            marker.y() >= KCELLS_CLUSTER_MAX || marker.y() < 0) {
        kDebug(36001) << "KCCluster::removeShiftLeft: invalid column or row value (col:"
        << marker.x() << "  | row: " << marker.y() << ")" << endl;
        return;
    }

    int cx = marker.x() / KCELLS_CLUSTER_LEVEL2;
    int cy = marker.y() / KCELLS_CLUSTER_LEVEL2;
    int dx = marker.x() % KCELLS_CLUSTER_LEVEL2;
    int dy = marker.y() % KCELLS_CLUSTER_LEVEL2;

    bool a = autoDelete();
    setAutoDelete(false);

    // Move cells in this row one row up.
    for (int i = cx; i < KCELLS_CLUSTER_LEVEL1; ++i) {
        KCCell** cl = m_cluster[ cy * KCELLS_CLUSTER_LEVEL1 + i ];
        if (cl) {
            work = true;

            int left = 0;
            if (i == cx)
                left = dx + 1;
            int right = KCELLS_CLUSTER_LEVEL2 - 1;
            for (int k = left; k <= right; ++k) {
                KCCell* c = cl[ dy * KCELLS_CLUSTER_LEVEL2 + k ];
                if (c) {
                    remove(c->column(), c->row());
                    c->move(c->column() - 1, c->row());
                    insert(c, c->column(), c->row());
                }
            }
        }
    }

    setAutoDelete(a);
}

void KCCluster::removeColumn(int col)
{
    if (col >= KCELLS_CLUSTER_MAX || col < 0) {
        kDebug(36001) << "KCCluster::removeColumn: invalid column value (col:"
        << col << ")" << endl;
        return;
    }

    int cx = col / KCELLS_CLUSTER_LEVEL2;
    int dx = col % KCELLS_CLUSTER_LEVEL2;

    for (int y1 = 0; y1 < KCELLS_CLUSTER_LEVEL1; ++y1) {
        KCCell** cl = m_cluster[ y1 * KCELLS_CLUSTER_LEVEL1 + cx ];
        if (cl)
            for (int y2 = 0; y2 < KCELLS_CLUSTER_LEVEL2; ++y2)
                if (cl[ y2 * KCELLS_CLUSTER_LEVEL2 + dx ])
                    remove(col, y1 * KCELLS_CLUSTER_LEVEL1 + y2);
    }

    for (int t1 = 0; t1 < KCELLS_CLUSTER_LEVEL1; ++t1) {
        bool work = true;
        for (int t2 = 0; work && t2 < KCELLS_CLUSTER_LEVEL2; ++t2)
            removeShiftLeft(QPoint(col, t1 * KCELLS_CLUSTER_LEVEL2 + t2), work);
    }
}

void KCCluster::removeRow(int row)
{
    if (row >= KCELLS_CLUSTER_MAX || row < 0) {
        kDebug(36001) << "KCCluster::removeRow: invalid row value (row:"
        << row << ")" << endl;
        return;
    }

    int cy = row / KCELLS_CLUSTER_LEVEL2;
    int dy = row % KCELLS_CLUSTER_LEVEL2;

    for (int x1 = 0; x1 < KCELLS_CLUSTER_LEVEL1; ++x1) {
        KCCell** cl = m_cluster[ cy * KCELLS_CLUSTER_LEVEL1 + x1 ];
        if (cl)
            for (int x2 = 0; x2 < KCELLS_CLUSTER_LEVEL2; ++x2)
                if (cl[ dy * KCELLS_CLUSTER_LEVEL2 + x2 ])
                    remove(x1 * KCELLS_CLUSTER_LEVEL2 + x2, row);
    }

    for (int t1 = 0; t1 < KCELLS_CLUSTER_LEVEL1; ++t1) {
        bool work = true;
        for (int t2 = 0; work && t2 < KCELLS_CLUSTER_LEVEL2; ++t2)
            removeShiftUp(QPoint(t1 * KCELLS_CLUSTER_LEVEL2 + t2, row), work);
    }
}

void KCCluster::clearColumn(int col)
{
    if (col >= KCELLS_CLUSTER_MAX || col < 0) {
        kDebug(36001) << "KCCluster::clearColumn: invalid column value (col:"
        << col << ")" << endl;
        return;
    }

    int cx = col / KCELLS_CLUSTER_LEVEL2;
    int dx = col % KCELLS_CLUSTER_LEVEL2;

    for (int cy = 0; cy < KCELLS_CLUSTER_LEVEL1; ++cy) {
        KCCell** cl = m_cluster[ cy * KCELLS_CLUSTER_LEVEL1 + cx ];
        if (cl)
            for (int dy = 0; dy < KCELLS_CLUSTER_LEVEL2; ++dy)
                if (cl[ dy * KCELLS_CLUSTER_LEVEL2 + dx ]) {
                    int row = cy * KCELLS_CLUSTER_LEVEL2 + dy ;
                    remove(col, row);
                }
    }
}

void KCCluster::clearRow(int row)
{
    if (row >= KCELLS_CLUSTER_MAX || row < 0) {
        kDebug(36001) << "KCCluster::clearRow: invalid row value (row:"
        << row << ")" << endl;
        return;
    }

    int cy = row / KCELLS_CLUSTER_LEVEL2;
    int dy = row % KCELLS_CLUSTER_LEVEL2;

    for (int cx = 0; cx < KCELLS_CLUSTER_LEVEL1; ++cx) {
        KCCell** cl = m_cluster[ cy * KCELLS_CLUSTER_LEVEL2 + cx ];
        if (cl)
            for (int dx = 0; dx < KCELLS_CLUSTER_LEVEL2; ++dx)
                if (cl[ dy * KCELLS_CLUSTER_LEVEL2 + dx ]) {
                    int column = cx * KCELLS_CLUSTER_LEVEL2 + dx ;
                    remove(column, row);
                }
    }
}

KCValue KCCluster::valueRange(int col1, int row1,
                          int col2, int row2) const
{
    KCValue empty;

    //swap first/second values if needed
    if (col1 > col2) {
        int p = col1; col1 = col2; col2 = p;
    }
    if (row1 > row2) {
        int p = row1; row1 = col2; row2 = p;
    }
    if ((row1 < 0) || (col1 < 0) || (row2 > KCELLS_CLUSTER_MAX) ||
            (col2 > KCELLS_CLUSTER_MAX))
        return empty;

    // if we are out of range occupied by cells, we return an empty
    // array of the requested size
    if ((row1 > m_biggestY) || (col1 > m_biggestX))
        return KCValue(KCValue::Array);

    return makeArray(col1, row1, col2, row2);
}

KCValue KCCluster::makeArray(int col1, int row1, int col2, int row2) const
{
    // this generates an array of values
    KCValue array(KCValue::Array);
    for (int row = row1; row <= row2; ++row) {
        for (KCCell* cell = getFirstCellRow(row); cell; cell = getNextCellRight(cell->column(), row)) {
            if (cell->column() >= col1 && cell->column() <= col2)
                array.setElement(cell->column() - col1, row - row1, cell->value());
        }
    }
    //return the result
    return array;
}

KCCell* KCCluster::getFirstCellColumn(int col) const
{
    KCCell* cell = lookup(col, 1);

    if (cell == 0) {
        cell = getNextCellDown(col, 1);
    }
    return cell;
}

KCCell* KCCluster::getLastCellColumn(int col) const
{
    KCCell* cell = lookup(col, KS_rowMax);

    if (cell == 0) {
        cell = getNextCellUp(col, KS_rowMax);
    }
    return cell;
}

KCCell* KCCluster::getFirstCellRow(int row) const
{
    KCCell* cell = lookup(1, row);

    if (cell == 0) {
        cell = getNextCellRight(1, row);
    }
    return cell;
}

KCCell* KCCluster::getLastCellRow(int row) const
{
    KCCell* cell = lookup(KS_colMax, row);

    if (cell == 0) {
        cell = getNextCellLeft(KS_colMax, row);
    }
    return cell;
}

KCCell* KCCluster::getNextCellUp(int col, int row) const
{
    int cx = col / KCELLS_CLUSTER_LEVEL2;
    int cy = (row - 1) / KCELLS_CLUSTER_LEVEL2;
    int dx = col % KCELLS_CLUSTER_LEVEL2;
    int dy = (row - 1) % KCELLS_CLUSTER_LEVEL2;

    while (cy >= 0) {
        if (m_cluster[ cy * KCELLS_CLUSTER_LEVEL1 + cx ] != 0) {
            while (dy >= 0) {

                if (m_cluster[ cy*KCELLS_CLUSTER_LEVEL1 + cx]
                        [ dy*KCELLS_CLUSTER_LEVEL2 + dx] != 0) {
                    return m_cluster[ cy*KCELLS_CLUSTER_LEVEL1 + cx ]
                           [ dy*KCELLS_CLUSTER_LEVEL2 + dx];
                }
                dy--;
            }
        }
        cy--;
        dy = KCELLS_CLUSTER_LEVEL2 - 1;
    }
    return 0;
}

KCCell* KCCluster::getNextCellDown(int col, int row) const
{
    int cx = col / KCELLS_CLUSTER_LEVEL2;
    int cy = (row + 1) / KCELLS_CLUSTER_LEVEL2;
    int dx = col % KCELLS_CLUSTER_LEVEL2;
    int dy = (row + 1) % KCELLS_CLUSTER_LEVEL2;

    while (cy < KCELLS_CLUSTER_LEVEL1) {
        if (m_cluster[ cy * KCELLS_CLUSTER_LEVEL1 + cx ] != 0) {
            while (dy < KCELLS_CLUSTER_LEVEL2) {

                if (m_cluster[ cy*KCELLS_CLUSTER_LEVEL1 + cx]
                        [ dy*KCELLS_CLUSTER_LEVEL2 + dx] != 0) {
                    return m_cluster[ cy*KCELLS_CLUSTER_LEVEL1 + cx ]
                           [ dy*KCELLS_CLUSTER_LEVEL2 + dx];
                }
                dy++;
            }
        }
        cy++;
        dy = 0;
    }
    return 0;
}

KCCell* KCCluster::getNextCellLeft(int col, int row) const
{
    int cx = (col - 1) / KCELLS_CLUSTER_LEVEL2;
    int cy = row / KCELLS_CLUSTER_LEVEL2;
    int dx = (col - 1) % KCELLS_CLUSTER_LEVEL2;
    int dy = row % KCELLS_CLUSTER_LEVEL2;

    while (cx >= 0) {
        if (m_cluster[ cy * KCELLS_CLUSTER_LEVEL1 + cx ] != 0) {
            while (dx >= 0) {

                if (m_cluster[ cy*KCELLS_CLUSTER_LEVEL1 + cx]
                        [ dy*KCELLS_CLUSTER_LEVEL2 + dx] != 0) {
                    return m_cluster[ cy*KCELLS_CLUSTER_LEVEL1 + cx ]
                           [ dy*KCELLS_CLUSTER_LEVEL2 + dx];
                }
                dx--;
            }
        }
        cx--;
        dx = KCELLS_CLUSTER_LEVEL2 - 1;
    }
    return 0;
}

KCCell* KCCluster::getNextCellRight(int col, int row) const
{
    int cx = (col + 1) / KCELLS_CLUSTER_LEVEL2;
    int cy = row / KCELLS_CLUSTER_LEVEL2;
    int dx = (col + 1) % KCELLS_CLUSTER_LEVEL2;
    int dy = row % KCELLS_CLUSTER_LEVEL2;

    while (cx < KCELLS_CLUSTER_LEVEL1) {
        if (m_cluster[ cy * KCELLS_CLUSTER_LEVEL1 + cx ] != 0) {
            while (dx < KCELLS_CLUSTER_LEVEL2) {

                if (m_cluster[ cy*KCELLS_CLUSTER_LEVEL1 + cx]
                        [ dy*KCELLS_CLUSTER_LEVEL2 + dx] != 0) {
                    return m_cluster[ cy*KCELLS_CLUSTER_LEVEL1 + cx ]
                           [ dy*KCELLS_CLUSTER_LEVEL2 + dx];
                }
                dx++;
            }
        }
        cx++;
        dx = 0;
    }
    return 0;
}
#endif

/****************************************************
 *
 * KCColumnCluster
 *
 ****************************************************/

KCColumnCluster::KCColumnCluster()
        : m_first(0), m_autoDelete(false)
{
    m_cluster = (KCColumnFormat***)malloc(KCELLS_CLUSTER_LEVEL1 * sizeof(KCColumnFormat**));

    for (int x = 0; x < KCELLS_CLUSTER_LEVEL1; ++x)
        m_cluster[ x ] = 0;
}

KCColumnCluster::~KCColumnCluster()
{
    for (int x = 0; x < KCELLS_CLUSTER_LEVEL1; ++x) {
        KCColumnFormat** cl = m_cluster[ x ];
        if (cl) {
            free(cl);
            m_cluster[ x ] = 0;
        }
    }

    if (m_autoDelete) {
        KCColumnFormat* cell = m_first;
        while (cell) {
            KCColumnFormat* n = cell->next();
            delete cell;
            cell = n;
        }
    }


    free(m_cluster);
}

KCColumnFormat* KCColumnCluster::lookup(int col)
{
    if (col >= KCELLS_CLUSTER_MAX || col < 0) {
        kDebug(36001) << "KCColumnCluster::lookup: invalid column value (col:"
        << col << ")" << endl;
        return 0;
    }

    int cx = col / KCELLS_CLUSTER_LEVEL2;
    int dx = col % KCELLS_CLUSTER_LEVEL2;

    KCColumnFormat** cl = m_cluster[ cx ];
    if (!cl)
        return 0;

    return cl[ dx ];
}

const KCColumnFormat* KCColumnCluster::lookup(int col) const
{
    if (col >= KCELLS_CLUSTER_MAX || col < 0) {
        kDebug(36001) << "KCColumnCluster::lookup: invalid column value (col:"
        << col << ")" << endl;
        return 0;
    }

    int cx = col / KCELLS_CLUSTER_LEVEL2;
    int dx = col % KCELLS_CLUSTER_LEVEL2;

    KCColumnFormat** cl = m_cluster[ cx ];
    if (!cl)
        return 0;

    return cl[ dx ];
}

void KCColumnCluster::clear()
{
    for (int x = 0; x < KCELLS_CLUSTER_LEVEL1; ++x) {
        KCColumnFormat** cl = m_cluster[ x ];
        if (cl) {
            free(cl);
            m_cluster[ x ] = 0;
        }
    }

    if (m_autoDelete) {
        KCColumnFormat* cell = m_first;
        while (cell) {
            KCColumnFormat* n = cell->next();
            delete cell;
            cell = n;
        }
    }

    m_first = 0;
}

void KCColumnCluster::insertElement(KCColumnFormat* lay, int col)
{
    if (col >= KCELLS_CLUSTER_MAX || col < 0) {
        kDebug(36001) << "KCColumnCluster::insertElement: invalid column value (col:"
        << col << ")" << endl;
        return;
    }

    int cx = col / KCELLS_CLUSTER_LEVEL2;
    int dx = col % KCELLS_CLUSTER_LEVEL2;

    KCColumnFormat** cl = m_cluster[ cx ];
    if (!cl) {
        cl = (KCColumnFormat**)malloc(KCELLS_CLUSTER_LEVEL2 * sizeof(KCColumnFormat*));
        m_cluster[ cx ] = cl;

        for (int a = 0; a < KCELLS_CLUSTER_LEVEL2; ++a)
            cl[ a ] = 0;
    }

    if (cl[ dx ])
        removeElement(col);

    cl[ dx ] = lay;

    if (m_first) {
        lay->setNext(m_first);
        m_first->setPrevious(lay);
    }
    m_first = lay;
}

void KCColumnCluster::removeElement(int col)
{
    if (col >= KCELLS_CLUSTER_MAX || col < 0) {
        kDebug(36001) << "KCColumnCluster::removeElement: invalid column value (col:"
        << col << ")" << endl;
        return;
    }

    int cx = col / KCELLS_CLUSTER_LEVEL2;
    int dx = col % KCELLS_CLUSTER_LEVEL2;

    KCColumnFormat** cl = m_cluster[ cx ];
    if (!cl)
        return;

    KCColumnFormat* c = cl[ dx ];
    if (!c)
        return;

    cl[ dx ] = 0;

    if (m_autoDelete) {
        if (m_first == c)
            m_first = c->next();
        delete c;
    } else {
        if (m_first == c)
            m_first = c->next();
        if (c->previous())
            c->previous()->setNext(c->next());
        if (c->next())
            c->next()->setPrevious(c->previous());
        c->setNext(0);
        c->setPrevious(0);
    }
}

bool KCColumnCluster::insertColumn(int col)
{
    if (col >= KCELLS_CLUSTER_MAX || col < 0) {
        kDebug(36001) << "KCColumnCluster::insertColumn: invalid column value (col:"
        << col << ")" << endl;
        return false;
    }

    int cx = col / KCELLS_CLUSTER_LEVEL2;
    int dx = col % KCELLS_CLUSTER_LEVEL2;

    // Is there a column layout at the right most position ?
    // In this case the shift is impossible.
    KCColumnFormat** cl = m_cluster[ KCELLS_CLUSTER_LEVEL1 - 1 ];
    if (cl && cl[ KCELLS_CLUSTER_LEVEL2 - 1 ])
        return false;

    bool a = autoDelete();
    setAutoDelete(false);

    for (int i = KCELLS_CLUSTER_LEVEL1 - 1; i >= cx ; --i) {
        KCColumnFormat** cl = m_cluster[ i ];
        if (cl) {
            int left = 0;
            if (i == cx)
                left = dx;
            int right = KCELLS_CLUSTER_LEVEL2 - 1;
            if (i == KCELLS_CLUSTER_LEVEL1 - 1)
                right = KCELLS_CLUSTER_LEVEL2 - 2;
            for (int k = right; k >= left; --k) {
                KCColumnFormat* c = cl[ k ];
                if (c) {
                    removeElement(c->column());
                    c->setColumn(c->column() + 1);
                    insertElement(c, c->column());
                }
            }
        }
    }

    setAutoDelete(a);

    return true;
}

bool KCColumnCluster::removeColumn(int column)
{
    if (column >= KCELLS_CLUSTER_MAX || column < 0) {
        kDebug(36001) << "KCColumnCluster::removeColumn: invalid column value (col:"
        << column << ")" << endl;
        return false;
    }

    int cx = column / KCELLS_CLUSTER_LEVEL2;
    int dx = column % KCELLS_CLUSTER_LEVEL2;

    removeElement(column);

    bool a = autoDelete();
    setAutoDelete(false);

    for (int i = cx; i < KCELLS_CLUSTER_LEVEL1; ++i) {
        KCColumnFormat** cl = m_cluster[ i ];
        if (cl) {
            int left = 0;
            if (i == cx)
                left = dx + 1;
            int right = KCELLS_CLUSTER_LEVEL2 - 1;
            for (int k = left; k <= right; ++k) {
                KCColumnFormat* c = cl[ k ];
                if (c) {
                    removeElement(c->column());
                    c->setColumn(c->column() - 1);
                    insertElement(c, c->column());
                }
            }
        }
    }

    setAutoDelete(a);

    return true;
}

void KCColumnCluster::setAutoDelete(bool a)
{
    m_autoDelete = a;
}

bool KCColumnCluster::autoDelete() const
{
    return m_autoDelete;
}

KCColumnFormat* KCColumnCluster::next(int col) const
{
    if (col >= KCELLS_CLUSTER_MAX || col < 0) {
        kDebug(36001) << "KCColumnCluster::next: invalid column value (col:"
        << col << ")" << endl;
        return 0;
    }

    int cx = (col + 1) / KCELLS_CLUSTER_LEVEL2;
    int dx = (col + 1) % KCELLS_CLUSTER_LEVEL2;

    while (cx < KCELLS_CLUSTER_LEVEL1) {
        if (m_cluster[ cx ]) {
            while (dx < KCELLS_CLUSTER_LEVEL2) {

                if (m_cluster[ cx ][  dx ]) {
                    return m_cluster[ cx ][ dx ];
                }
                ++dx;
            }
        }
        ++cx;
        dx = 0;
    }
    return 0;
}

void KCColumnCluster::operator=(const KCColumnCluster & other)
{
    m_first = 0;
    m_autoDelete = other.m_autoDelete;
    // TODO Stefan: Optimize!
    m_cluster = (KCColumnFormat***)malloc(KCELLS_CLUSTER_LEVEL1 * sizeof(KCColumnFormat**));
    for (int i = 0; i < KCELLS_CLUSTER_LEVEL1; ++i) {
        if (other.m_cluster[i]) {
            m_cluster[i] = (KCColumnFormat**)malloc(KCELLS_CLUSTER_LEVEL2 * sizeof(KCColumnFormat*));
            for (int j = 0; j < KCELLS_CLUSTER_LEVEL2; ++j) {
                m_cluster[i][j] = 0;
                if (other.m_cluster[i][j]) {
                    KCColumnFormat* columnFormat = new KCColumnFormat(*other.m_cluster[i][j]);
                    columnFormat->setNext(0);
                    columnFormat->setPrevious(0);
                    insertElement(columnFormat, columnFormat->column());
                }
            }
        } else
            m_cluster[i] = 0;
    }
}

/****************************************************
 *
 * KCRowCluster
 *
 ****************************************************/

KCRowCluster::KCRowCluster()
        : m_first(0), m_autoDelete(false)
{
    m_cluster = (KCRowFormat***)malloc(KCELLS_CLUSTER_LEVEL1 * sizeof(KCRowFormat**));

    for (int x = 0; x < KCELLS_CLUSTER_LEVEL1; ++x)
        m_cluster[ x ] = 0;
}

KCRowCluster::~KCRowCluster()
{
    for (int x = 0; x < KCELLS_CLUSTER_LEVEL1; ++x) {
        KCRowFormat** cl = m_cluster[ x ];
        if (cl) {
            free(cl);
            m_cluster[ x ] = 0;
        }
    }

    if (m_autoDelete) {
        KCRowFormat* cell = m_first;
        while (cell) {
            KCRowFormat* n = cell->next();
            delete cell;
            cell = n;
        }
    }

    free(m_cluster);
}

const KCRowFormat* KCRowCluster::lookup(int row) const
{
    if (row >= KCELLS_CLUSTER_MAX || row < 0) {
        kDebug(36001) << "KCRowCluster::lookup: invalid row value (row:"
        << row << ")" << endl;
        return 0;
    }

    int cx = row / KCELLS_CLUSTER_LEVEL2;
    int dx = row % KCELLS_CLUSTER_LEVEL2;

    KCRowFormat** cl = m_cluster[ cx ];
    if (!cl)
        return 0;

    return cl[ dx ];
}

KCRowFormat* KCRowCluster::lookup(int row)
{
    if (row >= KCELLS_CLUSTER_MAX || row < 0) {
        kDebug(36001) << "KCRowCluster::lookup: invalid row value (row:"
        << row << ")" << endl;
        return 0;
    }

    int cx = row / KCELLS_CLUSTER_LEVEL2;
    int dx = row % KCELLS_CLUSTER_LEVEL2;

    KCRowFormat** cl = m_cluster[ cx ];
    if (!cl)
        return 0;

    return cl[ dx ];
}

void KCRowCluster::clear()
{
    for (int x = 0; x < KCELLS_CLUSTER_LEVEL1; ++x) {
        KCRowFormat** cl = m_cluster[ x ];
        if (cl) {
            free(cl);
            m_cluster[ x ] = 0;
        }
    }

    if (m_autoDelete) {
        KCRowFormat* cell = m_first;
        while (cell) {
            KCRowFormat* n = cell->next();
            delete cell;
            cell = n;
        }
    }

    m_first = 0;
}

void KCRowCluster::insertElement(KCRowFormat* lay, int row)
{
    if (row >= KCELLS_CLUSTER_MAX || row < 0) {
        kDebug(36001) << "KCRowCluster::insertElement: invalid row value (row:"
        << row << ")" << endl;
        return;
    }

    int cx = row / KCELLS_CLUSTER_LEVEL2;
    int dx = row % KCELLS_CLUSTER_LEVEL2;

    KCRowFormat** cl = m_cluster[ cx ];
    if (!cl) {
        cl = (KCRowFormat**)malloc(KCELLS_CLUSTER_LEVEL2 * sizeof(KCRowFormat*));
        m_cluster[ cx ] = cl;

        for (int a = 0; a < KCELLS_CLUSTER_LEVEL2; ++a)
            cl[ a ] = 0;
    }

    if (cl[ dx ])
        removeElement(row);

    cl[ dx ] = lay;

    if (m_first) {
        lay->setNext(m_first);
        m_first->setPrevious(lay);
    }
    m_first = lay;
}

void KCRowCluster::removeElement(int row)
{
    if (row >= KCELLS_CLUSTER_MAX || row < 0) {
        kDebug(36001) << "KCRowCluster::removeElement: invalid row value (row:"
        << row << ")" << endl;
        return;
    }

    int cx = row / KCELLS_CLUSTER_LEVEL2;
    int dx = row % KCELLS_CLUSTER_LEVEL2;

    KCRowFormat** cl = m_cluster[ cx ];
    if (!cl)
        return;

    KCRowFormat* c = cl[ dx ];
    if (!c)
        return;

    cl[ dx ] = 0;

    if (m_autoDelete) {
        if (m_first == c)
            m_first = c->next();
        delete c;
    } else {
        if (m_first == c)
            m_first = c->next();
        if (c->previous())
            c->previous()->setNext(c->next());
        if (c->next())
            c->next()->setPrevious(c->previous());
        c->setNext(0);
        c->setPrevious(0);
    }
}

bool KCRowCluster::insertRow(int row)
{
    if (row >= KCELLS_CLUSTER_MAX || row < 0) {
        kDebug(36001) << "KCRowCluster::insertRow: invalid row value (row:"
        << row << ")" << endl;
        return false;
    }

    int cx = row / KCELLS_CLUSTER_LEVEL2;
    int dx = row % KCELLS_CLUSTER_LEVEL2;

    // Is there a row layout at the bottom most position ?
    // In this case the shift is impossible.
    KCRowFormat** cl = m_cluster[ KCELLS_CLUSTER_LEVEL1 - 1 ];
    if (cl && cl[ KCELLS_CLUSTER_LEVEL2 - 1 ])
        return false;

    bool a = autoDelete();
    setAutoDelete(false);

    for (int i = KCELLS_CLUSTER_LEVEL1 - 1; i >= cx ; --i) {
        KCRowFormat** cl = m_cluster[ i ];
        if (cl) {
            int left = 0;
            if (i == cx)
                left = dx;
            int right = KCELLS_CLUSTER_LEVEL2 - 1;
            if (i == KCELLS_CLUSTER_LEVEL1 - 1)
                right = KCELLS_CLUSTER_LEVEL2 - 2;
            for (int k = right; k >= left; --k) {
                KCRowFormat* c = cl[ k ];
                if (c) {
                    removeElement(c->row());
                    c->setRow(c->row() + 1);
                    insertElement(c, c->row());
                }
            }
        }
    }

    setAutoDelete(a);

    return true;
}

bool KCRowCluster::removeRow(int row)
{
    if (row >= KCELLS_CLUSTER_MAX || row < 0) {
        kDebug(36001) << "KCRowCluster::removeRow: invalid row value (row:"
        << row << ")" << endl;
        return false;
    }

    int cx = row / KCELLS_CLUSTER_LEVEL2;
    int dx = row % KCELLS_CLUSTER_LEVEL2;

    removeElement(row);

    bool a = autoDelete();
    setAutoDelete(false);

    for (int i = cx; i < KCELLS_CLUSTER_LEVEL1; ++i) {
        KCRowFormat** cl = m_cluster[ i ];
        if (cl) {
            int left = 0;
            if (i == cx)
                left = dx + 1;
            int right = KCELLS_CLUSTER_LEVEL2 - 1;
            for (int k = left; k <= right; ++k) {
                KCRowFormat* c = cl[ k ];
                if (c) {
                    removeElement(c->row());
                    c->setRow(c->row() - 1);
                    insertElement(c, c->row());
                }
            }
        }
    }

    setAutoDelete(a);

    return true;
}

void KCRowCluster::setAutoDelete(bool a)
{
    m_autoDelete = a;
}

bool KCRowCluster::autoDelete() const
{
    return m_autoDelete;
}

void KCRowCluster::operator=(const KCRowCluster & other)
{
    m_first = 0;
    m_autoDelete = other.m_autoDelete;
    // TODO Stefan: Optimize!
    m_cluster = (KCRowFormat***)malloc(KCELLS_CLUSTER_LEVEL1 * sizeof(KCRowFormat**));
    for (int i = 0; i < KCELLS_CLUSTER_LEVEL1; ++i) {
        if (other.m_cluster[i]) {
            m_cluster[i] = (KCRowFormat**)malloc(KCELLS_CLUSTER_LEVEL2 * sizeof(KCRowFormat*));
            for (int j = 0; j < KCELLS_CLUSTER_LEVEL2; ++j) {
                m_cluster[i][j] = 0;
                if (other.m_cluster[i][j]) {
                    KCRowFormat* rowFormat = new KCRowFormat(*other.m_cluster[i][j]);
                    rowFormat->setNext(0);
                    rowFormat->setPrevious(0);
                    insertElement(rowFormat, rowFormat->row());
                }
            }
        } else
            m_cluster[i] = 0;
    }
}
