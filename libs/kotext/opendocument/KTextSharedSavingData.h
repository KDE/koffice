/* This file is part of the KDE project
Copyright (C) 2004-2006 David Faure <faure@kde.org>
Copyright (C) 2007-2008 Thorsten Zachmann <zachmann@kde.org>

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

#ifndef KTEXTSHAREDSAVINGDATA_H
#define KTEXTSHAREDSAVINGDATA_H

#include <KSharedSavingData.h>
#include "kotext_export.h"

#include <QMap>

#define KODFTEXT_SHARED_SAVING_ID "KoTextSharedSavingId"

class KOdfGenericChanges;

namespace Soprano
{
class Model;
}

class KODFTEXT_EXPORT KTextSharedSavingData : public KSharedSavingData
{
public:
    KTextSharedSavingData();
    virtual ~KTextSharedSavingData();

    void setGenChanges(KOdfGenericChanges &changes);

    KOdfGenericChanges& genChanges();

    void addRdfIdMapping(QString oldid, QString newid);
    QMap<QString, QString> getRdfIdMapping();

    /**
     * The Rdf Model ownership is not taken, you must still delete it,
     * and you need to ensure that it lives longer than this object
     * unless you reset the model to 0.
     */
    void setRdfModel(Soprano::Model *m);
    Soprano::Model* rdfModel() const;

private:

    class Private;
    Private * const d;
};

#endif // KTEXTSHAREDSAVINGDATA_H
