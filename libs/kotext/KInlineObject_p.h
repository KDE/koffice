/* This file is part of the KDE project
 * Copyright (C) 2006-2011 Thomas Zander <zander@kde.org>
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
#ifndef KINLINEOBJECTPRIVATE_H
#define KINLINEOBJECTPRIVATE_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the KOdfText API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

class KTextInlineRdf;
class QTextDocument;

class KInlineObjectPrivate
{
public:
    KInlineObjectPrivate(KInlineObject *qq)
            : manager(0),
            q_ptr(qq),
            id(-1),
            positionInDocument(-1),
            propertyChangeListener(0),
            rdf(0),
            document(0)
    {
    }
    virtual ~KInlineObjectPrivate();

    KInlineTextObjectManager *manager;
    KInlineObject *q_ptr;
    int id;
    int positionInDocument;
    bool propertyChangeListener;
    KTextInlineRdf *rdf; //< An inline object might have RDF, we own it.
    QTextDocument *document;

    virtual QDebug printDebug(QDebug dbg) const;

    void callbackPositionChanged();

    Q_DECLARE_PUBLIC(KInlineObject)
};

#endif
