/*
This file is part of the KDE project
Copyright (C) 2002 Fred Malabre <fmalabre@yahoo.com>

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
 * Boston, MA 02110-1301, USA.
*/

#ifndef KCELLS_LEADER_H
#define KCELLS_LEADER_H

#include <KCellsBaseWorker.h>
#include <KoFilterChain.h>
#include <kdebug.h>
#include <kcells_doc.h>
#include <KoDocumentInfo.h>
#include <kcells_map.h>

namespace KCells
{

/**
* This class is a leader which will call the callbacks
* to the worker with your own implementation for your export filter.
*
* To write an export filter, just instanciate a leader and
* implement you own worker based on the BaseWorker.
*
* @short A Leader for an KCells export filter
* @author Fred Malabre
* @see KCellsBaseWorker
* @see KoFilter
*/
class Leader
{
public:
    /**
    * Default constructor.
    * Call setWorker() before starting to convert the KCells document.
    *
    * @param filterChain the filter chain of the filter using the lib.
    */
    Leader(KoFilterChain *filterChain);

    /**
    * This constructor set up the Worker at initialization.
    * convert can be called right away after instanciation of the Leader.
    *
    * @param filterChain the filter chain of the filter using the lib.
    * @param newWorker implementation of a class derived from KCellsWorker.
    */
    Leader(KoFilterChain *filterChain, KCellsBaseWorker *newWorker);

    /**
    * Default destructor.
    * It does nothing.
    */
    virtual ~Leader();

private:
    KCellsBaseWorker *m_worker;
    KoFilterChain *m_filterChain;
    int m_maxCellColumn;
    int m_maxCellRow;
    void updateMaxCells(KCellsSheet *spreadSheet);

protected:
    KCellsBaseWorker *getWorker() const;
    KoFilter::ConversionStatus doSpreadCell(const KCCell& spreadCell, int column, int row);
    KoFilter::ConversionStatus doSpreadSheet(Sheet *spreadSheet);
    KoFilter::ConversionStatus doSpreadBook(KCDoc *document);
    KoFilter::ConversionStatus doInfo(KoDocumentInfo *info);

public:
    /**
    * Set your own implementation of the Worker.
    *
    * @param newWorker implementation of a class derived from KCellsWorker.
    */
    void setWorker(KCellsBaseWorker *newWorker);

    /**
    * Start the conversion process.
    * The callbacks of your Worker will be called at this time.
    *
    * @return status of the conversion.
    */
    KoFilter::ConversionStatus convert();
};

} // namespace KCells

#endif /* KCELLS_LEADER_H */
