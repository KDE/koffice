/* This file is part of the KDE project

   Copyright 2004 Ariya Hidayat <ariya@kde.org>
   Copyright 2003 Norbert Andres <nandres@web.de>
   Copyright 2002 Laurent Montel <montel@kde.org>
   Copyright 2002 Simon Hausmann <hausmann@kde.org>
   Copyright 2001-2002 Philipp Mueller <philipp.mueller@gmx.de>
   Copyright 2000 Stephan Kulow <coolo@kde.org>
   Copyright 1999-2000 David Faure <faure@kde.org>
   Copyright 1999 Torben Weis <weis@kde.org>

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
   Boston, MA 02110-1301, USA.
*/

#include "KSpreadDocIface.h"
#include <KoDocumentIface.h>

#include "Doc.h"
#include "Map.h"

#include <dcopclient.h>
#include <QColor>
#include <kdebug.h>

using namespace KSpread;

DocIface::DocIface( Doc* _doc )
    : KoDocumentIface( _doc )
{
    doc=_doc;
}

DCOPRef DocIface::map()
{
    return DCOPRef( kapp->dcopClient()->appId(),
                    doc->map()->dcopObject()->objId() );
}

void DocIface::changeDefaultGridPenColor( const QColor &_col)
{
    doc->setGridColor(_col);
}

QColor DocIface::pageBorderColor() const
{
    return doc->pageBorderColor();
}

bool DocIface::showFormulaBar()const
{
    return doc->showFormulaBar();
}

bool DocIface::showStatusBar()const
{
    return doc->showStatusBar();
}

bool DocIface::showTabBar()const
{
    return doc->showTabBar();
}

void DocIface::setShowVerticalScrollBar(bool _show)
{
    doc->setShowVerticalScrollBar(_show);
    doc->refreshInterface();
}

void DocIface::setShowHorizontalScrollBar(bool _show)
{
    doc->setShowHorizontalScrollBar(_show);
    doc->refreshInterface();
}

void DocIface::setShowColumnHeader(bool _show)
{
    doc->setShowColumnHeader(_show);
    doc->refreshInterface();
}

void DocIface::setShowRowHeader(bool _show)
{
    doc->setShowRowHeader(_show);
    doc->refreshInterface();
}

void DocIface::setShowTabBar(bool _show)
{
    doc->setShowTabBar(_show);
    doc->refreshInterface();
}

void DocIface::changePageBorderColor( const QColor & _color)
{
    doc->changePageBorderColor( _color);
    doc->refreshInterface();
}

void DocIface::addIgnoreWordAll( const QString &word)
{
    doc->addIgnoreWordAll( word );
}

void DocIface::clearIgnoreWordAll( )
{
    doc->clearIgnoreWordAll();
}

QStringList DocIface::spellListIgnoreAll() const
{
    return doc->spellListIgnoreAll();
}

void DocIface::addStringCompletion(const QString & stringCompletion)
{
    doc->addStringCompletion( stringCompletion );
}

int DocIface::zoom() const
{
    return doc->zoom();
}


QString DocIface::moveToValue()const
{
    switch(doc->moveToValue())
    {
    case Bottom:
        return QString("bottom");
        break;
    case Left:
        return QString("left");
        break;
    case Top:
        return QString("top");
        break;
    case Right:
        return QString("right");
        break;
    case BottomFirst:
        return QString("bottomFirst");
        break;
    }
    return QString();
}

void DocIface::setMoveToValue(const QString & move)
{
    if ( move.toLower()=="bottom" )
        doc->setMoveToValue(Bottom);
    else if ( move.toLower()=="top" )
        doc->setMoveToValue(Top);
    else if ( move.toLower()=="left" )
        doc->setMoveToValue(Left);
    else if ( move.toLower()=="right" )
        doc->setMoveToValue(Right);
    else if ( move.toLower()=="bottomfirst" )
        doc->setMoveToValue(BottomFirst);
}

void DocIface::setTypeOfCalc( const QString & calc )
{
    if ( calc.toLower()=="sum")
        doc->setTypeOfCalc(SumOfNumber );
    else if ( calc.toLower()=="min")
        doc->setTypeOfCalc( Min );
    else if ( calc.toLower()=="max")
        doc->setTypeOfCalc(Max );
    else if ( calc.toLower()=="average")
        doc->setTypeOfCalc(Average );
    else if ( calc.toLower()=="count")
        doc->setTypeOfCalc(Count );
    else if ( calc.toLower()=="none")
        doc->setTypeOfCalc(NoneCalc );
    else
        kDebug()<<"Error in setTypeOfCalc( const QString & calc ) :"<<calc<<endl;
    doc->refreshInterface();
}

QString DocIface::typeOfCalc() const
{
    switch( doc->getTypeOfCalc() )
    {
    case SumOfNumber:
        return QString("sum");
        break;
    case Min:
        return QString("min");
        break;
    case Max:
        return QString("max");
        break;
    case Average:
        return QString("average");
        break;
    case Count:
        return QString("count");
        break;
    case NoneCalc:
    default:
       return QString("none");
       break;
    }
}

