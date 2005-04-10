/* This file is part of the KDE project
   Copyright (C) 2002 Ulrich Kuettler <ulrich.kuettler@mailbox.tu-dresden.de>

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

#include <qcheckbox.h>
#include <qimage.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qpaintdevice.h>
#include <qrect.h>
#include <qvbuttongroup.h>
#include <qwidget.h>

#include <kapplication.h>
#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <knuminput.h>

#include "pngexportdia.h"


PNGExportDia::PNGExportDia( int width, int height, 
			    QWidget *parent, const char *name )
    : KDialogBase( parent, name, true,
		   i18n("PNG Export Filter Parameters" ), Ok|Cancel )
{
    kapp->restoreOverrideCursor();

    setupGUI();

    m_realWidth  = width;
    m_realHeight = height;
    m_widthEdit ->setValue( m_realWidth );
    m_heightEdit->setValue( m_realHeight  );
    m_percWidthEdit->setValue( 100 );
    m_percHeightEdit->setValue( 100 );

    connectAll();
    connect( m_proportional, SIGNAL( clicked() ),
             this,         SLOT( proportionalClicked() ) );
}


PNGExportDia::~PNGExportDia()
{
}


void PNGExportDia::setupGUI()
{
    //resize( size() );
    QWidget *page = new QWidget( this );
    setMainWidget(page);

#if 0
    QBoxLayout* mainLayout = new QVBoxLayout( page, 
					      KDialog::marginHint(), 
					      KDialog::spacingHint() );
#else
    QGridLayout *mainLayout = new QGridLayout( page, 5, 2,
					       KDialog::marginHint(), 
					       KDialog::spacingHint() );
#endif
    m_proportional = new QCheckBox( page, "proportional" );
    m_proportional->setText( i18n( "Keep ratio" ) );
    m_proportional->setChecked( true );
    mainLayout->addWidget( m_proportional, 0, 0 );

    QLabel* width = new QLabel( page, "width" );
    width->setText( i18n( "Width:" ) );
    m_widthEdit = new KIntNumInput( page, "widthEdit" );
    QLabel* height = new QLabel( page, "height" );
    height->setText( i18n( "Height:" ) );
    m_heightEdit = new KIntNumInput( page, "heightEdit" );

    mainLayout->addWidget( width,      1, 0 );
    mainLayout->addWidget( m_widthEdit,  1, 1 );
    mainLayout->addWidget( height,     2, 0 );
    mainLayout->addWidget( m_heightEdit, 2, 1 );

    QLabel* percentWidth = new QLabel( page, "PercentWidth" );
    percentWidth->setText( i18n( "Width (%):" ) );
    m_percWidthEdit = new KDoubleNumInput( page, "percWidthEdit" );
    QLabel* percentHeight = new QLabel( page, "PercentHeight" );
    percentHeight->setText( i18n( "Height (%):" ) );
    m_percHeightEdit = new KDoubleNumInput( page, "percHeightEdit" );

    mainLayout->addWidget( percentWidth,   3, 0 );
    mainLayout->addWidget( m_percHeightEdit, 3, 1 );
    mainLayout->addWidget( percentHeight,  4, 0 );
    mainLayout->addWidget( m_percWidthEdit,  4, 1 );

    /* Display the main layout */
    //mainLayout->addStretch( 5 );
    mainLayout->activate();
}


// ----------------------------------------------------------------
//                          public methods

int PNGExportDia::width()
{
    return m_widthEdit->value();
}


int PNGExportDia::height()
{
    return m_heightEdit->value();
}


// ----------------------------------------------------------------
//                            slots


void PNGExportDia::widthChanged( int width )
{
    disconnectAll();
    width = QMIN( width, m_realWidth * 10 );
    width = QMAX( width, m_realWidth / 10 );
    double percent = (100.0 * static_cast<double>( width ) 
		      / static_cast<double>( m_realWidth ));
    m_percWidthEdit->setValue(  percent  );
    if ( m_proportional->isChecked() ) {
        m_percHeightEdit->setValue( percent );
        int height = static_cast<int>( m_realHeight * percent / 100.0 );
        m_heightEdit->setValue(  height );
    }
    connectAll();
}


void PNGExportDia::heightChanged( int height )
{
    disconnectAll();
    height = QMIN( height, m_realHeight * 10 );
    height = QMAX( height, m_realHeight / 10 );
    double percent = (100.0 * static_cast<double>( height )
		      / static_cast<double>( m_realHeight ));
    m_percHeightEdit->setValue( percent  );
    if ( m_proportional->isChecked() ) {
        m_percWidthEdit->setValue(  percent  );
        int width = static_cast<int>( m_realWidth * percent / 100.0 );
        m_widthEdit->setValue( width );
    }
    connectAll();
}


void PNGExportDia::percentWidthChanged( double percent )
{
    disconnectAll();
    percent = QMIN( percent, 1000 );
    percent = QMAX( percent, 10 );
    int width = static_cast<int>( m_realWidth * percent / 100. );
    m_widthEdit->setValue(  width  );
    if ( m_proportional->isChecked() ) {
        int height = static_cast<int>( m_realHeight * percent / 100. );
        m_heightEdit->setValue(  height  );
        m_percHeightEdit->setValue(  percent );
    }
    connectAll();
}


void PNGExportDia::percentHeightChanged( double percent )
{
    disconnectAll();
    percent = QMIN( percent, 1000 );
    percent = QMAX( percent, 10 );
    if ( m_proportional->isChecked() ) {
        int width = static_cast<int>( m_realWidth * percent / 100. );
        m_widthEdit->setValue(  width  );
        m_percWidthEdit->setValue(  percent  );
    }
    int height = static_cast<int>( m_realHeight * percent / 100. );
    m_heightEdit->setValue(  height  );
    connectAll();
}


void PNGExportDia::proportionalClicked()
{
    if ( m_proportional->isChecked() ) {
        disconnectAll();
        int width = m_widthEdit->value( );
        width = QMIN( width, m_realWidth * 10 );
        width = QMAX( width, m_realWidth / 10 );
        double percent = (100.0 * static_cast<double>( width )
			  / static_cast<double>( m_realWidth ));
        m_percHeightEdit->setValue(  percent  );
        int height = static_cast<int>( m_realHeight * percent / 100. );
        m_heightEdit->setValue(  height  );
        connectAll();
    }
}


// ----------------------------------------------------------------
//                          private methods


void PNGExportDia::connectAll()
{
    connect( m_widthEdit,      SIGNAL( valueChanged(int) ),
             this,             SLOT( widthChanged( int ) ) );
    connect( m_heightEdit,     SIGNAL( valueChanged(int) ),
             this,             SLOT( heightChanged( int ) ) );
    connect( m_percWidthEdit,  SIGNAL( valueChanged(double) ),
             this,             SLOT( percentWidthChanged( double ) ) );
    connect( m_percHeightEdit, SIGNAL( valueChanged(double) ),
             this,             SLOT( percentHeightChanged(double ) ) );
}


void PNGExportDia::disconnectAll()
{
    disconnect( m_widthEdit,      SIGNAL( valueChanged(int) ),
		this,             SLOT( widthChanged( int ) ) );
    disconnect( m_heightEdit,     SIGNAL( valueChanged(int) ),
		this,             SLOT( heightChanged( int ) ) );
    disconnect( m_percWidthEdit,  SIGNAL( valueChanged(double) ),
		this,             SLOT( percentWidthChanged( double ) ) );
    disconnect( m_percHeightEdit, SIGNAL( valueChanged(double) ),
		this,             SLOT( percentHeightChanged(double ) ) );
}


#if 0
void PNGExportDia::slotOk()
{
    hide();
    //doc->setZoomAndResolution( 100, 600, 600 );
    //doc->setZoomAndResolution( 1000, QPaintDevice::x11AppDpiX(), QPaintDevice::x11AppDpiY() );
    //doc->newZoomAndResolution( false, false );
    int width = widthEdit->value();
    int height = heightEdit->value();
//     kdDebug( KFormula::DEBUGID ) << k_funcinfo
//                                  << "(" << width << " " << height << ")"
//                                  << endl;
//     width = realWidth;
//     height = realHeight;
    QImage image = formula->drawImage( width, height );
    if ( !image.save( _fileOut, "PNG" ) ) {
        KMessageBox::error( 0, i18n( "Failed to write file." ), i18n( "PNG Export Error" ) );
    }
    reject();
}
#endif

#include "pngexportdia.moc"
