/*
 * Kivio - Visual Modelling and Flowcharting
 * Copyright (C) 2000 theKompany.com
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */
#include "tkfloatspinbox.h"
#include "tktoolbarbutton.h"
#include "tkunits.h"

#include "qglobal.h"
#include <limits.h>

#include <qpushbutton.h>
#include <qpainter.h>
#include <qbitmap.h>
#include <qlineedit.h>
#include <qvalidator.h>
#include <qpixmapcache.h>
#include <qapplication.h>
#include <kdebug.h>

TKFloatSpinBox::TKFloatSpinBox( QWidget * parent , const char *name )
: QFrame( parent, name ), TKFloatRangeControl()
{
  flength = 0;
  m_decimal = 3;
  initSpinBox();
}

TKFloatSpinBox::TKFloatSpinBox( float minValue, float maxValue, float step, int decimal, QWidget* parent, const char* name )
: QFrame( parent, name ), TKFloatRangeControl( minValue, maxValue, step, step, minValue )
{
  m_decimal = decimal;
  initSpinBox();
}

void TKFloatSpinBox::initSpinBox()
{
  buttonSymbol = TKFloatSpinBox::UpDownArrows;
  wrap = false;
  edited = false;
  pfix = QString::null;
  sfix = QString::null;

  m_minimum = false;

  up = new TKToolBarButton(QPixmap(),QString::null,this,"up");
  up->setAutoRaised(false);
  up->setRaised(true);
  up->setFocusPolicy( QWidget::NoFocus );
  up->setAutoRepeat( true );

  down = new TKToolBarButton(QPixmap(),QString::null,this,"down");
  down->setAutoRaised(false);
  down->setRaised(true);
  down->setFocusPolicy( QWidget::NoFocus );
  down->setAutoRepeat( true );

  setButtonSymbols(UpDownArrows);

  validate = new QDoubleValidator( minValue(), maxValue(), m_decimal, this );
  vi = new QLineEdit(this);
  vi->setFrame(false);
  vi->setAlignment(AlignRight);
  setFocusProxy( vi );
  setFocusPolicy( StrongFocus );
  vi->setValidator( validate );
  vi->installEventFilter( this );
  
  setSizePolicy( QSizePolicy( QSizePolicy::Minimum, QSizePolicy::Fixed ) );
  setFrameStyle( Panel|Sunken );
  updateDisplay();

  connect( up, SIGNAL(pressed()), SLOT(stepUp()) );
  connect( down, SIGNAL(pressed()), SLOT(stepDown()) );
  connect( vi, SIGNAL(textChanged(const QString&)), SLOT(textChanged()) );
}

TKFloatSpinBox::~TKFloatSpinBox()
{
}

void TKFloatSpinBox::setDecimals( int decimal )
{
  m_decimal = decimal;

  if (validate->inherits("QDoubleValidator"))
    ((QDoubleValidator*)validate)->setDecimals( m_decimal );

  updateDisplay();
}

QString TKFloatSpinBox::text() const
{
  return vi->text();
}

QString TKFloatSpinBox::cleanText() const
{
  QString s = QString(text()).stripWhiteSpace();
  if ( !prefix().isEmpty() ) {
    QString px = QString(prefix()).stripWhiteSpace();
    int len = px.length();
    if ( len && s.left(len) == px )
      s.remove( 0, len );
  }
  if ( !suffix().isEmpty() ) {
    QString sx = QString(suffix()).stripWhiteSpace();
    int len = sx.length();
    if ( len && s.right(len) == sx )
      s.truncate( s.length() - len );
  }
  return s.stripWhiteSpace();
}

void TKFloatSpinBox::setPrefix( const QString &text )
{
  pfix = text;
  updateDisplay();
}

void TKFloatSpinBox::setSuffix( const QString &text )
{
  sfix = text;
  updateDisplay();
}

QString TKFloatSpinBox::prefix() const
{
  if ( pfix.isEmpty() )
    return QString::null;

  return pfix;
}

QString TKFloatSpinBox::suffix() const
{
  if ( sfix.isEmpty() )
    return QString::null;

  return sfix;
}

void TKFloatSpinBox::setWrapping( bool on )
{
  wrap = on;
  updateDisplay();
}

bool TKFloatSpinBox::wrapping() const
{
  return wrap;
}

void TKFloatSpinBox::setFixedLength(int l)
{
  flength = l;
  updateGeometry();
}

QSize TKFloatSpinBox::sizeHint() const
{
  constPolish();
  QFontMetrics fm = fontMetrics();
  int fw = frameWidth();

  int h = m_minimum ? 9 : QMAX(12,fm.height()); // ensure enough space for the button pixmaps
	
	// button width - see resizeevent()
	int bw;
	if (m_minimum)
	  bw = 2*14;
	else
  	bw = (height()/2 - fw)*8/5;

  int w = 35; 	// minimum width for the value
  QString s;
  if (flength != 0) {
    int wl = fm.width('0')*QABS(flength+1);
    if (flength < 0)
      s = prefix() + suffix();
    w = QMAX( w, fm.width(s) + wl);
  } else {
    s = prefix() + ( (TKFloatSpinBox*)this )->mapValueToText( minValue() ) + suffix();
    w = QMAX(w, fm.width(s));
    s = prefix() + ( (TKFloatSpinBox*)this )->mapValueToText( maxValue() ) + suffix();
    w = QMAX(w, fm.width(s));
  }

  QSize r( bw + fw * 2 // buttons AND frame both sides - see resizeevent()
  + 6 // right/left margins
  + w, // widest value
  fw * 2 // top/bottom frame
  + (m_minimum ? 0:4) // top/bottom margins
  + h // font height
  );
  return r.expandedTo( QApplication::globalStrut() );
}

void TKFloatSpinBox::setMinimumStyle(bool minimum)
{
  if (m_minimum == minimum)
    return;

  m_minimum = minimum;
  setFrameStyle( m_minimum ? NoFrame:Panel|Sunken );
  setSizePolicy( QSizePolicy( QSizePolicy::Minimum, m_minimum ? QSizePolicy::Minimum : QSizePolicy::Fixed ) );

  arrangeWidgets();
}

void TKFloatSpinBox::arrangeWidgets()
{
  if ( !up || !down )
  return;

  int fw = frameWidth();
  QSize bs;
	if (m_minimum) {
	  bs.setHeight( height() - 2*fw );
    bs.setWidth(14);
	} else {
  	bs.setHeight( height()/2 - fw );
    bs.setWidth( bs.height() * 8 / 5 ); // 1.6 - approximate golden mean
  }

  if ( up->size() != bs || down->size() != bs ) {
    up->resize( bs );
    down->resize( bs );
    updateButtonSymbols();
  }

  if (m_minimum) {
    int y = fw;
    int x = width() - fw - 2*bs.width();
    up->move( x, y );
    down->move( x+bs.width(), y );
  	vi->setGeometry( fw, fw, x - fw, height() - 2*fw );
  } else {
    int y = fw;
    int x = width() - fw - bs.width();
    up->move( x, y );
    down->move( x, height() - y - up->height() );
  	vi->setGeometry( fw, fw, x - fw, height() - 2*fw );
	}
}

void TKFloatSpinBox::setValue( float value )
{
  directSetValue( value );
  updateDisplay();
}

void TKFloatSpinBox::stepUp()
{
  if ( edited )
    interpretText();
  if ( wrapping() && ( value()+lineStep() > maxValue() ) )
    TKFloatRangeControl::setValue( minValue() );
  else
    addLine();
}

void TKFloatSpinBox::stepDown()
{
  if ( edited )
    interpretText();
  if ( wrapping() && ( value()-lineStep() < minValue() ) )
    TKFloatRangeControl::setValue( maxValue() );
  else
    subtractLine();
}

bool TKFloatSpinBox::eventFilter( QObject* obj, QEvent* ev )
{
  if ( obj != vi )
    return false;

  if ( ev->type() == QEvent::FocusOut || ev->type() == QEvent::Leave || ev->type() == QEvent::Hide ) {
    if ( edited )
      interpretText();
  } else if ( ev->type() == QEvent::KeyPress ) {
    QKeyEvent* k = (QKeyEvent*)ev;
    if ( k->key() == Key_Up ) {
      stepUp();
      return true;
    } else if ( k->key() == Key_Down ) {
      stepDown();
      return true;
    } else if ( k->key() == Key_Return ) {
      interpretText();
      return false;
    }
  }
  return false;
}

void TKFloatSpinBox::leaveEvent( QEvent* )
{
  if (edited)
    interpretText();
}

void TKFloatSpinBox::resizeEvent( QResizeEvent* )
{
  arrangeWidgets();
}

void TKFloatSpinBox::wheelEvent( QWheelEvent* e )
{
   kdDebug() << "WHEEL EVENT **************************************\n";
  e->accept();
  static float offset = 0;
  static TKFloatSpinBox* offset_owner = 0;
  if (offset_owner != this) {
    offset_owner = this;
    offset = 0;
  }
  offset += -e->delta()/120;

  if (QABS(offset) < 1)
    return;

  float ioff = offset;
  for (int i=0; i<QABS(ioff); i++)
    offset > 0 ? stepDown() : stepUp();

  offset -= ioff;
}

void TKFloatSpinBox::valueChange()
{
  updateDisplay();
  emit valueChanged( value() );
}

void TKFloatSpinBox::rangeChange()
{
  if ( validate->inherits( "QDoubleValidator" ) )
    ((QDoubleValidator*)validate)->setRange( minValue(), maxValue() );
  updateDisplay();
}

void TKFloatSpinBox::setValidator( const QValidator* v )
{
  if ( vi )
    vi->setValidator( v );
}

const QValidator * TKFloatSpinBox::validator() const
{
  return vi ? vi->validator() : 0;
}

void TKFloatSpinBox::updateDisplay()
{
  vi->setText( currentValueText() );
  edited = false;
  up->setEnabled( isEnabled() && (wrapping() || value() < maxValue()) );
  down->setEnabled( isEnabled() && (wrapping() || value() > minValue()) );
}

void TKFloatSpinBox::interpretText()
{
  bool ok = true;
  float newVal = mapTextToValue( &ok );
  if ( ok )
    TKFloatRangeControl::setValue( newVal );

  updateDisplay();
}

TKToolBarButton* TKFloatSpinBox::upButton() const
{
  return up;
}

TKToolBarButton* TKFloatSpinBox::downButton() const
{
  return down;
}

QLineEdit* TKFloatSpinBox::editor() const
{
  return vi;
}

void TKFloatSpinBox::textChanged()
{
  edited = true;
}

QString TKFloatSpinBox::mapValueToText( float v )
{
  QString s = QString::number(v,'f',m_decimal);
  return s;
}

float TKFloatSpinBox::mapTextToValue( bool* ok )
{
  QString s = text();
  float newVal = s.toDouble( ok );
  if ( !(*ok) && !( !prefix() && !suffix() ) ) {
    s = cleanText();
    newVal = s.toDouble( ok );
  }
  return newVal;
}

QString TKFloatSpinBox::currentValueText()
{
  QString s;
  s = prefix();
  s.append( mapValueToText( value() ) );
  s.append( suffix() );
  return s;
}

void TKFloatSpinBox::setEnabled( bool on )
{
  bool b = isEnabled();
  QFrame::setEnabled( on );
  if ( isEnabled() != b ) {
    updateDisplay();
  }
}

void TKFloatSpinBox::styleChange( QStyle& old )
{
	setFrameStyle( Panel | Sunken );
  arrangeWidgets();
  QWidget::styleChange( old );
}

void TKFloatSpinBox::setButtonSymbols( ButtonSymbols newSymbols )
{
  if ( buttonSymbols() == newSymbols )
    return;

  buttonSymbol = newSymbols;
  updateButtonSymbols();
}

TKFloatSpinBox::ButtonSymbols TKFloatSpinBox::buttonSymbols() const
{
  return buttonSymbol;
}

void TKFloatSpinBox::updateButtonSymbols()
{
  QString key( QString::fromLatin1("$qt$qspinbox$"));

  bool pmSym = buttonSymbols() == PlusMinus;

  key += QString::fromLatin1( pmSym ? "+-" : "^v" );
  key += QString::number( down->height() );

  QString upKey = key + QString::fromLatin1( "$up" );
  QString dnKey = key + QString::fromLatin1( "$down" );

  QBitmap upBm;
  QBitmap dnBm;

  bool found = QPixmapCache::find( dnKey, dnBm ) && QPixmapCache::find( upKey, upBm );

  if ( !found ) {
    QPainter p;
    if ( pmSym ) {
      int h = down->height()-4;
      if ( h < 3 )
        return;
      else if ( h == 4 )
        h = 3;
      else if ( (h > 6) && (h & 1) )
        h--;

      h -= ( h / 8 ) * 2;     // Empty border
      dnBm.resize( h, h );
      p.begin( &dnBm );
      p.eraseRect( 0, 0, h, h );
      p.setBrush( color1 );
      int c = h/2;
      p.drawLine( 0, c, h, c );
      if ( !(h & 1) )
        p.drawLine( 0, c-1, h, c-1 );
      p.end();

      upBm = dnBm;
      p.begin( &upBm );
      p.drawLine( c, 0, c, h );
      if ( !(h & 1) )
        p.drawLine( c-1, 0, c-1, h );
      p.end();
    } else {
      int w = down->width()-4;
      if ( w < 3 )
        return;
      else if ( !(w & 1) )
        w--;

      w -= ( w / 7 ) * 2;     // Empty border
      int h = w/2 + 2;        // Must have empty row at foot of arrow
      dnBm.resize( w, h );
      p.begin( &dnBm );
      p.eraseRect( 0, 0, w, h );
      QPointArray a;
      a.setPoints( 3,  0, 1,  w-1, 1,  h-2, h-1 );
      p.setBrush( color1 );
      p.drawPolygon( a );
      p.end();
      QWMatrix wm;
      wm.scale( 1, -1 );
      upBm = dnBm.xForm( wm );
    }

    QPixmapCache::insert( dnKey, dnBm );
    QPixmapCache::insert( upKey, upBm );
  }

  down->setPixmap( dnBm );
  up->setPixmap( upBm );
}

float TKFloatSpinBox::minValue() const
{
  return TKFloatRangeControl::minValue();
}

float TKFloatSpinBox::maxValue() const
{
  return TKFloatRangeControl::maxValue();
}

void TKFloatSpinBox::setMinValue( float i )
{
  setRange( i, maxValue() );
}

void TKFloatSpinBox::setMaxValue( float i )
{
  setRange( minValue(), i );
}

float TKFloatSpinBox::lineStep() const
{
  return TKFloatRangeControl::lineStep();
}

void TKFloatSpinBox::setLineStep( float i )
{
  setSteps( i, pageStep() );
}

float TKFloatSpinBox::value()
{
  if (edited) {
    edited = false;
    interpretText();
  }
  return TKFloatRangeControl::value();
}

/*******************************************************************************************/

TKFloatRangeControl::TKFloatRangeControl()
{
  minVal = 0.0;
  maxVal = 1000000.0;
  line   = 0.1;
  page   = 1.0;
  val    = prevVal = 0.0;
}

TKFloatRangeControl::TKFloatRangeControl( float minValue, float maxValue, float lineStep, float pageStep, float value )
{
  minVal = minValue;
  maxVal = maxValue;
  line   = QABS( lineStep );
  page   = QABS( pageStep );
  val    = prevVal = bound( value );
}

void TKFloatRangeControl::setValue(float value)
{
  directSetValue( value );
  if ( prevVal != val )
    valueChange();
}

void TKFloatRangeControl::directSetValue(float value)
{
  prevVal = val;
  val = bound( value );
}

void TKFloatRangeControl::addPage()
{
  if ( value() + pageStep() > value() )
    setValue( value() + pageStep() );
  else
    setValue( maxValue() );
}

void TKFloatRangeControl::subtractPage()
{
  if ( value() - pageStep() < value() )
    setValue( value() - pageStep() );
  else
    setValue( minValue() );
}

void TKFloatRangeControl::addLine()
{
  setValue( value() + lineStep() );
}

void TKFloatRangeControl::subtractLine()
{
  setValue( value() - lineStep() );
}

void TKFloatRangeControl::setRange(float minValue, float maxValue)
{
  if ( minValue == minVal && maxValue == maxVal )
    return;
  if ( minValue > maxValue ) {
  	minVal = maxVal = minValue;
  } else {
    minVal = minValue;
    maxVal = maxValue;
  }
  float tmp = bound( val );
  rangeChange();
  if ( tmp != val ) {
    prevVal = tmp;
    val = tmp;
    valueChange();
  }
}

void TKFloatRangeControl::setSteps(float lineStep,float pageStep)
{
  if (lineStep != line || pageStep != page) {
    line = QABS(lineStep);
    page = QABS(pageStep);
    stepChange();
  }
}

void TKFloatRangeControl::valueChange()
{
}

void TKFloatRangeControl::rangeChange()
{
}

void TKFloatRangeControl::stepChange()
{
}

float TKFloatRangeControl::bound(float v) const
{
  if ( v - minVal < 1.0e-05 )
    return minVal;
  if ( v > maxVal )
    return maxVal;
  return v;
}
/***************************************************************************************/
TKUFloatSpinBox::TKUFloatSpinBox( QWidget* parent, const char* name)
: TKFloatSpinBox(parent,name)
{
  hideSuffix = false;
  setUnit((int)UnitPoint);
}

TKUFloatSpinBox::TKUFloatSpinBox( float minValue, float maxValue, float step, int decimal, QWidget* parent, const char* name)
: TKFloatSpinBox(minValue,maxValue,step,decimal,parent,name)
{
  setUnit((int)UnitPoint);
}

TKUFloatSpinBox::~TKUFloatSpinBox()
{
}

void TKUFloatSpinBox::setUnit(int unit)
{
  blockSignals(true);
  setSuffix(hideSuffix ? QString::null : unitToString(unit));
  float v = cvtPtToUnit(unit,cvtUnitToPt(m_unit,TKFloatSpinBox::value()));
  setMinValue( cvtPtToUnit(unit,cvtUnitToPt(m_unit,minValue())) );
  setMaxValue( cvtPtToUnit(unit,cvtUnitToPt(m_unit,maxValue())) );
  TKFloatSpinBox::setValue( v );

  m_unit = unit;
  blockSignals(false);
}

float TKUFloatSpinBox::value(int unit)
{
  return cvtPtToUnit(unit,cvtUnitToPt(m_unit,TKFloatSpinBox::value()));
}

void TKUFloatSpinBox::setValue(float value,int unit)
{
  TKFloatSpinBox::setValue( cvtPtToUnit(m_unit,cvtUnitToPt(unit,value)) );
}

void TKUFloatSpinBox::setHideSuffix(bool f)
{
  if (f== hideSuffix)
    return;

  hideSuffix = f;
  setSuffix(hideSuffix ? QString::null : unitToString(m_unit));
  updateGeometry();
}

#include "tkfloatspinbox.moc"
