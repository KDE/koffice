/* This file is part of the KDE project
   Copyright (C) 2002 Chris Machemer <machey@ceinetworks.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include "kdatabase_view.h"
#include "kdatabase_factory.h"
#include "kdatabase_part.h"
#include "maindlg.h"

#include <qpainter.h>
#include <qiconset.h>
#include <kaction.h>
#include <kstdaction.h>
#include <klocale.h>
#include <kdebug.h>

KDatabaseView::KDatabaseView( KDatabasePart* part, QWidget* parent, const char* name )
    : KoView( part, parent, name )
{
    setInstance( KDatabaseFactory::global() );
    setXMLFile( "kdatabase.rc" );
    KStdAction::copy(this, SLOT( copy() ), actionCollection(), "copy" );
    KStdAction::cut(this, SLOT( cut() ), actionCollection(), "cut" );
    KStdAction::paste(this, SLOT( paste() ), actionCollection(), "paste" );
    myMainDlg = new MainDlg(this,name);
    myMainDlg->initStruct(part->getKDBFile());
    myMainDlg->show();

    // Note: Prefer KStdAction::* to any custom action if possible.
    //m_cut = new KAction( i18n("&Cut"), "editcut", 0, this, SLOT( cut() ),
    //                   actionCollection(), "cut");
}

void KDatabaseView::paintEvent( QPaintEvent* ev )
{
    QPainter painter;
    painter.begin( this );

    // ### TODO: Scaling

    // Let the document do the drawing
    //koDocument()->paintEverything( painter, ev->rect(), FALSE, this );

    painter.end();
}

void KDatabaseView::updateReadWrite( bool /*readwrite*/ )
{
#ifdef __GNUC__
#warning TODO
#endif
}

void KDatabaseView::cut()
{
    kdDebug(31000) << "KDatabaseView::cut(): CUT called" << endl;
}

void KDatabaseView::copy()
{
    kdDebug(31000) << "KDatabaseView::copy(): COPY called" << endl;
}

void KDatabaseView::paste()
{
    kdDebug(31000) << "KDatabaseView::paste(): PASTE called" << endl;
}

#include "kdatabase_view.moc"
