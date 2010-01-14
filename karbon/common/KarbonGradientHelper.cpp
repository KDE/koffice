/* This file is part of the KDE project
 * Copyright (C) 2008 Jan Hambrecht <jaham@gmx.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "KarbonGradientHelper.h"

#include <KoShape.h>
#include <KoLineBorder.h>
#include <KoGradientBackground.h>

#include <QtGui/QGradient>
#include <math.h>

QGradient* KarbonGradientHelper::defaultGradient(QGradient::Type type, QGradient::Spread spread, const QGradientStops &stops)
{
    QGradient *gradient = 0;
    switch (type) {
    case QGradient::LinearGradient:
        gradient = new QLinearGradient(QPointF(0.0, 0.5), QPointF(1, 0.5));
        break;
    case QGradient::RadialGradient:
        gradient = new QRadialGradient(QPointF(0.5, 0.5), sqrt(0.5));
        break;
    case QGradient::ConicalGradient:
        gradient = new QConicalGradient(QPointF(0.5, 0.5), 0.0);
        break;
    default:
        return 0;
    }
    gradient->setCoordinateMode(QGradient::ObjectBoundingMode);
    gradient->setSpread(spread);
    gradient->setStops(stops);

    return gradient;
}

QGradient* KarbonGradientHelper::convertGradient(const QGradient * gradient, QGradient::Type newType)
{
    QPointF start, stop;
    // try to preserve gradient positions
    switch (gradient->type()) {
    case QGradient::LinearGradient: {
        const QLinearGradient *g = static_cast<const QLinearGradient*>(gradient);
        start = g->start();
        stop = g->finalStop();
        break;
    }
    case QGradient::RadialGradient: {
        const QRadialGradient *g = static_cast<const QRadialGradient*>(gradient);
        start = g->center();
        stop = QPointF(g->radius(), 0.0);
        break;
    }
    case QGradient::ConicalGradient: {
        const QConicalGradient *g = static_cast<const QConicalGradient*>(gradient);
        start = g->center();
        qreal radAngle = g->angle() * M_PI / 180.0;
        stop = QPointF(0.5 * cos(radAngle), 0.5 * sin(radAngle));
        break;
    }
    default:
        start = QPointF(0.0, 0.0);
        stop = QPointF(0.5, 0.5);
    }

    QGradient *newGradient = 0;
    switch (newType) {
    case QGradient::LinearGradient:
        newGradient = new QLinearGradient(start, stop);
        break;
    case QGradient::RadialGradient: {
        QPointF diff(stop - start);
        qreal radius = sqrt(diff.x()*diff.x() + diff.y()*diff.y());
        newGradient = new QRadialGradient(start, radius, start);
        break;
    }
    case QGradient::ConicalGradient: {
        QPointF diff(stop - start);
        qreal angle = atan2(diff.y(), diff.x());
        if (angle < 0.0)
            angle += 2 * M_PI;
        newGradient = new QConicalGradient(start, angle * 180/M_PI);
        break;
    }
    default:
        return 0;
    }
    newGradient->setCoordinateMode(QGradient::ObjectBoundingMode);
    newGradient->setSpread(gradient->spread());
    newGradient->setStops(gradient->stops());

    return newGradient;
}

QColor KarbonGradientHelper::colorAt(qreal position, const QGradientStops &stops)
{
    if (! stops.count())
        return QColor();

    if (stops.count() == 1)
        return stops.first().second;

    QGradientStop prevStop(-1.0, QColor());
    QGradientStop nextStop(2.0, QColor());
    // find framing gradient stops
    foreach(const QGradientStop & stop, stops) {
        if (stop.first > prevStop.first && stop.first < position)
            prevStop = stop;
        if (stop.first < nextStop.first && stop.first > position)
            nextStop = stop;
    }

    QColor theColor;

    if (prevStop.first < 0.0) {
        // new stop is before the first stop
        theColor = nextStop.second;
    } else if (nextStop.first > 1.0) {
        // new stop is after the last stop
        theColor = prevStop.second;
    } else {
        // linear interpolate colors between framing stops
        QColor prevColor = prevStop.second, nextColor = nextStop.second;
        qreal colorScale = (position - prevStop.first) / (nextStop.first - prevStop.first);
        theColor.setRedF(prevColor.redF() + colorScale *(nextColor.redF() - prevColor.redF()));
        theColor.setGreenF(prevColor.greenF() + colorScale *(nextColor.greenF() - prevColor.greenF()));
        theColor.setBlueF(prevColor.blueF() + colorScale *(nextColor.blueF() - prevColor.blueF()));
        theColor.setAlphaF(prevColor.alphaF() + colorScale *(nextColor.alphaF() - prevColor.alphaF()));
    }
    return theColor;
}
