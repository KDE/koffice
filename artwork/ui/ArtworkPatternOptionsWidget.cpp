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

#include "ArtworkPatternOptionsWidget.h"
#include "ui_ArtworkPatternOptionsWidget.h"

class ArtworkPatternOptionsWidget::Private {
public:
    Ui_PatternOptionsWidget widget;
};

ArtworkPatternOptionsWidget::ArtworkPatternOptionsWidget(QWidget * parent)
        : QWidget(parent), d(new Private())
{
    d->widget.setupUi(this);

    d->widget.patternRepeat->insertItem(0, i18nc("Original", "Original Image"));
    d->widget.patternRepeat->insertItem(1, i18nc("Tiled", "Tiled Image"));
    d->widget.patternRepeat->insertItem(2, i18nc("Stretched", "Stretched Image"));

    d->widget.referencePoint->insertItem(0, i18nc("Top Left", "Align to top-left corner"));
    d->widget.referencePoint->insertItem(1, i18nc("Top", "Align to top side"));
    d->widget.referencePoint->insertItem(2, i18nc("Top Right", "Align to top-right corner"));
    d->widget.referencePoint->insertItem(3, i18nc("Left", "Align to left side"));
    d->widget.referencePoint->insertItem(4, i18nc("Center", "Align in the center"));
    d->widget.referencePoint->insertItem(5, i18nc("Right", "Align to right side"));
    d->widget.referencePoint->insertItem(6, i18nc("Bottom Left", "Align to bottom-left corner"));
    d->widget.referencePoint->insertItem(7, i18nc("Bottom", "Align to bottom side"));
    d->widget.referencePoint->insertItem(8, i18nc("Bottom Right", "Align to bottom-right corner"));

    d->widget.refPointOffsetX->setRange(0.0, 100.0);
    d->widget.refPointOffsetX->setSuffix(QString('%'));
    d->widget.refPointOffsetY->setRange(0.0, 100.0);
    d->widget.refPointOffsetY->setSuffix(QString('%'));
    d->widget.tileOffsetX->setRange(0.0, 100.0);
    d->widget.tileOffsetX->setSuffix(QString('%'));
    d->widget.tileOffsetY->setRange(0.0, 100.0);
    d->widget.tileOffsetY->setSuffix(QString('%'));
    d->widget.patternWidth->setRange(1, 10000);
    d->widget.patternHeight->setRange(1, 10000);

    connect(d->widget.patternRepeat, SIGNAL(activated(int)), this, SIGNAL(patternChanged()));
    connect(d->widget.patternRepeat, SIGNAL(activated(int)), this, SLOT(updateControls()));
    connect(d->widget.referencePoint, SIGNAL(activated(int)), this, SIGNAL(patternChanged()));
    connect(d->widget.refPointOffsetX, SIGNAL(valueChanged(double)), this, SIGNAL(patternChanged()));
    connect(d->widget.refPointOffsetY, SIGNAL(valueChanged(double)), this, SIGNAL(patternChanged()));
    connect(d->widget.tileOffsetX, SIGNAL(valueChanged(double)), this, SIGNAL(patternChanged()));
    connect(d->widget.tileOffsetY, SIGNAL(valueChanged(double)), this, SIGNAL(patternChanged()));
    connect(d->widget.patternWidth, SIGNAL(valueChanged(int)), this, SIGNAL(patternChanged()));
    connect(d->widget.patternHeight, SIGNAL(valueChanged(int)), this, SIGNAL(patternChanged()));
}

ArtworkPatternOptionsWidget::~ArtworkPatternOptionsWidget()
{
    delete d;
}

void ArtworkPatternOptionsWidget::setRepeat(KPatternBackground::PatternRepeat repeat)
{
    d->widget.patternRepeat->blockSignals(true);
    d->widget.patternRepeat->setCurrentIndex(repeat);
    d->widget.patternRepeat->blockSignals(false);
    updateControls();
}

KPatternBackground::PatternRepeat ArtworkPatternOptionsWidget::repeat() const
{
    return static_cast<KPatternBackground::PatternRepeat>(d->widget.patternRepeat->currentIndex());
}

KPatternBackground::ReferencePoint ArtworkPatternOptionsWidget::referencePoint() const
{
    return static_cast<KPatternBackground::ReferencePoint>(d->widget.referencePoint->currentIndex());
}

void ArtworkPatternOptionsWidget::setReferencePoint(KPatternBackground::ReferencePoint referencePoint)
{
    d->widget.referencePoint->blockSignals(true);
    d->widget.referencePoint->setCurrentIndex(referencePoint);
    d->widget.referencePoint->blockSignals(false);
}

QPointF ArtworkPatternOptionsWidget::referencePointOffset() const
{
    return QPointF(d->widget.refPointOffsetX->value(), d->widget.refPointOffsetY->value());
}

void ArtworkPatternOptionsWidget::setReferencePointOffset(const QPointF &offset)
{
    d->widget.refPointOffsetX->blockSignals(true);
    d->widget.refPointOffsetY->blockSignals(true);
    d->widget.refPointOffsetX->setValue(offset.x());
    d->widget.refPointOffsetY->setValue(offset.y());
    d->widget.refPointOffsetX->blockSignals(false);
    d->widget.refPointOffsetY->blockSignals(false);
}

QPointF ArtworkPatternOptionsWidget::tileRepeatOffset() const
{
    return QPointF(d->widget.tileOffsetX->value(), d->widget.tileOffsetY->value());
}

void ArtworkPatternOptionsWidget::setTileRepeatOffset(const QPointF &offset)
{
    d->widget.tileOffsetX->blockSignals(true);
    d->widget.tileOffsetY->blockSignals(true);
    d->widget.tileOffsetX->setValue(offset.x());
    d->widget.tileOffsetY->setValue(offset.y());
    d->widget.tileOffsetX->blockSignals(false);
    d->widget.tileOffsetY->blockSignals(false);
}

QSize ArtworkPatternOptionsWidget::patternSize() const
{
    return QSize(d->widget.patternWidth->value(), d->widget.patternHeight->value());
}

void ArtworkPatternOptionsWidget::setPatternSize(const QSize &size)
{
    d->widget.patternWidth->blockSignals(true);
    d->widget.patternHeight->blockSignals(true);
    d->widget.patternWidth->setValue(size.width());
    d->widget.patternHeight->setValue(size.height());
    d->widget.patternWidth->blockSignals(false);
    d->widget.patternHeight->blockSignals(false);
}

void ArtworkPatternOptionsWidget::updateControls()
{
    bool stretch = d->widget.patternRepeat->currentIndex() == KPatternBackground::Stretched;
    d->widget.patternWidth->setEnabled(! stretch);
    d->widget.patternHeight->setEnabled(! stretch);

    bool tiled = d->widget.patternRepeat->currentIndex() == KPatternBackground::Tiled;
    d->widget.referencePoint->setEnabled(tiled);
    d->widget.refPointOffsetX->setEnabled(tiled);
    d->widget.refPointOffsetY->setEnabled(tiled);
    d->widget.tileOffsetX->setEnabled(tiled);
    d->widget.tileOffsetY->setEnabled(tiled);
}

#include "ArtworkPatternOptionsWidget.moc"
