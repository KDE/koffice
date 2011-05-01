/* This file is part of the KDE project
   Copyright (C) 2003,2004 Ariya Hidayat <ariya@kde.org>
   Copyright (C) 2005 Tomas Mecir <mecirt@gmail.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; only
   version 2 of the License.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/


#ifndef KSPREAD_FUNCTION_REPOSITORY
#define KSPREAD_FUNCTION_REPOSITORY

#include <QSharedPointer>
#include <QStringList>

#include "kspread_export.h"

class KCFunction;
class KCFunctionDescription;

/**
 * \ingroup KCValue
 * The function repository.
 */
class KSPREAD_EXPORT FunctionRepository
{
public:
    FunctionRepository();
    ~FunctionRepository();

    static FunctionRepository *self();

    /**
     * Adds \p function to the repository.
     */
    void add(const QSharedPointer<KCFunction>& function);
    void add(KCFunctionDescription *desc);

    /**
     * Removes \p function from the repository.
     * The KCFunction object and the appropriate description will be destroyed.
     */
    void remove(const QSharedPointer<KCFunction>& function);

    QSharedPointer<KCFunction> function(const QString& name);

    KCFunctionDescription *functionInfo(const QString& name);

    /** return functions within a group, or all if no group given */
    QStringList functionNames(const QString& group = QString());

    const QStringList &groups() const;
    void addGroup(const QString& groupname);

    /**
     * Loads function descriptions from an XML file.
     */
    void loadFunctionDescriptions(const QString& filename);

private:

    class Private;
    Private * const d;

    // no copy or assign
    FunctionRepository(const FunctionRepository&);
    FunctionRepository& operator=(const FunctionRepository&);
};

#endif // KSPREAD_FUNCTION_REPOSITORY
