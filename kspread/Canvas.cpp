/* This file is part of the KDE project

   Copyright 2006 Robert Knight <robertknight@gmail.com>
   Copyright 2006 Inge Wallin <inge@lysator.liu.se>
   Copyright 2006 Stefan Nikolaus <stefan.nikolaus@kdemail.net>
   Copyright 1999-2002,2004 Laurent Montel <montel@kde.org>
   Copyright 2002-2005 Ariya Hidayat <ariya@kde.org>
   Copyright 1999-2004 David Faure <faure@kde.org>
   Copyright 2004-2005 Meni Livne <livne@kde.org>
   Copyright 2001-2003 Philipp Mueller <philipp.mueller@gmx.de>
   Copyright 2002-2003 Norbert Andres <nandres@web.de>
   Copyright 2003 Hamish Rodda <rodda@kde.org>
   Copyright 2003 Joseph Wenninger <jowenn@kde.org>
   Copyright 2003 Lukas Tinkl <lukas@kde.org>
   Copyright 2000-2002 Werner Trobin <trobin@kde.org>
   Copyright 2002 Harri Porten <porten@kde.org>
   Copyright 2002 John Dailey <dailey@vt.edu>
   Copyright 2002 Daniel Naber <daniel.naber@t-online.de>
   Copyright 1999-2000 Torben Weis <weis@kde.org>
   Copyright 1999-2000 Stephan Kulow <coolo@kde.org>
   Copyright 2000 Bernd Wuebben <wuebben@kde.org>
   Copyright 2000 Wilco Greven <greven@kde.org>
   Copyright 2000 Simon Hausmann <hausmann@kde.org
   Copyright 1999 Michael Reiher <michael.reiher@gmx.de>
   Copyright 1999 Boris Wedl <boris.wedl@kfunigraz.ac.at>
   Copyright 1999 Reginald Stadlbauer <reggie@kde.org>

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
 * Boston, MA 02110-1301, USA.
*/

// std
#include <assert.h>
#include <float.h>
#include <stdlib.h>

// Qt
#include <QApplication>
#include <QBuffer>
#include <QByteArray>
#include <QClipboard>
#include <QDragLeaveEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QEvent>
#include <QFocusEvent>
#include <QKeyEvent>
#include <QLabel>
#include <QList>
#include <QMouseEvent>
#include <QPainter>
#include <QPaintEvent>
#include <QPixmap>
#include <QPoint>
#include <QResizeEvent>
#include <QScrollBar>
#include <QTextStream>
#include <QTimer>
#include <QToolTip>
#include <QWheelEvent>
#include <QWidget>

// KDE
#include <kcursor.h>
#include <kdebug.h>
#include <kmessagebox.h>
#include <krun.h>
#include <kmimetype.h>
#include <ksharedptr.h>
#include <kwordwrap.h>

// KOffice
#include <KoOasisStore.h>
#include <KoStore.h>
#include <KoXmlWriter.h>
#include <KoDocumentChild.h>

// KSpread
#include "CellView.h"
#include "Commands.h"
#include "Doc.h"
#include "Editors.h"
#include "Global.h"
#include "Localization.h"
#include "Map.h"
#include "Format.h"
#include "Selection.h"
#include "Sheet.h"
#include "SheetView.h"
#include "StyleManipulators.h"
#include "Undo.h"
#include "Util.h"
#include "Validity.h"
#include "View.h"

#include "Border.h"
#include "CanvasPrivate.h"
#include "Canvas.h"

#define MIN_SIZE 10

using namespace KSpread;

/****************************************************************
 *
 * Canvas
 *
 ****************************************************************/

Canvas::Canvas(View *view)
  : QWidget( view )
{
  d = new Private;

  setAttribute( Qt::WA_OpaquePaintEvent );
  setAttribute( Qt::WA_StaticContents );

  d->cellEditor = 0;
  d->chooseCell = false;
  d->validationInfo = 0;

  QWidget::setFocusPolicy( Qt::StrongFocus );

  d->dragStart = QPoint( -1, -1 );
  d->dragging = false;


  d->defaultGridPen.setColor( Qt::lightGray );
  d->defaultGridPen.setWidth( 1 );
  d->defaultGridPen.setStyle( Qt::SolidLine );

  d->xOffset = 0.0;
  d->yOffset = 0.0;

  d->view = view;
  // m_eAction = DefaultAction;
  d->mouseAction = NoAction;
  d->rubberBandStarted = false;
  // m_bEditDirtyFlag = false;

  //Now built afterwards(David)
  //d->editWidget = d->view->editWidget();
  d->posWidget = d->view->posWidget();

  setMouseTracking( true );
  d->mousePressed = false;
  d->mouseSelectedObject = false;
  d->drawContour = false;
  d->modType = MT_NONE;

  d->m_resizeObject = 0;
  d->m_ratio = 0.0;
  d->m_isMoving = false;
  d->m_objectDisplayAbove = false;
  d->m_isResizing = false;

  d->prevSpokenPointerRow = -1;
  d->prevSpokenPointerCol = -1;
  d->prevSpokenFocusRow = -1;
  d->prevSpokenFocusCol = -1;
  d->prevSpokenRow = -1;
  d->prevSpokenCol = -1;

// TODO Stefan: Still neeeded?
//   d->scrollTimer = new QTimer( this );
//   connect (d->scrollTimer, SIGNAL( timeout() ), this, SLOT( doAutoScroll() ) );

  if (d->view)
  {
    connect( d->view, SIGNAL( autoScroll( const QPoint & )),
             this, SLOT( slotAutoScroll( const QPoint &)));
  }
  setFocus();
  installEventFilter( this ); // for TAB key processing, otherwise focus change
  setAcceptDrops( true );
  setAttribute(Qt::WA_InputMethodEnabled, true); // ensure using the InputMethod
}

Canvas::~Canvas()
{
// FIXME Stefan: Still needed?
//   delete d->scrollTimer;
  delete d->validationInfo;
  delete d;
}

KSpread::View* Canvas::view() const
{
  return d->view;
}

Doc* Canvas::doc() const
{
  return d->view->doc();
}

void Canvas::setEditWidget( KSpread::EditWidget * ew )
{
  d->editWidget = ew;
}

KSpread::EditWidget* Canvas::editWidget() const
{
  return d->editWidget;
}

CellEditor* Canvas::editor() const
{
  return d->cellEditor;
}

double Canvas::xOffset() const
{
  return d->xOffset;
}

double Canvas::yOffset() const
{
  return d->yOffset;
}

void Canvas::setXOffset( double _xOffset )
{
  d->xOffset = _xOffset;
}

void Canvas::setYOffset( double _yOffset )
{
  d->yOffset = _yOffset;
}

const QPen& Canvas::defaultGridPen() const
{
  return d->defaultGridPen;
}

void Canvas::setLastEditorWithFocus( Canvas::EditorType type )
{
  d->focusEditorType = type;
}

Canvas::EditorType Canvas::lastEditorWithFocus() const
{
  return d->focusEditorType;
}


bool Canvas::eventFilter( QObject *o, QEvent *e )
{
  /* this canvas event filter acts on events sent to the line edit as well
     as events to this filter itself.
  */
  if ( !o || !e )
    return true;
  switch ( e->type() )
  {
  case QEvent::KeyPress:
  {
    QKeyEvent * keyev = static_cast<QKeyEvent *>(e);
    if ((keyev->key()==Qt::Key_Tab) || (keyev->key()==Qt::Key_Backtab))
    {
      keyPressEvent ( keyev );
      return true;
    }
    break;
  }
  case QEvent::InputMethod:
  {
      //QIMEvent * imev = static_cast<QIMEvent *>(e);
      //processIMEvent( imev );
      //break;
  }
  case QEvent::ToolTip:
  {
    QHelpEvent* helpEvent = static_cast<QHelpEvent*>( e );
    showToolTip( helpEvent->pos() );
  }
  default:
    break;
  }
  return false;
}

bool Canvas::focusNextPrevChild( bool )
{
    return true; // Don't allow to go out of the canvas widget by pressing "Tab"
}

Selection* Canvas::selection() const
{
  return d->view->selection();
}

Selection* Canvas::choice() const
{
  return d->view->choice();
}

QPoint Canvas::marker() const
{
    return d->view->selection()->marker();
}

int Canvas::markerColumn() const
{
    return d->view->selection()->marker().x();
}

int Canvas::markerRow() const
{
    return d->view->selection()->marker().y();
}

double Canvas::zoom() const
{
  return d->view->zoom();
}

void Canvas::setChooseMode(bool state)
{
  d->chooseCell = state;
}

bool Canvas::chooseMode() const
{
  return d->chooseCell;
}

void Canvas::startChoose()
{
  if ( d->chooseCell )
    return;

  choice()->clear();
  choice()->setSheet(activeSheet());

  // It is important to enable this AFTER we set the rect!
  d->chooseCell = true;
}

void Canvas::startChoose( const QRect& rect )
{
  if (d->chooseCell)
    return;

  choice()->setSheet(activeSheet());
  choice()->initialize(rect);

  // It is important to enable this AFTER we set the rect!
  d->chooseCell = true;
}

void Canvas::endChoose()
{
  // While entering a formula the choose mode is turned on and off.
  // Clear the choice even if we are not in choose mode. Otherwise,
  // cell references will stay highlighted.
  if (!choice()->isEmpty())
  {
    choice()->clear();
    update();
  }

  if ( !d->chooseCell )
    return;

  d->chooseCell = false;

  Sheet* sheet = choice()->sheet();
  if (sheet)
  {
    d->view->setActiveSheet(sheet);
  }
}

HBorder* Canvas::hBorderWidget() const
{
  return d->view->hBorderWidget();
}

VBorder* Canvas::vBorderWidget() const
{
  return d->view->vBorderWidget();
}

QScrollBar* Canvas::horzScrollBar() const
{
  return d->view->horzScrollBar();
}

QScrollBar* Canvas::vertScrollBar() const
{
  return d->view->vertScrollBar();
}

Sheet* Canvas::findSheet( const QString& _name ) const
{
  return d->view->doc()->map()->findSheet( _name );
}

Sheet* Canvas::activeSheet() const
{
  return d->view->activeSheet();
}

void Canvas::validateSelection()
{
  register Sheet * const sheet = activeSheet();
  if (!sheet)
    return;

    if ( selection()->isSingular() )
    {
        int col = selection()->marker().x();
        int row = selection()->marker().y();
        Cell * cell = sheet->cellAt( col,row );
        Validity validity = cell->validity( col, row );
        if ( cell && validity.displayValidationInformation() )
        {
            QString title = validity.titleInfo();
            QString message = validity.messageInfo();
            if ( title.isEmpty() && message.isEmpty() )
                return;

            if ( !d->validationInfo )
                d->validationInfo = new QLabel(  this );
            kDebug(36001)<<" display info validation\n";
            double u = cell->dblWidth( col );
            double v = cell->dblHeight( row );
            double xpos = sheet->dblColumnPos( markerColumn() ) - xOffset();
            double ypos = sheet->dblRowPos( markerRow() ) - yOffset();
            // Special treatment for obscured cells.
            if ( cell->isPartOfMerged() )
            {
                cell = cell->masterCell();
                int moveX = cell->column();
                int moveY = cell->row();

                // Use the obscuring cells dimensions
                u = cell->dblWidth( moveX );
                v = cell->dblHeight( moveY );
                xpos = sheet->dblColumnPos( moveX );
                ypos = sheet->dblRowPos( moveY );
            }
            //d->validationInfo->setGeometry( 3, y + 3, len + 2, hei + 2 );
            d->validationInfo->setAlignment( Qt::AlignVCenter );
            QPainter painter;
            painter.begin( this );
            int len = 0;
            int hei = 0;
            QString resultText;
            if ( !title.isEmpty() )
            {
                len = painter.fontMetrics().width( title );
                hei = painter.fontMetrics().height();
                resultText = title + '\n';
            }
            if ( !message.isEmpty() )
            {
                int i = 0;
                int pos = 0;
                QString t;
                do
                {
                    i = message.indexOf( "\n", pos );
                    if ( i == -1 )
                        t = message.mid( pos, message.length() - pos );
                    else
                    {
                        t = message.mid( pos, i - pos );
                        pos = i + 1;
                    }
                    hei += painter.fontMetrics().height();
                    len = qMax( len, painter.fontMetrics().width( t ) );
                }
                while ( i != -1 );
                resultText += message;
            }
            painter.end();
            d->validationInfo->setText( resultText );

            QRectF unzoomedMarker( xpos - xOffset()+u,
                                   ypos - yOffset()+v,
                                   len,
                                   hei );
            QRect marker( d->view->doc()->zoomRectOld( unzoomedMarker ) );

            d->validationInfo->setGeometry( marker );
            d->validationInfo->show();
        }
        else
        {
            delete d->validationInfo;
            d->validationInfo = 0;
        }
    }
    else
    {
        delete d->validationInfo;
        d->validationInfo = 0;
    }
}


void Canvas::scrollToCell(QPoint location) const
{
  register Sheet * const sheet = activeSheet();
  if (!sheet)
    return;

  if (d->view->isLoading())
    return;

  /* we don't need this cell ptr, but this call is necessary to update the
     scroll bar correctly.  I don't like having that as part of the cellAt function
     but I suppose that's ok for now.
  */
  Cell* cell = sheet->cellAt(location.x(), location.y(), true);
  Q_UNUSED(cell);

  double  unzoomedWidth  = d->view->doc()->unzoomItXOld( width() );
  double  unzoomedHeight = d->view->doc()->unzoomItYOld( height() );

  // xpos is the position of the cell in the current window in unzoomed
  // document coordinates.
  double xpos = sheet->dblColumnPos( location.x() ) - xOffset();
  double ypos = sheet->dblRowPos( location.y() ) - yOffset();

  double minY = 40.0;
  double maxY = unzoomedHeight - 40.0;

    // Left to right sheet.

    double minX = 100.0; // less than that, we scroll
    double maxX = unzoomedWidth - 100.0; // more than that, we scroll

    // Do we need to scroll left?
    if ( xpos < minX )
    {
        if ( sheet->layoutDirection() == Sheet::LeftToRight )
            horzScrollBar()->setValue( (int) (xOffset() + xpos - minX ) );
        else
            horzScrollBar()->setValue( (int) ( horzScrollBar()->maximum() - (xOffset() + xpos - minX ) ) );
    }

    // Do we need to scroll right?
    else if ( xpos > maxX )
    {
      double horzScrollBarValue = xOffset() + xpos - maxX;
      double horzScrollBarValueMax = sheet->sizeMaxX() - unzoomedWidth;

      //We don't want to display any area > KS_colMax widths
      if ( horzScrollBarValue > horzScrollBarValueMax )
        horzScrollBarValue = horzScrollBarValueMax;

        if ( sheet->layoutDirection() == Sheet::LeftToRight )
            horzScrollBar()->setValue( (int) horzScrollBarValue );
        else
            horzScrollBar()->setValue( (int) ( horzScrollBar()->maximum() - horzScrollBarValue ) );
    }

#if 0
  kDebug(36005) << "------------------------------------------------" << endl;
  kDebug(36005) << "scrollToCell(): at location [" << location.x() << ","
           << location.y() << "]" << endl;
  kDebug(36005) << "Unzoomed view size: [" << unzoomedWidth << ","
           << unzoomedHeight << "]" << endl;
  kDebug(36005) << "Position: [" << xpos << "," << ypos << "]" << endl;
  kDebug(36005) << "Canvas::scrollToCell : height=" << height() << endl;
  kDebug(36005) << "Canvas::scrollToCell : width=" << width() << endl;
  kDebug(36005) << "ltr: XPos: " << xpos << ", min: " << minX << ", maxX: " << maxX << endl;
  kDebug(36005) << "ltr: YPos: " << ypos << ", min: " << minY << ", maxY: " << maxY << endl;
#endif

  // do we need to scroll up
  if ( ypos < minY )
    vertScrollBar()->setValue( (int) (yOffset() + ypos - minY ) );

  // do we need to scroll down
  else if ( ypos > maxY )
  {
    double vertScrollBarValue = yOffset() + ypos - maxY;
    double vertScrollBarValueMax = sheet->sizeMaxY() - unzoomedHeight;

    //We don't want to display any area > KS_rowMax heights
    if ( vertScrollBarValue > vertScrollBarValueMax )
      vertScrollBarValue = vertScrollBarValueMax;

    vertScrollBar()->setValue( (int) vertScrollBarValue );
  }
}

void Canvas::slotScrollHorz( int _value )
{
    register Sheet * const sheet = activeSheet();
    if (!sheet)
        return;

    kDebug(36005) << "slotScrollHorz: value = " << _value << endl;
    //kDebug(36005) << kBacktrace() << endl;

    if ( sheet->layoutDirection() == Sheet::RightToLeft )
        _value = horzScrollBar()->maximum() - _value;

    if ( _value < 0.0 ) {
        kDebug (36001)
                << "Canvas::slotScrollHorz: value out of range (_value: "
                << _value << ')' << endl;
        _value = 0;
    }

    double xpos = sheet->dblColumnPos( qMin( KS_colMax, sheet->maxColumn()+10 ) ) - d->xOffset;
    if ( _value > ( xpos + d->xOffset ) )
        _value = (int) ( xpos + d->xOffset );

    sheet->enableScrollBarUpdates( false );

    // Relative movement
    // NOTE Stefan: Always scroll by whole pixels, otherwise we'll get offsets.
    int dx = qRound( d->view->doc()->zoomItX( d->xOffset - _value ) );

    // New absolute position
    // NOTE Stefan: Always store whole pixels, otherwise we'll get offsets.
    d->xOffset -=  d->view->doc()->unzoomItX( dx );

    // scrolling the widgets in the right direction
    if ( sheet->layoutDirection() == Sheet::RightToLeft )
        dx = -dx;
    scroll( dx, 0 );
    hBorderWidget()->scroll( dx, 0 );

    sheet->enableScrollBarUpdates( true );
}

void Canvas::slotScrollVert( int _value )
{
    register Sheet * const sheet = activeSheet();
    if (!sheet)
        return;

    if ( _value < 0 )
    {
        _value = 0;
        kDebug (36001) << "Canvas::slotScrollVert: value out of range (_value: " <<
                _value << ')' << endl;
    }

    double ypos = sheet->dblRowPos( qMin( KS_rowMax, sheet->maxRow()+10 ) );
    if ( _value > ypos )
        _value = (int) ypos;

    sheet->enableScrollBarUpdates( false );

    // Relative movement
    // NOTE Stefan: Always scroll by whole pixels, otherwise we'll get offsets.
    int dy = qRound( d->view->doc()->zoomItY( d->yOffset - _value ) );
    scroll( 0, dy );
    vBorderWidget()->scroll( 0, dy );

    // New absolute position
    // NOTE Stefan: Always store whole pixels, otherwise we'll get offsets.
    d->yOffset -= d->view->doc()->unzoomItY( dy );

    sheet->enableScrollBarUpdates( true );
}

void Canvas::slotMaxColumn( int _max_column )
{
  register Sheet * const sheet = activeSheet();
  if (!sheet)
    return;

  int oldValue = horzScrollBar()->maximum() - horzScrollBar()->value();
  double xpos = sheet->dblColumnPos( qMin( KS_colMax, _max_column + 10 ) ) - xOffset();
  double unzoomWidth = d->view->doc()->unzoomItXOld( width() );

  //Don't go beyond the maximum column range (KS_colMax)
  double sizeMaxX = sheet->sizeMaxX();
  if ( xpos > sizeMaxX - xOffset() - unzoomWidth )
    xpos = sizeMaxX - xOffset() - unzoomWidth;

  horzScrollBar()->setRange( 0, (int) ( xpos + xOffset() ) );

  if ( sheet->layoutDirection()==Sheet::RightToLeft )
    horzScrollBar()->setValue( horzScrollBar()->maximum() - oldValue );
}

void Canvas::slotMaxRow( int _max_row )
{
  register Sheet * const sheet = activeSheet();
  if (!sheet)
    return;

  double ypos = sheet->dblRowPos( qMin( KS_rowMax, _max_row + 10 ) ) - yOffset();
  double unzoomHeight = d->view->doc()->unzoomItYOld( height() );

  //Don't go beyond the maximum row range (KS_rowMax)
  double sizeMaxY = sheet->sizeMaxY();
  if ( ypos > sizeMaxY - yOffset() - unzoomHeight )
    ypos = sizeMaxY - yOffset() - unzoomHeight;

  vertScrollBar()->setRange( 0, (int) ( ypos + yOffset() ) );
}

void Canvas::mouseMoveEvent( QMouseEvent * _ev )
{
  // Dont allow modifications if document is readonly. Selecting is no modification
  if ( (!d->view->koDocument()->isReadWrite()) && (d->mouseAction!=Mark))
    return;

  if ( d->mousePressed && d->modType != MT_NONE )
  {
    QPointF docPoint ( doc()->unzoomPointOldF( _ev->pos() ) );
    docPoint += QPointF( xOffset(), yOffset() );

    if ( d->modType == MT_MOVE )
    {
      if ( !d->m_isMoving )
      {
        d->m_moveStartPoint = objectRect( false ).topLeft();
        d->m_isMoving = true;
      }
      moveObjectsByMouse( docPoint, _ev->modifiers() & Qt::AltModifier || _ev->modifiers() & Qt::ControlModifier );
    }
    else if ( d->m_resizeObject )
    {
      if ( !d->m_isResizing )
        d->m_isResizing = true;

      bool keepRatio = d->m_resizeObject->isKeepRatio();
      if ( _ev->modifiers() & Qt::AltModifier )
      {
        keepRatio = true;
      }
      docPoint  = QPointF( doc()->unzoomPointOldF( _ev->pos() ) );
      resizeObject( d->modType, docPoint, keepRatio );
    }
    return;
  }


  /*if ( d->mousePressed && d->m_resizeObject && d->modType != MT_NONE )
  {
    if ( !d->m_isMoving )
    {
      d->m_isMoving = true;
      update();
    }
    else
      update( d->m_boundingRealRect );


    QRect drawingRect;

    if ( d->modType == MT_MOVE )
    {
      drawingRect = QRect( _ev->pos() - d->m_origPos, d->m_origSize );
      d->m_boundingRealRect = drawingRect;
    }
    else
    {
      drawingRect = doc()->zoomRectOld( calculateNewGeometry(d->modType,  _ev->pos().x(), _ev->pos().y() ) );
      drawingRect.moveBy( (int)( -xOffset() * doc()->zoomedResolutionX() ) , (int)( -yOffset() * doc()->zoomedResolutionY() ) );
    }

    // Autoscrolling
    if ( ( d->modType == MT_MOVE && drawingRect.top() < 0 ) ||  ( d->modType != MT_MOVE && _ev->pos().y() < 0 ) )
    {
      vertScrollBar()->setValue ((int) ( vertScrollBar()->value() -
          autoScrollAccelerationY( - drawingRect.top() ) ) );
    }
    else if ( ( d->modType == MT_MOVE && drawingRect.bottom() > height() ) ||  ( d->modType != MT_MOVE && _ev->pos().y() > height() ) )
    {
      vertScrollBar()->setValue ((int) ( vertScrollBar()->value() +
          autoScrollAccelerationY ( drawingRect.bottom() - height() ) ) );
    }
    if ( ( d->modType == MT_MOVE && drawingRect.left() < 0 ) ||  ( d->modType != MT_MOVE && _ev->pos().x() < 0 ) )
    {
      horzScrollBar()->setValue ((int) ( horzScrollBar()->value() -
          autoScrollAccelerationX( - drawingRect.left() ) ) );
    }
    else if ( ( d->modType == MT_MOVE && drawingRect.right() > width() ) ||  ( d->modType != MT_MOVE && _ev->pos().x() > width() )  )
    {
      horzScrollBar()->setValue ((int) (horzScrollBar()->value() +
          autoScrollAccelerationX( drawingRect.right() - width() ) ) );
    }

    if ( drawingRect.left() < 0 )
    {
        drawingRect.setRight( drawingRect.right() -drawingRect.left() );
        drawingRect.setLeft( 0 );
    }
    if ( drawingRect.top() < 0 )
    {
        drawingRect.setBottom( drawingRect.bottom() -drawingRect.top() );
        drawingRect.setTop( 0 );
    }

    d->m_boundingRealRect = drawingRect; //redraw this area next time the mouse has been moved

    //update( d->m_boundingRealRect );
    QPainter p(this);
    p.setCompositionMode( QPainter::CompositionMode_DestinationOut );
    p.setPen( QPen( black, 0, DotLine ) );
    p.drawRect( drawingRect );
    p.end();
    return;
}*/

  if ( d->dragging )
  {
    return;
  }
  if ( d->dragStart.x() != -1 )
  {
    QPoint p ( (int) _ev->pos().x() + (int) xOffset(),
               (int) _ev->pos().y() + (int) yOffset() );

    if ( ( d->dragStart - p ).manhattanLength() > 4 )
    {
      d->dragging = true;
      startTheDrag();
      d->dragStart.setX( -1 );
    }
    d->dragging = false;
    return;
  }

  // Get info about where the event occurred - this is duplicated
  // in ::mousePressEvent, needs to be separated into one function
  register Sheet * const sheet = activeSheet();
  if (!sheet)
    return;

  if ( d->mouseSelectedObject )
  {
    EmbeddedObject *obj = 0;
    QPoint p ( (int) _ev->x(),
              (int) _ev->y() );
    if ( ( obj = getObject( p, sheet ) ) && obj->isSelected() )
    {
      QRectF const bound = obj->geometry();
      QRect zoomedBound = doc()->zoomRectOld( QRectF( bound.left(), bound.top(),
                                                      bound.width(), bound.height() ) );
      zoomedBound.translate( (int)(-xOffset() * doc()->zoomedResolutionX() ), (int)(-yOffset() * doc()->zoomedResolutionY() ));
      setCursor( obj->getCursor( p, d->modType, zoomedBound ) );
      return;
    }
  }

  double dwidth = d->view->doc()->unzoomItXOld( width() );
  double ev_PosX;
  if ( sheet->layoutDirection()==Sheet::RightToLeft )
  {
    ev_PosX = dwidth - d->view->doc()->unzoomItXOld( _ev->pos().x() ) + xOffset();
  }
  else
  {
    ev_PosX = d->view->doc()->unzoomItXOld( _ev->pos().x() ) + xOffset();
  }
  double ev_PosY = d->view->doc()->unzoomItYOld( _ev->pos().y() ) + yOffset();

  // In which cell did the user click ?
  double xpos;
  double ypos;
  int col = sheet->leftColumn( ev_PosX, xpos );
  int row  = sheet->topRow( ev_PosY, ypos );

  // you cannot move marker when col > KS_colMax or row > KS_rowMax
  if ( col > KS_colMax || row > KS_rowMax )
  {
    kDebug(36005) << "Canvas::mouseMoveEvent: col or row is out of range: "
                   << "col: " << col << " row: " << row << endl;
    return;
  }


  //*** Highlighted Range Resize Handling ***
  if (d->mouseAction == ResizeSelection)
  {
    choice()->update(QPoint(col,row));
    return;
  }

  //Check to see if the mouse is over a highlight range size grip and if it is, change the cursor
  //shape to a resize arrow
  if (highlightRangeSizeGripAt(ev_PosX,ev_PosY))
  {
    if ( sheet->layoutDirection()==Sheet::RightToLeft )
      setCursor( Qt::SizeBDiagCursor );
    else
      setCursor( Qt::SizeFDiagCursor );
    return;
  }

  QRect rct( (d->chooseCell ? choice() : selection())->lastRange() );

  QRect r1;
  QRect r2;

  double lx = sheet->dblColumnPos( rct.left() );
  double rx = sheet->dblColumnPos( rct.right() + 1 );
  double ty = sheet->dblRowPos( rct.top() );
  double by = sheet->dblRowPos( rct.bottom() + 1 );

  r1.setLeft( (int) (lx - 1) );
  r1.setTop( (int) (ty - 1) );
  r1.setRight( (int) (rx + 1) );
  r1.setBottom( (int) (by + 1) );

  r2.setLeft( (int) (lx + 1) );
  r2.setTop( (int) (ty + 1) );
  r2.setRight( (int) (rx - 1) );
  r2.setBottom( (int) (by - 1) );

  // Test whether the mouse is over some anchor
  {
    Cell *cell = sheet->visibleCellAt( col, row );
    QString anchor;
    if ( sheet->layoutDirection()==Sheet::RightToLeft )
    {
        CellView cellView = view()->sheetView( sheet )->cellView( col, row );
        anchor = cellView.testAnchor( cell, cell->dblWidth() - ev_PosX + xpos, ev_PosY - ypos );
    }
    else
    {
        CellView cellView = view()->sheetView( sheet )->cellView( col, row );
        anchor = cellView.testAnchor( cell, ev_PosX - xpos, ev_PosY - ypos );
    }
    if ( !anchor.isEmpty() && anchor != d->anchor )
    {
      setCursor( KCursor::handCursor() );
    }

    d->anchor = anchor;
  }

  // Test wether mouse is over the Selection.handle
  const QRectF selectionHandle = d->view->selection()->selectionHandleArea();
  if ( selectionHandle.contains( QPointF( ev_PosX, ev_PosY ) ) )
  {
    //If the cursor is over the handle, than it might be already on the next cell.
    //Recalculate the cell position!
    col  = sheet->leftColumn( ev_PosX - d->view->doc()->unzoomItXOld( 2 ), xpos );
    row  = sheet->topRow( ev_PosY - d->view->doc()->unzoomItYOld( 2 ), ypos );

    if ( !sheet->isProtected() )
    {
      if ( sheet->layoutDirection()==Sheet::RightToLeft )
        setCursor( Qt::SizeBDiagCursor );
      else
        setCursor( Qt::SizeFDiagCursor );
    }
  }
  else if ( !d->anchor.isEmpty() )
  {
    if ( !sheet->isProtected() )
      setCursor( KCursor::handCursor() );
  }
  else if ( r1.contains( QPoint( (int) ev_PosX, (int) ev_PosY ) )
            && !r2.contains( QPoint( (int) ev_PosX, (int) ev_PosY ) ) )
  {
    setCursor( KCursor::handCursor() );
  }
  else if ( d->chooseCell )
  {
	//Visual cue to indicate that the user can drag-select the choice selection
	setCursor( KCursor::crossCursor() );
  }
  else
  {
	//Nothing special is happening, use a normal arrow cursor
    setCursor( Qt::ArrowCursor );
  }

  // No marking, selecting etc. in progress? Then quit here.
  if ( d->mouseAction == NoAction )
    return;

  // Set the new extent of the selection
  (d->chooseCell ? choice() : selection())->update(QPoint(col,row));
}

void Canvas::mouseReleaseEvent( QMouseEvent* /*_ev*/)
{
  register Sheet * const sheet = activeSheet();
  if (!sheet)
    return;

// TODO Stefan: Still needed?
//   if ( d->scrollTimer->isActive() )
//     d->scrollTimer->stop();

  d->mousePressed = false;
  d->view->disableAutoScroll();

  if ( d->modType != MT_NONE /*&& d->m_resizeObject && d->m_resizeObject->isSelected() */)
  {
    switch ( d->modType )
    {
      case MT_MOVE:
      {
        QPointF move( objectRect( false ).topLeft() - d->m_moveStartPosMouse );
        if ( move != QPointF( 0, 0 ) )
        {
          KCommand *cmd= sheet->moveObject( view(), move.x(), move.y() );
          if(cmd)
            doc()->addCommand( cmd );
        } else
        {
          repaint();
        }
        d->m_isMoving = false;
        break;
      }
      case MT_RESIZE_UP: case MT_RESIZE_LF: case MT_RESIZE_RT: case MT_RESIZE_LU: case MT_RESIZE_LD: case MT_RESIZE_RU: case MT_RESIZE_RD:
        finishResizeObject( i18n("Resize Object") );
        break;
      case MT_RESIZE_DN:
        finishResizeObject( i18n("Resize Object"), false );
        break;
      default:
        break;
    }
    return;
  }

  Selection* selection = d->view->selection();
  QRect s( selection->lastRange() );

  // The user started the drag in the lower right corner of the marker ?
  if ( d->mouseAction == ResizeCell && !sheet->isProtected() )
  {
    sheet->mergeCells(Region(selection->lastRange()));
    d->view->updateEditWidget();
  }
  else if ( d->mouseAction == AutoFill && !sheet->isProtected() )
  {
    QRect dest = s;
    sheet->autofill( d->autoFillSource, dest );

    d->view->updateEditWidget();
  }
  // The user started the drag in the middle of a cell ?
  else if ( d->mouseAction == Mark && !d->chooseCell )
  {
    d->view->updateEditWidget();
  }

  d->mouseAction = NoAction;
  d->dragging = false;
  d->dragStart.setX( -1 );
}

void Canvas::processClickSelectionHandle( QMouseEvent *event )
{
  // Auto fill ? That is done using the left mouse button.
  if ( event->button() == Qt::LeftButton )
  {
    d->mouseAction = AutoFill;
    d->autoFillSource = selection()->lastRange();
  }
  // Resize a cell (done with the right mouse button) ?
  // But for that to work there must not be a selection.
  else if ( event->button() == Qt::MidButton && selection()->isSingular())
  {
    d->mouseAction = ResizeCell;
  }

  return;
}

void Canvas::processLeftClickAnchor()
{
    bool isRefLink = localReferenceAnchor( d->anchor );
    bool isLocalLink = (d->anchor.indexOf("file:") == 0);
    if ( !isRefLink )
    {
	QString type=KMimeType::findByUrl(d->anchor, 0, isLocalLink)->name();

	if ( KRun::isExecutableFile( d->anchor , type ) )
	{
        	//QString question = i18n("Do you want to open this link to '%1'?\n").arg(d->anchor);

          	//question += i18n("Note that opening a link to a local file may "
                          //   "compromise your system's security.");

		QString question = i18n("This link points to the program or script '%1'.\n"
					"Malicious programs can harm your computer.  Are you sure that you want to run this program?", d->anchor);
        	// this will also start local programs, so adding a "don't warn again"
        	// checkbox will probably be too dangerous
        	int choice = KMessageBox::warningYesNo(this, question, i18n("Open Link?"));
        	if ( choice != KMessageBox::Yes )
        	{
			return;
            		//(void) new KRun( d->anchor );
        	}
	}

	new KRun(d->anchor, this);
    }
    else
    {
      selection()->initialize(Region(d->view->doc()->map(), d->anchor, activeSheet()));
    }
}

bool Canvas::highlightRangeSizeGripAt(double x, double y)
{
  if ( !d->chooseCell )
    return false;

  Region::ConstIterator end = choice()->constEnd();
  for (Region::ConstIterator it = choice()->constBegin(); it != end; ++it)
  {
    // TODO Stefan: adapt to Selection::selectionHandleArea
    QRectF visibleRect = cellCoordinatesToDocument( (*it)->rect() );

    QPoint bottomRight((int) visibleRect.right(), (int) visibleRect.bottom());
    QRect handle( ( (int) bottomRight.x() - 6 ),
                  ( (int) bottomRight.y() - 6 ),
                  ( 6 ),
                  ( 6 ) );

    if (handle.contains(QPoint((int) x,(int) y)))
			{
				return true;
			}
	}

	return false;
}

void Canvas::mousePressEvent( QMouseEvent * _ev )
{
  register Sheet * const sheet = activeSheet();
  if (!sheet)
    return;

  if ( _ev->button() == Qt::LeftButton )
  {
    d->mousePressed = true;
    d->view->enableAutoScroll();
  }

  if ( sheet && _ev->button() == Qt::LeftButton)
  {
    d->m_moveStartPosMouse = objectRect( false ).topLeft();
    EmbeddedObject *obj = getObject( _ev->pos(), sheet );

    if ( obj )
    {
       // use ctrl + Button to select / deselect object
      if ( _ev->modifiers() & Qt::ControlModifier && obj->isSelected() )
        deselectObject( obj );
      else if ( _ev->modifiers() & Qt::ControlModifier )
      {
        if ( d->modType == MT_NONE)
          return;

        selectObject( obj );
        raiseObject( obj );
        d->m_moveStartPosMouse = objectRect( false ).topLeft();
      }
      else
      {
        if ( d->modType != MT_MOVE || !obj->isSelected() )
            deselectAllObjects();

        selectObject( obj );

        raiseObject( obj );
        d->m_moveStartPosMouse = objectRect( false ).topLeft();
      }

      // start resizing
      if ( d->modType != MT_MOVE && d->modType != MT_NONE && !obj->isProtect() )
      {
        deselectAllObjects();
        selectObject( obj );
        raiseObject( obj );

        d->m_resizeObject = obj;

        d->m_ratio = static_cast<double>( obj->geometry().width() ) /
            static_cast<double>( obj->geometry().height() );
        d->m_rectBeforeResize = obj->geometry();
      }

      QPointF docPoint ( doc()->unzoomPointOldF( _ev->pos() ) );
      docPoint += QPointF( xOffset(), yOffset() );
      d->m_origMousePos = docPoint;
      d->m_moveStartPosMouse = objectRect( false ).topLeft();
      return;
    }
    else
    {
      d->modType = MT_NONE;
      if ( !( _ev->modifiers() & Qt::ShiftModifier ) && !( _ev->modifiers() & Qt::ControlModifier ) )
        deselectAllObjects();
    }
  }

  // Get info about where the event occurred - this is duplicated
  // in ::mouseMoveEvent, needs to be separated into one function
  double dwidth = d->view->doc()->unzoomItXOld( width() );
  double ev_PosX;
  if ( sheet->layoutDirection()==Sheet::RightToLeft )
  {
    ev_PosX = dwidth - d->view->doc()->unzoomItXOld( _ev->pos().x() ) + xOffset();
  }
  else
  {
    ev_PosX = d->view->doc()->unzoomItXOld( _ev->pos().x() ) + xOffset();
  }
  double ev_PosY = d->view->doc()->unzoomItYOld( _ev->pos().y() ) + yOffset();

  // In which cell did the user click ?
  double xpos;
  double ypos;
  int col  = sheet->leftColumn( ev_PosX, xpos );
  int row  = sheet->topRow( ev_PosY, ypos );
  // you cannot move marker when col > KS_colMax or row > KS_rowMax
  if ( col > KS_colMax || row > KS_rowMax )
  {
    kDebug(36005) << "Canvas::mousePressEvent: col or row is out of range: "
                   << "col: " << col << " row: " << row << endl;
    return;
  }

  // you cannot move marker when col > KS_colMax or row > KS_rowMax
  if ( col > KS_colMax || row > KS_rowMax )
  {
    kDebug(36005) << "Canvas::mousePressEvent: col or row is out of range: "
                   << "col: " << col << " row: " << row << endl;
    return;
  }

  if (d->chooseCell && highlightRangeSizeGripAt(ev_PosX,ev_PosY))
  {
    choice()->setActiveElement(QPoint(col,row));
    d->mouseAction = ResizeSelection;
    return;
  }

  // We were editing a cell -> save value and get out of editing mode
  if ( d->cellEditor && !d->chooseCell )
  {
    deleteEditor( true ); // save changes
  }

// FIXME Stefan: Still needed?
//   d->scrollTimer->start( 50 );

  // Did we click in the lower right corner of the marker/marked-area ?
  if ( selection()->selectionHandleArea().contains( QPointF( ev_PosX, ev_PosY ) ) )
  {
    processClickSelectionHandle( _ev );
    return;
  }


  // TODO Stefan: adapt to non-cont. selection
  {
    // start drag ?
    QRect rct( selection()->lastRange() );

    QRect r1;
    QRect r2;
    {
      double lx = sheet->dblColumnPos( rct.left() );
      double rx = sheet->dblColumnPos( rct.right() + 1 );
      double ty = sheet->dblRowPos( rct.top() );
      double by = sheet->dblRowPos( rct.bottom() + 1 );

      r1.setLeft( (int) (lx - 1) );
      r1.setTop( (int) (ty - 1) );
      r1.setRight( (int) (rx + 1) );
      r1.setBottom( (int) (by + 1) );

      r2.setLeft( (int) (lx + 1) );
      r2.setTop( (int) (ty + 1) );
      r2.setRight( (int) (rx - 1) );
      r2.setBottom( (int) (by - 1) );
    }

    d->dragStart.setX( -1 );

    if ( r1.contains( QPoint( (int) ev_PosX, (int) ev_PosY ) )
         && !r2.contains( QPoint( (int) ev_PosX, (int) ev_PosY ) ) )
    {
      d->dragStart.setX( (int) ev_PosX );
      d->dragStart.setY( (int) ev_PosY );

      return;
    }
  }

  //  kDebug(36005) << "Clicked in cell " << col << ", " << row << endl;

  // Extending an existing selection with the shift button ?
  if ((_ev->modifiers() & Qt::ShiftModifier) &&
      d->view->koDocument()->isReadWrite() &&
      !selection()->isColumnOrRowSelected())
  {
    (d->chooseCell ? choice() : selection())->update(QPoint(col,row));
    return;
  }


  // Go to the upper left corner of the obscuring object if cells are merged
  Cell *cell = sheet->cellAt( col, row );
  if (cell->isPartOfMerged())
  {
    cell = cell->masterCell();
    col = cell->column();
    row = cell->row();
  }

  switch (_ev->button())
  {
    case Qt::LeftButton:
      if (!d->anchor.isEmpty())
      {
        // Hyperlink pressed
        processLeftClickAnchor();
      }
      else if ( _ev->modifiers() & Qt::ControlModifier )
      {
        if (d->chooseCell)
        {
#if 0 // TODO Stefan: remove for NCS of choices
          // Start a marking action
          d->mouseAction = Mark;
          // extend the existing selection
          choice()->extend(QPoint(col,row), sheet);
#endif
        }
        else
        {
          // Start a marking action
          d->mouseAction = Mark;
          // extend the existing selection
          selection()->extend(QPoint(col,row), sheet);
        }
// TODO Stefan: simplification, if NCS of choices is working
/*        (d->chooseCell ? choice() : selection())->extend(QPoint(col,row), sheet);*/
      }
      else
      {
        // Start a marking action
        d->mouseAction = Mark;
        // reinitialize the selection
        (d->chooseCell ? choice() : selection())->initialize(QPoint(col,row), sheet);
      }
      break;
    case Qt::MidButton:
      // Paste operation with the middle button?
      if ( d->view->koDocument()->isReadWrite() && !sheet->isProtected() )
      {
        (d->chooseCell ? choice() : selection())->initialize( QPoint( col, row ), sheet );
        sheet->paste(selection()->lastRange(), true, Paste::Normal,
                     Paste::OverWrite, false, 0, false, QClipboard::Selection);
        sheet->setRegionPaintDirty(*selection());
      }
      break;
    case Qt::RightButton:
      if (!selection()->contains( QPoint( col, row ) ))
      {
        // No selection or the mouse press was outside of an existing selection?
        (d->chooseCell ? choice() : selection())->initialize(QPoint(col,row), sheet);
      }
      break;
    default:
      break;
  }

  scrollToCell(selection()->marker());
  if ( !d->chooseCell )
  {
    d->view->updateEditWidgetOnPress();
  }
  updatePosWidget();

  // Context menu?
  if ( _ev->button() == Qt::RightButton )
  {
    // TODO: Handle anchor // TODO Stefan: ???
    QPoint p = mapToGlobal( _ev->pos() );
    d->view->openPopupMenu( p );
  }
}

void Canvas::startTheDrag()
{
  register Sheet * const sheet = activeSheet();
  if (!sheet)
    return;

  // right area for start dragging
  setCursor( KCursor::handCursor() );

  QDomDocument doc = sheet->saveCellRegion(*selection());

  // Save to buffer
  QBuffer buffer;
  buffer.open( QIODevice::WriteOnly );
  QTextStream str( &buffer );
  str.setCodec( "UTF-8" );
  str << doc;
  buffer.close();

  QMimeData* mimeData = new QMimeData();
  mimeData->setText( sheet->copyAsText( selection() ) );
  mimeData->setData( "application/x-kspread-snippet", buffer.buffer() );

  QDrag *drag = new QDrag(this);
  drag->setMimeData( mimeData );
  drag->start();

  setCursor( KCursor::arrowCursor() );
}

void Canvas::mouseDoubleClickEvent( QMouseEvent*  _ev)
{
  register Sheet * const sheet = activeSheet();
  if (!sheet)
    return;

  EmbeddedObject *obj;
  if ( ( obj = getObject( _ev->pos(), sheet ) ) )
  {
    switch ( obj->getType() )
    {
      case OBJECT_KOFFICE_PART: case OBJECT_CHART:
      {
        dynamic_cast<EmbeddedKOfficeObject*>(obj)->activate( view(), this );
        return;
        break;
      }
      default:
      {
        view()->extraProperties();
        return;
        break;
      }
    }
  }

  if ( d->view->koDocument()->isReadWrite() && sheet )
    createEditor( false /* keep content */);
}

void Canvas::wheelEvent( QWheelEvent* _ev )
{
  if ( _ev->orientation() == Qt::Vertical )
  {
    if ( vertScrollBar() )
      QApplication::sendEvent( vertScrollBar(), _ev );
  }
  else if ( horzScrollBar() )
  {
    QApplication::sendEvent( horzScrollBar(), _ev );
  }
}

void Canvas::paintEvent( QPaintEvent* event )
{
    if ( d->view->doc()->isLoading() || d->view->isLoading() )
        return;

    register Sheet * const sheet = activeSheet();
    if (!sheet)
        return;

    ElapsedTime et( "Painting cells", ElapsedTime::PrintOnlyTime );

    QPainter painter(this);
    painter.setClipRegion( event->region() );

    //Save clip region
    QMatrix matrix;
    if ( d->view )
        matrix = d->view->matrix();
    else
        matrix = painter.matrix();

    paintChildren( painter, matrix );

//     painter.save();
    clipoutChildren( painter );

    painter.setRenderHints( QPainter::Antialiasing | QPainter::TextAntialiasing );
    painter.scale( d->view->doc()->zoomedResolutionX(), d->view->doc()->zoomedResolutionY() );

    QRectF paintRect = d->view->doc()->viewToDocument( rect() );
//     paintRect.translate( -xOffset(), -yOffset() );

    /* paint any visible cell that has the paintDirty flag */
    const QRect visibleRect = visibleCells();
// kDebug() << "visibleCells: " << visibleRect << endl;
// kDebug() << "offset: " << xOffset() << ", " << yOffset() << endl;
    const QPointF topLeft( sheet->dblColumnPos(visibleRect.left()) - xOffset(),
                           sheet->dblRowPos(visibleRect.top()) - yOffset() );
    view()->sheetView( sheet )->setPaintCellRange( visibleRect );
    view()->sheetView( sheet )->paintCells( d->view, painter, paintRect, topLeft );

    /* now paint the selection */
    paintHighlightedRanges(painter, paintRect);
    paintNormalMarker(painter, paintRect);

    //restore clip region with children area
//     painter.restore();
    event->accept();
}

void Canvas::focusInEvent( QFocusEvent* )
{
  if ( !d->cellEditor )
    return;

  //kDebug(36005) << "d->chooseCell : " << ( d->chooseCell ? "true" : "false" ) << endl;
  // If we are in editing mode, we redirect the
  // focus to the CellEditor or EditWidget
  // And we know which, using lastEditorWithFocus.
  // This screws up <Tab> though (David)
  if ( lastEditorWithFocus() == EditWidget )
  {
    d->editWidget->setFocus();
    //kDebug(36005) << "Focus to EditWidget" << endl;
    return;
  }

  //kDebug(36005) << "Redirecting focus to editor" << endl;
  d->cellEditor->setFocus();
}

void Canvas::focusOutEvent( QFocusEvent* )
{
// FIXME Stefan: Still needed?
//     if ( d->scrollTimer->isActive() )
//         d->scrollTimer->stop();
    d->mousePressed = false;
    d->view->disableAutoScroll();
}

void Canvas::dragEnterEvent( QDragEnterEvent* event )
{
  const QMimeData* mimeData = event->mimeData();
  if ( mimeData->hasText() ||
       mimeData->hasFormat( "application/x-kspread-snippet" ) )
  {
    event->acceptProposedAction();
  }
}

void Canvas::dragMoveEvent( QDragMoveEvent* event )
{
  register Sheet * const sheet = activeSheet();
  if (!sheet)
  {
    event->ignore();
    return;
  }

  const QMimeData* mimeData = event->mimeData();
  if ( mimeData->hasText() || mimeData->hasFormat( "application/x-kspread-snippet" ) )
  {
    event->acceptProposedAction();
  }
  else
  {
    event->ignore();
    return;
  }
#if 0 // TODO Stefan: implement drag marking rectangle
  QRect dragMarkingRect;
  if ( mimeData->hasFormat( "application/x-kspread-snippet" ) )
  {
    if ( event->source() == this  )
    {
      kDebug(36005) << "source == this" << endl;
      dragMarkingRect = selection()->boundingRect();
    }
    else
    {
      kDebug(36005) << "source != this" << endl;
      QByteArray data = mimeData->data( "application/x-kspread-snippet" );
      QString errorMsg;
      int errorLine;
      int errorColumn;
      QDomDocument doc;
      if ( !doc.setContent( data, false, &errorMsg, &errorLine, &errorColumn ) )
      {
        // an error occurred
        kDebug(36005) << "Canvas::daragMoveEvent: an error occurred" << endl
                 << "line: " << errorLine << " col: " << errorColumn
                 << ' ' << errorMsg << endl;
        dragMarkingRect = QRect(1,1,1,1);
      }
      else
      {
        QDomElement root = doc.documentElement(); // "spreadsheet-snippet"
        dragMarkingRect = QRect(1,1,
                                root.attribute( "columns" ).toInt(),
                                root.attribute( "rows" ).toInt());
      }
    }
  }
  else // if ( mimeData->hasText() )
  {
    kDebug(36005) << "has text" << endl;
    dragMarkingRect = QRect(1,1,1,1);
  }
#endif
  const QPoint dragAnchor = selection()->boundingRect().topLeft();
  double xpos = sheet->dblColumnPos( dragAnchor.x() );
  double ypos = sheet->dblRowPos( dragAnchor.y() );
  double width  = sheet->columnFormat( dragAnchor.x() )->dblWidth();
  double height = sheet->rowFormat( dragAnchor.y() )->dblHeight();

  // consider also the selection rectangle
  const QRectF noGoArea( xpos - 1, ypos - 1, width + 3, height + 3 );

  // determine the current position
  double eventPosX;
  if (sheet->layoutDirection()==Sheet::RightToLeft)
  {
    eventPosX = this->width() - d->view->doc()->unzoomItXOld( event->pos().x() ) + xOffset();
  }
  else
  {
    eventPosX = d->view->doc()->unzoomItXOld( event->pos().x() ) + xOffset();
  }
  double eventPosY = d->view->doc()->unzoomItYOld( event->pos().y() ) + yOffset();

  if ( noGoArea.contains( QPointF( eventPosX, eventPosY ) ) )
  {
    event->ignore( noGoArea.toRect() );
    return;
  }

#if 0 // TODO Stefan: implement drag marking rectangle
  // determine the cell position under the mouse
  double tmp;
  const int col = sheet->leftColumn( eventPosX, tmp );
  const int row = sheet->topRow( eventPosY, tmp );
  dragMarkingRect.moveTo( QPoint( col, row ) );
  kDebug(36005) << "MARKING RECT = " << dragMarkingRect << endl;
#endif
}

void Canvas::dragLeaveEvent( QDragLeaveEvent * )
{
// FIXME Stefan: Still needed?
//   if ( d->scrollTimer->isActive() )
//     d->scrollTimer->stop();
}

void Canvas::dropEvent( QDropEvent * _ev )
{
  d->dragging = false;
  d->view->disableAutoScroll();
// FIXME Stefan: Still needed?
//   if ( d->scrollTimer->isActive() )
//     d->scrollTimer->stop();
  register Sheet * const sheet = activeSheet();
  if ( !sheet || sheet->isProtected() )
  {
    _ev->ignore();
    return;
  }

  double xpos = sheet->dblColumnPos( selection()->lastRange().left() );
  double ypos = sheet->dblRowPos( selection()->lastRange().top() );
  double width  = sheet->columnFormat( selection()->lastRange().left() )->dblWidth();
  double height = sheet->rowFormat( selection()->lastRange().top() )->dblHeight();

  const QRectF noGoArea( xpos - 1, ypos - 1, width + 3, height + 3 );

  double ev_PosX;
  if ( sheet->layoutDirection() == Sheet::RightToLeft )
    ev_PosX = this->width() - d->view->doc()->unzoomItXOld( _ev->pos().x() ) + xOffset();
  else
    ev_PosX = d->view->doc()->unzoomItXOld( _ev->pos().x() ) + xOffset();

  double ev_PosY = d->view->doc()->unzoomItYOld( _ev->pos().y() ) + yOffset();

  if ( noGoArea.contains( QPointF( ev_PosX, ev_PosY ) ) )
  {
    _ev->ignore();
    return;
  }
  else
    _ev->setAccepted(true);

  double tmp;
  int col = sheet->leftColumn( ev_PosX, tmp );
  int row = sheet->topRow( ev_PosY, tmp );

  const QMimeData* mimeData = _ev->mimeData();
  if ( !mimeData->hasText() && !mimeData->hasFormat( "application/x-kspread-snippet" ) )
  {
    _ev->ignore();
    return;
  }

  QByteArray b;

  bool makeUndo = true;

  if ( mimeData->hasFormat( "application/x-kspread-snippet" ) )
  {
    if ( _ev->source() == this  )
    {
      if ( !d->view->doc()->undoLocked() )
      {
        UndoDragDrop * undo
          = new UndoDragDrop(d->view->doc(), sheet, *selection(),
                             Region(QRect(col, row,
                                   selection()->boundingRect().width(),
                                   selection()->boundingRect().height())));
        d->view->doc()->addCommand( undo );
        makeUndo = false;
      }
      sheet->deleteSelection( selection(), false );
    }


    b = mimeData->data( "application/x-kspread-snippet" );
    sheet->paste( b, QRect( col, row, 1, 1 ), makeUndo );

    _ev->setAccepted(true);
  }
  else
  {
    QString text = mimeData->text();
    sheet->pasteTextPlain( text, QRect( col, row, 1, 1 ) );
    _ev->setAccepted(true);
    return;
  }
}

void Canvas::resizeEvent( QResizeEvent* _ev )
{
  register Sheet * const sheet = activeSheet();
  if (!sheet)
    return;

    double ev_Width = d->view->doc()->unzoomItX( _ev->size().width() );
    double ev_Height = d->view->doc()->unzoomItY( _ev->size().height() );

    int dx = _ev->size().width() - _ev->oldSize().width();
    scroll(-dx, 0);

    // If we rise horizontally, then check if we are still within the valid area (KS_colMax)
    if ( _ev->size().width() > _ev->oldSize().width() )
    {
        int oldValue = horzScrollBar()->maximum() - horzScrollBar()->value();

        if ( ( xOffset() + ev_Width ) > sheet->sizeMaxX() )
        {
          horzScrollBar()->setRange( 0, (int) ( sheet->sizeMaxX() - ev_Width ) );
          if ( sheet->layoutDirection() == Sheet::RightToLeft )
            horzScrollBar()->setValue( horzScrollBar()->maximum() - oldValue );
        }
    }
    // If we lower vertically, then check if the range should represent the maximum range
    else if ( _ev->size().width() < _ev->oldSize().width() )
    {
        int oldValue = horzScrollBar()->maximum() - horzScrollBar()->value();

        if ( horzScrollBar()->maximum() == int( sheet->sizeMaxX() - ev_Width ) )
        {
          horzScrollBar()->setRange( 0, (int) (sheet->sizeMaxX() - ev_Width ) );
          if ( sheet->layoutDirection() == Sheet::RightToLeft )
            horzScrollBar()->setValue( horzScrollBar()->maximum() - oldValue );
        }
    }

    // If we rise vertically, then check if we are still within the valid area (KS_rowMax)
    if ( _ev->size().height() > _ev->oldSize().height() )
    {
        if ( ( yOffset() + ev_Height ) > sheet->sizeMaxY() )
        {
            vertScrollBar()->setRange( 0, (int) (sheet->sizeMaxY() - ev_Height ) );
        }
    }
    // If we lower vertically, then check if the range should represent the maximum range
    else if ( _ev->size().height() < _ev->oldSize().height() )
    {
        if ( vertScrollBar()->maximum() == int( sheet->sizeMaxY() - ev_Height ) )
        {
            vertScrollBar()->setRange( 0, (int) ( sheet->sizeMaxY() - ev_Height ) );
        }
    }
}

QPoint Canvas::cursorPos()
{
  QPoint cursor;
  if (d->chooseCell && !choice()->isEmpty())
    cursor = choice()->cursor();
  else
    cursor = selection()->cursor();

  return cursor;
}

QRect Canvas::moveDirection( KSpread::MoveTo direction, bool extendSelection )
{
  kDebug(36005) << "Canvas::moveDirection" << endl;

  register Sheet * const sheet = activeSheet();
  if (!sheet)
    return QRect();

  QPoint destination;
  QPoint cursor = cursorPos();

  QPoint cellCorner = cursor;
  Cell* cell = sheet->cellAt(cursor.x(), cursor.y());

  /* cell is either the same as the marker, or the cell that is forced obscuring
     the marker cell
  */
  if (cell->isPartOfMerged())
  {
    cell = cell->masterCell();
    cellCorner = QPoint(cell->column(), cell->row());
  }

  /* how many cells must we move to get to the next cell? */
  int offset = 0;
  RowFormat *rl = 0;
  ColumnFormat *cl = 0;
  switch (direction)
    /* for each case, figure out how far away the next cell is and then keep
       going one row/col at a time after that until a visible row/col is found

       NEVER use cell->column() or cell->row() -- it might be a default cell
    */
  {
    case Bottom:
      offset = cell->mergedYCells() - (cursor.y() - cellCorner.y()) + 1;
      rl = sheet->rowFormat( cursor.y() + offset );
      while ( ((cursor.y() + offset) <= KS_rowMax) && rl->hidden())
      {
        offset++;
        rl = sheet->rowFormat( cursor.y() + offset );
      }

      destination = QPoint(cursor.x(), qMin(cursor.y() + offset, KS_rowMax));
      break;
    case Top:
      offset = (cellCorner.y() - cursor.y()) - 1;
      rl = sheet->rowFormat( cursor.y() + offset );
      while ( ((cursor.y() + offset) >= 1) && rl->hidden())
      {
        offset--;
        rl = sheet->rowFormat( cursor.y() + offset );
      }
      destination = QPoint(cursor.x(), qMax(cursor.y() + offset, 1));
      break;
    case Left:
      offset = (cellCorner.x() - cursor.x()) - 1;
      cl = sheet->columnFormat( cursor.x() + offset );
      while ( ((cursor.x() + offset) >= 1) && cl->hidden())
      {
        offset--;
        cl = sheet->columnFormat( cursor.x() + offset );
      }
      destination = QPoint(qMax(cursor.x() + offset, 1), cursor.y());
      break;
    case Right:
      offset = cell->mergedXCells() - (cursor.x() - cellCorner.x()) + 1;
      cl = sheet->columnFormat( cursor.x() + offset );
      while ( ((cursor.x() + offset) <= KS_colMax) && cl->hidden())
      {
        offset++;
        cl = sheet->columnFormat( cursor.x() + offset );
      }
      destination = QPoint(qMin(cursor.x() + offset, KS_colMax), cursor.y());
      break;
    case BottomFirst:
      offset = cell->mergedYCells() - (cursor.y() - cellCorner.y()) + 1;
      rl = sheet->rowFormat( cursor.y() + offset );
      while ( ((cursor.y() + offset) <= KS_rowMax) && rl->hidden())
      {
        ++offset;
        rl = sheet->rowFormat( cursor.y() + offset );
      }

      destination = QPoint( 1, qMin( cursor.y() + offset, KS_rowMax ) );
      break;
  }

  if (extendSelection)
  {
    (d->chooseCell ? choice() : selection())->update(destination);
  }
  else
  {
    (d->chooseCell ? choice() : selection())->initialize(destination, sheet);
  }
  d->view->updateEditWidget();

  return QRect( cursor, destination );
}

void Canvas::processEnterKey(QKeyEvent* event)
{
  // array is true, if ctrl+alt are pressed
  bool array = (event->modifiers() & Qt::AltModifier) &&
      (event->modifiers() & Qt::ControlModifier);

  /* save changes to the current editor */
  if (!d->chooseCell)
  {
    deleteEditor(true, array);
  }

  /* use the configuration setting to see which direction we're supposed to move
     when enter is pressed.
  */
  KSpread::MoveTo direction = d->view->doc()->moveToValue();

  //if shift Button clicked inverse move direction
  if (event->modifiers() & Qt::ShiftModifier)
  {
    switch( direction )
    {
     case Bottom:
      direction = Top;
      break;
     case Top:
      direction = Bottom;
      break;
     case Left:
      direction = Right;
      break;
     case Right:
      direction = Left;
      break;
     case BottomFirst:
      direction = BottomFirst;
      break;
    }
  }

  /* never extend a selection with the enter key -- the shift key reverses
     direction, not extends the selection
  */
  QRect r( moveDirection( direction, false ) );
  d->view->doc()->emitEndOperation( Region(r) );
}

void Canvas::processArrowKey( QKeyEvent *event)
{
  /* NOTE:  hitting the tab key also calls this function.  Don't forget
     to account for it
  */
  register Sheet * const sheet = activeSheet();
  if (!sheet)
    return;

  /* save changes to the current editor */
  if (!d->chooseCell)
  {
    deleteEditor( true );
  }

  KSpread::MoveTo direction = Bottom;
  bool makingSelection = event->modifiers() & Qt::ShiftModifier;

  switch (event->key())
  {
  case Qt::Key_Down:
    direction = Bottom;
    break;
  case Qt::Key_Up:
    direction = Top;
    break;
  case Qt::Key_Left:
    if (sheet->layoutDirection()==Sheet::RightToLeft)
      direction = Right;
    else
      direction = Left;
    break;
  case Qt::Key_Right:
    if (sheet->layoutDirection()==Sheet::RightToLeft)
      direction = Left;
    else
      direction = Right;
    break;
  case Qt::Key_Tab:
      direction = Right;
      break;
  case Qt::Key_Backtab:
      //Shift+Tab moves to the left
      direction = Left;
      makingSelection = false;
      break;
  default:
    Q_ASSERT(false);
    break;
  }

  QRect r( moveDirection( direction, makingSelection ) );
  d->view->doc()->emitEndOperation( Region(r) );
}

void Canvas::processEscapeKey(QKeyEvent * event)
{
  register Sheet * const sheet = activeSheet();
  if (!sheet)
    return;

  if ( d->cellEditor )
    deleteEditor( false );

  if ( view()->isInsertingObject() )
  {
    view()->resetInsertHandle();
    setCursor( Qt::ArrowCursor );
    return;
  }

  event->setAccepted(true); // ?
  QPoint cursor = cursorPos();

  d->view->doc()->emitEndOperation( Region(QRect( cursor, cursor )) );

  if ( d->mousePressed /*&& toolEditMode == TEM_MOUSE */)
  {
    switch (d->modType)
    {
      case MT_RESIZE_UP:
      case MT_RESIZE_DN:
      case MT_RESIZE_LF:
      case MT_RESIZE_RT:
      case MT_RESIZE_LU:
      case MT_RESIZE_LD:
      case MT_RESIZE_RU:
      case MT_RESIZE_RD:
      {
        QRect oldBoundingRect = doc()->zoomRectOld( d->m_resizeObject->geometry()/*getRepaintRect()*/);
        d->m_resizeObject->setGeometry( d->m_rectBeforeResize );
        oldBoundingRect.translate( (int)( -xOffset()*doc()->zoomedResolutionX() ) ,
                            (int)( -yOffset() * doc()->zoomedResolutionY()) );

        sheet->setRegionPaintDirty( Region(oldBoundingRect) );
        repaint( oldBoundingRect );
        repaintObject( d->m_resizeObject );
        d->m_ratio = 0.0;
        d->m_resizeObject = 0;
        d->m_isResizing = false;
        view()->disableAutoScroll();
        d->mousePressed = false;
        d->modType = MT_NONE;
        break;
      }
      case MT_MOVE:
      {
        if ( d->m_isMoving )
        {
          QPointF move( d->m_moveStartPoint - objectRect( false ).topLeft() );
          sheet->moveObject( view(), move, false );
          view()->disableAutoScroll();
          d->mousePressed = false;
          d->modType = MT_NONE;
          d->m_isMoving = false;
          update();
        }
        break;
      }
      default:
        break;
    }
  }
}

bool Canvas::processHomeKey(QKeyEvent* event)
{
  register Sheet * const sheet = activeSheet();
  if (!sheet)
    return false;

  bool makingSelection = event->modifiers() & Qt::ShiftModifier;

  if ( d->cellEditor )
  // We are in edit mode -> go beginning of line
  {
    QApplication::sendEvent( d->editWidget, event );
    return false;
  }
  else
  {
    QPoint destination;
    /* start at the first used cell in the row and cycle through the right until
       we find a cell that has some output text.  But don't look past the current
       marker.
       The end result we want is to move to the left to the first cell with text,
       or just to the first column if there is no more text to the left.

       But why?  In excel, home key sends you to the first column always.
       We might want to change to that behavior.
    */

    if (event->modifiers() & Qt::ControlModifier)
    {
      /* ctrl + Home will always just send us to location (1,1) */
      destination = QPoint( 1, 1 );
    }
    else
    {
      QPoint marker = d->chooseCell ? choice()->marker() : selection()->marker();

      Cell * cell = sheet->getFirstCellRow(marker.y());
      while (cell != 0 && cell->column() < marker.x() && cell->isEmpty())
      {
        cell = sheet->getNextCellRight(cell->column(), cell->row());
      }

      int col = ( cell ? cell->column() : 1 );
      if ( col == marker.x())
        col = 1;
      destination = QPoint(col, marker.y());
    }

    if ( selection()->marker() == destination )
    {
      d->view->doc()->emitEndOperation( Region(QRect( destination, destination ) ) );
      return false;
    }

    if (makingSelection)
    {
      (d->chooseCell ? choice() : selection())->update(destination);
    }
    else
    {
      (d->chooseCell ? choice() : selection())->initialize(destination, sheet);
    }
  }
  return true;
}

bool Canvas::processEndKey( QKeyEvent *event )
{
  register Sheet * const sheet = activeSheet();
  if (!sheet)
    return false;

  bool makingSelection = event->modifiers() & Qt::ShiftModifier;
  Cell* cell = 0;
  QPoint marker = d->chooseCell ? choice()->marker() : selection()->marker();

  // move to the last used cell in the row
  // We are in edit mode -> go beginning of line
  if ( d->cellEditor )
  {
    QApplication::sendEvent( d->editWidget, event );
    d->view->doc()->emitEndOperation( Region( QRect( marker, marker ) ) );
    return false;
  }
  else
  {
    int col = 1;

    cell = sheet->getLastCellRow(marker.y());
    while (cell != 0 && cell->column() > markerColumn() && cell->isEmpty())
    {
      cell = sheet->getNextCellLeft(cell->column(), cell->row());
    }

    col = (cell == 0) ? KS_colMax : cell->column();

    QPoint destination( col, marker.y() );
    if ( destination == marker )
    {
      d->view->doc()->emitEndOperation( Region( QRect( destination, destination ) ) );
      return false;
    }

    if (makingSelection)
    {
      (d->chooseCell ? choice() : selection())->update(destination);
    }
    else
    {
      (d->chooseCell ? choice() : selection())->initialize(destination, sheet);
    }
  }
  return true;
}

bool Canvas::processPriorKey(QKeyEvent *event)
{
  bool makingSelection = event->modifiers() & Qt::ShiftModifier;
  if (!d->chooseCell)
  {
    deleteEditor( true );
  }

  QPoint marker = d->chooseCell ? choice()->marker() : selection()->marker();

  QPoint destination(marker.x(), qMax(1, marker.y() - 10));
  if ( destination == marker )
  {
    d->view->doc()->emitEndOperation( Region( QRect( destination, destination ) ) );
    return false;
  }

  if (makingSelection)
  {
    (d->chooseCell ? choice() : selection())->update(destination);
  }
  else
  {
    (d->chooseCell ? choice() : selection())->initialize(destination, activeSheet());
  }
  return true;
}

bool Canvas::processNextKey(QKeyEvent *event)
{
  bool makingSelection = event->modifiers() & Qt::ShiftModifier;

  if (!d->chooseCell)
  {
    deleteEditor( true /*save changes*/ );
  }

  QPoint marker = d->chooseCell ? choice()->marker() : selection()->marker();
  QPoint destination(marker.x(), qMax(1, marker.y() + 10));

  if ( marker == destination )
  {
    d->view->doc()->emitEndOperation( Region( QRect( destination, destination ) ) );
    return false;
  }

  if (makingSelection)
  {
    (d->chooseCell ? choice() : selection())->update(destination);
  }
  else
  {
    (d->chooseCell ? choice() : selection())->initialize(destination, activeSheet());
  }
  return true;
}

void Canvas::processDeleteKey(QKeyEvent* /* event */)
{
  register Sheet * const sheet = activeSheet();
  if (!sheet)
    return;

  if ( isObjectSelected() )
  {
    d->view->doc()->emitEndOperation( Region( visibleCells() ) );
    d->view->deleteSelectedObjects();
    return;
  }

  d->view->clearTextSelection();
  d->editWidget->setText( "" );

  QPoint cursor = cursorPos();

  d->view->doc()->emitEndOperation( Region( QRect( cursor, cursor ) ) );
  return;
}

void Canvas::processF2Key(QKeyEvent* /* event */)
{
  d->editWidget->setFocus();
  if ( d->cellEditor )
    d->editWidget->setCursorPosition( d->cellEditor->cursorPosition() - 1 );
  d->editWidget->cursorForward( false );


  QPoint cursor = cursorPos();

  d->view->doc()->emitEndOperation( Region( QRect( cursor, cursor ) ) );
  return;
}

void Canvas::processF4Key(QKeyEvent* event)
{
  /* passes F4 to the editor (if any), which will process it
   */
  if ( d->cellEditor )
  {
    d->cellEditor->handleKeyPressEvent( event );
//    d->editWidget->setFocus();
    d->editWidget->setCursorPosition( d->cellEditor->cursorPosition() );
  }
  QPoint cursor = cursorPos();

  d->view->doc()->emitEndOperation( Region( QRect( cursor, cursor ) ) );
  return;
}

void Canvas::processOtherKey(QKeyEvent *event)
{
  register Sheet * const sheet = activeSheet();

  // No null character ...
  if ( event->text().isEmpty() || !d->view->koDocument()->isReadWrite()
       || !sheet || sheet->isProtected() )
  {
    event->setAccepted(true);
  }
  else
  {
    if ( !d->cellEditor && !d->chooseCell )
    {
      // Switch to editing mode
      createEditor();
      d->cellEditor->handleKeyPressEvent( event );
    }
    else if ( d->cellEditor )
      d->cellEditor->handleKeyPressEvent( event );
  }

  QPoint cursor = cursorPos();

  d->view->doc()->emitEndOperation( Region( QRect( cursor, cursor ) ) );

  return;
}

bool Canvas::processControlArrowKey( QKeyEvent *event )
{
  register Sheet * const sheet = activeSheet();
  if (!sheet)
    return false;

  bool makingSelection = event->modifiers() & Qt::ShiftModifier;

  Cell* cell = 0;
  Cell* lastCell;
  QPoint destination;
  bool searchThroughEmpty = true;
  int row;
  int col;

  QPoint marker = d->chooseCell ? choice()->marker() : selection()->marker();

  /* here, we want to move to the first or last cell in the given direction that is
     actually being used.  Ignore empty cells and cells on hidden rows/columns */
  switch ( event->key() )
  {
    //Ctrl+Qt::Key_Up
   case Qt::Key_Up:

    cell = sheet->cellAt( marker.x(), marker.y() );
    if ( (cell != 0) && (!cell->isEmpty()) && (marker.y() != 1))
    {
      lastCell = cell;
      row = marker.y()-1;
      cell = sheet->cellAt(cell->column(), row);
      while ((cell != 0) && (row > 0) && (!cell->isEmpty()) )
      {
        if (!(sheet->rowFormat(cell->row())->hidden()))
        {
          lastCell = cell;
          searchThroughEmpty = false;
        }
        row--;
        if ( row > 0 )
          cell = sheet->cellAt(cell->column(), row);
      }
      cell = lastCell;
    }
    if (searchThroughEmpty)
    {
      cell = sheet->getNextCellUp(marker.x(), marker.y());

      while ((cell != 0) &&
            (cell->isEmpty() || (sheet->rowFormat(cell->row())->hidden())))
      {
        cell = sheet->getNextCellUp(cell->column(), cell->row());
      }
    }

    if (cell == 0)
      row = 1;
    else
      row = cell->row();

    while ( sheet->rowFormat(row)->hidden() )
    {
      row++;
    }

    destination.setX(marker.x());
    destination.setY(row);
    break;

    //Ctrl+Qt::Key_Down
   case Qt::Key_Down:

    cell = sheet->cellAt( marker.x(), marker.y() );
    if ( (cell != 0) && (!cell->isEmpty()) && (marker.y() != KS_rowMax))
    {
      lastCell = cell;
      row = marker.y()+1;
      cell = sheet->cellAt(cell->column(), row);
      while ((cell != 0) && (row < KS_rowMax) && (!cell->isEmpty()) )
      {
        if (!(sheet->rowFormat(cell->row())->hidden()))
        {
          lastCell = cell;
          searchThroughEmpty = false;
        }
        row++;
        cell = sheet->cellAt(cell->column(), row);
      }
      cell = lastCell;
    }
    if (searchThroughEmpty)
    {
      cell = sheet->getNextCellDown(marker.x(), marker.y());

      while ((cell != 0) &&
            (cell->isEmpty() || (sheet->rowFormat(cell->row())->hidden())))
      {
        cell = sheet->getNextCellDown(cell->column(), cell->row());
      }
    }

    if (cell == 0)
      row = marker.y();
    else
      row = cell->row();

    while ( sheet->rowFormat(row)->hidden() )
    {
      row--;
    }

    destination.setX(marker.x());
    destination.setY(row);
    break;

  //Ctrl+Qt::Key_Left
  case Qt::Key_Left:

  if ( sheet->layoutDirection()==Sheet::RightToLeft )
  {
    cell = sheet->cellAt( marker.x(), marker.y() );
    if ( (cell != 0) && (!cell->isEmpty()) && (marker.x() != KS_colMax))
    {
      lastCell = cell;
      col = marker.x()+1;
      cell = sheet->cellAt(col, cell->row());
      while ((cell != 0) && (col < KS_colMax) && (!cell->isEmpty()) )
      {
        if (!(sheet->columnFormat(cell->column())->hidden()))
        {
          lastCell = cell;
          searchThroughEmpty = false;
        }
        col++;
        cell = sheet->cellAt(col, cell->row());
      }
      cell = lastCell;
    }
    if (searchThroughEmpty)
    {
      cell = sheet->getNextCellRight(marker.x(), marker.y());

      while ((cell != 0) &&
            (cell->isEmpty() || (sheet->columnFormat(cell->column())->hidden())))
      {
        cell = sheet->getNextCellRight(cell->column(), cell->row());
      }
    }

    if (cell == 0)
      col = marker.x();
    else
      col = cell->column();

    while ( sheet->columnFormat(col)->hidden() )
    {
      col--;
    }

    destination.setX(col);
    destination.setY(marker.y());
  }
  else
  {
    cell = sheet->cellAt( marker.x(), marker.y() );
    if ( (cell != 0) && (!cell->isEmpty()) && (marker.x() != 1))
    {
      lastCell = cell;
      col = marker.x()-1;
      cell = sheet->cellAt(col, cell->row());
      while ((cell != 0) && (col > 0) && (!cell->isEmpty()) )
      {
        if (!(sheet->columnFormat(cell->column())->hidden()))
        {
          lastCell = cell;
          searchThroughEmpty = false;
        }
        col--;
        if ( col > 0 )
            cell = sheet->cellAt(col, cell->row());
      }
      cell = lastCell;
    }
    if (searchThroughEmpty)
    {
      cell = sheet->getNextCellLeft(marker.x(), marker.y());

      while ((cell != 0) &&
            (cell->isEmpty() || (sheet->columnFormat(cell->column())->hidden())))
      {
        cell = sheet->getNextCellLeft(cell->column(), cell->row());
      }
    }

    if (cell == 0)
      col = 1;
    else
      col = cell->column();

    while ( sheet->columnFormat(col)->hidden() )
    {
      col++;
    }

    destination.setX(col);
    destination.setY(marker.y());
  }
    break;

  //Ctrl+Qt::Key_Right
  case Qt::Key_Right:

  if ( sheet->layoutDirection()==Sheet::RightToLeft )
  {
    cell = sheet->cellAt( marker.x(), marker.y() );
    if ( (cell != 0) && (!cell->isEmpty()) && (marker.x() != 1))
    {
      lastCell = cell;
      col = marker.x()-1;
      cell = sheet->cellAt(col, cell->row());
      while ((cell != 0) && (col > 0) && (!cell->isEmpty()) )
      {
        if (!(sheet->columnFormat(cell->column())->hidden()))
        {
          lastCell = cell;
          searchThroughEmpty = false;
        }
        col--;
        if ( col > 0 )
            cell = sheet->cellAt(col, cell->row());
      }
      cell = lastCell;
    }
    if (searchThroughEmpty)
    {
      cell = sheet->getNextCellLeft(marker.x(), marker.y());

      while ((cell != 0) &&
            (cell->isEmpty() || (sheet->columnFormat(cell->column())->hidden())))
      {
        cell = sheet->getNextCellLeft(cell->column(), cell->row());
      }
    }

    if (cell == 0)
      col = 1;
    else
      col = cell->column();

    while ( sheet->columnFormat(col)->hidden() )
    {
      col++;
    }

    destination.setX(col);
    destination.setY(marker.y());
  }
  else
  {
    cell = sheet->cellAt( marker.x(), marker.y() );
    if ( (cell != 0) && (!cell->isEmpty()) && (marker.x() != KS_colMax))
    {
      lastCell = cell;
      col = marker.x()+1;
      cell = sheet->cellAt(col, cell->row());
      while ((cell != 0) && (col < KS_colMax) && (!cell->isEmpty()) )
      {
        if (!(sheet->columnFormat(cell->column())->hidden()))
        {
          lastCell = cell;
          searchThroughEmpty = false;
        }
        col++;
        cell = sheet->cellAt(col, cell->row());
      }
      cell = lastCell;
    }
    if (searchThroughEmpty)
    {
      cell = sheet->getNextCellRight(marker.x(), marker.y());

      while ((cell != 0) &&
            (cell->isEmpty() || (sheet->columnFormat(cell->column())->hidden())))
      {
        cell = sheet->getNextCellRight(cell->column(), cell->row());
      }
    }

    if (cell == 0)
      col = marker.x();
    else
      col = cell->column();

    while ( sheet->columnFormat(col)->hidden() )
    {
      col--;
    }

    destination.setX(col);
    destination.setY(marker.y());
  }
    break;

  }

  if ( marker == destination )
  {
    d->view->doc()->emitEndOperation( Region( QRect( destination, destination ) ) );
    return false;
  }

  if (makingSelection)
  {
    (d->chooseCell ? choice() : selection())->update(destination);
  }
  else
  {
    (d->chooseCell ? choice() : selection())->initialize(destination, sheet);
  }
  return true;
}


void Canvas::keyPressEvent ( QKeyEvent * _ev )
{
  register Sheet * const sheet = activeSheet();

  if ( !sheet || formatKeyPress( _ev ))
    return;

  // Dont handle the remaining special keys.
  if ( _ev->modifiers() & ( Qt::AltModifier | Qt::ControlModifier ) &&
       (_ev->key() != Qt::Key_Down) &&
       (_ev->key() != Qt::Key_Up) &&
       (_ev->key() != Qt::Key_Right) &&
       (_ev->key() != Qt::Key_Left) &&
       (_ev->key() != Qt::Key_Home) &&
       (_ev->key() != Qt::Key_Enter) &&
       (_ev->key() != Qt::Key_Return) &&
       (_ev->key() != KGlobalSettings::contextMenuKey()))
  {
    QWidget::keyPressEvent( _ev );
    return;
  }

  // Always accept so that events are not
  // passed to the parent.
  _ev->setAccepted(true);

  d->view->doc()->emitBeginOperation(false);
  if ( _ev->key() == KGlobalSettings::contextMenuKey() ) {
    int row = markerRow();
    int col = markerColumn();
    QPoint p(sheet->columnPos(col), sheet->rowPos(row));
    p = mapToGlobal(p);
    d->view->openPopupMenu( p );
  }
  switch( _ev->key() )
  {
   case Qt::Key_Return:
   case Qt::Key_Enter:
    processEnterKey( _ev );
    return;
    break;
   case Qt::Key_Down:
   case Qt::Key_Up:
   case Qt::Key_Left:
   case Qt::Key_Right:
   case Qt::Key_Tab: /* a tab behaves just like a right/left arrow */
   case Qt::Key_Backtab:  /* and so does Shift+Tab */
    if (_ev->modifiers() & Qt::ControlModifier)
    {
      if ( !processControlArrowKey( _ev ) )
        return;
    }
    else
    {
      processArrowKey( _ev );
      return;
    }
    break;

   case Qt::Key_Escape:
    processEscapeKey( _ev );
    return;
    break;

   case Qt::Key_Home:
    if ( !processHomeKey( _ev ) )
      return;
    break;

   case Qt::Key_End:
    if ( !processEndKey( _ev ) )
      return;
    break;

   case Qt::Key_PageUp:  /* Page Up */
    if ( !processPriorKey( _ev ) )
      return;
    break;

   case Qt::Key_PageDown:   /* Page Down */
    if ( !processNextKey( _ev ) )
      return;
    break;

   case Qt::Key_Delete:
    processDeleteKey( _ev );
    return;
    break;

   case Qt::Key_F2:
    processF2Key( _ev );
    return;
    break;

   case Qt::Key_F4:
    processF4Key( _ev );
    return;
    break;

   default:
    processOtherKey( _ev );
    return;
    break;
  }

  //most process*Key methods call emitEndOperation, this only gets called in some situations
  // (after some move operations)
  d->view->doc()->emitEndOperation( Region( visibleCells() ) );
  return;
}
#if 0
void Canvas::processIMEvent( QIMEvent * event )
{
  d->view->doc()->emitBeginOperation( false );
  if ( !d->cellEditor && !d->chooseCell )
  {
    // Switch to editing mode
    createEditor();
    d->cellEditor->handleIMEvent( event );
  }

  QPoint cursor;

  if ( d->chooseCell )
  {
    cursor = choice()->cursor();
    /* if the cursor is unset, pretend we're starting at the regular cursor */
    if (cursor.x() == 0 || cursor.y() == 0)
      cursor = choice()->cursor();
  }
  else
    cursor = selection()->cursor();

  d->view->doc()->emitEndOperation( QRect( cursor, cursor ) );
}
#endif

bool Canvas::formatKeyPress( QKeyEvent * _ev )
{
    if (!(_ev->modifiers() & Qt::ControlModifier ))
        return false;

    int key = _ev->key();
    if ( key != Qt::Key_Exclam && key != Qt::Key_At &&
         key != Qt::Key_Ampersand && key != Qt::Key_Dollar &&
         key != Qt::Key_Percent && key != Qt::Key_AsciiCircum &&
         key != Qt::Key_NumberSign )
        return false;

    StyleManipulator* manipulator = new StyleManipulator();
    manipulator->setSheet( activeSheet() );

    switch ( _ev->key() )
    {
        case Qt::Key_Exclam:
            manipulator->setName( i18n("Number Format") );
            manipulator->setFormatType (Number_format);
            manipulator->setPrecision( 2 );
            break;

        case Qt::Key_Dollar:
            manipulator->setName( i18n("Currency Format") );
            manipulator->setFormatType (Money_format);
            manipulator->setPrecision( d->view->doc()->locale()->fracDigits() );
            break;

        case Qt::Key_Percent:
            manipulator->setName( i18n("Percentage Format") );
            manipulator->setFormatType (Percentage_format);
            break;

        case Qt::Key_At:
            manipulator->setName( i18n("Time Format") );
            manipulator->setFormatType( SecondeTime_format );
            break;

        case Qt::Key_NumberSign:
            manipulator->setName( i18n("Date Format") );
            manipulator->setFormatType( ShortDate_format );
            break;

        case Qt::Key_AsciiCircum:
            manipulator->setName( i18n("Scientific Format") );
            manipulator->setFormatType( Scientific_format );
            break;

        case Qt::Key_Ampersand:
            manipulator->setName( i18n("Change Border") );
            manipulator->setTopBorderPen( QPen( d->view->borderColor(), 1, Qt::SolidLine) );
            manipulator->setBottomBorderPen( QPen( d->view->borderColor(), 1, Qt::SolidLine) );
            manipulator->setLeftBorderPen( QPen( d->view->borderColor(), 1, Qt::SolidLine) );
            manipulator->setRightBorderPen( QPen( d->view->borderColor(), 1, Qt::SolidLine) );
            break;

        default:
            delete manipulator;
            return false;
    }

    manipulator->add( *selection() );
    manipulator->execute();
    _ev->setAccepted(true);

    return true;
}

void Canvas::slotAutoScroll(const QPoint &scrollDistance)
{
  // NOTE Stefan: This slot is triggered by the same signal as
  //              HBorder::slotAutoScroll and VBorder::slotAutoScroll.
  //              Therefore, nothing has to be done except the scrolling was
  //              initiated in the canvas.
  if (!d->mousePressed)
    return;
//   kDebug(36005) << "Canvas::slotAutoScroll(" << scrollDistance << " " << endl;
  horzScrollBar()->setValue( horzScrollBar()->value() + scrollDistance.x() );
  vertScrollBar()->setValue( vertScrollBar()->value() + scrollDistance.y() );
}

// TODO Stefan: Still needed?
#if 0
void Canvas::doAutoScroll()
{
    if ( !d->mousePressed )
    {
        d->scrollTimer->stop();
        return;
    }
    bool select = false;
    QPoint pos = mapFromGlobal( QCursor::pos() );

    //Provide progressive scrolling depending on the mouse position
    if ( pos.y() < 0 )
    {
        vertScrollBar()->setValue ((int) (vertScrollBar()->value() -
                                   autoScrollAccelerationY( - pos.y())));
        select = true;
    }
    else if ( pos.y() > height() )
    {
        vertScrollBar()->setValue ((int) (vertScrollBar()->value() +
                                   autoScrollAccelerationY (pos.y() - height())));
        select = true;
    }

    if ( pos.x() < 0 )
    {
        horzScrollBar()->setValue ((int) (horzScrollBar()->value() -
                                   autoScrollAccelerationX( - pos.x() )));
        select = true;
    }
    else if ( pos.x() > width() )
    {
        horzScrollBar()->setValue ((int) (horzScrollBar()->value() +
                                 autoScrollAccelerationX( pos.x() - width())));
        select = true;
    }

    if ( select )
    {
        QMouseEvent * event = new QMouseEvent(QEvent::MouseMove, pos,
                                              Qt::NoButton, Qt::NoButton, Qt::NoModifier);
        mouseMoveEvent( event );
        delete event;
    }

    //Restart timer
    d->scrollTimer->start( 50 );
}

double Canvas::autoScrollAccelerationX( int offset )
{
    switch( static_cast<int>( offset / 20 ) )
    {
        case 0: return 5.0;
        case 1: return 20.0;
        case 2: return d->view->doc()->unzoomItXOld( width() );
        case 3: return d->view->doc()->unzoomItXOld( width() );
        default: return d->view->doc()->unzoomItXOld( (int) (width() * 5.0) );
    }
}

double Canvas::autoScrollAccelerationY( int offset )
{
    switch( static_cast<int>( offset / 20 ) )
    {
        case 0: return 5.0;
        case 1: return 20.0;
        case 2: return d->view->doc()->unzoomItYOld( height() );
        case 3: return d->view->doc()->unzoomItYOld( height() );
        default: return d->view->doc()->unzoomItYOld( (int) (height() * 5.0) );
    }
}
#endif

KSpread::EmbeddedObject *Canvas::getObject( const QPoint &pos, Sheet *_sheet )
{
  QPoint const p ( (int) pos.x() ,
              (int) pos.y() );

  foreach ( EmbeddedObject* object, doc()->embeddedObjects() )
  {
    if ( object->sheet() == _sheet )
    {
        QRectF const bound = ( object )->geometry();
        QRect zoomedBound = doc()->zoomRectOld( QRectF(bound.left(), bound.top(),
                                bound.width(),
                                bound.height() ) );
        zoomedBound.translate( (int)( -xOffset() * doc()->zoomedResolutionX() ), (int)( -yOffset() * doc()->zoomedResolutionY() ) );
         if ( zoomedBound.contains( p ) )
              return object;
    }
  }
  return 0;
}

void Canvas::selectObject( EmbeddedObject *obj )
{
  if ( obj->sheet() != activeSheet() || obj->isSelected() )
    return;
  obj->setSelected( true );
  repaintObject( obj );

  d->mouseSelectedObject = true;
  emit objectSelectedChanged();
  deleteEditor( true );
}

void Canvas::deselectObject( EmbeddedObject *obj )
{
  if ( obj->sheet() != activeSheet() || !obj->isSelected() )
    return;
  obj->setSelected( false );
  repaintObject( obj );

  d->mouseSelectedObject = false;
  emit objectSelectedChanged();
}

void Canvas::selectAllObjects()
{
  foreach ( EmbeddedObject* object, doc()->embeddedObjects() )
  {
    if ( object->sheet() == activeSheet() )
      object->setSelected( true );
  }

   d->mouseSelectedObject = true;
//   emit objectSelectedChanged();
}

void Canvas::deselectAllObjects()
{
  if( activeSheet()->numberSelectedObjects() == 0 )
    return;

  //lowerObject();

  foreach ( EmbeddedObject* object, doc()->embeddedObjects() )
      deselectObject( object );

   d->mouseSelectedObject = false;
//   emit objectSelectedChanged();
}



void Canvas::setMouseSelectedObject(bool b)
{
  d->mouseSelectedObject = b;
  emit objectSelectedChanged();
}

bool Canvas::isObjectSelected()
{
  return d->mouseSelectedObject;
}


void Canvas::moveObjectsByMouse( QPointF &pos, bool keepXorYunchanged )
{
  QRectF rect( objectRect( false ) );
  QPointF move( 0, 0 );
  double diffx = pos.x() - d->m_origMousePos.x();
  double diffy = pos.y() - d->m_origMousePos.y();

  move = QPointF( diffx, diffy );
  d->m_origMousePos = pos;

    // unwind last snapping
  QRectF movedRect( rect );
  movedRect.translate( diffx, diffy );

    // don't move object off canvas
  QPointF diffDueToBorders(0,0);
//   KoRect pageRect( m_activePage->getPageRect() );
  if ( rect.left() + move.x() < 0/*pageRect.left()*/ )
    diffDueToBorders.setX( -rect.left() - move.x() );
//   else if ( rect.right() + move.x() > pageRect.right() )
//     diffDueToBorders.setX( pageRect.right() - (rect.right() + move.x()) );


  //kDebug(36005) << "rect.top() + move.y():" << rect.top() + move.y()<< endl;
  if ( rect.top() + move.y() < 0 )
    diffDueToBorders.setY( -rect.top() - move.y() );
//   else if ( rect.bottom() + move.y() > pageRect.bottom() )
//     diffDueToBorders.setY( pageRect.bottom() - (rect.bottom() + move.y()) );

//   m_moveSnapDiff += diffDueToBorders;
  move += diffDueToBorders;

//   movedRect.moveBy( m_moveSnapDiff.x(), m_moveSnapDiff.y() );
  if ( keepXorYunchanged )
  {
    QPointF diff( d->m_moveStartPosMouse - movedRect.topLeft() );
    if ( fabs( diff.x() ) > fabs( diff.y() ) )
    {
//       m_moveSnapDiff.setY( /*m_moveSnapDiff.y() + */m_moveStartPosMouse.y() - movedRect.y() );
      movedRect.moveTopLeft( QPointF( movedRect.x(), d->m_moveStartPosMouse.y() ) );
      move.setY( movedRect.y() - rect.y() );
    }
    else
    {
//       m_moveSnapDiff.setX( /*m_moveSnapDiff.x() + */m_moveStartPosMouse.x() - movedRect.x() );
      movedRect.moveTopLeft( QPointF( d->m_moveStartPosMouse.x(), movedRect.y() ) );
      move.setX( movedRect.x() - rect.x() );
    }
  }

  if ( move != QPointF( 0, 0 ) )
  {
        //kDebug(33001) << "moveObjectsByMouse move = " << move << endl;
    activeSheet()->moveObject( view(), move, false );
  }
}


void Canvas::resizeObject( ModifyType _modType, const QPointF & point, bool keepRatio )
{
    EmbeddedObject *obj = d->m_resizeObject;

    QRectF objRect = obj->geometry();
    /*objRect.moveBy( -xOffset(), -yOffset() );*/
    objRect.translate(-xOffset(), -yOffset() );
    QRect oldBoundingRect( doc()->zoomRectOld( objRect ) );

    bool left = false;
    bool right = false;
    bool top = false;
    bool bottom = false;
    if ( _modType == MT_RESIZE_UP || _modType == MT_RESIZE_LU || _modType == MT_RESIZE_RU )
    {
        top = true;
//         snapStatus |= KoGuides::SNAP_HORIZ;
    }
    if ( _modType == MT_RESIZE_DN || _modType == MT_RESIZE_LD || _modType == MT_RESIZE_RD )
    {
        bottom = true;
//         snapStatus |= KoGuides::SNAP_HORIZ;
    }
    if ( _modType == MT_RESIZE_LF || _modType == MT_RESIZE_LU || _modType == MT_RESIZE_LD )
    {
        left = true;
//         snapStatus |= KoGuides::SNAP_VERT;
    }
    if ( _modType == MT_RESIZE_RT || _modType == MT_RESIZE_RU || _modType == MT_RESIZE_RD )
    {
        right = true;
//         snapStatus |= KoGuides::SNAP_VERT;
    }

    double newLeft = objRect.left();
    double newRight = objRect.right();
    double newTop = objRect.top();
    double newBottom = objRect.bottom();
    if ( top )
    {
        if ( point.y() < objRect.bottom() - MIN_SIZE )
        {
            newTop = point.y();
        }
        else
        {
            newTop = objRect.bottom() - MIN_SIZE;
        }
    }
    if ( bottom )
    {
        if ( point.y() > objRect.top() + MIN_SIZE )
        {
            newBottom = point.y();
        }
        else
        {
            newBottom = objRect.top() + MIN_SIZE;
        }
    }
    if ( left )
    {
        if ( point.x() < objRect.right() - MIN_SIZE )
        {
            newLeft = point.x();
        }
        else
        {
            newLeft = objRect.right() - MIN_SIZE;
        }
    }
    if ( right )
    {
        if ( point.x() > objRect.left() + MIN_SIZE )
        {
            newRight = point.x();
        }
        else
        {
            newRight = objRect.left() + MIN_SIZE;
        }
    }

  double width = newRight - newLeft;
  double height = newBottom - newTop;

  if ( keepRatio && d->m_ratio != 0 )
  {
    if ( ( top || bottom ) && ( right || left ) )
    {
      if ( height * height * d->m_ratio > width * width / d->m_ratio )
      {
        width = height * d->m_ratio;
      }
      else
      {
        height = width / d->m_ratio;
      }
    }
    else if ( top || bottom )
    {
      width = height * d->m_ratio;
    }
    else
    {
      height = width / d->m_ratio;
    }

    if ( top )
    {
      newTop = objRect.bottom() - height;
    }
    else
    {
      newBottom = objRect.top() + height;
    }
    if ( left )
    {
      newLeft = objRect.right() - width;
    }
    else
    {
      newRight = objRect.right() + width;
    }
  }

  if ( newLeft != objRect.left() || newRight != objRect.right() || newTop != objRect.top() || newBottom != objRect.bottom() )
  {
        // resizeBy and moveBy have to been used to make it work with rotated objects
        obj->resizeBy( width - objRect.width(), height - objRect.height() );

        if ( objRect.left() != newLeft || objRect.top() != newTop )
        {
            obj->moveBy( QPointF( newLeft - objRect.left(), newTop - objRect.top() ) );
        }

//     if ( doc()->showGuideLines() && !m_disableSnapping )
//     {
//       QRectF rect( obj->getRealRect() );
//       QPointF sp( rect.topLeft() );
//       if ( right )
//       {
//         sp.setX( rect.right() );
//       }
//       if ( bottom )
//       {
//         sp.setY( rect.bottom() );
//       }
//       m_gl.repaintSnapping( sp, snapStatus );
//     }

    repaint( oldBoundingRect );
    repaintObject( obj );
    emit objectSizeChanged();
  }
}


void Canvas::finishResizeObject( const QString &/*name*/, bool /*layout*/ )
{
  if ( d->m_resizeObject )
  {
    QPointF move = QPointF( d->m_resizeObject->geometry().x() - d->m_rectBeforeResize.x(),
                            d->m_resizeObject->geometry().y() - d->m_rectBeforeResize.y() );
    QSizeF size = QSizeF( d->m_resizeObject->geometry().width() - d->m_rectBeforeResize.width(),
                          d->m_resizeObject->geometry().height() - d->m_rectBeforeResize.height() );

    if ( ( d->m_resizeObject->geometry() ) != d->m_rectBeforeResize )
    {
        ChangeObjectGeometryCommand *resizeCmd = new ChangeObjectGeometryCommand( d->m_resizeObject, move, size );
        // the command is not executed as the object is already resized.
        doc()->addCommand( resizeCmd );
    }

//     if ( layout )
//       doc()->layout( m_resizeObject );

    d->m_ratio = 0.0;
    d->m_isResizing = false;
    repaintObject( d->m_resizeObject );
    d->m_resizeObject = 0;
  }
}

void Canvas::raiseObject( EmbeddedObject *object )
{
    if ( doc()->embeddedObjects().count() <= 1 )
        return;

    if ( d->m_objectDisplayAbove == 0 )
    {
        if ( activeSheet()->numberSelectedObjects() == 1 )
        {
            d->m_objectDisplayAbove = object;
        }
    }
}

void Canvas::lowerObject()
{
    d->m_objectDisplayAbove = 0;
}

void Canvas::displayObjectList( QList<EmbeddedObject*> &list )
{
  list = doc()->embeddedObjects();

    if ( d->m_objectDisplayAbove )
    {
        // it can happen that the object is no longer there e.g. when
        // the insert of the object is undone
        int pos = doc()->embeddedObjects().indexOf( d->m_objectDisplayAbove );
        if ( pos != -1 && d->m_objectDisplayAbove->isSelected() )
        {
            list.removeAt( pos );
            list.append( d->m_objectDisplayAbove );
        }
        else
        {
            //tz not possible due to const. should const be removed?
            //m_objectDisplayAbove = 0;
        }
    }
}


QRectF Canvas::objectRect( bool all ) const
{
  return activeSheet()->getRealRect( all );
}

void Canvas::deleteEditor (bool saveChanges, bool array)
{
  if ( !d->cellEditor )
    return;

  //There may be highlighted areas on the sheet which will need to be erased
  activeSheet()->setRegionPaintDirty( *choice() );

  d->editWidget->setEditMode( false );

  QString t = d->cellEditor->text();
  // Delete the cell editor first and after that update the document.
  // That means we get a synchronous repaint after the cell editor
  // widget is gone. Otherwise we may get painting errors.
  delete d->cellEditor;
  d->cellEditor = 0;

  if ( saveChanges )
  {
      if ( (!t.isEmpty()) && (t.at(0)=='=') )
      {
          //a formula
          int openParenthese = t.count( '(' );
          int closeParenthese = t.count( ')' );
          int diff = QABS( openParenthese - closeParenthese );
          if ( openParenthese > closeParenthese )
          {
              for (int i=0; i < diff;i++)
              {
                  t=t+')';
              }
          }
      }
    d->view->setText (t, array);
  }
  else
  {
    d->view->updateEditWidget();
  }

  setFocus();
}


bool Canvas::createEditor( bool clear,  bool focus )
{
    register Sheet * const sheet = activeSheet();
    if (!sheet)
        return false;

    Cell* const cell = sheet->nonDefaultCell( marker().x(), marker().y() );

    if ( sheet->isProtected() && !cell->style().notProtected() )
        return false;

    // Set the starting sheet of the choice.
    choice()->setSheet( sheet );

    if ( !d->cellEditor )
    {
        d->editWidget->setEditMode( true );
        d->cellEditor = new KSpread::CellEditor( cell, this, doc()->captureAllArrowKeys() );

        double w = cell->dblWidth( markerColumn() );
        double h = cell->dblHeight( markerRow() );
        double min_w = cell->dblWidth( markerColumn() );
        double min_h = cell->dblHeight( markerRow() );

        double xpos = sheet->dblColumnPos( markerColumn() ) - xOffset();

        Sheet::LayoutDirection sheetDir = sheet->layoutDirection();
        bool rtlText = cell->strOutText().isRightToLeft();

        // if sheet and cell direction don't match, then the editor's location
        // needs to be shifted backwards so that it's right above the cell's text
        if ( w > 0 && ( ( sheetDir == Sheet::RightToLeft && !rtlText ) ||
             ( sheetDir == Sheet::LeftToRight && rtlText  ) ) )
            xpos -= w - min_w;

        // paint editor above correct cell if sheet direction is RTL
        if ( sheetDir == Sheet::RightToLeft )
        {
            double dwidth = d->view->doc()->unzoomItXOld( width() );
            double w2 = qMax( w, min_w );
            xpos = dwidth - w2 - xpos;
        }

        double ypos = sheet->dblRowPos( markerRow() ) - yOffset();
        QPalette editorPalette( d->cellEditor->palette() );

        QColor color = cell->style().fontColor();

        if ( !color.isValid() )
            color = palette().text().color();
        editorPalette.setColor( QPalette::Text, color );

        color = sheet->style(  marker().x(), marker().y() ).backgroundColor(); // FIXME effective!
        if ( !color.isValid() )
            color = editorPalette.base().color();
        editorPalette.setColor( QPalette::Background, color );

        d->cellEditor->setPalette( editorPalette );

        QFont tmpFont = cell->style().font();

        tmpFont.setPointSizeF( 0.01 * d->view->doc()->zoomInPercent() * tmpFont.pointSizeF() );
        d->cellEditor->setFont( tmpFont );

        QRectF rect( xpos, ypos, w, h ); //needed to circumvent rounding issue with height/width


        QRect zoomedRect=d->view->doc()->zoomRectOld( rect );
        /*zoomedRect.setLeft(zoomedRect.left()-2);
        zoomedRect.setRight(zoomedRect.right()+4);
        zoomedRect.setTop(zoomedRect.top()-1);
        zoomedRect.setBottom(zoomedRect.bottom()+2);*/

        d->cellEditor->setGeometry( zoomedRect );
        d->cellEditor->setMinimumSize( QSize( d->view->doc()->zoomItXOld( min_w ), d->view->doc()->zoomItYOld( min_h ) ) );
        d->cellEditor->show();

        // Laurent 2001-12-05
        // Don't add focus when we create a new editor and
        // we select text in edit widget otherwise we don't delete
        // selected text.
        if ( focus )
            d->cellEditor->setFocus();

        sheet->setRegionPaintDirty( *selection() );
        repaint();
    }

    if ( !clear && cell )
        d->cellEditor->setText( cell->text() );

    return true;
}

void Canvas::repaintObject( EmbeddedObject *obj )
{
	//Calculate where the object appears on the canvas widget and then repaint that part of the widget
	QRect canvasRelativeGeometry = doc()->zoomRectOld( obj->geometry() );
	canvasRelativeGeometry.translate( (int)( -xOffset()*doc()->zoomedResolutionX() ) ,
			   			(int)( -yOffset() * doc()->zoomedResolutionY()) );

    update( canvasRelativeGeometry );

 /* if ( !obj->isSelected() )
  {
    QRectF g = obj->geometry();
    g.moveBy( -xOffset(), -yOffset() );
    QRect geometry( doc()->zoomRectOld( g ) );

    update( geometry );
  }
  else
  {
    QPainter p(this);
    p.translate( -xOffset() * doc()->zoomedResolutionX() , -yOffset() * doc()->zoomedResolutionY() );
    obj->draw(&p); //this goes faster than calling repaint
    p.end();
  }*/
}

void Canvas::copyOasisObjects()
{
    // We'll create a store (ZIP format) in memory
    QBuffer buffer;
    QByteArray mimeType = "application/vnd.oasis.opendocument.spreadsheet";
    KoStore* store = KoStore::createStore( &buffer, KoStore::Write, mimeType );
    Q_ASSERT( store );
    Q_ASSERT( !store->bad() );
    KoOasisStore oasisStore( store );

    KoXmlWriter* manifestWriter = oasisStore.manifestWriter( mimeType );

    QString plainText;
    KoPicture picture;
    if ( !doc()->saveOasisHelper( store, manifestWriter, Doc::SaveSelected, &plainText, &picture )
         || !oasisStore.closeManifestWriter() )
    {
        delete store;
        return;
    }
    delete store;
    QMimeData* mimeData = new QMimeData();
    if ( !plainText.isEmpty() )
        mimeData->setText( plainText );
#warning TODO KDE4 portage: Drag'n'drop of KoPicture
#if 0
    if ( !picture.isNull() )
        multiDrag->setMimeData( picture.dragObject( 0 ) );
#endif
    kDebug(36003) << k_funcinfo << "setting zip data: " << buffer.buffer().size() << " bytes." << endl;
    mimeData->setData( mimeType, buffer.buffer() );

    //save the objects as pictures too so that other programs can access them
    foreach ( EmbeddedObject* object, doc()->embeddedObjects() )
    {
      QRectF kr = objectRect(false);
      QRect r( kr.toRect() );
      QPixmap pixmap( r.width(), r.height() );
      pixmap.fill( "white" );
      QPainter p(&pixmap);
      if ( object->isSelected() )
      {
          p.drawPixmap( object->geometry().toRect().left() - r.left(), object->geometry().toRect().top() - r.top(), object->toPixmap( 1.0 , 1.0 ) );
      }
      p.end();
      if (!pixmap.isNull())
      {
        mimeData->setImageData( pixmap.toImage() );
      }
    }

    QApplication::clipboard()->setMimeData( mimeData, QClipboard::Clipboard );
}

void Canvas::closeEditor()
{
  if ( d->chooseCell )
    return;

  if ( d->cellEditor )
  {
    deleteEditor( true ); // save changes
  }
}

void Canvas::updateEditor()
{
  if (!d->chooseCell)
    return;

  register Sheet * const sheet = activeSheet();
  if (!sheet)
    return;

  if (d->cellEditor)
  {
    if (choice()->sheet() != sheet)
    {
      d->cellEditor->hide();
    }
    else
    {
      d->cellEditor->show();
    }
    d->cellEditor->updateChoice();
  }
}

void Canvas::updatePosWidget()
{
  register Sheet * const sheet = activeSheet();
  if (!sheet)
    return;

    QString buffer;
    // No selection, or only one cell merged selected
    if ( selection()->isSingular() )
    {
        if (sheet->getLcMode())
        {
            buffer = 'L' + QString::number( markerRow() ) +
		'C' + QString::number( markerColumn() );
        }
        else
        {
            buffer = Cell::columnName( markerColumn() ) +
		QString::number( markerRow() );
        }
    }
    else
    {
        if (sheet->getLcMode())
        {
          buffer = QString::number( (selection()->lastRange().bottom()-selection()->lastRange().top()+1) )+"Lx";
          if ( util_isRowSelected( selection()->lastRange() ) )
            buffer+=QString::number((KS_colMax-selection()->lastRange().left()+1))+'C';
            else
              buffer+=QString::number((selection()->lastRange().right()-selection()->lastRange().left()+1))+'C';
        }
        else
        {
                //encodeColumnLabelText return @@@@ when column >KS_colMax
                //=> it's not a good display
                //=> for the moment I display pos of marker
          buffer=Cell::columnName( selection()->lastRange().left() ) +
                    QString::number(selection()->lastRange().top()) + ':' +
                    Cell::columnName( qMin( KS_colMax, selection()->lastRange().right() ) ) +
                    QString::number(selection()->lastRange().bottom());
                //buffer=sheet->columnLabel( m_iMarkerColumn );
                //buffer+=tmp.setNum(m_iMarkerRow);
        }
  }

    if (buffer != d->posWidget->lineEdit()->text())
      d->posWidget->lineEdit()->setText(buffer);
}

void Canvas::equalizeRow()
{
  register Sheet * const sheet = activeSheet();
  if (!sheet)
    return;

  QRect s( selection()->lastRange() );
  RowFormat* rowFormat = sheet->rowFormat(s.top());
  double size = rowFormat->dblHeight();
  if ( s.top() == s.bottom() )
      return;
  for ( int i = s.top() + 1; i <= s.bottom(); i++ )
  {
      size = qMax( sheet->rowFormat(i)->dblHeight(), size );
  }
  d->view->vBorderWidget()->equalizeRow(size);
}

void Canvas::equalizeColumn()
{
  register Sheet * const sheet = activeSheet();
  if (!sheet)
    return;

  QRect s( selection()->lastRange() );
  ColumnFormat* columnFormat = sheet->columnFormat(s.left());
  double size = columnFormat->dblWidth();
  if ( s.left() == s.right() )
      return;

  for(int i=s.left()+1;i<=s.right();i++)
  {
    size = qMax( sheet->columnFormat(i)->dblWidth(), size );
  }
  d->view->hBorderWidget()->equalizeColumn(size);
}

QRect Canvas::viewToCellCoordinates( const QRectF& viewRegion ) const
{
  register Sheet * const sheet = activeSheet();
  if (!sheet)
    return QRect();

  const QRectF unzoomedRect = doc()->viewToDocument( viewRegion ).translated( xOffset(), yOffset() );

  double tmp;
  const int left = sheet->leftColumn( unzoomedRect.left(), tmp );
  const int right = sheet->rightColumn( unzoomedRect.right() );
  const int top = sheet->topRow( unzoomedRect.top(), tmp );
  const int bottom = sheet->bottomRow( unzoomedRect.bottom() );

  return QRect( left, top, right - left + 1, bottom - top + 1 );
}

QRect Canvas::visibleCells() const
{
  return viewToCellCoordinates( rect() );
}

//---------------------------------------------
//
// Drawing Engine
//
//---------------------------------------------

void Canvas::clipoutChildren( QPainter& painter ) const
{
  QRegion rgn = painter.clipRegion();
  if ( rgn.isEmpty() )
    rgn = QRegion( QRect( 0, 0, width(), height() ) );

  const double horizontalOffset = -xOffset() * doc()->zoomedResolutionX();
  const double verticalOffset = -yOffset() * doc()->zoomedResolutionY();

  foreach ( EmbeddedObject* object, doc()->embeddedObjects() )
  {
    if ( ( object )->sheet() == activeSheet() )
    {
	QRect childGeometry = doc()->zoomRectOld( object->geometry());

	//The clipping region is given in device coordinates
	//so subtract the current offset (scroll position) of the canvas
	childGeometry.translate( (int)horizontalOffset , (int)verticalOffset );

	if (painter.window().intersects(childGeometry))
		rgn -= childGeometry;

      //painter.fillRect( doc()->zoomRectOld( object->geometry() ), QColor("red" ) );
    }
  }

  painter.setClipRegion( rgn );
}

QRect Canvas::painterWindowGeometry( const QPainter& painter ) const
{
  QRect zoomedWindowGeometry = painter.window();

  zoomedWindowGeometry.translate( (int)( xOffset() * doc()->zoomedResolutionX() ) , (int)( yOffset() * doc()->zoomedResolutionY() ) );

	return zoomedWindowGeometry;
}

void Canvas::paintChildren( QPainter& painter, QMatrix& /*matrix*/ )
{
  if ( doc()->embeddedObjects().isEmpty() )
    return;

  register Sheet * const sheet = activeSheet();
  if (!sheet)
    return;

  painter.save();
  painter.translate( -xOffset() * doc()->zoomedResolutionX() , -yOffset() * doc()->zoomedResolutionY() );

  const QRect zoomedWindowGeometry = painterWindowGeometry( painter );

  foreach ( EmbeddedObject* object, doc()->embeddedObjects() )
  {
    QRect const zoomedObjectGeometry = doc()->zoomRectOld( object->geometry() );
    if ( ( object )->sheet() == sheet &&
           zoomedWindowGeometry.intersects( zoomedObjectGeometry ) )
    {
	    //To prevent unnecessary redrawing of the embedded object, we only repaint
	    //if one or more of the cells underneath the object has been marked as 'dirty'.

	   QRect canvasRelativeGeometry = zoomedObjectGeometry;
	   canvasRelativeGeometry.translate( (int)( -xOffset()*doc()->zoomedResolutionX() ) ,
			   			(int)( -yOffset() * doc()->zoomedResolutionY()) );

	   const QRect cellsUnderObject = viewToCellCoordinates( canvasRelativeGeometry );
	   bool redraw=false;

      Region paintDirtyList = sheet->paintDirtyData();
      Region::ConstIterator end(paintDirtyList.constEnd());
      for (Region::ConstIterator it(paintDirtyList.constBegin()); it != end; ++it)
      {
        QRect range = (*it)->rect() & cellsUnderObject;
        int right = range.right();
        for (int x = range.left(); x <= right; ++x)
        {
          int bottom = range.bottom();
          for (int y = range.top(); y <= bottom; ++y)
          {
            redraw=true;
            break;
          }
        }
      }
      if ( redraw )
        object->draw( &painter );
    }
  }
  painter.restore();
}

void Canvas::paintHighlightedRanges(QPainter& painter, const QRectF& /*viewRect*/)
{
  const QList<QColor> colors = choice()->colors();
  int index = 0;
  Region::ConstIterator end(choice()->constEnd());
  for (Region::ConstIterator it = choice()->constBegin(); it != end; ++it)
  {
    //Only paint ranges or cells on the current sheet
    if ((*it)->sheet() != activeSheet())
    {
      index++;
      continue;
    }

    const QRect range = choice()->extendToMergedAreas( (*it)->rect() );

    QRectF unzoomedRect = cellCoordinatesToDocument( range ).translated( -xOffset(), -yOffset() );

    //Convert region from sheet coordinates to canvas coordinates for use with the painter
    //retrieveMarkerInfo(region,viewRect,positions,paintSides);

    painter.setPen( colors[(index) % colors.size()] );

    //Now adjust the highlight rectangle is slightly inside the cell borders (this means that multiple highlighted cells
    //look nicer together as the borders do not clash)
    const double unzoomedXPixel = d->view->doc()->unzoomItX( 1.0 );
    const double unzoomedYPixel = d->view->doc()->unzoomItY( 1.0 );

    unzoomedRect.adjust( unzoomedXPixel, unzoomedYPixel, -unzoomedXPixel, -unzoomedYPixel );

    painter.setBrush( QBrush() );
    painter.drawRect( unzoomedRect );

    //Now draw the size grip (the little rectangle on the bottom right-hand corner of the range which the user can
    //click and drag to resize the region)

    painter.setPen( Qt::white );
    painter.setBrush( colors[(index) % colors.size()] );

    painter.drawRect( QRectF( unzoomedRect.right() - 3 * unzoomedXPixel,
                              unzoomedRect.bottom() - 3 * unzoomedYPixel,
                              6 * unzoomedXPixel,
                              6 * unzoomedYPixel ) );
    index++;
  }
}

void Canvas::paintNormalMarker(QPainter& painter, const QRectF &viewRect)
{
    if( d->chooseCell )
        return;
    if (d->cellEditor)
        return;

    // disable antialiasing
    painter.setRenderHint( QPainter::Antialiasing, false );
    // save old clip region
    const QRegion oldClipRegion = painter.clipRegion();
    // Extend the clip rect by one in each direction to avoid artefacts caused by rounding errors.
    // TODO Stefan: This unites the region's rects. May be bad. Check!
    painter.setClipRegion( oldClipRegion.boundingRect().adjusted(-1,-1,1,1) );

    QLineF line;
    QPen pen( Qt::black, doc()->unzoomItX( 2 ) );
    painter.setPen( pen );

    const Selection* selection = this->selection();
    const QRect currentRange = Region::normalized( QRect( selection->anchor(), selection->marker() ) );
    const QRect effMarker = selection->extendToMergedAreas( QRect( selection->marker(), selection->marker() ) );
    const QRectF markerRegion = doc()->viewToDocument( cellCoordinatesToView( effMarker ) );
    Region::ConstIterator end(selection->constEnd());
    for (Region::ConstIterator it(selection->constBegin()); it != end; ++it)
    {
        const QRect range = (*it)->isAll() ? (*it)->rect() : selection->extendToMergedAreas( (*it)->rect() );

        // Only the active element (the one with the anchor) will be drawn with a border
        const bool current = (currentRange == range);

        double positions[4];
        bool paintSides[4];
        retrieveMarkerInfo( range, viewRect, positions, paintSides );

        double left =   positions[0];
        double top =    positions[1];
        double right =  positions[2];
        double bottom = positions[3];

        bool paintLeft =   paintSides[0];
        bool paintTop =    paintSides[1];
        bool paintRight =  paintSides[2];
        bool paintBottom = paintSides[3];

        // get the transparent selection color
        QColor selectionColor( QApplication::palette().highlight().color() );
        selectionColor.setAlpha( 127 );
        if ( current )
        {
            // save old clip region
            const QRegion clipRegion = painter.clipRegion();
            // clip out the marker region
            painter.setClipRegion( clipRegion.subtracted( markerRegion.toRect() ) );
            // draw the transparent selection background
            painter.fillRect( QRectF( left, top, right - left, bottom - top ), selectionColor );
            // restore clip region
            painter.setClipRegion( clipRegion );
        }
        else
            // draw the transparent selection background
            painter.fillRect( QRectF( left, top, right - left, bottom - top ), selectionColor );

        if ( paintTop )
        {
            line = QLineF( left, top, right, top );
            painter.drawLine( line );
        }
        if ( activeSheet()->layoutDirection()==Sheet::RightToLeft )
        {
            if ( paintRight )
            {
                line = QLineF( right, top, right, bottom );
                painter.drawLine( line );
            }
            if ( paintLeft && paintBottom && current )
            {
                /* then the 'handle' in the bottom left corner is visible. */
                line = QLineF( left, top, left, bottom - doc()->unzoomItY( 3 ) );
                painter.drawLine( line );
                line = QLineF( left + doc()->unzoomItX( 4 ),  bottom, right + doc()->unzoomItY( 1 ), bottom );
                painter.drawLine( line );
                painter.fillRect( QRectF( left - doc()->unzoomItX( 2 ), bottom - doc()->unzoomItY( 2 ),
                                  doc()->unzoomItX( 5 ), doc()->unzoomItY( 5 ) ), painter.pen().color() );
            }
            else
            {
                if ( paintLeft )
                {
                    line = QLineF( left, top, left, bottom );
                    painter.drawLine( line );
                }
                if ( paintBottom )
                {
                    line = QLineF( left, bottom, right, bottom );
                    painter.drawLine( line );
                }
            }
        }
        else // activeSheet()->layoutDirection()==Sheet::LeftToRight
        {
            if ( paintLeft )
            {
                line = QLineF( left, top, left, bottom );
                painter.drawLine( line );
            }
            if ( paintRight && paintBottom && current )
            {
                /* then the 'handle' in the bottom right corner is visible. */
                line = QLineF( right, top, right, bottom - doc()->unzoomItY( 3 ) );
                painter.drawLine( line );
                line = QLineF( left, bottom, right - doc()->unzoomItX( 3 ), bottom );
                painter.drawLine( line );
                painter.fillRect( QRectF( right - doc()->unzoomItX( 2 ), bottom - doc()->unzoomItY( 2 ),
                                  doc()->unzoomItX( 5 ), doc()->unzoomItY( 5 ) ), painter.pen().color() );
            }
            else
            {
                if ( paintRight )
                {
                    line = QLineF( right, top, right, bottom );
                    painter.drawLine( line );
                }
                if ( paintBottom )
                {
                    line = QLineF( left, bottom, right, bottom );
                    painter.drawLine( line );
                }
            }
        }
    }
    // restore clip region
    painter.setClipRegion( oldClipRegion );
    // restore antialiasing
    painter.setRenderHint( QPainter::Antialiasing, true );
}

QRectF Canvas::cellCoordinatesToDocument( const QRect& cellRange ) const
{
    register Sheet * const sheet = activeSheet();
    if (!sheet)
        return QRectF();

    QRectF rect;
    rect.setLeft  ( sheet->dblColumnPos( cellRange.left() ) );
    rect.setRight ( sheet->dblColumnPos( cellRange.right() + 1 ) );
    rect.setTop   ( sheet->dblRowPos( cellRange.top() ) );
    rect.setBottom( sheet->dblRowPos( cellRange.bottom() + 1 ) );
    return rect;
}

QRectF Canvas::cellCoordinatesToView( const QRect& cellRange ) const
{
    register Sheet * const sheet = activeSheet();
    if (!sheet)
        return QRectF();

    QRectF rect = cellCoordinatesToDocument( cellRange );
    // apply scrolling offset
    rect.translate( -xOffset(), -yOffset() );
    // convert it to view coordinates
    rect = doc()->documentToView( rect );
    // apply layout direction
    if ( sheet->layoutDirection() == Sheet::RightToLeft )
    {
        const double left = rect.left();
        const double right = rect.right();
        rect.setLeft( width() - right );
        rect.setRight( width() - left );
    }
    return rect;
}

void Canvas::retrieveMarkerInfo( const QRect &cellRange,
                                 const QRectF &viewRect,
                                 double positions[],
                                 bool paintSides[] )
{
    const Sheet* sheet = activeSheet();
    const QRectF visibleRect = cellCoordinatesToDocument( cellRange ).translated( -xOffset(), -yOffset() );

    /* these vars are used for clarity, the array for simpler function arguments  */
    double left = visibleRect.left();
    double top = visibleRect.top();
    double right = visibleRect.right();
    double bottom = visibleRect.bottom();
    if ( sheet->layoutDirection() == Sheet::RightToLeft )
    {
        const double docWidth = d->view->doc()->unzoomItX( width() );
        left = docWidth - visibleRect.right();
        right = docWidth - visibleRect.left();
    }

    /* left, top, right, bottom */
    paintSides[0] = (viewRect.left() <= left) && (left <= viewRect.right()) &&
                    (bottom >= viewRect.top()) && (top <= viewRect.bottom());
    paintSides[1] = (viewRect.top() <= top) && (top <= viewRect.bottom()) &&
                    (right >= viewRect.left()) && (left <= viewRect.right());
    paintSides[2] = (viewRect.left() <= right ) && (right <= viewRect.right()) &&
                    (bottom >= viewRect.top()) && (top <= viewRect.bottom());
    paintSides[3] = (viewRect.top() <= bottom) && (bottom <= viewRect.bottom()) &&
                    (right >= viewRect.left()) && (left <= viewRect.right());

    positions[0] = qMax( left,   viewRect.left() );
    positions[1] = qMax( top,    viewRect.top() );
    positions[2] = qMin( right,  viewRect.right() );
    positions[3] = qMin( bottom, viewRect.bottom() );
}

// find the label for the tip
// this is a hack of course, because it's not available from QToolTip
QLabel *tip_findLabel()
{
    QWidgetList widgets = QApplication::allWidgets();
    foreach ( QWidget* widget, widgets )
    {
      QLabel* label = qobject_cast<QLabel*>(widget);
      return label;
    }
    return 0;
}

void Canvas::showToolTip( const QPoint& p )
{
    register Sheet * const sheet = activeSheet();
    if (!sheet)
        return;

    // Over which cell is the mouse ?
    double ypos, xpos;
    double dwidth = doc()->unzoomItXOld( width() );
    int col;
    if ( sheet->layoutDirection()==Sheet::RightToLeft )
      col = sheet->leftColumn( (dwidth - doc()->unzoomItXOld( p.x() ) +
                                              xOffset()), xpos );
    else
      col = sheet->leftColumn( (doc()->unzoomItXOld( p.x() ) +
                                     xOffset()), xpos );


    int row = sheet->topRow( (doc()->unzoomItYOld( p.y() ) +
                                   yOffset()), ypos );

    const Cell* cell = sheet->visibleCellAt( col, row );
    if ( !cell )
        return;

#if 0
    // Quick cut
    if( cell->strOutText().isEmpty() )
        return;
#endif
    // displayed tool tip, which has the following priorities:
    //  - cell content if the cell dimension is too small
    //  - cell comment
    //  - hyperlink
    QString tipText;
    QString comment = sheet->comment( col, row );
    // If cell is too small, show the content
    if ( !view()->sheetView( sheet )->cellView( col, row ).dimensionFits() )
    {
        tipText = cell->strOutText();
    }

    // Show hyperlink, if any
    if ( tipText.isEmpty() )
    {
      tipText = cell->link();
    }

    // Nothing to display, bail out
    if ( tipText.isEmpty() && comment.isEmpty() )
      return;

    // Cut if the tip is ridiculously long
    const int maxLen = 256;
    if ( tipText.length() > maxLen )
        tipText = tipText.left(maxLen).append("...");

    // Determine position and width of the current cell.
    double u = cell->dblWidth( col );
    double v = cell->dblHeight( row );

    // Special treatment for obscured cells.
    if ( cell->isPartOfMerged() )
    {
      cell = cell->masterCell();
      const int moveX = cell->column();
      const int moveY = cell->row();

      // Use the obscuring cells dimensions
      u = cell->dblWidth( moveX );
      v = cell->dblHeight( moveY );
      xpos = sheet->dblColumnPos( moveX );
      ypos = sheet->dblRowPos( moveY );
    }

    // Get the cell dimensions
    QRectF marker;
    bool insideMarker = false;
    if ( sheet->layoutDirection()==Sheet::RightToLeft )
    {
      QRectF unzoomedMarker( dwidth - u - xpos + xOffset(),
                             ypos - yOffset(),
                             u,
                             v );
      marker = doc()->documentToView( unzoomedMarker );
      insideMarker = marker.contains( p );
    }
    else
    {
      QRectF unzoomedMarker( xpos - xOffset(),
                             ypos - yOffset(),
                             u,
                             v );
      marker = doc()->documentToView( unzoomedMarker );
      insideMarker = marker.contains( p );
    }

    // No use if mouse is somewhere else
    if ( !insideMarker )
        return;

    // Find the tipLabel
    // NOTE: if we failed, check again when the tip is shown already
    QLabel* tipLabel = tip_findLabel();

    // Ensure that it is plain text
    // Not funny if (intentional or not) <a> appears as hyperlink
    if ( tipLabel )
         tipLabel->setTextFormat( Qt::PlainText );

    QFontMetrics fm = tipLabel? tipLabel->fontMetrics() : fontMetrics();
    const QRect r( 0, 0, 200, -1 );
    // Wrap the text if too long
    if ( tipText.length() > 16 )
    {
        KWordWrap* wrap = KWordWrap::formatText( fm, r, 0, tipText );
        tipText = wrap->wrappedString();
        delete wrap;
    }
    // Wrap the comment if too long
    if ( comment.length() > 16 )
    {
      KWordWrap* wrap = KWordWrap::formatText( fm, r, 0, comment );
      comment = wrap->wrappedString();
      delete wrap;
    }

    // Show comment, if any
    if ( tipText.isEmpty() )
    {
      tipText = comment;
    }
    else if ( !comment.isEmpty() )
    {
      //Add 2 extra lines and a text, when both should be in the tooltip
      if ( !comment.isEmpty() )
        comment = "\n\n" + i18n("Comment:") + '\n' + comment;

      tipText += comment;
    }

    // Now we shows the tip
    QToolTip::showText( mapToGlobal( marker.toRect().bottomRight() ), tipText, this );

    // Here we try to find the tip label again
    // Reason: the previous tip_findLabel might fail if no tip has ever shown yet
    if ( !tipLabel )
    {
      tipLabel = tip_findLabel();
      if ( tipLabel )
        tipLabel->setTextFormat( Qt::PlainText );
    }

}


#include "Canvas.moc"
