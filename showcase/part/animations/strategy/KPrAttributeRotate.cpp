/* This file is part of the KDE project
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

#include "KPrAttributeRotate.h"
#include "../KPrAnimationCache.h"
#include "KoShape.h"
#include "KPrShapeAnimations.h"
#include "KoTextBlockData.h"
#include "KoTextShapeData.h"
#include <QTextDocument>
#include <QTextLayout>
#include "kdebug.h"

KPrAttributeRotate::KPrAttributeRotate() : KPrAnimationAttribute("rotate")
{
}

void KPrAttributeRotate::updateCache(KPrAnimationCache *cache, KPrShapeAnimation *shapeAnimation, qreal value)
{
    qreal tx = 0.0, ty = 0.0;
    KoShape * shape = shapeAnimation->shape();
    KoTextBlockData * textBlockData = shapeAnimation->textBlockData();
    QTransform transform;
    if (textBlockData) {
        if (KoTextShapeData *textShapeData = dynamic_cast<KoTextShapeData*>(shape->userData())) {
            QTextDocument *textDocument = textShapeData->document();
            for (int i = 0; i < textDocument->blockCount(); i++) {
                QTextBlock textBlock = textDocument->findBlockByNumber(i);
                if (textBlock.userData() == textBlockData) {
                    QTextLayout *layout = textBlock.layout();
                    tx = layout->minimumWidth() * cache->zoom() / 2;
                    ty = layout->boundingRect().height() * cache->zoom() / 2;
                }
            }
        }
    }
    else {
        tx = shape->size().width() * cache->zoom() / 2;
        ty = shape->size().height() * cache->zoom() / 2;
    }
    transform.translate(tx, ty).rotate(value).translate(-tx, -ty);
    cache->update(shape, shapeAnimation->textBlockData(),"transform", transform);
}

void KPrAttributeRotate::initCache(KPrAnimationCache *animationCache, int step, KPrShapeAnimation * shapeAnimation, qreal startValue, qreal endValue)
{
    qreal tx = 0.0, ty = 0.0;
    KoShape * shape = shapeAnimation->shape();
    KoTextBlockData * textBlockData = shapeAnimation->textBlockData();
    if (textBlockData) {
        if (KoTextShapeData *textShapeData = dynamic_cast<KoTextShapeData*>(shape->userData())) {
            QTextDocument *textDocument = textShapeData->document();
            for (int i = 0; i < textDocument->blockCount(); i++) {
                QTextBlock textBlock = textDocument->findBlockByNumber(i);
                if (textBlock.userData() == textBlockData) {
                    QTextLayout *layout = textBlock.layout();
                    tx = layout->minimumWidth() * animationCache->zoom() / 2;
                    ty = layout->boundingRect().height() * animationCache->zoom() / 2;
                }
            }
        }
    }
    else {
        tx = shape->size().width() * animationCache->zoom() / 2;
        ty = shape->size().height() * animationCache->zoom() / 2;
    }
    animationCache->init(step, shape, textBlockData, "transform", QTransform().translate(tx, ty).rotate(startValue).translate(-tx, -ty));
    animationCache->init(step + 1, shape, textBlockData, "transform", QTransform().translate(tx, ty).rotate(endValue).translate(-tx, -ty));
}
