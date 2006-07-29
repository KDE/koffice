/*
 *  kis_controlframe.cc - part of Krita
 *
 *  Copyright (c) 1999 Matthias Elter  <elter@kde.org>
 *  Copyright (c) 2003 Patrick Julien  <freak@codepimps.org>
 *  Copyright (c) 2004 Sven Langkamp  <longamp@reallygood.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.g
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include <stdlib.h>

#include <qapplication.h>
#include <qlayout.h>
#include <qtabwidget.h>
#include <qframe.h>
#include <qwidget.h>
#include <qevent.h>
#include <qtimer.h>

#include <ktoolbar.h>
#include <kmainwindow.h>
#include <kglobalsettings.h>
#include <kstandarddirs.h>
#include <kdebug.h>
#include <klocale.h>
#include <koFrameButton.h>
#include <kactioncollection.h>

#include "kis_resourceserver.h"
#include "kis_controlframe.h"
#include "kis_resource_mediator.h"
#include "kis_itemchooser.h"
#include "kis_pattern_chooser.h"
#include "kis_gradient_chooser.h"
#include "kis_icon_item.h"
#include "kis_iconwidget.h"
#include "kis_brush.h"
#include "kis_pattern.h"
#include "kis_gradient.h"
#include "kis_brush_chooser.h"
#include "kis_view.h"
#include "kis_autobrush.h"
#include "kis_autogradient.h"
#include "kis_config.h"
#include "kis_paintop_box.h"
#include "kis_custom_brush.h"
#include "kis_custom_pattern.h"
#ifdef HAVE_TEXT_BRUSH
#include "kis_text_brush.h"
#endif
KisPopupFrame::KisPopupFrame(QWidget * parent, const char* name)
    : QPopupMenu(parent, name)
{
    setFocusPolicy(StrongFocus);
}

void KisPopupFrame::keyPressEvent(QKeyEvent * e)
{
    if (e->key()== Qt::Key_Escape)
    {
        hide();
        e->accept();
    }
    else {
        e->ignore();
    }
}


KisControlFrame::KisControlFrame( KMainWindow * /*window*/, KisView * view, const char* name )
    : QObject(view, name)
    //: KToolBar ( window, Qt::DockTop, false, name, true, true )
    , m_view(view)
    , m_brushWidget(0)
    , m_patternWidget(0)
    , m_gradientWidget(0)
    , m_brushChooserPopup(0)
    , m_patternChooserPopup(0)
    , m_gradientChooserPopup(0)
    , m_brushMediator(0)
    , m_patternMediator(0)
    , m_gradientMediator(0)
    , m_paintopBox(0)
{

    KisConfig cfg;
    m_font  = KGlobalSettings::generalFont();
    m_font.setPointSize((int)cfg.dockerFontSize());

    m_brushWidget = new KisIconWidget(view, "brushes");
    m_brushWidget->setTextLabel( i18n("Brush Shapes") );
    // XXX: An action without a slot -- that's silly, what kind of action could we use here?
    KAction * action = new KWidgetAction(m_brushWidget,
                                         i18n("&Brush"),
                                         0,
                                         view,
                                         0,
                                         view->actionCollection(),
                                         "brushes");


    m_patternWidget = new KisIconWidget(view, "patterns");
    m_patternWidget->setTextLabel( i18n("Fill Patterns") );
    action = new KWidgetAction(m_patternWidget,
                               i18n("&Patterns"),
                               0,
                               view,
                               0,
                               view->actionCollection(),
                               "patterns");

    m_gradientWidget = new KisIconWidget(view, "gradients");
    m_gradientWidget->setTextLabel( i18n("Gradients") );
    action = new KWidgetAction(m_gradientWidget,
                               i18n("&Gradients"),
                               0,
                               view,
                               0,
                               view->actionCollection(),
                               "gradients");

    m_paintopBox = new KisPaintopBox( view, view, "paintopbox" );
    action = new KWidgetAction(m_paintopBox,
                               i18n("&Painter's Tools"),
                               0,
                               view,
                               0,
                               view->actionCollection(),
                               "paintops");

    m_brushWidget->setFixedSize( 26, 26 );
    m_patternWidget->setFixedSize( 26, 26 );
    m_gradientWidget->setFixedSize( 26, 26 );

    createBrushesChooser(m_view);
    createPatternsChooser(m_view);
    createGradientsChooser(m_view);

    m_brushWidget->setPopup(m_brushChooserPopup);
    m_brushWidget->setPopupDelay(1);
    m_patternWidget->setPopup(m_patternChooserPopup);
    m_patternWidget->setPopupDelay(1);
    m_gradientWidget->setPopup(m_gradientChooserPopup);
    m_gradientWidget->setPopupDelay(1);
}


void KisControlFrame::slotSetBrush(KoIconItem *item)
{
    if (item)
        m_brushWidget->slotSetItem(*item);
}

void KisControlFrame::slotSetPattern(KoIconItem *item)
{
    if (item)
        m_patternWidget->slotSetItem(*item);
}

void KisControlFrame::slotSetGradient(KoIconItem *item)
{
    if (item)
        m_gradientWidget->slotSetItem(*item);
}

void KisControlFrame::slotBrushChanged(KisBrush * brush)
{
        KisIconItem *item;

        if((item = m_brushMediator->itemFor(brush)))
        {
                slotSetBrush(item);
        } else {
                slotSetBrush( new KisIconItem(brush) );
        }

}

void KisControlFrame::slotPatternChanged(KisPattern * pattern)
{
        KisIconItem *item;
        if (!pattern)
                return;

        if ( (item = m_patternMediator->itemFor(pattern)) )
                slotSetPattern(item);
        else
                slotSetPattern( new KisIconItem(pattern) );
}


void KisControlFrame::slotGradientChanged(KisGradient * gradient)
{
        KisIconItem *item;
        if (!gradient)
                return;

        if ( (item = m_gradientMediator->itemFor(gradient)) )
                slotSetGradient(item);
        else
                slotSetGradient( new KisIconItem(gradient) );
}

void KisControlFrame::createBrushesChooser(KisView * view)
{

    m_brushChooserPopup = new KisPopupFrame(m_brushWidget, "brush_chooser_popup");

    QHBoxLayout * l = new QHBoxLayout(m_brushChooserPopup, 2, 2, "brushpopuplayout");

    QTabWidget * m_brushesTab = new QTabWidget(m_brushChooserPopup, "brushestab");
    m_brushesTab->setTabShape(QTabWidget::Triangular);
    m_brushesTab->setFocusPolicy(QWidget::NoFocus);
    m_brushesTab->setFont(m_font);
    m_brushesTab->setMargin(1);

    l->add(m_brushesTab);

    KisBrushChooser * m_brushChooser = new KisBrushChooser(m_brushesTab, "brush_chooser");
    m_brushesTab->addTab( m_brushChooser, i18n("Predefined Brushes"));

    KisAutobrush * m_autobrush = new KisAutobrush(m_brushesTab, "autobrush", i18n("Autobrush"));
    m_brushesTab->addTab( m_autobrush, i18n("Autobrush"));
    connect(m_autobrush, SIGNAL(activatedResource(KisResource*)), m_view, SLOT(brushActivated( KisResource* )));

    KisCustomBrush* customBrushes = new KisCustomBrush(m_brushesTab, "custombrush",
            i18n("Custom Brush"), m_view);
    m_brushesTab->addTab( customBrushes, i18n("Custom Brush"));
    connect(customBrushes, SIGNAL(activatedResource(KisResource*)),
            m_view, SLOT(brushActivated(KisResource*)));
#ifdef HAVE_TEXT_BRUSH
    KisTextBrush* textBrushes = new KisTextBrush(m_brushesTab, "textbrush",
            i18n("Text Brush")/*, m_view*/);
    m_brushesTab->addTab( textBrushes, i18n("Text Brush"));
    connect(textBrushes, SIGNAL(activatedResource(KisResource*)),
            m_view, SLOT(brushActivated(KisResource*)));
#endif

    m_brushChooser->setFont(m_font);
    m_brushMediator = new KisResourceMediator( m_brushChooser, this);
    connect(m_brushMediator, SIGNAL(activatedResource(KisResource*)), m_view, SLOT(brushActivated(KisResource*)));

    KisResourceServerBase* rServer;
    rServer = KisResourceServerRegistry::instance()->get("ImagePipeBrushServer");
    m_brushMediator->connectServer(rServer);
    rServer = KisResourceServerRegistry::instance()->get("BrushServer");
    m_brushMediator->connectServer(rServer);

    KisControlFrame::connect(view, SIGNAL(brushChanged(KisBrush *)), this, SLOT(slotBrushChanged( KisBrush *)));
    m_brushChooser->setCurrent( 0 );
    m_brushMediator->setActiveItem( m_brushChooser->currentItem() );
    customBrushes->setResourceServer(rServer);

    m_autobrush->activate();
}

void KisControlFrame::createPatternsChooser(KisView * view)
{
    m_patternChooserPopup = new KisPopupFrame(m_patternWidget, "pattern_chooser_popup");

    QHBoxLayout * l2 = new QHBoxLayout(m_patternChooserPopup, 2, 2, "patternpopuplayout");

    QTabWidget * m_patternsTab = new QTabWidget(m_patternChooserPopup, "patternstab");
    m_patternsTab->setTabShape(QTabWidget::Triangular);
    m_patternsTab->setFocusPolicy(QWidget::NoFocus);
    m_patternsTab->setFont(m_font);
    m_patternsTab->setMargin(1);
    l2->add( m_patternsTab );

    KisPatternChooser * chooser = new KisPatternChooser(m_patternChooserPopup, "pattern_chooser");
    chooser->setFont(m_font);
    chooser->setMinimumSize(200, 150);
    m_patternsTab->addTab(chooser, i18n("Patterns"));

    KisCustomPattern* customPatterns = new KisCustomPattern(m_patternsTab, "custompatterns",
            i18n("Custom Pattern"), m_view);
    customPatterns->setFont(m_font);
    m_patternsTab->addTab( customPatterns, i18n("Custom Pattern"));


    m_patternMediator = new KisResourceMediator( chooser, view);
    connect( m_patternMediator, SIGNAL(activatedResource(KisResource*)), view, SLOT(patternActivated(KisResource*)));
    connect(customPatterns, SIGNAL(activatedResource(KisResource*)),
            view, SLOT(patternActivated(KisResource*)));

    KisResourceServerBase* rServer;
    rServer = KisResourceServerRegistry::instance()->get("PatternServer");
    m_patternMediator->connectServer(rServer);

    KisControlFrame::connect(view, SIGNAL(patternChanged(KisPattern *)), this, SLOT(slotPatternChanged( KisPattern *)));
    chooser->setCurrent( 0 );
    m_patternMediator->setActiveItem( chooser->currentItem() );

    customPatterns->setResourceServer(rServer);
}


void KisControlFrame::createGradientsChooser(KisView * view)
{
    m_gradientChooserPopup = new KisPopupFrame(m_gradientWidget, "gradient_chooser_popup");

    QHBoxLayout * l2 = new QHBoxLayout(m_gradientChooserPopup, 2, 2, "gradientpopuplayout");

    QTabWidget * m_gradientTab = new QTabWidget(m_gradientChooserPopup, "gradientstab");
    m_gradientTab->setTabShape(QTabWidget::Triangular);
    m_gradientTab->setFocusPolicy(QWidget::NoFocus);
    m_gradientTab->setFont(m_font);
    m_gradientTab->setMargin(1);

    l2->add( m_gradientTab);

    KisGradientChooser * m_gradientChooser = new KisGradientChooser(m_view, m_gradientChooserPopup, "gradient_chooser");
    m_gradientChooser->setFont(m_font);
    m_gradientChooser->setMinimumSize(200, 150);
    m_gradientTab->addTab( m_gradientChooser, i18n("Gradients"));

    m_gradientMediator = new KisResourceMediator( m_gradientChooser, view);
    connect(m_gradientMediator, SIGNAL(activatedResource(KisResource*)), view, SLOT(gradientActivated(KisResource*)));

    KisResourceServerBase* rServer;
    rServer = KisResourceServerRegistry::instance()->get("GradientServer");
    m_gradientMediator->connectServer(rServer);

    connect(view, SIGNAL(gradientChanged(KisGradient *)), this, SLOT(slotGradientChanged( KisGradient *)));
    m_gradientChooser->setCurrent( 0 );
    m_gradientMediator->setActiveItem( m_gradientChooser->currentItem() );
}


#include "kis_controlframe.moc"

