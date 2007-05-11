/****************************************************************************
 ** Copyright (C) 2006 Klarälvdalens Datakonsult AB.  All rights reserved.
 **
 ** This file is part of the KD Chart library.
 **
 ** This file may be distributed and/or modified under the terms of the
 ** GNU General Public License version 2 as published by the Free Software
 ** Foundation and appearing in the file LICENSE.GPL included in the
 ** packaging of this file.
 **
 ** Licensees holding valid commercial KD Chart licenses may use this file in
 ** accordance with the KD Chart Commercial License Agreement provided with
 ** the Software.
 **
 ** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 ** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 **
 ** See http://www.kdab.net/kdchart for
 **   information about KDChart Commercial License Agreements.
 **
 ** Contact info@kdab.net if any conditions of this
 ** licensing are not clear to you.
 **
 **********************************************************************/

#ifndef KDCHARTCARTESIANGRID_H
#define KDCHARTCARTESIANGRID_H

#include "KDChartCartesianCoordinatePlane.h"
#include "KDChartAbstractGrid.h"

namespace KDChart {

    class PaintContext;
    class CartesianCoordinatePlane;

    /**
     * \internal
     *
     * \brief Class for the grid in a cartesian plane.
     *
     * The CartesianGrid interface is used
     * for calculating and for drawing
     * the horizonal grid lines, and the vertical grid lines
     * of a cartesian coordinate plane.
     */
    class CartesianGrid : public AbstractGrid
    {
    public:
        CartesianGrid() : AbstractGrid(){}
        virtual ~CartesianGrid(){}

        virtual void drawGrid( PaintContext* context );

    private:
        virtual DataDimensionsList calculateGrid(
              const DataDimensionsList& rawDataDimensions ) const;

        /**
          * Helper function called by calculateGrid().
          *
          * Classes derived from CartesianGrid can overwrite calculateGridXY() if they need
          * a way of calculating the start/end/step width of their horizontal grid
          * lines (or of their vertical grid lines, resp.), that is different from the
          * default implementation of this method.
          */
        virtual DataDimension calculateGridXY(
            const DataDimension& rawDataDimension,
            Qt::Orientation orientation ) const;

        /**
          * Helper function called by calculateGridXY().
          *
          * Classes derived from CartesianGrid can overwrite calculateStepWidth() if they need
          * a way of calculating the step width, based upon given start/end values
          * for their horizontal grid lines (or for their vertical grid lines, resp.),
          * that is different from the default implementation of this method.
          *
          * \note The CartesianGrid class tries to keep the displayed range as near to
          * the raw data range as possible, so in most cases there should be no reason
          * to change the default implementation:  Adjusting
          * KDChart::GridAttributes::setGridGranularitySequence should be sufficient.
          *
          * \param start The raw start value of the data range.
          * \param end The raw end value of the data range.
          * \param granularities The list of allowed granularities.
          *
          * \returns stepWidth: One of the values from the granularities
          * list, optionally multiplied by a positive (or negative, resp.)
          * power of ten. subStepWidth: The matching width for sub-grid lines.
          */
        virtual void calculateStepWidth(
            qreal start, qreal end,
            const QList<qreal>& granularities,
            Qt::Orientation orientation,
            qreal& stepWidth, qreal& subStepWidth ) const;
    };

}

#endif
