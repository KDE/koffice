/* This file is part of the KDE project
 * Copyright 2007 Marijn Kruisselbrink <m.kruiselbrink@student.tue.nl>
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
#include <QPainter>
#include <QGridLayout>
#include <QToolButton>
#include <QTabWidget>

#include <kdebug.h>
#include <klocale.h>
#include <kiconloader.h>
#include <KFileDialog>

#include <KoCanvasBase.h>
#include <KoSelection.h>
#include <KoShapeManager.h>

#include "MusicShape.h"

#include "MusicTool.h"
#include "MusicTool.moc"

#include "dialogs/PartsWidget.h"

MusicTool::MusicTool( KoCanvasBase* canvas )
    : KoTool( canvas ),
      m_musicshape(0)
{
}

MusicTool::~MusicTool()
{
}

void MusicTool::activate (bool temporary)
{
    Q_UNUSED( temporary );
    kDebug() << k_funcinfo << endl;

    KoSelection* selection = m_canvas->shapeManager()->selection();
    foreach ( KoShape* shape, selection->selectedShapes() )
    {
        m_musicshape = dynamic_cast<MusicShape*>( shape );
        if ( m_musicshape )
            break;
    }
    if ( !m_musicshape )
    {
        emit sigDone();
        return;
    }
    emit sheetChanged(m_musicshape->sheet());
    useCursor( Qt::ArrowCursor, true );
}

void MusicTool::deactivate()
{
  kDebug()<<"MusicTool::deactivate\n";
  m_musicshape = 0;
}

void MusicTool::paint( QPainter& painter, KoViewConverter& viewConverter )
{
    Q_UNUSED( viewConverter );
}

void MusicTool::mousePressEvent( KoPointerEvent* )
{
}

void MusicTool::mouseMoveEvent( KoPointerEvent* )
{
}

void MusicTool::mouseReleaseEvent( KoPointerEvent* )
{
}


QWidget * MusicTool::createOptionWidget()
{
    QTabWidget *widget = new QTabWidget();

    PartsWidget *pw = new PartsWidget(this, widget);
    widget->addTab(pw, i18n("Parts"));

    connect(this, SIGNAL(sheetChanged(MusicCore::Sheet*)), pw, SLOT(setSheet(MusicCore::Sheet*)));

    if (m_musicshape) pw->setSheet(m_musicshape->sheet());

    return widget;

}


