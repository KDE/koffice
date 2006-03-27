/*
 *  Copyright (c) 2005 Boudewijn Rempt <boud@valdyas.org>
 *
   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2, as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include <qwidget.h>
#include <q3dockwindow.h>
#include <qvariant.h>
#include <qlabel.h>
#include <qtoolbutton.h>
#include <qtabwidget.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <q3whatsthis.h>
#include <qimage.h>
#include <qpixmap.h>
#include <qlayout.h>

#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>
#include <kglobalsettings.h>
#include <kacceleratormanager.h>
#include <kconfig.h>

#include <KoView.h>

#include "kopalette.h"

KoPalette::KoPalette(QWidget * parent, const char * name)
    : Q3DockWindow(parent, name)
{

#if KDE_VERSION >= KDE_MAKE_VERSION(3,3,90)
    KAcceleratorManager::setNoAccel(this);
#endif
    setCloseMode( Q3DockWindow::Never);
    setResizeEnabled(true);
    setOpaqueMoving(true);
    setFocusPolicy(Qt::NoFocus);
    setVerticallyStretchable(false);
    setHorizontallyStretchable(false);

    setNewLine(true);
    layout() -> setSpacing(0);
    layout() -> setMargin(0);

    resetFont();
}

void KoPalette::resetFont()
{
            
    KConfig * cfg = KGlobal::config();
    Q_ASSERT(cfg);
    cfg->setGroup("");
    m_font  = KGlobalSettings::generalFont();
    float ps = qMin(9.0, KGlobalSettings::generalFont().pointSize() * 0.8);
    ps = cfg->readNumEntry("palettefontsize", (int)ps);
    if (ps < 6) ps = 6;
    m_font.setPointSize((int)ps);
    setFont(m_font);

}

KoPalette::~KoPalette()
{
}

void KoPalette::setMainWidget(QWidget * widget)
{
    setWidget(widget);
    resize( QSize(285, 233).expandedTo(minimumSizeHint()) );
#warning "kde 4: port it"
    //clearWState( WState_Polished );
    widget->setFont(m_font);
    m_page = widget;
}

#include "kopalette.moc"
