/* This file is part of the KDE project
 * Copyright (c) 2010 Jan Hambrecht <jaham@gmx.net>
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

#ifndef MORPHOLOGYEFFECTCONFIGWIDGET_H
#define MORPHOLOGYEFFECTCONFIGWIDGET_H

#include "KFilterEffectConfigWidgetBase.h"

class KFilterEffect;
class MorphologyEffect;
class KDoubleNumInput;
class QButtonGroup;

class MorphologyEffectConfigWidget : public KFilterEffectConfigWidgetBase
{
    Q_OBJECT
public:
    MorphologyEffectConfigWidget(QWidget *parent = 0);

    /// reimplemented from KFilterEffectConfigWidgetBase
    virtual bool editFilterEffect(KFilterEffect * filterEffect);

private slots:
    void radiusXChanged(double x);
    void radiusYChanged(double y);
    void operatorChanged(int op);
private:
    MorphologyEffect * m_effect;
    QButtonGroup *m_operator;
    KDoubleNumInput *m_radiusX;
    KDoubleNumInput *m_radiusY;
};

#endif // MORPHOLOGYEFFECTCONFIGWIDGET_H
