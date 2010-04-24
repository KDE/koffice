/*
 *  Copyright (c) 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
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

#include "KoCompositeOpsBenchmark.h"
#include "../compositeops/KoCompositeOpAlphaDarken.h"
#include "../compositeops/KoCompositeOpOver.h"

#include <KoColorSpaceTraits.h>
#include <KoCompositeOpAdd.h>
#include <KoCompositeOpBurn.h>
#include <KoCompositeOpDivide.h>
#include <KoCompositeOpDodge.h>
#include <KoCompositeOpInversedSubtract.h>
#include <KoCompositeOpMultiply.h>
#include <KoCompositeOpOverlay.h>
#include <KoCompositeOpScreen.h>
#include <KoCompositeOpSubtract.h>

const int TILE_WIDTH = 64;
const int TILE_HEIGHT = 64;

const int IMG_WIDTH = 4096;
const int IMG_HEIGHT = 4096;

const quint8 OPACITY_HALF = 128;

const int TILES_IN_WIDTH = IMG_WIDTH / TILE_WIDTH;
const int TILES_IN_HEIGHT = IMG_HEIGHT / TILE_HEIGHT;


#define COMPOSITE_BENCHMARK \
        for (int y = 0; y < TILES_IN_HEIGHT; y++){                                              \
            for (int x = 0; x < TILES_IN_WIDTH; x++){                                           \
                compositeOp.composite(m_dstBuffer, TILE_WIDTH * KoRgbU16Traits::pixelSize,      \
                                      m_srcBuffer, TILE_WIDTH * KoRgbU16Traits::pixelSize,      \
                                      0, 0,                                                            \
                                      TILE_WIDTH, TILE_HEIGHT,                                         \
                                      OPACITY_HALF);                                                   \
            }                                                                                   \
        }

void KoCompositeOpsBenchmark::initTestCase()
{
    m_dstBuffer = new quint8[ TILE_WIDTH * TILE_HEIGHT * KoRgbU16Traits::pixelSize ];
    m_srcBuffer = new quint8[ TILE_WIDTH * TILE_HEIGHT * KoRgbU16Traits::pixelSize ];
}

// this is called before every benchmark
void KoCompositeOpsBenchmark::init()
{
    memset(m_dstBuffer, 42 , TILE_WIDTH * TILE_HEIGHT * KoRgbU16Traits::pixelSize);
    memset(m_srcBuffer, 42 , TILE_WIDTH * TILE_HEIGHT * KoRgbU16Traits::pixelSize);
}


void KoCompositeOpsBenchmark::cleanupTestCase()
{
    delete [] m_dstBuffer;
    delete [] m_srcBuffer;
}

void KoCompositeOpsBenchmark::benchmarkCompositeOver()
{
    KoCompositeOpOver<KoRgbU16Traits> compositeOp(0);
    QBENCHMARK{
        COMPOSITE_BENCHMARK
    }
}

// void KoCompositeOpsBenchmark::benchmarkCompositeAdd()
// {
//     KoCompositeOpAdd<KoRgbU16Traits> compositeOp(0);
//     QBENCHMARK{
//         COMPOSITE_BENCHMARK
//     }
// }    
// 
// void KoCompositeOpsBenchmark::benchmarkCompositeAlphaDarken()
// {
//     KoCompositeOpAlphaDarken<KoRgbU16Traits> compositeOp(0);
//     QBENCHMARK{
//         COMPOSITE_BENCHMARK
//     }
// }
// 
// void KoCompositeOpsBenchmark::benchmarkCompositeBurn()
// {
//     KoCompositeOpBurn<KoRgbU16Traits> compositeOp(0);
//     QBENCHMARK{
//         COMPOSITE_BENCHMARK
//     }
// }
// 
// void KoCompositeOpsBenchmark::benchmarkCompositeDivide()
// {
//     KoCompositeOpDivide<KoRgbU16Traits> compositeOp(0);
//     QBENCHMARK{
//         COMPOSITE_BENCHMARK
//     }
// }
// 
// void KoCompositeOpsBenchmark::benchmarkCompositeDodge()
// {
//     KoCompositeOpDodge<KoRgbU16Traits> compositeOp(0);
//     QBENCHMARK{
//         COMPOSITE_BENCHMARK
//     }
// }
// 
// void KoCompositeOpsBenchmark::benchmarkCompositeInversedSubtract()
// {
//     KoCompositeOpInversedSubtract<KoRgbU16Traits> compositeOp(0);
//     QBENCHMARK{
//         COMPOSITE_BENCHMARK
//     }
// }
// 
// void KoCompositeOpsBenchmark::benchmarkCompositeMulitply()
// {
//     KoCompositeOpMultiply<KoRgbU16Traits> compositeOp(0);
//     QBENCHMARK{
//         COMPOSITE_BENCHMARK
//     }
// }
// 
// void KoCompositeOpsBenchmark::benchmarkCompositeOverlay()
// {
//     KoCompositeOpOverlay<KoRgbU16Traits> compositeOp(0);
//     QBENCHMARK{
//         COMPOSITE_BENCHMARK
//     }
// }
// 
// void KoCompositeOpsBenchmark::benchmarkCompositeScreen()
// {
//     KoCompositeOpScreen<KoRgbU16Traits> compositeOp(0);
//     QBENCHMARK{
//         COMPOSITE_BENCHMARK
//     }
// }
// 
// void KoCompositeOpsBenchmark::benchmarkCompositeSubtract()
// {
//     KoCompositeOpSubtract<KoRgbU16Traits> compositeOp(0);
//     QBENCHMARK{
//         COMPOSITE_BENCHMARK
//     }
// }

QTEST_KDEMAIN(KoCompositeOpsBenchmark, NoGUI)
#include "KoCompositeOpsBenchmark.moc"

