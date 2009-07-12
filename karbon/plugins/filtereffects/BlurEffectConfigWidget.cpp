/* This file is part of the KDE project
 * Copyright (c) 2009 Jan Hambrecht <jaham@gmx.net>
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "BlurEffectConfigWidget.h"
#include "BlurEffect.h"
#include "KoFilterEffect.h"
#include <KNumInput>
#include <QtGui/QGridLayout>

BlurEffectConfigWidget::BlurEffectConfigWidget(QWidget *parent)
    : KoFilterEffectConfigWidgetBase(parent), m_effect(0)
{
    QGridLayout * g = new QGridLayout(this);
    
    m_stdDeviation = new KDoubleNumInput(this);
    m_stdDeviation->setRange(0.0, 100.0, 1.0, true);
    g->addWidget(m_stdDeviation, 0, 0);
    setLayout(g);
    
    connect(m_stdDeviation, SIGNAL(valueChanged(double)), this, SLOT(stdDeviationChanged(double)));
}

bool BlurEffectConfigWidget::editFilterEffect(KoFilterEffect * filterEffect)
{
    m_effect = dynamic_cast<BlurEffect*>(filterEffect);
    if (!m_effect)
        return false;
    
    m_stdDeviation->setValue(m_effect->deviation().x());
    return true;
}

void BlurEffectConfigWidget::stdDeviationChanged(double stdDeviation)
{
    if( !m_effect)
        return;
    
    m_effect->setDeviation(QPointF(stdDeviation, stdDeviation));
    emit filterChanged();
}

#include "BlurEffectConfigWidget.moc"
