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
#include <knuminput.h>
#include <qcheckbox.h>
#include <qbuttongroup.h>

#include <kiconloader.h>

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
				  KoUnit::Unit unit, bool modal )
    : KDialogBase( KDialogBase::Tabbed, i18n("Page Layout"), KDialogBase::Ok | KDialogBase::Cancel,
                   KDialogBase::Ok, parent, name, modal)
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
    cl.ptColumnSpacing = KoUnit::ptFromUnit( nCSpacing->value(), m_unit  );
    return cl;
}

/*================================================================*/
KoKWHeaderFooter KoPageLayoutDia::getKWHeaderFooter()
{
    if ( rhFirst->isChecked() && rhEvenOdd->isChecked() )
        kwhf.header = HF_FIRST_EO_DIFF;
    else if ( rhFirst->isChecked() )
        kwhf.header = HF_FIRST_DIFF;
    else if ( rhEvenOdd->isChecked() )
        kwhf.header = HF_EO_DIFF;
    else
        kwhf.header = HF_SAME;

    kwhf.ptHeaderBodySpacing = KoUnit::ptFromUnit( nHSpacing->value(), m_unit );
    kwhf.ptFooterBodySpacing = KoUnit::ptFromUnit( nFSpacing->value(), m_unit );
    kwhf.ptFootNoteBodySpacing = KoUnit::ptFromUnit( nFNSpacing->value(), m_unit);
    if ( rfFirst->isChecked() && rfEvenOdd->isChecked() )
        kwhf.footer = HF_FIRST_EO_DIFF;
    else if ( rfFirst->isChecked() )
        kwhf.footer = HF_FIRST_DIFF;
    else if ( rfEvenOdd->isChecked() )
        kwhf.footer = HF_EO_DIFF;
    else
        kwhf.footer = HF_SAME;

    return kwhf;
}

/*================ setup page size & margins tab ==================*/
void KoPageLayoutDia::setupTab1()
{
    QWidget *tab1 = addPage(i18n( "Page Size && Margins" ));

    QGridLayout *grid1 = new QGridLayout( tab1, 4, 2, KDialog::marginHint(), KDialog::spacingHint() );

    QLabel *lpgUnit;
    if ( !( flags & DISABLE_UNIT ) ) {
        // ------------- unit _______________
        QWidget* unitFrame = new QWidget( tab1 );
        grid1->addWidget( unitFrame, 0, 0 );
        QLayout* unitLayout = new QHBoxLayout( unitFrame, 5 );
        unitLayout->setAutoAdd( true );

        // label unit
        lpgUnit = new QLabel( i18n( "Unit:" ), unitFrame );
        lpgUnit->setAlignment( Qt::AlignRight || Qt::AlignVCenter );

        // combo unit
        cpgUnit = new QComboBox( false, unitFrame, "cpgUnit" );
        cpgUnit->insertItem( i18n( "Millimeters (mm)" ) );
        cpgUnit->insertItem( i18n( "Points (pt)" ) );
        cpgUnit->insertItem( i18n( "Inches (in)" ) );
        connect( cpgUnit, SIGNAL( activated( int ) ), this, SLOT( unitChanged( int ) ) );
    } else {
        QString str=KoUnit::unitDescription(m_unit);

        lpgUnit = new QLabel( i18n("All values are given in %1.").arg(str), tab1 );
        grid1->addWidget( lpgUnit, 0, 0 );
    }

    // -------------- page size -----------------
    QGroupBox *formatFrame = new QGroupBox( i18n( "Page Size" ), tab1 );
    grid1->addWidget( formatFrame, 1, 0 );
    QGridLayout *formatGrid = new QGridLayout( formatFrame, 3, 2, 
       2*KDialog::marginHint(), KDialog::spacingHint() );

    // label format
    QLabel *lpgFormat = new QLabel( i18n( "Size:" ), formatFrame );
    formatGrid->addWidget( lpgFormat, 0, 0 );

    // combo format
    cpgFormat = new QComboBox( false, formatFrame, "cpgFormat" );
    cpgFormat->setAutoResize( false );
    cpgFormat->insertStringList( KoPageFormat::allFormats() );
    formatGrid->addWidget( cpgFormat, 0, 1 );
    connect( cpgFormat, SIGNAL( activated( int ) ), this, SLOT( formatChanged( int ) ) );

    // label width
    QLabel *lpgWidth = new QLabel( i18n( "Width:" ), formatFrame );
    formatGrid->addWidget( lpgWidth, 1, 0 );

    // linedit width
    epgWidth = new KDoubleNumInput( formatFrame, "Width" );
    formatGrid->addWidget( epgWidth, 1, 1 );
    if ( layout.format != PG_CUSTOM )
        epgWidth->setEnabled( false );
    connect( epgWidth, SIGNAL( valueChanged(double) ), this, SLOT( widthChanged() ) );

    // label height
    QLabel *lpgHeight = new QLabel( i18n( "Height:" ), formatFrame );
    formatGrid->addWidget( lpgHeight, 2, 0 );

    // linedit height
    epgHeight = new KDoubleNumInput( formatFrame, "Height" );
    formatGrid->addWidget( epgHeight, 2, 1 );
    if ( layout.format != PG_CUSTOM )
        epgHeight->setEnabled( false );
    connect( epgHeight, SIGNAL( valueChanged(double ) ), this, SLOT( heightChanged() ) );

    // --------------- orientation ---------------
    QButtonGroup *orientFrame = new QButtonGroup( i18n( "Orientation" ), tab1 );
    grid1->addWidget( orientFrame, 2, 0 );
    QLayout *orientLayout = new QGridLayout( orientFrame, 2, 2,  
       2*KDialog::marginHint(), KDialog::spacingHint() );
    orientLayout->setAutoAdd( true );

    QLabel* lbPortrait = new QLabel( orientFrame );
    lbPortrait->setPixmap( QPixmap( UserIcon( "koPortrait" ) ) );
    rbPortrait = new QRadioButton( i18n("Portrait"), orientFrame );

    QLabel* lbLandscape = new QLabel( orientFrame );
    lbLandscape->setPixmap( QPixmap( UserIcon( "koLandscape" ) ) );
    rbLandscape = new QRadioButton( i18n("Landscape"), orientFrame );

    connect( rbPortrait, SIGNAL( clicked() ), this, SLOT( orientationChanged() ) );
    connect( rbLandscape, SIGNAL( clicked() ), this, SLOT( orientationChanged() ) );

    // --------------- page margins ---------------
    QButtonGroup *marginsFrame = new QButtonGroup( i18n( "Margins" ), tab1 );
    grid1->addWidget( marginsFrame, 3, 0 );
    QLayout *marginsLayout = new QGridLayout( marginsFrame, 4, 2, 
       2*KDialog::marginHint(), KDialog::spacingHint() );
    marginsLayout->setAutoAdd( true );

    // left margin
    new QLabel( i18n( "Left:" ), marginsFrame );
    ebrLeft = new KDoubleNumInput( marginsFrame, "Left" );
    connect( ebrLeft, SIGNAL( valueChanged( double ) ), this, SLOT( leftChanged() ) );
    if ( !enableBorders ) ebrLeft->setEnabled( false );

    // right margin
    new QLabel( i18n( "Right:" ), marginsFrame );
    ebrRight = new KDoubleNumInput( marginsFrame, "Right" );
    connect( ebrRight, SIGNAL( valueChanged( double ) ), this, SLOT( rightChanged() ) );
    if ( !enableBorders ) ebrRight->setEnabled( false );

    // top margin
    new QLabel( i18n( "Top:" ), marginsFrame );
    ebrTop = new KDoubleNumInput( marginsFrame, "Top" );
    connect( ebrTop, SIGNAL( valueChanged( double ) ), this, SLOT( topChanged() ) );
    if ( !enableBorders ) ebrTop->setEnabled( false );

    // bottom margin
    new QLabel( i18n( "Bottom:" ), marginsFrame );
    ebrBottom = new KDoubleNumInput( marginsFrame, "Bottom" );
    connect( ebrBottom, SIGNAL( valueChanged( double ) ), this, SLOT( bottomChanged() ) );
    if ( !enableBorders ) ebrBottom->setEnabled( false );

    // ------------- preview -----------
    pgPreview = new KoPagePreview( tab1, "Preview", layout );
    grid1->addMultiCellWidget( pgPreview, 1, 4, 1, 1 );

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
    if( layout.orientation == PG_PORTRAIT )
       rbPortrait->setChecked( true );
    else
       rbLandscape->setChecked( true );

    setValuesTab1Helper();

    pgPreview->setPageLayout( layout );
}

void KoPageLayoutDia::setValuesTab1Helper() {
    epgWidth->setValue( KoUnit::ptToUnit( layout.ptWidth, m_unit ) );
    epgHeight->setValue( KoUnit::ptToUnit( layout.ptHeight, m_unit ) );
    ebrLeft->setValue( KoUnit::ptToUnit( layout.ptLeft, m_unit ) );
    ebrRight->setValue( KoUnit::ptToUnit( layout.ptRight, m_unit ) );
    ebrTop->setValue( KoUnit::ptToUnit( layout.ptTop, m_unit ) );
    ebrBottom->setValue( KoUnit::ptToUnit( layout.ptBottom, m_unit ) );
}

/*================ setup header and footer tab ===================*/
void KoPageLayoutDia::setupTab2()
{
    QWidget *tab2 = addPage(i18n( "Header and Footer" ));
    QGridLayout *grid2 = new QGridLayout( tab2, 8, 6, KDialog::marginHint(), KDialog::spacingHint() );

    // ------------- header ---------------
    QLabel *lHead = new QLabel( i18n( "Head Line" ), tab2 );
    grid2->addMultiCellWidget( lHead, 0, 0, 0, 5 );

    QLabel *lHeadLeft = new QLabel( i18n( "Left:" ), tab2 );
    grid2->addMultiCellWidget( lHeadLeft, 1, 1, 0, 1 );

    eHeadLeft = new QLineEdit( tab2 );
    grid2->addMultiCellWidget( eHeadLeft, 2, 2, 0, 1 );
    eHeadLeft->setText( hf.headLeft );

    QLabel *lHeadMid = new QLabel( i18n( "Mid:" ), tab2 );
    grid2->addMultiCellWidget( lHeadMid, 1, 1, 2, 3 );

    eHeadMid = new QLineEdit( tab2 );
    grid2->addMultiCellWidget( eHeadMid, 2, 2, 2, 3 );
    eHeadMid->setText( hf.headMid );

    QLabel *lHeadRight = new QLabel( i18n( "Right:" ), tab2 );
    grid2->addMultiCellWidget( lHeadRight, 1, 1, 4, 5 );

    eHeadRight = new QLineEdit( tab2 );
    grid2->addMultiCellWidget( eHeadRight, 2, 2, 4, 5 );
    eHeadRight->setText( hf.headRight );

    // ------------- footer ---------------
    QLabel *lFoot = new QLabel( i18n( "Foot Line" ), tab2 );
    grid2->addMultiCellWidget( lFoot, 3, 3, 0, 5 );

    QLabel *lFootLeft = new QLabel( i18n( "Left:" ), tab2 );
    grid2->addMultiCellWidget( lFootLeft, 4, 4, 0, 1 );

    eFootLeft = new QLineEdit( tab2 );
    grid2->addMultiCellWidget( eFootLeft, 5, 5, 0, 1 );
    eFootLeft->setText( hf.footLeft );

    QLabel *lFootMid = new QLabel( i18n( "Mid:" ), tab2 );
    grid2->addMultiCellWidget( lFootMid, 4, 4, 2, 3 );

    eFootMid = new QLineEdit( tab2 );
    grid2->addMultiCellWidget( eFootMid, 5, 5, 2, 3 );
    eFootMid->setText( hf.footMid );

    QLabel *lFootRight = new QLabel( i18n( "Right:" ), tab2 );
    grid2->addMultiCellWidget( lFootRight, 4, 4, 4, 5 );

    eFootRight = new QLineEdit( tab2 );
    grid2->addMultiCellWidget( eFootRight, 5, 5, 4, 5 );
    eFootRight->setText( hf.footRight );

    QLabel *lMacros2 = new QLabel( i18n( "You can insert several tags in the text:" ), tab2 );
    grid2->addMultiCellWidget( lMacros2, 6, 6, 0, 5 );

    QLabel *lMacros3 = new QLabel( i18n("<qt><ul><li>&lt;sheet&gt; The sheet name</li>"
                           "<li>&lt;page&gt; The current page</li>"
                           "<li>&lt;pages&gt; The total number of pages</li>"
                           "<li>&lt;name&gt; The filename or URL</li>"
                           "<li>&lt;file&gt; The filename with complete path or the URL</li></ul></qt>"), tab2 );
    grid2->addMultiCellWidget( lMacros3, 7, 7, 0, 2, Qt::AlignTop );

    QLabel *lMacros4 = new QLabel( i18n("<qt><ul><li>&lt;time&gt; The current time</li>"
                           "<li>&lt;date&gt; The current date</li>"
                           "<li>&lt;author&gt; Your full name</li>"
                           "<li>&lt;org&gt; Your organization</li>"
                           "<li>&lt;email&gt; Your email address</li></ul></qt>"), tab2 );
    grid2->addMultiCellWidget( lMacros4, 7, 7, 3, 5, Qt::AlignTop );
}

/*================================================================*/
void KoPageLayoutDia::setupTab3()
{
    QWidget *tab3 = addPage(i18n( "Columns" ));

    QGridLayout *grid3 = new QGridLayout( tab3, 5, 2, KDialog::marginHint(), KDialog::spacingHint() );

    QLabel *lColumns = new QLabel( i18n( "Columns:" ), tab3 );
    grid3->addWidget( lColumns, 0, 0 );

    nColumns = new QSpinBox( 1, 16, 1, tab3 );
    grid3->addWidget( nColumns, 1, 0 );
    nColumns->setValue( cl.columns );
    connect( nColumns, SIGNAL( valueChanged( int ) ), this, SLOT( nColChanged( int ) ) );

    QString str = KoUnit::unitName( m_unit );

    QLabel *lCSpacing = new QLabel( i18n("Column spacing (%1):").arg(str), tab3 );
    grid3->addWidget( lCSpacing, 2, 0 );

    nCSpacing = new KDoubleNumInput( tab3, "" );
    grid3->addWidget( nCSpacing, 3, 0 );

    nCSpacing->setValue( KoUnit::ptToUnit( cl.ptColumnSpacing, m_unit ) );
    connect( nCSpacing, SIGNAL( valueChanged(double) ),
             this, SLOT( nSpaceChanged( double ) ) );

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
    QString str = KoUnit::unitName(m_unit);

    QWidget *tab4 = addPage(i18n( "Header and Footer" ));
    QGridLayout *grid4 = new QGridLayout( tab4, 4, 1, KDialog::marginHint(), KDialog::spacingHint() );

    QButtonGroup *gHeader = new QButtonGroup( i18n( "Header" ), tab4 );
    QGridLayout *headerGrid = new QGridLayout( gHeader, 4, 2, KDialog::marginHint(), KDialog::spacingHint() );

    rhFirst = new QCheckBox( i18n( "Different header for the first page" ), gHeader );
    gHeader->insert( rhFirst );
    headerGrid->addMultiCellWidget( rhFirst, 1, 1, 0, 1 );
    if ( kwhf.header == HF_FIRST_DIFF || kwhf.header == HF_FIRST_EO_DIFF )
        rhFirst->setChecked( true );

    rhEvenOdd = new QCheckBox( i18n( "Different header for even and odd pages" ), gHeader );
    gHeader->insert( rhEvenOdd );
    headerGrid->addMultiCellWidget( rhEvenOdd, 2, 2, 0, 1 );
    if ( kwhf.header == HF_EO_DIFF || kwhf.header == HF_FIRST_EO_DIFF )
        rhEvenOdd->setChecked( true );

    QLabel *lHSpacing = new QLabel( i18n("Spacing between header and body (%1):").arg(str), gHeader );
    lHSpacing->setAlignment( AlignRight | AlignVCenter );
    headerGrid->addWidget( lHSpacing, 4, 0 );

    nHSpacing = new KDoubleNumInput( gHeader, "" );
    headerGrid->addWidget( nHSpacing, 4, 1 );

    nHSpacing->setValue( KoUnit::ptToUnit( kwhf.ptHeaderBodySpacing, m_unit ) );

    headerGrid->addRowSpacing( 0, KDialog::spacingHint() );

    grid4->addWidget( gHeader, 0, 0 );

    QButtonGroup *gFooter = new QButtonGroup( i18n( "Footer" ), tab4 );
    QGridLayout *footerGrid = new QGridLayout( gFooter, 4, 2, KDialog::marginHint(), KDialog::spacingHint() );

    rfFirst = new QCheckBox( i18n( "Different footer for the first page" ), gFooter );
    gFooter->insert( rfFirst );
    footerGrid->addMultiCellWidget( rfFirst, 1, 1, 0, 1 );
    if ( kwhf.footer == HF_FIRST_DIFF || kwhf.footer == HF_FIRST_EO_DIFF )
        rfFirst->setChecked( true );

    rfEvenOdd = new QCheckBox( i18n( "Different footer for even and odd pages" ), gFooter );
    gFooter->insert( rfEvenOdd );
    footerGrid->addMultiCellWidget( rfEvenOdd, 2, 2, 0, 1 );
    if ( kwhf.footer == HF_EO_DIFF || kwhf.footer == HF_FIRST_EO_DIFF )
        rfEvenOdd->setChecked( true );

    QLabel *lFSpacing = new QLabel( i18n("Spacing between footer and body (%1):").arg(str), gFooter );
    lFSpacing->setAlignment( AlignRight | AlignVCenter );
    footerGrid->addWidget( lFSpacing, 4, 0 );

    nFSpacing = new KDoubleNumInput( gFooter, "" );
    footerGrid->addWidget( nFSpacing, 4, 1 );

    nFSpacing->setValue(KoUnit::ptToUnit( kwhf.ptFooterBodySpacing, m_unit ) );

    footerGrid->addRowSpacing( 0, KDialog::spacingHint() );

    grid4->addWidget( gFooter, 2, 0 );

    QButtonGroup *gFootNote = new QButtonGroup( i18n( "Foot-/Endnote" ), tab4 );
    QGridLayout *footNoteGrid = new QGridLayout( gFootNote, 2, 2, KDialog::marginHint(), KDialog::spacingHint() );

    QLabel *lFNSpacing = new QLabel( i18n("Spacing between footnote and body (%1):").arg(str), gFootNote );
    lFNSpacing->setAlignment( AlignRight | AlignVCenter );
    footNoteGrid->addWidget( lFNSpacing, 1, 0 );

    nFNSpacing = new KDoubleNumInput( gFootNote, "" );
    footNoteGrid->addWidget( nFNSpacing, 1, 1 );

    nFNSpacing->setValue(KoUnit::ptToUnit( kwhf.ptFootNoteBodySpacing, m_unit ) );

    footerGrid->addRowSpacing( 0, KDialog::spacingHint() );

    grid4->addWidget( gFootNote, 3, 0 );

    grid4->setRowStretch( 1, 1 ); // between the groupboxes
    grid4->setRowStretch( 2, 1 ); // between the groupboxes
    grid4->setRowStretch( 4, 10 ); // bottom
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

        epgWidth->setValue( KoUnit::ptToUnit( layout.ptWidth, m_unit ) );
        epgHeight->setValue( KoUnit::ptToUnit( layout.ptHeight, m_unit ) );

        updatePreview( layout );
    }
}

/*===================== format changed =============================*/

void KoPageLayoutDia::orientationChanged()
{
    layout.orientation = ( rbPortrait->isChecked() ) ?  PG_PORTRAIT : PG_LANDSCAPE;

    layout.ptWidth = KoUnit::ptFromUnit( epgWidth->value(), m_unit );
    layout.ptHeight = KoUnit::ptFromUnit( epgHeight->value(), m_unit );
    layout.ptLeft = KoUnit::ptFromUnit( ebrLeft->value(), m_unit );
    layout.ptRight = KoUnit::ptFromUnit( ebrRight->value(), m_unit );
    layout.ptTop = KoUnit::ptFromUnit( ebrTop->value(), m_unit );
    layout.ptBottom = KoUnit::ptFromUnit( ebrBottom->value(), m_unit );

    qSwap( layout.ptWidth, layout.ptHeight );

    if( layout.orientation == PG_PORTRAIT )
    {
       double tmp = layout.ptTop;
       layout.ptTop = layout.ptRight;
       layout.ptRight = layout.ptBottom;
       layout.ptBottom = layout.ptLeft;
       layout.ptLeft = tmp;
    }
    else
    {
       double tmp = layout.ptTop;
       layout.ptTop = layout.ptRight;
       layout.ptRight = layout.ptBottom;
       layout.ptBottom = layout.ptLeft;
       layout.ptLeft = tmp;
    }

    setValuesTab1();
    updatePreview( layout );
}

void KoPageLayoutDia::changed(KDoubleNumInput *line, double &pt) {

    if ( line->value() == 0 && retPressed )
        line->setValue( 0.0 );
    if ( line->value()<0)
        line->setValue( 0.0 );
    pt = KoUnit::ptFromUnit( line->value(), m_unit );
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
void KoPageLayoutDia::nSpaceChanged( double _val )
{
    cl.ptColumnSpacing = KoUnit::ptFromUnit( _val, m_unit );
    updatePreview( layout );
}

#include <koPageLayoutDia.moc>
