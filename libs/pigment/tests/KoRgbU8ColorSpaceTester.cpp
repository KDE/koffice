/*
 *  Copyright (c) 2005 Adrian Page <adrian@pagenet.plus.com>
 *  Copyright (c) 2008 Bart Coppens <kde@bartcoppens.be>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include <qtest_kde.h>
#include <kdebug.h>
#include <string.h>

#include "KoColorModelStandardIds.h"

#include "KoColor.h"
#include "KoColorSpace.h"
#include "KoColorSpaceRegistry.h"
#include "KoCompositeOp.h"


#include "KoRgbU8ColorSpaceTester.h"


#define NUM_CHANNELS 4

#define RED_CHANNEL 0
#define GREEN_CHANNEL 1
#define BLUE_CHANNEL 2
#define ALPHA_CHANNEL 3

void KoRgbColorSpaceTester::testBasics()
{
#if 0
    KoColorProfile *defProfile = new KoColorProfile(cmsCreate_sRGBProfile());  
    KisRgbColorSpace *cs = new KisRgbColorSpace(defProfile);

    quint8 pixel[NUM_CHANNELS];

    pixel[PIXEL_RED] = 255;
    pixel[PIXEL_GREEN] = 128;
    pixel[PIXEL_BLUE] = 64;
    pixel[PIXEL_ALPHA] = 0;

    QString valueText = cs->channelValueText(pixel, RED_CHANNEL);
    CHECK(valueText, QString("255"));

    valueText = cs->channelValueText(pixel, GREEN_CHANNEL);
    CHECK(valueText, QString("128"));

    valueText = cs->channelValueText(pixel, BLUE_CHANNEL);
    CHECK(valueText, QString("64"));

    valueText = cs->channelValueText(pixel, ALPHA_CHANNEL);
    CHECK(valueText, QString("0"));

    valueText = cs->normalisedChannelValueText(pixel, RED_CHANNEL);
    CHECK(valueText, QString().setNum(1.0));

    valueText = cs->normalisedChannelValueText(pixel, GREEN_CHANNEL);
    CHECK(valueText, QString().setNum(128.0 / 255.0));

    valueText = cs->normalisedChannelValueText(pixel, BLUE_CHANNEL);
    CHECK(valueText, QString().setNum(64.0 / 255.0));

    valueText = cs->normalisedChannelValueText(pixel, ALPHA_CHANNEL);
    CHECK(valueText, QString().setNum(0.0));

    cs->setPixel(pixel, 128, 192, 64, 99);
    CHECK((uint)pixel[PIXEL_RED], 128u);
    CHECK((uint)pixel[PIXEL_GREEN], 192u);
    CHECK((uint)pixel[PIXEL_BLUE], 64u);
    CHECK((uint)pixel[PIXEL_ALPHA], 99u);

    quint8 red;
    quint8 green;
    quint8 blue;
    quint8 alpha;

    cs->getPixel(pixel, &red, &green, &blue, &alpha);
    CHECK((uint)red, 128u);
    CHECK((uint)green, 192u);
    CHECK((uint)blue, 64u);
    CHECK((uint)alpha, 99u);
#endif
}

#define PIXEL_RED 0
#define PIXEL_GREEN 1
#define PIXEL_BLUE 2
#define PIXEL_ALPHA 3

void KoRgbColorSpaceTester::testMixColors()
{
    const KoColorSpace* cs = KoColorSpaceRegistry::instance()->colorSpace(
                KoColorSpaceRegistry::instance()->colorSpaceId( RGBAColorModelID.id(), Integer8BitsColorDepthID.id() ) , "");
    KoMixColorsOp * mixOp = cs->mixColorsOp();

    // Test mixColors.
    quint8 pixel1[4];
    quint8 pixel2[4];
    quint8 outputPixel[4];

    pixel1[PIXEL_RED] = 255;
    pixel1[PIXEL_GREEN] = 255;
    pixel1[PIXEL_BLUE] = 255;
    pixel1[PIXEL_ALPHA] = 255;

    pixel2[PIXEL_RED] = 0;
    pixel2[PIXEL_GREEN] = 0;
    pixel2[PIXEL_BLUE] = 0;
    pixel2[PIXEL_ALPHA] = 0;

    const quint8 *pixelPtrs[2];
    qint16 weights[2];

    pixelPtrs[0] = pixel1;
    pixelPtrs[1] = pixel2;

    weights[0] = 255;
    weights[1] = 0;

    mixOp->mixColors(pixelPtrs, weights, 2, outputPixel);

    QVERIFY((int)outputPixel[PIXEL_RED] == 255);
    QVERIFY((int)outputPixel[PIXEL_GREEN] == 255);
    QVERIFY((int)outputPixel[PIXEL_BLUE] == 255);
    QVERIFY((int)outputPixel[PIXEL_ALPHA] == 255);

    weights[0] = 0;
    weights[1] = 255;

    mixOp->mixColors(pixelPtrs, weights, 2, outputPixel);

    QVERIFY((int)outputPixel[PIXEL_RED] == 0);
    QVERIFY((int)outputPixel[PIXEL_GREEN] == 0);
    QVERIFY((int)outputPixel[PIXEL_BLUE] == 0);
    QVERIFY((int)outputPixel[PIXEL_ALPHA] == 0);

    weights[0] = 128;
    weights[1] = 127;

    mixOp->mixColors(pixelPtrs, weights, 2, outputPixel);
     
    QVERIFY((int)outputPixel[PIXEL_RED] == 255);
    QVERIFY((int)outputPixel[PIXEL_GREEN] == 255);
    QVERIFY((int)outputPixel[PIXEL_BLUE] == 255);
    QVERIFY((int)outputPixel[PIXEL_ALPHA] == 128);

    pixel1[PIXEL_RED] = 200;
    pixel1[PIXEL_GREEN] = 100;
    pixel1[PIXEL_BLUE] = 50;
    pixel1[PIXEL_ALPHA] = 255;

    pixel2[PIXEL_RED] = 100;
    pixel2[PIXEL_GREEN] = 200;
    pixel2[PIXEL_BLUE] = 20;
    pixel2[PIXEL_ALPHA] = 255;

    mixOp->mixColors(pixelPtrs, weights, 2, outputPixel);

    QVERIFY((int)outputPixel[PIXEL_RED] == 150);
    QCOMPARE((int)outputPixel[PIXEL_GREEN], 149);
    QVERIFY((int)outputPixel[PIXEL_BLUE] == 35);
    QVERIFY((int)outputPixel[PIXEL_ALPHA] == 255);

    pixel1[PIXEL_RED] = 0;
    pixel1[PIXEL_GREEN] = 0;
    pixel1[PIXEL_BLUE] = 0;
    pixel1[PIXEL_ALPHA] = 0;

    pixel2[PIXEL_RED] = 255;
    pixel2[PIXEL_GREEN] = 255;
    pixel2[PIXEL_BLUE] = 255;
    pixel2[PIXEL_ALPHA] = 254;

    weights[0] = 89;
    weights[1] = 166;

    mixOp->mixColors(pixelPtrs, weights, 2, outputPixel);

    QVERIFY((int)outputPixel[PIXEL_RED] == 255);
    QVERIFY((int)outputPixel[PIXEL_GREEN] == 255);
    QVERIFY((int)outputPixel[PIXEL_BLUE] == 255);
    QVERIFY((int)outputPixel[PIXEL_ALPHA] == 165);
}

void KoRgbColorSpaceTester::testCompositeOps()
{
    // Just COMPOSITE_COPY for now

    QList<KoID> depthIDs = KoColorSpaceRegistry::instance()->colorDepthList(RGBAColorModelID.id(),
            KoColorSpaceRegistry::AllColorSpaces);

    foreach( KoID depthId, depthIDs)
    {
	kDebug() << depthId.id();
        const KoColorSpace* cs = KoColorSpaceRegistry::instance()->colorSpace(
                KoColorSpaceRegistry::instance()->colorSpaceId( RGBAColorModelID.id(), depthId.id() ) , "");
        const KoCompositeOp* copyOp = cs->compositeOp(COMPOSITE_COPY);
        KoColor src(cs), dst(cs);

        QColor red(255, 0, 0);
        QColor blue(0, 0, 255);
        QColor transparentRed(255, 0, 0, 0);

        // Copying a color over another color should replace the original color
        src.fromQColor(red);
        dst.fromQColor(blue);

        QVERIFY(memcmp(dst.data(), src.data(), cs->pixelSize()) != 0);

        copyOp->composite(dst.data(), cs->pixelSize(), src.data(), cs->pixelSize(),
                          0, 0, 1, 1, OPACITY_OPAQUE);

        src.fromQColor(red);
        QVERIFY(memcmp(dst.data(), src.data(), cs->pixelSize()) == 0);

        // Copying something transparent over something non-transparent should, of course, make the dst transparent
        src.fromQColor(transparentRed);
        dst.fromQColor(blue);

        QVERIFY(memcmp(dst.data(), src.data(), cs->pixelSize()) != 0);

        copyOp->composite(dst.data(), cs->pixelSize(), src.data(), cs->pixelSize(),
                          0, 0, 1, 1, OPACITY_OPAQUE);

        src.fromQColor(transparentRed);
        QVERIFY(memcmp(dst.data(), src.data(), cs->pixelSize()) == 0);

        // Copying something solid over something transparent
        src.fromQColor(blue);
        dst.fromQColor(transparentRed);

        QVERIFY(memcmp(dst.data(), src.data(), cs->pixelSize()) != 0);

        copyOp->composite(dst.data(), cs->pixelSize(), src.data(), cs->pixelSize(),
                          0, 0, 1, 1, OPACITY_OPAQUE);

        src.fromQColor(blue);
        QVERIFY(memcmp(dst.data(), src.data(), cs->pixelSize()) == 0);

    }

}

QTEST_KDEMAIN(KoRgbColorSpaceTester, NoGUI)
#include "KoRgbU8ColorSpaceTester.moc"

