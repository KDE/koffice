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

#include "paragdia.h"
#include "paragdia_p.h"
#include "kwdoc.h"
#include "counter.h"
#include "defs.h"

#include <qbrush.h>
#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qcolor.h>
#include <qcombobox.h>
#include <qframe.h>
#include <qgroupbox.h>
#include <qhbuttongroup.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qlistbox.h>
#include <qpainter.h>
#include <qpen.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qspinbox.h>
#include <qvalidator.h>
#include <qvbox.h>
#include <qwhatsthis.h>
#include <qwidget.h>
#include <qdict.h>

#include <kapp.h>
#include <kbuttonbox.h>
#include <kcharselectdia.h>
#include <kcolorbtn.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kwutils.h>
#include <kwstyle.h>
#include "kwtextdocument.h"

KWSpinBox::KWSpinBox( QWidget * parent, const char * name )
    : QSpinBox(parent,name)
{
    m_Etype=NONE;
    //max value supported by roman number
    setMaxValue ( 3999 );
}
KWSpinBox::~KWSpinBox( )
{
}

KWSpinBox::KWSpinBox( int minValue, int maxValue, int step ,
           QWidget * parent , const char * name  )
    : QSpinBox(minValue, maxValue,step ,
           parent , name)
{
    m_Etype=NONE;
}

void KWSpinBox::setCounterType(counterType _type)
{
    m_Etype=_type;
    editor()->setText(mapValueToText(value()));
}


QString KWSpinBox::mapValueToText( int value )
{
    if(value==0 && m_Etype==NUM)
        return QString("0");
    else if(value==0 && m_Etype!=NUM)
        return QString::null;
    switch(m_Etype)
    {
        case NUM:
            return QString::number(value);
        case ALPHAB_L:
            return makeAlphaLowerNumber( value );
        case ALPHAB_U:
            return makeAlphaUpperNumber( value );
        case ROM_NUM_L:
            return makeRomanNumber( value );
        case ROM_NUM_U:
            return makeRomanNumber( value ).upper();
        case NONE:
        default:
            return QString::null;
    }
    //never here
    return QString::null;
}



/******************************************************************/
/* class KWPagePreview                                            */
/******************************************************************/

KWPagePreview::KWPagePreview( QWidget* parent, const char* name )
    : QGroupBox( i18n( "Preview" ), parent, name )
{
    left = 0;
    right = 0;
    first = 0;
    spacing = 0;
    before = 0;
    after = 0;
}

void KWPagePreview::drawContents( QPainter* p )
{
    int wid = 148;
    int hei = 210;
    int _x = ( width() - wid ) / 5;
    int _y = ( height() - hei ) / 5;

    int dl = static_cast<int>( left / 5 );
    int dr = static_cast<int>( right / 5 );
    //first+left because firstlineIndent is relative to leftIndent
    int df = static_cast<int>( (first+left) / 5 );

    int spc = static_cast<int>( spacing / 15 );

    // draw page
    p->setPen( QPen( black ) );
    p->setBrush( QBrush( black ) );

    p->drawRect( _x + 1, _y + 1, wid, hei );

    p->setBrush( QBrush( white ) );
    p->drawRect( _x, _y, wid, hei );

    // draw parags
    p->setPen( NoPen );
    p->setBrush( QBrush( lightGray ) );

    for ( int i = 1; i <= 4; i++ )
        p->drawRect( _x + 6, _y + 6 + ( i - 1 ) * 12 + 2, wid - 12 - ( ( i / 4 ) * 4 == i ? 50 : 0 ), 6 );

    p->setBrush( QBrush( darkGray ) );

    for ( int i = 5; i <= 8; i++ )
      {
	QRect rect( ( i == 5 ? df : dl ) + _x + 6, _y + 6 + ( i - 1 ) * 12 + 2 + ( i - 5 ) * spc + static_cast<int>( before / 2 ),
		    wid - 12 - ( ( i / 4 ) * 4 == i ? 50 : 0 ) - ( ( i == 12 ? 0 : dr ) + ( i == 5 ? df : dl ) ), 6);

	if(rect.width ()>=0)
	  p->drawRect( rect );
      }
    p->setBrush( QBrush( lightGray ) );

    for ( int i = 9; i <= 12; i++ )
        p->drawRect( _x + 6, _y + 6 + ( i - 1 ) * 12 + 2 + 3 * spc +
                     static_cast<int>( before / 2 ) + static_cast<int>( after / 2 ),
                     wid - 12 - ( ( i / 4 ) * 4 == i ? 50 : 0 ), 6 );

}

/******************************************************************/
/* class KWPagePreview2                                           */
/******************************************************************/

KWPagePreview2::KWPagePreview2( QWidget* parent, const char* name )
    : QGroupBox( i18n( "Preview" ), parent, name )
{
    align = Qt::AlignLeft;
}

void KWPagePreview2::drawContents( QPainter* p )
{
    int wid = 148;
    int hei = 210;
    int _x = ( width() - wid ) / 2;
    int _y = ( height() - hei ) / 2;

    // draw page
    p->setPen( QPen( black ) );
    p->setBrush( QBrush( black ) );

    p->drawRect( _x + 1, _y + 1, wid, hei );

    p->setBrush( QBrush( white ) );
    p->drawRect( _x, _y, wid, hei );

    // draw parags
    p->setPen( NoPen );
    p->setBrush( QBrush( lightGray ) );

    for ( int i = 1; i <= 4; i++ )
        p->drawRect( _x + 6, _y + 6 + ( i - 1 ) * 12 + 2, wid - 12 - ( ( i / 4 ) * 4 == i ? 50 : 0 ), 6 );

    p->setBrush( QBrush( darkGray ) );

    int __x = 0, __w = 0;
    for ( int i = 5; i <= 8; i++ ) {
        switch ( i ) {
        case 5: __w = wid - 12;
            break;
        case 6: __w = wid - 52;
            break;
        case 7: __w = wid - 33;
            break;
        case 8: __w = wid - 62;
        default: break;
        }

        switch ( align ) {
            case Qt3::AlignAuto:
            case Qt::AlignLeft:
                __x = _x + 6;
                break;
            case Qt::AlignCenter:
                __x = _x + ( wid - __w ) / 2;
                break;
            case Qt::AlignRight:
                __x = _x + ( wid - __w ) - 6;
                break;
            case Qt3::AlignJustify:
            {
                if ( i < 8 ) __w = wid - 12;
                __x = _x + 6;
            } break;
        }

        p->drawRect( __x, _y + 6 + ( i - 1 ) * 12 + 2 + ( i - 5 ), __w, 6 );
    }

    p->setBrush( QBrush( lightGray ) );

    for ( int i = 9; i <= 12; i++ )
        p->drawRect( _x + 6, _y + 6 + ( i - 1 ) * 12 + 2 + 3, wid - 12 - ( ( i / 4 ) * 4 == i ? 50 : 0 ), 6 );

}

/******************************************************************/
/* class KWBorderPreview                                          */
/******************************************************************/


KWBorderPreview::KWBorderPreview( QWidget* parent, const char* name )
    :QFrame(parent,name)
{
}

void KWBorderPreview::mousePressEvent( QMouseEvent *_ev )
{
    emit choosearea(_ev);
}


void KWBorderPreview::drawContents( QPainter* painter )
{
    QRect r = contentsRect();
    QFontMetrics fm( font() );

    painter->fillRect( r.x() + fm.width( 'W' ), r.y() + fm.height(), r.width() - 2 * fm.width( 'W' ),
                       r.height() - 2 * fm.height(), white );
    painter->setClipRect( r.x() + fm.width( 'W' ), r.y() + fm.height(), r.width() - 2 * fm.width( 'W' ),
                          r.height() - 2 * fm.height() );

    if ( m_topBorder.ptWidth > 0 ) {
        painter->setPen( setBorderPen( m_topBorder ) );
        painter->drawLine( r.x() + 20, r.y() + 20, r.right() - 20, r.y() + 20 );
    }

    if ( m_bottomBorder.ptWidth > 0 ) {
        painter->setPen( setBorderPen( m_bottomBorder ) );
        painter->drawLine( r.x() + 20, r.bottom() - 20, r.right() - 20, r.bottom() - 20 );
    }

    if ( m_leftBorder.ptWidth > 0 ) {
        painter->setPen( setBorderPen( m_leftBorder ) );
        painter->drawLine( r.x() + 20, r.y() + 20, r.x() + 20, r.bottom() - 20 );
    }

    if ( m_rightBorder.ptWidth > 0 ) {
        painter->setPen( setBorderPen( m_rightBorder ) );
        painter->drawLine( r.right() - 20, r.y() + 20, r.right() - 20, r.bottom() - 20 );
    }
}

QPen KWBorderPreview::setBorderPen( Border _brd )
{
    QPen pen( black, 1, SolidLine );

    pen.setWidth( static_cast<int>( _brd.ptWidth ) );
    pen.setColor( _brd.color );

    switch ( _brd.style ) {
    case Border::SOLID:
        pen.setStyle( SolidLine );
        break;
    case Border::DASH:
        pen.setStyle( DashLine );
        break;
    case Border::DOT:
        pen.setStyle( DotLine );
        break;
    case Border::DASH_DOT:
        pen.setStyle( DashDotLine );
        break;
    case Border::DASH_DOT_DOT:
        pen.setStyle( DashDotDotLine );
        break;
    }

    return QPen( pen );
}

/******************************************************************/
/* class KWNumPreview                                             */
/******************************************************************/

KWNumPreview::KWNumPreview( QWidget* parent, const char* name )
    : QGroupBox( i18n( "Preview" ), parent, name ) {
    setMinimumHeight(80);
    m_zoomHandler = new KoZoomHandler;
    m_textdoc = new KWTextDocument( m_zoomHandler );
    KWTextParag * parag = static_cast<KWTextParag *>(m_textdoc->firstParag());
    parag->insert( 0, i18n("Normal paragraph text") );
}

KWNumPreview::~KWNumPreview()
{
    delete m_textdoc;
    delete m_zoomHandler;
}

void KWNumPreview::setCounter( const KoParagCounter & counter )
{
    KWTextParag * parag = static_cast<KWTextParag *>(m_textdoc->firstParag());
    parag->setCounter( counter );
    repaint( true );
}

void KWNumPreview::setStyle( KWStyle * style )
{
    KWTextParag * parag = static_cast<KWTextParag *>(m_textdoc->firstParag());
    parag->setStyle( style );
    repaint(true);
}

void KWNumPreview::drawContents( QPainter* painter )
{
    // see also KWStylePreview::drawContents
    painter->save();
    QRect r = contentsRect();
    QFontMetrics fm( font() );

    // (Hmm, why use width('W') etc. here ? +/- 10 would be fine too IMHO)
    QRect textRect( r.x() + fm.width( 'W' ), r.y() + fm.height(),
                    r.width() - 2 * fm.width( 'W' ), r.height() - 2 * fm.height() );
    //kdDebug() << "KWNumPreview::drawContents textRect=" << DEBUGRECT(textRect) << endl;
    painter->fillRect( textRect, white );
    painter->setClipRect( textRect );

    KWTextParag * parag = static_cast<KWTextParag *>(m_textdoc->firstParag());
    parag->format();

    painter->translate( textRect.x(), textRect.y() );

    m_textdoc->draw( painter, 0, 0, textRect.width(), textRect.height(),
                     QApplication::palette().active() );
    painter->restore();
}


KWIndentSpacingWidget::KWIndentSpacingWidget( KWUnit::Unit unit, QWidget * parent, const char * name )
        : KWParagLayoutWidget( KWParagDia::PD_SPACING, parent, name ), m_unit( unit )
{
    QString unitName = KWUnit::unitName( m_unit );
    QGridLayout *mainGrid = new QGridLayout( this, 4, 2, KDialog::marginHint(), KDialog::spacingHint() );

    // mainGrid gives equal space to each groupbox, apparently
    // I tried setRowStretch but the result is awful (much space between them and not equal!)
    // Any other way (in order to make the 2nd, the one with a single checkbox, a bit
    // smaller than the other 3) ? (DF)

    // --------------- indent ---------------
    QGroupBox * indentFrame = new QGroupBox( i18n( "Indent" ), this );
    QGridLayout * indentGrid = new QGridLayout( indentFrame, 4, 2, KDialog::marginHint(), KDialog::spacingHint() );

    QLabel * lLeft = new QLabel( i18n("Left (%1):").arg(unitName), indentFrame );
    lLeft->setAlignment( AlignRight );
    indentGrid->addWidget( lLeft, 1, 0 );

    eLeft = new QLineEdit( indentFrame );
    eLeft->setValidator( new QDoubleValidator( eLeft ) );
    eLeft->setText( i18n("0.00") );
    eLeft->setMaxLength( 5 );
    eLeft->setEchoMode( QLineEdit::Normal );
    eLeft->setFrame( true );
    indentGrid->addWidget( eLeft, 1, 1 );
    connect( eLeft, SIGNAL( textChanged( const QString & ) ), this, SLOT( leftChanged( const QString & ) ) );

    QLabel * lRight = new QLabel( i18n("Right (%1):").arg(unitName), indentFrame );
    lRight->setAlignment( AlignRight );
    indentGrid->addWidget( lRight, 2, 0 );

    eRight = new QLineEdit( indentFrame );
    eRight->setValidator( new QDoubleValidator( eRight ) );
    eRight->setText( i18n("0.00") );
    eRight->setMaxLength( 5 );
    eRight->setEchoMode( QLineEdit::Normal );
    eRight->setFrame( true );
    indentGrid->addWidget( eRight, 2, 1 );
    connect( eRight, SIGNAL( textChanged( const QString & ) ), this, SLOT( rightChanged( const QString & ) ) );

    QLabel * lFirstLine = new QLabel( i18n("First Line (%1):").arg(unitName), indentFrame );
    lFirstLine->setAlignment( AlignRight );
    indentGrid->addWidget( lFirstLine, 3, 0 );

    eFirstLine = new QLineEdit( indentFrame );
    eFirstLine->setValidator( new QDoubleValidator( eFirstLine ) );
    eFirstLine->setText( i18n("0.00") );
    eFirstLine->setMaxLength( 5 );
    eFirstLine->setEchoMode( QLineEdit::Normal );
    eFirstLine->setFrame( true );
    connect( eFirstLine, SIGNAL( textChanged( const QString & ) ), this, SLOT( firstChanged( const QString & ) ) );
    indentGrid->addWidget( eFirstLine, 3, 1 );

    // grid row spacing
    indentGrid->addRowSpacing( 0, 12 );
    for ( int i = 1 ; i < indentGrid->numRows() ; ++i )
        indentGrid->setRowStretch( i, 1 );
    mainGrid->addWidget( indentFrame, 0, 0 );

    // --------------- End of page /frame ---------------
    QGroupBox * endFramePage = new QGroupBox( i18n( "Behavior at end of frame/page" ), this );
    QGridLayout * endFramePageGrid = new QGridLayout( endFramePage, 4, 1,
                                                      KDialog::marginHint(), KDialog::spacingHint() );

    cKeepLinesTogether = new QCheckBox( i18n("Keep lines together"),endFramePage);
    endFramePageGrid->addWidget( cKeepLinesTogether, 1, 0 );
    cHardBreakBefore = new QCheckBox( i18n("Insert Break Before Paragraph"),endFramePage);
    endFramePageGrid->addWidget( cHardBreakBefore, 2, 0 );
    cHardBreakAfter = new QCheckBox( i18n("Insert Break After Paragraph"),endFramePage);
    endFramePageGrid->addWidget( cHardBreakAfter, 3, 0 );

    endFramePageGrid->addRowSpacing( 0, 12 ); // groupbox title
    for ( int i = 0 ; i < endFramePageGrid->numRows()-1 ; ++i )
        endFramePageGrid->setRowStretch( 0, 0 );
    endFramePageGrid->setRowStretch( endFramePageGrid->numRows()-1, 1 );
    mainGrid->addWidget( endFramePage, 2, 0 );


    // --------------- line spacing ---------------
    QGroupBox * spacingFrame = new QGroupBox( i18n( "Line Spacing" ), this );
    QGridLayout * spacingGrid = new QGridLayout( spacingFrame, 2, 1,
                                                 KDialog::marginHint(), KDialog::spacingHint() );

    cSpacing = new QComboBox( false, spacingFrame, "" );
    cSpacing->insertItem( i18n( "Line spacing value", "Single" ) );
    cSpacing->insertItem( i18n( "Line spacing value", "1.5 lines" ) );
    cSpacing->insertItem( i18n( "Line spacing value", "Double" ) );
    cSpacing->insertItem( i18n( "Custom (%1)" ).arg(unitName) );
    connect( cSpacing, SIGNAL( activated( int ) ), this, SLOT( spacingActivated( int ) ) );
    spacingGrid->addWidget( cSpacing, 1, 0 );

    eSpacing = new QLineEdit( spacingFrame );
    eSpacing->setValidator( new QDoubleValidator( eSpacing ) );
    eSpacing->setText( i18n("0") );
    eSpacing->setMaxLength( 2 );
    eSpacing->setEchoMode( QLineEdit::Normal );
    eSpacing->setFrame( true );
    eSpacing->setEnabled(false);
    connect( eSpacing, SIGNAL( textChanged( const QString & ) ), this, SLOT( spacingChanged( const QString & ) ) );
    spacingGrid->addWidget( eSpacing, 1, 1 );

    // grid row spacing
    spacingGrid->addRowSpacing( 0, 12 );
    for ( int i = 1 ; i < spacingGrid->numRows() ; ++i )
        spacingGrid->setRowStretch( i, 1 );
    mainGrid->addWidget( spacingFrame, 4, 0 );

    eSpacing->setEnabled( true );

    // --------------- paragraph spacing ---------------
    QGroupBox * pSpaceFrame = new QGroupBox( i18n( "Paragraph Space" ), this );
    QGridLayout * pSpaceGrid = new QGridLayout( pSpaceFrame, 3, 2,
                                                KDialog::marginHint(), KDialog::spacingHint() );

    QLabel * lBefore = new QLabel( i18n("Before (%1):").arg(unitName), pSpaceFrame );
    lBefore->setAlignment( AlignRight );
    pSpaceGrid->addWidget( lBefore, 1, 0 );

    eBefore = new QLineEdit( pSpaceFrame );
    eBefore->setValidator( new QDoubleValidator( eBefore ) );
    eBefore->setText( i18n("0.00") );
    eBefore->setMaxLength( 5 );
    eBefore->setEchoMode( QLineEdit::Normal );
    eBefore->setFrame( true );
    connect( eBefore, SIGNAL( textChanged( const QString & ) ), this, SLOT( beforeChanged( const QString & ) ) );
    pSpaceGrid->addWidget( eBefore, 1, 1 );

    QLabel * lAfter = new QLabel( i18n("After (%1):").arg(unitName), pSpaceFrame );
    lAfter->setAlignment( AlignRight );
    pSpaceGrid->addWidget( lAfter, 2, 0 );

    eAfter = new QLineEdit( pSpaceFrame );
    eAfter->setValidator( new QDoubleValidator( eAfter ) );
    eAfter->setText( i18n("0.00") );
    eAfter->setMaxLength( 5 );
    eAfter->setEchoMode( QLineEdit::Normal );
    eAfter->setFrame( true );
    connect( eAfter, SIGNAL( textChanged( const QString & ) ), this, SLOT( afterChanged( const QString & ) ) );
    pSpaceGrid->addWidget( eAfter, 2, 1 );

    // grid row spacing
    pSpaceGrid->addRowSpacing( 0, 12 );
    for ( int i = 1 ; i < pSpaceGrid->numRows() ; ++i )
        pSpaceGrid->setRowStretch( i, 1 );
    mainGrid->addWidget( pSpaceFrame, 6, 0 );

    // --------------- preview --------------------
    prev1 = new KWPagePreview( this );
    mainGrid->addMultiCellWidget( prev1, 0, mainGrid->numRows()-1, 1, 1 );

    mainGrid->setColStretch( 1, 1 );
    //mainGrid->setRowStretch( 4, 1 );
}

double KWIndentSpacingWidget::leftIndent() const
{
    return KWUnit::fromUserValue( QMAX(eLeft->text().toDouble(),0), m_unit );
}

double KWIndentSpacingWidget::rightIndent() const
{
    return KWUnit::fromUserValue( QMAX(eRight->text().toDouble(),0), m_unit );
}

double KWIndentSpacingWidget::firstLineIndent() const
{
    return KWUnit::fromUserValue( eFirstLine->text().toDouble(), m_unit );
}

double KWIndentSpacingWidget::spaceBeforeParag() const
{
    return KWUnit::fromUserValue( QMAX(eBefore->text().toDouble(),0), m_unit );
}

double KWIndentSpacingWidget::spaceAfterParag() const
{
    return KWUnit::fromUserValue( QMAX(eAfter->text().toDouble(),0), m_unit );
}

double KWIndentSpacingWidget::lineSpacing() const
{
    int index = cSpacing->currentItem();
    switch ( index ) {
    case 0: // single
        return 0;
    case 1: // one-and-half
        return KWParagLayout::LS_ONEANDHALF;
    case 2:
        return KWParagLayout::LS_DOUBLE;
    case 3:
    default:
        return KWUnit::fromUserValue( QMAX(eSpacing->text().toDouble(),0), m_unit );
    }
}

int KWIndentSpacingWidget::pageBreaking() const
{
    int pb = 0;
    if ( cKeepLinesTogether->isChecked() )
        pb |= KWParagLayout::KeepLinesTogether;
    if ( cHardBreakBefore->isChecked() )
        pb |= KWParagLayout::HardFrameBreakBefore;
    if ( cHardBreakAfter->isChecked() )
        pb |= KWParagLayout::HardFrameBreakAfter;
    return pb;
}

void KWIndentSpacingWidget::display( const KWParagLayout & lay )
{
    double _left = lay.margins[QStyleSheetItem::MarginLeft];
    QString str = QString::number( KWUnit::userValue( _left, m_unit ) );
    eLeft->setText( str );
    prev1->setLeft( _left );

    double _right = lay.margins[QStyleSheetItem::MarginRight];
    str = QString::number( KWUnit::userValue( _right, m_unit )  );
    eRight->setText( str );
    prev1->setRight( _right );

    double _first = lay.margins[QStyleSheetItem::MarginFirstLine];
    str = QString::number( KWUnit::userValue( _first, m_unit )  );
    eFirstLine->setText( str );
    prev1->setFirst( _first  );

    double _before = lay.margins[QStyleSheetItem::MarginTop];
    str = QString::number( KWUnit::userValue( _before, m_unit ) );
    eBefore->setText( str );
    prev1->setBefore( _before );

    double _after = lay.margins[QStyleSheetItem::MarginBottom];
    str = QString::number( KWUnit::userValue( _after, m_unit ) );
    eAfter->setText( str );
    prev1->setAfter( _after );

    double _spacing = lay.lineSpacing;
    str = QString::null;
    eSpacing->setEnabled(false);
    if ( _spacing == 0 )
        cSpacing->setCurrentItem( 0 );
    else if ( _spacing == KWParagLayout::LS_ONEANDHALF )
        cSpacing->setCurrentItem( 1 );
    else if ( _spacing == KWParagLayout::LS_DOUBLE )
        cSpacing->setCurrentItem( 2 );
    else
    {
        cSpacing->setCurrentItem( 3 );
        eSpacing->setEnabled(true);
        str = QString::number( KWUnit::userValue( _spacing, m_unit ) );
    }
    eSpacing->setText( str );
    prev1->setSpacing( _spacing );

    cKeepLinesTogether->setChecked( lay.pageBreaking & KWParagLayout::KeepLinesTogether );
    cHardBreakBefore->setChecked( lay.pageBreaking & KWParagLayout::HardFrameBreakBefore );
    cHardBreakAfter->setChecked( lay.pageBreaking & KWParagLayout::HardFrameBreakAfter );
    // ## preview support for end-of-frame ?
}

void KWIndentSpacingWidget::save( KWParagLayout & lay )
{
    lay.lineSpacing = lineSpacing();
    lay.margins[QStyleSheetItem::MarginLeft] = leftIndent();
    lay.margins[QStyleSheetItem::MarginRight] = rightIndent();
    lay.margins[QStyleSheetItem::MarginFirstLine] = firstLineIndent();
    lay.margins[QStyleSheetItem::MarginTop] = spaceBeforeParag();
    lay.margins[QStyleSheetItem::MarginBottom] = spaceAfterParag();
    lay.pageBreaking = pageBreaking();
}

QString KWIndentSpacingWidget::tabName()
{
    return i18n( "Indent and Spacing" );
}

void KWIndentSpacingWidget::leftChanged( const QString & _text )
{
    prev1->setLeft( _text.toDouble() );
}

void KWIndentSpacingWidget::rightChanged( const QString & _text )
{
    prev1->setRight( _text.toDouble() );
}

void KWIndentSpacingWidget::firstChanged( const QString & _text )
{
    prev1->setFirst( _text.toDouble() );
}

void KWIndentSpacingWidget::spacingActivated( int _index )
{
    if ( _index == cSpacing->count()-1 /* last item */ ) {
        eSpacing->setEnabled( true );
        eSpacing->setFocus();
        prev1->setSpacing( eSpacing->text().toDouble() );
    } else {
        eSpacing->setEnabled( false );
        // 1 -> oneandhalf (8)
        // 2 -> double (16)
        prev1->setSpacing( _index == 1 ? 8 : _index == 2 ? 16 : 0 );
    }
}

void KWIndentSpacingWidget::spacingChanged( const QString & _text )
{
    prev1->setSpacing( _text.toDouble() );
}

void KWIndentSpacingWidget::beforeChanged( const QString & _text )
{
    prev1->setBefore( _text.toDouble() );
}

void KWIndentSpacingWidget::afterChanged( const QString & _text )
{
    prev1->setAfter( _text.toDouble() );
}

KWParagAlignWidget::KWParagAlignWidget( QWidget * parent, const char * name )
        : KWParagLayoutWidget( KWParagDia::PD_ALIGN, parent, name )
{
    QGridLayout *grid = new QGridLayout( this, 6, 2, KDialog::marginHint(), KDialog::spacingHint() );

    QLabel * lAlign = new QLabel( i18n( "Align:" ), this );
    grid->addWidget( lAlign, 0, 0 );

    rLeft = new QRadioButton( i18n( "Left" ), this );
    grid->addWidget( rLeft, 1, 0 );
    connect( rLeft, SIGNAL( clicked() ), this, SLOT( alignLeft() ) );

    rCenter = new QRadioButton( i18n( "Center" ), this );
    grid->addWidget( rCenter, 2, 0 );
    connect( rCenter, SIGNAL( clicked() ), this, SLOT( alignCenter() ) );

    rRight = new QRadioButton( i18n( "Right" ), this );
    grid->addWidget( rRight, 3, 0 );
    connect( rRight, SIGNAL( clicked() ), this, SLOT( alignRight() ) );

    rJustify = new QRadioButton( i18n( "Justify" ), this );
    grid->addWidget( rJustify, 4, 0 );
    connect( rJustify, SIGNAL( clicked() ), this, SLOT( alignJustify() ) );

    clearAligns();
    rLeft->setChecked( true );

    // --------------- preview --------------------
    prev2 = new KWPagePreview2( this );
    grid->addMultiCellWidget( prev2, 0, 5, 1, 1 );

    // --------------- main grid ------------------
    grid->setColStretch( 1, 1 );
    grid->setRowStretch( 5, 1 );
}

void KWParagAlignWidget::display( const KWParagLayout & lay )
{
    int align = lay.alignment;
    prev2->setAlign( align );

    clearAligns();
    switch ( align ) {
        case Qt3::AlignAuto: // see KWView::setAlign
        case Qt::AlignLeft:
            rLeft->setChecked( true );
            break;
        case Qt::AlignCenter:
            rCenter->setChecked( true );
            break;
        case Qt::AlignRight:
            rRight->setChecked( true );
            break;
        case Qt3::AlignJustify:
            rJustify->setChecked( true );
            break;
    }
}

void KWParagAlignWidget::save( KWParagLayout & lay )
{
    lay.alignment = align();
}

int KWParagAlignWidget::align() const
{
    if ( rLeft->isChecked() ) return Qt::AlignLeft;
    else if ( rCenter->isChecked() ) return Qt::AlignCenter;
    else if ( rRight->isChecked() ) return Qt::AlignRight;
    else if ( rJustify->isChecked() ) return Qt3::AlignJustify;

    return Qt::AlignLeft;
}

QString KWParagAlignWidget::tabName()
{
    return i18n( "Aligns" );
}

void KWParagAlignWidget::alignLeft()
{
    prev2->setAlign( Qt::AlignLeft );
    clearAligns();
    rLeft->setChecked( true );
}

void KWParagAlignWidget::alignCenter()
{
    prev2->setAlign( Qt::AlignCenter );
    clearAligns();
    rCenter->setChecked( true );
}

void KWParagAlignWidget::alignRight()
{
    prev2->setAlign( Qt::AlignRight );
    clearAligns();
    rRight->setChecked( true );
}

void KWParagAlignWidget::alignJustify()
{
    prev2->setAlign( Qt3::AlignJustify );
    clearAligns();
    rJustify->setChecked( true );
}

void KWParagAlignWidget::clearAligns()
{
    rLeft->setChecked( false );
    rCenter->setChecked( false );
    rRight->setChecked( false );
    rJustify->setChecked( false );
}

KWParagBorderWidget::KWParagBorderWidget( QWidget * parent, const char * name )
    : KWParagLayoutWidget( KWParagDia::PD_BORDERS, parent, name )
{
    QGridLayout *grid = new QGridLayout( this, 8, 2, KDialog::marginHint(), KDialog::spacingHint() );

    QLabel * lStyle = new QLabel( i18n( "Style:" ), this );
    grid->addWidget( lStyle, 0, 0 );

    cStyle = new QComboBox( false, this );
    cStyle->insertItem( Border::getStyle( Border::SOLID ) );
    cStyle->insertItem( Border::getStyle( Border::DASH ) );
    cStyle->insertItem( Border::getStyle( Border::DOT ) );
    cStyle->insertItem( Border::getStyle( Border::DASH_DOT ) );
    cStyle->insertItem( Border::getStyle( Border::DASH_DOT_DOT ) );
    grid->addWidget( cStyle, 1, 0 );
    //connect( cStyle, SIGNAL( activated( const QString & ) ), this, SLOT( brdStyleChanged( const QString & ) ) );

    QLabel * lWidth = new QLabel( i18n( "Width:" ), this );
    grid->addWidget( lWidth, 2, 0 );

    cWidth = new QComboBox( false, this );
    for( unsigned int i = 1; i <= 10; i++ )
        cWidth->insertItem(QString::number(i));
    grid->addWidget( cWidth, 3, 0 );
    //connect( cWidth, SIGNAL( activated( const QString & ) ), this, SLOT( brdWidthChanged( const QString & ) ) );

    QLabel * lColor = new QLabel( i18n( "Color:" ), this );
    grid->addWidget( lColor, 4, 0 );

    bColor = new KColorButton( this );
    grid->addWidget( bColor, 5, 0 );
    //connect( bColor, SIGNAL( changed( const QColor& ) ), this, SLOT( brdColorChanged( const QColor& ) ) );

    QButtonGroup * bb = new QHButtonGroup( this );
    bb->setFrameStyle(QFrame::NoFrame);
    bLeft = new QPushButton(bb);
    bLeft->setPixmap( KWBarIcon( "borderleft" ) );
    bLeft->setToggleButton( true );
    bRight = new QPushButton(bb);
    bRight->setPixmap( KWBarIcon( "borderright" ) );
    bRight->setToggleButton( true );
    bTop = new QPushButton(bb);
    bTop->setPixmap( KWBarIcon( "bordertop" ) );
    bTop->setToggleButton( true );
    bBottom = new QPushButton(bb);
    bBottom->setPixmap( KWBarIcon( "borderbottom" ) );
    bBottom->setToggleButton( true );
    grid->addWidget( bb, 6, 0 );

    connect( bLeft, SIGNAL( toggled( bool ) ), this, SLOT( brdLeftToggled( bool ) ) );
    connect( bRight, SIGNAL( toggled( bool ) ), this, SLOT( brdRightToggled( bool ) ) );
    connect( bTop, SIGNAL( toggled( bool ) ), this, SLOT( brdTopToggled( bool ) ) );
    connect( bBottom, SIGNAL( toggled( bool ) ), this, SLOT( brdBottomToggled( bool ) ) );

    QGroupBox *grp=new QGroupBox( i18n( "Preview" ), this );
    grid->addMultiCellWidget( grp , 0, 7, 1, 1 );
    prev3 = new KWBorderPreview( grp );
    QVBoxLayout *lay1 = new QVBoxLayout( grp );
    lay1->setMargin( 15 );
    lay1->setSpacing( 1 );
    lay1->addWidget(prev3);

    connect( prev3, SIGNAL( choosearea(QMouseEvent * ) ),
             this, SLOT( slotPressEvent(QMouseEvent *) ) );

    grid->setRowStretch( 7, 1 );
    grid->setColStretch( 1, 1 );
}

void KWParagBorderWidget::display( const KWParagLayout & lay )
{
    m_leftBorder = lay.leftBorder;
    m_rightBorder = lay.rightBorder;
    m_topBorder = lay.topBorder;
    m_bottomBorder = lay.bottomBorder;
    bLeft->blockSignals( true );
    bRight->blockSignals( true );
    bTop->blockSignals( true );
    bBottom->blockSignals( true );
    updateBorders();
    bLeft->blockSignals( false );
    bRight->blockSignals( false );
    bTop->blockSignals( false );
    bBottom->blockSignals( false );
}

void KWParagBorderWidget::save( KWParagLayout & lay )
{
    lay.leftBorder = m_leftBorder;
    lay.rightBorder = m_rightBorder;
    lay.topBorder = m_topBorder;
    lay.bottomBorder = m_bottomBorder;
}

#define OFFSETX 15
#define OFFSETY 7
#define KW_SPACE 30
void KWParagBorderWidget::slotPressEvent(QMouseEvent *_ev)
{
    QRect r = prev3->contentsRect();
    QRect rect(r.x()+OFFSETX,r.y()+OFFSETY,r.width()-OFFSETX,r.y()+OFFSETY+KW_SPACE);
    if(rect.contains(QPoint(_ev->x(),_ev->y())))
    {
        if( (  ((int)m_topBorder.ptWidth != cWidth->currentText().toInt()) ||(m_topBorder.color != bColor->color() )
               ||(m_topBorder.style!=Border::getStyle(cStyle->currentText()) )) && bTop->isOn() )
        {
            m_topBorder.ptWidth = cWidth->currentText().toInt();
            m_topBorder.color = QColor( bColor->color() );
            m_topBorder.style=Border::getStyle(cStyle->currentText());
            prev3->setTopBorder( m_topBorder );
        }
        else
            bTop->setOn(!bTop->isOn());
    }
    rect.setCoords(r.x()+OFFSETX,r.height()-OFFSETY-KW_SPACE,r.width()-OFFSETX,r.height()-OFFSETY);
    if(rect.contains(QPoint(_ev->x(),_ev->y())))
    {
        if( (  ((int)m_bottomBorder.ptWidth != cWidth->currentText().toInt()) ||(m_bottomBorder.color != bColor->color() )
               ||(m_bottomBorder.style!=Border::getStyle(cStyle->currentText()) )) && bBottom->isOn() )
        {
            m_bottomBorder.ptWidth = cWidth->currentText().toInt();
            m_bottomBorder.color = QColor( bColor->color() );
            m_bottomBorder.style=Border::getStyle(cStyle->currentText());
            prev3->setBottomBorder( m_bottomBorder );
        }
        else
            bBottom->setOn(!bBottom->isOn());
    }

    rect.setCoords(r.x()+OFFSETX,r.y()+OFFSETY,r.x()+KW_SPACE+OFFSETX,r.height()-OFFSETY);
    if(rect.contains(QPoint(_ev->x(),_ev->y())))
    {

        if( (  ((int)m_leftBorder.ptWidth != cWidth->currentText().toInt()) ||(m_leftBorder.color != bColor->color() )
               ||(m_leftBorder.style!=Border::getStyle(cStyle->currentText()) )) && bLeft->isOn() )
        {
            m_leftBorder.ptWidth = cWidth->currentText().toInt();
            m_leftBorder.color = QColor( bColor->color() );
            m_leftBorder.style=Border::getStyle(cStyle->currentText());
            prev3->setLeftBorder( m_leftBorder );
        }
        else
            bLeft->setOn(!bLeft->isOn());
    }
    rect.setCoords(r.width()-OFFSETX-KW_SPACE,r.y()+OFFSETY,r.width()-OFFSETX,r.height()-OFFSETY);
    if(rect.contains(QPoint(_ev->x(),_ev->y())))
    {

        if( (  ((int)m_rightBorder.ptWidth != cWidth->currentText().toInt()) ||(m_rightBorder.color != bColor->color() )
               ||(m_rightBorder.style!=Border::getStyle(cStyle->currentText()) )) && bRight->isOn() )
        {
            m_rightBorder.ptWidth = cWidth->currentText().toInt();
            m_rightBorder.color = bColor->color();
            m_rightBorder.style=Border::getStyle(cStyle->currentText());
            prev3->setRightBorder( m_rightBorder );
        }
        else
            bRight->setOn(!bRight->isOn());
    }
}
#undef OFFSETX
#undef OFFSETY
#undef KW_SPACE

void KWParagBorderWidget::updateBorders()
{
    bLeft->setOn( m_leftBorder.ptWidth > 0 );
    bRight->setOn( m_rightBorder.ptWidth > 0 );
    bTop->setOn( m_topBorder.ptWidth > 0 );
    bBottom->setOn( m_bottomBorder.ptWidth > 0 );
    prev3->setLeftBorder( m_leftBorder );
    prev3->setRightBorder( m_rightBorder );
    prev3->setTopBorder( m_topBorder );
    prev3->setBottomBorder( m_bottomBorder );
}

void KWParagBorderWidget::brdLeftToggled( bool _on )
{
    if ( !_on )
        m_leftBorder.ptWidth = 0;
    else {
        m_leftBorder.ptWidth = cWidth->currentText().toInt();
        m_leftBorder.color = bColor->color();
        m_leftBorder.style= Border::getStyle( cStyle->currentText() );
    }
    prev3->setLeftBorder( m_leftBorder );
}

void KWParagBorderWidget::brdRightToggled( bool _on )
{
    if ( !_on )
        m_rightBorder.ptWidth = 0;
    else {
        m_rightBorder.ptWidth = cWidth->currentText().toInt();
        m_rightBorder.color = bColor->color();
        m_rightBorder.style= Border::getStyle( cStyle->currentText() );
    }
    prev3->setRightBorder( m_rightBorder );
}

void KWParagBorderWidget::brdTopToggled( bool _on )
{
    if ( !_on )
        m_topBorder.ptWidth = 0;
    else {
        m_topBorder.ptWidth = cWidth->currentText().toInt();
        m_topBorder.color = bColor->color();
        m_topBorder.style= Border::getStyle( cStyle->currentText() );
    }
    prev3->setTopBorder( m_topBorder );
}

void KWParagBorderWidget::brdBottomToggled( bool _on )
{
    if ( !_on )
        m_bottomBorder.ptWidth = 0;
    else {
        m_bottomBorder.ptWidth = cWidth->currentText().toInt();
        m_bottomBorder.color = bColor->color();
        m_bottomBorder.style=Border::getStyle(cStyle->currentText());
    }
    prev3->setBottomBorder( m_bottomBorder );
}

QString KWParagBorderWidget::tabName()
{
    return i18n( "Borders" );
}


KWParagCounterWidget::KWParagCounterWidget( QWidget * parent, const char * name )
    : KWParagLayoutWidget( KWParagDia::PD_NUMBERING, parent, name ), stylesList()
{

    QVBoxLayout *Form1Layout = new QVBoxLayout( this );
    Form1Layout->setSpacing( 6 );
    Form1Layout->setMargin( 11 );

    gNumbering = new QButtonGroup( this, "numberingGroup" );
    gNumbering->setTitle( i18n( "Numbering" ) );
    gNumbering->setColumnLayout(0, Qt::Vertical );
    gNumbering->layout()->setSpacing( 0 );
    gNumbering->layout()->setMargin( 0 );
    QHBoxLayout *numberingGroupLayout = new QHBoxLayout( gNumbering->layout() );
    numberingGroupLayout->setAlignment( Qt::AlignTop );
    numberingGroupLayout->setSpacing( 6 );
    numberingGroupLayout->setMargin( 11 );

    // What type of numbering is required?
    QRadioButton *rNone = new QRadioButton( gNumbering, "rNone" );
    rNone->setText( i18n( "&None" ) );
    numberingGroupLayout->addWidget( rNone );

    gNumbering->insert( rNone , KoParagCounter::NUM_NONE);

    QRadioButton *rList = new QRadioButton( gNumbering, "rList" );
    rList->setText( i18n( "List" ) );
    gNumbering->insert( rList , KoParagCounter::NUM_LIST);
    numberingGroupLayout->addWidget( rList );

    QRadioButton *rChapter = new QRadioButton( gNumbering, "rChapter" );
    rChapter->setText( i18n( "Chapter" ) );
    gNumbering->insert( rChapter , KoParagCounter::NUM_CHAPTER);
    numberingGroupLayout->addWidget( rChapter );
    Form1Layout->addWidget( gNumbering );
    connect( gNumbering, SIGNAL( clicked( int ) ), this, SLOT( numTypeChanged( int ) ) );

    gStyle = new QGroupBox( this, "styleLayout" );
    gStyle->setTitle( i18n( "Style" ) );
    gStyle->setColumnLayout(0, Qt::Vertical );
    gStyle->layout()->setSpacing( 0 );
    gStyle->layout()->setMargin( 0 );

    QHBoxLayout *layout8 = new QHBoxLayout( gStyle->layout() );
    layout8->setSpacing( 6 );
    layout8->setMargin( 11 );

    stylesList.append( new StyleRepresenter(i18n( "Arabic Numbers" )
            ,  KoParagCounter::STYLE_NUM));
    stylesList.append( new StyleRepresenter(i18n( "Lower Alphabetical" )
            ,  KoParagCounter::STYLE_ALPHAB_L ));
    stylesList.append( new StyleRepresenter(i18n( "Upper Alphabetical" )
            ,  KoParagCounter::STYLE_ALPHAB_U ));
    stylesList.append( new StyleRepresenter(i18n( "Lower Roman Numbers" )
            ,  KoParagCounter::STYLE_ROM_NUM_L ));
    stylesList.append( new StyleRepresenter(i18n( "Upper Roman Numbers" )
            ,  KoParagCounter::STYLE_ROM_NUM_U ));
    stylesList.append( new StyleRepresenter(i18n( "Disc Bullet" )
            ,  KoParagCounter::STYLE_DISCBULLET , true));
    stylesList.append( new StyleRepresenter(i18n( "Square Bullet" )
            ,  KoParagCounter::STYLE_SQUAREBULLET , true));
    stylesList.append( new StyleRepresenter(i18n( "Circle Bullet" )
            ,  KoParagCounter::STYLE_CIRCLEBULLET , true));
    stylesList.append( new StyleRepresenter(i18n( "Custom Bullet" )
            ,  KoParagCounter::STYLE_CUSTOMBULLET , true));
    stylesList.append( new StyleRepresenter(i18n( "None" ) , KoParagCounter::STYLE_NONE));

    lstStyle = new QListBox( gStyle, "styleListBox" );
    fillStyleCombo();
    layout8->addWidget( lstStyle );
    layout8->activate();
    connect( lstStyle, SIGNAL( selectionChanged() ), this, SLOT( numStyleChanged() ) );

    QVBoxLayout *styleLayoutLayout = new QVBoxLayout(layout8 );
    styleLayoutLayout->setAlignment( Qt::AlignTop );
    styleLayoutLayout->setSpacing( 6 );
    styleLayoutLayout->setMargin( 11 );

    QHBoxLayout *Layout2 = new QHBoxLayout;
    Layout2->setSpacing( 6 );
    Layout2->setMargin( 0 );

    QLabel *lPrefix = new QLabel( gStyle, "lPrefix" );
    lPrefix->setText( i18n( "Prefix Text" ) );
    Layout2->addWidget( lPrefix );

    sPrefix = new QLineEdit( gStyle, "sPrefix" );
    Layout2->addWidget( sPrefix );

    QLabel *lSuffix = new QLabel( gStyle, "lSuffix" );
    lSuffix->setText( i18n( "Suffix Text" ) );
    Layout2->addWidget( lSuffix );

    sSuffix = new QLineEdit( gStyle, "sSuffix" );
    Layout2->addWidget( sSuffix );
    styleLayoutLayout->addLayout( Layout2 );

    QGridLayout *Layout7 = new QGridLayout;
    Layout7->setSpacing( 6 );
    Layout7->setMargin( 0 );

    bCustom = new QPushButton( gStyle, "bCustom" );
    bCustom->setText("");

    Layout7->addWidget( bCustom, 2, 1 );
    connect( bCustom, SIGNAL( clicked() ), this, SLOT( selectCustomBullet() ) );

    lStart = new QLabel( gStyle, "lStart" );
    lStart->setText( i18n( "Start at" ) );

    Layout7->addWidget( lStart, 0, 0 );

    spnDepth = new QSpinBox( 0, 15, 1, gStyle );

    Layout7->addWidget( spnDepth, 1, 1 );

    lCustom = new QLabel( gStyle, "lCustom" );
    lCustom->setText( i18n( "Custom character" ) );

    Layout7->addWidget( lCustom, 2, 0 );

    spnStart = new KWSpinBox( gStyle );
    spnStart->setMinValue ( 1);

    Layout7->addWidget( spnStart, 0, 1 );
    QSpacerItem* spacer = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    Layout7->addItem( spacer, 1, 2 );

    QLabel *lDepth = new QLabel( gStyle, "lDepth" );
    lDepth->setText( i18n( "Depth" ) );

    Layout7->addWidget( lDepth, 1, 0 );
    styleLayoutLayout->addLayout( Layout7 );
    styleLayoutLayout->addItem(new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding ));
    Form1Layout->addWidget( gStyle );

    preview = new KWNumPreview( this );
    Form1Layout->addWidget( preview );

    connect( sSuffix, SIGNAL( textChanged (const QString &) ), this, SLOT( suffixChanged(const QString &) ) );
    connect( sPrefix, SIGNAL( textChanged (const QString &) ), this, SLOT( prefixChanged(const QString &) ) );
    connect( spnStart, SIGNAL( valueChanged (int) ), this, SLOT( startChanged(int) ) );
    connect( spnDepth, SIGNAL( valueChanged (int) ), this, SLOT( depthChanged(int) ) );
}

void KWParagCounterWidget::fillStyleCombo(KoParagCounter::Numbering type) {
    if(lstStyle==NULL) return;
    noSignals=true;
    unsigned int cur = lstStyle->currentItem();

    lstStyle->clear();

    QListIterator<StyleRepresenter> style( stylesList );
    while ( style.current() ) {
        if(style.current()->style() == KoParagCounter::STYLE_NONE) {
            if(type == KoParagCounter::NUM_NONE)
                lstStyle->insertItem( style.current()->name() );
        } else if(type == KoParagCounter::NUM_LIST || !style.current()->listStyle())
            if(type != KoParagCounter::NUM_NONE)
                lstStyle->insertItem( style.current()->name() );
        ++style;
    }

    if(styleBuffer <= lstStyle->count())
        lstStyle->setCurrentItem(styleBuffer);
    else
        if(cur <= lstStyle->count())
            lstStyle->setCurrentItem(cur);

    if(cur > lstStyle->count()) {
        styleBuffer = cur;
    }
    noSignals=false;
}

QString KWParagCounterWidget::tabName() {
    return i18n( "Bullets/Numbers" );
}

void KWParagCounterWidget::selectCustomBullet() {
    unsigned int i;
    for (i=0; stylesList.count() > i && stylesList.at(i)->style() != KoParagCounter::STYLE_CUSTOMBULLET; i++);
    lstStyle->setCurrentItem(i);

    QString f = m_counter.customBulletFont();
    if ( f.isEmpty() )
        f = "symbol";
    QChar c = m_counter.customBulletCharacter();

    if ( KCharSelectDia::selectChar( f, c ) ) {
        m_counter.setCustomBulletFont( f );
        m_counter.setCustomBulletCharacter( c );
        bCustom->setText( c );
        if ( !f.isEmpty() )
            bCustom->setFont( QFont( m_counter.customBulletFont() ) );
        preview->setCounter( m_counter );
    }
}

void KWParagCounterWidget::numStyleChanged() {
    // We selected another style from the list box.
    styleBuffer = 999;
    StyleRepresenter *sr = stylesList.at(lstStyle->currentItem());
    m_counter.setStyle(sr->style());

    bool hasStart = !sr->listStyle() && !sr->style() == KoParagCounter::STYLE_NONE;
    lStart->setEnabled( hasStart );
    spnStart->setEnabled( hasStart );
    changeKWSpinboxType();
    updatePreview();
}

void KWParagCounterWidget::changeKWSpinboxType() {
    switch(m_counter.style())
    {
        case KoParagCounter::STYLE_NONE:
            spnStart->setCounterType(KWSpinBox::NONE);
            break;
        case KoParagCounter::STYLE_NUM:
            spnStart->setCounterType(KWSpinBox::NUM);
            break;
        case KoParagCounter::STYLE_ALPHAB_L:
            spnStart->setCounterType(KWSpinBox::ALPHAB_L);
            break;
        case KoParagCounter::STYLE_ALPHAB_U:
            spnStart->setCounterType(KWSpinBox::ALPHAB_U);
            break;
        case KoParagCounter::STYLE_ROM_NUM_L:
            spnStart->setCounterType(KWSpinBox::ROM_NUM_L);
            break;
        case KoParagCounter::STYLE_ROM_NUM_U:
            spnStart->setCounterType(KWSpinBox::ROM_NUM_U);
            break;
        default:
            spnStart->setCounterType(KWSpinBox::NONE);
    }
}

void KWParagCounterWidget::numTypeChanged( int nType ) {
    // radio buttons pressed to change numbering type
    m_counter.setNumbering( static_cast<KoParagCounter::Numbering>( nType ) );

    preview->setEnabled( m_counter.numbering() != KoParagCounter::NUM_NONE );
    gStyle->setEnabled( m_counter.numbering() != KoParagCounter::NUM_NONE );

    fillStyleCombo(m_counter.numbering());
    bool state=m_counter.numbering()==KoParagCounter::NUM_LIST;
    bCustom->setEnabled(state);
    lCustom->setEnabled(state);
}

void KWParagCounterWidget::display( const KWParagLayout & lay ) {
    KoParagCounter::Style style =KoParagCounter::STYLE_NONE;
    if ( lay.counter )
    {
        style=lay.counter->style();
        m_counter = *lay.counter;
    }
    else
    {
        m_counter = KoParagCounter();
    }
    preview->setCounter( m_counter );
    preview->setStyle(lay.style);
    styleBuffer = 999;

    gNumbering->setButton( m_counter.numbering() );
    numTypeChanged( m_counter.numbering() );

    unsigned int i;

    for (i=0; stylesList.count() > i && stylesList.at(i)->style() != style/*m_counter.style()*/; i++);
    lstStyle->setCurrentItem(i);
    bCustom->setText( m_counter.customBulletCharacter() );
    if ( !m_counter.customBulletFont().isEmpty() )
        bCustom->setFont( QFont( m_counter.customBulletFont() ) );

    sPrefix->setText( m_counter.prefix() );
    sSuffix->setText( m_counter.suffix() );

    spnDepth->setValue( m_counter.depth() );
    spnStart->setValue( m_counter.startNumber() );
}

void KWParagCounterWidget::updatePreview() {
    preview->setCounter(m_counter);
    preview->repaint(true);
}

void KWParagCounterWidget::save( KWParagLayout & lay ) {
/*    m_counter.setDepth(spnDepth->value());
    m_counter.setStartNumber(spnStart->value());
    m_counter.setPrefix(sPrefix->text());
    m_counter.setSuffix(sSuffix->text()); */

    if ( lay.counter )
        *lay.counter = m_counter;
    else
        lay.counter = new KoParagCounter( m_counter );
}

KWParagTabulatorsWidget::KWParagTabulatorsWidget( KWUnit::Unit unit, QWidget * parent, const char * name )
    : KWParagLayoutWidget( KWParagDia::PD_TABS, parent, name ), m_unit(unit)
{
    QGridLayout *grid = new QGridLayout( this, 4, 2, KDialog::marginHint(), KDialog::spacingHint() );

    lTab = new QLabel(  this );
    grid->addWidget( lTab, 0, 0 );

    eTabPos = new QLineEdit( this );

    eTabPos->setValidator( new QDoubleValidator( eTabPos ) );
    grid->addWidget( eTabPos, 1, 0 );

    // ## move to KWUnit
    QString unitText;
    switch ( m_unit )
    {
        case KWUnit::U_MM:
            unitText=i18n("Millimeters (mm)");
            break;
        case KWUnit::U_INCH:
            unitText=i18n("Inches (inch)");
            break;
        case KWUnit::U_PT:
        default:
            unitText=i18n("Points (pt)" );
    }
    lTab->setText(i18n( "1 is a unit name", "Tabulator positions are given in %1" ).arg(unitText));

    KButtonBox * bbTabs = new KButtonBox( this );
    bAdd = bbTabs->addButton( i18n( "Add" ), false );
    bDel = bbTabs->addButton( i18n( "Delete" ), false );
    bModify = bbTabs->addButton( i18n( "Modify" ), false );
    grid->addWidget( bbTabs, 2, 0 );

    lTabs = new QListBox( this );
    grid->addWidget( lTabs, 3, 0 );

    QButtonGroup * g3 = new QButtonGroup( this );
    QGridLayout * tabGrid = new QGridLayout( g3, 5, 1, KDialog::marginHint(), KDialog::spacingHint() );
    g3->setExclusive( true );

    rtLeft = new QRadioButton( i18n( "Left" ), g3 );
    rtLeft->setChecked(true);
    tabGrid->addWidget( rtLeft, 0, 0 );
    g3->insert( rtLeft );

    rtCenter = new QRadioButton( i18n( "Center" ), g3 );
    tabGrid->addWidget( rtCenter, 1, 0 );
    g3->insert( rtCenter );

    rtRight = new QRadioButton( i18n( "Right" ), g3 );
    tabGrid->addWidget( rtRight, 2, 0 );
    g3->insert( rtRight );

    rtDecimal = new QRadioButton( i18n( "Decimal" ), g3 );
    tabGrid->addWidget( rtDecimal, 3, 0 );
    g3->insert( rtDecimal );

    tabGrid->setRowStretch( 4, 1 );
    tabGrid->setColStretch( 0, 1 );
    grid->addWidget( g3, 3, 1 );
    grid->setRowStretch( 3, 1 );
    if(lTabs->count()==0)
    {
	bDel->setEnabled(false);
	bModify->setEnabled(false);
    }
    bAdd->setEnabled(false);
    connect(eTabPos,SIGNAL(textChanged ( const QString & )),
            this, SLOT(slotTabValueChanged( const QString & )));
    connect(bAdd,SIGNAL(clicked ()),this,SLOT(addClicked()));
    connect(bModify,SIGNAL(clicked ()),this,SLOT(modifyClicked()));
    connect(bDel,SIGNAL(clicked ()),this,SLOT(delClicked()));
    connect(lTabs,SIGNAL(doubleClicked( QListBoxItem * ) ),
            this,SLOT(slotDoubleClicked( QListBoxItem * ) ));
    connect(lTabs,SIGNAL(clicked( QListBoxItem * ) ),
            this,SLOT(slotDoubleClicked( QListBoxItem * ) ));
}

void KWParagTabulatorsWidget::slotTabValueChanged( const QString &_text )
{
    bool state=!_text.isEmpty();
    bAdd->setEnabled(state);
    bModify->setEnabled(state && lTabs->currentItem()!=-1);
}

void KWParagTabulatorsWidget::addClicked()
{
    if(!eTabPos->text().isEmpty())
    {
        if(findExistingValue(eTabPos->text().toDouble()))
	{
            QString tmp=i18n("There is a tabulator at this position");
            KMessageBox::error( this, tmp );
            eTabPos->setText("");
            return;
	}

        lTabs->insertItem(eTabPos->text());
        bDel->setEnabled(lTabs->currentItem()!=-1);
        bModify->setEnabled(true);

        KoTabulator tab;
        if(rtLeft->isChecked())
            tab.type=T_LEFT;
        else if(rtCenter->isChecked())
            tab.type=T_CENTER;
        else if(rtRight->isChecked())
            tab.type=T_RIGHT;
        else if(rtDecimal->isChecked())
            tab.type=T_DEC_PNT;
        else
            tab.type=T_LEFT;
        tab.ptPos = KWUnit::fromUserValue( eTabPos->text().toDouble(), m_unit );
        m_tabList.append(tab);
        eTabPos->setText("");
    }
}

bool KWParagTabulatorsWidget::findExistingValue(double val /* user value */)
{
    KoTabulatorList::Iterator it = m_tabList.begin();
    for ( ; it != m_tabList.end(); ++it )
    {
        if ( KWUnit::userValue( ( *it ).ptPos, m_unit ) == val )
            return true;
    }
    return false;
}

void KWParagTabulatorsWidget::modifyClicked()
{
    if(!eTabPos->text().isEmpty() && lTabs->currentItem()!=-1)
    {
        m_tabList.remove( m_tabList.at( lTabs->currentItem() ) );
        lTabs->removeItem(lTabs->currentItem());
        addClicked();
        eTabPos->setText("");
    }
}

void KWParagTabulatorsWidget::delClicked()
{
    if(lTabs->currentItem()!=-1)
    {
        m_tabList.remove( m_tabList.at( lTabs->currentItem() ) );
        eTabPos->setText("");
        lTabs->removeItem(lTabs->currentItem());
        if(lTabs->count()==0)
	{
            bDel->setEnabled(false);
            bModify->setEnabled(false);

	}
        else
	{
            lTabs->setCurrentItem(0);
            setActiveItem(lTabs->currentText().toDouble());
        }
    }
}

void KWParagTabulatorsWidget::setActiveItem(double value /* user value */)
{
    KoTabulatorList::Iterator it = m_tabList.begin();
    for ( ; it != m_tabList.end(); ++it )
    {
       if ( KWUnit::userValue( ( *it ).ptPos, m_unit ) == value )
       {
            switch(( *it ).type)
            {
            case T_CENTER:
                rtCenter->setChecked(true);
                break;
            case  T_RIGHT:
                rtRight->setChecked(true);
                break;
            case T_DEC_PNT:
                rtDecimal->setChecked(true);
                break;
            case T_LEFT:
            default:
                rtLeft->setChecked(true);
                break;
            }
       }
       break;
    }
}

void KWParagTabulatorsWidget::slotDoubleClicked( QListBoxItem * )
{
    if(lTabs->currentItem()!=-1)
    {
        eTabPos->setText(lTabs->currentText());
        double value = lTabs->currentText().toDouble();
        bDel->setEnabled(true);
        bModify->setEnabled(true);
        setActiveItem(value);
    }
}

void KWParagTabulatorsWidget::display( const KWParagLayout & lay )
{
    m_tabList.clear();
    lTabs->clear();
    m_tabList = lay.tabList();
    KoTabulatorList::ConstIterator it = m_tabList.begin();
    for ( ; it != m_tabList.end(); ++it )
        lTabs->insertItem( QString::number( KWUnit::userValue( (*it).ptPos, m_unit ) ) );
}

void KWParagTabulatorsWidget::save( KWParagLayout & lay )
{
    lay.setTabList( m_tabList );
}

QString KWParagTabulatorsWidget::tabName()
{
    return i18n( "Tabulators" );
}

/******************************************************************/
/* Class: KWParagDia                                              */
/******************************************************************/
KWParagDia::KWParagDia( QWidget* parent, const char* name,
                        int flags, KWDocument *doc )
    : KDialogBase(Tabbed, QString::null, Ok | Cancel, Ok, parent, name, true )
{
    m_flags = flags;
    m_doc = doc;
    if ( m_flags & PD_SPACING )
    {
        QVBox * page = addVBoxPage( i18n( "Indent and Spacing" ) );
        m_indentSpacingWidget = new KWIndentSpacingWidget( m_doc->getUnit(), page, "indent-spacing" );
    }
    if ( m_flags & PD_ALIGN )
    {
        QVBox * page = addVBoxPage( i18n( "Aligns" ) );
        m_alignWidget = new KWParagAlignWidget( page, "align" );
    }
    if ( m_flags & PD_BORDERS )
    {
        QVBox * page = addVBoxPage( i18n( "Borders" ) );
        m_borderWidget = new KWParagBorderWidget( page, "border" );
    }
    if ( m_flags & PD_NUMBERING )
    {
        QVBox * page = addVBoxPage( i18n( "Bullets/Numbers" ) );
        m_counterWidget = new KWParagCounterWidget( page, "numbers" );
    }
    if ( m_flags & PD_TABS )
    {
        QVBox * page = addVBoxPage( i18n( "Tabulators" ) );
        m_tabulatorsWidget = new KWParagTabulatorsWidget( m_doc->getUnit(), page, "tabs" );
    }
    setInitialSize( QSize(600, 500) );
}

KWParagDia::~KWParagDia()
{
}

void KWParagDia::setParagLayout( const KWParagLayout & lay )
{
    m_indentSpacingWidget->display( lay );
    m_alignWidget->display( lay );
    m_borderWidget->display( lay );
    m_counterWidget->display( lay );
    m_tabulatorsWidget->display( lay );
    oldLayout = lay;
}

bool KWParagDia::isCounterChanged() const
{
    if ( oldLayout.counter ) // We had a counter
        return ! ( *oldLayout.counter == counter() );
    else // We had no counter -> changed if we have one now
        return counter().numbering() != KoParagCounter::NUM_NONE;
}

#include "paragdia.moc"
#include "paragdia_p.moc"
