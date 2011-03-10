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

#include "SCFactory.h"
#include "SCDocument.h"
#include "SCAboutData.h"

#include <kiconloader.h>
#include <kcomponentdata.h>
#include <kstandarddirs.h>


KComponentData* SCFactory::s_instance = 0;
KAboutData* SCFactory::s_aboutData = 0;
KIconLoader* SCFactory::s_iconLoader = 0;

SCFactory::SCFactory(QObject* parent, const char* /*name*/)
    : KPluginFactory(*aboutData(), parent)
{
    (void)componentData();
}

SCFactory::~SCFactory()
{
    delete s_aboutData;
    s_aboutData = 0;
    delete s_instance;
    s_instance = 0;
}

QObject* SCFactory::create(const char* iface, QWidget* parentWidget, QObject *parent,
                             const QVariantList& args, const QString& keyword)
{
    Q_UNUSED(args);
    Q_UNUSED(keyword);
    bool bWantKoDocument = (strcmp(iface, "KoDocument") == 0);

    SCDocument *doc = new SCDocument(parentWidget, parent, !bWantKoDocument);

    if (!bWantKoDocument)
        doc->setReadWrite(false);

    return doc;
}

KAboutData* SCFactory::aboutData()
{
    if(!s_aboutData)
        s_aboutData = newShowcaseAboutData();

    return s_aboutData;
}

KIconLoader* SCFactory::iconLoader()
{
    if(!s_iconLoader)
    {
        s_iconLoader = new KIconLoader(componentData().componentName());
        // Tell the iconloader about share/apps/koffice/icons
        s_iconLoader->addAppDir("koffice");
    }

    return s_iconLoader;
}

const KComponentData &SCFactory::componentData()
{
    if (!s_instance) {
        s_instance = new KComponentData(aboutData());

        s_instance->dirs()->addResourceType("showcase_template", "data", "showcase/templates/");
        s_instance->dirs()->addResourceType("slideshow", "data", "showcase/slideshow/");
        s_instance->dirs()->addResourceType("styles", "data", "showcase/styles/");
    }
    return *s_instance;
}

#include "SCFactory.moc"
