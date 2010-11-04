/* This file is part of the KDE project
   Copyright (C) 2006, 2007 Laurent Montel <montel@kde.org>

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

#include <kpluginfactory.h>
#include <KoStoreDevice.h>
#include <KoFilterChain.h>
#include <KoGlobal.h>
#include <kpr2odp.h>

K_PLUGIN_FACTORY(Kpr2OdpFactory, registerPlugin<Kpr2Odp>();)
K_EXPORT_PLUGIN(Kpr2OdpFactory("kofficefilters"))

Kpr2Odp::Kpr2Odp(QObject *parent, const QStringList&) :
        KoFilter(parent)
{
}

KoFilter::ConversionStatus Kpr2Odp::convert(const QByteArray& from, const QByteArray& to)
{
    if (from != "application/x-kpresenter"
            || to != "application/vnd.oasis.opendocument.presentation")
        return KoFilter::NotImplemented;

    KoStoreDevice* inpdev = m_chain->storageFile("root", KoStore::Read);
    if (!inpdev) {
        kError(30502) << "Unable to open input stream";
        return KoFilter::StorageCreationError;
    }

    inpdoc.setContent(inpdev);

    //convert it.

    return KoFilter::OK;
}

#include <kpr2odp.moc>
