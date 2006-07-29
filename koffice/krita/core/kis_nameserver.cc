/*
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#include "kis_nameserver.h"

KisNameServer::KisNameServer(const QString& prefix, Q_INT32 seed)
{
    m_prefix = prefix;
    m_generator = seed;
}

KisNameServer::~KisNameServer()
{
}

QString KisNameServer::name()
{
    return m_prefix.arg(m_generator++);
}

Q_INT32 KisNameServer::currentSeed() const
{
    return m_generator;
}

Q_INT32 KisNameServer::number()
{
    return m_generator++;
}

void KisNameServer::rollback()
{
    m_generator--;
}

