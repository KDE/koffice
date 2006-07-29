/*
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *
 *  this program is free software; you can redistribute it and/or modify
 *  it under the terms of the gnu general public license as published by
 *  the free software foundation; either version 2 of the license, or
 *  (at your option) any later version.
 *
 *  this program is distributed in the hope that it will be useful,
 *  but without any warranty; without even the implied warranty of
 *  merchantability or fitness for a particular purpose.  see the
 *  gnu general public license for more details.
 *
 *  you should have received a copy of the gnu general public license
 *  along with this program; if not, write to the free software
 *  foundation, inc., 675 mass ave, cambridge, ma 02139, usa.
 */
#include "kis_global.h"
#include "kis_background.h"
#include "kis_integer_maths.h"

KisBackground::KisBackground()
    : KShared()
{
    m_patternTile = QImage(PATTERN_WIDTH, PATTERN_HEIGHT, 32);
    m_patternTile.setAlphaBuffer(false);

    for (int y = 0; y < PATTERN_HEIGHT; y++)
    {
        for (int x = 0; x < PATTERN_WIDTH; x++)
        {
            Q_UINT8 v = 128 + 63 * ((x / 16 + y / 16) % 2);
            m_patternTile.setPixel(x, y, qRgb(v, v, v));
        }
    }
}

KisBackground::~KisBackground()
{
}

const QImage& KisBackground::patternTile() const
{
    return m_patternTile;
}

void KisBackground::paintBackground(QImage image, int imageLeftX, int imageTopY)
{
    int patternLeftX;

    if (imageLeftX >= 0) {
        patternLeftX = imageLeftX % PATTERN_WIDTH;
    } else {
        patternLeftX = (PATTERN_WIDTH - (-imageLeftX % PATTERN_WIDTH)) % PATTERN_WIDTH;
    }

    int patternTopY;

    if (imageTopY >= 0) {
        patternTopY = imageTopY % PATTERN_HEIGHT;
    } else {
        patternTopY = (PATTERN_HEIGHT - (-imageTopY % PATTERN_HEIGHT)) % PATTERN_HEIGHT;
    }

    int imageWidth = image.width();
    int imageHeight = image.height();

    int patternY = patternTopY;

    for (int y = 0; y < imageHeight; y++)
    {
        QRgb *imagePixelPtr = reinterpret_cast<QRgb *>(image.scanLine(y));
        const QRgb *patternScanLine = reinterpret_cast<const QRgb *>(m_patternTile.scanLine(patternY));
        int patternX = patternLeftX;

        for (int x = 0; x < imageWidth; x++)
        {
            QRgb imagePixel = *imagePixelPtr;
            Q_UINT8 imagePixelAlpha = qAlpha(imagePixel);

            if (imagePixelAlpha != 255) {

                QRgb patternPixel = patternScanLine[patternX];
                Q_UINT8 imageRed = UINT8_BLEND(qRed(imagePixel), qRed(patternPixel), imagePixelAlpha);
                Q_UINT8 imageGreen = UINT8_BLEND(qGreen(imagePixel), qGreen(patternPixel), imagePixelAlpha);
                Q_UINT8 imageBlue = UINT8_BLEND(qBlue(imagePixel), qBlue(patternPixel), imagePixelAlpha);

                *imagePixelPtr = qRgba(imageRed, imageGreen, imageBlue, 255);
            }

            ++imagePixelPtr;
            ++patternX;

            if (patternX == PATTERN_WIDTH) {
                patternX = 0;
            }
        }

        ++patternY;

        if (patternY == PATTERN_HEIGHT) {
            patternY = 0;
        }
    }
}

void KisBackground::paintBackground(QImage img, const QRect& scaledImageRect, const QSize& scaledImageSize, const QSize& imageSize)
{
    if (scaledImageRect.isEmpty() || scaledImageSize.isEmpty() || imageSize.isEmpty()) {
        return;
    }

    Q_ASSERT(img.size() == scaledImageRect.size());

    if (img.size() != scaledImageRect.size()) {
        return;
    }

    Q_INT32 imageWidth = imageSize.width();
    Q_INT32 imageHeight = imageSize.height();

    for (Q_INT32 y = 0; y < scaledImageRect.height(); ++y) {

        Q_INT32 scaledY = scaledImageRect.y() + y;
        Q_INT32 srcY = (scaledY * imageHeight) / scaledImageSize.height();
        Q_INT32 patternY = srcY % PATTERN_HEIGHT;

        QRgb *imagePixelPtr = reinterpret_cast<QRgb *>(img.scanLine(y));
        const QRgb *patternScanLine = reinterpret_cast<const QRgb *>(m_patternTile.scanLine(patternY));

        for (Q_INT32 x = 0; x < scaledImageRect.width(); ++x) {

            QRgb imagePixel = *imagePixelPtr;
            Q_UINT8 imagePixelAlpha = qAlpha(imagePixel);

            if (imagePixelAlpha != 255) {

                Q_INT32 scaledX = scaledImageRect.x() + x;
                Q_INT32 srcX = (scaledX * imageWidth) / scaledImageSize.width();
                Q_INT32 patternX = srcX % PATTERN_WIDTH;

                QRgb patternPixel = patternScanLine[patternX];
                Q_UINT8 imageRed = UINT8_BLEND(qRed(imagePixel), qRed(patternPixel), imagePixelAlpha);
                Q_UINT8 imageGreen = UINT8_BLEND(qGreen(imagePixel), qGreen(patternPixel), imagePixelAlpha);
                Q_UINT8 imageBlue = UINT8_BLEND(qBlue(imagePixel), qBlue(patternPixel), imagePixelAlpha);

                *imagePixelPtr = qRgba(imageRed, imageGreen, imageBlue, 255);
            }

            ++imagePixelPtr;
        }
    }
}


