/* This file is part of the KDE project
   Copyright (C) 2001-2002 Lennart Kudling <kudling@kde.org>
   Copyright (C) 2002-2003 Rob Buis <buis@kde.org>
   Copyright (C) 2005 Laurent Montel <montel@kde.org>
   Copyright (C) 2005 Thomas Zander <zander@kde.org>
   Copyright (C) 2006 Inge Wallin <inge@lysator.liu.se>
   Copyright (C) 2006 Tim Beaulen <tbscope@gmail.com>
   Copyright (C) 2007 David Faure <faure@kde.org>
   Copyright (C) 2007 Matthias Kretz <kretz@kde.org>
   Copyright (C) 2007 Jan Hambrecht <jaham@gmx.net>

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

#ifndef __KARBON_FACTORY_H__
#define __KARBON_FACTORY_H__

#include <KPluginFactory>
#include <karbonui_export.h>

class KAboutData;
class KComponentData;

class KARBONUI_EXPORT KarbonFactory : public KPluginFactory
{
    Q_OBJECT

public:
    explicit KarbonFactory(QObject* parent = 0);
    ~KarbonFactory();

    virtual QObject* create(const char* iface, QWidget* parentWidget, QObject *parent, const QVariantList& args, const QString& keyword);

    static const KComponentData &componentData();
    static KAboutData* aboutData();

private:
    static KComponentData* s_instance;
    static KAboutData* s_aboutData;
};

#endif

