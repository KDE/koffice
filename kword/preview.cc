/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Reginald Stadlbauer <reggie@kde.org>

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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include "preview.h"
#include "preview.moc"
#include <koClipartCollection.h>
#include <kdialog.h>
#include <kurl.h>
#include <qlayout.h>
#include <qfileinfo.h>
#include <qpainter.h>
#include <qpicture.h>
#include <qscrollview.h>

class PixmapView : public QScrollView
{
public:
    PixmapView( QWidget *parent )
	: QScrollView( parent ) { viewport()->setBackgroundMode( PaletteBase ); }

    void setPixmap( const QPixmap &pix ) {
	pixmap = pix;
	resizeContents( pixmap.size().width(), pixmap.size().height() );
	viewport()->repaint( FALSE );
    }

    void setClipart( const QString &s ) {
        QPicture pic;
        if ( KoClipartCollection::loadFromFile( s, &pic ) )
        {
 	    pixmap = QPixmap( 200, 200 );
 	    QPainter p;

 	    p.begin( &pixmap );
 	    p.setBackgroundColor( Qt::white );
 	    pixmap.fill( Qt::white );

            QRect br = pic.boundingRect();
            if ( br.width() && br.height() ) // just to avoid an impossible crash
                p.scale( (double)pixmap.width() / (double)br.width(), (double)pixmap.height() / (double)br.height() );
 	    p.drawPicture( pic );
 	    p.end();
 	    resizeContents( pixmap.width(), pixmap.height() );
 	    viewport()->repaint( FALSE );
 	}
    }

    void drawContents( QPainter *p, int, int, int, int ) {
	p->drawPixmap( 0, 0, pixmap );
    }

private:
    QPixmap pixmap;

};

Preview::Preview( QWidget *parent )
    : KPreviewWidgetBase( parent )
{
    QVBoxLayout *vb = new QVBoxLayout( this, KDialog::marginHint() );
    pixmapView = new PixmapView( this );
    vb->addWidget( pixmapView, 1 );
}

void Preview::showPreview( const KURL &u )
{
    if ( u.isLocalFile() ) {
	QString path = u.path();
	QFileInfo fi( path );
	if ( fi.extension().lower() == "wmf" || fi.extension().lower() == "emf" || fi.extension().lower() == "svg")
	    pixmapView->setClipart( path );
	else {
	    QPixmap pix( path );
	    pixmapView->setPixmap( pix );
	}
    } else {
	pixmapView->setPixmap( QPixmap() );
    }
}

void Preview::clearPreview()
{
    pixmapView->setPixmap( QPixmap() );
}
