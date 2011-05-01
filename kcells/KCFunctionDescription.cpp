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

// Local
#include "KCFunctionDescription.h"

#include <QDomElement>
#include <QDomNode>

#include <kdebug.h>
#include <klocale.h>

static ParameterType toType(const QString& type)
{
    if (type == "Boolean")
        return KCells_Boolean;
    if (type == "Int")
        return KCells_Int;
    if (type == "String")
        return KCells_String;
    if (type == "Any")
        return KCells_Any;

    return KCells_Float;
}

static QString toString(ParameterType type, bool range = false)
{
    if (!range) {
        switch (type) {
        case KCells_String:
            return i18n("Text");
        case KCells_Int:
            return i18n("Whole number (like 1, 132, 2344)");
        case KCells_Boolean:
            return i18n("A truth value (TRUE or FALSE)");
        case KCells_Float:
            return i18n("A floating point value (like 1.3, 0.343, 253 )");
        case KCells_Any:
            return i18n("Any kind of value");
        }
    } else {
        switch (type) {
        case KCells_String:
            return i18n("A range of strings");
        case KCells_Int:
            return i18n("A range of whole numbers (like 1, 132, 2344)");
        case KCells_Boolean:
            return i18n("A range of truth values (TRUE or FALSE)");
        case KCells_Float:
            return i18n("A range of floating point values (like 1.3, 0.343, 253 )");
        case KCells_Any:
            return i18n("A range of any kind of values");
        }
    }

    return QString();
}

KCFunctionParameter::KCFunctionParameter()
{
    m_type = KCells_Float;
    m_range = false;
}

KCFunctionParameter::KCFunctionParameter(const KCFunctionParameter& param)
{
    m_help = param.m_help;
    m_type = param.m_type;
    m_range = param.m_range;
}

KCFunctionParameter::KCFunctionParameter(const QDomElement& element)
{
    m_type  = KCells_Float;
    m_range = false;

    QDomNode n = element.firstChild();
    for (; !n.isNull(); n = n.nextSibling())
        if (n.isElement()) {
            QDomElement e = n.toElement();
            if (e.tagName() == "Comment")
                m_help = i18n(e.text().toUtf8());
            else if (e.tagName() == "Type") {
                m_type = toType(e.text());
                if (e.hasAttribute("range")) {
                    if (e.attribute("range").toLower() == "true")
                        m_range = true;
                }
            }
        }
}

KCFunctionDescription::KCFunctionDescription()
{
    m_type = KCells_Float;
}

KCFunctionDescription::KCFunctionDescription(const QDomElement& element)
{
    QDomNode n = element.firstChild();
    for (; !n.isNull(); n = n.nextSibling()) {
        if (!n.isElement())
            continue;
        QDomElement e = n.toElement();
        if (e.tagName() == "Name")
            m_name = e.text();
        else if (e.tagName() == "Type")
            m_type = toType(e.text());
        else if (e.tagName() == "Parameter")
            m_params.append(KCFunctionParameter(e));
        else if (e.tagName() == "Help") {
            QDomNode n2 = e.firstChild();
            for (; !n2.isNull(); n2 = n2.nextSibling()) {
                if (!n2.isElement())
                    continue;
                QDomElement e2 = n2.toElement();
                if (e2.tagName() == "Text")
                    m_help.append(i18n(e2.text().toUtf8()));
                else if (e2.tagName() == "Syntax")
                    m_syntax.append(i18n(e2.text().toUtf8()));
                else if (e2.tagName() == "Example")
                    m_examples.append(i18n(e2.text().toUtf8()));
                else if (e2.tagName() == "Related")
                    m_related.append(i18n(e2.text().toUtf8()));
            }
        }
    }
}

KCFunctionDescription::KCFunctionDescription(const KCFunctionDescription& desc)
{
    m_examples = desc.m_examples;
    m_related = desc.m_related;
    m_syntax = desc.m_syntax;
    m_help = desc.m_help;
    m_name = desc.m_name;
    m_type = desc.m_type;
}

QString KCFunctionDescription::toQML() const
{
    QString text("<qt><h1>");
    text += name();
    text += "</h1>";

    if (!m_help.isEmpty()) {
        text += "<p>";
        QStringList::ConstIterator it = m_help.begin();
        for (; it != m_help.end(); ++it) {
            text += *it;
            text += "<p>";
        }
        text += "</p>";
    }

    text += i18n("<p><b>Return type:</b> %1</p>", toString(type()));

    if (!m_syntax.isEmpty()) {
        text += "<h2>" + i18n("Syntax") + "</h2><ul>";
        QStringList::ConstIterator it = m_syntax.begin();
        for (; it != m_syntax.end(); ++it) {
            text += "<li>";
            text += *it;
        }
        text += "</ul>";
    }

    if (!m_params.isEmpty()) {
        text += "<h2>" + i18n("Parameters") + "</h2><ul>";
        QList<KCFunctionParameter>::ConstIterator it = m_params.begin();
        for (; it != m_params.end(); ++it) {
            text += i18n("<li><b>Comment:</b> %1", (*it).helpText());
            text += i18n("<br><b>Type:</b> %1", toString((*it).type(), (*it).hasRange()));
        }
        text += "</ul>";
    }

    if (!m_examples.isEmpty()) {
        text += "<h2>" + i18n("Examples") + "</h2><ul>";
        QStringList::ConstIterator it = m_examples.begin();
        for (; it != m_examples.end(); ++it) {
            text += "<li>";
            text += *it;
        }
        text += "</ul>";
    }

    if (!m_related.isEmpty()) {
        text += "<h2>" + i18n("Related Functions") + "</h2><ul>";
        QStringList::ConstIterator it = m_related.begin();
        for (; it != m_related.end(); ++it) {
            text += "<li>";
            text += "<a href=\"" + *it + "\">";
            text += *it;
            text += "</a>";
        }
        text += "</ul>";
    }

    text += "</qt>";
    return text;
}
