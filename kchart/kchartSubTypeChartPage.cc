/* This file is part of the KDE project
   Copyright (C) 1999,2000 Matthias Kalle Dalheimer <kalle@kde.org>

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

#include <stdlib.h>

#include "kchartSubTypeChartPage.h"
#include "kchartSubTypeChartPage.moc"

#include <kapplication.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kdebug.h>
#include <qvbuttongroup.h>
#include <qradiobutton.h>
#include <qspinbox.h>
#include <qlabel.h>
#include <qhgroupbox.h>
#include <qlayout.h>
#include <qwhatsthis.h>
#include <qvbox.h>

#include "kchart_params.h"

#include "kchart_factory.h"

namespace KChart
{

KChartHiloSubTypeChartPage::KChartHiloSubTypeChartPage( KChartParams* params,
                                                        QWidget* parent ) :
    KChartSubTypeChartPage(  params, parent )
{
    QHBoxLayout* toplevel = new QHBoxLayout( this, 10 );
    QVButtonGroup* subtypeBG = new QVButtonGroup( i18n( "Sub-type" ), this );
    QWhatsThis::add(subtypeBG, i18n("Select the desired sub-type of a chart. The available sub-types depend on the chart type. Some chart types have no sub-type at all, in which case this configuration page is not shown."));
    toplevel->addWidget( subtypeBG, AlignCenter| AlignVCenter );
    normal = new QRadioButton( i18n( "Normal" ), subtypeBG );
    subtypeBG->insert( normal, KDChartParams::AreaNormal );
    stacked = new QRadioButton(i18n("HiLoClose"), subtypeBG );
    subtypeBG->insert( stacked, KDChartParams::AreaStacked );
    percent = new QRadioButton( i18n("HiLoOpenClose"), subtypeBG );
    subtypeBG->insert( percent, KDChartParams::AreaPercent );
    subtypeBG->setFixedWidth( subtypeBG->sizeHint().width() );
    connect( subtypeBG, SIGNAL( clicked( int ) ),
             this, SLOT( slotChangeSubType( int ) ) );

    QHGroupBox* exampleGB = new QHGroupBox( i18n( "Example" ), this );
    QWhatsThis::add(exampleGB, i18n("Preview the sub-type you choose."));
    toplevel->addWidget( exampleGB, 2 );
    exampleLA = new QLabel( exampleGB );
    exampleLA->setAlignment( AlignCenter | AlignVCenter );
    // PENDING(kalle) Make image scale with available space once Qt 2.2 is out.
}

void KChartHiloSubTypeChartPage::init()
{
    switch( m_params->hiLoChartSubType() ) {
    case KDChartParams::HiLoNormal:
        normal->setChecked( true );
        break;
    case KDChartParams::HiLoClose:
        stacked->setChecked( true );
        break;
    case KDChartParams::HiLoOpenClose:
        percent->setChecked( true );
        break;
    default:
        {
            kdDebug( 35001 ) << "Error in stack_type" << endl;
            abort();
            break;
        }
    }

    slotChangeSubType( m_params->hiLoChartSubType() );
}

void KChartHiloSubTypeChartPage::slotChangeSubType( int type )
{
    switch( type ) {
    case KDChartParams::HiLoNormal:
        exampleLA->setPixmap( UserIcon( "chart_hilo_normal", KChartFactory::global()  ) );
        break;
    case KDChartParams::HiLoClose:
        exampleLA->setPixmap( UserIcon( "chart_hilo_close", KChartFactory::global()  ) );
        break;
    case KDChartParams::HiLoOpenClose:
        exampleLA->setPixmap( UserIcon( "chart_hilo_openclose", KChartFactory::global()  ) );
        break;
    };
}



void KChartHiloSubTypeChartPage::apply()
{
    if( normal->isChecked() )
	m_params->setHiLoChartSubType( KDChartParams::HiLoNormal );
    else if( stacked->isChecked() )
        m_params->setHiLoChartSubType( KDChartParams::HiLoClose );
    else if( percent->isChecked() )
        m_params->setHiLoChartSubType( KDChartParams::HiLoOpenClose );
    else {
        kdDebug( 35001 ) << "Error in groupbutton" << endl;
    }
}

KChartAreaSubTypeChartPage::KChartAreaSubTypeChartPage( KChartParams* params,
                                                        QWidget* parent ) :
    KChartSubTypeChartPage(  params, parent )
{
    QHBoxLayout* toplevel = new QHBoxLayout( this, 10 );
    QVButtonGroup* subtypeBG = new QVButtonGroup( i18n( "Sub-type" ), this );
    QWhatsThis::add(subtypeBG, i18n("Select the desired sub-type of a chart. The available sub-types depend on the chart type. Some chart types have no sub-type at all, in which case this configuration page is not shown."));
    toplevel->addWidget( subtypeBG, AlignCenter| AlignVCenter );
    normal = new QRadioButton( i18n( "Normal" ), subtypeBG );
    subtypeBG->insert( normal, KDChartParams::AreaNormal );
    stacked = new QRadioButton( i18n( "Stacked" ), subtypeBG );
    subtypeBG->insert( stacked, KDChartParams::AreaStacked );
    percent = new QRadioButton( i18n( "Percent" ), subtypeBG );
    subtypeBG->insert( percent, KDChartParams::AreaPercent );
    subtypeBG->setFixedWidth( subtypeBG->sizeHint().width() );
    connect( subtypeBG, SIGNAL( clicked( int ) ),
             this, SLOT( slotChangeSubType( int ) ) );

    QHGroupBox* exampleGB = new QHGroupBox( i18n( "Example" ), this );
    QWhatsThis::add(exampleGB, i18n("Preview the sub-type you choose."));
    toplevel->addWidget( exampleGB, 2 );
    exampleLA = new QLabel( exampleGB );
    exampleLA->setAlignment( AlignCenter | AlignVCenter );
    // PENDING(kalle) Make image scale with available space once Qt 2.2 is out.
}


void KChartAreaSubTypeChartPage::init()
{
    switch( m_params->areaChartSubType() ) {
    case KDChartParams::AreaNormal:
        normal->setChecked( true );
        break;
    case KDChartParams::AreaStacked:
        stacked->setChecked( true );
        break;
    case KDChartParams::AreaPercent:
        percent->setChecked( true );
        break;
    default:
        {
            kdDebug( 35001 ) << "Error in stack_type" << endl;
            abort();
            break;
        }
    }

    slotChangeSubType( m_params->areaChartSubType() );
}

void KChartAreaSubTypeChartPage::slotChangeSubType( int type )
{
    switch( type ) {
    case KDChartParams::AreaNormal:
        exampleLA->setPixmap( UserIcon( "chart_area_normal", KChartFactory::global()  ) );
        break;
    case KDChartParams::AreaStacked:
        exampleLA->setPixmap( UserIcon( "chart_area_stacked", KChartFactory::global()  ) );
        break;
    case KDChartParams::AreaPercent:
        exampleLA->setPixmap( UserIcon( "chart_area_percent", KChartFactory::global()  ) );
        break;
    };
}



void KChartAreaSubTypeChartPage::apply()
{
    if( normal->isChecked() )
        m_params->setAreaChartSubType( KDChartParams::AreaNormal );
    else if( stacked->isChecked() )
        m_params->setAreaChartSubType( KDChartParams::AreaStacked );
    else if( percent->isChecked() )
        m_params->setAreaChartSubType( KDChartParams::AreaPercent );
    else {
        kdDebug( 35001 ) << "Error in groupbutton" << endl;
    }
}

KChartBarSubTypeChartPage::KChartBarSubTypeChartPage( KChartParams* params,
                                                      QWidget* parent ) :
    KChartSubTypeChartPage( params, parent )
{
    QHBoxLayout* toplevel = new QHBoxLayout( this, 10 );
    QVBox       *left = new QVBox( this );
    QVButtonGroup* subtypeBG = new QVButtonGroup( i18n( "Sub-type" ), left );
    QWhatsThis::add(subtypeBG, i18n("Select the desired sub-type of a chart. The available sub-types depend on the chart type. Some chart types have no sub-type at all, in which case this configuration page is not shown."));
    //toplevel->addWidget( subtypeBG, AlignCenter );
    toplevel->addWidget( left, AlignCenter );

    normal = new QRadioButton( i18n( "Normal" ), subtypeBG );
    subtypeBG->insert( normal, KDChartParams::BarNormal );
    stacked = new QRadioButton( i18n( "Stacked" ), subtypeBG );
    subtypeBG->insert( stacked, KDChartParams::BarStacked );
    percent = new QRadioButton( i18n( "Percent" ), subtypeBG );
    subtypeBG->insert( percent, KDChartParams::BarPercent );

    subtypeBG->setFixedWidth( subtypeBG->sizeHint().width() );
    connect( subtypeBG, SIGNAL( clicked( int ) ),
             this, SLOT( slotChangeSubType( int ) ) );

    //QHBox   *hbox = new QHBox( this );
    new QLabel( i18n( "Number of lines: "), left );
    m_numLines    = new QSpinBox( left );
    // FIXME: Use a grid layout instead
    new QLabel( "", left);
    left->setStretchFactor( left, 1 );

    QHGroupBox* exampleGB = new QHGroupBox( i18n( "Example" ), this );
    QWhatsThis::add(exampleGB, i18n("Preview the sub-type you choose."));
    toplevel->addWidget( exampleGB, 2 );
    exampleLA = new QLabel( exampleGB );
    exampleLA->setAlignment( AlignCenter | AlignVCenter );
}

void KChartBarSubTypeChartPage::init()
{
    // SUM is for areas only and therefore not configurable here.
    switch( m_params->barChartSubType() ) {
    case KDChartParams::BarNormal:
	normal->setChecked( true );
        break;
    case KDChartParams::BarStacked:
        stacked->setChecked( true );
        break;
    case KDChartParams::BarPercent:
        percent->setChecked( true );
        break;
    default:
        {
            kdDebug( 35001 ) << "Error in stack_type" << endl;
            break;
        }
    }

    m_numLines->setValue( m_params->barlinesNumLines() );

    slotChangeSubType( m_params->barChartSubType() );
}


void KChartBarSubTypeChartPage::slotChangeSubType( int type )
{
    switch( type ) {
    case KDChartParams::BarStacked:
	exampleLA->setPixmap( UserIcon( "chart_bar_layer", KChartFactory::global() ) );
	break;
    case KDChartParams::BarNormal:
	exampleLA->setPixmap( UserIcon( "chart_bar_beside", KChartFactory::global() ) );
	break;
    case KDChartParams::BarPercent:
	exampleLA->setPixmap( UserIcon( "chart_bar_percent", KChartFactory::global() ) );
	break;
    };
}


void KChartBarSubTypeChartPage::apply()
{
    if( normal->isChecked() ) {
        m_params->setBarChartSubType( KDChartParams::BarNormal );
    } else if( stacked->isChecked() ) {
        m_params->setBarChartSubType( KDChartParams::BarStacked );
    } else if( percent->isChecked() )	{
        m_params->setBarChartSubType( KDChartParams::BarPercent );
    } else {
        kdDebug( 35001 ) << "Error in groupbutton" << endl;
    }

    // FIXME: Error controls.
    m_params->setBarlinesNumLines( m_numLines->value() );
}

KChartLineSubTypeChartPage::KChartLineSubTypeChartPage( KChartParams* params,
                                                        QWidget* parent ) :
    KChartSubTypeChartPage(  params, parent )
{
    QHBoxLayout* toplevel = new QHBoxLayout( this, 10 );
    QVButtonGroup* subtypeBG = new QVButtonGroup( i18n( "Sub-type" ), this );
    QWhatsThis::add(subtypeBG, i18n("Select the desired sub-type of a chart. The available sub-types depend on the chart type. Some chart types have no sub-type at all, in which case this configuration page is not shown."));
    toplevel->addWidget( subtypeBG, AlignCenter| AlignVCenter );
    normal = new QRadioButton( i18n( "Normal" ), subtypeBG );
    subtypeBG->insert( normal, KDChartParams::AreaNormal );
    stacked = new QRadioButton( i18n( "Stacked" ), subtypeBG );
    subtypeBG->insert( stacked, KDChartParams::AreaStacked );
    percent = new QRadioButton( i18n( "Percent" ), subtypeBG );
    subtypeBG->insert( percent, KDChartParams::AreaPercent );
    subtypeBG->setFixedWidth( subtypeBG->sizeHint().width() );
    connect( subtypeBG, SIGNAL( clicked( int ) ),
             this, SLOT( slotChangeSubType( int ) ) );

    QHGroupBox* exampleGB = new QHGroupBox( i18n( "Example" ), this );
    QWhatsThis::add(exampleGB, i18n("Preview the sub-type you choose."));
    toplevel->addWidget( exampleGB, 2 );
    exampleLA = new QLabel( exampleGB );
    exampleLA->setAlignment( AlignCenter | AlignVCenter );
    // PENDING(kalle) Make image scale with available space once Qt 2.2 is out.
}

void KChartLineSubTypeChartPage::init()
{
    switch( m_params->lineChartSubType() ) {
    case KDChartParams::LineNormal:
        normal->setChecked( true );
        break;
    case KDChartParams::LineStacked:
        stacked->setChecked( true );
        break;
    case KDChartParams::LinePercent:
        percent->setChecked( true );
        break;
    default:
        {
            kdDebug( 35001 ) << "Error in stack_type" << endl;
            abort();
            break;
        }
    }

    slotChangeSubType( m_params->lineChartSubType() );
}

void KChartLineSubTypeChartPage::slotChangeSubType( int type )
{
    switch( type ) {
    case KDChartParams::AreaNormal:
        exampleLA->setPixmap( UserIcon( "chart_line_normal", KChartFactory::global()  ) );
        break;
    case KDChartParams::AreaStacked:
        exampleLA->setPixmap( UserIcon( "chart_line_stacked", KChartFactory::global()  ) );
        break;
    case KDChartParams::AreaPercent:
        exampleLA->setPixmap( UserIcon( "chart_line_percent", KChartFactory::global()  ) );
        break;
    };
}



void KChartLineSubTypeChartPage::apply()
{
    if( normal->isChecked() )
        m_params->setLineChartSubType( KDChartParams::LineNormal );
    else if( stacked->isChecked() )
        m_params->setLineChartSubType( KDChartParams::LineStacked );
    else if( percent->isChecked() )
        m_params->setLineChartSubType( KDChartParams::LinePercent );
    else {
        kdDebug( 35001 ) << "Error in groupbutton" << endl;
    }
}

KChartPolarSubTypeChartPage::KChartPolarSubTypeChartPage( KChartParams* params,
                                                        QWidget* parent ) :
    KChartSubTypeChartPage(  params, parent )
{
    QHBoxLayout* toplevel = new QHBoxLayout( this, 10 );
    QVButtonGroup* subtypeBG = new QVButtonGroup( i18n( "Sub-type" ), this );
    QWhatsThis::add(subtypeBG, i18n("Select the desired sub-type of a chart. The available sub-types depend on the chart type. Some chart types have no sub-type at all, in which case this configuration page is not shown."));
    toplevel->addWidget( subtypeBG, AlignCenter| AlignVCenter );
    normal = new QRadioButton( i18n( "Normal" ), subtypeBG );
    subtypeBG->insert( normal, KDChartParams::AreaNormal );
    stacked = new QRadioButton( i18n( "Stacked" ), subtypeBG );
    subtypeBG->insert( stacked, KDChartParams::AreaStacked );
    percent = new QRadioButton( i18n( "Percent" ), subtypeBG );
    subtypeBG->insert( percent, KDChartParams::AreaPercent );
    subtypeBG->setFixedWidth( subtypeBG->sizeHint().width() );
    connect( subtypeBG, SIGNAL( clicked( int ) ),
             this, SLOT( slotChangeSubType( int ) ) );

    QHGroupBox* exampleGB = new QHGroupBox( i18n( "Example" ), this );
    QWhatsThis::add(exampleGB, i18n("Preview the sub-type you choose."));
    toplevel->addWidget( exampleGB, 2 );
    exampleLA = new QLabel( exampleGB );
    exampleLA->setAlignment( AlignCenter | AlignVCenter );
    // PENDING(kalle) Make image scale with available space once Qt 2.2 is out.
}

void KChartPolarSubTypeChartPage::init()
{
    switch( m_params->polarChartSubType() ) {
    case KDChartParams::PolarNormal:
        normal->setChecked( true );
        break;
    case KDChartParams::PolarStacked:
        stacked->setChecked( true );
        break;
    case KDChartParams::PolarPercent:
        percent->setChecked( true );
        break;
    default:
        {
            kdDebug( 35001 ) << "Error in stack_type" << endl;
            abort();
            break;
        }
    }

    slotChangeSubType( m_params->lineChartSubType() );
}

void KChartPolarSubTypeChartPage::slotChangeSubType( int type )
{
    switch( type ) {
    case KDChartParams::PolarNormal:
        exampleLA->setPixmap( UserIcon( "chart_polar_normal", KChartFactory::global()  ) );
        break;
    case KDChartParams::PolarStacked:
        exampleLA->setPixmap( UserIcon( "chart_polar_stacked", KChartFactory::global()  ) );
        break;
    case KDChartParams::PolarPercent:
        exampleLA->setPixmap( UserIcon( "chart_polar_percent", KChartFactory::global()  ) );
        break;
    };
}



void KChartPolarSubTypeChartPage::apply()
{
    if( normal->isChecked() )
        m_params->setPolarChartSubType( KDChartParams::PolarNormal );
    else if( stacked->isChecked() )
        m_params->setPolarChartSubType( KDChartParams::PolarStacked );
    else if( percent->isChecked() )
        m_params->setPolarChartSubType( KDChartParams::PolarPercent );
    else {
        kdDebug( 35001 ) << "Error in groupbutton" << endl;
    }
}

}  //KChart namespace
