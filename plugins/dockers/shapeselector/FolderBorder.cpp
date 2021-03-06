/*
 * Copyright (C) 2008 Thomas Zander <zander@kde.org>
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
#include "FolderBorder.h"

#include <KShape.h>
#include <KViewConverter.h>

#include <KGlobalSettings>

#include <QFontMetrics>
#include <QPainter>

FolderBorder::FolderBorder()
{
}

KInsets FolderBorder::borderInsets() const
{
    KInsets insets(1, 1, 1, 1);
    QFontMetricsF fm(KGlobalSettings::windowTitleFont());
    insets.top = fm.height();
    return insets;
}

bool FolderBorder::hasTransparency() const
{
    return false;
}

void FolderBorder::paint(KShape *shape, QPainter &painter, const KViewConverter &converter)
{
    qreal zoomX, zoomY;
    converter.zoom(&zoomX, &zoomY);
    painter.scale(zoomX, zoomY);

    Q_ASSERT(shape);
    qreal topBorder = 2;
    painter.setPen(QPen(KGlobalSettings::activeTitleColor()));
    QSizeF size = shape->size();
    painter.drawRect(QRectF(QPointF(),size));
    if (!shape->name().isEmpty()) {
        QFont font = KGlobalSettings::windowTitleFont();
        QFontMetricsF fm(font);
        topBorder += fm.height();
        QRectF rect(QPointF(0, -topBorder), QSizeF(qRound(shape->size().width()), topBorder));
        painter.fillRect(rect, QBrush(KGlobalSettings::activeTitleColor()));
        painter.setPen(QPen(KGlobalSettings::activeTextColor()));
        painter.drawText(rect, shape->name());
    }
}
