/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>

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

#include "main.h"
#include "kcalc.h"
#include "kspread_view.h"
#include "kspread_events.h"
#include "kspread_doc.h"
#include "kspread_locale.h"
#include "kspread_util.h"
#include "kspread_map.h"


#include <kdebug.h>

#include <stdio.h>

/***************************************************
 *
 * Factory
 *
 ***************************************************/

K_EXPORT_COMPONENT_FACTORY( libkspreadcalc, CalcFactory )

KInstance* CalcFactory::s_global = 0;

CalcFactory::CalcFactory( QObject* parent, const char* name )
    : KLibFactory( parent, name )
{
    s_global = new KInstance( "kspreadcalc" );
}

CalcFactory::~CalcFactory()
{
    delete s_global;
}

QObject* CalcFactory::createObject( QObject* parent, const char* name, const char* /*classname*/, const QStringList & )
{
    if ( !parent->inherits("KSpreadView") )
    {
        kdError() << "CalcFactory: KSpreadView expected. Parent is " << parent->className() << endl;
        return 0;
    }

    QObject *obj = new Calculator( (KSpreadView*)parent, name );
    return obj;
}

KInstance* CalcFactory::global()
{
    return s_global;
}

/***************************************************
 *
 * Calculator
 *
 ***************************************************/

Calculator::Calculator( KSpreadView* parent, const char* name )
    : KParts::Plugin( parent, name )
{
    m_calc = 0;
    m_view = parent;

    KGlobal::locale()->insertCatalogue("kspreadcalc_calc");
    parent->installEventFilter( this );

    (void)new KAction( i18n("Calculator"), SmallIcon("kcalc", CalcFactory::global()),
                       0, this, SLOT( showCalculator() ), actionCollection(), "kspreadcalc");
}

void Calculator::showCalculator()
{
    if ( m_calc )
    {
        m_calc->show();
        m_calc->raise();
        return;
    }

     m_calc = new QtCalculator( this, (KSpreadView*)parent() );
     m_calc->setFixedSize( 9 + 100 + 9 + 233 + 9, 239);
     m_calc->show();
     m_calc->raise();
}

Calculator::~Calculator()
{
}

bool Calculator::eventFilter( QObject*, QEvent* ev )
{
    if ( !m_calc )
        return FALSE;

    if ( KSpreadSelectionChanged::test( ev ) )
    {
        KSpreadSelectionChanged* event = (KSpreadSelectionChanged*)ev;

        // Selection cleared ?
        if ( event->rect().left() == 0 )
            return FALSE;

        KSpreadSheet* sheet = m_view->doc()->map()->findSheet( event->sheet() );
        if ( !sheet )
            return FALSE;

        // A single cell selected ?
        if ( event->rect().left() == event->rect().right() &&
             event->rect().top() == event->rect().bottom() )
        {
            KSpread::Cell* cell = sheet->cellAt( event->rect().left(), event->rect().top(), false );
            if ( !cell )
                return FALSE;

            double d;
            if ( cell->isEmpty() )
                d = 0;
            else
                d = cell->value().asFloat();
            m_calc->setValue( d );

            return FALSE;
        }

        // Multiple cells selected ?
        m_calc->setData( event->rect(), event->sheet().latin1() );
        QString str = util_rangeName( sheet, event->rect() );
        m_calc->setLabel( str.latin1() );

        return FALSE;
    }

    return FALSE;
}

/***************************************************
 *
 * QtCalculator
 *
 ***************************************************/

/**
 * This is a hook that makes the calculator to
 * ask KSpread for the values of the selected cells.
 */
void QtCalculator::useData()
{
    stats.clearAll();

    // How many cells ?
    int len = ( sheet_range.right() - sheet_range.left() + 1 ) *
              ( sheet_range.bottom() - sheet_range.top() + 1 );

    double *v = new double[ len ];
    int n = 0;
    for( int x = sheet_range.left(); x <= sheet_range.right(); x++ )
        for( int y = sheet_range.top(); y <= sheet_range.bottom(); y++ )
        {
            KSpreadView* view = corba->view();
            KSpreadSheet* sheet = view->doc()->map()->findSheet( sheet_name );
            if ( !sheet )
                return;
            KSpread::Cell* cell = sheet->cellAt( x, y, false );
            if ( !cell )
                return;

            v[n++] = cell->value().asFloat();
        }

    for( int i = 0; i < n; i++ )
        stats.enterData( v[i] );

    delete []v;

    sheet_name = QString::null;
}

#include "main.moc"
