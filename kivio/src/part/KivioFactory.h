/* This file is part of the KDE project
   Copyright (C)  2006 Peter Simonsson <peter.simonsson@gmail.com>

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

#ifndef KIVIOFACTORY_H
#define KIVIOFACTORY_H

#include <KoFactory.h>
#include "kivio_export.h"

class KComponentData;
class KAboutData;

class KIVIO_EXPORT KivioFactory : public KoFactory
{
  Q_OBJECT

  public:
    KivioFactory(QObject* parent = 0);
    ~KivioFactory();

    virtual KParts::Part* createPartObject(QWidget* parentWidget = 0,
                                           QObject* parent = 0,
                                           const char* classname = "KoDocument",
                                           const QStringList &args = QStringList());

    static const KComponentData &componentData();

    /// Creates a KAboutData but doesn't keep ownership
    static KAboutData* aboutData();

  private:
    static KComponentData* s_instance;
    static KAboutData* s_aboutData;
};

#endif
