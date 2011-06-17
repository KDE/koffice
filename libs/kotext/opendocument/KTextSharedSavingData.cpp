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

#include "KTextSharedSavingData.h"

#include "KOdfGenericChanges.h"
#include "KoTextSopranoRdfModel_p.h"

class KTextSharedSavingData::Private
{
public:
    ~Private() {}

    KOdfGenericChanges *changes;
    QMap<QString, QString> m_rdfIdMapping; //< This lets the RDF system know old->new xml:id
    Soprano::Model* m_rdfModel; //< This is so cut/paste can serialize the relevant RDF to the clipboard
};

KTextSharedSavingData::KTextSharedSavingData()
        : d(new Private())
{
}

KTextSharedSavingData::~KTextSharedSavingData()
{
    delete d;
}

void KTextSharedSavingData::setGenChanges(KOdfGenericChanges& changes) {
    d->changes = &changes;
}

KOdfGenericChanges& KTextSharedSavingData::genChanges()
{
    return *(d->changes);
}

void KTextSharedSavingData::addRdfIdMapping(QString oldid, QString newid)
{
    d->m_rdfIdMapping[ oldid ] = newid;
}

QMap<QString, QString> KTextSharedSavingData::getRdfIdMapping()
{
    return d->m_rdfIdMapping;
}

void KTextSharedSavingData::setRdfModel(Soprano::Model* m)
{
    d->m_rdfModel = m;
}

Soprano::Model* KTextSharedSavingData::rdfModel() const
{
    return d->m_rdfModel;
}

