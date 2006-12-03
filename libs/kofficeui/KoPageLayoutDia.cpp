/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Reginald Stadlbauer <reggie@kde.org>
   Copyright (C) 2005 Thomas Zander <zander@kde.org>

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

// Description: Page Layout Dialog (sources)

/******************************************************************/

#include "KoPageLayoutDia.h"
#include "KoPageLayoutColumns.h"
#include "KoPageLayoutSize.h"
#include "KoPageLayoutHeader.h"
#include <KoUnit.h>
#include "KoUnitWidgets.h"

#include <klocale.h>
#include <kiconloader.h>
#include <kmessagebox.h>

#include <QLabel>
#include <QPainter>
#include <QLineEdit>
#include <QButtonGroup>
#include <QRadioButton>
#include <QCheckBox>


/******************************************************************/
/* class KoPagePreview                                            */
/******************************************************************/

/*===================== constrcutor ==============================*/
KoPagePreview::KoPagePreview( QWidget* parent, const char* /*name*/, const KoPageLayout& layout )
    : QGroupBox( i18n( "Page Preview" ), parent )
{
    setPageLayout( layout );
    columns = 1;
    setMinimumSize( 150, 150 );
}

/*====================== destructor ==============================*/
KoPagePreview::~KoPagePreview()
{
}

/*=================== set layout =================================*/
void KoPagePreview::setPageLayout( const KoPageLayout &layout )
{
    m_layout = layout;
    updateZoomedSize();
    update();
}

void KoPagePreview::updateZoomedSize()
{
    // resolution[XY] is in pixel per pt
    double resolutionX = POINT_TO_INCH( static_cast<double>(KoGlobal::dpiX()) );
    double resolutionY = POINT_TO_INCH( static_cast<double>(KoGlobal::dpiY()) );

    m_pageWidth = m_layout.ptWidth * resolutionX;
    m_pageHeight = m_layout.ptHeight * resolutionY;

    QRect cr = contentsRect();

    double zh = (cr.height()-30) / m_pageHeight;
    double zw = (cr.width()-30) / m_pageWidth;
    double z = qMin( zw, zh );

    m_pageWidth *= z;
    m_pageHeight *= z;

    m_textFrameX = m_layout.ptLeft * resolutionX * z;
    m_textFrameY = m_layout.ptTop * resolutionY * z;
    m_textFrameWidth = m_pageWidth - ( m_layout.ptLeft + m_layout.ptRight ) * resolutionX * z;
    m_textFrameHeight = m_pageHeight - ( m_layout.ptTop + m_layout.ptBottom ) * resolutionY * z;
}

/*=================== set layout =================================*/
void KoPagePreview::setPageColumns( const KoColumns &_columns )
{
    columns = _columns.columns;
    update();
}

/*======================== draw contents =========================*/

void KoPagePreview::paintEvent( QPaintEvent * event )
{
    QGroupBox::paintEvent( event );
    QPainter painter( this );

    double cw = m_textFrameWidth;
    if(columns!=1)
        cw/=static_cast<double>(columns);

    painter.setBrush( Qt::white );
    painter.setPen( QPen( Qt::black ) );

    QRect cr = contentsRect();

    int x=static_cast<int>( cr.left() + ( cr.width() - m_pageWidth ) * 0.5 );
    int y=static_cast<int>( cr.top() + ( cr.height() - m_pageHeight ) * 0.5 );
    int w=static_cast<int>(m_pageWidth);
    int h=static_cast<int>(m_pageHeight);
    //painter->drawRect( x + 1, y + 1, w, h);
    painter.drawRect( x, y, w, h );

    painter.setBrush( QBrush( Qt::black, Qt::HorPattern ) );
    if ( m_textFrameWidth == m_pageWidth || m_textFrameHeight == m_pageHeight )
        painter.setPen( Qt::NoPen );
    else
        painter.setPen( Qt::lightGray );

    for ( int i = 0; i < columns; ++i )
        painter.drawRect( x + static_cast<int>(m_textFrameX) + static_cast<int>(i * cw),
                          y + static_cast<int>(m_textFrameY), static_cast<int>(cw),
                          static_cast<int>(m_textFrameHeight) );
}

void KoPagePreview::resizeEvent ( QResizeEvent * event )
{
    updateZoomedSize();
}

/******************************************************************/
/* class KoPageLayoutDia                                          */
/******************************************************************/

/*==================== constructor ===============================*/
KoPageLayoutDia::KoPageLayoutDia( QWidget* parent, const char* name,
                                  const KoPageLayout& layout,
                                  const KoHeadFoot& hf, int tabs,
                                  KoUnit unit, bool modal )
    : KPageDialog( parent )
{
    setCaption( i18n("Page Layout") );
    setFaceType( KPageDialog::Tabbed );
    setButtons(  KDialog::Ok | KDialog::Cancel );
    setDefaultButton( KDialog::Ok );
    setObjectName( name );
    setModal( modal );
    flags = tabs;
    m_layout = layout;
    m_unit = unit;
    m_pageSizeTab = 0;
    m_columnsTab = 0;
    m_headerTab = 0;

    m_column.columns = 1;

    if ( tabs & FORMAT_AND_BORDERS ) setupTab1( true );
    if ( tabs & HEADER_AND_FOOTER ) setupTab2( hf );

    setFocusPolicy( Qt::StrongFocus );
    setFocus();
}

/*==================== constructor ===============================*/
KoPageLayoutDia::KoPageLayoutDia( QWidget* parent, const char* name,
                  const KoPageLayout& layout,
                  const KoHeadFoot& hf,
                  const KoColumns& columns,
                  const KoKWHeaderFooter& kwhf,
                  int tabs, KoUnit unit )
    : KPageDialog( parent )
{
    setCaption( i18n("Page Layout") );
    setFaceType( KPageDialog::Tabbed );
    setButtons(  KDialog::Ok | KDialog::Cancel );
    setDefaultButton( KDialog::Ok );
    setObjectName( name );
    setModal( true );

    flags = tabs;

    m_layout = layout;
    m_column = columns;
    m_unit = unit;
    m_pageSizeTab = 0;
    m_columnsTab = 0;
    m_headerTab = 0;

    if ( tabs & FORMAT_AND_BORDERS ) setupTab1( !( tabs & DISABLE_BORDERS ) );
    if ( tabs & HEADER_AND_FOOTER ) setupTab2( hf );
    if ( tabs & COLUMNS ) setupTab3();
    if ( tabs & KW_HEADER_AND_FOOTER ) setupTab4(kwhf);

    setFocusPolicy( Qt::StrongFocus );
    setFocus();
}

/*===================== destructor ===============================*/
KoPageLayoutDia::~KoPageLayoutDia()
{
}

/*======================= show dialog ============================*/
bool KoPageLayoutDia::pageLayout( KoPageLayout& layout, KoHeadFoot& hf, int tabs, KoUnit& unit, QWidget* parent )
{
    bool res = false;
    KoPageLayoutDia *dlg = new KoPageLayoutDia( parent, "PageLayout", layout, hf, tabs, unit );

    if ( dlg->exec() == QDialog::Accepted ) {
        res = true;
        if ( tabs & FORMAT_AND_BORDERS ) layout = dlg->layout();
        if ( tabs & HEADER_AND_FOOTER ) hf = dlg->headFoot();
        unit = dlg->unit();
    }

    delete dlg;

    return res;
}

/*======================= show dialog ============================*/
bool KoPageLayoutDia::pageLayout( KoPageLayout& layout, KoHeadFoot& hf, KoColumns& columns,
                                  KoKWHeaderFooter &_kwhf, int tabs, KoUnit& unit, QWidget* parent )
{
    bool res = false;
    KoPageLayoutDia *dlg = new KoPageLayoutDia( parent, "PageLayout", layout, hf, columns, _kwhf, tabs, unit );

    if ( dlg->exec() == QDialog::Accepted ) {
        res = true;
        if ( tabs & FORMAT_AND_BORDERS ) layout = dlg->layout();
        if ( tabs & HEADER_AND_FOOTER ) hf = dlg->headFoot();
        if ( tabs & COLUMNS ) columns = dlg->columns();
        if ( tabs & KW_HEADER_AND_FOOTER ) _kwhf = dlg->headerFooter();
        unit = dlg->unit();
    }

    delete dlg;

    return res;
}

/*===================== get a standard page layout ===============*/
KoPageLayout KoPageLayoutDia::standardLayout()
{
    return KoPageLayout::standardLayout();
}

/*====================== get header - footer =====================*/
KoHeadFoot KoPageLayoutDia::headFoot() const
{
    KoHeadFoot hf;
    hf.headLeft = eHeadLeft->text();
    hf.headMid = eHeadMid->text();
    hf.headRight = eHeadRight->text();
    hf.footLeft = eFootLeft->text();
    hf.footMid = eFootMid->text();
    hf.footRight = eFootRight->text();
    return hf;
}

/*================================================================*/
const KoKWHeaderFooter& KoPageLayoutDia::headerFooter()
{
    return m_headerTab->headerFooter();
}

/*================ setup page size & margins tab ==================*/
void KoPageLayoutDia::setupTab1( bool enableBorders )
{
    QWidget* tab1 = new QWidget();
    addPage( tab1, i18n( "Page Size && &Margins" ));
    QHBoxLayout *lay = new QHBoxLayout(tab1);
    m_pageSizeTab = new KoPageLayoutSize(tab1, m_layout, m_unit, m_column, !(flags & DISABLE_UNIT), enableBorders );
    lay->addWidget(m_pageSizeTab);
    tab1->setLayout( lay );

    m_pageSizeTab->show();
    connect (m_pageSizeTab, SIGNAL( propertyChange(KoPageLayout&)),
            this, SLOT (sizeUpdated( KoPageLayout&)));
}

void KoPageLayoutDia::sizeUpdated(KoPageLayout &layout) {
    m_layout.ptWidth = layout.ptWidth;
    m_layout.ptHeight = layout.ptHeight;
    m_layout.ptLeft = layout.ptLeft;
    m_layout.ptRight = layout.ptRight;
    m_layout.ptTop = layout.ptTop;
    m_layout.ptBottom = layout.ptBottom;
    m_layout.format = layout.format;
    m_layout.orientation = layout.orientation;
    if(m_columnsTab)
        m_columnsTab->setLayout(layout);
}

/*================ setup header and footer tab ===================*/
void KoPageLayoutDia::setupTab2( const KoHeadFoot& hf )
{
    QWidget *tab2 = new QWidget();
    addPage(tab2, i18n( "H&eader && Footer" ));
    QGridLayout *grid2 = new QGridLayout( tab2 );
    grid2->setSpacing( KDialog::spacingHint() );

    // ------------- header ---------------
    QGroupBox *gHead = new QGroupBox( i18n( "Head Line" ), tab2 );
    QGridLayout *headGrid = new QGridLayout( gHead );
    gHead->setLayout( headGrid );

    QLabel *lHeadLeft = new QLabel( i18n( "Left:" ), gHead );
    headGrid->addWidget( lHeadLeft, 0, 0 );

    eHeadLeft = new QLineEdit( gHead );
    headGrid->addWidget( eHeadLeft, 1, 0 );
    eHeadLeft->setText( hf.headLeft );

    QLabel *lHeadMid = new QLabel( i18n( "Mid:" ), gHead );
    headGrid->addWidget( lHeadMid, 0, 1 );

    eHeadMid = new QLineEdit( gHead );
    headGrid->addWidget( eHeadMid, 1, 1 );
    eHeadMid->setText( hf.headMid );

    QLabel *lHeadRight = new QLabel( i18n( "Right:" ), gHead );
    headGrid->addWidget( lHeadRight, 0, 2 );

    eHeadRight = new QLineEdit( gHead );
    headGrid->addWidget( eHeadRight, 1, 2 );
    eHeadRight->setText( hf.headRight );

    grid2->addWidget( gHead, 0, 2, 1, 1 );

    // ------------- footer ---------------
    QGroupBox *gFoot = new QGroupBox( i18n( "Foot Line" ), tab2 );
    QGridLayout *footGrid = new QGridLayout( gFoot );
    gFoot->setLayout( footGrid );

    QLabel *lFootLeft = new QLabel( i18n( "Left:" ), gFoot );
    footGrid->addWidget( lFootLeft, 0, 0 );

    eFootLeft = new QLineEdit( gFoot );
    footGrid->addWidget( eFootLeft, 1, 0 );
    eFootLeft->setText( hf.footLeft );

    QLabel *lFootMid = new QLabel( i18n( "Mid:" ), gFoot );
    footGrid->addWidget( lFootMid, 0, 1 );

    eFootMid = new QLineEdit( gFoot );
    footGrid->addWidget( eFootMid, 1, 1 );
    eFootMid->setText( hf.footMid );

    QLabel *lFootRight = new QLabel( i18n( "Right:" ), gFoot );
    footGrid->addWidget( lFootRight, 0, 2 );

    eFootRight = new QLineEdit( gFoot );
    footGrid->addWidget( eFootRight, 1, 2 );
    eFootRight->setText( hf.footRight );

    grid2->addWidget( gFoot, 2, 0, 2, 2 );

    QLabel *lMacros2 = new QLabel( i18n( "You can insert several tags in the text:" ), tab2 );
    grid2->addWidget( lMacros2, 4, 0, 1, 2 );

    QLabel *lMacros3 = new QLabel( i18n("<qt><ul><li>&lt;sheet&gt; The sheet name</li>"
                           "<li>&lt;page&gt; The current page</li>"
                           "<li>&lt;pages&gt; The total number of pages</li>"
                           "<li>&lt;name&gt; The filename or URL</li>"
                           "<li>&lt;file&gt; The filename with complete path or the URL</li></ul></qt>"), tab2 );
    grid2->addWidget( lMacros3, 5, 0, 2, 1, Qt::AlignTop );

    QLabel *lMacros4 = new QLabel( i18n("<qt><ul><li>&lt;time&gt; The current time</li>"
                           "<li>&lt;date&gt; The current date</li>"
                           "<li>&lt;author&gt; Your full name</li>"
                           "<li>&lt;org&gt; Your organization</li>"
                           "<li>&lt;email&gt; Your email address</li></ul></qt>"), tab2 );
    grid2->addWidget( lMacros4, 5, 1, 2, 1, Qt::AlignTop );
}

/*================================================================*/
void KoPageLayoutDia::setupTab3()
{
    QWidget *tab3 = new QWidget();
    addPage(tab3, i18n( "Col&umns" ));
    QHBoxLayout *lay = new QHBoxLayout(tab3);
    m_columnsTab = new KoPageLayoutColumns(tab3, m_column, m_unit, m_layout);
    m_columnsTab->layout()->setMargin(0);
    lay->addWidget(m_columnsTab);
    m_columnsTab->show();
    connect (m_columnsTab, SIGNAL( propertyChange(KoColumns&)),
            this, SLOT (columnsUpdated( KoColumns&)));
}

void KoPageLayoutDia::columnsUpdated(KoColumns &columns) {
    m_column.columns = columns.columns;
    m_column.ptColumnSpacing = columns.ptColumnSpacing;
    if(m_pageSizeTab)
        m_pageSizeTab->setColumns(columns);
}

/*================================================================*/
void KoPageLayoutDia::setupTab4(const KoKWHeaderFooter kwhf )
{
    QWidget *tab4 = new QWidget();
    addPage(tab4, i18n( "H&eader && Footer" ));
    QHBoxLayout *lay = new QHBoxLayout(tab4);
    m_headerTab = new KoPageLayoutHeader(tab4, m_unit, kwhf);
    m_headerTab->layout()->setMargin(0);
    lay->addWidget(m_headerTab);
    m_headerTab->show();

}


/* Validation when closing. Error messages are never liked, but
  better let the users enter all values in any order, and have one
  final validation, than preventing them from entering values. */
void KoPageLayoutDia::slotOk()
{
    if( m_pageSizeTab )
        m_pageSizeTab->queryClose();
    slotButtonClicked( KDialog::Ok );
}

#include "KoPageLayoutDia.moc"
