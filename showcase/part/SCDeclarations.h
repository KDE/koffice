/* This file is part of the KDE project
 *  Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
 *  Contact: Amit Aggarwal <amitcs06@gmail.com>
 *            <amit.5.aggarwal@nokia.com>
 *  Copyright (C) 2010 Thorsten Zachmann <t.zachmann@zagge.de>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 */

#ifndef SCDECLARATIONS_H
#define SCDECLARATIONS_H

#include <QString>
#include <QHash>
#include <QVariant>

class KXmlElement;
class KoPALoadingContext;
class KoPASavingContext;

class SCDeclarations
{
public:
    /**
     * Presentation declaration type
     */
    enum Type {
        Footer,
        Header,
        DateTime
    };

    /**
     * Constructor
     */
    SCDeclarations();

    /**
     * Destructor
     */
    ~SCDeclarations();

    /**
     * loadOdfDeclaration
     * @param KXmlElement &body
     * @param KoPALoadingContext
     * @return bool value
     */
    bool loadOdf(const KXmlElement &body, KoPALoadingContext &context);

    /**
     * Save presentation:declaration entries
     */
    bool saveOdf(KoPASavingContext &paContext) const;

    /**
     * Similar to findStyle but for decls only.
     * \note Searches in content.xml added declaration!
     */
    const QString declaration(Type type, const QString &key);

private:
    QHash<Type, QHash<QString, QVariant> > m_declarations;
};

#endif /* SCDECLARATIONS_H */
