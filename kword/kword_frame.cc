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

#include "kword_doc.h"
#include "kword_view.h"
#include "kword_page.h"
#include "kword_frame.h"
#include "image.h"
#include "parag.h"
#include "defs.h"
#include "kword_utils.h"
#include "format.h"
#include "font.h"

#include <komlMime.h>
#include <koPageLayoutDia.h>

#include <strstream>
#include <fstream>
#include <unistd.h>
#include <limits.h>

#include <qpicture.h>
#include <qwidget.h>
#include <qpixmap.h>
#include <qfile.h>
#include <qscrollview.h>
#include <qarray.h>

#include <frame.h>

/******************************************************************/
/* Class: KWFrame						  */
/******************************************************************/

/*================================================================*/
KWFrame::KWFrame()
    : QRect(), runAroundGap( 1.0 ), intersections(), bleft(), bright(), btop(), bbottom()
{
    runAround = RA_NO;
    intersections.setAutoDelete( true );
    selected = false;
    runAroundGap = 1;
    mostRight = false;
    emptyRegionDirty = TRUE;

    backgroundColor = QBrush( Qt::white );
    brd_left.color = getBackgroundColor().color();
    brd_left.style = KWParagLayout::SOLID;
    brd_left.ptWidth = 1;
    brd_right.color = getBackgroundColor().color();
    brd_right.style = KWParagLayout::SOLID;
    brd_right.ptWidth = 1;
    brd_top.color = getBackgroundColor().color();
    brd_top.style = KWParagLayout::SOLID;
    brd_top.ptWidth = 1;
    brd_bottom.color = getBackgroundColor().color();
    brd_bottom.style = KWParagLayout::SOLID;
    brd_bottom.ptWidth = 1;
}

/*================================================================*/
KWFrame::KWFrame( const QPoint &topleft, const QPoint &bottomright )
    : QRect( topleft, bottomright ), runAroundGap( 1.0 ), intersections(), bleft(), bright(), btop(), bbottom()
{
    runAround = RA_NO;
    intersections.setAutoDelete( true );
    selected = false;
    mostRight = false;
    emptyRegionDirty = TRUE;

    backgroundColor = QBrush( Qt::white );
    brd_left.color = getBackgroundColor().color();
    brd_left.style = KWParagLayout::SOLID;
    brd_left.ptWidth = 1;
    brd_right.color = getBackgroundColor().color();
    brd_right.style = KWParagLayout::SOLID;
    brd_right.ptWidth = 1;
    brd_top.color = getBackgroundColor().color();
    brd_top.style = KWParagLayout::SOLID;
    brd_top.ptWidth = 1;
    brd_bottom.color = getBackgroundColor().color();
    brd_bottom.style = KWParagLayout::SOLID;
    brd_bottom.ptWidth = 1;
}

/*================================================================*/
KWFrame::KWFrame( const QPoint &topleft, const QSize &size )
    : QRect( topleft, size ), runAroundGap( 1.0 ), intersections(), bleft(), bright(), btop(), bbottom()
{
    runAround = RA_NO;
    intersections.setAutoDelete( true );
    selected = false;
    mostRight = false;
    emptyRegionDirty = TRUE;

    backgroundColor = QBrush( Qt::white );
    brd_left.color = getBackgroundColor().color();
    brd_left.style = KWParagLayout::SOLID;
    brd_left.ptWidth = 1;
    brd_right.color = getBackgroundColor().color();
    brd_right.style = KWParagLayout::SOLID;
    brd_right.ptWidth = 1;
    brd_top.color = getBackgroundColor().color();
    brd_top.style = KWParagLayout::SOLID;
    brd_top.ptWidth = 1;
    brd_bottom.color = getBackgroundColor().color();
    brd_bottom.style = KWParagLayout::SOLID;
    brd_bottom.ptWidth = 1;
}

/*================================================================*/
KWFrame::KWFrame( int left, int top, int width, int height )
    : QRect( left, top, width, height ), runAroundGap( 1.0 ), intersections(), bleft(), bright(), btop(), bbottom()
{
    runAround = RA_NO;
    intersections.setAutoDelete( true );
    selected = false;
    mostRight = false;
    emptyRegionDirty = TRUE;

    backgroundColor = QBrush( Qt::white );
    brd_left.color = getBackgroundColor().color();
    brd_left.style = KWParagLayout::SOLID;
    brd_left.ptWidth = 1;
    brd_right.color = getBackgroundColor().color();
    brd_right.style = KWParagLayout::SOLID;
    brd_right.ptWidth = 1;
    brd_top.color = getBackgroundColor().color();
    brd_top.style = KWParagLayout::SOLID;
    brd_top.ptWidth = 1;
    brd_bottom.color = getBackgroundColor().color();
    brd_bottom.style = KWParagLayout::SOLID;
    brd_bottom.ptWidth = 1;
}

/*================================================================*/
KWFrame::KWFrame( int left, int top, int width, int height, RunAround _ra, KWUnit _gap )
    : QRect( left, top, width, height ), runAroundGap( _gap ), intersections(), bleft(), bright(), btop(), bbottom()
{
    runAround = _ra;
    intersections.setAutoDelete( true );
    selected = false;
    mostRight = false;
    emptyRegionDirty = TRUE;

    backgroundColor = QBrush( Qt::white );
    brd_left.color = getBackgroundColor().color();
    brd_left.style = KWParagLayout::SOLID;
    brd_left.ptWidth = 1;
    brd_right.color = getBackgroundColor().color();
    brd_right.style = KWParagLayout::SOLID;
    brd_right.ptWidth = 1;
    brd_top.color = getBackgroundColor().color();
    brd_top.style = KWParagLayout::SOLID;
    brd_top.ptWidth = 1;
    brd_bottom.color = getBackgroundColor().color();
    brd_bottom.style = KWParagLayout::SOLID;
    brd_bottom.ptWidth = 1;
}

/*================================================================*/
KWFrame::KWFrame( const QRect &_rect )
    : QRect( _rect ), runAroundGap( 1.0 ), intersections(), bleft(), bright(), btop(), bbottom()
{
    runAround = RA_NO;
    intersections.setAutoDelete( true );
    selected = false;
    mostRight = false;
    emptyRegionDirty = TRUE;

    backgroundColor = QBrush( Qt::white );
    brd_left.color = getBackgroundColor().color();
    brd_left.style = KWParagLayout::SOLID;
    brd_left.ptWidth = 1;
    brd_right.color = getBackgroundColor().color();
    brd_right.style = KWParagLayout::SOLID;
    brd_right.ptWidth = 1;
    brd_top.color = getBackgroundColor().color();
    brd_top.style = KWParagLayout::SOLID;
    brd_top.ptWidth = 1;
    brd_bottom.color = getBackgroundColor().color();
    brd_bottom.style = KWParagLayout::SOLID;
    brd_bottom.ptWidth = 1;
}

/*================================================================*/
KWFrame::~KWFrame()
{
    if ( handles.size() >= 8 ) {
	for ( unsigned int i = 0; i < 8; ++i ) {
	    if ( handles[ i ] )
		delete handles[ i ];
	}
    }
}

/*================================================================*/
void KWFrame::addIntersect( QRect &_r )
{
    emptyRegionDirty = TRUE;

    intersections.append( new QRect( _r.x(), _r.y(), _r.width(), _r.height() ) );
}

/*================================================================*/
int KWFrame::getLeftIndent( int _y, int _h )
{
    if ( runAround == RA_NO || intersections.isEmpty() )
	return 0;

    if ( emptyRegionDirty )
	getEmptyRegion();

    int _left = 0;
    QRect line( x(), _y, width(), _h );
    QRegion reg = emptyRegion.intersect( line );
    _left = reg.boundingRect().left() - x();
    for ( unsigned int i = 0; i < reg.rects().size(); ++i )
	_left = QMAX( _left, reg.rects()[ i ].left() - x() );
    if ( _left > 0 )
	_left += runAroundGap.pt();
    if ( _left > 0 && runAround == RA_SKIP )
	_left = width();

    return _left;
}

/*================================================================*/
int KWFrame::getRightIndent( int _y, int _h )
{
    if ( runAround == RA_NO || intersections.isEmpty() )
	return 0;

    if ( emptyRegionDirty )
	getEmptyRegion();

    int _right = 0;
    QRegion line( x(), _y, width(), _h );
    QRegion reg = line.subtract( emptyRegion );
    _right = 0;
    for ( unsigned int i = 0; i < reg.rects().size(); ++i ) {
	if ( reg.rects()[ i ].right() == right() )
	    _right = QMAX( _right, reg.rects()[ i ].width() );
    }
    if ( _right > 0 )
	_right += runAroundGap.pt();
    if ( _right > 0 && runAround == RA_SKIP )
	_right = width();

    return _right;
}

/*================================================================*/
unsigned int KWFrame::getNextFreeYPos( unsigned int _y, unsigned int _h )
{
    QRect rect;
    unsigned int __y = _y;

    for ( unsigned int i = 0; i < intersections.count(); i++ ) {
	rect = *intersections.at( i );

	if ( rect.intersects( QRect( 0, _y, INT_MAX, _h ) ) )
	    __y = __y == _y ? rect.bottom() : QMIN( static_cast<int>( __y ), rect.bottom() );
    }

    return __y;
}

/*================================================================*/
QRegion KWFrame::getEmptyRegion( bool useCached )
{
    if ( !emptyRegionDirty && useCached )
	return emptyRegion;

    emptyRegion = QRegion( x(), y(), width(), height() );
    QRect rect;

    for ( unsigned int i = 0; i < intersections.count(); i++ ) {
	rect = *intersections.at( i );
	emptyRegion = emptyRegion.subtract( QRect( rect.x() - 1, rect.y() - 1,
						   rect.width() + 2, rect.height() + 2 ) );
    }

    emptyRegionDirty = FALSE;

    return emptyRegion;
}

/*================================================================*/
QCursor KWFrame::getMouseCursor( int mx, int my, bool table )
{
    if ( !table ) {
	if ( mx >= x() && my >= y() && mx <= x() + 6 && my <= y() + 6 )
	    return Qt::sizeFDiagCursor;
	if ( mx >= x() && my >= y() + height() / 2 - 3 && mx <= x() + 6 && my <= y() + height() / 2 + 3 )
	    return Qt::sizeHorCursor;
	if ( mx >= x() && my >= y() + height() - 6 && mx <= x() + 6 && my <= y() + height() )
	    return Qt::sizeBDiagCursor;
	if ( mx >= x() + width() / 2 - 3 && my >= y() && mx <= x() + width() / 2 + 3 && my <= y() + 6 )
	    return Qt::sizeVerCursor;
	if ( mx >= x() + width() / 2 - 3 && my >= y() + height() - 6 && mx <= x() + width() / 2 + 3 &&
	     my <= y() + height() )
	    return Qt::sizeVerCursor;
	if ( mx >= x() + width() - 6 && my >= y() && mx <= x() + width() && my <= y() + 6 )
	    return Qt::sizeBDiagCursor;
	if ( mx >= x() + width() - 6 && my >= y() + height() / 2 - 3 && mx <= x() + width() &&
	     my <= y() + height() / 2 + 3 )
	    return Qt::sizeHorCursor;
	if ( mx >= x() + width() - 6 && my >= y() + height() - 6 && mx <= x() + width() && my <= y() + height() )
	    return Qt::sizeFDiagCursor;

	if ( selected )
	    return Qt::sizeAllCursor;
    } else {
	if ( mx >= x() + width() - 6 && my >= y() && mx <= x() + width() && my <= y() + height() )
	    return Qt::sizeHorCursor;
	if ( mx >= x() && my >= y() + height() - 6 && mx <= x() + width() && my <= y() + height() )
	    return Qt::sizeVerCursor;
	return Qt::sizeAllCursor;
    }

    return Qt::arrowCursor;
}

/*================================================================*/
QString KWFrame::leftBrd2String()
{
    QString str;
    str.sprintf( " lWidth=\"%d\" lRed=\"%d\" lGreen=\"%d\" lBlue=\"%d\" lStyle=\"%d\" ",
		 brd_left.ptWidth, brd_left.color.red(), brd_left.color.green(), brd_left.color.blue(),
		 static_cast<int>( brd_left.style ) );
    return str;
}

/*================================================================*/
QString KWFrame::rightBrd2String()
{
    QString str;
    str.sprintf( " rWidth=\"%d\" rRed=\"%d\" rGreen=\"%d\" rBlue=\"%d\" rStyle=\"%d\" ",
		 brd_right.ptWidth, brd_right.color.red(), brd_right.color.green(), brd_right.color.blue(),
		 static_cast<int>( brd_right.style ) );
    return str;
}

/*================================================================*/
QString KWFrame::topBrd2String()
{
    QString str;
    str.sprintf( " tWidth=\"%d\" tRed=\"%d\" tGreen=\"%d\" tBlue=\"%d\" tStyle=\"%d\" ",
		 brd_top.ptWidth, brd_top.color.red(), brd_top.color.green(), brd_top.color.blue(),
		 static_cast<int>( brd_top.style ) );
    return str;
}

/*================================================================*/
QString KWFrame::bottomBrd2String()
{
    QString str;
    str.sprintf( " bWidth=\"%d\" bRed=\"%d\" bGreen=\"%d\" bBlue=\"%d\" bStyle=\"%d\" ",
		 brd_bottom.ptWidth, brd_bottom.color.red(), brd_bottom.color.green(), brd_bottom.color.blue(),
		 static_cast<int>( brd_bottom.style ) );
    return str;
}

/******************************************************************/
/* Class: KWFrameSet						  */
/******************************************************************/

/*================================================================*/
KWFrameSet::KWFrameSet( KWordDocument *_doc )
    : frames(), removeableHeader( false ), visible( true )
{
    doc = _doc;
    frames.setAutoDelete( true );
    frameInfo = FI_BODY;
    current = 0;
    grpMgr = 0L;
}

/*================================================================*/
void KWFrameSet::addFrame( KWFrame _frame )
{
    KWFrame *frm = new KWFrame( _frame.x(), _frame.y(), _frame.width(), _frame.height(), _frame.getRunAround(),
				_frame.getRunAroundGap() );
    frm->setLeftBorder( _frame.getLeftBorder2() );
    frm->setRightBorder( _frame.getRightBorder2() );
    frm->setTopBorder( _frame.getTopBorder2() );
    frm->setBottomBorder( _frame.getBottomBorder2() );
    frm->setBLeft( _frame.getBLeft() );
    frm->setBRight( _frame.getBRight() );
    frm->setBTop( _frame.getBTop() );
    frm->setBBottom( _frame.getBBottom() );
    frm->setBackgroundColor( QBrush( _frame.getBackgroundColor() ) );

    frames.append( frm );
    if ( frames.count() == 1 ) init();
    update();
}

/*================================================================*/
void KWFrameSet::addFrame( KWFrame *_frame )
{
    if ( frames.findRef( _frame ) != -1 )
	return;

    frames.append( _frame );
    if ( frames.count() == 1 ) init();
    update();
}

/*================================================================*/
void KWFrameSet::delFrame( unsigned int _num )
{
    KWFrame *frm = frames.at( _num ), *f;
    bool del = true;
    unsigned int i = 0;
    for ( f = frames.first(); f != 0; f = frames.next(), i++ ) {
	if ( f == frm && i != _num ) {
	    del = false;
	    break;
	}
    }

    if ( !del )
	frames.take( _num );
    else
	frames.remove( _num );

    update();
}

/*================================================================*/
void KWFrameSet::delFrame( KWFrame *frm, bool remove )
{
    int _num = frames.findRef( frm );
    if ( _num == -1 )
	return;

    KWFrame *f;

    bool del = true;
    int i = 0;
    for ( f = frames.first(); f != 0; f = frames.next(), i++ ) {
	if ( f == frm && i != _num ) {
	    del = false;
	    break;
	}
    }

    if ( !del || !remove )
	frames.take( _num );
    else
	frames.remove( _num );

    update();
}

/*================================================================*/
int KWFrameSet::getFrame( int _x, int _y )
{
    for ( unsigned int i = 0; i < getNumFrames(); i++ ) {
	if ( getFrame( i )->contains( QPoint( _x, _y ) ) ) return i;
    }
    return -1;
}

/*================================================================*/
KWFrame *KWFrameSet::getFrame( unsigned int _num )
{
    return frames.at( _num );
}

/*================================================================*/
int KWFrameSet::getFrameFromPtr( KWFrame *frame )
{
    return frames.findRef( frame );
}

/*================================================================*/
bool KWFrameSet::contains( unsigned int mx, unsigned int my )
{
    for ( unsigned int i = 0; i < frames.count(); i++ ) {
	if ( frames.at( i )->contains( QPoint( mx, my ) ) ) return true;
    }

    return false;
}

/*================================================================*/
int KWFrameSet::selectFrame( unsigned int mx, unsigned int my, bool simulate )
{
    for ( unsigned int i = 0; i < frames.count(); i++ ) {
	if ( frames.at( i )->contains( QPoint( mx, my ) ) ) {
	    int r = 1;
	    if ( frames.at( i )->isSelected() )
		r = 2;
	    if ( !simulate )
		frames.at( i )->setSelected( true );
	    return r;
	}
    }
    return 0;
}

/*================================================================*/
void KWFrameSet::deSelectFrame( unsigned int mx, unsigned int my )
{
    for ( unsigned int i = 0; i < frames.count(); i++ ) {
	if ( frames.at( i )->contains( QPoint( mx, my ) ) )
	    frames.at( i )->setSelected( false );
    }
}

/*================================================================*/
QCursor KWFrameSet::getMouseCursor( unsigned int mx, unsigned int my )
{
    int frame = getFrame( mx, my );

    if ( frame == -1 )
	return Qt::arrowCursor;

    if ( !getFrame( frame )->isSelected() && !grpMgr )
	return Qt::arrowCursor;

    return getFrame( frame )->getMouseCursor( mx, my, grpMgr ? true : false );
}

/*================================================================*/
void KWFrameSet::save( ostream &out )
{
    KWFrame *frame;

    for ( unsigned int i = 0; i < frames.count(); i++ ) {
	frame = getFrame( i );
	out << indent << "<FRAME left=\"" << frame->left() << "\" top=\"" << frame->top()
	    << "\" right=\"" << frame->right() << "\" bottom=\"" << frame->bottom()
	    << "\" runaround=\"" << static_cast<int>( frame->getRunAround() )
	    << "\" runaGapPT=\"" << frame->getRunAroundGap().pt()
	    << "\" runaGapMM=\"" << frame->getRunAroundGap().mm()
	    << "\" runaGapINCH=\"" << frame->getRunAroundGap().inch() << "\" "
	    << correctQString( frame->leftBrd2String() ).latin1() << correctQString( frame->rightBrd2String() ).latin1()
	    << correctQString( frame->topBrd2String() ).latin1()
	    << correctQString( frame->bottomBrd2String() ).latin1() << "bkRed=\""
	    << frame->getBackgroundColor().color().red()
	    << "\" bkGreen=\"" << frame->getBackgroundColor().color().green()
	    << "\" bkBlue=\"" << frame->getBackgroundColor().color().blue()

	    << "\" bleftpt=\"" << frame->getBLeft().pt() << "\" bleftmm=\"" << frame->getBLeft().mm()
	    << "\" bleftinch=\"" << frame->getBLeft().inch()

	    << "\" brightpt=\"" << frame->getBRight().pt() << "\" brightmm=\"" << frame->getBRight().mm()
	    << "\" brightinch=\"" << frame->getBRight().inch()

	    << "\" btoppt=\"" << frame->getBTop().pt() << "\" btopmm=\"" << frame->getBTop().mm()
	    << "\" btopinch=\"" << frame->getBTop().inch()

	    << "\" bbottompt=\"" << frame->getBBottom().pt() << "\" bbottommm=\"" << frame->getBBottom().mm()
	    << "\" bbottominch=\"" << frame->getBBottom().inch()

	    << "\"/>" << endl;
    }
}

/*================================================================*/
int KWFrameSet::getNext( QRect _rect )
{
    for ( unsigned int i = 0; i < frames.count(); i++ ) {
	if ( frames.at( i )->intersects( _rect ) )
	    return i;
    }

    return -1;
}

/*================================================================*/
bool KWFrameSet::hasSelectedFrame()
{
    for ( unsigned int i = 0; i < frames.count(); i++ ) {
	if ( frames.at( i )->isSelected() )
	    return true;
    }

    return false;
}

/******************************************************************/
/* Class: KWTextFrameSet					  */
/******************************************************************/

/*================================================================*/
void KWTextFrameSet::init()
{
    parags = 0L;

    autoCreateNewFrame = true;

    parags = new KWParag( this, doc, 0L, 0L, doc->getDefaultParagLayout() );
    KWFormat *format = new KWFormat( doc );
    format->setDefaults( doc );

    updateCounters();
}

/*================================================================*/
void KWTextFrameSet::assign( KWTextFrameSet *fs )
{
    if ( parags )
	delete parags;

    //parags = fs->getFirstParag();

    parags = new KWParag( *fs->getFirstParag() );
    parags->setFrameSet( this );
    parags->setPrev( 0L );
    parags->setDocument( doc );

    KWParag *p1 = fs->getFirstParag()->getNext(), *p2 = parags, *tmp;
    while ( p1 ) {
	tmp = p2;
	p2 = new KWParag( *p1 );
	//*p2 = *p1;
	tmp->setNext( p2 );
	p2->setPrev( tmp );
	p2->setFrameSet( this );
	p2->setDocument( doc );
	tmp = p2;

	p1 = p1->getNext();
    }

    p2->setNext( 0L );

    getFrame( 0 )->setBackgroundColor( fs->getFrame( 0 )->getBackgroundColor() );
    getFrame( 0 )->setLeftBorder( fs->getFrame( 0 )->getLeftBorder2() );
    getFrame( 0 )->setRightBorder( fs->getFrame( 0 )->getRightBorder2() );
    getFrame( 0 )->setTopBorder( fs->getFrame( 0 )->getTopBorder2() );
    getFrame( 0 )->setBottomBorder( fs->getFrame( 0 )->getBottomBorder2() );
    getFrame( 0 )->setBLeft( fs->getFrame( 0 )->getBLeft() );
    getFrame( 0 )->setBRight( fs->getFrame( 0 )->getBRight() );
    getFrame( 0 )->setBTop( fs->getFrame( 0 )->getBTop() );
    getFrame( 0 )->setBBottom( fs->getFrame( 0 )->getBBottom() );
}

/*================================================================*/
KWTextFrameSet::~KWTextFrameSet()
{
    KWParag *p = getLastParag();

    while ( p != parags )
    {
	p = p->getPrev();
	delete p->getNext();
	p->setNext( 0L );
    }

    delete parags;
    parags = 0L;
}

/*================================================================*/
void KWTextFrameSet::update()
{
    typedef QList<KWFrame> FrameList;
    QList<FrameList> frameList;
    frameList.setAutoDelete( true );

    QRect pageRect;
    for ( unsigned int i = 0; i < static_cast<unsigned int>( doc->getPages() + 1 ); i++ ) {
	pageRect = QRect( 0, i * doc->getPTPaperHeight(), doc->getPTPaperWidth(), doc->getPTPaperHeight() );

	FrameList *l = new FrameList();
	l->setAutoDelete( false );
	for ( unsigned int j = 0; j < frames.count(); j++ ) {
	    if ( frames.at( j )->intersects( pageRect ) ) {
		frames.at( j )->setPageNum( i );
		l->append( frames.at( j ) );
	    }
	}

	if ( !l->isEmpty() ) {
	    FrameList *ll = new FrameList();
	    ll->setAutoDelete( false );
	    ll->append( l->first() );
	    unsigned int k = 0, m = 0;
	    for ( k = 1; k < l->count(); k++ ) {
		bool inserted = false;
		for ( m = 0; m < ll->count(); m++ ) {
		    if ( l->at( k )->y() < ll->at( m )->y() ) {
			inserted = true;
			ll->insert( m, l->at( k ) );
			break;
		    }
		}
		if ( !inserted ) ll->append( l->at( k ) );
	    }
	    FrameList *l2 = new FrameList();
	    l2->setAutoDelete( false );
	    l2->append( ll->first() );
	    for ( k = 1; k < ll->count(); k++ ) {
		bool inserted = false;
		for ( m = 0; m < l2->count(); m++ ) {
		    if ( ll->at( k )->x() < l2->at( m )->x() ) {
			inserted = true;
			l2->insert( m, ll->at( k ) );
			break;
		    }
		}
		if ( !inserted ) l2->append( ll->at( k ) );
	    }

	    delete ll;
	    delete l;
	    l = l2;
	}

	frameList.append( l );
    }

    frames.setAutoDelete( false );
    frames.clear();

    int rm = 0;
    for ( unsigned int n = 0; n < frameList.count(); n++ ) {
	for ( unsigned int o = 0; o < frameList.at( n )->count(); o++ ) {
	    frames.append( frameList.at( n )->at( o ) );
	    frames.at( frames.count() - 1 )->setMostRight( false );
	    if ( frames.count() > 1 ) {
		if ( frames.at( frames.count() - 2 )->right() > frames.at( frames.count() - 1 )->right() ) {
		    frames.at( frames.count() - 2 )->setMostRight( true );
		    rm++;
		}
	    }
	}
    }

    frames.setAutoDelete( true );
}

/*================================================================*/
void KWTextFrameSet::setFirstParag( KWParag *_parag )
{
    parags = _parag;
}

/*================================================================*/
KWParag* KWTextFrameSet::getFirstParag()
{
    return parags;
}

/*================================================================*/
bool KWTextFrameSet::isPTYInFrame( unsigned int _frame, unsigned int _ypos )
{
    KWFrame *frame = getFrame( _frame );
    return ( static_cast<int>( _ypos ) >= static_cast<int>( frame->top() + frame->getBTop().pt() ) &&
	     static_cast<int>( _ypos ) <= static_cast<int>( frame->bottom() - frame->getBBottom().pt() ) );
}

/*================================================================*/
void KWTextFrameSet::deleteParag( KWParag *_parag )
{
    if ( _parag->getInfo() == KWParag::PI_FOOTNOTE )
	return;

    KWParag *p, *p2;

    if ( !getFirstParag()->getPrev() && !getFirstParag()->getNext() )
	return;

    if ( !_parag->getPrev() ) {
	p = _parag->getNext();
	p->setPrev( 0L );
	setFirstParag( p );
	delete _parag;
    } else {
	p = _parag->getNext();
	p2 = _parag->getPrev();
	if ( p ) p->setPrev( p2 );
	p2->setNext( p );
	delete _parag;
    }

    updateCounters();
}

/*================================================================*/
void KWTextFrameSet::joinParag( KWParag *_parag1, KWParag *_parag2 )
{
    if ( !_parag1 || !_parag2 ) return;

    if ( _parag2->getNext() ) _parag2->getNext()->setPrev( _parag1 );
    _parag1->setNext( _parag2->getNext() );

    _parag1->appendText( _parag2->getText(), _parag2->getTextLen() );

    delete _parag2;

    updateCounters();
}

/*================================================================*/
void KWTextFrameSet::insertParag( KWParag *_parag, InsertPos _pos )
{
    KWParag *_new = 0L, *_prev = 0L, *_next = 0L;

    if ( _parag ) {
	_prev = _parag->getPrev();
	_next = _parag->getNext();
    }

    switch ( _pos ) {
    case I_AFTER: {
	_new = new KWParag( this, doc, _parag, _next,
			    doc->findParagLayout( _parag->getParagLayout()->getFollowingParagLayout() ) );
	if ( _new->getParagLayout()->getName() == _parag->getParagLayout()->getName() )
	    _new->setParagLayout( _parag->getParagLayout() );
	if ( _next ) _next->setPrev( _new );
    } break;
    case I_BEFORE: {
	_new = new KWParag( this, doc, _prev, _parag, _parag->getParagLayout() );
	if ( _parag ) _parag->setPrev( _new );
	if ( !_prev ) setFirstParag( _new );
    } break;
    }

    updateCounters();
}

/*================================================================*/
void KWTextFrameSet::splitParag( KWParag *_parag, unsigned int _pos )
{
    KWParag *_new = 0, *_next = 0;

    unsigned int len = _parag->getTextLen() - _pos;
    KWChar* _string = _parag->getKWString()->split( _pos );
    if ( _parag ) 
	_next = _parag->getNext();

    _new = new KWParag( this, doc, _parag, _next, _parag->getParagLayout() );
    if ( _next ) {
 	_next->setPrev( _new );
	_new->setNext( _next );
    } else {
	_new->setNext( 0 );
    }

    _new->appendText( _string, len );

    updateCounters();
}

/*================================================================*/
void KWTextFrameSet::save( ostream &out )
{
    QString grp = "";
    if ( grpMgr ) {
	grp = "\" grpMgr=\"";
	grp += correctQString( grpMgr->getName() );

	unsigned int _row = 0, _col = 0;
	grpMgr->getFrameSet( this, _row, _col );
	KWGroupManager::Cell *cell = grpMgr->getCell( _row, _col );
	QString tmp = "";
	tmp.sprintf( "\" row=\"%d\" col=\"%d\" rows=\"%d\" cols=\"%d", _row, _col, cell->rows, cell->cols  );
	grp += tmp.copy();
    }

    out << otag << "<FRAMESET frameType=\"" << static_cast<int>( getFrameType() )
	<< "\" autoCreateNewFrame=\"" << autoCreateNewFrame << "\" frameInfo=\""
	<< static_cast<int>( frameInfo ) << correctQString( grp ).latin1() << "\" removeable=\""
	<< static_cast<int>( removeableHeader )
	<< "\" visible=\"" << static_cast<int>( visible ) << "\" name=\"" << correctQString( name ).latin1()
	<< "\">" << endl;

    KWFrameSet::save( out );

    KWParag *parag = getFirstParag();
    while ( parag ) {
	out << otag << "<PARAGRAPH>" << endl;
	parag->save( out );
	parag = parag->getNext();
	out << etag << "</PARAGRAPH>" << endl;
    }

    out << etag << "</FRAMESET>" << endl;
}

/*================================================================*/
void KWTextFrameSet::load( KOMLParser& parser, vector<KOMLAttrib>& lst )
{
    init();

    string tag;
    string name;

    KWParag *last = 0L;

    while ( parser.open( 0L, tag ) ) {
	KOMLParser::parseTag( tag.c_str(), name, lst );

	// paragraph
	if ( name == "PARAGRAPH" ) {
	    KOMLParser::parseTag( tag.c_str(), name, lst );
	    vector<KOMLAttrib>::const_iterator it = lst.begin();
	    for( ; it != lst.end(); it++ ) {
	    }

	    if ( !last ) {
		delete parags;
		parags = new KWParag( this, doc, 0L, 0L, doc->getDefaultParagLayout() );
		if ( doc->getNumFrameSets() == 0 ) {
		    KWFormat *format = new KWFormat( doc );
		    format->setDefaults( doc );
		    parags->setFormat( 0, 1, *format );
		}
		parags->load( parser, lst );
		last = parags;
	    } else {
		last = new KWParag( this, doc, last, 0L, doc->getDefaultParagLayout() );
		last->load( parser, lst );
	    }
	} else if ( name == "FRAME" ) {
	    KWFrame rect;
	    KWParagLayout::Border l, r, t, b;
	    float lmm = 0, linch = 0, rmm = 0, rinch = 0, tmm = 0, tinch = 0, bmm = 0, binch = 0, ramm = 0, rainch = -1;
	    unsigned int lpt = 0, rpt = 0, tpt = 0, bpt = 0, rapt = 0;

	    l.color = Qt::white;
	    l.style = KWParagLayout::SOLID;
	    l.ptWidth = 1;
	    r.color = Qt::white;
	    r.style = KWParagLayout::SOLID;
	    r.ptWidth = 1;
	    t.color = Qt::white;
	    t.style = KWParagLayout::SOLID;
	    t.ptWidth = 1;
	    b.color = Qt::white;
	    b.style = KWParagLayout::SOLID;
	    b.ptWidth = 1;
	    QColor c( Qt::white );

	    KOMLParser::parseTag( tag.c_str(), name, lst );
	    vector<KOMLAttrib>::const_iterator it = lst.begin();
	    for( ; it != lst.end(); it++ ) {
		if ( ( *it ).m_strName == "left" )
		    rect.setLeft( atoi( ( *it ).m_strValue.c_str() ) );
		else if ( ( *it ).m_strName == "top" )
		    rect.setTop( atoi( ( *it ).m_strValue.c_str() ) );
		else if ( ( *it ).m_strName == "right" )
		    rect.setRight( atoi( ( *it ).m_strValue.c_str() ) );
		else if ( ( *it ).m_strName == "bottom" )
		    rect.setBottom( atoi( ( *it ).m_strValue.c_str() ) );
		else if ( ( *it ).m_strName == "runaround" )
		    rect.setRunAround( static_cast<RunAround>( atoi( ( *it ).m_strValue.c_str() ) ) );
		else if ( ( *it ).m_strName == "runaroundGap" )
		    rect.setRunAroundGap( KWUnit( atof( ( *it ).m_strValue.c_str() ) ) );
		else if ( ( *it ).m_strName == "runaGapPT" )
		    rapt = atoi( ( *it ).m_strValue.c_str() );
		else if ( ( *it ).m_strName == "runaGapMM" )
		    ramm = atof( ( *it ).m_strValue.c_str() );
		else if ( ( *it ).m_strName == "runaGapINCH" )
		    rainch = atof( ( *it ).m_strValue.c_str() );
		else if ( ( *it ).m_strName == "lWidth" )
		    l.ptWidth = atoi( ( *it ).m_strValue.c_str() );
		else if ( ( *it ).m_strName == "rWidth" )
		    r.ptWidth = atoi( ( *it ).m_strValue.c_str() );
		else if ( ( *it ).m_strName == "tWidth" )
		    t.ptWidth = atoi( ( *it ).m_strValue.c_str() );
		else if ( ( *it ).m_strName == "bWidth" )
		    b.ptWidth = atoi( ( *it ).m_strValue.c_str() );
		else if ( ( *it ).m_strName == "lRed" )
		    l.color.setRgb( atoi( ( *it ).m_strValue.c_str() ), l.color.green(), l.color.blue() );
		else if ( ( *it ).m_strName == "rRed" )
		    r.color.setRgb( atoi( ( *it ).m_strValue.c_str() ), r.color.green(), r.color.blue() );
		else if ( ( *it ).m_strName == "tRed" )
		    t.color.setRgb( atoi( ( *it ).m_strValue.c_str() ), t.color.green(), t.color.blue() );
		else if ( ( *it ).m_strName == "bRed" )
		    b.color.setRgb( atoi( ( *it ).m_strValue.c_str() ), b.color.green(), b.color.blue() );
		else if ( ( *it ).m_strName == "lGreen" )
		    l.color.setRgb( l.color.red(), atoi( ( *it ).m_strValue.c_str() ), l.color.blue() );
		else if ( ( *it ).m_strName == "rGreen" )
		    r.color.setRgb( r.color.red(), atoi( ( *it ).m_strValue.c_str() ), r.color.blue() );
		else if ( ( *it ).m_strName == "tGreen" )
		    t.color.setRgb( t.color.red(), atoi( ( *it ).m_strValue.c_str() ), t.color.blue() );
		else if ( ( *it ).m_strName == "bGreen" )
		    b.color.setRgb( b.color.red(), atoi( ( *it ).m_strValue.c_str() ), b.color.blue() );
		else if ( ( *it ).m_strName == "lBlue" )
		    l.color.setRgb( l.color.red(), l.color.green(), atoi( ( *it ).m_strValue.c_str() ) );
		else if ( ( *it ).m_strName == "rBlue" )
		    r.color.setRgb( r.color.red(), r.color.green(), atoi( ( *it ).m_strValue.c_str() ) );
		else if ( ( *it ).m_strName == "tBlue" )
		    t.color.setRgb( t.color.red(), t.color.green(), atoi( ( *it ).m_strValue.c_str() ) );
		else if ( ( *it ).m_strName == "bBlue" )
		    b.color.setRgb( b.color.red(), b.color.green(), atoi( ( *it ).m_strValue.c_str() ) );
		else if ( ( *it ).m_strName == "lStyle" )
		    l.style = static_cast<KWParagLayout::BorderStyle>( atoi( ( *it ).m_strValue.c_str() ) );
		else if ( ( *it ).m_strName == "rStyle" )
		    r.style = static_cast<KWParagLayout::BorderStyle>( atoi( ( *it ).m_strValue.c_str() ) );
		else if ( ( *it ).m_strName == "tStyle" )
		    t.style = static_cast<KWParagLayout::BorderStyle>( atoi( ( *it ).m_strValue.c_str() ) );
		else if ( ( *it ).m_strName == "bStyle" )
		    b.style = static_cast<KWParagLayout::BorderStyle>( atoi( ( *it ).m_strValue.c_str() ) );
		else if ( ( *it ).m_strName == "bkRed" )
		    c.setRgb( atoi( ( *it ).m_strValue.c_str() ), c.green(), c.blue() );
		else if ( ( *it ).m_strName == "bkGreen" )
		    c.setRgb( c.red(), atoi( ( *it ).m_strValue.c_str() ), c.blue() );
		else if ( ( *it ).m_strName == "bkBlue" )
		    c.setRgb( c.red(), c.green(), atoi( ( *it ).m_strValue.c_str() ) );
		else if ( ( *it ).m_strName == "bleftpt" )
		    lpt = atoi( ( *it ).m_strValue.c_str() );
		else if ( ( *it ).m_strName == "brightpt" )
		    rpt = atoi( ( *it ).m_strValue.c_str() );
		else if ( ( *it ).m_strName == "btoppt" )
		    tpt = atoi( ( *it ).m_strValue.c_str() );
		else if ( ( *it ).m_strName == "bbottompt" )
		    bpt = atoi( ( *it ).m_strValue.c_str() );
		else if ( ( *it ).m_strName == "bleftmm" )
		    lmm = atof( ( *it ).m_strValue.c_str() );
		else if ( ( *it ).m_strName == "brightmm" )
		    rmm = atof( ( *it ).m_strValue.c_str() );
		else if ( ( *it ).m_strName == "btopmm" )
		    tmm = atof( ( *it ).m_strValue.c_str() );
		else if ( ( *it ).m_strName == "bbottommm" )
		    bmm = atof( ( *it ).m_strValue.c_str() );
		else if ( ( *it ).m_strName == "bleftinch" )
		    linch = atof( ( *it ).m_strValue.c_str() );
		else if ( ( *it ).m_strName == "brightinch" )
		    rinch = atof( ( *it ).m_strValue.c_str() );
		else if ( ( *it ).m_strName == "btopinch" )
		    tinch = atof( ( *it ).m_strValue.c_str() );
		else if ((*it).m_strName == "bbottominch")
		    binch = atof( ( *it ).m_strValue.c_str() );
	    }
	    KWFrame *_frame = new KWFrame( rect.x(), rect.y(), rect.width(), rect.height(), rect.getRunAround(),
					   rainch == -1 ? rect.getRunAroundGap() : KWUnit( rapt, ramm, rainch ) );
	    _frame->setLeftBorder( l );
	    _frame->setRightBorder( r );
	    _frame->setTopBorder( t );
	    _frame->setBottomBorder( b );
	    _frame->setBackgroundColor( QBrush( c ) );
	    _frame->setBLeft( KWUnit( lpt, lmm, linch ) );
	    _frame->setBRight( KWUnit( rpt, rmm, rinch ) );
	    _frame->setBTop( KWUnit( tpt, tmm, tinch ) );
	    _frame->setBBottom( KWUnit( bpt, bmm, binch ) );
	    frames.append( _frame );
	} else
	    cerr << "Unknown tag '" << tag << "' in FRAMESET" << endl;

	if ( !parser.close( tag ) ) {
	    cerr << "ERR: Closing Child" << endl;
	    return;
	}
    }

    updateCounters();
}

/*================================================================*/
void KWTextFrameSet::updateCounters()
{
    KWParag *p = getFirstParag();

    int counterData[ 16 ], listData[ 16 ];
    unsigned int i = 0;
    for ( i = 0; i < 16; i++ ) {
	counterData[ i ] = -2;
	listData[ i ] = -2;
    }
    KWParagLayout::CounterType ct = KWParagLayout::CT_NONE;

    while ( p ) {
	if ( p->getParagLayout()->getCounterType() != KWParagLayout::CT_NONE ) {
	    if ( p->getParagLayout()->getNumberingType() == KWParagLayout::NT_CHAPTER ) {
		counterData[ p->getParagLayout()->getCounterDepth() ]++;
		for ( i = 0; i < 16; i++ ) {
		    if ( counterData[ i ] < 0 ) {
			// Reggie: I (Jost) changed this. I think startCounter should be the same
			// value, not depending on the numbering type. That caused problems.
			counterData[ i ] = atoi( p->getParagLayout()->getStartCounter() );
			/* switch ( p->getParagLayout()->getCounterType() ) {
			case KWParagLayout::CT_NUM: case KWParagLayout::CT_ROM_NUM_L: 
			case KWParagLayout::CT_ROM_NUM_U:
			    counterData[ i ] = atoi( p->getParagLayout()->getStartCounter() );
			    break;
			case KWParagLayout::CT_ALPHAB_L:
			    counterData[ i ] = atoi( p->getParagLayout()->getStartCounter() ); // TEST p->getParagLayout()->getStartCounter()[ 0 ].unicode();
			    break;
			case KWParagLayout::CT_ALPHAB_U:
			    counterData[ i ] = atoi( p->getParagLayout()->getStartCounter() ); // TEST p->getParagLayout()->getStartCounter()[ 0 ].unicode();
			    break;
			default: break;
			}*/
		    }
		    p->getCounterData()[ i ] = counterData[ i ];
		}
		p->makeCounterText();
		for ( i = p->getParagLayout()->getCounterDepth() + 1; i < 16; i++ )
		    counterData[ i ] = -2;
		if ( listData[ 0 ] != -2 ) {
		    for ( i = 0; i < 16; i++ )
			listData[ i ] = -2;
		}
	    } else {
		if ( ct != p->getParagLayout()->getCounterType() ) {
		    for ( i = 0; i < 16; i++ )
			listData[ i ] = -2;
		}
		ct = p->getParagLayout()->getCounterType();
		if ( p->getParagLayout()->getCounterType() != KWParagLayout::CT_BULLET ) {
		    listData[ p->getParagLayout()->getCounterDepth() ]++;
		} else if ( listData[ 0 ] != -2 ) {
		    for ( i = 0; i < 16; i++ )
			listData[ i ] = -2;
		}
		for ( i = 0; i < 16; i++ ) {
		    if ( listData[ i ] < 0 ) {
			switch ( p->getParagLayout()->getCounterType() ) {
			case KWParagLayout::CT_NUM: case KWParagLayout::CT_ROM_NUM_L: 
			case KWParagLayout::CT_ROM_NUM_U:
			    listData[ i ] = atoi( p->getParagLayout()->getStartCounter() );
			    break;
			case KWParagLayout::CT_ALPHAB_L:
			    listData[ i ] = p->getParagLayout()->getStartCounter()[ 0 ].unicode();
			    break;
			case KWParagLayout::CT_ALPHAB_U:
			    listData[ i ] = p->getParagLayout()->getStartCounter()[ 0 ].unicode();
			    break;
			default: break;
			}
		    }
		    p->getCounterData()[ i ] = listData[ i ];
		}
		p->makeCounterText();
		for ( i = p->getParagLayout()->getCounterDepth() + 1; i < 16; i++ )
		    listData[ i ] = -2;
	    }
	} else if ( listData[ 0 ] != -2 ) {
	    for ( i = 0; i < 16; i++ )
		listData[ i ] = -2;
	}
	p = p->getNext();
    }
}

/*================================================================*/
void KWTextFrameSet::updateAllStyles()
{
    KWParag *p = getFirstParag();

    while ( p ) {
	if ( doc->isStyleChanged( p->getParagLayout()->getName() ) )
	    p->applyStyle( p->getParagLayout()->getName() );
	p = p->getNext();
    }

    updateCounters();
}

/*================================================================*/
KWParag *KWTextFrameSet::getLastParag()
{
    KWParag *p = getFirstParag();
    KWParag *last = p;

    while ( p ) {
	last = p;
	p = p->getNext();
    }

    return last;
}

/******************************************************************/
/* Class: KWPictureFrameSet					  */
/******************************************************************/

/*================================================================*/
void KWPictureFrameSet::setFileName( QString _filename )
{
    int dashdash = _filename.findRev( "--" );
    if ( dashdash != -1 )
	_filename == _filename.left( dashdash );

    if ( image ) {
	image->decRef();
	image = 0L;
    }

    filename = _filename;

    bool del = false;
    KWImage *_image = doc->getImageCollection()->getImage( filename );
    if ( !_image ) {
	del = true;
	_image = new KWImage( doc, filename );
    }

    QString key;
    image = doc->getImageCollection()->getImage( *_image, key );

    if ( del )
	delete _image;
}

/*================================================================*/
void KWPictureFrameSet::setFileName( QString _filename, QSize _imgSize )
{
    int dashdash = _filename.findRev( "--" );
    if ( dashdash != -1 )
	_filename == _filename.left( dashdash );

    if ( image ) {
	image->decRef();
	image = 0L;
    }

    filename = _filename;

    bool del = false;
    KWImage *_image = doc->getImageCollection()->getImage( filename );
    if ( !_image ) {
	del = true;
	_image = new KWImage( doc, filename );
    }

    QString key;
    image = doc->getImageCollection()->getImage( *_image, key, _imgSize );

    if ( del )
	delete _image;
}

/*================================================================*/
void KWPictureFrameSet::setSize( QSize _imgSize )
{
    if ( image && _imgSize == image->size() ) return;

    QString key;
    image = doc->getImageCollection()->getImage( *image, key, _imgSize );
    if ( !image )
	setFileName( filename, _imgSize );
}

/*================================================================*/
void KWPictureFrameSet::save( ostream &out )
{
    out << otag << "<FRAMESET frameType=\"" << static_cast<int>( getFrameType() ) << "\" frameInfo=\""
	<< static_cast<int>( frameInfo ) << "\">" << endl;

    KWFrameSet::save( out );

    out << otag << "<IMAGE>" << endl;
    image->save( out );
    out << etag << "</IMAGE>" << endl;

    out << etag << "</FRAMESET>" << endl;
}

/*================================================================*/
void KWPictureFrameSet::load( KOMLParser& parser, vector<KOMLAttrib>& lst )
{
    string tag;
    string name;

    while ( parser.open( 0L, tag ) ) {
	KOMLParser::parseTag( tag.c_str(), name, lst );

	if ( name == "IMAGE" ) {
	    KOMLParser::parseTag( tag.c_str(), name, lst );
	    vector<KOMLAttrib>::const_iterator it = lst.begin();
	    for( ; it != lst.end(); it++ ) {
	    }

	    KWImage *_image = new KWImage();
	    _image->load( parser, lst, doc );
	    //setFileName( _image->getFilename() );
	    doc->addImageRequest( _image->getFilename(), this );
	    delete _image;
	} else if ( name == "FRAME" ) {
	    KWFrame rect;
	    KWParagLayout::Border l, r, t, b;
	    float lmm = 0, linch = 0, rmm = 0, rinch = 0, tmm = 0, tinch = 0, bmm = 0, binch = 0, ramm = 0, rainch = -1;
	    unsigned int lpt = 0, rpt = 0, tpt = 0, bpt = 0, rapt = 0;

	    l.color = Qt::white;
	    l.style = KWParagLayout::SOLID;
	    l.ptWidth = 1;
	    r.color = Qt::white;
	    r.style = KWParagLayout::SOLID;
	    r.ptWidth = 1;
	    t.color = Qt::white;
	    t.style = KWParagLayout::SOLID;
	    t.ptWidth = 1;
	    b.color = Qt::white;
	    b.style = KWParagLayout::SOLID;
	    b.ptWidth = 1;
	    QColor c( Qt::white );

	    KOMLParser::parseTag( tag.c_str(), name, lst );
	    vector<KOMLAttrib>::const_iterator it = lst.begin();
	    for( ; it != lst.end(); it++ ) {
		if ( ( *it ).m_strName == "left" )
		    rect.setLeft( atoi( ( *it ).m_strValue.c_str() ) );
		else if ( ( *it ).m_strName == "top" )
		    rect.setTop( atoi( ( *it ).m_strValue.c_str() ) );
		else if ( ( *it ).m_strName == "right" )
		    rect.setRight( atoi( ( *it ).m_strValue.c_str() ) );
		else if ( ( *it ).m_strName == "bottom" )
		    rect.setBottom( atoi( ( *it ).m_strValue.c_str() ) );
		else if ( ( *it ).m_strName == "runaround" )
		    rect.setRunAround( static_cast<RunAround>( atoi( ( *it ).m_strValue.c_str() ) ) );
		else if ( ( *it ).m_strName == "runaroundGap" )
		    rect.setRunAroundGap( KWUnit( atof( ( *it ).m_strValue.c_str() ) ) );
		else if ( ( *it ).m_strName == "runaGapPT" )
		    rapt = atoi( ( *it ).m_strValue.c_str() );
		else if ( ( *it ).m_strName == "runaGapMM" )
		    ramm = atof( ( *it ).m_strValue.c_str() );
		else if ( ( *it ).m_strName == "runaGapINCH" )
		    rainch = atof( ( *it ).m_strValue.c_str() );
		else if ( ( *it ).m_strName == "lWidth" )
		    l.ptWidth = atoi( ( *it ).m_strValue.c_str() );
		else if ( ( *it ).m_strName == "rWidth" )
		    r.ptWidth = atoi( ( *it ).m_strValue.c_str() );
		else if ( ( *it ).m_strName == "tWidth" )
		    t.ptWidth = atoi( ( *it ).m_strValue.c_str() );
		else if ( ( *it ).m_strName == "bWidth" )
		    b.ptWidth = atoi( ( *it ).m_strValue.c_str() );
		else if ( ( *it ).m_strName == "lRed" )
		    l.color.setRgb( atoi( ( *it ).m_strValue.c_str() ), l.color.green(), l.color.blue() );
		else if ( ( *it ).m_strName == "rRed" )
		    r.color.setRgb( atoi( ( *it ).m_strValue.c_str() ), r.color.green(), r.color.blue() );
		else if ( ( *it ).m_strName == "tRed" )
		    t.color.setRgb( atoi( ( *it ).m_strValue.c_str() ), t.color.green(), t.color.blue() );
		else if ( ( *it ).m_strName == "bRed" )
		    b.color.setRgb( atoi( ( *it ).m_strValue.c_str() ), b.color.green(), b.color.blue() );
		else if ( ( *it ).m_strName == "lGreen" )
		    l.color.setRgb( l.color.red(), atoi( ( *it ).m_strValue.c_str() ), l.color.blue() );
		else if ( ( *it ).m_strName == "rGreen" )
		    r.color.setRgb( r.color.red(), atoi( ( *it ).m_strValue.c_str() ), r.color.blue() );
		else if ( ( *it ).m_strName == "tGreen" )
		    t.color.setRgb( t.color.red(), atoi( ( *it ).m_strValue.c_str() ), t.color.blue() );
		else if ( ( *it ).m_strName == "bGreen" )
		    b.color.setRgb( b.color.red(), atoi( ( *it ).m_strValue.c_str() ), b.color.blue() );
		else if ( ( *it ).m_strName == "lBlue" )
		    l.color.setRgb( l.color.red(), l.color.green(), atoi( ( *it ).m_strValue.c_str() ) );
		else if ( ( *it ).m_strName == "rBlue" )
		    r.color.setRgb( r.color.red(), r.color.green(), atoi( ( *it ).m_strValue.c_str() ) );
		else if ( ( *it ).m_strName == "tBlue" )
		    t.color.setRgb( t.color.red(), t.color.green(), atoi( ( *it ).m_strValue.c_str() ) );
		else if ( ( *it ).m_strName == "bBlue" )
		    b.color.setRgb( b.color.red(), b.color.green(), atoi( ( *it ).m_strValue.c_str() ) );
		else if ( ( *it ).m_strName == "lStyle" )
		    l.style = static_cast<KWParagLayout::BorderStyle>( atoi( ( *it ).m_strValue.c_str() ) );
		else if ( ( *it ).m_strName == "rStyle" )
		    r.style = static_cast<KWParagLayout::BorderStyle>( atoi( ( *it ).m_strValue.c_str() ) );
		else if ( ( *it ).m_strName == "tStyle" )
		    t.style = static_cast<KWParagLayout::BorderStyle>( atoi( ( *it ).m_strValue.c_str() ) );
		else if ( ( *it ).m_strName == "bStyle" )
		    b.style = static_cast<KWParagLayout::BorderStyle>( atoi( ( *it ).m_strValue.c_str() ) );
		else if ( ( *it ).m_strName == "bkRed" )
		    c.setRgb( atoi( ( *it ).m_strValue.c_str() ), c.green(), c.blue() );
		else if ( ( *it ).m_strName == "bkGreen" )
		    c.setRgb( c.red(), atoi( ( *it ).m_strValue.c_str() ), c.blue() );
		else if ( ( *it ).m_strName == "bkBlue" )
		    c.setRgb( c.red(), c.green(), atoi( ( *it ).m_strValue.c_str() ) );
		else if ( ( *it ).m_strName == "bleftpt" )
		    lpt = atoi( ( *it ).m_strValue.c_str() );
		else if ( ( *it ).m_strName == "brightpt" )
		    rpt = atoi( ( *it ).m_strValue.c_str() );
		else if ( ( *it ).m_strName == "btoppt" )
		    tpt = atoi( ( *it ).m_strValue.c_str() );
		else if ( ( *it ).m_strName == "bbottompt" )
		    bpt = atoi( ( *it ).m_strValue.c_str() );
		else if ( ( *it ).m_strName == "bleftmm" )
		    lmm = atof( ( *it ).m_strValue.c_str() );
		else if ( ( *it ).m_strName == "brightmm" )
		    rmm = atof( ( *it ).m_strValue.c_str() );
		else if ( ( *it ).m_strName == "btopmm" )
		    tmm = atof( ( *it ).m_strValue.c_str() );
		else if ( ( *it ).m_strName == "bbottommm" )
		    bmm = atof( ( *it ).m_strValue.c_str() );
		else if ( ( *it ).m_strName == "bleftinch" )
		    linch = atof( ( *it ).m_strValue.c_str() );
		else if ( ( *it ).m_strName == "brightinch" )
		    rinch = atof( ( *it ).m_strValue.c_str() );
		else if ( ( *it ).m_strName == "btopinch" )
		    tinch = atof( ( *it ).m_strValue.c_str() );
		else if ((*it).m_strName == "bbottominch")
		    binch = atof( ( *it ).m_strValue.c_str() );
	    }
	    KWFrame *_frame = new KWFrame( rect.x(), rect.y(), rect.width(), rect.height(), rect.getRunAround(),
					   rainch == -1 ? rect.getRunAroundGap() : KWUnit( rapt, ramm, rainch ) );
	    _frame->setLeftBorder( l );
	    _frame->setRightBorder( r );
	    _frame->setTopBorder( t );
	    _frame->setBottomBorder( b );
	    _frame->setBackgroundColor( QBrush( c ) );
	    _frame->setBLeft( KWUnit( lpt, lmm, linch ) );
	    _frame->setBRight( KWUnit( rpt, rmm, rinch ) );
	    _frame->setBTop( KWUnit( tpt, tmm, tinch ) );
	    _frame->setBBottom( KWUnit( bpt, bmm, binch ) );
	    frames.append( _frame );
	} else
	    cerr << "Unknown tag '" << tag << "' in FRAMESET" << endl;

	if ( !parser.close( tag ) ) {
	    cerr << "ERR: Closing Child" << endl;
	    return;
	}
    }
}

/******************************************************************/
/* Class: KWPartFrameSet					  */
/******************************************************************/

/*================================================================*/
KWPartFrameSet::KWPartFrameSet( KWordDocument *_doc, KWordChild *_child )
    : KWFrameSet( _doc )
{
    child = _child;
    _enableDrawing = true;
    frame = 0L;
}

/*================================================================*/
KWPartFrameSet::~KWPartFrameSet()
{
}

/*================================================================*/
QPicture *KWPartFrameSet::getPicture()
{
    if ( !_enableDrawing )
	return 0;

    update();
    QPainter p( &pic );
    //child->transform( p );

    if ( child )
	child->part()->paintEverything( p, QRect( QPoint( 0, 0 ),
						  QSize( frames.at( 0 )->width(), frames.at( 0 )->height() ) ),
					TRUE, 0 );

    return &pic;
}

/*================================================================*/
void KWPartFrameSet::activate( QWidget *_widget, int diffx, int diffy, int diffxx )
{
//     child->setGeometry( QRect( frames.at( 0 )->x() - diffxx - diffx, frames.at( 0 )->y() - diffy,
// 			       frames.at( 0 )->width(), frames.at( 0 )->height() ) );
    update();
    KWordView *view = (KWordView*)_widget;
    Part* part = child->part();
    if ( !part )
	return;
    view->shell()->setActiveView( view, part );
    view->child( part )->frame()->move( frames.at( 0 )->x() - diffx - diffxx,
					frames.at( 0 )->y() - diffy );
}

/*================================================================*/
void KWPartFrameSet::deactivate()
{
}

/*================================================================*/
void KWPartFrameSet::update()
{
    child->setGeometry( QRect( frames.at( 0 )->x(), frames.at( 0 )->y(),
			       frames.at( 0 )->width(), frames.at( 0 )->height() ) );
}

/*================================================================*/
void KWPartFrameSet::save( ostream &out )
{
    KWFrameSet::save( out );
}

/*================================================================*/
void KWPartFrameSet::load( KOMLParser& parser, vector<KOMLAttrib>& lst )
{
    string tag;
    string name;

    while ( parser.open( 0L, tag ) ) {
	KOMLParser::parseTag( tag.c_str(), name, lst );

	if ( name == "FRAME" ) {
	    KWFrame rect;
	    KWParagLayout::Border l, r, t, b;
	    float lmm = 0, linch = 0, rmm = 0, rinch = 0, tmm = 0, tinch = 0, bmm = 0, binch = 0, ramm = 0, rainch = -1;
	    unsigned int lpt = 0, rpt = 0, tpt = 0, bpt = 0, rapt = 0;

	    l.color = Qt::white;
	    l.style = KWParagLayout::SOLID;
	    l.ptWidth = 1;
	    r.color = Qt::white;
	    r.style = KWParagLayout::SOLID;
	    r.ptWidth = 1;
	    t.color = Qt::white;
	    t.style = KWParagLayout::SOLID;
	    t.ptWidth = 1;
	    b.color = Qt::white;
	    b.style = KWParagLayout::SOLID;
	    b.ptWidth = 1;
	    QColor c( Qt::white );

	    KOMLParser::parseTag( tag.c_str(), name, lst );
	    vector<KOMLAttrib>::const_iterator it = lst.begin();
	    for( ; it != lst.end(); it++ ) {
		if ( ( *it ).m_strName == "left" )
		    rect.setLeft( atoi( ( *it ).m_strValue.c_str() ) );
		else if ( ( *it ).m_strName == "top" )
		    rect.setTop( atoi( ( *it ).m_strValue.c_str() ) );
		else if ( ( *it ).m_strName == "right" )
		    rect.setRight( atoi( ( *it ).m_strValue.c_str() ) );
		else if ( ( *it ).m_strName == "bottom" )
		    rect.setBottom( atoi( ( *it ).m_strValue.c_str() ) );
		else if ( ( *it ).m_strName == "runaround" )
		    rect.setRunAround( static_cast<RunAround>( atoi( ( *it ).m_strValue.c_str() ) ) );
		else if ( ( *it ).m_strName == "runaroundGap" )
		    rect.setRunAroundGap( KWUnit( atof( ( *it ).m_strValue.c_str() ) ) );
		else if ( ( *it ).m_strName == "runaGapPT" )
		    rapt = atoi( ( *it ).m_strValue.c_str() );
		else if ( ( *it ).m_strName == "runaGapMM" )
		    ramm = atof( ( *it ).m_strValue.c_str() );
		else if ( ( *it ).m_strName == "runaGapINCH" )
		    rainch = atof( ( *it ).m_strValue.c_str() );
		else if ( ( *it ).m_strName == "lWidth" )
		    l.ptWidth = atoi( ( *it ).m_strValue.c_str() );
		else if ( ( *it ).m_strName == "rWidth" )
		    r.ptWidth = atoi( ( *it ).m_strValue.c_str() );
		else if ( ( *it ).m_strName == "tWidth" )
		    t.ptWidth = atoi( ( *it ).m_strValue.c_str() );
		else if ( ( *it ).m_strName == "bWidth" )
		    b.ptWidth = atoi( ( *it ).m_strValue.c_str() );
		else if ( ( *it ).m_strName == "lRed" )
		    l.color.setRgb( atoi( ( *it ).m_strValue.c_str() ), l.color.green(), l.color.blue() );
		else if ( ( *it ).m_strName == "rRed" )
		    r.color.setRgb( atoi( ( *it ).m_strValue.c_str() ), r.color.green(), r.color.blue() );
		else if ( ( *it ).m_strName == "tRed" )
		    t.color.setRgb( atoi( ( *it ).m_strValue.c_str() ), t.color.green(), t.color.blue() );
		else if ( ( *it ).m_strName == "bRed" )
		    b.color.setRgb( atoi( ( *it ).m_strValue.c_str() ), b.color.green(), b.color.blue() );
		else if ( ( *it ).m_strName == "lGreen" )
		    l.color.setRgb( l.color.red(), atoi( ( *it ).m_strValue.c_str() ), l.color.blue() );
		else if ( ( *it ).m_strName == "rGreen" )
		    r.color.setRgb( r.color.red(), atoi( ( *it ).m_strValue.c_str() ), r.color.blue() );
		else if ( ( *it ).m_strName == "tGreen" )
		    t.color.setRgb( t.color.red(), atoi( ( *it ).m_strValue.c_str() ), t.color.blue() );
		else if ( ( *it ).m_strName == "bGreen" )
		    b.color.setRgb( b.color.red(), atoi( ( *it ).m_strValue.c_str() ), b.color.blue() );
		else if ( ( *it ).m_strName == "lBlue" )
		    l.color.setRgb( l.color.red(), l.color.green(), atoi( ( *it ).m_strValue.c_str() ) );
		else if ( ( *it ).m_strName == "rBlue" )
		    r.color.setRgb( r.color.red(), r.color.green(), atoi( ( *it ).m_strValue.c_str() ) );
		else if ( ( *it ).m_strName == "tBlue" )
		    t.color.setRgb( t.color.red(), t.color.green(), atoi( ( *it ).m_strValue.c_str() ) );
		else if ( ( *it ).m_strName == "bBlue" )
		    b.color.setRgb( b.color.red(), b.color.green(), atoi( ( *it ).m_strValue.c_str() ) );
		else if ( ( *it ).m_strName == "lStyle" )
		    l.style = static_cast<KWParagLayout::BorderStyle>( atoi( ( *it ).m_strValue.c_str() ) );
		else if ( ( *it ).m_strName == "rStyle" )
		    r.style = static_cast<KWParagLayout::BorderStyle>( atoi( ( *it ).m_strValue.c_str() ) );
		else if ( ( *it ).m_strName == "tStyle" )
		    t.style = static_cast<KWParagLayout::BorderStyle>( atoi( ( *it ).m_strValue.c_str() ) );
		else if ( ( *it ).m_strName == "bStyle" )
		    b.style = static_cast<KWParagLayout::BorderStyle>( atoi( ( *it ).m_strValue.c_str() ) );
		else if ( ( *it ).m_strName == "bkRed" )
		    c.setRgb( atoi( ( *it ).m_strValue.c_str() ), c.green(), c.blue() );
		else if ( ( *it ).m_strName == "bkGreen" )
		    c.setRgb( c.red(), atoi( ( *it ).m_strValue.c_str() ), c.blue() );
		else if ( ( *it ).m_strName == "bkBlue" )
		    c.setRgb( c.red(), c.green(), atoi( ( *it ).m_strValue.c_str() ) );
		else if ( ( *it ).m_strName == "bleftpt" )
		    lpt = atoi( ( *it ).m_strValue.c_str() );
		else if ( ( *it ).m_strName == "brightpt" )
		    rpt = atoi( ( *it ).m_strValue.c_str() );
		else if ( ( *it ).m_strName == "btoppt" )
		    tpt = atoi( ( *it ).m_strValue.c_str() );
		else if ( ( *it ).m_strName == "bbottompt" )
		    bpt = atoi( ( *it ).m_strValue.c_str() );
		else if ( ( *it ).m_strName == "bleftmm" )
		    lmm = atof( ( *it ).m_strValue.c_str() );
		else if ( ( *it ).m_strName == "brightmm" )
		    rmm = atof( ( *it ).m_strValue.c_str() );
		else if ( ( *it ).m_strName == "btopmm" )
		    tmm = atof( ( *it ).m_strValue.c_str() );
		else if ( ( *it ).m_strName == "bbottommm" )
		    bmm = atof( ( *it ).m_strValue.c_str() );
		else if ( ( *it ).m_strName == "bleftinch" )
		    linch = atof( ( *it ).m_strValue.c_str() );
		else if ( ( *it ).m_strName == "brightinch" )
		    rinch = atof( ( *it ).m_strValue.c_str() );
		else if ( ( *it ).m_strName == "btopinch" )
		    tinch = atof( ( *it ).m_strValue.c_str() );
		else if ((*it).m_strName == "bbottominch")
		    binch = atof( ( *it ).m_strValue.c_str() );
	    }
	    KWFrame *_frame = new KWFrame( rect.x(), rect.y(), rect.width(), rect.height(), rect.getRunAround(),
					   rainch == -1 ? rect.getRunAroundGap() : KWUnit( rapt, ramm, rainch ) );
	    _frame->setLeftBorder( l );
	    _frame->setRightBorder( r );
	    _frame->setTopBorder( t );
	    _frame->setBottomBorder( b );
	    _frame->setBackgroundColor( QBrush( c ) );
	    _frame->setBLeft( KWUnit( lpt, lmm, linch ) );
	    _frame->setBRight( KWUnit( rpt, rmm, rinch ) );
	    _frame->setBTop( KWUnit( tpt, tmm, tinch ) );
	    _frame->setBBottom( KWUnit( bpt, bmm, binch ) );
	    frames.append( _frame );
	} else
	    cerr << "Unknown tag '" << tag << "' in FRAMESET" << endl;

	if ( !parser.close( tag ) ) {
	    cerr << "ERR: Closing Child" << endl;
	    return;
	}
    }
}

#include <kformulaedit.h>

/******************************************************************/
/* Class: KWFormulaFrameSet					  */
/******************************************************************/

/*================================================================*/
KWFormulaFrameSet::KWFormulaFrameSet( KWordDocument *_doc, QWidget *parent )
    : KWFrameSet( _doc ), pic( 0 ), font( "times", 12 ), color( Qt::black )

{
    formulaEdit = new KFormulaEdit( ( (QScrollView*)parent )->viewport() );
    ( (QScrollView*)parent )->addChild( formulaEdit );
    formulaEdit->setFont( font );
    if ( pic )
	delete pic;
    pic = new QPicture;
    QPainter p;
    p.begin( pic );
    formulaEdit->getFormula()->setFont( font );
    formulaEdit->getFormula()->setBackColor( Qt::white );
    formulaEdit->getFormula()->setForeColor( color );
    formulaEdit->getFormula()->redraw( p );
    p.end();
    formulaEdit->hide();
}

/*================================================================*/
KWFormulaFrameSet::KWFormulaFrameSet( KWordDocument *_doc )
    : KWFrameSet( _doc ), formulaEdit( 0 ), pic( 0 )
{
}

/*================================================================*/
KWFormulaFrameSet::~KWFormulaFrameSet()
{
    if ( pic )
	delete pic;
}

/*================================================================*/
QPicture *KWFormulaFrameSet::getPicture()
{
    return pic;
}

/*================================================================*/
void KWFormulaFrameSet::setFormat( const QFont &f, const QColor &c )
{
    font = f;
    color = c;
    if ( formulaEdit && formulaEdit->isVisible() ) {
	formulaEdit->getFormula()->setFont( font );
	formulaEdit->getFormula()->setBackColor( frames.at( 0 )->getBackgroundColor().color() );
	formulaEdit->getFormula()->setForeColor( color );
	formulaEdit->getFormula()->makeDirty();
	formulaEdit->redraw( TRUE );
    }
    update();
}

/*================================================================*/
KWFormat *KWFormulaFrameSet::getFormat()
{
    KWFormat *f = new KWFormat( doc );
    f->setUserFont( doc->findUserFont( font.family() ) );
    f->setPTFontSize( font.pointSize() );
    f->setWeight( font.weight() );
    f->setUnderline( font.underline() );
    f->setItalic( font.italic() );
    f->setVertAlign( KWFormat::VA_NORMAL );
    f->setColor( color );

    return f;
}

/*================================================================*/
void KWFormulaFrameSet::activate( QWidget *_widget, int /*diffx*/, int /*diffy*/, int /*diffxx*/ )
{
    if ( formulaEdit->parent() != ( (QScrollView*)_widget )->viewport() )
	formulaEdit->reparent( ( (QScrollView*)_widget )->viewport(), 0, QPoint( 0, 0 ), FALSE );

    formulaEdit->getFormula()->setBackColor( frames.at( 0 )->getBackgroundColor().color() );
    formulaEdit->redraw( TRUE );
    formulaEdit->setBackgroundColor( frames.at( 0 )->getBackgroundColor().color() );
    formulaEdit->resize( frames.at( 0 )->width(), frames.at( 0 )->height() );
    ( (QScrollView*)_widget )->moveChild( formulaEdit, frames.at( 0 )->x(), frames.at( 0 )->y() );
    formulaEdit->show();
    ( (QScrollView*)_widget )->viewport()->setFocusProxy( formulaEdit );
    _widget->setFocusProxy( formulaEdit );
    formulaEdit->setFocus();
}

/*================================================================*/
void KWFormulaFrameSet::deactivate()
{
    formulaEdit->hide();

    if ( pic )
	delete pic;
    pic = new QPicture;
    QPainter p;
    p.begin( pic );
    formulaEdit->getFormula()->redraw( p );
    p.end();
}

/*================================================================*/
void KWFormulaFrameSet::insertChar( int c )
{
    if ( formulaEdit )
	formulaEdit->insertChar( c );
}

/*================================================================*/
void KWFormulaFrameSet::create( QWidget *parent )
{
    if ( formulaEdit ) {
	update();
	return;
    }

    formulaEdit = new KFormulaEdit( ( (QScrollView*)parent )->viewport() );
    ( (QScrollView*)parent )->addChild( formulaEdit );
    formulaEdit->getFormula()->setFont( font );
    formulaEdit->getFormula()->setBackColor( frames.at( 0 )->getBackgroundColor().color() );
    formulaEdit->getFormula()->setForeColor( color );
    formulaEdit->hide();
    formulaEdit->setText( text );
    update();
}

/*================================================================*/
void KWFormulaFrameSet::update()
{
    if ( !formulaEdit )
	return;
    formulaEdit->setFont( font );
    formulaEdit->resize( frames.at( 0 )->width(), frames.at( 0 )->height() );
    formulaEdit->getFormula()->setPos( formulaEdit->width() / 2, formulaEdit->height() / 2 );
    if ( pic )
	delete pic;
    pic = new QPicture;
    QPainter p;
    p.begin( pic );
    formulaEdit->getFormula()->redraw( p );
    p.end();
}

/*================================================================*/
void KWFormulaFrameSet::save( ostream &out )
{
    out << otag << "<FRAMESET frameType=\"" << static_cast<int>( getFrameType() ) << "\" frameInfo=\""
	<< static_cast<int>( frameInfo ) << "\">" << endl;

    KWFrameSet::save( out );

    out << otag << "<FORMULA>" << endl;
    out << formulaEdit->text().utf8().data() << endl;
    out << etag << "</FORMULA>" << endl;

    out << otag << "<FORMAT>" << endl;
    getFormat()->save( out );
    out << etag << "</FORMAT>" << endl;

    out << etag << "</FRAMESET>" << endl;
}

/*================================================================*/
void KWFormulaFrameSet::load( KOMLParser& parser, vector<KOMLAttrib>& lst )
{
    string tag;
    string name;
    string tmp;

    while ( parser.open( 0L, tag ) ) {
	KOMLParser::parseTag( tag.c_str(), name, lst );

	if ( name == "FORMULA" ) {
	    KOMLParser::parseTag( tag.c_str(), name, lst );
	    vector<KOMLAttrib>::const_iterator it = lst.begin();
	    for( ; it != lst.end(); it++ ) {
	    }
	    parser.readText( tmp );
	    text = QString::fromUtf8( tmp.c_str() );
	    text = text.stripWhiteSpace();
	} if ( name == "FORMAT" ) {
	    KWFormat f( doc );
	    f.load( parser, lst, doc );
	    font = QFont( f.getUserFont()->getFontName() );
	    font.setPointSize( f.getPTFontSize() );
	    font.setWeight( f.getWeight() );
	    font.setUnderline( f.getUnderline() );
	    font.setItalic( f.getItalic() );
	    color = f.getColor();
	} else if ( name == "FRAME" ) {
	    KWFrame rect;
	    KWParagLayout::Border l, r, t, b;
	    float lmm = 0, linch = 0, rmm = 0, rinch = 0, tmm = 0, tinch = 0, bmm = 0, binch = 0, ramm = 0, rainch = -1;
	    unsigned int lpt = 0, rpt = 0, tpt = 0, bpt = 0, rapt = 0;

	    l.color = Qt::white;
	    l.style = KWParagLayout::SOLID;
	    l.ptWidth = 1;
	    r.color = Qt::white;
	    r.style = KWParagLayout::SOLID;
	    r.ptWidth = 1;
	    t.color = Qt::white;
	    t.style = KWParagLayout::SOLID;
	    t.ptWidth = 1;
	    b.color = Qt::white;
	    b.style = KWParagLayout::SOLID;
	    b.ptWidth = 1;
	    QColor c( Qt::white );

	    KOMLParser::parseTag( tag.c_str(), name, lst );
	    vector<KOMLAttrib>::const_iterator it = lst.begin();
	    for( ; it != lst.end(); it++ ) {
		if ( ( *it ).m_strName == "left" )
		    rect.setLeft( atoi( ( *it ).m_strValue.c_str() ) );
		else if ( ( *it ).m_strName == "top" )
		    rect.setTop( atoi( ( *it ).m_strValue.c_str() ) );
		else if ( ( *it ).m_strName == "right" )
		    rect.setRight( atoi( ( *it ).m_strValue.c_str() ) );
		else if ( ( *it ).m_strName == "bottom" )
		    rect.setBottom( atoi( ( *it ).m_strValue.c_str() ) );
		else if ( ( *it ).m_strName == "runaround" )
		    rect.setRunAround( static_cast<RunAround>( atoi( ( *it ).m_strValue.c_str() ) ) );
		else if ( ( *it ).m_strName == "runaroundGap" )
		    rect.setRunAroundGap( KWUnit( atof( ( *it ).m_strValue.c_str() ) ) );
		else if ( ( *it ).m_strName == "runaGapPT" )
		    rapt = atoi( ( *it ).m_strValue.c_str() );
		else if ( ( *it ).m_strName == "runaGapMM" )
		    ramm = atof( ( *it ).m_strValue.c_str() );
		else if ( ( *it ).m_strName == "runaGapINCH" )
		    rainch = atof( ( *it ).m_strValue.c_str() );
		else if ( ( *it ).m_strName == "lWidth" )
		    l.ptWidth = atoi( ( *it ).m_strValue.c_str() );
		else if ( ( *it ).m_strName == "rWidth" )
		    r.ptWidth = atoi( ( *it ).m_strValue.c_str() );
		else if ( ( *it ).m_strName == "tWidth" )
		    t.ptWidth = atoi( ( *it ).m_strValue.c_str() );
		else if ( ( *it ).m_strName == "bWidth" )
		    b.ptWidth = atoi( ( *it ).m_strValue.c_str() );
		else if ( ( *it ).m_strName == "lRed" )
		    l.color.setRgb( atoi( ( *it ).m_strValue.c_str() ), l.color.green(), l.color.blue() );
		else if ( ( *it ).m_strName == "rRed" )
		    r.color.setRgb( atoi( ( *it ).m_strValue.c_str() ), r.color.green(), r.color.blue() );
		else if ( ( *it ).m_strName == "tRed" )
		    t.color.setRgb( atoi( ( *it ).m_strValue.c_str() ), t.color.green(), t.color.blue() );
		else if ( ( *it ).m_strName == "bRed" )
		    b.color.setRgb( atoi( ( *it ).m_strValue.c_str() ), b.color.green(), b.color.blue() );
		else if ( ( *it ).m_strName == "lGreen" )
		    l.color.setRgb( l.color.red(), atoi( ( *it ).m_strValue.c_str() ), l.color.blue() );
		else if ( ( *it ).m_strName == "rGreen" )
		    r.color.setRgb( r.color.red(), atoi( ( *it ).m_strValue.c_str() ), r.color.blue() );
		else if ( ( *it ).m_strName == "tGreen" )
		    t.color.setRgb( t.color.red(), atoi( ( *it ).m_strValue.c_str() ), t.color.blue() );
		else if ( ( *it ).m_strName == "bGreen" )
		    b.color.setRgb( b.color.red(), atoi( ( *it ).m_strValue.c_str() ), b.color.blue() );
		else if ( ( *it ).m_strName == "lBlue" )
		    l.color.setRgb( l.color.red(), l.color.green(), atoi( ( *it ).m_strValue.c_str() ) );
		else if ( ( *it ).m_strName == "rBlue" )
		    r.color.setRgb( r.color.red(), r.color.green(), atoi( ( *it ).m_strValue.c_str() ) );
		else if ( ( *it ).m_strName == "tBlue" )
		    t.color.setRgb( t.color.red(), t.color.green(), atoi( ( *it ).m_strValue.c_str() ) );
		else if ( ( *it ).m_strName == "bBlue" )
		    b.color.setRgb( b.color.red(), b.color.green(), atoi( ( *it ).m_strValue.c_str() ) );
		else if ( ( *it ).m_strName == "lStyle" )
		    l.style = static_cast<KWParagLayout::BorderStyle>( atoi( ( *it ).m_strValue.c_str() ) );
		else if ( ( *it ).m_strName == "rStyle" )
		    r.style = static_cast<KWParagLayout::BorderStyle>( atoi( ( *it ).m_strValue.c_str() ) );
		else if ( ( *it ).m_strName == "tStyle" )
		    t.style = static_cast<KWParagLayout::BorderStyle>( atoi( ( *it ).m_strValue.c_str() ) );
		else if ( ( *it ).m_strName == "bStyle" )
		    b.style = static_cast<KWParagLayout::BorderStyle>( atoi( ( *it ).m_strValue.c_str() ) );
		else if ( ( *it ).m_strName == "bkRed" )
		    c.setRgb( atoi( ( *it ).m_strValue.c_str() ), c.green(), c.blue() );
		else if ( ( *it ).m_strName == "bkGreen" )
		    c.setRgb( c.red(), atoi( ( *it ).m_strValue.c_str() ), c.blue() );
		else if ( ( *it ).m_strName == "bkBlue" )
		    c.setRgb( c.red(), c.green(), atoi( ( *it ).m_strValue.c_str() ) );
		else if ( ( *it ).m_strName == "bleftpt" )
		    lpt = atoi( ( *it ).m_strValue.c_str() );
		else if ( ( *it ).m_strName == "brightpt" )
		    rpt = atoi( ( *it ).m_strValue.c_str() );
		else if ( ( *it ).m_strName == "btoppt" )
		    tpt = atoi( ( *it ).m_strValue.c_str() );
		else if ( ( *it ).m_strName == "bbottompt" )
		    bpt = atoi( ( *it ).m_strValue.c_str() );
		else if ( ( *it ).m_strName == "bleftmm" )
		    lmm = atof( ( *it ).m_strValue.c_str() );
		else if ( ( *it ).m_strName == "brightmm" )
		    rmm = atof( ( *it ).m_strValue.c_str() );
		else if ( ( *it ).m_strName == "btopmm" )
		    tmm = atof( ( *it ).m_strValue.c_str() );
		else if ( ( *it ).m_strName == "bbottommm" )
		    bmm = atof( ( *it ).m_strValue.c_str() );
		else if ( ( *it ).m_strName == "bleftinch" )
		    linch = atof( ( *it ).m_strValue.c_str() );
		else if ( ( *it ).m_strName == "brightinch" )
		    rinch = atof( ( *it ).m_strValue.c_str() );
		else if ( ( *it ).m_strName == "btopinch" )
		    tinch = atof( ( *it ).m_strValue.c_str() );
		else if ((*it).m_strName == "bbottominch")
		    binch = atof( ( *it ).m_strValue.c_str() );
	    } KWFrame *_frame = new KWFrame( rect.x(), rect.y(), rect.width(), rect.height(), rect.getRunAround(),
					     rainch == -1 ? rect.getRunAroundGap() : KWUnit( rapt, ramm, rainch ) );
	    _frame->setLeftBorder( l );
	    _frame->setRightBorder( r );
	    _frame->setTopBorder( t );
	    _frame->setBottomBorder( b );
	    _frame->setBackgroundColor( QBrush( c ) );
	    _frame->setBLeft( KWUnit( lpt, lmm, linch ) );
	    _frame->setBRight( KWUnit( rpt, rmm, rinch ) );
	    _frame->setBTop( KWUnit( tpt, tmm, tinch ) );
	    _frame->setBBottom( KWUnit( bpt, bmm, binch ) );
	    frames.append( _frame );
	} else
	    cerr << "Unknown tag '" << tag << "' in FRAMESET" << endl;

	if ( !parser.close( tag ) ) {
	    cerr << "ERR: Closing Child" << endl;
	    return;
	}
    }
}


/******************************************************************/
/* Class: KWGroupManager					  */
/******************************************************************/

/*================================================================*/
void KWGroupManager::addFrameSet( KWFrameSet *fs, unsigned int row, unsigned int col )
{
    unsigned int sum = row * 10 + col;
    unsigned int i = 0;

    rows = QMAX( row + 1, rows );
    cols = QMAX( col + 1, cols );

    for ( i = 0; i < cells.count(); i++ ) {
	if ( cells.at( i )->row * 10 + cells.at( i )->col > sum )
	    break;
    }

    Cell *cell = new Cell;
    cell->frameSet = fs;
    cell->row = row;
    cell->col = col;
    cell->rows = 1;
    cell->cols = 1;

    cells.insert( i, cell );
}

/*================================================================*/
KWFrameSet *KWGroupManager::getFrameSet( unsigned int row, unsigned int col )
{
    for ( unsigned int i = 0; i < cells.count(); i++ ) {
	if ( cells.at( i )->row == row && cells.at( i )->col == col )
	    return cells.at( i )->frameSet;
    }

    return 0L;
}

/*================================================================*/
KWGroupManager::Cell *KWGroupManager::getCell( unsigned int row, unsigned int col )
{
    for ( unsigned int i = 0; i < cells.count(); i++ ) {
	if ( cells.at( i )->row == row && cells.at( i )->col == col )
	    return cells.at( i );
    }

    return 0L;
}

/*================================================================*/
bool KWGroupManager::getFrameSet( KWFrameSet *fs, unsigned int &row, unsigned int &col )
{
    for ( unsigned int i = 0; i < cells.count(); i++ ) {
	if ( cells.at( i )->frameSet == fs ) {
	    row = cells.at( i )->row;
	    col = cells.at( i )->col;
	    return true;
	}
    }

    return false;
}

/*================================================================*/
bool KWGroupManager::isTableHeader( KWFrameSet *fs )
{
    for ( unsigned int i = 0; i < cols; i++ ) {
	if ( cells.at( i )->frameSet == fs )
	    return true;
    }

    return false;
}

/*================================================================*/
void KWGroupManager::init( unsigned int x, unsigned int y, unsigned int width, unsigned int height,
			   KWTblCellSize widm, KWTblCellSize heim )
{
    unsigned int wid = width / cols - 2;
    unsigned int hei = height / rows - 2;

    if ( heim == TblAuto ||
	 static_cast<int>( hei ) < doc->getDefaultParagLayout()->getFormat().getPTFontSize() + 5 )
	hei = doc->getDefaultParagLayout()->getFormat().getPTFontSize() + 5;

    if ( widm == TblAuto ) {
	x = doc->getPTLeftBorder();
	width = doc->getPTPaperWidth() -
		( doc->getPTLeftBorder() + doc->getPTRightBorder() );
	wid = width / cols - 5;
    }

    if ( wid < 40 )
	wid = 40;

    unsigned int _wid, _hei;

    KWFrame *frame;

    for ( unsigned int i = 0; i < rows; i++ ) {
	for ( unsigned int j = 0; j < cols; j++ ) {
	    KWUnit u;
	    u.setMM( 1 );
	    frame = getFrameSet( i, j )->getFrame( 0 );
	    frame->setBLeft( u );
	    frame->setBRight( u );
	    frame->setBTop( u );
	    frame->setBBottom( u );
	    _wid = wid;
	    _wid += frame->getBLeft().pt() + frame->getBRight().pt();
	    _hei = hei;
	    _hei += frame->getBTop().pt() + frame->getBBottom().pt();
	    frame->setRect( x + j * _wid + 2 * j, y + i * _hei + 2 * i, _wid, _hei );
	}
    }

    for ( unsigned int k = 0; k < cells.count(); k++ )
	doc->addFrameSet( cells.at( k )->frameSet );
}

/*================================================================*/
void KWGroupManager::init()
{
    for ( unsigned int k = 0; k < cells.count(); k++ )
	doc->addFrameSet( cells.at( k )->frameSet );
}

/*================================================================*/
void KWGroupManager::recalcCols()
{
    for ( unsigned int i = 0; i < cols; i++ ) {
	unsigned int j = 0;
	int wid = -1;
	int _wid = 100000;
	for ( j = 0; j < rows; j++ ) {
	    if ( getFrameSet( j, i )->getFrame( 0 )->isSelected() )
		wid = getFrameSet( j, i )->getFrame( 0 )->width();
	    _wid = QMIN( getFrameSet( j, i )->getFrame( 0 )->width() , _wid );
	}
	if ( wid != -1 ) {
	    if ( getBoundingRect().x() + getBoundingRect().width() + ( wid - _wid ) >
		 static_cast<int>( doc->getPTPaperWidth() ) )
		wid = _wid;
	    for ( j = 0; j < rows; j++ ) {
		KWFrame *frame = getFrameSet( j, i )->getFrame( 0 );
		if ( wid < static_cast<int>( doc->getRastX() + frame->getBLeft().pt() + frame->getBRight().pt() ) )
		    frame->setWidth( doc->getRastX() + frame->getBLeft().pt() + frame->getBRight().pt() );
		else
		    frame->setWidth( wid );
	    }
	}
    }

    unsigned int x = getFrameSet( 0, 0 )->getFrame( 0 )->x();
    for ( unsigned int i = 0; i < cols; i++ ) {
	unsigned int j = 0;
	for ( j = 0; j < rows; j++ ) {
	    getFrameSet( j, i )->getFrame( 0 )->moveTopLeft( QPoint( x, getFrameSet( j, i )->getFrame( 0 )->y() ) );
	    getFrameSet( j, i )->update();
	}
	x = getFrameSet( 0, i )->getFrame( 0 )->right() + 3;
    }


    // Reggie: this has to be improved!
    QList<int> ws;
    ws.setAutoDelete( true );

    for ( unsigned int i = 0; i < cols; i++ )
	ws.append( new int( getFrameSet( 0, i )->getFrame( 0 )->width() ) );

    for ( unsigned int i = 0; i < rows; i++ ) {
	for ( unsigned int j = 0; j < cols; j++ ) {
	    Cell *cell = getCell( i, j );
	    if ( cell->cols != 1 ) {
		if ( cell->cols > 0 ) {
		    int w = 0;
		    for ( unsigned int k = 0; k < cell->cols; k++ )
			w += *ws.at( k + cell->col );

		    cell->frameSet->getFrame( 0 )->setWidth( w + ( cell->cols - 2 ) * 2 + 2 );
		}
	    }
	}
    }
}

/*================================================================*/
void KWGroupManager::recalcRows()
{
    // remove atomatically added headers
    for ( unsigned int j = 0; j < rows; j++ ) {
	if ( getFrameSet( j, 0 )->isRemoveableHeader() ) {
	    deleteRow( j, false );
	    j--;
	}
    }
    hasTmpHeaders = false;

    for ( unsigned int j = 0; j < rows; j++ ) {
	unsigned int i = 0;
	int hei = -1;
	int _hei = 100000;
	for ( i = 0; i < cols; i++ ) {
	    if ( getFrameSet( j, i )->getFrame( 0 )->isSelected() )
		hei = getFrameSet( j, i )->getFrame( 0 )->height();
	    _hei = min( getFrameSet( j, i )->getFrame( 0 )->height() , _hei );
	}
	if ( hei != -1 ) {
	    for ( i = 0; i < cols; i++ )
		getFrameSet( j, i )->getFrame( 0 )->setHeight( hei );
	}
    }

    unsigned int y = getFrameSet( 0, 0 )->getFrame( 0 )->y();
    for ( unsigned int j = 0; j < rows; j++ ) {
	unsigned int i = 0;
	bool _addRow = false;

	if ( doc->getProcessingType() == KWordDocument::DTP ) {
	    if ( j > 0 && y + getFrameSet( j, i )->getFrame( 0 )->height() >
		 ( getFrameSet( j - 1, i )->getPageOfFrame( 0 ) + 1 ) *
		 doc->getPTPaperHeight() - doc->getPTBottomBorder() ) {
		y = ( getFrameSet( j - 1, i )->getPageOfFrame( 0 ) + 1 ) *
		    doc->getPTPaperHeight() + doc->getPTTopBorder();
		_addRow = true;
	    }
	} else {
	    if ( j > 0 && static_cast<int>( y + getFrameSet( j, i )->getFrame( 0 )->height() ) >
		 static_cast<int>( ( doc->getFrameSet( 0 )->getFrame( getFrameSet( j - 1, i )->getPageOfFrame( 0 ) *
								      doc->getColumns() )->bottom() ) ) ) {
		if ( doc->getPages() < getFrameSet( j - 1, i )->getPageOfFrame( 0 ) + 2 )
		    doc->appendPage( doc->getPages() - 1 ); {
		    _addRow = true;
		    y = doc->getFrameSet( 0 )->getFrame( ( getFrameSet( j - 1, i )->getPageOfFrame( 0 ) + 1 ) *
							 doc->getColumns() )->y();
		}
	    }
	}

	if ( _addRow && showHeaderOnAllPages ) {
	    hasTmpHeaders = true;
	    insertRow( j, false, true );
	}

	for ( i = 0; i < cols; i++ ) {
	    if ( _addRow ) {
		KWTextFrameSet *f1, *f2;
		f1 = dynamic_cast<KWTextFrameSet*>( getFrameSet( j, i ) );
		f2 = dynamic_cast<KWTextFrameSet*>( getFrameSet( 0, i ) );
		f1->assign( f2 );
		f1->getFrame( 0 )->setHeight( f2->getFrame( 0 )->height() );
	    }

	    getFrameSet( j, i )->getFrame( 0 )->moveTopLeft( QPoint( getFrameSet( j, i )->getFrame( 0 )->x(), y ) );
	    getFrameSet( j, i )->update();
	}

	y = getFrameSet( j, 0 )->getFrame( 0 )->bottom() + 3;
    }

    if ( getBoundingRect().y() + getBoundingRect().height() >
	 static_cast<int>( doc->getPTPaperHeight() * doc->getPages() ) )
	doc->appendPage( doc->getPages() - 1 );

    // Reggie: UHHHHHHHHHHHH: Ugly and slow but it helps for now
    Cell *c;
    for ( unsigned int f = 0; f < cells.count(); f++ ) {
	c = cells.at( f );
	if ( c->frameSet->getNumFrames() > 1 ) {
	    while ( true ) {
		if ( c->frameSet->getNumFrames() > 1 )
		    c->frameSet->delFrame( 1 );
		else
		    break;
	    }
	}
    }

    // Reggie: this has to be improved
    QList<int> hs;
    hs.setAutoDelete( true );

    for ( unsigned int i = 0; i < rows; i++ )
	hs.append( new int( getFrameSet( i, 0 )->getFrame( 0 )->height() ) );

    for ( unsigned int i = 0; i < cols; i++ ) {
	for ( unsigned int j = 0; j < rows; j++ ) {
	    Cell *cell = getCell( j, i );
	    if ( cell->rows != 1 ) {
		if ( cell->rows > 0 ) {
		    int h = 0;
		    for ( unsigned int k = 0; k < cell->rows; k++ )
			h += *hs.at( k + cell->row );

		    cell->frameSet->getFrame( 0 )->setHeight( h + ( cell->rows - 2 ) * 2 + 2 );
		}
	    }
	}
    }
}

/*================================================================*/
QRect KWGroupManager::getBoundingRect()
{
    QRect r1, r2;
    KWFrame *first = getFrameSet( 0, 0 )->getFrame( 0 );
    KWFrame *last = getFrameSet( rows - 1, cols - 1 )->getFrame( 0 );

    r1 = QRect( first->x(), first->y(), first->width(), first->height() );
    r2 = QRect( last->x(), last->y(), last->width(), last->height() );

    r1 = r1.unite( r2 );
    return QRect( r1 );
}

/*================================================================*/
bool KWGroupManager::hasSelectedFrame()
{
    for ( unsigned int i = 0; i < cells.count(); i++ ) {
	if ( cells.at( i )->frameSet->getFrame( 0 )->isSelected() )
	    return true;
    }

    return false;
}

/*================================================================*/
void KWGroupManager::moveBy( unsigned int dx, unsigned int dy )
{
    for ( unsigned int i = 0; i < cells.count(); i++ )
	cells.at( i )->frameSet->getFrame( 0 )->moveBy( dx, dy );
}

/*================================================================*/
void KWGroupManager::drawAllRects( QPainter &p, int xOffset, int yOffset )
{
    KWFrame *frame = 0L;

    for ( unsigned int i = 0; i < cells.count(); i++ ) {
	frame = cells.at( i )->frameSet->getFrame( 0 );
	p.drawRect( frame->x() - xOffset, frame->y() - yOffset, frame->width(), frame->height() );
    }
}


/*================================================================*/
void KWGroupManager::deselectAll()
{
    KWFrame *frame = 0L;

    for ( unsigned int i = 0; i < cells.count(); i++ ) {
	frame = cells.at( i )->frameSet->getFrame( 0 );
	frame->setSelected( false );
    }
}

/*================================================================*/
void KWGroupManager::selectUntil( KWFrameSet *fs, KWPage *page )
{
    unsigned int row = 0, col = 0;
    getFrameSet( fs, row, col );

    unsigned int srow = 0, scol = 0;
    if ( !isOneSelected( fs, srow, scol ) )
	srow = scol = 0;

    if ( srow > row ) {
	srow = srow^row;
	row = srow^row;
	srow = srow^row;
    }

    if ( scol > col ) {
	scol = scol^col;
	col = scol^col;
	scol = scol^col;
    }

    for ( unsigned int i = 0; i < cells.count(); i++ ) {
	bool s = cells.at( i )->frameSet->getFrame( 0 )->isSelected();
	if ( cells.at( i )->row <= row && cells.at( i )->col <= col &&
	     cells.at( i )->row >= srow && cells.at( i )->col >= scol )
	    cells.at( i )->frameSet->getFrame( 0 )->setSelected( TRUE );
	else
	    cells.at( i )->frameSet->getFrame( 0 )->setSelected( FALSE );
	if ( cells.at( i )->frameSet->getFrame( 0 )->isSelected() != s ) {
	    if ( cells.at( i )->frameSet->getFrame( 0 )->isSelected() )
		page->createResizeHandles( cells.at( i )->frameSet->getFrame( 0 ) );
	    else
		page->removeResizeHandles( cells.at( i )->frameSet->getFrame( 0 ) );
	}
    }
}

/*================================================================*/
bool KWGroupManager::isOneSelected( KWFrameSet *fs, unsigned int &row, unsigned int &col )
{
    bool one = false;

    for ( unsigned int i = 0; i < cells.count(); i++ ) {
	if ( cells.at( i )->frameSet->getFrame( 0 )->isSelected() && fs != cells.at( i )->frameSet ) {
	    if ( !one ) {
		row = cells.at( i )->row;
		col = cells.at( i )->col;
		one = true;
	    }
	}
    }

    return one;
}

/*================================================================*/
void KWGroupManager::insertRow( unsigned int _idx, bool _recalc, bool _removeable )
{
    unsigned int i = 0;
    unsigned int _rows = rows;

    QList<int> w;
    w.setAutoDelete( true );
    QRect r = getBoundingRect();

    for ( i = 0; i < cells.count(); i++ ) {
	if ( cells.at( i )->row == 0 ) w.append( new int( cells.at( i )->frameSet->getFrame( 0 )->width() ) );
	if ( cells.at( i )->row >= _idx ) cells.at( i )->row++;
    }

    QList<KWTextFrameSet> nCells;
    nCells.setAutoDelete( false );

    int ww = 0;
    for ( i = 0; i < getCols(); i++ ) {
	KWFrame *frame = new KWFrame( r.x() + ww, r.y(), *w.at( i ),
				      doc->getDefaultParagLayout()->getFormat().getPTFontSize() + 10 );
	KWTextFrameSet *_frameSet = new KWTextFrameSet( doc );
	_frameSet->addFrame( frame );
	_frameSet->setAutoCreateNewFrame( false );
	_frameSet->setGroupManager( this );
	_frameSet->setIsRemoveableHeader( _removeable );
	addFrameSet( _frameSet, _idx, i );
	nCells.append( _frameSet );
	ww += *w.at( i ) + 2;
    }

    rows = ++_rows;

    for ( i = 0; i < nCells.count(); i++ ) {
	KWUnit u;
	u.setMM( 1 );
	doc->addFrameSet( nCells.at( i ) );
	KWFrame *frame = nCells.at( i )->getFrame( 0 );
	frame->setBLeft( u );
	frame->setBRight( u );
	frame->setBTop( u );
	frame->setBBottom( u );
    }

    if ( _recalc )
	recalcRows();
}

/*================================================================*/
void KWGroupManager::insertCol( unsigned int _idx )
{
    unsigned int i = 0;
    unsigned int _cols = cols;

    QList<int> h;
    h.setAutoDelete( true );
    QRect r = getBoundingRect();

    for ( i = 0; i < cells.count(); i++ ) {
	if ( cells.at( i )->col == 0 ) h.append( new int( cells.at( i )->frameSet->getFrame( 0 )->height() ) );
	if ( cells.at( i )->col >= _idx ) cells.at( i )->col++;
    }

    QList<KWTextFrameSet> nCells;
    nCells.setAutoDelete( false );

    int hh = 0;
    for ( i = 0; i < getRows(); i++ ) {
	KWFrame *frame = new KWFrame( r.x(), r.y() + hh, 60, *h.at( i ) );
	KWTextFrameSet *_frameSet = new KWTextFrameSet( doc );
	_frameSet->addFrame( frame );
	_frameSet->setAutoCreateNewFrame( false );
	_frameSet->setGroupManager( this );
	addFrameSet( _frameSet, i, _idx );
	nCells.append( _frameSet );
	hh += *h.at( i ) + 2;
    }

    cols = ++_cols;

    for ( i = 0; i < nCells.count(); i++ ) {
	KWUnit u;
	u.setMM( 1 );
	doc->addFrameSet( nCells.at( i ) );
	KWFrame *frame = nCells.at( i )->getFrame( 0 );
	frame->setBLeft( u );
	frame->setBRight( u );
	frame->setBTop( u );
	frame->setBBottom( u );
    }

    recalcCols();
}

/*================================================================*/
void KWGroupManager::deleteRow( unsigned int _idx, bool _recalc )
{
    for ( unsigned int i = 0; i < cells.count(); i++ ) {
	if ( cells.at( i )->row == _idx ) {
	    doc->delFrameSet( cells.at( i )->frameSet );
	    cells.at( i )->frameSet = 0L;
	    cells.remove( i );
	    i--;
	    continue;
	}
	if ( cells.at( i )->row > _idx ) cells.at( i )->row--;
    }

    rows--;

    if ( _recalc )
	recalcRows();
}

/*================================================================*/
void KWGroupManager::deleteCol( unsigned int _idx )
{
    for ( unsigned int i = 0; i < cells.count(); i++ ) {
	if ( cells.at( i )->col == _idx ) {
	    doc->delFrameSet( cells.at( i )->frameSet );
	    cells.at( i )->frameSet = 0L;
	    cells.remove( i );
	    i--;
	    continue;
	}
	if ( cells.at( i )->col > _idx ) cells.at( i )->col--;
    }

    cols--;

    recalcCols();
}

/*================================================================*/
void KWGroupManager::updateTempHeaders()
{
    //if ( !hasTmpHeaders ) return;

    for ( unsigned int i = 1; i < rows; i++ ) {
	for ( unsigned int j = 0; j < cols; j++ ) {
	    KWFrameSet *fs = getFrameSet( i, j );
	    if ( fs->isRemoveableHeader() ) {
		dynamic_cast<KWTextFrameSet*>( fs )->assign( dynamic_cast<KWTextFrameSet*>( getFrameSet( 0, j ) ) );

		QPainter p;
		QPicture pic;
		p.begin( &pic );

		KWFormatContext fc( doc, doc->getFrameSetNum( fs ) + 1 );
		fc.init( dynamic_cast<KWTextFrameSet*>( fs )->getFirstParag(), true );

		bool bend = false;
		while ( !bend )
		    bend = !fc.makeNextLineLayout();

		p.end();
	    }
	}
    }
}

/*================================================================*/
void KWGroupManager::ungroup()
{
    for ( unsigned int i = 0; i < cells.count(); i++ )
	cells.at( i )->frameSet->setGroupManager( 0L );

    cells.setAutoDelete( false );
    cells.clear();

    active = false;
}

/*================================================================*/
bool KWGroupManager::joinCells()
{
    enum Orientation {Vertical, Horizontal};

    unsigned int col, row;
    if ( !isOneSelected( 0L, row, col ) ) return false;

    bool enough = false;
    Orientation orient = Horizontal;

    if ( col > 0 && getFrameSet( row, col - 1 )->getFrame( 0 )->isSelected() ) {
	enough = true;
	orient = Horizontal;
    } else if ( col < cols - 1 && getFrameSet( row, col + 1 )->getFrame( 0 )->isSelected() ) {
	enough = true;
	orient = Horizontal;
    } else if ( row > 0 && getFrameSet( row - 1, col )->getFrame( 0 )->isSelected() ) {
	enough = true;
	orient = Vertical;
    } else if ( row < rows - 1 && getFrameSet( row + 1, col )->getFrame( 0 )->isSelected() ) {
	enough = true;
	orient = Vertical;
    }

    QList<Cell> _cells;
    _cells.setAutoDelete( false );

    if ( enough ) {
	switch ( orient ) {
	case Horizontal: {
	    Cell *cell = getCell( row, col );
	    int tmpCol = col;
	    while ( cell && cell->frameSet->getFrame( 0 )->isSelected() ) {
		if ( cell->rows > 1 || cell->cols > 1 )
		    return false;
		_cells.insert( 0, cell );
		if ( --tmpCol >= 0 )
		    cell = getCell( row, tmpCol );
		else
		    cell = 0L;
	    }
	    if ( col + 1 > cols - 1 )
		break;

	    cell = getCell( row, col + 1 );
	    tmpCol = col + 1;
	    while ( cell && cell->frameSet->getFrame( 0 )->isSelected() ) {
		if ( cell->rows > 1 || cell->cols > 1 )
		    return false;
		_cells.append( cell );
		if ( ++tmpCol <= static_cast<int>( cols - 1 ) )
		    cell = getCell( row, tmpCol );
		else
		    cell = 0L;
	    }
	} break;
	case Vertical: {
	    Cell *cell = getCell( row, col );
	    int tmpRow = row;
	    while ( cell && cell->frameSet->getFrame( 0 )->isSelected() ) {
		if ( cell->rows > 1 || cell->cols > 1 )
		    return false;
		_cells.insert( 0, cell );
		if ( --tmpRow >= 0 )
		    cell = getCell( tmpRow, col );
		else
		    cell = 0L;
	    }
	    if ( row + 1 > rows - 1 ) break;

	    cell = getCell( row + 1, col );
	    tmpRow = row + 1;
	    while ( cell && cell->frameSet->getFrame( 0 )->isSelected() ) {
		if ( cell->rows > 1 || cell->cols > 1 )
		    return false;
		_cells.append( cell );
		if ( ++tmpRow <= static_cast<int>( rows - 1 ) )
		    cell = getCell( tmpRow, col );
		else
		    cell = 0L;
	    }
	} break;
	}

	// this just can't happen :- )
	if ( _cells.count() < 1 )
	    return false;

	switch ( orient ) {
	case Horizontal: {
	    Cell *cell = _cells.first();
	    cell->cols = _cells.count();
	    for ( cell = _cells.next(); cell != 0; cell = _cells.next() )
	    {
		cell->cols = 0;
		cell->frameSet->setVisible( false );
	    }
	    recalcCols();
	} break;
	case Vertical: {
	    Cell *cell = _cells.first();
	    cell->rows = _cells.count();
	    for ( cell = _cells.next(); cell != 0; cell = _cells.next() )
	    {
		cell->frameSet->setVisible( false );
		cell->rows = 0;
	    }
	    recalcRows();
	} break;
	}

	return true;
    }

    return false;
}

/*================================================================*/
bool KWGroupManager::splitCell()
{
    unsigned int col, row;
    if ( !isOneSelected( 0L, row, col ) ) return false;

    Cell *cell = getCell( row, col );
    if ( cell->rows > 1 ) {
	unsigned int rows = cell->rows;
	for ( unsigned int i = 0; i < rows; i++ ) {
	    getCell( i + cell->row, col )->rows = 1;
	    getCell( i + cell->row, col )->frameSet->setVisible( true );
	}
	recalcRows();

	return true;
    } else if ( cell->cols > 1 ) {
	unsigned int cols = cell->cols;
	for ( unsigned int i = 0; i < cols; i++ ) {
	    getCell( row, i + cell->col )->cols = 1;
	    getCell( row, i + cell->col )->frameSet->setVisible( true );
	}
	recalcCols();

	return true;
    }

    return false;
}

/*================================================================*/
bool isAHeader( FrameInfo fi )
{
    return ( fi == FI_FIRST_HEADER || fi == FI_EVEN_HEADER || fi == FI_ODD_HEADER );
}

/*================================================================*/
bool isAFooter( FrameInfo fi )
{
    return ( fi == FI_FIRST_FOOTER || fi == FI_EVEN_FOOTER || fi == FI_ODD_FOOTER );
}

/*================================================================*/
bool isAWrongHeader( FrameInfo fi, KoHFType t )
{
    switch ( fi ) {
    case FI_FIRST_HEADER: {
	if ( t == HF_FIRST_DIFF ) return false;
	return true;
    } break;
    case FI_EVEN_HEADER: {
	return false;
    } break;
    case FI_ODD_HEADER: {
	if ( t == HF_EO_DIFF ) return false;
	return true;
    } break;
    default: return false;
    }

    return false;
}

/*================================================================*/
bool isAWrongFooter( FrameInfo fi, KoHFType t )
{
    switch ( fi ) {
    case FI_FIRST_FOOTER: {
	if ( t == HF_FIRST_DIFF ) return false;
	return true;
    } break;
    case FI_EVEN_FOOTER: {
	return false;
    } break;
    case FI_ODD_FOOTER: {
	if ( t == HF_EO_DIFF ) return false;
	return true;
    } break;
    default: return false;
    }

    return false;
}
