#include "KSpreadCellIface.h"

#include "kspread_sheet.h"

KSpreadCellIface::KSpreadCellIface()
  : m_point( 0, 0 ), m_table( 0 )
{
}

void KSpreadCellIface::setCell( KSpreadSheet* table, const QPoint& point )
{
    m_table = table;
    m_point = point;
}

bool KSpreadCellIface::isDefault() const
{
    if( !m_table ) return false;
    KSpreadCell* cell = m_table->cellAt( m_point );
    return cell->isDefault();
}

bool KSpreadCellIface::isEmpty() const
{
	if (!m_table) return true;
	KSpreadCell *cell=m_table->cellAt(m_point);
	return cell->isEmpty();
}

QString KSpreadCellIface::text() const
{
    if( !m_table ) return QString::null;
    KSpreadCell* cell = m_table->cellAt( m_point );
    return cell->value().asString();
}

void KSpreadCellIface::setText( const QString& text )
{
    if( !m_table ) return;
    KSpreadCell* cell = m_table->nonDefaultCell( m_point.x(), m_point.y() );
    cell->setCellText( text );
}

QString KSpreadCellIface::visibleContentAsString() const
{
    if( !m_table ) return QString::null;
    KSpreadCell* cell = m_table->nonDefaultCell( m_point.x(), m_point.y() );
    if (cell->isEmpty()) return QString::null;
    QString ret;
    ret=cell->value().asString();

    if (ret.isEmpty()) 
    {
	ret=QString::number(cell->value().asFloat());
    }
    return ret;
}

QString KSpreadCellIface::comment() const
{
    if( !m_table ) return QString::null;
    KSpreadCell* cell = m_table->cellAt( m_point );
    return cell->comment(m_point.x(), m_point.y());
}

void KSpreadCellIface::setComment( const QString& comment )
{
    if( !m_table ) return;
    KSpreadCell* cell = m_table->nonDefaultCell( m_point.x(), m_point.y() );
    cell->setComment( comment);
    m_table->setRegionPaintDirty(cell->cellRect());
}

void KSpreadCellIface::setValue( int value )
{
    if( !m_table ) return;
    KSpreadCell* cell = m_table->nonDefaultCell( m_point.x(), m_point.y() );
    cell->setValue( (double)value );
}

void KSpreadCellIface::setValue( double value )
{
    if( !m_table ) return;
    KSpreadCell* cell = m_table->nonDefaultCell( m_point.x(), m_point.y() );
    cell->setValue( value );
}

double KSpreadCellIface::value() const
{
    if( !m_table ) return 0.0;
    KSpreadCell* cell = m_table->cellAt( m_point );
    return cell->value().asFloat();
}

void KSpreadCellIface::setBgColor(const QString& _c)
{
    if( !m_table ) return;
    KSpreadCell* cell = m_table->nonDefaultCell( m_point.x(), m_point.y() );
    QColor c(_c);
    cell->setBgColor(c);
    m_table->setRegionPaintDirty(cell->cellRect());
}

void KSpreadCellIface::setBgColor(int r,int g,int b)
{
    if( !m_table ) return;
    KSpreadCell* cell = m_table->nonDefaultCell( m_point.x(), m_point.y() );
    QColor c(r,g,b);
    cell->setBgColor(c);
    m_table->setRegionPaintDirty(cell->cellRect());
}

QString KSpreadCellIface::bgColor() const
{
    if( !m_table ) return QString::null;
    KSpreadCell* cell = m_table->cellAt( m_point );
    return cell->bgColor( m_point.x(), m_point.y() ).name();
}

QString KSpreadCellIface::textColor() const
{
    if( !m_table ) return QString::null;
    KSpreadCell* cell = m_table->cellAt( m_point );
    return cell->textColor( m_point.x(), m_point.y() ).name();
}

void KSpreadCellIface::setTextColor(int r,int g,int b)
{
    if( !m_table ) return;
    KSpreadCell* cell = m_table->nonDefaultCell( m_point.x(), m_point.y() );
    QColor c(r,g,b);
    cell->setTextColor(c);
    m_table->setRegionPaintDirty(cell->cellRect());
}

void KSpreadCellIface::setTextColor(const QString& _c)
{
    if( !m_table ) return;
    KSpreadCell* cell = m_table->nonDefaultCell( m_point.x(), m_point.y() );
    QColor c(_c);
    cell->setTextColor(c);
    m_table->setRegionPaintDirty(cell->cellRect());
}

void KSpreadCellIface::setAngle(int angle)
{
    if( !m_table ) return;
    KSpreadCell* cell = m_table->nonDefaultCell( m_point.x(), m_point.y() );
    cell->setAngle(angle);
    m_table->setRegionPaintDirty(cell->cellRect());
}

int  KSpreadCellIface::angle() const
{
    if( !m_table ) return 0;
    KSpreadCell* cell = m_table->nonDefaultCell( m_point.x(), m_point.y() );
    return cell->getAngle(m_point.x(), m_point.y());
}

void KSpreadCellIface::setVerticalText(bool _vertical)
{
    if( !m_table ) return;
    KSpreadCell* cell = m_table->nonDefaultCell( m_point.x(), m_point.y() );
    cell->setVerticalText(_vertical);
    m_table->setRegionPaintDirty(cell->cellRect());
}

bool KSpreadCellIface::verticalText() const
{
    if( !m_table ) return false;
    KSpreadCell* cell = m_table->nonDefaultCell( m_point.x(), m_point.y() );
    return cell->verticalText( m_point.x(), m_point.y() );
}


void KSpreadCellIface::setMultiRow(bool _multi)
{
    if( !m_table ) return;
    KSpreadCell* cell = m_table->nonDefaultCell( m_point.x(), m_point.y() );
    cell->setMultiRow( _multi );
    m_table->setRegionPaintDirty(cell->cellRect());
}

bool KSpreadCellIface::multiRow() const
{
    if( !m_table ) return false;
    KSpreadCell* cell = m_table->nonDefaultCell( m_point.x(), m_point.y() );
    return cell->multiRow( m_point.x(), m_point.y() );
}

void KSpreadCellIface::setAlign( const QString& _Align )
{
    if( !m_table ) return;
    KSpreadCell* cell = m_table->nonDefaultCell( m_point.x(), m_point.y() );
    KSpreadCell::Align Align;
    if(_Align=="Left")
        Align=KSpreadCell::Left;
    else if(_Align=="Right")
        Align=KSpreadCell::Right;
    else if(_Align=="Center")
        Align=KSpreadCell::Center;
    else
        Align=KSpreadCell::Undefined;
    cell->setAlign( Align);
    m_table->setRegionPaintDirty(cell->cellRect());
}

QString KSpreadCellIface::align() const
{
    if( !m_table ) return QString::null;
    KSpreadCell* cell = m_table->nonDefaultCell( m_point.x(), m_point.y() );
    QString alignString;
    switch( cell->align( m_point.x(), m_point.y() ) )
        {
        case KSpreadCell::Left :
                alignString="Left";
                break;
        case KSpreadCell::Right :
                alignString="Right";
                break;
        case KSpreadCell::Center :
                alignString="Center";
                break;
        case KSpreadCell::Undefined :
                alignString="Undefined";
                break;
        }
    return alignString;
}

void KSpreadCellIface::setAlignY( const QString& _AlignY )
{
    if( !m_table ) return;
    KSpreadCell* cell = m_table->nonDefaultCell( m_point.x(), m_point.y() );
    KSpreadCell::AlignY AlignY;
    if(_AlignY=="Top")
        AlignY=KSpreadCell::Top;
    else if(_AlignY=="Middle")
        AlignY=KSpreadCell::Middle;
    else if(_AlignY=="Bottom")
        AlignY=KSpreadCell::Bottom;
    else
        AlignY=KSpreadCell::Middle;
    cell->setAlignY( AlignY);
    m_table->setRegionPaintDirty(cell->cellRect());
}

QString KSpreadCellIface::alignY() const
{
    if( !m_table ) return QString::null;
    KSpreadCell* cell = m_table->nonDefaultCell( m_point.x(), m_point.y() );
    QString alignString;
    switch( cell->alignY( m_point.x(), m_point.y() ) )
        {
        case KSpreadCell::Top :
                alignString="Top";
                break;
        case KSpreadCell::Middle :
                alignString="Middle";
                break;
        case KSpreadCell::Bottom :
                alignString="Bottom";
                break;
        }
    return alignString;
}

void KSpreadCellIface::setPostfix(const QString &_postfix)
{
    if( !m_table ) return;
    KSpreadCell* cell = m_table->nonDefaultCell( m_point.x(), m_point.y() );
    cell->setPostfix( _postfix );
    m_table->setRegionPaintDirty(cell->cellRect());
}

QString KSpreadCellIface::prefix() const
{
    if( !m_table ) return QString::null;
    KSpreadCell* cell = m_table->nonDefaultCell( m_point.x(), m_point.y() );
    return cell->prefix( m_point.x(), m_point.y() );
}

void KSpreadCellIface::setPrefix(const QString &_prefix)
{
    if( !m_table ) return;
    KSpreadCell* cell = m_table->nonDefaultCell( m_point.x(), m_point.y() );
    cell->setPrefix( _prefix );
    m_table->setRegionPaintDirty(cell->cellRect());
}

QString KSpreadCellIface::postfix() const
{
    if( !m_table ) return QString::null;
    KSpreadCell* cell = m_table->nonDefaultCell( m_point.x(), m_point.y() );
    return cell->postfix( m_point.x(), m_point.y() );
}

void KSpreadCellIface::setFormatType(const QString &_formatType)
{
    if( !m_table ) return;

    KSpreadCell* cell = m_table->nonDefaultCell( m_point.x(), m_point.y() );
    KSpreadCell::FormatType format;
    cell->setFactor( 1.0);
    cell->setPrecision(2);
    if(_formatType=="Number")
        format=KSpreadCell::Number;
    else if (_formatType=="Text")
        format=KSpreadCell::Text_format;
    else if(_formatType=="Money")
        format=KSpreadCell::Money;
    else if(_formatType=="Percentage")
        {
        format=KSpreadCell::Percentage;
        cell->setFactor( 100.0);
        }
    else if(_formatType=="Scientific")
        format=KSpreadCell::Scientific;
    else if(_formatType=="ShortDate")
        format=KSpreadCell::ShortDate;
    else if(_formatType=="TextDate")
        format=KSpreadCell::TextDate;
    else if(_formatType=="Time")
        format=KSpreadCell::Time;
    else if(_formatType=="SecondeTime")
        format=KSpreadCell::SecondeTime;
    else if(_formatType=="fraction_half")
        format=KSpreadCell::fraction_half;
    else if(_formatType=="fraction_quarter")
        format=KSpreadCell::fraction_quarter;
    else if(_formatType=="fraction_eighth")
        format=KSpreadCell::fraction_eighth;
    else if(_formatType=="fraction_sixteenth")
        format=KSpreadCell::fraction_sixteenth;
    else if(_formatType=="fraction_tenth")
        format=KSpreadCell::fraction_tenth;
    else if(_formatType=="fraction_hundredth")
        format=KSpreadCell::fraction_hundredth;
    else if(_formatType=="fraction_one_digit")
        format=KSpreadCell::fraction_one_digit;
    else if(_formatType=="fraction_two_digits")
        format=KSpreadCell::fraction_two_digits;
    else if(_formatType=="fraction_three_digits")
        format=KSpreadCell::fraction_three_digits;
    else
        format=KSpreadCell::Number;
    cell->setFormatType( format);
    m_table->setRegionPaintDirty(cell->cellRect());
}

QString KSpreadCellIface::getFormatType() const
{
    if( !m_table ) return QString::null;

    KSpreadCell* cell = m_table->nonDefaultCell( m_point.x(), m_point.y() );
    QString stringFormat;
    switch( cell->getFormatType(m_point.x(), m_point.y()))
        {
        case KSpreadCell::Text_format:
            stringFormat="Text";
            break;
        case KSpreadCell::Number:
                stringFormat="Number";
                break;
        case KSpreadCell::Money:
                stringFormat="Money";
                break;
        case KSpreadCell::Percentage:
                stringFormat="Percentage";
                break;
        case KSpreadCell::Scientific:
                stringFormat="Scientific";
                break;
        case KSpreadCell::ShortDate:
                stringFormat="ShortDate";
                break;
        case KSpreadCell::TextDate:
                stringFormat="TextDate";
                break;
        case KSpreadCell::date_format1:
        case KSpreadCell::date_format2:
        case KSpreadCell::date_format3:
        case KSpreadCell::date_format4:
        case KSpreadCell::date_format5:
        case KSpreadCell::date_format6:
        case KSpreadCell::date_format7:
        case KSpreadCell::date_format8:
        case KSpreadCell::date_format9:
        case KSpreadCell::date_format10:
        case KSpreadCell::date_format11:
        case KSpreadCell::date_format12:
        case KSpreadCell::date_format13:
        case KSpreadCell::date_format14:
        case KSpreadCell::date_format15:
        case KSpreadCell::date_format16:
	case KSpreadCell::date_format17:
	case KSpreadCell::date_format18:
	case KSpreadCell::date_format19:
	case KSpreadCell::date_format20:
	case KSpreadCell::date_format21:
	case KSpreadCell::date_format22:
	case KSpreadCell::date_format23:
	case KSpreadCell::date_format24:
	case KSpreadCell::date_format25:
	case KSpreadCell::date_format26:
                stringFormat="date format";
                break;
        case KSpreadCell::Time:
                stringFormat="Time";
                break;
        case KSpreadCell::SecondeTime:
                stringFormat="SecondeTime";
                break;
        case KSpreadCell::Time_format1:
        case KSpreadCell::Time_format2:
        case KSpreadCell::Time_format3:
        case KSpreadCell::Time_format4:
        case KSpreadCell::Time_format5:
        case KSpreadCell::Time_format6:
        case KSpreadCell::Time_format7:
        case KSpreadCell::Time_format8:
                stringFormat="time format";
                break;
        case KSpreadCell::fraction_half:
                stringFormat="fraction_half";
                break;
        case KSpreadCell::fraction_quarter:
                stringFormat="fraction_quarter";
                break;
        case KSpreadCell::fraction_eighth:
                stringFormat="fraction_eighth";
                break;
        case KSpreadCell::fraction_sixteenth:
                stringFormat="fraction_sixteenth";
                break;
        case KSpreadCell::fraction_tenth:
                stringFormat="fraction_tenth";
                break;
        case KSpreadCell::fraction_hundredth:
                stringFormat="fraction_hundredth";
                break;
        case KSpreadCell::fraction_one_digit:
                stringFormat="fraction_one_digit";
                break;
        case KSpreadCell::fraction_two_digits:
                stringFormat="fraction_two_digits";
                break;
        case KSpreadCell::fraction_three_digits:
                stringFormat="fraction_three_digits";
                break;
        }
    return stringFormat;
}

void KSpreadCellIface::setPrecision(int _p)
{
    if( !m_table ) return;
    KSpreadCell* cell = m_table->nonDefaultCell( m_point.x(), m_point.y() );
    cell->setPrecision( _p );
    m_table->setRegionPaintDirty(cell->cellRect());
}

int KSpreadCellIface::precision() const
{
    if( !m_table ) return 0;
    KSpreadCell* cell = m_table->nonDefaultCell( m_point.x(), m_point.y() );
    return cell->precision( m_point.x(), m_point.y() );
}

void KSpreadCellIface::setTextFontBold(bool _b)
{
    if( !m_table ) return;
    KSpreadCell* cell = m_table->nonDefaultCell( m_point.x(), m_point.y() );
    cell->setTextFontBold( _b );
    m_table->setRegionPaintDirty(cell->cellRect());
}

bool KSpreadCellIface::textFontBold() const
{
    if( !m_table ) return false;
    KSpreadCell* cell = m_table->nonDefaultCell( m_point.x(), m_point.y() );
    return cell->textFontBold( m_point.x(), m_point.y() );
}

void KSpreadCellIface::setTextFontItalic(bool _b)
{
    if( !m_table ) return;
    KSpreadCell* cell = m_table->nonDefaultCell( m_point.x(), m_point.y() );
    cell->setTextFontItalic( _b );
    m_table->setRegionPaintDirty(cell->cellRect());
}

bool KSpreadCellIface::textFontItalic() const
{
    if( !m_table ) return false;
    KSpreadCell* cell = m_table->nonDefaultCell( m_point.x(), m_point.y() );
    return cell->textFontItalic( m_point.x(), m_point.y() );
}


void KSpreadCellIface::setTextFontUnderline(bool _b)
{
    if( !m_table ) return;
    KSpreadCell* cell = m_table->nonDefaultCell( m_point.x(), m_point.y() );
    cell->setTextFontUnderline( _b );
    m_table->setRegionPaintDirty(cell->cellRect());
}

bool KSpreadCellIface::textFontUnderline() const
{
    if( !m_table ) return false;
    KSpreadCell* cell = m_table->nonDefaultCell( m_point.x(), m_point.y() );
    return cell->textFontUnderline( m_point.x(), m_point.y() );
}

void KSpreadCellIface::setTextFontStrike(bool _b)
{
    if( !m_table ) return;
    KSpreadCell* cell = m_table->nonDefaultCell( m_point.x(), m_point.y() );
    cell->setTextFontStrike( _b );
    m_table->setRegionPaintDirty(cell->cellRect());
}

bool KSpreadCellIface::textFontStrike() const
{
    if( !m_table ) return false;
    KSpreadCell* cell = m_table->nonDefaultCell( m_point.x(), m_point.y() );
    return cell->textFontStrike( m_point.x(), m_point.y() );
}

void KSpreadCellIface::setTextFontSize( int _size )
{
    if( !m_table ) return;
    KSpreadCell* cell = m_table->nonDefaultCell( m_point.x(), m_point.y() );
    cell->setTextFontSize( _size );
    m_table->setRegionPaintDirty(cell->cellRect());
}

int KSpreadCellIface::textFontSize() const
{
    if( !m_table ) return 10;
    KSpreadCell* cell = m_table->nonDefaultCell( m_point.x(), m_point.y() );
    return cell->textFontSize( m_point.x(), m_point.y() );
}

void KSpreadCellIface::setTextFontFamily( const QString& _font )
{
    if( !m_table ) return;
    KSpreadCell* cell = m_table->nonDefaultCell( m_point.x(), m_point.y() );
    cell->setTextFontFamily( _font );
    m_table->setRegionPaintDirty(cell->cellRect());
}

QString KSpreadCellIface::textFontFamily() const
{
    if( !m_table ) return QString::null;
    KSpreadCell* cell = m_table->nonDefaultCell( m_point.x(), m_point.y() );
    return cell->textFontFamily( m_point.x(), m_point.y() );
}

void KSpreadCellIface::setFaktor( double _factor )
{
    if( !m_table ) return;
    KSpreadCell* cell = m_table->nonDefaultCell( m_point.x(), m_point.y() );
    cell->setFactor( _factor );
    m_table->setRegionPaintDirty(cell->cellRect());
}

double KSpreadCellIface::faktor() const
{
    if( !m_table ) return 1.0;
    KSpreadCell* cell = m_table->nonDefaultCell( m_point.x(), m_point.y() );
    return cell->factor( m_point.x(), m_point.y() );
}

//border left
void KSpreadCellIface::setLeftBorderStyle( const QString& _style )
{
    if( !m_table ) return;
    KSpreadCell* cell = m_table->nonDefaultCell( m_point.x(), m_point.y() );
    if(_style=="DotLine")
        cell->setLeftBorderStyle(Qt::DotLine);
    else if(_style=="DashLine")
        cell->setLeftBorderStyle(Qt::DashLine);
    else if(_style=="DashDotLine")
        cell->setLeftBorderStyle(Qt::DashDotLine);
    else if(_style=="DashDotDotLine")
        cell->setLeftBorderStyle(Qt::DashDotDotLine);
    else if(_style=="SolidLine")
        cell->setLeftBorderStyle(Qt::SolidLine);
    else
        cell->setLeftBorderStyle(Qt::SolidLine);
    m_table->setRegionPaintDirty(cell->cellRect());

}

void KSpreadCellIface::setLeftBorderColor(const QString& _c)
{
    if( !m_table ) return;
    QColor c(_c);
    KSpreadCell* cell = m_table->nonDefaultCell( m_point.x(), m_point.y() );
    cell->setLeftBorderColor(c );
    m_table->setRegionPaintDirty(cell->cellRect());

}

void KSpreadCellIface::setLeftBorderColor(int r,int g,int b)
{
    if( !m_table ) return;
    KSpreadCell* cell = m_table->nonDefaultCell( m_point.x(), m_point.y() );
    QColor c(r,g,b);
    cell->setLeftBorderColor(c );
    m_table->setRegionPaintDirty(cell->cellRect());
}

void KSpreadCellIface::setLeftBorderWidth( int _size )
{
    if( !m_table ) return;
    KSpreadCell* cell = m_table->nonDefaultCell( m_point.x(), m_point.y() );
    cell->setLeftBorderWidth( _size );
    m_table->setRegionPaintDirty(cell->cellRect());
}


int  KSpreadCellIface::leftBorderWidth() const
{
    if( !m_table ) return 0;
    KSpreadCell* cell = m_table->nonDefaultCell( m_point.x(), m_point.y() );
    return cell->leftBorderWidth(m_point.x(), m_point.y());
}

QString KSpreadCellIface::leftBorderColor() const
{
    if( !m_table ) return QString::null;
    KSpreadCell* cell = m_table->cellAt( m_point );
    return cell->leftBorderColor( m_point.x(), m_point.y() ).name();
}

QString KSpreadCellIface::leftBorderStyle() const
{
    if( !m_table ) return QString::null;
    KSpreadCell* cell = m_table->cellAt( m_point );
    Qt::PenStyle penStyle=cell->leftBorderStyle( m_point.x(), m_point.y() );
    QString tmp;
    if( penStyle==Qt::DotLine)
        tmp="DotLine";
    else if( penStyle==Qt::DashLine)
        tmp="DashLine";
    else if( penStyle==Qt::DashDotLine)
        tmp="DashDotLine";
    else if( penStyle==Qt::DashDotDotLine)
        tmp="DashDotDotLine";
    else if( penStyle==Qt::SolidLine)
        tmp="SolidLine";
    else
        tmp="SolidLine";
    return tmp;
}

//border right
void KSpreadCellIface::setRightBorderStyle( const QString& _style )
{
    if( !m_table ) return;
    KSpreadCell* cell = m_table->nonDefaultCell( m_point.x(), m_point.y() );
    if(_style=="DotLine")
        cell->setRightBorderStyle(Qt::DotLine);
    else if(_style=="DashLine")
        cell->setRightBorderStyle(Qt::DashLine);
    else if(_style=="DashDotLine")
        cell->setRightBorderStyle(Qt::DashDotLine);
    else if(_style=="DashDotDotLine")
        cell->setRightBorderStyle(Qt::DashDotDotLine);
    else if(_style=="SolidLine")
        cell->setRightBorderStyle(Qt::SolidLine);
    else
        cell->setRightBorderStyle(Qt::SolidLine);
    m_table->setRegionPaintDirty(cell->cellRect());

}

void KSpreadCellIface::setRightBorderColor(const QString& _c)
{
    if( !m_table ) return;
    QColor c(_c);
    KSpreadCell* cell = m_table->nonDefaultCell( m_point.x(), m_point.y() );
    cell->setRightBorderColor(c );
    m_table->setRegionPaintDirty(cell->cellRect());

}
void KSpreadCellIface::setRightBorderColor(int r,int g,int b)
{
    if( !m_table ) return;
    KSpreadCell* cell = m_table->nonDefaultCell( m_point.x(), m_point.y() );
    QColor c(r,g,b);
    cell->setRightBorderColor(c );
    m_table->setRegionPaintDirty(cell->cellRect());
}

void KSpreadCellIface::setRightBorderWidth( int _size )
{
    if( !m_table ) return;
    KSpreadCell* cell = m_table->nonDefaultCell( m_point.x(), m_point.y() );
    cell->setRightBorderWidth( _size );
    m_table->setRegionPaintDirty(cell->cellRect());

}

int  KSpreadCellIface::rightBorderWidth() const
{
    if( !m_table ) return 0;
    KSpreadCell* cell = m_table->nonDefaultCell( m_point.x(), m_point.y() );
    return cell->rightBorderWidth(m_point.x(), m_point.y());
}

QString KSpreadCellIface::rightBorderColor() const
{
    if( !m_table ) return QString::null;
    KSpreadCell* cell = m_table->cellAt( m_point );
    return cell->rightBorderColor( m_point.x(), m_point.y() ).name();
}

QString KSpreadCellIface::rightBorderStyle() const
{
    if( !m_table ) return QString::null;
    KSpreadCell* cell = m_table->cellAt( m_point );
    Qt::PenStyle penStyle=cell->rightBorderStyle( m_point.x(), m_point.y() );
    QString tmp;
    if( penStyle==Qt::DotLine)
        tmp="DotLine";
    else if( penStyle==Qt::DashLine)
        tmp="DashLine";
    else if( penStyle==Qt::DashDotLine)
        tmp="DashDotLine";
    else if( penStyle==Qt::DashDotDotLine)
        tmp="DashDotDotLine";
    else if( penStyle==Qt::SolidLine)
        tmp="SolidLine";
    else
        tmp="SolidLine";
    return tmp;
}

//border top
void KSpreadCellIface::setTopBorderStyle( const QString& _style )
{
    if( !m_table ) return;
    KSpreadCell* cell = m_table->nonDefaultCell( m_point.x(), m_point.y() );
    if(_style=="DotLine")
        cell->setTopBorderStyle(Qt::DotLine);
    else if(_style=="DashLine")
        cell->setTopBorderStyle(Qt::DashLine);
    else if(_style=="DashDotLine")
        cell->setTopBorderStyle(Qt::DashDotLine);
    else if(_style=="DashDotDotLine")
        cell->setTopBorderStyle(Qt::DashDotDotLine);
    else if(_style=="SolidLine")
        cell->setTopBorderStyle(Qt::SolidLine);
    else
        cell->setTopBorderStyle(Qt::SolidLine);
    m_table->setRegionPaintDirty(cell->cellRect());

}

void KSpreadCellIface::setTopBorderColor(const QString& _c)
{
    if( !m_table ) return;
    QColor c(_c);
    KSpreadCell* cell = m_table->nonDefaultCell( m_point.x(), m_point.y() );
    cell->setTopBorderColor(c );
    m_table->setRegionPaintDirty(cell->cellRect());

}
void KSpreadCellIface::setTopBorderColor(int r,int g,int b)
{
    if( !m_table ) return;
    KSpreadCell* cell = m_table->nonDefaultCell( m_point.x(), m_point.y() );
    QColor c(r,g,b);
    cell->setTopBorderColor(c );
    m_table->setRegionPaintDirty(cell->cellRect());
}

void KSpreadCellIface::setTopBorderWidth( int _size )
{
    if( !m_table ) return;
    KSpreadCell* cell = m_table->nonDefaultCell( m_point.x(), m_point.y() );
    cell->setTopBorderWidth( _size );
    m_table->setRegionPaintDirty(cell->cellRect());
}

int  KSpreadCellIface::topBorderWidth() const
{
    if( !m_table ) return 0;
    KSpreadCell* cell = m_table->nonDefaultCell( m_point.x(), m_point.y() );
    return cell->topBorderWidth(m_point.x(), m_point.y());
}

QString KSpreadCellIface::topBorderColor() const
{
    if( !m_table ) return QString::null;
    KSpreadCell* cell = m_table->cellAt( m_point );
    return cell->topBorderColor( m_point.x(), m_point.y() ).name();
}

QString KSpreadCellIface::topBorderStyle() const
{
    if( !m_table ) return QString::null;
    KSpreadCell* cell = m_table->cellAt( m_point );
    Qt::PenStyle penStyle=cell->topBorderStyle( m_point.x(), m_point.y() );
    QString tmp;
    if( penStyle==Qt::DotLine)
        tmp="DotLine";
    else if( penStyle==Qt::DashLine)
        tmp="DashLine";
    else if( penStyle==Qt::DashDotLine)
        tmp="DashDotLine";
    else if( penStyle==Qt::DashDotDotLine)
        tmp="DashDotDotLine";
    else if( penStyle==Qt::SolidLine)
        tmp="SolidLine";
    else
        tmp="SolidLine";
    return tmp;
}

//border bottom
void KSpreadCellIface::setBottomBorderStyle( const QString& _style )
{
    if( !m_table ) return;
    KSpreadCell* cell = m_table->nonDefaultCell( m_point.x(), m_point.y() );
    if(_style=="DotLine")
        cell->setBottomBorderStyle(Qt::DotLine);
    else if(_style=="DashLine")
        cell->setBottomBorderStyle(Qt::DashLine);
    else if(_style=="DashDotLine")
        cell->setBottomBorderStyle(Qt::DashDotLine);
    else if(_style=="DashDotDotLine")
        cell->setBottomBorderStyle(Qt::DashDotDotLine);
    else if(_style=="SolidLine")
        cell->setBottomBorderStyle(Qt::SolidLine);
    else
        cell->setBottomBorderStyle(Qt::SolidLine);
    m_table->setRegionPaintDirty(cell->cellRect());

}

void KSpreadCellIface::setBottomBorderColor(const QString& _c)
{
    if( !m_table ) return;
    QColor c(_c);
    KSpreadCell* cell = m_table->nonDefaultCell( m_point.x(), m_point.y() );
    cell->setBottomBorderColor(c );
    m_table->setRegionPaintDirty(cell->cellRect());

}
void KSpreadCellIface::setBottomBorderColor(int r,int g,int b)
{
    if( !m_table ) return;
    KSpreadCell* cell = m_table->nonDefaultCell( m_point.x(), m_point.y() );
    QColor c(r,g,b);
    cell->setBottomBorderColor(c );
    m_table->setRegionPaintDirty(cell->cellRect());
}

void KSpreadCellIface::setBottomBorderWidth( int _size )
{
    if( !m_table ) return;
    KSpreadCell* cell = m_table->nonDefaultCell( m_point.x(), m_point.y() );
    cell->setBottomBorderWidth( _size );
    m_table->setRegionPaintDirty(cell->cellRect());
}

int  KSpreadCellIface::bottomBorderWidth() const
{
    if( !m_table ) return 0;
    KSpreadCell* cell = m_table->nonDefaultCell( m_point.x(), m_point.y() );
    return cell->bottomBorderWidth(m_point.x(), m_point.y());
}

QString KSpreadCellIface::bottomBorderColor() const
{
    if( !m_table ) return QString::null;
    KSpreadCell* cell = m_table->cellAt( m_point );
    return cell->bottomBorderColor( m_point.x(), m_point.y() ).name();
}

QString KSpreadCellIface::bottomBorderStyle() const
{
    if( !m_table ) return QString::null;
    KSpreadCell* cell = m_table->cellAt( m_point );
    Qt::PenStyle penStyle=cell->bottomBorderStyle( m_point.x(), m_point.y() );
    QString tmp;
    if( penStyle==Qt::DotLine)
        tmp="DotLine";
    else if( penStyle==Qt::DashLine)
        tmp="DashLine";
    else if( penStyle==Qt::DashDotLine)
        tmp="DashDotLine";
    else if( penStyle==Qt::DashDotDotLine)
        tmp="DashDotDotLine";
    else if( penStyle==Qt::SolidLine)
        tmp="SolidLine";
    else
        tmp="SolidLine";
    return tmp;
}

//fall back diagonal
void KSpreadCellIface::setFallDiagonalStyle( const QString& _style )
{
    if( !m_table ) return;
    KSpreadCell* cell = m_table->nonDefaultCell( m_point.x(), m_point.y() );
    if(_style=="DotLine")
        cell->setFallDiagonalStyle(Qt::DotLine);
    else if(_style=="DashLine")
        cell->setFallDiagonalStyle(Qt::DashLine);
    else if(_style=="DashDotLine")
        cell->setFallDiagonalStyle(Qt::DashDotLine);
    else if(_style=="DashDotDotLine")
        cell->setFallDiagonalStyle(Qt::DashDotDotLine);
    else if(_style=="SolidLine")
        cell->setFallDiagonalStyle(Qt::SolidLine);
    else
        cell->setFallDiagonalStyle(Qt::SolidLine);
    m_table->setRegionPaintDirty(cell->cellRect());

}

void KSpreadCellIface::setFallDiagonalColor(const QString& _c)
{
    if( !m_table ) return;
    QColor c(_c);
    KSpreadCell* cell = m_table->nonDefaultCell( m_point.x(), m_point.y() );
    cell->setFallDiagonalColor(c );
    m_table->setRegionPaintDirty(cell->cellRect());

}
void KSpreadCellIface::setFallDiagonalColor(int r,int g,int b)
{
    if( !m_table ) return;
    KSpreadCell* cell = m_table->nonDefaultCell( m_point.x(), m_point.y() );
    QColor c(r,g,b);
    cell->setFallDiagonalColor(c );
    m_table->setRegionPaintDirty(cell->cellRect());
}

void KSpreadCellIface::setFallDiagonalWidth( int _size )
{
    if( !m_table ) return;
    KSpreadCell* cell = m_table->nonDefaultCell( m_point.x(), m_point.y() );
    cell->setFallDiagonalWidth( _size );
    m_table->setRegionPaintDirty(cell->cellRect());
}

int  KSpreadCellIface::fallDiagonalWidth() const
{
    if( !m_table ) return 0;
    KSpreadCell* cell = m_table->nonDefaultCell( m_point.x(), m_point.y() );
    return cell->fallDiagonalWidth(m_point.x(), m_point.y());
}

QString KSpreadCellIface::fallDiagonalColor() const
{
    if( !m_table ) return QString::null;
    KSpreadCell* cell = m_table->cellAt( m_point );
    return cell->fallDiagonalColor( m_point.x(), m_point.y() ).name();
}

QString KSpreadCellIface::fallDiagonalStyle() const
{
    if( !m_table ) return QString::null;
    KSpreadCell* cell = m_table->cellAt( m_point );
    Qt::PenStyle penStyle=cell->fallDiagonalStyle( m_point.x(), m_point.y() );
    QString tmp;
    if( penStyle==Qt::DotLine)
        tmp="DotLine";
    else if( penStyle==Qt::DashLine)
        tmp="DashLine";
    else if( penStyle==Qt::DashDotLine)
        tmp="DashDotLine";
    else if( penStyle==Qt::DashDotDotLine)
        tmp="DashDotDotLine";
    else if( penStyle==Qt::SolidLine)
        tmp="SolidLine";
    else
        tmp="SolidLine";
    return tmp;
}


//GoUpDiagonal
void KSpreadCellIface::setGoUpDiagonalStyle( const QString& _style )
{
    if( !m_table ) return;
    KSpreadCell* cell = m_table->nonDefaultCell( m_point.x(), m_point.y() );
    if(_style=="DotLine")
        cell->setGoUpDiagonalStyle(Qt::DotLine);
    else if(_style=="DashLine")
        cell->setGoUpDiagonalStyle(Qt::DashLine);
    else if(_style=="DashDotLine")
        cell->setGoUpDiagonalStyle(Qt::DashDotLine);
    else if(_style=="DashDotDotLine")
        cell->setGoUpDiagonalStyle(Qt::DashDotDotLine);
    else if(_style=="SolidLine")
        cell->setGoUpDiagonalStyle(Qt::SolidLine);
    else
        cell->setGoUpDiagonalStyle(Qt::SolidLine);
    m_table->setRegionPaintDirty(cell->cellRect());

}

void KSpreadCellIface::setGoUpDiagonalColor(const QString& _c)
{
    if( !m_table ) return;
    QColor c(_c);
    KSpreadCell* cell = m_table->nonDefaultCell( m_point.x(), m_point.y() );
    cell->setGoUpDiagonalColor(c );
    m_table->setRegionPaintDirty(cell->cellRect());

}
void KSpreadCellIface::setGoUpDiagonalColor(int r,int g,int b)
{
    if( !m_table ) return;
    KSpreadCell* cell = m_table->nonDefaultCell( m_point.x(), m_point.y() );
    QColor c(r,g,b);
    cell->setGoUpDiagonalColor(c );
    m_table->setRegionPaintDirty(cell->cellRect());
}

void KSpreadCellIface::setGoUpDiagonalWidth( int _size )
{
    if( !m_table ) return;
    KSpreadCell* cell = m_table->nonDefaultCell( m_point.x(), m_point.y() );
    cell->setGoUpDiagonalWidth( _size );
    m_table->setRegionPaintDirty(cell->cellRect());
}

int  KSpreadCellIface::goUpDiagonalWidth() const
{
    if( !m_table ) return 0;
    KSpreadCell* cell = m_table->nonDefaultCell( m_point.x(), m_point.y() );
    return cell->goUpDiagonalWidth(m_point.x(), m_point.y());
}

QString KSpreadCellIface::goUpDiagonalColor() const
{
    if( !m_table ) return QString::null;
    KSpreadCell* cell = m_table->cellAt( m_point );
    return cell->goUpDiagonalColor( m_point.x(), m_point.y() ).name();
}

QString KSpreadCellIface::goUpDiagonalStyle() const
{
    if( !m_table ) return QString::null;
    KSpreadCell* cell = m_table->cellAt( m_point );
    Qt::PenStyle penStyle=cell->goUpDiagonalStyle( m_point.x(), m_point.y() );
    QString tmp;
    if( penStyle==Qt::DotLine)
        tmp="DotLine";
    else if( penStyle==Qt::DashLine)
        tmp="DashLine";
    else if( penStyle==Qt::DashDotLine)
        tmp="DashDotLine";
    else if( penStyle==Qt::DashDotDotLine)
        tmp="DashDotDotLine";
    else if( penStyle==Qt::SolidLine)
        tmp="SolidLine";
    else
        tmp="SolidLine";
    return tmp;
}

void KSpreadCellIface::setIndent(double indent)
{
    if( !m_table ) return;
    KSpreadCell* cell = m_table->nonDefaultCell( m_point.x(), m_point.y() );
    if( indent >= 0.0 )
        cell->setIndent( indent );
    else
        cell->setIndent( 0.0 );
    m_table->setRegionPaintDirty(cell->cellRect());
}

double KSpreadCellIface::getIndent() const
{
    if( !m_table ) return 0.0;
    KSpreadCell* cell = m_table->nonDefaultCell( m_point.x(), m_point.y() );
    return cell->getIndent( m_point.x(), m_point.y() );
}

void KSpreadCellIface::setDontPrintText ( bool _print)
{
    if( !m_table ) return;
    KSpreadCell* cell = m_table->nonDefaultCell( m_point.x(), m_point.y() );
    cell->setDontPrintText ( _print);
}

bool KSpreadCellIface::getDontprintText() const
{
    if( !m_table ) return false;
    KSpreadCell* cell = m_table->nonDefaultCell( m_point.x(), m_point.y() );
    return cell->getDontprintText(m_point.x(), m_point.y());
}
