/* This file is part of the KDE project
   Copyright (C) 2006 Sven Langkamp <sven.langkamp@gmail.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/
#include "KoColorSlider.h"
#include "KoColorSpace.h"

#include <QColor>
#include <QPainter>

KoColorSlider::KoColorSlider(QWidget* parent)
  : KSelector(parent)
{
    setMaximum(255);
}

KoColorSlider::KoColorSlider(Qt::Orientation o, QWidget *parent)
  : KSelector(o, parent)
{
    setMaximum(255);
}

KoColorSlider::~KoColorSlider()
{
}

void KoColorSlider::setColors(const KoColor& mincolor, const KoColor& maxcolor)
{
  m_minColor = mincolor;
  m_maxColor = maxcolor;

  update();
}

void KoColorSlider::drawContents( QPainter *painter )
{
  QPixmap checker(8, 8);
  QPainter p(&checker);
  p.fillRect(0, 0, 4, 4, Qt::lightGray);
  p.fillRect(4, 0, 4, 4, Qt::darkGray);
  p.fillRect(0, 4, 4, 4, Qt::darkGray);
  p.fillRect(4, 4, 4, 4, Qt::lightGray);
  p.end();
  QRect contentsRect_(contentsRect());
  painter->fillRect(contentsRect_, QBrush(checker));

  KoColor c = m_minColor; // smart way to fetch colorspace
  QColor color;

  const quint8 *colors[2];
  colors[0] = m_minColor.data();
  colors[1] = m_maxColor.data();

  KoMixColorsOp * mixOp = c.colorSpace()->mixColorsOp();

  QImage image(contentsRect_.width(), contentsRect_.height(), QImage::Format_ARGB32 );

  if( orientation() == Qt::Horizontal ) {
    for (int x = 0; x < contentsRect_.width(); x++) {

        qreal t = static_cast<qreal>(x) / (contentsRect_.width() - 1);

        qint16 colorWeights[2];
        colorWeights[0] = static_cast<quint8>((1.0 - t) * 255 + 0.5);
        colorWeights[1] = 255 - colorWeights[0];

        mixOp->mixColors(colors, colorWeights, 2, c.data());

        c.toQColor(&color);

        for (int y = 0; y < contentsRect_.height(); y++)
          image.setPixel(x, y, color.rgba());
    }
  }
  else {
    for (int y = 0; y < contentsRect_.height(); y++) {

        qreal t = static_cast<qreal>(y) / (contentsRect_.height() - 1);

        qint16 colorWeights[2];
        colorWeights[0] = static_cast<quint8>((t) * 255 + 0.5);
        colorWeights[1] = 255 - colorWeights[0];

        mixOp->mixColors(colors, colorWeights, 2, c.data());

        c.toQColor(&color);

        for (int x = 0; x < contentsRect_.width(); x++)
          image.setPixel(x, y, color.rgba());
    }
  }
  painter->drawImage( contentsRect_, image, QRect( 0, 0, image.width(), image.height()) );
}

#include "KoColorSlider.moc"
