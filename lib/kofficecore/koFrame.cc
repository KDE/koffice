
#include "koFrame.h"
#include "koView.h"

#include <qpainter.h>
#include <kparts/event.h>
#include <kdebug.h>

class KoFramePrivate
{
public:
  KoFramePrivate()
  {
  }
  ~KoFramePrivate()
  {
  }

  KoView *m_view;

  QPoint m_mousePressPos;
  QPoint m_framePos;
  int m_width;
  int m_height;
  int m_mode;

  KoFrame::State m_state;
};

KoFrame::KoFrame( QWidget *parent, const char *name )
 : QWidget( parent, name )
{
  d = new KoFramePrivate;
  d->m_state = Inactive;
  d->m_mode = -1;
  d->m_view = 0;

  setBackgroundColor( white );
  setMouseTracking( true );
}

KoFrame::~KoFrame()
{
  delete d;
}

void KoFrame::setView( KoView *view )
{
  if ( view == d->m_view )
    return;

  if ( d->m_view )
    d->m_view->removeEventFilter( this );

  d->m_view = view;
  if ( d->m_view )
  {
    d->m_view->installEventFilter( this );

    setMaximumSize( QMAX( d->m_view->maximumWidth() + 2 * border(), d->m_view->maximumWidth() ),
		    QMAX( d->m_view->maximumHeight() + 2 * border(), d->m_view->maximumHeight() ) );
    setMinimumSize( d->m_view->minimumWidth() + leftBorder() + rightBorder(),
		    d->m_view->minimumHeight() + topBorder() + bottomBorder() );

  }

  resizeEvent( 0L );
}

KoView *KoFrame::view() const
{
  return d->m_view;
}

void KoFrame::setState( State s )
{
  if ( d->m_state == s )
    return;

  State old = d->m_state;
  d->m_state = s;

  if ( d->m_view )
  {
      setMaximumSize( QMAX( d->m_view->maximumWidth() + 2 * border(), d->m_view->maximumWidth() ),
		      QMAX( d->m_view->maximumHeight() + 2 * border(), d->m_view->maximumHeight() ) );
      setMinimumSize( d->m_view->minimumWidth() + leftBorder() + rightBorder(),
		      d->m_view->minimumHeight() + topBorder() + bottomBorder() );
  }

  if ( d->m_state == Inactive )
  {
    d->m_state = old;
    int l = leftBorder();
    int r = rightBorder();
    int t = topBorder();
    int b = bottomBorder();
    d->m_state = Inactive;
    setGeometry( x() + l, y() + t, width() - l - r, height() - t - b );
  }
  else if ( ( d->m_state == Active || d->m_state == Selected ) && old == Inactive )	
    setGeometry( x() - leftBorder(), y() - topBorder(),
		 width() + leftBorder() + rightBorder(),
		 height() + topBorder() + bottomBorder()  );
  else if ( d->m_state == Active  && old == Selected )
  {
    setGeometry( x() - leftBorder() + border(), y() - topBorder() + border(),
		 width() + leftBorder() + rightBorder() - 2 * border(),
		 height() + topBorder() + bottomBorder() - 2 * border() );
  }

  update();
}

KoFrame::State KoFrame::state() const
{
  return d->m_state;
}

int KoFrame::leftBorder() const
{
  if ( d->m_state == Inactive )
    return 0;
  if ( d->m_state == Selected || !d->m_view )
    return border();

  return d->m_view->leftBorder() + border();
}

int KoFrame::rightBorder() const
{
  if ( d->m_state == Inactive )
    return 0;
  if ( d->m_state == Selected || !d->m_view )
    return border();

  return d->m_view->rightBorder() + border();
}

int KoFrame::topBorder() const
{
  if ( d->m_state == Inactive )
    return 0;
  if ( d->m_state == Selected || !d->m_view )
    return border();

  return d->m_view->topBorder() + border();
}

int KoFrame::bottomBorder() const
{
  if ( d->m_state == Inactive )
    return 0;
  if ( d->m_state == Selected || !d->m_view )
    return border();

  return d->m_view->bottomBorder() + border();
}

int KoFrame::border() const
{
  if ( d->m_state == Inactive )
    return 0;
  return 5;
}

void KoFrame::paintEvent( QPaintEvent* )
{
  QPainter painter;
  painter.begin( this );

  painter.setPen( black );
  painter.fillRect( 0, 0, width(), height(), BDiagPattern );

  if ( d->m_state == Selected )
  {
    painter.fillRect( 0, 0, 5, 5, black );
    painter.fillRect( 0, height() - 5, 5, 5, black );
    painter.fillRect( width() - 5, height() - 5, 5, 5, black );
    painter.fillRect( width() - 5, 0, 5, 5, black );
    painter.fillRect( width() / 2 - 3, 0, 5, 5, black );
    painter.fillRect( width() / 2 - 3, height() - 5, 5, 5, black );
    painter.fillRect( 0, height() / 2 - 3, 5, 5, black );
    painter.fillRect( width() - 5, height() / 2 - 3, 5, 5, black );
  }

  painter.end();
}

void KoFrame::mousePressEvent( QMouseEvent* ev )
{
  QRect r1( 0, 0, 5, 5 );
  QRect r2( 0, height() - 5, 5, 5 );
  QRect r3( width() - 5, height() - 5, 5, 5 );
  QRect r4( width() - 5, 0, 5, 5 );
  QRect r5( width() / 2 - 3, 0, 5, 5 );
  QRect r6( width() / 2 - 3, height() - 5, 5, 5 );
  QRect r7( 0, height() / 2 - 3, 5, 5 );
  QRect r8( width()- 5, height() / 2 - 3, 5, 5 );

  if ( r1.contains( ev->pos() ) )
    d->m_mode = 1;
  else if ( r2.contains( ev->pos() ) )
    d->m_mode = 2;
  else if ( r3.contains( ev->pos() ) )
    d->m_mode = 3;
  else if ( r4.contains( ev->pos() ) )
    d->m_mode = 4;
  else if ( r5.contains( ev->pos() ) )
    d->m_mode = 5;
  else if ( r6.contains( ev->pos() ) )
    d->m_mode = 6;
  else if ( r7.contains( ev->pos() ) )
    d->m_mode = 7;
  else if ( r8.contains( ev->pos() ) )
    d->m_mode = 8;
  else
    d->m_mode = 0;

  if ( d->m_state == Active )
    d->m_mode = 0;

  kdDebug(30003) << "---- MODE=" << d->m_mode << endl;

  d->m_mousePressPos = mapToParent( ev->pos() );
  d->m_framePos = mapToParent( QPoint( 0, 0 ) );
  d->m_width = width();
  d->m_height = height();
}

void KoFrame::mouseMoveEvent( QMouseEvent* ev )
{
  if ( d->m_mode == 0 )
  {
      QPoint p = parentWidget()->mapFromGlobal( ev->globalPos() );
      move( QPoint( p.x() + d->m_framePos.x() - d->m_mousePressPos.x(),
		    p.y() + d->m_framePos.y() - d->m_mousePressPos.y() ) );
  }
  else if ( d->m_mode == 1 )
  {
      QPoint p = parentWidget()->mapFromGlobal( ev->globalPos() );
      int w = QMIN( QMAX( d->m_width + d->m_mousePressPos.x() - p.x(), minimumWidth() ), maximumWidth() );
      int h = QMIN( QMAX( d->m_height + d->m_mousePressPos.y() - p.y(), minimumHeight() ), maximumHeight() );
      setGeometry( d->m_framePos.x() - w + d->m_width,
		   d->m_framePos.y() - h + d->m_height, w, h );	
  }
  else if ( d->m_mode == 2 )
  {
    QPoint p = parentWidget()->mapFromGlobal( ev->globalPos() );
    int w = QMIN( QMAX( d->m_width + d->m_mousePressPos.x() - p.x(), minimumWidth() ), maximumWidth() );
    int h = QMIN( QMAX( d->m_height - d->m_mousePressPos.y() + p.y(), minimumHeight() ), maximumHeight() );
    setGeometry( d->m_framePos.x() - w + d->m_width,
		 d->m_framePos.y(), w, h );
  }
  else if ( d->m_mode == 3 )
  {
    QPoint p = parentWidget()->mapFromGlobal( ev->globalPos() );
    int w = QMIN( QMAX( d->m_width - d->m_mousePressPos.x() + p.x(), minimumWidth() ), maximumWidth() );
    int h = QMIN( QMAX( d->m_height - d->m_mousePressPos.y() + p.y(), minimumHeight() ), maximumHeight() );
    resize( w, h );
  }
  else if ( d->m_mode == 4 )
  {
    QPoint p = parentWidget()->mapFromGlobal( ev->globalPos() );
    int w = QMIN( QMAX( d->m_width - d->m_mousePressPos.x() + p.x(), minimumWidth() ), maximumWidth() );
    int h = QMIN( QMAX( d->m_height + d->m_mousePressPos.y() - p.y(), minimumHeight() ), maximumHeight() );
    setGeometry( d->m_framePos.x(),
		 d->m_framePos.y() - h + d->m_height, w, h );
  }
  else if ( d->m_mode == 5 )
  {
    QPoint p = parentWidget()->mapFromGlobal( ev->globalPos() );
    int h = QMIN( QMAX( d->m_height + d->m_mousePressPos.y() - p.y(), minimumHeight() ), maximumHeight() );
    setGeometry( d->m_framePos.x(),
		 d->m_framePos.y() - h + d->m_height, d->m_width, h );
  }
  else if ( d->m_mode == 6 )
  {
    QPoint p = parentWidget()->mapFromGlobal( ev->globalPos() );
    int h = QMIN( QMAX( d->m_height - d->m_mousePressPos.y() + p.y(), minimumHeight() ), maximumHeight() );
    resize( d->m_width, h );
  }
  else if ( d->m_mode == 7 )
  {
    QPoint p = parentWidget()->mapFromGlobal( ev->globalPos() );
    int w = QMIN( QMAX( d->m_width + d->m_mousePressPos.x() - p.x(), minimumWidth() ), maximumWidth() );
    setGeometry( d->m_framePos.x() - w + d->m_width,
		 d->m_framePos.y(), w, d->m_height );
  }
  else if ( d->m_mode == 8 )
  {
    QPoint p = parentWidget()->mapFromGlobal( ev->globalPos() );
    int w = QMIN( QMAX( d->m_width - d->m_mousePressPos.x() + p.x(), minimumWidth() ), maximumWidth() );
    resize( w, d->m_height );
  }
  else if ( d->m_state == Selected )
  {
    QRect r1( 0, 0, 5, 5 );
    QRect r2( 0, height() - 5, 5, 5 );
    QRect r3( width() - 5, height() - 5, 5, 5 );
    QRect r4( width() - 5, 0, 5, 5 );
    QRect r5( width() / 2 - 3, 0, 5, 5 );
    QRect r6( width() / 2 - 3, height() - 5, 5, 5 );
    QRect r7( 0, height() / 2 - 3, 5, 5 );
    QRect r8( width()- 5, height() / 2 - 3, 5, 5 );
	
    if ( r1.contains( ev->pos() ) || r3.contains( ev->pos() ) )
      setCursor( sizeFDiagCursor );
    else if ( r2.contains( ev->pos() ) || r4.contains( ev->pos() ) )
      setCursor( sizeBDiagCursor );	
    else if ( r5.contains( ev->pos() ) || r6.contains( ev->pos() ) )
      setCursor( sizeVerCursor );
    else if ( r7.contains( ev->pos() ) || r8.contains( ev->pos() ) )
      setCursor( sizeHorCursor );
    else
      setCursor( pointingHandCursor );
  }
  else
    setCursor( pointingHandCursor );
}

void KoFrame::mouseReleaseEvent( QMouseEvent* )
{
    d->m_mode = -1;
}

void KoFrame::resizeEvent( QResizeEvent* )
{
  if ( !d->m_view )
    return;

  if ( d->m_state == Active || d->m_state == Selected )
    d->m_view->setGeometry( 5, 5, width() - 10, height() - 10 );
  else
    d->m_view->setGeometry( 0, 0, width(), height() );
}

bool KoFrame::eventFilter( QObject* obj, QEvent* ev )
{
  if ( obj == d->m_view && KParts::PartActivateEvent::test( ev ) )
  {
    kdDebug(30003) << "Activate event"<< endl;
    KParts::PartActivateEvent* e = (KParts::PartActivateEvent*)ev;
    if ( e->part() == (KParts::Part *)d->m_view->koDocument() )
    {
      if ( e->activated() )
        setState( Active );
      else
	setState( Inactive );
    }
  }
  else if ( obj == d->m_view && KParts::PartSelectEvent::test( ev ) )
  {
    kdDebug(30003) << "Selected event" << endl;
    KParts::PartSelectEvent* e = (KParts::PartSelectEvent*)ev;
    if ( e->part() == (KParts::Part *)d->m_view->koDocument() )
    {
      if ( e->selected() )
        setState( Selected );
      else
        setState( Inactive );
    }
  }

  return FALSE;
}
