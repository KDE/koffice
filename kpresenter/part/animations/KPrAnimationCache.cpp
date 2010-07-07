/* This file is part of the KDE project
 * Copyright ( C ) 2010 Casper Boemann <cbo@boemannn.dk>
 * Copyright (C) 2010 Benjamin Port <port.benjamin@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (  at your option ) any later version.
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
#include "KPrAnimationCache.h"
#include <QList>
#include <QMap>
#include <QString>
#include <QVariant>

#include <KoShape.h>
#include <KoTextBlockData.h>

#include "kdebug.h"

KPrAnimationCache::KPrAnimationCache()
{
    clear();
}

KPrAnimationCache::~KPrAnimationCache()
{
}


bool KPrAnimationCache::hasValue(KoShape *shape, const QString &id)
{
    if (m_currentShapeValues.contains(shape))
        return m_currentShapeValues.value(shape).contains(id);
    return false;
}

bool KPrAnimationCache::hasValue(int step, KoTextBlockData *textBlockData, const QString &id)
{
    if (m_textBlockDataValuesStack[step].contains(textBlockData))
        return m_textBlockDataValuesStack[step].value(textBlockData).contains(id);
    return false;
}

bool KPrAnimationCache::hasValue(int step, KoShape *shape, const QString &id)
{
    if (m_shapeValuesStack[step].contains(shape))
        return m_shapeValuesStack[step].value(shape).contains(id);
    return false;
}

void KPrAnimationCache::setValue(int step, KoShape *shape, const QString &id, const QVariant &value)
{
    m_shapeValuesStack[step][shape][id] = value;
}

void KPrAnimationCache::setValue(int step, KoTextBlockData *textBlockData, const QString &id, const QVariant &value)
{
    m_textBlockDataValuesStack[step][textBlockData][id] = value;
}

QVariant KPrAnimationCache::value(KoShape *shape, const QString &id, const QVariant &defaultValue)
{
    if (m_currentShapeValues.contains(shape))
        return m_currentShapeValues.value(shape).value(id, defaultValue);
    return defaultValue;
}

QVariant KPrAnimationCache::value(int step, KoShape *shape, const QString &id)
{
    if (m_shapeValuesStack[step].contains(shape))
        return m_shapeValuesStack[step].value(shape).value(id);
    return QVariant();
}


QVariant KPrAnimationCache::value(KoTextBlockData *textBlockData, const QString &id, const QVariant &defaultValue)
{
    if (m_currentTextBlockDataValues.contains(textBlockData))
        return m_currentTextBlockDataValues.value(textBlockData).value(id, defaultValue);
    return defaultValue;
}

void KPrAnimationCache::init(int step, KoShape *shape, KoTextBlockData * textBlockData, const QString &id, const QVariant &value)
{
    if (textBlockData) {
        for (int i = m_textBlockDataValuesStack.size(); i <= step; ++i) {
            // copy previous values
            if (i > 0) {
                m_textBlockDataValuesStack.append(m_textBlockDataValuesStack[i-1]);
            }
            else {
                m_textBlockDataValuesStack.append(QMap<KoTextBlockData *, QMap<QString, QVariant> >());
            }
        }
        // check if value is valid
        if (value.isValid()) {
            m_textBlockDataValuesStack[step][textBlockData][id] = value;
        }
        else {
            m_textBlockDataValuesStack[step][textBlockData].remove(id);
        }

        // Check visibility
        if (id == "visibility") {
            for(int i = step - 1; i >= 0; i--)
            {
                if(!this->hasValue(i, textBlockData, id)){
                    this->setValue(i, textBlockData, id, value.toBool());
                }
            }
        }
    }
    else {
        for (int i = m_shapeValuesStack.size(); i <= step; ++i) {
            // copy previous values
            if (i > 0) {
                m_shapeValuesStack.append(m_shapeValuesStack[i-1]);
            }
            else {
                m_shapeValuesStack.append(QMap<KoShape *, QMap<QString, QVariant> >());
            }
        }
        // check if value is valid
        if (value.isValid()) {
            m_shapeValuesStack[step][shape][id] = value;
        }
        else {
            m_shapeValuesStack[step][shape].remove(id);
        }

        // Check visibility
        if (id == "visibility") {
            for(int i = step - 1; i >= 0; i--)
            {
                if (!this->hasValue(i, shape, id)) {
                    this->setValue(i, shape, id, value.toBool());
                }
            }
        }
    }
}

void KPrAnimationCache::update(KoShape *shape, KoTextBlockData * textBlockData, const QString &id, const QVariant &value)
{
    if (textBlockData) {
        if (id == "transform" && !m_next) {
            QTransform transform = m_currentTextBlockDataValues[textBlockData][id].value<QTransform>();
            m_currentTextBlockDataValues[textBlockData][id] = transform * value.value<QTransform>();
        }
        else {
            m_currentTextBlockDataValues[textBlockData][id] = value;
        }
    }
    else {
        if (id == "transform" && !m_next) {
            QTransform transform = m_currentShapeValues[shape][id].value<QTransform>();
            m_currentShapeValues[shape][id] = transform * value.value<QTransform>();
        }
        else {
            m_currentShapeValues[shape][id] = value;
        }
    }
    if (id == "transform") {
        m_next = false;
    }
}

void KPrAnimationCache::startStep(int step)
{
    if (m_shapeValuesStack.size() > step) {
        m_currentShapeValues = m_shapeValuesStack[step];
    }
    if (m_textBlockDataValuesStack.size() > step) {
        m_currentTextBlockDataValues = m_textBlockDataValuesStack[step];
    }
}

void KPrAnimationCache::endStep(int step)
{
    if (m_shapeValuesStack.size() > step) {
        m_currentShapeValues = m_shapeValuesStack[step+1];
    }
    if (m_textBlockDataValuesStack.size() > step) {
        m_currentTextBlockDataValues = m_textBlockDataValuesStack[step+1];
    }
}

void KPrAnimationCache::next()
{
    m_next = true;
}



void KPrAnimationCache::setPageSize(const QSizeF size)
{
    m_pageSize = size;
}

QSizeF KPrAnimationCache::pageSize() const
{
    return m_pageSize;
}

void KPrAnimationCache::setZoom(const qreal zoom)
{
    m_zoom = zoom;
}

qreal KPrAnimationCache::zoom() const
{
    return m_zoom;
}

void KPrAnimationCache::clear()
{
    m_zoom = 1;
    m_pageSize = QSizeF();
    m_currentShapeValues.clear();
    m_currentTextBlockDataValues.clear();
    m_shapeValuesStack.clear();
    m_textBlockDataValuesStack.clear();
    m_next = false;
    m_step = 0;
}
