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

#ifndef BLENDEFFECTCONFIGWIDGET_H
#define BLENDEFFECTCONFIGWIDGET_H

#include "KoFilterEffectConfigWidgetBase.h"

class BlendEffect;
class KoFilterEffect;
class KComboBox;

class BlendEffectConfigWidget : public KoFilterEffectConfigWidgetBase
{
    Q_OBJECT
public:
    BlendEffectConfigWidget(QWidget *parent = 0);
    
    /// reimplemented from KoFilterEffectConfigWidgetBase
    virtual bool editFilterEffect(KoFilterEffect * filterEffect);
    
private slots:
    void modeChanged(int index);
private:
    KComboBox * m_mode;
    BlendEffect * m_effect;
};

#endif // BLENDEFFECTCONFIGWIDGET_H
