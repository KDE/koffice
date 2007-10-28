/* This file is part of the KDE project

   Copyright 1999-2007  Kalle Dalheimer <kalle@kde.org>
   Copyright 2005-2007  Inge Wallin <inge@lysator.liu.se>

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


#ifndef KCHART_FACTORY_H
#define KCHART_FACTORY_H


#include <KoFactory.h>
#include "kchart_export.h"

class KComponentData;
class KIconLoader;
class KAboutData;

namespace KChart
{

class KCHARTCOMMON_EXPORT KChartFactory : public KoFactory
{
    Q_OBJECT
public:
    explicit KChartFactory( QObject* parent = 0, const char* name = 0 );
    virtual ~KChartFactory();

    virtual KParts::Part  *createPartObject( QWidget* = 0,
					     QObject* parent = 0,
					     const char* classname = "KoDocument",
					     const QStringList &args = QStringList() );

    static const KComponentData &global();

    static KIconLoader* iconLoader();

    // _Creates_ a KAboutData but doesn't keep ownership
    static KAboutData* aboutData();
};

}  //namespace KChart

#endif
