// -*- Mode: c++; c-basic-offset: 4; indent-tabs-mode: nil; tab-width: 4; -*-
/* This file is part of the KDE project
   Copyright (C) 2005 Thorsten Zachmann <zachmann@kde.org>
   Copyright (C) 2005 Casper Boemann Rasmussen <cbr@boemann.dk>

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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "KoGuides.h"

#include <QCursor>
#include <QPainter>
#include <QPixmap>
#include <QAction>
#include <QMouseEvent>
#include <QList>
#include <QKeyEvent>
#include <QEvent>

#include <klocale.h>
#include <kmenu.h>

#include <KoDocument.h>
#include <KoView.h>
#include <KoZoomHandler.h>

#include "KoGuideLineDia.h"

class KoGuides::Popup : public KMenu
{
public: 
    Popup( KoGuides * guides )
    {
        setTitle( i18n( "Guide Line" ) );
        m_delete = addAction( i18n( "&Delete" ), guides, SLOT( slotRemove() ) );
        m_seperator = addSeparator();
        m_pos = addAction( i18n( "&Set Position..." ), guides, SLOT( slotChangePosition() ) );
    }

    void update( int count )
    {
        if ( count == 1 )
        {
            setTitle( i18n( "Guide Line" ) );
            m_seperator->setVisible( true );
            m_pos->setVisible( true );
        }
        else
        {
            setTitle( i18n( "Guide Lines" ) );
            m_seperator->setVisible( false );
            m_pos->setVisible( false );
        }
    }
private:
    QAction* m_delete;
    QAction* m_seperator;
    QAction* m_pos;
};

KoGuides::KoGuides( KoView *view, KoZoomHandler *zoomHandler )
: m_view( view )
, m_zoomHandler( zoomHandler )
{
    m_popup = new Popup( this );
    m_mouseSelected = false;
}


KoGuides::~KoGuides()
{
    delete m_popup;
}


void KoGuides::paintGuides( QPainter &painter )
{
    //painter.setRasterOp( NotROP );
    const KoPageLayout& pl = m_view->koDocument()->pageLayout();
    int width = qMax( m_view->canvas()->width(), qRound(m_zoomHandler->zoomItX( pl.width )) );
    int height = qMax( m_view->canvas()->height(), qRound(m_zoomHandler->zoomItY(pl.height)) );

    for ( int i = 0; i < GL_END; ++i )
    {
        QList<KoGuideLine *>::iterator it = m_guideLines[i].begin();
        for ( ; it != m_guideLines[i].end(); ++it )
        {
            if ( !( *it )->automatic || ( *it )->snapping ) // do not paint autoStyle guides when they are not snapping
            {
                if ( ( *it )->snapping )
                    painter.setPen( QPen( Qt::green, 0, Qt::DotLine ) );
                else if ( ( *it )->selected )
                    painter.setPen( QPen( Qt::red, 0, Qt::DotLine ) );
                else
                    painter.setPen( QPen( Qt::blue, 0, Qt::DotLine ) );

                painter.save();
                if ( ( *it )->orientation == Qt::Vertical )
                {
                    painter.translate( m_zoomHandler->zoomItX( ( *it )->position ), 0 );
                    painter.drawLine( 0, 0, 0, height );
                }
                else
                {
                    painter.translate( 0, m_zoomHandler->zoomItY( ( *it )->position ) );
                    painter.drawLine( 0, 0, width, 0 );
                }
                painter.restore();
            }
        }
    }
}

bool KoGuides::mousePressEvent( QMouseEvent *e )
{
    bool eventProcessed = true;
    bool changed = false;
    m_mouseSelected = false;
    
    QPointF p( mapFromScreen( e->pos() ) );
    KoGuideLine * guideLine = find( p, m_zoomHandler->unzoomItY( 2 ) );
    if ( guideLine )
    {
        m_lastPoint = e->pos();
        if ( e->button() == Qt::LeftButton || e->button() == Qt::RightButton )
        {
            if ( e->button() == Qt::LeftButton )
            {
                m_mouseSelected = true;
            }
            if ( e->modifiers() & Qt::ControlModifier )
            {
                if ( guideLine->selected )
                {
                    unselect( guideLine );
                    m_mouseSelected = false;
                }
                else
                {
                    select( guideLine );
                }
                changed = true;
            }
            else if ( ! guideLine->selected )
            {
                unselectAll();
                select( guideLine );
                changed = true;
            }
        }
    }
    else 
    {   
        if ( !( e->modifiers() & Qt::ControlModifier ) )
        {
            changed = unselectAll();
        }
        eventProcessed = false;
    }
    
    if ( changed || hasSelected() )
    {
        emit moveGuides( true );
    }

    if ( changed )
    {
        paint();
    }

    if ( changed && ! hasSelected() )
    {
        emit moveGuides( false );
    }

    if ( e->button() == Qt::RightButton && hasSelected() )
    {
        m_popup->update( m_guideLines[GL_SELECTED].count() );
        m_popup->exec( QCursor::pos() );
        emit moveGuides( false );
    }
    
    return eventProcessed;
}


bool KoGuides::mouseMoveEvent( QMouseEvent *e )
{
    bool eventProcessed = false;
    if ( m_mouseSelected )
    {
        QPoint p( e->pos() );
        p -= m_lastPoint;
        m_lastPoint = e->pos();
        moveSelectedBy( p );
        paint();
        emit guideLinesChanged( m_view );
        eventProcessed = true;
    }
    else if ( e->modifiers() == Qt::NoModifier )
    {
        QPointF p( mapFromScreen( e->pos() ) );
        KoGuideLine * guideLine = find( p, m_zoomHandler->unzoomItY( 2 ) );
        if ( guideLine )
        {
            m_view->canvas()->setCursor( guideLine->orientation == Qt::Vertical ? Qt::SizeHorCursor : Qt::SizeHorCursor );
            eventProcessed = true;
        }
    }
    return eventProcessed;
}


bool KoGuides::mouseReleaseEvent( QMouseEvent *e )
{
    bool eventProcessed = false;
    if ( m_mouseSelected )
    {
        QPointF p( mapFromScreen( e->pos() ) );
        if ( m_guideLines[GL_SELECTED].count() == 1 )
        {
            int x1, y1, x2, y2;
            m_view->canvas()->rect().getCoords( &x1, &y1, &x2, &y2 );
            QPoint gp( m_view->canvas()->mapFromGlobal( e->globalPos() ) );
            if ( m_guideLines[GL_SELECTED].first()->orientation == Qt::Vertical )
            {
                if ( gp.x() < x1 || gp.x() > x2 )
                    removeSelected();
            }
            else
            {
                if ( gp.y() < y1 || gp.y() > y2 )
                    removeSelected();
            }
        }
        KoGuideLine * guideLine = find( p, m_zoomHandler->unzoomItY( 2 ) );
        if ( guideLine )
        {
            m_view->canvas()->setCursor( guideLine->orientation == Qt::Vertical ? Qt::SizeHorCursor : Qt::SizeHorCursor );
        }
        m_mouseSelected = false;
        eventProcessed = true;
        emit guideLinesChanged( m_view );
    }
    emit moveGuides( false );
    return eventProcessed;
}


bool KoGuides::keyPressEvent( QKeyEvent *e )
{
    bool eventProcessed = false;
    switch( e->key() )
    {
        case Qt::Key_Delete: 
            if ( hasSelected() )
            {
                removeSelected();
                paint();
                emit guideLinesChanged( m_view );
                eventProcessed = true;
            }
            break;
        default:
            break;
    }
    return eventProcessed;
}

void KoGuides::setGuideLines( const QList<qreal> &horizontalPos, const QList<qreal> &verticalPos )
{
    removeSelected();

    QList<KoGuideLine *>::iterator it = m_guideLines[GL].begin();
    for ( ; it != m_guideLines[GL].end(); ++it )
    {
        delete ( *it );
    }
    m_guideLines[GL].clear();

    QList<qreal>::ConstIterator posIt = horizontalPos.begin();
    for ( ; posIt != horizontalPos.end(); ++posIt )
    {
        KoGuideLine *guideLine = new KoGuideLine( Qt::Horizontal, *posIt, false );
        m_guideLines[GL].append( guideLine );
    }
    posIt = verticalPos.begin();
    for ( ; posIt != verticalPos.end(); ++posIt )
    {
        KoGuideLine *guideLine = new KoGuideLine( Qt::Vertical, *posIt, false );
        m_guideLines[GL].append( guideLine );
    }
    paint();
}

void KoGuides::setAutoGuideLines( const QList<qreal> &horizontalPos, const QList<qreal> &verticalPos )
{
    QList<KoGuideLine *>::iterator it = m_guideLines[GL_AUTOMATIC].begin();
    for ( ; it != m_guideLines[GL_AUTOMATIC].end(); ++it )
    {
        delete ( *it );
    }
    m_guideLines[GL_AUTOMATIC].clear();

    QList<qreal>::ConstIterator posIt = horizontalPos.begin();
    for ( ; posIt != horizontalPos.end(); ++posIt )
    {
        KoGuideLine *guideLine = new KoGuideLine( Qt::Horizontal, *posIt, true );
        m_guideLines[GL_AUTOMATIC].append( guideLine );
    }
    posIt = verticalPos.begin();
    for ( ; posIt != verticalPos.end(); ++posIt )
    {
        KoGuideLine *guideLine = new KoGuideLine( Qt::Vertical, *posIt, true );
        m_guideLines[GL_AUTOMATIC].append( guideLine );
    }
}


void KoGuides::getGuideLines( QList<qreal> &horizontalPos, QList<qreal> &verticalPos ) const
{
    horizontalPos.clear();
    verticalPos.clear();

    QList<KoGuideLine *>::const_iterator it = m_guideLines[GL].constBegin();
    for ( ; it != m_guideLines[GL].constEnd(); ++it )
    {
        if ( ( *it )->orientation == Qt::Horizontal )
        {
            horizontalPos.append( ( *it )->position );
        }
        else
        {
            verticalPos.append( ( *it )->position );
        }
    }
    it = m_guideLines[GL_SELECTED].constBegin();
    for ( ; it != m_guideLines[GL_SELECTED].constEnd(); ++it )
    {
        if ( ( *it )->orientation == Qt::Horizontal )
        {
            horizontalPos.append( ( *it )->position );
        }
        else
        {
            verticalPos.append( ( *it )->position );
        }
    }
}


void KoGuides::snapToGuideLines( QRectF &rect, int snap, SnapStatus &snapStatus, QPointF &diff )
{
    if( !(snapStatus & Qt::Vertical))
        diff.setX(10000);
    if( !(snapStatus & Qt::Horizontal))
        diff.setY(10000);

    for ( int i = 0; i < GL_END; ++i )
    {
        QList<KoGuideLine *>::const_iterator it = m_guideLines[i].constBegin();
        for ( ; it != m_guideLines[i].constEnd(); ++it )
        {
            if ( ( *it )->orientation == Qt::Horizontal )
            {
                qreal tmp = (*it)->position - rect.top();
                if ( snapStatus & Qt::Horizontal || qAbs( tmp ) < m_zoomHandler->unzoomItY( snap ) )
                {
                    if(qAbs( tmp ) < qAbs(diff.y()))
                    {
                        diff.setY( tmp );
                        snapStatus |= Qt::Horizontal;
                    }
                }
                tmp = (*it)->position - rect.bottom();
                if ( snapStatus & Qt::Horizontal || qAbs( tmp ) < m_zoomHandler->unzoomItY( snap ) )
                {
                    if(qAbs( tmp ) < qAbs(diff.y()))
                    {
                        diff.setY( tmp );
                        snapStatus |= Qt::Horizontal;
                    }
                }
            }
            else
            {
                qreal tmp = (*it)->position - rect.left();
                if ( snapStatus & Qt::Vertical || qAbs( tmp ) < m_zoomHandler->unzoomItX( snap ) )
                {
                    if(qAbs( tmp ) < qAbs(diff.x()))
                    {
                        diff.setX( tmp );
                        snapStatus |= Qt::Vertical;
                    }
                }
                tmp = (*it)->position - rect.right();
                if ( snapStatus & Qt::Vertical || qAbs( tmp ) < m_zoomHandler->unzoomItX( snap ) )
                {
                    if(qAbs( tmp ) < qAbs(diff.x()))
                    {
                        diff.setX( tmp );
                        snapStatus |= Qt::Vertical;
                    }
                }
            }
        }
    }

    if(!(snapStatus & Qt::Vertical))
        diff.setX( 0 );

    if(!(snapStatus & Qt::Horizontal))
        diff.setY( 0 );
}

void KoGuides::snapToGuideLines( QPointF &pos, int snap, SnapStatus &snapStatus, QPointF &diff )
{
    if( !(snapStatus & Qt::Vertical))
        diff.setX(10000);
    if( !(snapStatus & Qt::Horizontal))
        diff.setY(10000);

    for ( int i = 0; i < GL_END; ++i )
    {
        QList<KoGuideLine *>::const_iterator it = m_guideLines[i].constBegin();
        for ( ; it != m_guideLines[i].constEnd(); ++it )
        {
            if ( ( *it )->orientation == Qt::Horizontal )
            {
                qreal tmp = (*it)->position - pos.y();
                if ( snapStatus & Qt::Horizontal || qAbs( tmp ) < m_zoomHandler->unzoomItY( snap ) )
                {
                    if(qAbs( tmp ) < qAbs(diff.y()))
                    {
                        diff.setY( tmp );
                        snapStatus |= Qt::Horizontal;
                    }
                }
            }
            else
            {
                qreal tmp = (*it)->position - pos.x();
                if ( snapStatus & Qt::Vertical || qAbs( tmp ) < m_zoomHandler->unzoomItX( snap ) )
                {
                    if(qAbs( tmp ) < qAbs(diff.x()))
                    {
                        diff.setX( tmp );
                        snapStatus |= Qt::Vertical;
                    }
                }
            }
        }
    }

    if(!(snapStatus & Qt::Vertical))
        diff.setX( 0 );

    if(!(snapStatus & Qt::Horizontal))
        diff.setY( 0 );
}


void KoGuides::repaintSnapping( const QRectF &snappedRect )
{
    bool needRepaint = false;
    for ( int i = 0; i < GL_END; ++i )
    {
        QList<KoGuideLine *>::const_iterator it = m_guideLines[i].constBegin();
        for ( ; it != m_guideLines[i].constEnd(); ++it )
        {
            if ( ( *it )->orientation == Qt::Horizontal )
            {
                if ( virtuallyEqual( snappedRect.top(), (*it)->position ) 
                        || virtuallyEqual( snappedRect.bottom(), ( *it )->position ) )
                {
                    if ( ! ( *it )->snapping )
                    {
                        ( *it )->snapping = true;
                        needRepaint = true;
                    }
                }
                else if ( ( *it )->snapping )
                {
                    ( *it )->snapping = false;
                    needRepaint = true;
                }
            }
            else
            {
                if ( virtuallyEqual( snappedRect.left(), (*it)->position )
                        || virtuallyEqual( snappedRect.right(), ( *it )->position ) )
                {
                    if ( ! ( *it )->snapping )
                    {
                        ( *it )->snapping = true;
                        needRepaint = true;
                    }
                }
                else if ( ( *it )->snapping )
                {
                    ( *it )->snapping = false;
                    needRepaint = true;
                }
            }
        }
    }

    if ( needRepaint )
    {
        emit paintGuides( true );
        paint();
        emit paintGuides( false );
    }
}


void KoGuides::repaintSnapping( const QPointF &snappedPoint, SnapStatus snapStatus )
{
    bool needRepaint = false;
    for ( int i = 0; i < GL_END; ++i )
    {
        QList<KoGuideLine *>::const_iterator it = m_guideLines[i].constBegin();
        for ( ; it != m_guideLines[i].constEnd(); ++it )
        {
            if ( ( *it )->orientation == Qt::Horizontal && ( snapStatus & Qt::Horizontal ) )
            {
                if( virtuallyEqual( snappedPoint.y(), (*it)->position ) )
                {
                    if ( ! ( *it )->snapping )
                    {
                        ( *it )->snapping = true;
                        needRepaint = true;
                    }
                }
                else if ( ( *it )->snapping )
                {
                    ( *it )->snapping = false;
                    needRepaint = true;
                }
            }
            else
            {
                if ( snapStatus & Qt::Vertical )
                {
                    if( virtuallyEqual( snappedPoint.x(), (*it)->position ) )
                    {
                        if ( ! ( *it )->snapping )
                        {
                            ( *it )->snapping = true;
                            needRepaint = true;
                        }
                    }
                    else if ( ( *it )->snapping )
                    {
                        ( *it )->snapping = false;
                        needRepaint = true;
                    }
                }
            }
        }
    }

    if ( needRepaint )
    {
        emit paintGuides( true );
        paint();
        emit paintGuides( false );
    }
}


void KoGuides::repaintAfterSnapping()
{
    bool needRepaint = false;

    for ( int i = 0; i < GL_END; ++i )
    {
        QList<KoGuideLine *>::const_iterator it = m_guideLines[i].constBegin();
        for ( ; it != m_guideLines[i].constEnd(); ++it )
        {
            if ( ( *it )->snapping )
            {
                needRepaint = true;
                ( *it )->snapping = false;
            }
        }
    }

    if ( needRepaint )
    {
        emit paintGuides( true );
        paint();
        emit paintGuides( false );
    }
}


void KoGuides::diffNextGuide( QRectF &rect, QPointF &diff )
{
    for ( int i = 0; i < GL_END; ++i )
    {
        QList<KoGuideLine *>::const_iterator it = m_guideLines[i].constBegin();
        for ( ; it != m_guideLines[i].constEnd(); ++it )
        {
            if ( ( *it )->orientation == Qt::Horizontal )
            {
                qreal moveyl = ( *it )->position - rect.top();
                qreal moveyr = ( *it )->position - rect.bottom();
                if ( diff.y() > 0 )
                {
                    if ( moveyl < diff.y() && moveyl > 1E-10 )
                    {
                        diff.setY( moveyl );
                    }
                    if ( moveyr < diff.y() && moveyr > 1E-10 )
                    {
                        diff.setY( moveyr );
                    }
                }
                else if ( diff.y() < 0 )
                {
                    if ( moveyl > diff.y() && moveyl < -1E-10 )
                    {
                        diff.setY( moveyl );
                    }
                    if ( moveyr > diff.y() && moveyr < -1E-10 )
                    {
                        diff.setY( moveyr );
                    }
                }
            }
            else
            {
                qreal movexl = ( *it )->position - rect.left();
                qreal movexr = ( *it )->position - rect.right();
                if ( diff.x() > 0 )
                {
                    if ( movexl < diff.x() && movexl > 1E-10 )
                    {
                        diff.setX( movexl );
                    }
                    if ( ( movexr < diff.x() ) && movexr > 1E-10 )
                    {
                        diff.setX( movexr );
                    }
                }
                else if ( diff.x() < 0 )
                {
                    if ( movexl > diff.x() && movexl < -1E-10 )
                    {
                        diff.setX( movexl );
                    }
                    if ( movexr > diff.x() && movexr < -1E-10 )
                    {
                        diff.setX( movexr );
                    }
                }
            }
        }
    }
}


void KoGuides::moveGuide( const QPoint &pos, bool horizontal, int rulerWidth )
{
    int x = pos.x() - rulerWidth;
    int y = pos.y() - rulerWidth;
    QPoint p( x, y );
    if ( !m_insertGuide )
    {
        if ( ! horizontal && x > 0 )
        {
            m_insertGuide = true;
            add( Qt::Vertical, p );
        }
        else if ( horizontal && y > 0 )
        {
            m_insertGuide = true;
            add( Qt::Horizontal, p );
        }
        if ( m_insertGuide )
        {
            QMouseEvent e( QEvent::MouseButtonPress, p, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier );
            mousePressEvent( &e );
        }
    }
    else
    {
        QMouseEvent e( QEvent::MouseMove, p, Qt::NoButton, Qt::LeftButton, Qt::NoModifier );
        mouseMoveEvent( &e );
    }
}


void KoGuides::addGuide( const QPoint &pos, bool /* horizontal */, int rulerWidth )
{
    int x = pos.x() - rulerWidth;
    int y = pos.y() - rulerWidth;
    QPoint p( x, y );
    m_insertGuide = false;
    QMouseEvent e( QEvent::MouseButtonRelease, p, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier );
    mouseReleaseEvent( &e );
}


void KoGuides::slotChangePosition()
{
    QPointF p( mapFromScreen( m_lastPoint ) );
    KoGuideLine * guideLine = find( p, m_zoomHandler->unzoomItY( 2 ) );

    const KoPageLayout& pl = m_view->koDocument()->pageLayout();
    qreal max = 0.0;
    if ( guideLine->orientation == Qt::Vertical )
    {
        max = qMax( pl.width, m_zoomHandler->unzoomItX( m_view->canvas()->size().width() + m_view->canvasXOffset() - 1 ) );
    }
    else
    {
        max = qMax( pl.height, m_zoomHandler->unzoomItY( m_view->canvas()->size().height() + m_view->canvasYOffset() - 1 ) );
    }

    KoGuideLineDia dia( 0, guideLine->position, 0.0, max, m_view->koDocument()->unit() );
    if ( dia.exec() == QDialog::Accepted )
    {
        guideLine->position = dia.pos();
        paint();
        emit guideLinesChanged( m_view );
    }
}


void KoGuides::slotRemove()
{
    removeSelected();
    paint();
}


void KoGuides::paint()
{
    m_view->canvas()->repaint();
}


void KoGuides::add( Qt::Orientation o, QPoint &pos )
{
    QPointF p( mapFromScreen( pos ) );
    KoGuideLine *guideLine = new KoGuideLine( o, o == Qt::Vertical ? p.x(): p.y() );
    m_guideLines[GL].append( guideLine );
}


void KoGuides::select( KoGuideLine *guideLine )
{
    guideLine->selected = true;
    if ( m_guideLines[GL].removeAll( guideLine ) == 1 )
    {
        m_guideLines[GL_SELECTED].append( guideLine );
    }
}


void KoGuides::unselect( KoGuideLine *guideLine )
{
    guideLine->selected = false;
    if ( m_guideLines[GL_SELECTED].removeAll( guideLine ) == 1 )
    {
        m_guideLines[GL].append( guideLine );
    }
}


bool KoGuides::unselectAll()
{
    bool selected = m_guideLines[GL_SELECTED].empty() == false;
    
    QList<KoGuideLine *>::iterator it = m_guideLines[GL_SELECTED].begin();
    for ( ; it != m_guideLines[GL_SELECTED].end(); ++it )
    {
        ( *it )->selected = false;
        m_guideLines[GL].append( *it );
    }
    m_guideLines[GL_SELECTED].clear();
    
    return selected;
}


void KoGuides::removeSelected()
{
    QList<KoGuideLine *>::iterator it = m_guideLines[GL_SELECTED].begin();
    for ( ; it != m_guideLines[GL_SELECTED].end(); ++it )
    {
        delete ( *it );
    }
    m_guideLines[GL_SELECTED].clear();
}


bool KoGuides::hasSelected()
{
    return m_guideLines[GL_SELECTED].empty() == false;
}


KoGuides::KoGuideLine * KoGuides::find( QPointF &p, qreal diff )
{
    QList<KoGuideLine *>::iterator it = m_guideLines[GL_SELECTED].begin();
    for ( ; it != m_guideLines[GL_SELECTED].end(); ++it )
    {
        if ( ( *it )->orientation == Qt::Vertical && qAbs( ( *it )->position - p.x() ) < diff )
        {
            return *it;
        }
        if ( ( *it )->orientation == Qt::Horizontal && qAbs( ( *it )->position - p.y() ) < diff )
        {
            return *it;
        }
    }

    it = m_guideLines[GL].begin();
    for ( ; it != m_guideLines[GL].end(); ++it )
    {
        if ( ( *it )->orientation == Qt::Vertical && qAbs( ( *it )->position - p.x() ) < diff )
        {
            return *it;
        }
        if ( ( *it )->orientation == Qt::Horizontal && qAbs( ( *it )->position - p.y() ) < diff )
        {
            return *it;
        }
    }
    return 0;
}


void KoGuides::moveSelectedBy( QPoint &p )
{
    QPointF point( m_zoomHandler->viewToDocument( p ) );
    if ( m_guideLines[GL_SELECTED].count() > 1 )
    {
        const KoPageLayout& pl = m_view->koDocument()->pageLayout();
        qreal right = qMax( pl.width, m_zoomHandler->unzoomItX( m_view->canvas()->width() + m_view->canvasXOffset() - 1 ) );
        qreal bottom = qMax( pl.height, m_zoomHandler->unzoomItY( m_view->canvas()->height() + m_view->canvasYOffset() - 1 ) );

        QList<KoGuideLine *>::iterator it = m_guideLines[GL_SELECTED].begin();
        for ( ; it != m_guideLines[GL_SELECTED].end(); ++it )
        {
            if ( ( *it )->orientation == Qt::Vertical )
            {
                qreal tmp = ( *it )->position + point.x();
                if ( tmp < 0 )
                {
                    point.setX( point.x() - tmp );
                }
                else if ( tmp > right )
                {
                    point.setX( point.x() - ( tmp - right ) );
                }
            }
            else
            {
                qreal tmp = ( *it )->position + point.y();
                if ( tmp < 0 )
                {
                    point.setY( point.y() - tmp );
                }
                else if ( tmp > bottom )
                {
                    point.setY( point.y() - ( tmp - bottom ) );
                }
            }
        }
    }
    QList<KoGuideLine *>::iterator it = m_guideLines[GL_SELECTED].begin();
    for ( ; it != m_guideLines[GL_SELECTED].end(); ++it )
    {
        ( *it )->snapping = false;

        if ( ( *it )->orientation == Qt::Vertical && p.x() != 0 )
        {
            ( *it )->position = ( *it )->position + point.x();
        }
        else if ( ( *it )->orientation == Qt::Horizontal && p.y() != 0 )
        {
            ( *it )->position = ( *it )->position + point.y();
        }
    }
}


QPointF KoGuides::mapFromScreen( const QPoint & pos )
{
    int x = pos.x() + m_view->canvasXOffset();
    int y = pos.y() + m_view->canvasYOffset();
    qreal xf = m_zoomHandler->unzoomItX( x );
    qreal yf = m_zoomHandler->unzoomItY( y );
    return QPointF( xf, yf );
}


QPoint KoGuides::mapToScreen( const QPointF & pos )
{
    int x = m_zoomHandler->zoomItX( pos.x() ) - m_view->canvasXOffset();
    int y = m_zoomHandler->zoomItY( pos.y() ) - m_view->canvasYOffset();
    return QPoint( x, y );
}


#include "KoGuides.moc"
