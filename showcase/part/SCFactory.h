// -*- Mode: c++; c-basic-offset: 4; indent-tabs-mode: nil; tab-width: 4; -*-
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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#ifndef SCFACTORY_H
#define SCFACTORY_H

#include <KPluginFactory>
#include "showcase_export.h"

class KAboutData;
class KIconLoader;

class SHOWCASE_EXPORT SCFactory : public KPluginFactory
{
    Q_OBJECT
public:
    explicit SCFactory(QObject* parent = 0, const char* name = 0);
    ~SCFactory();

    virtual QObject* create(const char* iface, QWidget* parentWidget, QObject *parent, const QVariantList &args, const QString &keyword);
    static const KComponentData &componentData();

    // _Creates_ a KAboutData but doesn't keep ownership
    static KAboutData* aboutData();

    static KIconLoader* iconLoader();

private:
    static KComponentData* s_instance;
    static KAboutData* s_aboutData;
    static KIconLoader* s_iconLoader;
};

#endif
