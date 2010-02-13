/* This file is part of the KDE project
Copyright (C) 2008 Pierre Stirnweiss \pierre.stirnweiss_koffice@gadz.org>

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

#include "KoTextSharedSavingData.h"

#include "KoGenChanges.h"
#include "KoTextSopranoRdfModel_p.h"

class KoTextSharedSavingData::Private
{
public:
    ~Private() {}

    KoGenChanges *changes;
    QMap<QString, QString> m_rdfIdMapping; //< This lets the RDF system know old->new xml:id
    Soprano::Model* m_rdfModel; //< This is so cut/paste can serialize the relevant RDF to the clipboard
};

KoTextSharedSavingData::KoTextSharedSavingData()
        : d(new Private())
{
}

KoTextSharedSavingData::~KoTextSharedSavingData()
{
    delete d;
}

void KoTextSharedSavingData::setGenChanges(KoGenChanges& changes) {
    d->changes = &changes;
}

KoGenChanges& KoTextSharedSavingData::genChanges()
{
    return *(d->changes);
}

void KoTextSharedSavingData::addRdfIdMapping(QString oldid, QString newid)
{
    d->m_rdfIdMapping[ oldid ] = newid;
}

QMap<QString, QString> KoTextSharedSavingData::getRdfIdMapping()
{
    return d->m_rdfIdMapping;
}

void KoTextSharedSavingData::setRdfModel(Soprano::Model* m)
{
    d->m_rdfModel = m;
}

Soprano::Model* KoTextSharedSavingData::rdfModel() const
{
    return d->m_rdfModel;
}

