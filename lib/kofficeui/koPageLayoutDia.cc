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

// Description: Page Layout Dialog (header)

/******************************************************************/

#include <koPageLayoutDia.h>

#include <qcombobox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpainter.h>
#include <qlineedit.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>
#include <knumvalidator.h>
#include <qspinbox.h>

#include <klocale.h>
#include <koUnit.h>


/******************************************************************/
/* class KoPagePreview                                            */
/******************************************************************/

/*===================== constrcutor ==============================*/
KoPagePreview::KoPagePreview( QWidget* parent, const char *name, const KoPageLayout& _layout )
    : QGroupBox( i18n( "Page Preview" ), parent, name )
{
    setPageLayout( _layout );
    columns = 1;
}

/*====================== destructor ==============================*/
KoPagePreview::~KoPagePreview()
{
}

/*=================== set layout =================================*/
void KoPagePreview::setPageLayout( const KoPageLayout &_layout )
{
    pgWidth = POINT_TO_MM(_layout.ptWidth) * 0.5;
    pgHeight = POINT_TO_MM(_layout.ptHeight) * 0.5;

    pgX = POINT_TO_MM(_layout.ptLeft) * 0.5;
    pgY = POINT_TO_MM(_layout.ptTop) * 0.5;
    pgW = pgWidth - ( POINT_TO_MM(_layout.ptLeft) + POINT_TO_MM(_layout.ptRight) ) * 0.5;
    pgH = pgHeight - ( POINT_TO_MM(_layout.ptTop) + POINT_TO_MM(_layout.ptBottom) ) * 0.5;

    repaint( true );
}

/*=================== set layout =================================*/
void KoPagePreview::setPageColumns( const KoColumns &_columns )
{
    columns = _columns.columns;
    repaint( true );
}

/*======================== draw contents =========================*/
void KoPagePreview::drawContents( QPainter *painter )
{
    double cw = pgW;
    if(columns!=1)
        cw/=static_cast<double>(columns);

    painter->setBrush( white );
    painter->setPen( QPen( black ) );

    int x=static_cast<int>( ( width() - pgWidth ) * 0.5 );
    int y=static_cast<int>( ( height() - pgHeight ) * 0.5 );
    int w=static_cast<int>(pgWidth);
    int h=static_cast<int>(pgHeight);
    painter->drawRect( x + 1, y + 1, w, h);
    painter->drawRect( x, y, w, h );

    painter->setBrush( QBrush( black, HorPattern ) );
    if ( pgW == pgWidth || pgH == pgHeight )
        painter->setPen( NoPen );
    else
        painter->setPen( lightGray );

    for ( int i = 0; i < columns; ++i )
        painter->drawRect( x + static_cast<int>(pgX) + static_cast<int>(i * cw),
                           y + static_cast<int>(pgY), static_cast<int>(cw),
                           static_cast<int>(pgH) );
}

/******************************************************************/
/* class KoPageLayoutDia                                          */
/******************************************************************/

/*==================== constructor ===============================*/
KoPageLayoutDia::KoPageLayoutDia( QWidget* parent, const char* name, 
				  const KoPageLayout& _layout,
                                  const KoHeadFoot& _hf, int tabs,
				  KoUnit::Unit unit )
    : KDialogBase( KDialogBase::Tabbed, i18n("Page Layout"), KDialogBase::Ok | KDialogBase::Cancel,
                   KDialogBase::Ok, parent, name, true)
{

    flags = tabs;
    pgPreview = 0;
    pgPreview2 = 0;

    layout = _layout;
    hf = _hf;
    m_unit = unit;

    cl.columns = 1;

    enableBorders = true;

    if ( tabs & FORMAT_AND_BORDERS ) setupTab1();
    if ( tabs & HEADER_AND_FOOTER ) setupTab2();

    retPressed = false;

    setFocusPolicy( QWidget::StrongFocus );
    setFocus();
}

/*==================== constructor ===============================*/
KoPageLayoutDia::KoPageLayoutDia( QWidget* parent, const char* name,
				  const KoPageLayout& _layout,
				  const KoHeadFoot& _hf,
				  const KoColumns& _cl, 
				  const KoKWHeaderFooter& _kwhf,
				  int tabs, KoUnit::Unit unit )
    : KDialogBase( KDialogBase::Tabbed, i18n("Page Layout"), KDialogBase::Ok | KDialogBase::Cancel,
                   KDialogBase::Ok, parent, name, true)
{
    flags = tabs;
    pgPreview = 0;
    pgPreview2 = 0;

    layout = _layout;
    hf = _hf;
    cl = _cl;
    kwhf = _kwhf;
    m_unit = unit;

    enableBorders = true;

    if ( tabs & DISABLE_BORDERS ) enableBorders = false;
    if ( tabs & FORMAT_AND_BORDERS ) setupTab1();
    if ( tabs & HEADER_AND_FOOTER ) setupTab2();
    if ( tabs & COLUMNS ) setupTab3();
    if ( tabs & KW_HEADER_AND_FOOTER ) setupTab4();

    retPressed = false;

    setFocusPolicy( QWidget::StrongFocus );
    setFocus();
}

/*===================== destructor ===============================*/
KoPageLayoutDia::~KoPageLayoutDia()
{
}

/*======================= show dialog ============================*/
bool KoPageLayoutDia::pageLayout( KoPageLayout& _layout, KoHeadFoot& _hf, int _tabs, KoUnit::Unit& unit )
{
    bool res = false;
    KoPageLayoutDia *dlg = new KoPageLayoutDia( 0, "PageLayout", _layout, _hf, _tabs, unit );

    if ( dlg->exec() == QDialog::Accepted ) {
        res = true;
        if ( _tabs & FORMAT_AND_BORDERS ) _layout = dlg->getLayout();
        if ( _tabs & HEADER_AND_FOOTER ) _hf = dlg->getHeadFoot();
        unit = dlg->unit();
    }

    delete dlg;

    return res;
}

/*======================= show dialog ============================*/
bool KoPageLayoutDia::pageLayout( KoPageLayout& _layout, KoHeadFoot& _hf, KoColumns& _cl,
                                  KoKWHeaderFooter &_kwhf, int _tabs, KoUnit::Unit& unit )
{
    bool res = false;
    KoPageLayoutDia *dlg = new KoPageLayoutDia( 0, "PageLayout", _layout, _hf, _cl, _kwhf, _tabs, unit );

    if ( dlg->exec() == QDialog::Accepted ) {
        res = true;
        if ( _tabs & FORMAT_AND_BORDERS ) _layout = dlg->getLayout();
        if ( _tabs & HEADER_AND_FOOTER ) _hf = dlg->getHeadFoot();
        if ( _tabs & COLUMNS ) _cl = dlg->getColumns();
        if ( _tabs & KW_HEADER_AND_FOOTER ) _kwhf = dlg->getKWHeaderFooter();
        unit = dlg->unit();
    }

    delete dlg;

    return res;
}

/*===================== get a standard page layout ===============*/
KoPageLayout KoPageLayoutDia::standardLayout()
{
    KoPageLayout        _layout;
    _layout.format = PG_DIN_A4;
    _layout.orientation = PG_PORTRAIT;
    _layout.ptWidth = MM_TO_POINT( PG_A4_WIDTH );
    _layout.ptHeight = MM_TO_POINT( PG_A4_HEIGHT );
    _layout.ptLeft = MM_TO_POINT( 20.0 );
    _layout.ptRight = MM_TO_POINT( 20.0 );
    _layout.ptTop = MM_TO_POINT( 20.0 );
    _layout.ptBottom = MM_TO_POINT( 20.0 );

    return  _layout;
}

/*====================== get header - footer =====================*/
KoHeadFoot KoPageLayoutDia::getHeadFoot()
{
    hf.headLeft = eHeadLeft->text();
    hf.headMid = eHeadMid->text();
    hf.headRight = eHeadRight->text();
    hf.footLeft = eFootLeft->text();
    hf.footMid = eFootMid->text();
    hf.footRight = eFootRight->text();

    return hf;
}

/*================================================================*/
KoColumns KoPageLayoutDia::getColumns()
{
    cl.columns = nColumns->value();
    cl.ptColumnSpacing = KoUnit::fromUserValue( nCSpacing->text(), m_unit  );
    return cl;
}

/*================================================================*/
KoKWHeaderFooter KoPageLayoutDia::getKWHeaderFooter()
{
    if ( rhSame->isChecked() )
        kwhf.header = HF_SAME;
    else if ( rhFirst->isChecked() )
        kwhf.header = HF_FIRST_DIFF;
    else if ( rhEvenOdd->isChecked() )
        kwhf.header = HF_EO_DIFF;

    kwhf.ptHeaderBodySpacing = KoUnit::fromUserValue( nHSpacing->text(), m_unit );
    kwhf.ptFooterBodySpacing = KoUnit::fromUserValue( nFSpacing->text(), m_unit );

    if ( rfSame->isChecked() )
        kwhf.footer = HF_SAME;
    else if ( rfFirst->isChecked() )
        kwhf.footer = HF_FIRST_DIFF;
    else if ( rfEvenOdd->isChecked() )
        kwhf.footer = HF_EO_DIFF;

    return kwhf;
}

/*================ setup format and borders tab ==================*/
void KoPageLayoutDia::setupTab1()
{
    QWidget *tab1 = addPage(i18n( "Format and Borders" ));

    QGridLayout *grid1 = new QGridLayout( tab1, 5, 2, 15, 7 );

    QLabel *lpgUnit;
    if ( !( flags & DISABLE_UNIT ) ) {
        // ------------- unit _______________
        // label unit
        lpgUnit = new QLabel( i18n( "Unit:" ), tab1 );
        grid1->addWidget( lpgUnit, 0, 0 );

        // combo unit
        cpgUnit = new QComboBox( false, tab1, "cpgUnit" );
        cpgUnit->setAutoResize( false );
        cpgUnit->insertItem( i18n( "Millimeters (mm)" ) );
        cpgUnit->insertItem( i18n( "Points (pt)" ) );
        cpgUnit->insertItem( i18n( "Inches (in)" ) );
        grid1->addWidget( cpgUnit, 1, 0 );
        connect( cpgUnit, SIGNAL( activated( int ) ), this, SLOT( unitChanged( int ) ) );
    } else {
        QString str=KoUnit::unitDescription(m_unit);

        lpgUnit = new QLabel( i18n("All values are given in %1.").arg(str), tab1 );
        grid1->addWidget( lpgUnit, 0, 0 );
    }

    // -------------- page format -----------------
    QGroupBox *formatFrame = new QGroupBox( i18n( "Page Format" ), tab1 );
    QGridLayout *formatGrid = new QGridLayout( formatFrame, 4, 2, 15, 7 );

    // label format
    QLabel *lpgFormat = new QLabel( i18n( "\nFormat:" ), formatFrame );
    formatGrid->addWidget( lpgFormat, 0, 0 );

    // label orientation
    QLabel *lpgOrientation = new QLabel( i18n( "\nOrientation:" ), formatFrame );
    formatGrid->addWidget( lpgOrientation, 0, 1 );

    // combo format
    cpgFormat = new QComboBox( false, formatFrame, "cpgFormat" );
    cpgFormat->setAutoResize( false );
    cpgFormat->insertStringList( KoPageFormat::allFormats() );
    formatGrid->addWidget( cpgFormat, 1, 0 );
    connect( cpgFormat, SIGNAL( activated( int ) ), this, SLOT( formatChanged( int ) ) );

    // combo orientation
    cpgOrientation = new QComboBox( false, formatFrame, "cpgOrientation" );
    cpgOrientation->setAutoResize( false );
    cpgOrientation->insertItem( i18n( "Portrait" ) );
    cpgOrientation->insertItem( i18n( "Landscape" ) );
    formatGrid->addWidget( cpgOrientation, 1, 1 );
    connect( cpgOrientation, SIGNAL( activated( int ) ), this, SLOT( orientationChanged( int ) ) );

    // label width
    QLabel *lpgWidth = new QLabel( i18n( "Width:" ), formatFrame );
    formatGrid->addWidget( lpgWidth, 2, 0 );

    // linedit width
    epgWidth = new QLineEdit( formatFrame, "Width" );
    epgWidth->setValidator( new KFloatValidator( 0,9999,true,epgWidth ) );
    epgWidth->setText( i18n("000.00") );
    epgWidth->setMaxLength( 6 );
    epgWidth->setEchoMode( QLineEdit::Normal );
    epgWidth->setFrame( true );
    formatGrid->addWidget( epgWidth, 3, 0 );
    if ( layout.format != PG_CUSTOM )
        epgWidth->setEnabled( false );
    connect( epgWidth, SIGNAL( returnPressed() ), this, SLOT( rPressed() ) );
    connect( epgWidth, SIGNAL( returnPressed() ), this, SLOT( widthChanged() ) );
    connect( epgWidth, SIGNAL( textChanged( const QString& ) ), this, SLOT( widthChanged() ) );

    // label height
    QLabel *lpgHeight = new QLabel( i18n( "Height:" ), formatFrame );
    formatGrid->addWidget( lpgHeight, 2, 1 );

    // linedit height
    epgHeight = new QLineEdit( formatFrame, "Height" );
    epgHeight->setValidator( new KFloatValidator( 0,9999,true,epgHeight ) );
    epgHeight->setText( i18n("000.00") );
    epgHeight->setMaxLength( 6 );
    epgHeight->setEchoMode( QLineEdit::Normal );
    epgHeight->setFrame( true );
    formatGrid->addWidget( epgHeight, 3, 1 );
    if ( layout.format != PG_CUSTOM )
        epgHeight->setEnabled( false );
    connect( epgHeight, SIGNAL( returnPressed() ), this, SLOT( rPressed() ) );
    connect( epgHeight, SIGNAL( returnPressed() ), this, SLOT( heightChanged() ) );
    connect( epgHeight, SIGNAL( textChanged( const QString & ) ), this, SLOT( heightChanged() ) );

    // grid col spacing
    formatGrid->addColSpacing( 0, lpgFormat->width() );
    formatGrid->addColSpacing( 0, cpgFormat->width() );
    formatGrid->addColSpacing( 0, lpgWidth->width() );
    formatGrid->addColSpacing( 0, epgWidth->width() );
    formatGrid->addColSpacing( 1, lpgOrientation->width() );
    formatGrid->addColSpacing( 1, cpgOrientation->width() );
    formatGrid->addColSpacing( 1, lpgHeight->width() );
    formatGrid->addColSpacing( 1, epgHeight->width() );

    // grid row spacing
    formatGrid->addRowSpacing( 0, lpgFormat->height() );
    formatGrid->addRowSpacing( 0, lpgOrientation->height() );
    formatGrid->addRowSpacing( 1, cpgFormat->height() );
    formatGrid->addRowSpacing( 1, cpgOrientation->height() );
    formatGrid->addRowSpacing( 2, lpgWidth->height() );
    formatGrid->addRowSpacing( 2, lpgHeight->height() );
    formatGrid->addRowSpacing( 3, epgWidth->height() );
    formatGrid->addRowSpacing( 3, epgHeight->height() );

    grid1->addWidget( formatFrame, 2, 0 );

    // --------------- page borders ---------------
    QGroupBox *borderFrame = new QGroupBox( i18n( "Page Borders" ), tab1 );
    QGridLayout *borderGrid = new QGridLayout( borderFrame, 4, 2, 15, 7 );

    // label left
    QLabel *lbrLeft = new QLabel( i18n( "\nLeft:" ), borderFrame );
    borderGrid->addWidget( lbrLeft, 0, 0 );

    // linedit left
    ebrLeft = new QLineEdit( borderFrame, "Left" );
    ebrLeft->setValidator( new KFloatValidator( 0,9999,true,ebrLeft ) );
    ebrLeft->setText( i18n("000.00") );
    ebrLeft->setMaxLength( 6 );
    ebrLeft->setEchoMode( QLineEdit::Normal );
    ebrLeft->setFrame( true );
    borderGrid->addWidget( ebrLeft, 1, 0 );
    connect( ebrLeft, SIGNAL( returnPressed() ), this, SLOT( rPressed() ) );
    connect( ebrLeft, SIGNAL( returnPressed() ), this, SLOT( leftChanged() ) );
    connect( ebrLeft, SIGNAL( textChanged( const QString & ) ), this, SLOT( leftChanged() ) );
    if ( !enableBorders ) ebrLeft->setEnabled( false );

    // label right
    QLabel *lbrRight = new QLabel( i18n( "\nRight:" ), borderFrame );
    borderGrid->addWidget( lbrRight, 0, 1 );

    // linedit right
    ebrRight = new QLineEdit( borderFrame, "Right" );
    ebrRight->setValidator( new KFloatValidator( 0,9999,true,ebrRight ) );
    ebrRight->setText( i18n("000.00") );
    ebrRight->setMaxLength( 6 );
    ebrRight->setEchoMode( QLineEdit::Normal );
    ebrRight->setFrame( true );
    borderGrid->addWidget( ebrRight, 1, 1 );
    connect( ebrRight, SIGNAL( returnPressed() ), this, SLOT( rPressed() ) );
    connect( ebrRight, SIGNAL( returnPressed() ), this, SLOT( rightChanged() ) );
    connect( ebrRight, SIGNAL( textChanged( const QString & ) ), this, SLOT( rightChanged() ) );
    if ( !enableBorders ) ebrRight->setEnabled( false );

    // label top
    QLabel *lbrTop = new QLabel( i18n( "Top:" ), borderFrame );
    borderGrid->addWidget( lbrTop, 2, 0 );

    // linedit top
    ebrTop = new QLineEdit( borderFrame, "Top" );
    ebrTop->setValidator( new KFloatValidator( 0,9999,true,ebrTop ) );
    ebrTop->setText( i18n("000.00") );
    ebrTop->setMaxLength( 6 );
    ebrTop->setEchoMode( QLineEdit::Normal );
    ebrTop->setFrame( true );
    borderGrid->addWidget( ebrTop, 3, 0 );
    connect( ebrTop, SIGNAL( returnPressed() ), this, SLOT( rPressed() ) );
    connect( ebrTop, SIGNAL( returnPressed() ), this, SLOT( topChanged() ) );
    connect( ebrTop, SIGNAL( textChanged( const QString & ) ), this, SLOT( topChanged() ) );
    if ( !enableBorders ) ebrTop->setEnabled( false );

    // label bottom
    QLabel *lbrBottom = new QLabel( i18n( "Bottom:" ), borderFrame );
    borderGrid->addWidget( lbrBottom, 2, 1 );

    // linedit bottom
    ebrBottom = new QLineEdit( borderFrame, "Bottom" );
    ebrBottom->setValidator( new KFloatValidator( 0,9999,true,ebrBottom ) );
    ebrBottom->setText( i18n("000.00") );
    ebrBottom->setMaxLength( 6 );
    ebrBottom->setEchoMode( QLineEdit::Normal );
    ebrBottom->setFrame( true );
    borderGrid->addWidget( ebrBottom, 3, 1 );
    connect( ebrBottom, SIGNAL( returnPressed() ), this, SLOT( rPressed() ) );
    connect( ebrBottom, SIGNAL( returnPressed() ), this, SLOT( bottomChanged() ) );
    connect( ebrBottom, SIGNAL( textChanged( const QString & ) ), this, SLOT( bottomChanged() ) );
    if ( !enableBorders ) ebrBottom->setEnabled( false );

    // grid col spacing
    borderGrid->addColSpacing( 0, lbrLeft->width() );
    borderGrid->addColSpacing( 0, ebrLeft->width() );
    borderGrid->addColSpacing( 0, lbrTop->width() );
    borderGrid->addColSpacing( 0, ebrTop->width() );
    borderGrid->addColSpacing( 1, lbrRight->width() );
    borderGrid->addColSpacing( 1, ebrRight->width() );
    borderGrid->addColSpacing( 1, lbrBottom->width() );
    borderGrid->addColSpacing( 1, ebrBottom->width() );

    // grid row spacing
    borderGrid->addRowSpacing( 0, lbrLeft->height() );
    borderGrid->addRowSpacing( 0, lbrRight->height() );
    borderGrid->addRowSpacing( 1, ebrLeft->height() );
    borderGrid->addRowSpacing( 1, ebrRight->height() );
    borderGrid->addRowSpacing( 2, lbrTop->height() );
    borderGrid->addRowSpacing( 2, lbrBottom->height() );
    borderGrid->addRowSpacing( 3, ebrTop->height() );
    borderGrid->addRowSpacing( 3, ebrBottom->height() );

    grid1->addWidget( borderFrame, 3, 0 );

    // ------------- preview -----------
    pgPreview = new KoPagePreview( tab1, "Preview", layout );
    grid1->addMultiCellWidget( pgPreview, 2, 4, 1, 1 );

    // --------------- main grid ------------------
    grid1->addColSpacing( 0, lpgUnit->width() );
    grid1->addColSpacing( 0, ( flags & DISABLE_UNIT ) ? 0 : cpgUnit->width() );
    grid1->addColSpacing( 0, formatFrame->width() );
    grid1->addColSpacing( 0, borderFrame->width() );
    grid1->addColSpacing( 1, 280 );
    grid1->setColStretch( 1, 1 );

    grid1->addRowSpacing( 0, lpgUnit->height() );
    grid1->addRowSpacing( 1, ( flags & DISABLE_UNIT ) ? 0 : cpgUnit->height() );
    grid1->addRowSpacing( 2, formatFrame->height() );
    grid1->addRowSpacing( 2, 120 );
    grid1->addRowSpacing( 3, borderFrame->height() );
    grid1->addRowSpacing( 3, 120 );
    grid1->setRowStretch( 4, 1 );

    setValuesTab1();
    updatePreview( layout );
}

/*================= setup values for tab one =====================*/
void KoPageLayoutDia::setValuesTab1()
{
    // unit
    if ( !( flags & DISABLE_UNIT ) )
        cpgUnit->setCurrentItem( m_unit );

    // page format
    cpgFormat->setCurrentItem( layout.format );

    // orientation
    cpgOrientation->setCurrentItem( layout.orientation );

    setValuesTab1Helper();

    pgPreview->setPageLayout( layout );
}

void KoPageLayoutDia::setValuesTab1Helper() {
    epgWidth->setText( KoUnit::userValue( layout.ptWidth, m_unit ) );
    epgHeight->setText( KoUnit::userValue( layout.ptHeight, m_unit ) );
    ebrLeft->setText( KoUnit::userValue( layout.ptLeft, m_unit ) );
    ebrRight->setText( KoUnit::userValue( layout.ptRight, m_unit ) );
    ebrTop->setText( KoUnit::userValue( layout.ptTop, m_unit ) );
    ebrBottom->setText( KoUnit::userValue( layout.ptBottom, m_unit ) );
}

/*================ setup header and footer tab ===================*/
void KoPageLayoutDia::setupTab2()
{
    QWidget *tab2 = addPage(i18n( "Header and Footer" ));
    QGridLayout *grid2 = new QGridLayout( tab2, 7, 3, 6, 6 );

    // ------------- header ---------------
    QLabel *lHead = new QLabel( i18n( "Head line:" ), tab2 );
    grid2->addWidget( lHead, 0, 0 );

    QLabel *lHeadLeft = new QLabel( i18n( "Left:" ), tab2 );
    grid2->addWidget( lHeadLeft, 1, 0 );

    eHeadLeft = new QLineEdit( tab2 );
    grid2->addWidget( eHeadLeft, 2, 0 );
    eHeadLeft->setText( hf.headLeft );

    QLabel *lHeadMid = new QLabel( i18n( "Mid:" ), tab2 );
    grid2->addWidget( lHeadMid, 1, 1 );

    eHeadMid = new QLineEdit( tab2 );
    grid2->addWidget( eHeadMid, 2, 1 );
    eHeadMid->setText( hf.headMid );

    QLabel *lHeadRight = new QLabel( i18n( "Right:" ), tab2 );
    grid2->addWidget( lHeadRight, 1, 2 );

    eHeadRight = new QLineEdit( tab2 );
    grid2->addWidget( eHeadRight, 2, 2 );
    eHeadRight->setText( hf.headRight );

    // ------------- footer ---------------
    QLabel *lFoot = new QLabel( i18n( "\nFoot line:" ), tab2 );
    grid2->addWidget( lFoot, 3, 0 );

    QLabel *lFootLeft = new QLabel( i18n( "Left:" ), tab2 );
    grid2->addWidget( lFootLeft, 4, 0 );

    eFootLeft = new QLineEdit( tab2 );
    grid2->addWidget( eFootLeft, 5, 0 );
    eFootLeft->setText( hf.footLeft );

    QLabel *lFootMid = new QLabel( i18n( "Mid:" ), tab2 );
    grid2->addWidget( lFootMid, 4, 1 );

    eFootMid = new QLineEdit( tab2 );
    grid2->addWidget( eFootMid, 5, 1 );
    eFootMid->setText( hf.footMid );

    QLabel *lFootRight = new QLabel( i18n( "Right:" ), tab2 );
    grid2->addWidget( lFootRight, 4, 2 );

    eFootRight = new QLineEdit( tab2 );
    grid2->addWidget( eFootRight, 5, 2 );
    eFootRight->setText( hf.footRight );

    QLabel *lMacros2 = new QLabel( i18n("<qt>You can insert several tags in the text:"
                           "<ul><li>&lt;page&gt;: The current page</li>"
                           "<li>&lt;name&gt;: The filename or URL</li>"
                           "<li>&lt;file&gt;: The filename with complete path or the URL</li>"
                           "<li>&lt;time&gt;: The current time</li>"
                           "<li>&lt;date&gt;: The current date</li>"
                           "<li>&lt;author&gt;: Your full name</li>"
                           "<li>&lt;org&gt;: Your organization</li>"
                           "<li>&lt;email&gt;: Your email address</li></ul></qt>"), tab2 );
    grid2->addMultiCellWidget( lMacros2, 6, 6, 0, 2 );
}

/*================================================================*/
void KoPageLayoutDia::setupTab3()
{
    QWidget *tab3 = addPage(i18n( "Columns" ));

    QGridLayout *grid3 = new QGridLayout( tab3, 5, 2, 15, 7 );

    QLabel *lColumns = new QLabel( i18n( "Columns:" ), tab3 );
    grid3->addWidget( lColumns, 0, 0 );

    nColumns = new QSpinBox( 1, 16, 1, tab3 );
    grid3->addWidget( nColumns, 1, 0 );
    nColumns->setValue( cl.columns );
    connect( nColumns, SIGNAL( valueChanged( int ) ), this, SLOT( nColChanged( int ) ) );

    QString str = KoUnit::unitDescription( m_unit );

    QLabel *lCSpacing = new QLabel( i18n("Column Spacing (%1):").arg(str), tab3 );
    grid3->addWidget( lCSpacing, 2, 0 );

    nCSpacing = new QLineEdit( tab3, "" );
    nCSpacing->setValidator( new KFloatValidator( 0,9999,true,nCSpacing ) );
    nCSpacing->setText( i18n("0.00") );
    nCSpacing->setMaxLength( 5 );
    nCSpacing->setEchoMode( QLineEdit::Normal );
    nCSpacing->setFrame( true );
    grid3->addWidget( nCSpacing, 3, 0 );

    nCSpacing->setText( KoUnit::userValue( cl.ptColumnSpacing, m_unit ) );
    connect( nCSpacing, SIGNAL( textChanged( const QString & ) ),
             this, SLOT( nSpaceChanged( const QString & ) ) );

    // ------------- preview -----------
    pgPreview2 = new KoPagePreview( tab3, "Preview", layout );
    grid3->addMultiCellWidget( pgPreview2, 0, 4, 1, 1 );

    // --------------- main grid ------------------
    grid3->addColSpacing( 0, lColumns->width() );
    grid3->addColSpacing( 0, nColumns->width() );
    grid3->addColSpacing( 0, lCSpacing->width() );
    grid3->addColSpacing( 0, nCSpacing->width() );
    grid3->addColSpacing( 1, pgPreview2->width() );
    grid3->setColStretch( 1, 1 );

    grid3->addRowSpacing( 0, lColumns->height() );
    grid3->addRowSpacing( 1, nColumns->height() );
    grid3->addRowSpacing( 2, lCSpacing->height() );
    grid3->addRowSpacing( 3, nCSpacing->height() );
    grid3->setRowStretch( 4, 1 );

    if ( pgPreview ) pgPreview->setPageColumns( cl );
    pgPreview2->setPageColumns( cl );
}

/*================================================================*/
void KoPageLayoutDia::setupTab4()
{
    QString str = KoUnit::unitDescription(m_unit);

    QWidget *tab4 = addPage(i18n( "Header and Footer" ));
    QGridLayout *grid4 = new QGridLayout( tab4, 3, 1, 15, 7 );

    QButtonGroup *gHeader = new QButtonGroup( i18n( "Header" ), tab4 );
    gHeader->setExclusive( true );
    QGridLayout *headerGrid = new QGridLayout( gHeader, 5, 2, 15, 7 );

    rhSame = new QRadioButton( i18n( "Same header for all pages" ), gHeader );
    gHeader->insert( rhSame );
    headerGrid->addMultiCellWidget( rhSame, 1, 1, 0, 1 );
    if ( kwhf.header == HF_SAME ) rhSame->setChecked( true );

    rhFirst = new QRadioButton( i18n( "Different header for the first page" ), gHeader );
    gHeader->insert( rhFirst );
    headerGrid->addMultiCellWidget( rhFirst, 2, 2, 0, 1 );
    if ( kwhf.header == HF_FIRST_DIFF ) rhFirst->setChecked( true );

    rhEvenOdd = new QRadioButton( i18n( "Different header for even and odd pages" ), gHeader );
    gHeader->insert( rhEvenOdd );
    headerGrid->addMultiCellWidget( rhEvenOdd, 3, 3, 0, 1 );
    if ( kwhf.header == HF_EO_DIFF ) rhEvenOdd->setChecked( true );

    QLabel *lHSpacing = new QLabel( i18n("Spacing between header and body (%1):").arg(str), gHeader );
    lHSpacing->setAlignment( AlignRight | AlignVCenter );
    headerGrid->addWidget( lHSpacing, 4, 0 );

    nHSpacing = new QLineEdit( gHeader, "" );
    nHSpacing->setValidator( new KFloatValidator( 0,9999,true,nHSpacing ) );
    nHSpacing->setText( i18n("0.00") );
    nHSpacing->setMaxLength( 5 );
    nHSpacing->setEchoMode( QLineEdit::Normal );
    nHSpacing->setFrame( true );
    headerGrid->addWidget( nHSpacing, 4, 1 );

    nHSpacing->setText( KoUnit::userValue( kwhf.ptHeaderBodySpacing, m_unit ) );

    headerGrid->addColSpacing( 0, rhSame->width() / 2 );
    headerGrid->addColSpacing( 1, rhSame->width() / 2 );
    headerGrid->addColSpacing( 0, rhFirst->width() / 2 );
    headerGrid->addColSpacing( 1, rhFirst->width() / 2 );
    headerGrid->addColSpacing( 0, rhEvenOdd->width() / 2 );
    headerGrid->addColSpacing( 1, rhEvenOdd->width() / 2 );
    headerGrid->addColSpacing( 0, lHSpacing->width() );
    headerGrid->addColSpacing( 1, nHSpacing->width() );
    headerGrid->setColStretch( 1, 1 );

    headerGrid->addRowSpacing( 0, 7 );
    headerGrid->addRowSpacing( 1, rhSame->height() );
    headerGrid->addRowSpacing( 2, rhFirst->height() );
    headerGrid->addRowSpacing( 3, rhEvenOdd->height() );
    headerGrid->addRowSpacing( 4, lHSpacing->height() );
    headerGrid->addRowSpacing( 4, nHSpacing->height() );
    headerGrid->setRowStretch( 0, 0 );
    headerGrid->setRowStretch( 1, 0 );
    headerGrid->setRowStretch( 2, 0 );
    headerGrid->setRowStretch( 3, 0 );
    headerGrid->setRowStretch( 4, 0 );

    grid4->addWidget( gHeader, 0, 0 );

    QButtonGroup *gFooter = new QButtonGroup( i18n( "Footer" ), tab4 );
    gFooter->setExclusive( true );
    QGridLayout *footerGrid = new QGridLayout( gFooter, 5, 2, 15, 7 );

    rfSame = new QRadioButton( i18n( "Same footer for all pages" ), gFooter );
    gFooter->insert( rfSame );
    footerGrid->addMultiCellWidget( rfSame, 1, 1, 0, 1 );
    if ( kwhf.footer == HF_SAME ) rfSame->setChecked( true );

    rfFirst = new QRadioButton( i18n( "Different footer for the first page" ), gFooter );
    gFooter->insert( rfFirst );
    footerGrid->addMultiCellWidget( rfFirst, 2, 2, 0, 1 );
    if ( kwhf.footer == HF_FIRST_DIFF ) rfFirst->setChecked( true );

    rfEvenOdd = new QRadioButton( i18n( "Different footer for even and odd pages" ), gFooter );
    gFooter->insert( rfEvenOdd );
    footerGrid->addMultiCellWidget( rfEvenOdd, 3, 3, 0, 1 );
    if ( kwhf.footer == HF_EO_DIFF ) rfEvenOdd->setChecked( true );

    QLabel *lFSpacing = new QLabel( i18n("Spacing between footer and body (%1):").arg(str), gFooter );
    lFSpacing->setAlignment( AlignRight | AlignVCenter );
    footerGrid->addWidget( lFSpacing, 4, 0 );

    nFSpacing = new QLineEdit( gFooter, "" );
    nFSpacing->setValidator( new KFloatValidator( 0,9999,true,nFSpacing ));
    nFSpacing->setText( i18n("0.00") );
    nFSpacing->setMaxLength( 5 );
    nFSpacing->setEchoMode( QLineEdit::Normal );
    nFSpacing->setFrame( true );
    footerGrid->addWidget( nFSpacing, 4, 1 );

    nFSpacing->setText( KoUnit::userValue( kwhf.ptFooterBodySpacing, m_unit ) );

    footerGrid->addColSpacing( 0, rfSame->width() / 2 );
    footerGrid->addColSpacing( 1, rfSame->width() / 2 );
    footerGrid->addColSpacing( 0, rfFirst->width() / 2 );
    footerGrid->addColSpacing( 1, rfFirst->width() / 2 );
    footerGrid->addColSpacing( 0, rfEvenOdd->width() / 2 );
    footerGrid->addColSpacing( 1, rfEvenOdd->width() / 2 );
    footerGrid->addColSpacing( 0, lFSpacing->width() );
    footerGrid->addColSpacing( 1, nFSpacing->width() );
    footerGrid->setColStretch( 1, 1 );

    footerGrid->addRowSpacing( 0, 7 );
    footerGrid->addRowSpacing( 1, rfSame->height() );
    footerGrid->addRowSpacing( 2, rfFirst->height() );
    footerGrid->addRowSpacing( 3, rfEvenOdd->height() );
    footerGrid->addRowSpacing( 4, lFSpacing->height() );
    footerGrid->addRowSpacing( 4, nFSpacing->height() );
    footerGrid->setRowStretch( 0, 0 );
    footerGrid->setRowStretch( 1, 0 );
    footerGrid->setRowStretch( 2, 0 );
    footerGrid->setRowStretch( 3, 0 );
    footerGrid->setRowStretch( 4, 0 );

    grid4->addWidget( gFooter, 1, 0 );

    grid4->addColSpacing( 0, gHeader->width() );
    grid4->addColSpacing( 0, gFooter->width() );
    grid4->setColStretch( 0, 1 );

    grid4->addRowSpacing( 0, gHeader->height() );
    grid4->addRowSpacing( 1, gFooter->height() );
    grid4->setRowStretch( 2, 0 );
    grid4->setRowStretch( 2, 1 );
}

/*====================== update the preview ======================*/
void KoPageLayoutDia::updatePreview( const KoPageLayout& )
{
    if ( pgPreview ) pgPreview->setPageLayout( layout );
    if ( pgPreview ) pgPreview->setPageColumns( cl );
    if ( pgPreview2 ) pgPreview2->setPageLayout( layout );
    if ( pgPreview2 ) pgPreview2->setPageColumns( cl );
}

/*===================== unit changed =============================*/
void KoPageLayoutDia::unitChanged( int _unit )
{
    m_unit = static_cast<KoUnit::Unit>( _unit );
    setValuesTab1Helper();
    updatePreview( layout );
}

/*===================== format changed =============================*/
void KoPageLayoutDia::formatChanged( int _format )
{
    if ( ( KoFormat )_format != layout.format ) {
        bool enable = true;

        layout.format = ( KoFormat )_format;
        if ( ( KoFormat )_format != PG_CUSTOM ) enable = false;
        epgWidth->setEnabled( enable );
        epgHeight->setEnabled( enable );

        double w = layout.ptWidth;
        double h = layout.ptHeight;
        if ( layout.format != PG_CUSTOM )
        {
            w = MM_TO_POINT( KoPageFormat::width( layout.format, layout.orientation ) );
            h = MM_TO_POINT( KoPageFormat::height( layout.format, layout.orientation ) );
        }

        layout.ptWidth = w;
        layout.ptHeight = h;

        epgWidth->setText( KoUnit::userValue( layout.ptWidth, m_unit ) );
        epgHeight->setText( KoUnit::userValue( layout.ptHeight, m_unit ) );

        updatePreview( layout );
    }
}

/*===================== format changed =============================*/
void KoPageLayoutDia::orientationChanged( int _orientation )
{
    if ( ( KoOrientation )_orientation != layout.orientation ) {

        layout.ptWidth = KoUnit::fromUserValue( epgWidth->text(), m_unit );
        layout.ptHeight = KoUnit::fromUserValue( epgHeight->text(), m_unit );
        layout.ptLeft = KoUnit::fromUserValue( ebrLeft->text(), m_unit );
        layout.ptRight = KoUnit::fromUserValue( ebrRight->text(), m_unit );
        layout.ptTop = KoUnit::fromUserValue( ebrTop->text(), m_unit );
        layout.ptBottom = KoUnit::fromUserValue( ebrBottom->text(), m_unit );

        qSwap( layout.ptWidth, layout.ptHeight );

        if ( ( KoOrientation )_orientation == PG_LANDSCAPE ) {
            double tmp = layout.ptLeft;
            layout.ptLeft = layout.ptBottom;
            layout.ptBottom = layout.ptRight;
            layout.ptRight = layout.ptTop;
            layout.ptTop = tmp;
        } else {
            double tmp = layout.ptTop;
            layout.ptTop = layout.ptRight;
            layout.ptRight = layout.ptBottom;
            layout.ptBottom = layout.ptLeft;
            layout.ptLeft = tmp;
        }

        layout.orientation = ( KoOrientation )_orientation;
        setValuesTab1();
        updatePreview( layout );
    }
}

void KoPageLayoutDia::changed(QLineEdit *line, double &pt) {

    if ( line->text().length() == 0 && retPressed )
        line->setText( i18n("0.00") );
    if ( line->text().toDouble()<0)
        line->setText( i18n("0.00") );
    pt = KoUnit::fromUserValue( line->text(), m_unit );
    retPressed = false;
}

/*===================== width changed =============================*/
void KoPageLayoutDia::widthChanged()
{
    changed(epgWidth, layout.ptWidth);
    updatePreview( layout );
}

/*===================== height changed ============================*/
void KoPageLayoutDia::heightChanged()
{
    changed(epgHeight, layout.ptHeight);
    updatePreview( layout );
}

/*===================== left border changed =======================*/
void KoPageLayoutDia::leftChanged()
{
    changed(ebrLeft, layout.ptLeft);
    updatePreview( layout );
}

/*===================== right border changed =======================*/
void KoPageLayoutDia::rightChanged()
{
    changed(ebrRight, layout.ptRight);
    updatePreview( layout );
}

/*===================== top border changed =========================*/
void KoPageLayoutDia::topChanged()
{
    changed(ebrTop, layout.ptTop);
    updatePreview( layout );
}

/*===================== bottom border changed ======================*/
void KoPageLayoutDia::bottomChanged()
{
    changed(ebrBottom, layout.ptBottom);
    updatePreview( layout );
}

/*==================================================================*/
void KoPageLayoutDia::nColChanged( int _val )
{
    cl.columns = _val;
    updatePreview( layout );
}

/*==================================================================*/
void KoPageLayoutDia::nSpaceChanged( const QString &_val )
{
    cl.ptColumnSpacing = KoUnit::fromUserValue( _val, m_unit );
    updatePreview( layout );
}

#include <koPageLayoutDia.moc>
