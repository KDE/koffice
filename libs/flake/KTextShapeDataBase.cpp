/* This file is part of the KDE project
 * Copyright (C) 2006, 2009-2010 Thomas Zander <zander@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "KTextShapeDataBase.h"
#include "KTextShapeDataBase_p.h"

KTextShapeDataBasePrivate::KTextShapeDataBasePrivate()
        : document(0),
        textAlignment(Qt::AlignLeft | Qt::AlignTop)
{
}

KTextShapeDataBasePrivate::~KTextShapeDataBasePrivate()
{
}

KTextShapeDataBase::KTextShapeDataBase(KTextShapeDataBasePrivate &dd)
    : d_ptr(&dd)
{
}

KTextShapeDataBase::~KTextShapeDataBase()
{
    delete d_ptr;
}

QTextDocument *KTextShapeDataBase::document() const
{
    Q_D(const KTextShapeDataBase);
    return d->document;
}

void KTextShapeDataBase::setShapeMargins(const KInsets &margins)
{
    Q_D(KTextShapeDataBase);
    d->margins = margins;
}

KInsets KTextShapeDataBase::shapeMargins() const
{
    Q_D(const KTextShapeDataBase);
    return d->margins;
}

void KTextShapeDataBase::setVerticalAlignment(Qt::Alignment alignment)
{
    Q_D(KTextShapeDataBase);
    d->textAlignment = (d->textAlignment & Qt::AlignHorizontal_Mask)
        | (alignment & Qt::AlignVertical_Mask);
}

Qt::Alignment KTextShapeDataBase::verticalAlignment() const
{
    Q_D(const KTextShapeDataBase);
    return d->textAlignment & Qt::AlignVertical_Mask;
}

#include <KTextShapeDataBase.moc>
