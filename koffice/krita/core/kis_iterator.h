/* This file is part of the KDE project
 *   Copyright (c) 2004 Casper Boemann <cbr@boemann.dkt>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

*/
#ifndef KIS_ITERATOR_H_
#define KIS_ITERATOR_H_

#include <qglobal.h>
#include <ksharedptr.h>

class KisTiledRectIterator;
typedef KSharedPtr<KisTiledRectIterator> KisTiledRectIteratorSP;

class KisTiledVLineIterator;
typedef KSharedPtr<KisTiledVLineIterator> KisTiledVLineIteratorSP;

class KisTiledHLineIterator;
typedef KSharedPtr<KisTiledHLineIterator> KisTiledHLineIteratorSP;

class KisDataManager;

/**
 * The KisRectIterator iterators over a rectangular area in the most efficient order. That is,
 * there is no guarantee that the iterator will work scanline by scanline.
 */
class KisRectIterator
{


public:
    KisRectIterator ( KisDataManager *dm, Q_INT32  x, Q_INT32  y, Q_INT32  w, Q_INT32  h, bool writable);

public:
    virtual ~KisRectIterator();
    KisRectIterator(const KisRectIterator& rhs);
    KisRectIterator& operator=(const KisRectIterator& rhs);


public:
    /// returns a pointer to the pixel data. Do NOT interpret the data - leave that to a colorstrategy
    Q_UINT8 * rawData() const;

    /// Returns a pointer to the pixel data as it was at the moment of the last memento creation.
    const Q_UINT8 * oldRawData() const;

    /// Returns the number of consequtive pixels that we point at
    /// This is useful for optimizing
    Q_INT32 nConseqPixels() const;

    /// Advances a number of pixels until it reaches the end of the rect
    KisRectIterator & operator+=(int n);

    /// Advances one pixel going to the beginning of the next line when it reaches the end of a line
    KisRectIterator & operator++();

    /// returns true when iterators has reached the end
    bool isDone()  const;

     // current x position
     Q_INT32 x() const;

     // current y position
     Q_INT32 y() const;

private:

    KisTiledRectIteratorSP m_iter;
};

class KisHLineIterator
{

public:

    KisHLineIterator ( KisDataManager *dm, Q_INT32  x, Q_INT32 y, Q_INT32 w, bool writable);

public:

    virtual ~KisHLineIterator();
    KisHLineIterator(const KisHLineIterator& rhs);
    KisHLineIterator& operator=(const KisHLineIterator& rhs);

public:
    /// Returns a pointer to the pixel data. Do NOT interpret the data - leave that to a colorstrategy
    Q_UINT8 *rawData() const;

    /// Returns a pointer to the pixel data as it was at the moment of the last memento creation.
    const Q_UINT8 *oldRawData() const;

    /// Advances one pixel until it reaches the end of the line
    KisHLineIterator & operator++();

    /// Returns the number of consequtive horizontal pixels that we point at
    /// This is useful for optimizing
    Q_INT32 nConseqHPixels() const;

    /// Advances a number of pixels until it reaches the end of the line
    KisHLineIterator & operator+=(int n);

    /// Goes back one pixel until it reaches the beginning of the line
    KisHLineIterator & operator--();

    /// returns true when iterators has reached the end
    bool isDone()  const;

    /// current x position
    Q_INT32 x() const;

    /// current y position
    Q_INT32 y() const;

    /// increment to the next row and rewind to the begining
    void nextRow();


private:

    KisTiledHLineIteratorSP m_iter;
};

class KisVLineIterator
{

public:
    KisVLineIterator ( KisDataManager *dm, Q_INT32  x, Q_INT32 y, Q_INT32  h, bool writable);
public:
    ~KisVLineIterator();
    KisVLineIterator(const KisVLineIterator& rhs);
    KisVLineIterator& operator=(const KisVLineIterator& rhs);

public:
    /// returns a pointer to the pixel data. Do NOT interpret the data - leave that to a colorstrategy
    Q_UINT8 *rawData() const;

    /// Returns a pointer to the pixel data as it was at the moment of the last memento creation.
    const Q_UINT8 * oldRawData() const;

    /// Advances one pixel until it reaches the end of the line
    KisVLineIterator & operator++();

    /// returns true when iterators has reached the end
    bool isDone() const;

    /// current x position
    Q_INT32 x() const;

    /// current y position
    Q_INT32 y() const;
    
    /// increment to the next column and rewind to the begining
    void nextCol();

private:

    KisTiledVLineIteratorSP m_iter;

};

#endif
