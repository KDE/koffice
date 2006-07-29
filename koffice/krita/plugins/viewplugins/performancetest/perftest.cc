/*
 * perftest.cc -- Part of Krita
 *
 * Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
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

#include <math.h>

#include <stdlib.h>

#include <qslider.h>
#include <qpoint.h>
#include <qradiobutton.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qtextedit.h>
#include <qdatetime.h>

#include <klocale.h>
#include <kiconloader.h>
#include <kinstance.h>
#include <kdialogbase.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <ktempfile.h>
#include <kdebug.h>
#include <kgenericfactory.h>
#include <knuminput.h>

#include <qcolor.h>

#include "kis_meta_registry.h"
#include <kis_resourceserver.h>
#include "kis_cursor.h"
#include <kis_doc.h>
#include <kis_config.h>
#include <kis_image.h>
#include <kis_layer.h>
#include <kis_global.h>
#include <kis_types.h>
#include <kis_view.h>
#include <kis_selection.h>
#include <kis_colorspace_factory_registry.h>
#include <kis_colorspace.h>
#include <kis_painter.h>
#include <kis_fill_painter.h>
#include <kis_id.h>
#include <kis_paint_device.h>
#include <kis_iterators_pixel.h>
#include "perftest.h"
#include "kis_filter_config_widget.h"
#include "kis_factory.h"

#include "dlg_perftest.h"
#include "wdg_perftest.h"

#define USE_CALLGRIND 0

#if USE_CALLGRIND
#include <valgrind/callgrind.h>
#endif


typedef KGenericFactory<PerfTest> PerfTestFactory;
K_EXPORT_COMPONENT_FACTORY( kritaperftest, PerfTestFactory( "krita" ) )

PerfTest::PerfTest(QObject *parent, const char *name, const QStringList &)
    : KParts::Plugin(parent, name)
{
    if ( parent->inherits("KisView") )
    {
        setInstance(PerfTestFactory::instance());
        setXMLFile(locate("data","kritaplugins/perftest.rc"), true);

        (void) new KAction(i18n("&Performance Test..."), 0, 0, this, SLOT(slotPerfTest()), actionCollection(), "perf_test");

        m_view = (KisView*) parent;
    }
}

PerfTest::~PerfTest()
{
    m_view = 0;
}

void PerfTest::slotPerfTest()
{
    KisImageSP image = m_view->canvasSubject()->currentImg();

    if (!image) return;

    DlgPerfTest * dlgPerfTest = new DlgPerfTest(m_view, "PerfTest");
    Q_CHECK_PTR(dlgPerfTest);

    dlgPerfTest->setCaption(i18n("Performance Test"));

    QString report = QString("");

        if (dlgPerfTest->exec() == QDialog::Accepted) {

        Q_INT32 testCount = (Q_INT32)qRound(dlgPerfTest->page()->intTestCount->value());

        if (dlgPerfTest->page()->chkBitBlt->isChecked()) {
            kdDebug() << "bltTest:\n";
            QString s = bltTest(testCount);
            report = report.append(s);
            kdDebug() << s << "\n";
        }
        if (dlgPerfTest->page()->chkFill->isChecked()) {
            kdDebug() << "Filltest\n";
            QString s= fillTest(testCount);
            report = report.append(s);
            kdDebug() << s << "\n";
        }
        if (dlgPerfTest->page()->chkGradient->isChecked()) {
            kdDebug() << "Gradienttest\n";
            QString s = gradientTest(testCount);
            report = report.append(s);
            kdDebug() << s << "\n";
        }
        if (dlgPerfTest->page()->chkPixel->isChecked()) {
            kdDebug() << "Pixeltest\n";
            QString s = pixelTest(testCount);
            report = report.append(s);
            kdDebug() << s << "\n";
        }
        if (dlgPerfTest->page()->chkShape->isChecked()) {
            kdDebug() << "Shapetest\n";
            QString s = shapeTest(testCount);
            report = report.append(s);
            kdDebug() << s << "\n";
        }
        if (dlgPerfTest->page()->chkLayer->isChecked()) {
            kdDebug() << "LayerTest\n";
            QString s = layerTest(testCount);
            report = report.append(s);
            kdDebug() << s << "\n";
        }
        if (dlgPerfTest->page()->chkScale->isChecked()) {
            kdDebug() << "Scaletest\n";
            QString s = scaleTest(testCount);
            report = report.append(s);
            kdDebug() << s << "\n";
        }
        if (dlgPerfTest->page()->chkRotate->isChecked()) {
            kdDebug() << "Rotatetest\n";
            QString s = rotateTest(testCount);
            report = report.append(s);
            kdDebug() << s << "\n";
        }
        if (dlgPerfTest->page()->chkRender->isChecked()) {
            kdDebug() << "Rendertest\n";
            QString s = renderTest(testCount);
            report = report.append(s);
            kdDebug() << s << "\n";
        }
        if (dlgPerfTest->page()->chkSelection->isChecked()) {
            kdDebug() << "Selectiontest\n";
            QString s = selectionTest(testCount);
            report = report.append(s);
            kdDebug() << s << "\n";
        }
        if (dlgPerfTest->page()->chkColorConversion->isChecked()) {
            kdDebug() << "Colorconversiontest\n";
            QString s = colorConversionTest(testCount);
            report = report.append(s);
            kdDebug() << s << "\n";
        }
        if (dlgPerfTest->page()->chkFilter-> isChecked()) {
            kdDebug() << "filtertest\n";
            QString s = filterTest(testCount);
            report = report.append(s);
            kdDebug() << s << "\n";
        }
        if (dlgPerfTest->page()->chkReadBytes->isChecked()) {
            kdDebug() << "Readbytes test\n";
            QString s = readBytesTest(testCount);
            report = report.append(s);
            kdDebug() << s << "\n";
        }
        if (dlgPerfTest->page()->chkWriteBytes-> isChecked()) {
            kdDebug() << "Writebytes test\n";
            QString s = writeBytesTest(testCount);
            report = report.append(s);
            kdDebug() << s << "\n";
        }
        if (dlgPerfTest->page()->chkIterators->isChecked()) {
            kdDebug() << "Iterators test\n";
            QString s = iteratorTest(testCount);
            report = report.append(s);
            kdDebug() << s << "\n";
        }
        if (dlgPerfTest->page()->chkPaintView->isChecked()) {
            kdDebug() << "paintview test\n";
            QString s = paintViewTest(testCount);
            report = report.append(s);
            kdDebug() << s << "\n";
        }
        if (dlgPerfTest->page()->chkPaintViewFPS->isChecked()) {
            kdDebug() << "paint current view (fps) test\n";
            QString s = paintViewFPSTest();
            report = report.append(s);
            kdDebug() << s << "\n";
        }
        KDialogBase * d = new KDialogBase(m_view, "", true, "", KDialogBase::Ok);
        Q_CHECK_PTR(d);

        d->setCaption("Performance test results");
        QTextEdit * e = new QTextEdit(d);
        Q_CHECK_PTR(e);
        d->setMainWidget(e);
        e->setText(report);
        e->setMinimumWidth(600);
        e->setMinimumHeight(600);
        d->exec();
        delete d;

    }
        delete dlgPerfTest;
}

QString PerfTest::bltTest(Q_UINT32 testCount)
{
    QString report = QString("* bitBlt test\n");

    KisDoc * doc = m_view->canvasSubject()->document();
    KisIDList l = KisMetaRegistry::instance()->csRegistry()->listKeys();

    for (KisIDList::Iterator it = l.begin(); it != l.end(); ++it) {

        kdDebug() << "Image->" << (*it).name() << "\n";

        report = report.append( "  Testing blitting on " + (*it).name() + "\n");

         KisImageSP img = doc->newImage("blt-" + (*it).name(), 1000, 1000,
                KisMetaRegistry::instance()->csRegistry()->getColorSpace(*it,""));

        report = report.append(doBlit(COMPOSITE_OVER, *it, OPACITY_OPAQUE, testCount, img));
        report = report.append( "\n");
        report = report.append(doBlit(COMPOSITE_OVER, *it, OPACITY_OPAQUE / 2, testCount, img));
        report = report.append( "\n");
        report = report.append(doBlit(COMPOSITE_COPY, *it, OPACITY_OPAQUE, testCount, img));
        report = report.append( "\n");
        report = report.append(doBlit(COMPOSITE_COPY, *it, OPACITY_OPAQUE / 2, testCount, img));
        report = report.append( "\n");
    }

    return report;


}


QString PerfTest::doBlit(const KisCompositeOp& op,
                         KisID cspace,
                         Q_UINT8 opacity,
                         Q_UINT32 testCount,
                         KisImageSP img)
{

    QTime t;
    QString report;

    // ------------------------------------------------------------------------------
    // Small

    KisPaintDeviceSP small = new KisPaintDevice(KisMetaRegistry::instance()->csRegistry()->getColorSpace(cspace,""), "small blit");
    Q_CHECK_PTR(small);

    KisFillPainter pf(small.data()) ;
    pf.fillRect(0, 0, 32, 32, KisColor(Qt::black, KisMetaRegistry::instance()->csRegistry()->getRGB8()));
    pf.end();

    t.restart();
    KisPainter p(img->activeDevice());
    for (Q_UINT32 i = 0; i < testCount; ++i) {
        p.bitBlt(0, 0, op, small.data(),0,0,32, 32);
    }
    p.end();

    report = report.append(QString("   %1 blits of rectangles < tilesize with opacity %2 and composite op %3: %4ms\n")
                   .arg(testCount)
                   .arg(opacity)
                   .arg(op.id().name())
                   .arg(t.elapsed()));


    // ------------------------------------------------------------------------------
    // Medium
    KisPaintDeviceSP medium = new KisPaintDevice(KisMetaRegistry::instance()->csRegistry()->getColorSpace(cspace,""), "medium blit");
    Q_CHECK_PTR(medium);

    pf.begin(medium.data()) ;
    pf.fillRect(0, 0, 64 * 3, 64 * 3, KisColor(Qt::black, KisMetaRegistry::instance()->csRegistry()->getRGB8()));
    pf.end();

    t.restart();
    p.begin(img->activeDevice().data());
    for (Q_UINT32 i = 0; i < testCount; ++i) {
        p.bitBlt(0, 0, op, medium.data(),0,0,96, 96);
    }
    p.end();

    report = report.append(QString("   %1 blits of rectangles 3 * tilesize with opacity %2 and composite op %3: %4ms\n")
                   .arg(testCount)
                   .arg(opacity)
                   .arg(op.id().name())
                   .arg(t.elapsed()));


    // ------------------------------------------------------------------------------
    // Big
    KisPaintDeviceSP big = new KisPaintDevice(KisMetaRegistry::instance()->csRegistry()->getColorSpace(cspace,""), "big blit");
    Q_CHECK_PTR(big);

    pf.begin(big.data()) ;
    pf.fillRect(0, 0, 800, 800, KisColor(Qt::black, KisMetaRegistry::instance()->csRegistry()->getRGB8()));
    pf.end();

    t.restart();
    p.begin(img->activeDevice().data());
    for (Q_UINT32 i = 0; i < testCount; ++i) {
        p.bitBlt(0, 0, op, big.data(),0,0,800,800);

    }
    p.end();
    report = report.append(QString("   %1 blits of rectangles 800 x 800 with opacity %2 and composite op %3: %4ms\n")
                   .arg(testCount)
                   .arg(opacity)
                   .arg(op.id().name())
                   .arg(t.elapsed()));


    // ------------------------------------------------------------------------------
    // Outside

    KisPaintDeviceSP outside = new KisPaintDevice(KisMetaRegistry::instance()->csRegistry()->getColorSpace(cspace,""), "outside blit");
    Q_CHECK_PTR(outside);
    pf.begin(outside.data()) ;
    pf.fillRect(0, 0, 500, 500, KisColor(Qt::black, KisMetaRegistry::instance()->csRegistry()->getRGB8()));
    pf.end();

    t.restart();
    p.begin(img->activeDevice().data());
    for (Q_UINT32 i = 0; i < testCount; ++i) {
        p.bitBlt(600, 600, op, outside.data(),0,0,500,500);

    }
    p.end();
    report = report.append(QString("   %1 blits of rectangles 500 x 500 at 600,600 with opacity %2 and composite op %3: %4ms\n")
                   .arg(testCount)
                   .arg(opacity)
                   .arg(op.id().name())
                   .arg(t.elapsed()));

    // ------------------------------------------------------------------------------
    // Small with varied source opacity

    KisPaintDeviceSP small_with_alpha = new KisPaintDevice(KisMetaRegistry::instance()->csRegistry()->getColorSpace(cspace,""), "small blit with alpha");
    Q_CHECK_PTR(small_with_alpha);

    pf.begin(small_with_alpha.data()) ;
    pf.fillRect(0, 0, 32, 32, KisColor(Qt::black, KisMetaRegistry::instance()->csRegistry()->getRGB8()), OPACITY_TRANSPARENT);
    pf.fillRect(4, 4, 24, 24, KisColor(Qt::black, KisMetaRegistry::instance()->csRegistry()->getRGB8()), OPACITY_OPAQUE / 2);
    pf.fillRect(8, 8, 16, 16, KisColor(Qt::black, KisMetaRegistry::instance()->csRegistry()->getRGB8()), OPACITY_OPAQUE);
    pf.end();

    t.restart();
    p.begin(img->activeDevice().data());
    for (Q_UINT32 i = 0; i < testCount; ++i) {
        p.bitBlt(0, 0, op, small_with_alpha.data(), 0, 0, 32, 32);
    }
    p.end();

    report = report.append(QString("   %1 blits of rectangles < tilesize with source alpha, with opacity %2 and composite op %3: %4ms\n")
                   .arg(testCount)
                   .arg(opacity)
                   .arg(op.id().name())
                   .arg(t.elapsed()));

    return report;

}

QString PerfTest::fillTest(Q_UINT32 testCount)
{
    QString report = QString("* Fill test\n");

    KisDoc * doc = m_view->canvasSubject()->document();
    KisIDList l = KisMetaRegistry::instance()->csRegistry()->listKeys();

    for (KisIDList::Iterator it = l.begin(); it != l.end(); ++it) {
        kdDebug() << "Filltest on " << (*it).name() + "\n";

        report = report.append( "  Testing blitting on " + (*it).name() + "\n");

        KisImageSP img = doc->newImage("fill-" + (*it).name(), 1000, 1000, KisMetaRegistry::instance()->csRegistry()->getColorSpace(*it,""));
        KisPaintDeviceSP l = img->activeDevice();

        // Rect fill
        KisFillPainter p(l.data());
        QTime t;
        t.restart();
        for (Q_UINT32 i = 0; i < testCount; ++i) {
            p.eraseRect(0, 0, 1000, 1000);
        }
        report = report.append(QString("    Erased 1000 x 1000 layer %1 times: %2\n").arg(testCount).arg(t.elapsed()));


        t.restart();
        for (Q_UINT32 i = 0; i < testCount; ++i) {
            p.eraseRect(50, 50, 500, 500);
        }
        report = report.append(QString("    Erased 500 x 500 layer %1 times: %2\n").arg(testCount).arg(t.elapsed()));


        t.restart();
        for (Q_UINT32 i = 0; i < testCount; ++i) {
            p.eraseRect(-50, -50, 1100, 1100);
        }
        report = report.append(QString("    Erased rect bigger than layer %1 times: %2\n").arg(testCount).arg(t.elapsed()));


        // Opaque Rect fill
        t.restart();
        for (Q_UINT32 i = 0; i < testCount; ++i) {
            p.fillRect(0, 0, 1000, 1000, KisColor(Qt::black, KisMetaRegistry::instance()->csRegistry()->getRGB8()));
        }
        report = report.append(QString("    Opaque fill 1000 x 1000 layer %1 times: %2\n").arg(testCount).arg(t.elapsed()));


        t.restart();
        for (Q_UINT32 i = 0; i < testCount; ++i) {
            p.fillRect(50, 50, 500, 500, KisColor(Qt::black, KisMetaRegistry::instance()->csRegistry()->getRGB8()));
        }
        report = report.append(QString("    Opaque fill 500 x 500 layer %1 times: %2\n").arg(testCount).arg(t.elapsed()));


        t.restart();
        for (Q_UINT32 i = 0; i < testCount; ++i) {
            p.fillRect(-50, -50, 1100, 1100, KisColor(Qt::black, KisMetaRegistry::instance()->csRegistry()->getRGB8()));
        }
        report = report.append(QString("    Opaque fill rect bigger than layer %1 times: %2\n").arg(testCount).arg(t.elapsed()));

        // Transparent rect fill

        t.restart();
        for (Q_UINT32 i = 0; i < testCount; ++i) {
            p.fillRect(0, 0, 1000, 1000, KisColor(Qt::black, KisMetaRegistry::instance()->csRegistry()->getRGB8()), OPACITY_OPAQUE / 2);
        }
        report = report.append(QString("    Opaque fill 1000 x 1000 layer %1 times: %2\n").arg(testCount).arg(t.elapsed()));


        t.restart();
        for (Q_UINT32 i = 0; i < testCount; ++i) {
            p.fillRect(50, 50, 500, 500, KisColor(Qt::black, KisMetaRegistry::instance()->csRegistry()->getRGB8()), OPACITY_OPAQUE / 2);
        }
        report = report.append(QString("    Opaque fill 500 x 500 layer %1 times: %2\n").arg(testCount).arg(t.elapsed()));


        t.restart();
        for (Q_UINT32 i = 0; i < testCount; ++i) {
            p.fillRect(-50, -50, 1100, 1100, KisColor(Qt::black, KisMetaRegistry::instance()->csRegistry()->getRGB8()), OPACITY_OPAQUE / 2);
        }
        report = report.append(QString("    Opaque fill rect bigger than layer %1 times: %2\n").arg(testCount).arg(t.elapsed()));

        // Colour fill

        t.restart();
        for (Q_UINT32 i = 0; i < testCount; ++i) {
            p.eraseRect(0, 0, 1000, 1000);
//             p.paintEllipse(500, 1000, 100, 0, 0);
            p.setPaintColor(KisColor(Qt::black, KisMetaRegistry::instance()->csRegistry()->getRGB8()));
            p.setFillThreshold(15);
            p.setCompositeOp(COMPOSITE_OVER);
            p.fillColor(0,0);
        }
        report = report.append(QString("    Opaque floodfill of whole circle (incl. erase and painting of circle) %1 times: %2\n").arg(testCount).arg(t.elapsed()));


        // Pattern fill
        t.restart();
        for (Q_UINT32 i = 0; i < testCount; ++i) {
            p.eraseRect(0, 0, 1000, 1000);
//             p.paintEllipse(500, 1000, 100, 0, 0);
            p.setPaintColor(KisColor(Qt::black, KisMetaRegistry::instance()->csRegistry()->getRGB8()));
            KisResourceServerBase* r = KisResourceServerRegistry::instance()->get("PatternServer");
            Q_CHECK_PTR(r);
            p.setPattern((KisPattern*)r->resources().first());
            p.setFillThreshold(15);
            p.setCompositeOp(COMPOSITE_OVER);
            p.fillPattern(0,0);
        }
        report = report.append(QString("    Opaque patternfill  of whole circle (incl. erase and painting of circle) %1 times: %2\n").arg(testCount).arg(t.elapsed()));



    }



    return report;

}

QString PerfTest::gradientTest(Q_UINT32 testCount)
{
    return QString("Gradient test\n");
}

QString PerfTest::pixelTest(Q_UINT32 testCount)
{
    QString report = QString("* pixel/setpixel test\n");

    KisDoc * doc = m_view->canvasSubject()->document();
    KisIDList l = KisMetaRegistry::instance()->csRegistry()->listKeys();


    for (KisIDList::Iterator it = l.begin(); it != l.end(); ++it) {
        report = report.append( "  Testing pixel/setpixel on " + (*it).name() + "\n");

         KisImageSP img = doc->newImage("fill-" + (*it).name(), 1000, 1000, KisMetaRegistry::instance()->csRegistry()->getColorSpace(*it,""));

        KisPaintDeviceSP l = img->activeDevice();

         QTime t;
         t.restart();

        QColor c = Qt::black;
        Q_UINT8 opacity = OPACITY_OPAQUE;
         for (Q_UINT32 i = 0; i < testCount; ++i) {
            for (Q_UINT32 x = 0; x < 1000; ++x) {
                for (Q_UINT32 y = 0; y < 1000; ++y) {
                    l->pixel(x, y, &c, &opacity);
                }
            }
         }
        report = report.append(QString("    read 1000 x 1000 pixels %1 times: %2\n").arg(testCount).arg(t.elapsed()));

        c= Qt::black;
         t.restart();
         for (Q_UINT32 i = 0; i < testCount; ++i) {
            for (Q_UINT32 x = 0; x < 1000; ++x) {
                for (Q_UINT32 y = 0; y < 1000; ++y) {
                    l->setPixel(x, y, c, 128);
                }
            }
         }
        report = report.append(QString("    written 1000 x 1000 pixels %1 times: %2\n").arg(testCount).arg(t.elapsed()));

    }




    return report;

}

QString PerfTest::shapeTest(Q_UINT32 testCount)
{
    return QString("Shape test\n");
}

QString PerfTest::layerTest(Q_UINT32 testCount)
{
    return QString("Layer test\n");
}

QString PerfTest::scaleTest(Q_UINT32 testCount)
{
    return QString("Scale test\n");
}

QString PerfTest::rotateTest(Q_UINT32 testCount)
{
    QString report = QString("* Rotate test\n");

    KisDoc * doc = m_view->canvasSubject()->document();
    KisIDList l = KisMetaRegistry::instance()->csRegistry()->listKeys();
    for (KisIDList::Iterator it = l.begin(); it != l.end(); ++it) {

        doc->undoAdapter()->setUndo( false );
        QTime t;

        for (uint i = 0; i < testCount; ++i) {
            for (double angle = 0; angle < 360; ++angle) {
                kdDebug() << "Rotating " << (*it).name() << " at " << angle << " degrees\n";
                KisImage * img = doc->newImage("cs-" + (*it).name(), 1000, 1000, KisMetaRegistry::instance()->csRegistry()->getColorSpace(*it,""));
                img->rotate(angle, m_view->canvasSubject()->progressDisplay());
                kdDebug() << "Size: " << img->projection()->extent() << endl;
                delete img;
            }
        }
        report = report.append(QString("    rotated  1000 x 1000 pixels over 360 degrees, degree by degree, %1 times: %2\n").arg(testCount).arg(t.elapsed()));
    }
    return report;
}

QString PerfTest::renderTest(Q_UINT32 restCount)
{
    return QString("Render test\n");
}

QString PerfTest::selectionTest(Q_UINT32 testCount)
{
    return QString("Selection test\n");
}

QString PerfTest::colorConversionTest(Q_UINT32 testCount)
{
    QString report = QString("* Colorspace conversion test\n");

    KisDoc * doc = m_view->canvasSubject()->document();
    KisIDList l = KisMetaRegistry::instance()->csRegistry()->listKeys();
    for (KisIDList::Iterator it = l.begin(); it != l.end(); ++it) {

        KisImage * img = doc->newImage("cs-" + (*it).name(), 1000, 1000, KisMetaRegistry::instance()->csRegistry()->getColorSpace(*it,""));

        QTime t;

        KisIDList l2 = KisMetaRegistry::instance()->csRegistry()->listKeys();
        for (KisIDList::Iterator it2 = l2.begin(); it2 != l2.end(); ++it2) {
            kdDebug() << "test conversion from " << (*it).name() << " to " << (*it2).name() << endl;
            
            t.restart();
            for (uint i = 0; i < testCount; ++i) {
                KisImage * img2 = new KisImage(*img);
                img2->convertTo(KisMetaRegistry::instance()->csRegistry()->getColorSpace(*it2,""));
                delete img2;
            }
            report = report.append(QString("    converted from " + (*it).name() + " to " + (*it2).name() + " 1000 x 1000 pixels %1 times: %2\n").arg(testCount).arg(t.elapsed()));

        }

        delete img;

    }
    return report;

}

QString PerfTest::filterTest(Q_UINT32 testCount)
{

    QString report = QString("* Filter test\n");

    KisIDList filters = KisFilterRegistry::instance()->listKeys();
    KisDoc * doc = m_view->canvasSubject()->document();
    KisIDList l = KisMetaRegistry::instance()->csRegistry()->listKeys();

    for (KisIDList::Iterator it = l.begin(); it != l.end(); ++it) {
        report = report.append( "  Testing filtering on " + (*it).name() + "\n");

        KisImageSP img = doc->newImage("filter-" + (*it).name(), 1000, 1000, KisMetaRegistry::instance()->csRegistry()->getColorSpace(*it,""));
        KisPaintDeviceSP l = img->activeDevice();

        QTime t;

        for (KisIDList::Iterator it = filters.begin(); it != filters.end(); ++it) {
            
            KisFilterSP f = KisFilterRegistry::instance()->get(*it);
            t.restart();
            kdDebug() << "test filter " << f->id().name() << " on " << img->colorSpace()->id().name() << endl;
            for (Q_UINT32 i = 0; i < testCount; ++i) {
                f->enableProgress();
                f->process(l.data(), l.data(), f->configuration(f->createConfigurationWidget(m_view, l.data())), QRect(0, 0, 1000, 1000));
                f->disableProgress();
            }
            report = report.append(QString("    filtered " + (*it).name() + "1000 x 1000 pixels %1 times: %2\n").arg(testCount).arg(t.elapsed()));

        }

    }
    return report;

}

QString PerfTest::readBytesTest(Q_UINT32 testCount)
{
    QString report = QString("* Read bytes test\n\n");

    // On default tiles
    KisDoc * doc = m_view->canvasSubject()->document();
    KisImageSP img = doc->newImage("Readbytes ", 1000, 1000, KisMetaRegistry::instance()->csRegistry()->getColorSpace(KisID("RGBA",""),""));
    KisPaintDeviceSP l = img->activeDevice();

    QTime t;
    t.restart();

    for (Q_UINT32 i = 0; i < testCount; ++i) {
        Q_UINT8 * newData = new Q_UINT8[1000 * 1000 * l->pixelSize()];
        Q_CHECK_PTR(newData);
        l->readBytes(newData, 0, 0, 1000, 1000);
        delete[] newData;
    }

    report = report.append(QString("    read 1000 x 1000 pixels %1 times from empty image: %2\n").arg(testCount).arg(t.elapsed()));

    // On tiles with data

    KisFillPainter p(l.data());
    p.fillRect(0, 0, 1000, 1000, KisColor(Qt::black, KisMetaRegistry::instance()->csRegistry()->getRGB8()));
    p.end();

    t.restart();

    for (Q_UINT32 i = 0; i < testCount; ++i) {
        Q_UINT8 * newData = new Q_UINT8[1000 * 1000 * l->pixelSize()];
        Q_CHECK_PTR(newData);
        l->readBytes(newData, 0, 0, 1000, 1000);
        delete[] newData;
    }

    report = report.append(QString("    read 1000 x 1000 pixels %1 times from filled image: %2\n").arg(testCount).arg(t.elapsed()));

    return report;
}


QString PerfTest::writeBytesTest(Q_UINT32 testCount)
{
    QString report = QString("* Write bytes test");

    // On default tiles
    KisDoc * doc = m_view->canvasSubject()->document();
    KisImageSP img = doc->newImage("Writebytes ", 1000, 1000, KisMetaRegistry::instance()->csRegistry()->getColorSpace(KisID("RGBA", ""),""));
    KisPaintDeviceSP l = img->activeDevice();
    KisFillPainter p(l.data());
    p.fillRect(0, 0, 1000, 1000, KisColor(Qt::black, KisMetaRegistry::instance()->csRegistry()->getRGB8()));
    p.end();


    Q_UINT8 * data = new Q_UINT8[1000 * 1000 * l->pixelSize()];
    Q_CHECK_PTR(data);
    l->readBytes(data, 0, 0, 1000, 1000);

    QTime t;
    t.restart();
    for (Q_UINT32 i = 0; i < testCount; ++i) {
        l->writeBytes(data, 0, 0, 1000, 1000);
    }
    delete[] data;
    report = report.append(QString("    written 1000 x 1000 pixels %1 times: %2\n").arg(testCount).arg(t.elapsed()));
    return report;


}

/////// Iterator tests


QString hlineRODefault(KisDoc * doc, Q_UINT32 testCount)
{
    KisImageSP img = doc->newImage("", 1000, 1000, KisMetaRegistry::instance()->csRegistry()->getColorSpace(KisID("RGBA", ""),""));
    KisPaintDeviceSP l = img->activeDevice();

    QTime t;
    t.restart();


    for (Q_UINT32 i = 0; i < testCount; ++i) {
         int adv;

        for(Q_INT32 y2 = 0; y2 < 0 + 1000; y2++)
        {
            KisHLineIterator hiter = l->createHLineIterator(0, y2, 1000, false);
            while(! hiter.isDone())
            {
                 adv = hiter.nConseqHPixels();
                 hiter += adv;
            }
        }

    }

    return QString("    hline iterated read-only 1000 x 1000 pixels %1 times over default tile: %2\n").arg(testCount).arg(t.elapsed());


}

QString hlineRO(KisDoc * doc, Q_UINT32 testCount)
{
    KisImageSP img = doc->newImage("", 1000, 1000, KisMetaRegistry::instance()->csRegistry()->getColorSpace(KisID("RGBA", ""),""));
    KisPaintDeviceSP l = img->activeDevice();

    KisFillPainter p(l.data());
    p.fillRect(0, 0, 1000, 1000, KisColor(Qt::black, KisMetaRegistry::instance()->csRegistry()->getRGB8()));
    p.end();

    QTime t;
    t.restart();

    for (Q_UINT32 i = 0; i < testCount; ++i) {
         int adv;

        for(Q_INT32 y2 = 0; y2 < 0 + 1000; y2++)
        {
            KisHLineIterator hiter = l->createHLineIterator(0, y2, 1000, false);
            while(! hiter.isDone())
            {
                 adv = hiter.nConseqHPixels();
                 hiter += adv;
            }
        }

    }

    return QString("    hline iterated read-only 1000 x 1000 pixels %1 times over existing tile: %2\n").arg(testCount).arg(t.elapsed());

}

QString hlineWRDefault(KisDoc * doc, Q_UINT32 testCount)
{
    KisImageSP img = doc->newImage("", 1000, 1000, KisMetaRegistry::instance()->csRegistry()->getColorSpace(KisID("RGBA", ""),""));
    KisPaintDeviceSP l = img->activeDevice();

    QTime t;
    t.restart();

    for (Q_UINT32 i = 0; i < testCount; ++i) {
         int adv;

        for(Q_INT32 y2 = 0; y2 < 0 + 1000; y2++)
        {
            KisHLineIterator hiter = l->createHLineIterator(0, y2, 1000, true);
            while(! hiter.isDone())
            {
                 adv = hiter.nConseqHPixels();
                 hiter += adv;
            }
        }

    }

    return QString("    hline iterated writable 1000 x 1000 pixels %1 times over default tile: %2\n").arg(testCount).arg(t.elapsed());

}

QString hlineWR(KisDoc * doc, Q_UINT32 testCount)
{
    KisImageSP img = doc->newImage("", 1000, 1000, KisMetaRegistry::instance()->csRegistry()->getColorSpace(KisID("RGBA", ""),""));
    KisPaintDeviceSP l = img->activeDevice();

    KisFillPainter p(l.data());
    p.fillRect(0, 0, 1000, 1000, KisColor(Qt::black, KisMetaRegistry::instance()->csRegistry()->getRGB8()));
    p.end();


    QTime t;
    t.restart();

    for (Q_UINT32 i = 0; i < testCount; ++i) {
         int adv;
        for(Q_INT32 y2 = 0; y2 < 0 + 1000; y2++)
        {
            KisHLineIterator hiter = l->createHLineIterator(0, y2, 1000, true);
            while(! hiter.isDone())
            {
                 adv = hiter.nConseqHPixels();
                 hiter += adv;
            }
        }

    }

    return QString("    hline iterated writable 1000 x 1000 pixels %1 times over existing tile: %2\n").arg(testCount).arg(t.elapsed());

}


QString vlineRODefault(KisDoc * doc, Q_UINT32 testCount)
{
    KisImageSP img = doc->newImage("", 1000, 1000, KisMetaRegistry::instance()->csRegistry()->getColorSpace(KisID("RGBA", ""),""));
    KisPaintDeviceSP l = img->activeDevice();

    QTime t;
    t.restart();

    for (Q_UINT32 i = 0; i < testCount; ++i) {
        for(Q_INT32 y2 = 0; y2 < 0 + 1000; y2++)
        {
            KisVLineIterator hiter = l->createVLineIterator(y2, 0, 1000, true);
            while(! hiter.isDone())
            {
                 ++hiter;
            }
        }

    }

    return QString("    vline iterated read-only 1000 x 1000 pixels %1 times over default tile: %2\n").arg(testCount).arg(t.elapsed());

}

QString vlineRO(KisDoc * doc, Q_UINT32 testCount)
{
    KisImageSP img = doc->newImage("", 1000, 1000, KisMetaRegistry::instance()->csRegistry()->getColorSpace(KisID("RGBA", ""),""));
    KisPaintDeviceSP l = img->activeDevice();

    KisFillPainter p(l.data());
    p.fillRect(0, 0, 1000, 1000, KisColor(Qt::black, KisMetaRegistry::instance()->csRegistry()->getRGB8()));
    p.end();


    QTime t;
    t.restart();

    for (Q_UINT32 i = 0; i < testCount; ++i) {
        for(Q_INT32 y2 = 0; y2 < 0 + 1000; y2++)
        {
            KisVLineIterator hiter = l->createVLineIterator(y2, 0, 1000, true);
            while(! hiter.isDone())
            {
                 ++hiter;
            }
        }

    }

    return QString("    vline iterated read-only 1000 x 1000 pixels %1 times over existing tile: %2\n").arg(testCount).arg(t.elapsed());

}

QString vlineWRDefault(KisDoc * doc, Q_UINT32 testCount)
{
    KisImageSP img = doc->newImage("", 1000, 1000, KisMetaRegistry::instance()->csRegistry()->getColorSpace(KisID("RGBA", ""),""));
    KisPaintDeviceSP l = img->activeDevice();

    QTime t;
    t.restart();

    for (Q_UINT32 i = 0; i < testCount; ++i) {

        for(Q_INT32 y2 = 0; y2 < 0 + 1000; y2++)
        {
            KisVLineIterator hiter = l->createVLineIterator(y2, 0, 1000, true);
            while(! hiter.isDone())
            {
                 ++hiter;
            }
        }

    }

    return QString("    vline iterated writable 1000 x 1000 pixels %1 times over default tile: %2\n").arg(testCount).arg(t.elapsed());
}

QString vlineWR(KisDoc * doc, Q_UINT32 testCount)
{

    KisImageSP img = doc->newImage("", 1000, 1000, KisMetaRegistry::instance()->csRegistry()->getColorSpace(KisID("RGBA", ""),""));
    KisPaintDeviceSP l = img->activeDevice();

    KisFillPainter p(l.data());
    p.fillRect(0, 0, 1000, 1000, KisColor(Qt::black, KisMetaRegistry::instance()->csRegistry()->getRGB8()));
    p.end();

    QTime t;
    t.restart();

    for (Q_UINT32 i = 0; i < testCount; ++i) {
        for(Q_INT32 y2 = 0; y2 < 0 + 1000; y2++)
        {
            KisHLineIterator hiter = l->createHLineIterator(y2, 0, 1000, true);
            while(! hiter.isDone())
            {
                 ++hiter;
            }
        }

    }

    return QString("    vline iterated writable 1000 x 1000 pixels %1 times over existing tile: %2\n").arg(testCount).arg(t.elapsed());

}

QString rectRODefault(KisDoc * doc, Q_UINT32 testCount)
{
    KisImageSP img = doc->newImage("", 1000, 1000, KisMetaRegistry::instance()->csRegistry()->getColorSpace(KisID("RGBA", ""),""));
    KisPaintDeviceSP l = img->activeDevice();
;
    QTime t;
    t.restart();

    for (Q_UINT32 i = 0; i < testCount; ++i) {
        KisRectIterator r = l->createRectIterator(0, 0, 1000, 1000, false);
        while(! r.isDone())
        {
            ++r;
        }
    }

    return QString("    rect iterated read-only 1000 x 1000 pixels %1 times over default tile: %2\n").arg(testCount).arg(t.elapsed());


}

QString rectRO(KisDoc * doc, Q_UINT32 testCount)
{
    KisImageSP img = doc->newImage("", 1000, 1000, KisMetaRegistry::instance()->csRegistry()->getColorSpace(KisID("RGBA", ""),""));
    KisPaintDeviceSP l = img->activeDevice();

    KisFillPainter p(l.data());
    p.fillRect(0, 0, 1000, 1000, KisColor(Qt::black, KisMetaRegistry::instance()->csRegistry()->getRGB8()));
    p.end();

    QTime t;
    t.restart();

    for (Q_UINT32 i = 0; i < testCount; ++i) {
        KisRectIterator r = l->createRectIterator(0, 0, 1000, 1000, false);
        while(! r.isDone())
        {
            ++r;
        }
    }

    return QString("    rect iterated read-only 1000 x 1000 pixels %1 times over existing tile: %2\n").arg(testCount).arg(t.elapsed());

}

QString rectWRDefault(KisDoc * doc, Q_UINT32 testCount)
{


    KisImageSP img = doc->newImage("", 1000, 1000, KisMetaRegistry::instance()->csRegistry()->getColorSpace(KisID("RGBA", ""),""));
    KisPaintDeviceSP l = img->activeDevice();

    QTime t;
    t.restart();

    for (Q_UINT32 i = 0; i < testCount; ++i) {
        KisRectIterator r = l->createRectIterator(0, 0, 1000, 1000, true);
        while(! r.isDone())
        {
            ++r;
        }
    }

    return QString("    rect iterated writable 1000 x 1000 pixels %1 times over default tile: %2\n").arg(testCount).arg(t.elapsed());

}

QString rectWR(KisDoc * doc, Q_UINT32 testCount)
{
    KisImageSP img = doc->newImage("", 1000, 1000, KisMetaRegistry::instance()->csRegistry()->getColorSpace(KisID("RGBA", ""),""));
    KisPaintDeviceSP l = img->activeDevice();

    KisFillPainter p(l.data());
    p.fillRect(0, 0, 1000, 1000, KisColor(Qt::black, KisMetaRegistry::instance()->csRegistry()->getRGB8()));
    p.end();


    QTime t;
    t.restart();


    for (Q_UINT32 i = 0; i < testCount; ++i) {
        KisRectIterator r = l->createRectIterator(0, 0, 1000, 1000, true);
        while(! r.isDone())
        {
            ++r;
        }
    }


    return QString("    rect iterated writable 1000 x 1000 pixels %1 times over existing tile: %2\n").arg(testCount).arg(t.elapsed());


}
QString PerfTest::iteratorTest(Q_UINT32 testCount)
{
    QString report = "Iterator test";

    KisDoc * doc = m_view->canvasSubject()->document();

    report = report.append(hlineRODefault(doc, testCount));
    report = report.append(hlineRO(doc, testCount));
    report = report.append(hlineWRDefault(doc, testCount));
    report = report.append(hlineWR(doc, testCount));

    report = report.append(vlineRODefault(doc, testCount));
    report = report.append(vlineRO(doc, testCount));
    report = report.append(vlineWRDefault(doc, testCount));
    report = report.append(vlineWR(doc, testCount));

    report = report.append(rectRODefault(doc, testCount));
    report = report.append(rectRO(doc, testCount));
    report = report.append(rectWRDefault(doc, testCount));
    report = report.append(rectWR(doc, testCount));

    return report;


}

QString PerfTest::paintViewTest(Q_UINT32 testCount)
{
    QString report = QString("* paintView test\n\n");

    KisDoc * doc = m_view->canvasSubject()->document();

    KisImageSP img = doc->currentImage();
    img->resize(512,512);


    KisPaintDeviceSP l = img->activeDevice();

    KisFillPainter p(l.data());
    p.fillRect(0, 0, 512, 512, KisColor(Qt::black, KisMetaRegistry::instance()->csRegistry()->getRGB8()));
    p.end();

    QTime t;
    t.restart();

#if USE_CALLGRIND
    CALLGRIND_ZERO_STATS();
#endif

    for (Q_UINT32 i = 0; i < testCount; ++i) {
        m_view->getCanvasController()->updateCanvas(QRect(0, 0, 512, 512));
    }

#if USE_CALLGRIND
    CALLGRIND_DUMP_STATS();
#endif

    report = report.append(QString("    painted a 512 x 512 image %1 times: %2 ms\n").arg(testCount).arg(t.elapsed()));

    img->newLayer("layer 2", OPACITY_OPAQUE);
    l = img->activeDevice();

    p.begin(l.data());
    p.fillRect(0, 0, 512, 512, KisColor(Qt::black, KisMetaRegistry::instance()->csRegistry()->getRGB8()));
    p.end();

    img->newLayer("layer 3", OPACITY_OPAQUE);
    l = img->activeDevice();

    p.begin(l.data());
    p.fillRect(0, 0, 512, 512, KisColor(Qt::black, KisMetaRegistry::instance()->csRegistry()->getRGB8()));
    p.end();

    t.restart();

    for (Q_UINT32 i = 0; i < testCount; ++i) {
        m_view->getCanvasController()->updateCanvas(QRect(0, 0, 512, 512));
    }

    report = report.append(QString("    painted a 512 x 512 image with 3 layers %1 times: %2 ms\n").arg(testCount).arg(t.elapsed()));

    return report;
}

QString PerfTest::paintViewFPSTest()
{
    QString report = QString("* paintView (fps) test\n\n");

    QTime t;
    t.restart();

#if USE_CALLGRIND
    CALLGRIND_ZERO_STATS();
#endif

    int numViewsPainted = 0;
    const int millisecondsPerSecond = 1000;

    while (t.elapsed() < millisecondsPerSecond) {
        m_view->getCanvasController()->updateCanvas();
        numViewsPainted++;
    }

#if USE_CALLGRIND
    CALLGRIND_DUMP_STATS();
#endif

    report = report.append(QString("    painted current view at %1 frames per second\n").arg(numViewsPainted));

    return report;
}

#include "perftest.moc"
