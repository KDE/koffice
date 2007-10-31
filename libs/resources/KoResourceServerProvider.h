/*  This file is part of the KDE project

    Copyright (c) 1999 Matthias Elter <elter@kde.org>
    Copyright (c) 2003 Patrick Julien <freak@codepimps.org>
    Copyright (c) 2005 Sven Langkamp <sven.langkamp@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef KORESOURCESERVERPROVIDER_H
#define KORESOURCESERVERPROVIDER_H

#include <koresource_export.h>

#include "KoResourceServer.h"
#include "KoPattern.h"
#include "KoAbstractGradient.h"
#include "KoColorSet.h"

class KORESOURCES_EXPORT KoResourceServerProvider
{
public:
    virtual ~KoResourceServerProvider();

    static KoResourceServerProvider* instance();

    KoResourceServer<KoPattern>* patternServer();
    KoResourceServer<KoAbstractGradient>* gradientServer();
    KoResourceServer<KoColorSet>* paletteServer();
private:
    KoResourceServerProvider();
    KoResourceServerProvider(const KoResourceServerProvider&);
    KoResourceServerProvider operator=(const KoResourceServerProvider&);

    QStringList getFileNames( const QString & extensions, const QString & type );

    static KoResourceServerProvider *m_singleton;
    KoResourceServer<KoPattern>* m_patternServer;
    KoResourceServer<KoAbstractGradient>* m_gradientServer;
    KoResourceServer<KoColorSet>* m_paletteServer;
};

#endif // KORESOURCESERVERPROVIDER_H
