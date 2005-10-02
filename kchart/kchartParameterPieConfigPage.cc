/* This file is part of the KDE project
   Copyright (C) 2001,2002,2003,2004 Laurent Montel <montel@kde.org>

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

#include "kchartParameterPieConfigPage.h"

#include "kchartParameterPieConfigPage.moc"

#include <kapplication.h>
#include <klocale.h>
#include <kdebug.h>
#include <kdialog.h>

#include <qlayout.h>
#include <qlabel.h>
#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qlineedit.h>
#include <qradiobutton.h>
#include <qspinbox.h>
#include <qvbuttongroup.h>

#include "kchart_params.h"

namespace KChart
{

KChartParameterPieConfigPage::KChartParameterPieConfigPage( KChartParams* params,
                                                            QWidget* parent ) :
    QWidget( parent ),_params( params )
{
    QVBoxLayout  *toplevel = new QVBoxLayout( this, 10 );
    QVBoxLayout  *grid1 = new QVBoxLayout(this);
    toplevel->addLayout( grid1 );

    QVButtonGroup* gb = new QVButtonGroup( i18n( "Parameter" ), this );
    grid1->addWidget(gb);

    pie3d = new QCheckBox(i18n("Pie 3D"), gb);
    drawShadowColor=new QCheckBox(i18n("Draw shadow color"), gb);


    QLabel *label = new QLabel( i18n( "Explode factor (%):" ), gb );
    explode = new QSpinBox(0, 100, 1, gb);

    label = new QLabel( i18n( "Start angle:" ), gb );
    angle = new QSpinBox(0, 90, 1, gb);

    label = new QLabel( i18n( "3D-depth:" ), gb );
    depth = new QSpinBox(0, 40, 1, gb);

    grid1->activate();

    connect(pie3d,SIGNAL(toggled ( bool )),this, SLOT(active3DPie(bool)));
}

void KChartParameterPieConfigPage::active3DPie(bool b)
{
    drawShadowColor->setEnabled(b);
    depth->setEnabled(b);
}

void KChartParameterPieConfigPage::init()
{
    bool state=_params->threeDPies();
    pie3d->setChecked(state);
    depth->setEnabled(state);
    active3DPie(state);

    explode->setValue((int)(_params->explodeFactor() * 100));
    depth->setValue( _params->threeDPieHeight() );
    drawShadowColor->setChecked(_params->threeDShadowColors());
    angle->setValue( _params->pieStart() );

}


void KChartParameterPieConfigPage::apply()
{
     _params->setThreeDPies( pie3d->isChecked() );
     if( _params->threeDPies() )	{
        _params->setThreeDPieHeight( depth->value() );
    }
    _params->setThreeDShadowColors( drawShadowColor->isChecked());
    _params->setExplodeFactor(((double)(explode->value()))/100.0);
    _params->setPieStart( angle->value() );
}

}  //KChart namespace
