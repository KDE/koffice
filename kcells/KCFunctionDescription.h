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


#ifndef KCELLS_FUNCTION_DESCRIPTION
#define KCELLS_FUNCTION_DESCRIPTION

#include <QList>
#include <QStringList>

#include "kcells_export.h"

class QDomElement;

enum ParameterType { KCells_Int, KCells_Float, KCells_String, KCells_Boolean, KCells_Any };

/**
 * \ingroup KCValue
 * A function parameter.
 */
class KCELLS_EXPORT KCFunctionParameter
{
public:
    KCFunctionParameter();
    KCFunctionParameter(const KCFunctionParameter& param);
    KCFunctionParameter(const QDomElement& element);

    QString helpText() const {
        return m_help;
    }
    ParameterType type() const {
        return m_type;
    }
    bool hasRange() const {
        return m_range;
    }

private:
    QString m_help;
    ParameterType m_type;
    bool m_range;
};

/**
 * \ingroup KCValue
 * A function description.
 */
class KCELLS_EXPORT KCFunctionDescription
{
public:
    KCFunctionDescription();
    KCFunctionDescription(const QDomElement& element);
    KCFunctionDescription(const KCFunctionDescription& desc);

    const QStringList& examples() {
        return m_examples;
    }
    const QStringList& syntax() {
        return m_syntax;
    }
    const QStringList& related() {
        return m_related;
    }
    const QStringList& helpText() const {
        return m_help;
    }
    QString name() const {
        return m_name;
    }
    ParameterType type() const {
        return m_type;
    }

    int params() const {
        return m_params.count();
    }
    KCFunctionParameter& param(int i) {
        return m_params[ i ];
    }

    void setGroup(const QString& g) {
        m_group = g;
    }
    QString group() const {
        return m_group;
    }

    QString toQML() const;

private:
    QString m_group;
    QStringList m_examples;
    QStringList m_syntax;
    QStringList m_related;
    QStringList m_help;
    QString m_name;
    ParameterType m_type;
    QList<KCFunctionParameter> m_params;
};

#endif // KCELLS_FUNCTION_DESCRIPTION
