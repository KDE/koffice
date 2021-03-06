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

#include "SnapGuideConfigWidget_p.h"
#include "KSnapGuide.h"

SnapGuideConfigWidget::SnapGuideConfigWidget(KSnapGuide * snapGuide, QWidget * parent)
        : QWidget(parent), m_snapGuide(snapGuide)
{
    widget.setupUi(this);

    widget.orthogonalSnapGuide->setIcon(KIcon("snap-orto"));
    widget.nodeSnapGuide->setIcon(KIcon("snap-node"));
    widget.extensionSnapGuide->setIcon(KIcon("snap-extension"));
    widget.intersectionSnapGuide->setIcon(KIcon("snap-intersection"));
    widget.boundingBoxSnapGuide->setIcon(KIcon("snap-boundingbox"));
    widget.lineGuideSnapGuide->setIcon(KIcon("snap-guideline"));

    updateControls();

    connect(widget.useSnapGuides, SIGNAL(stateChanged(int)), this, SLOT(snappingEnabled(int)));
    connect(widget.orthogonalSnapGuide, SIGNAL(stateChanged(int)), this, SLOT(strategyChanged()));
    connect(widget.nodeSnapGuide, SIGNAL(stateChanged(int)), this, SLOT(strategyChanged()));
    connect(widget.extensionSnapGuide, SIGNAL(stateChanged(int)), this, SLOT(strategyChanged()));
    connect(widget.intersectionSnapGuide, SIGNAL(stateChanged(int)), this, SLOT(strategyChanged()));
    connect(widget.boundingBoxSnapGuide, SIGNAL(stateChanged(int)), this, SLOT(strategyChanged()));
    connect(widget.lineGuideSnapGuide, SIGNAL(stateChanged(int)), this, SLOT(strategyChanged()));
    connect(widget.snapDistance, SIGNAL(valueChanged(int)), this, SLOT(distanceChanged(int)));

    widget.useSnapGuides->setCheckState(snapGuide->isSnapping() ? Qt::Checked : Qt::Unchecked);
}

SnapGuideConfigWidget::~SnapGuideConfigWidget()
{
}

void SnapGuideConfigWidget::snappingEnabled(int state)
{
    widget.orthogonalSnapGuide->setEnabled(state == Qt::Checked);
    widget.nodeSnapGuide->setEnabled(state == Qt::Checked);
    widget.extensionSnapGuide->setEnabled(state == Qt::Checked);
    widget.intersectionSnapGuide->setEnabled(state == Qt::Checked);
    widget.boundingBoxSnapGuide->setEnabled(state == Qt::Checked);
    widget.lineGuideSnapGuide->setEnabled(state == Qt::Checked);
    widget.snapDistance->setEnabled(state == Qt::Checked);

    m_snapGuide->enableSnapping(state == Qt::Checked);
}

void SnapGuideConfigWidget::strategyChanged()
{
    KSnapGuide::Strategies strategies;
    if (widget.orthogonalSnapGuide->checkState() == Qt::Checked)
        strategies |= KSnapGuide::OrthogonalSnapping;
    if (widget.nodeSnapGuide->checkState() == Qt::Checked)
        strategies |= KSnapGuide::NodeSnapping;
    if (widget.extensionSnapGuide->checkState() == Qt::Checked)
        strategies |= KSnapGuide::ExtensionSnapping;
    if (widget.intersectionSnapGuide->checkState() == Qt::Checked)
        strategies |= KSnapGuide::IntersectionSnapping;
    if (widget.boundingBoxSnapGuide->checkState() == Qt::Checked)
        strategies |= KSnapGuide::BoundingBoxSnapping;
    if (widget.lineGuideSnapGuide->checkState() == Qt::Checked)
        strategies |= KSnapGuide::GuideLineSnapping;

    m_snapGuide->enableSnapStrategies(strategies);
}

void SnapGuideConfigWidget::distanceChanged(int distance)
{
    m_snapGuide->setSnapDistance(distance);
}

void SnapGuideConfigWidget::updateControls()
{
    if (m_snapGuide->enabledSnapStrategies() & KSnapGuide::OrthogonalSnapping)
        widget.orthogonalSnapGuide->setCheckState(Qt::Checked);
    else
        widget.orthogonalSnapGuide->setCheckState(Qt::Unchecked);
    if (m_snapGuide->enabledSnapStrategies() & KSnapGuide::NodeSnapping)
        widget.nodeSnapGuide->setCheckState(Qt::Checked);
    else
        widget.nodeSnapGuide->setCheckState(Qt::Unchecked);
    if (m_snapGuide->enabledSnapStrategies() & KSnapGuide::ExtensionSnapping)
        widget.extensionSnapGuide->setCheckState(Qt::Checked);
    else
        widget.extensionSnapGuide->setCheckState(Qt::Unchecked);
    if (m_snapGuide->enabledSnapStrategies() & KSnapGuide::IntersectionSnapping)
        widget.intersectionSnapGuide->setCheckState(Qt::Checked);
    else
        widget.intersectionSnapGuide->setCheckState(Qt::Unchecked);
    if (m_snapGuide->enabledSnapStrategies() & KSnapGuide::BoundingBoxSnapping)
        widget.boundingBoxSnapGuide->setCheckState(Qt::Checked);
    else
        widget.boundingBoxSnapGuide->setCheckState(Qt::Unchecked);
    if (m_snapGuide->enabledSnapStrategies() & KSnapGuide::GuideLineSnapping)
        widget.lineGuideSnapGuide->setCheckState(Qt::Checked);
    else
        widget.lineGuideSnapGuide->setCheckState(Qt::Unchecked);

    widget.snapDistance->setValue(m_snapGuide->snapDistance());
}

void SnapGuideConfigWidget::showEvent(QShowEvent * event)
{
    Q_UNUSED(event);
    updateControls();
}

#include <SnapGuideConfigWidget_p.moc>
