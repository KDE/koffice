/* This file is part of the KDE project
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
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

#include "KNamedVariable_p.h"
#include "KInlineTextObjectManager.h"
#include <KProperties.h>

#include <KXmlReader.h> // for usage in Q_UNUSED
#include <KXmlWriter.h> // for usage in Q_UNUSED
#include <KShapeLoadingContext.h> // for usage in Q_UNUSED
#include <KShapeSavingContext.h> // for usage in Q_UNUSED

KNamedVariable::KNamedVariable(Property key, const QString &name)
        : KVariable(true),
        m_name(name),
        m_key(key)
{
}

void KNamedVariable::propertyChanged(Property property, const QVariant &value)
{
    if (property == m_key)
        setValue(qvariant_cast<QString>(value));
}

void KNamedVariable::setup()
{
    setValue(manager()->stringProperty(m_key));
}

bool KNamedVariable::loadOdf(const KXmlElement &element, KShapeLoadingContext &context)
{
    Q_UNUSED(element);
    Q_UNUSED(context);
    // TODO
    return false;
}

void KNamedVariable::saveOdf(KShapeSavingContext &context)
{
	KXmlWriter& bodyWriter = context.xmlWriter();
	bodyWriter.startElement("text:user-field-get");
	bodyWriter.addAttribute("text:name", m_name);
	bodyWriter.endElement(); //text:user-field-get
}
