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
the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.
*/

#ifndef KSPREAD_LEADER_H
#define KSPREAD_LEADER_H

#include <KSpreadBaseWorker.h>
#include <koFilterChain.h>
#include <kdebug.h>
#include <kspread_doc.h>
#include <koDocumentInfo.h>
#include <kspread_map.h>


/**
* This class is a leader which will call the callbacks
* to the worker with your own implementation for your export filter.
*
* To write an export filter, just instanciate a leader and
* implement you own worker based on the BaseWorker.
*
* @short A Leader for an KSpread export filter
* @author Fred Malabre
* @see KSpreadBaseWorker
* @see KoFilter
*/
class KSpreadLeader
{
public:
	/**
	* Default constructor.
	* Call setWorker() before starting to convert the KSpread document.
	*
	* @param the filter chain of the filter using the lib.
	*/
	KSpreadLeader(KoFilterChain *filterChain);

	/**
	* This constructor set up the Worker at initialization.
	* convert can be called right away after instanciation of the Leader.
	*
	* @param the filter chain of the filter using the lib.
	* @param implementation of a class derived from KSpreadWorker.
	*/
	KSpreadLeader(KoFilterChain *filterChain, KSpreadBaseWorker *newWorker);

	/**
	* Default destructor.
	* It does nothing.
	*/
	virtual ~KSpreadLeader();

private:
	KSpreadBaseWorker *m_worker;
	KoFilterChain *m_filterChain;
	int m_maxCellColumn;
	int m_maxCellRow;
	void updateMaxCells(KSpreadSheet *spreadSheet);

protected:
	KSpreadBaseWorker *getWorker() const;
	KoFilter::ConversionStatus doSpreadCell(KSpreadCell *spreadCell, int column, int row);
	KoFilter::ConversionStatus doSpreadSheet(KSpreadSheet *spreadSheet);
	KoFilter::ConversionStatus doSpreadBook(KSpreadDoc *document);
	KoFilter::ConversionStatus doInfo(KoDocumentInfo *info);

public:
	/**
	* Set your own implementation of the Worker.
	*
	* @param implementation of a class derived from KSpreadWorker.
	*/
	void setWorker(KSpreadBaseWorker *newWorker);

	/**
	* Start the conversion process.
	* The callbacks of your Worker will be called at this time.
	*
	* @return status of the conversion.
	*/
	KoFilter::ConversionStatus convert();
};

#endif /* KSPREAD_LEADER_H */
