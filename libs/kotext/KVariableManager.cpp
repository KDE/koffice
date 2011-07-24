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
#include "KVariableManager.h"
#include "KInlineTextObjectManager.h"
#include "KNamedVariable_p.h"
#include <KXmlWriter.h>

class KVariableManagerPrivate
{
public:
    KVariableManagerPrivate()
            : lastId(KInlineObject::VariableManagerStart) { }
    KInlineTextObjectManager *inlineObjectManager;
    QHash<QString, int> variableMapping;
    int lastId;
};

KVariableManager::KVariableManager(KInlineTextObjectManager *inlineObjectManager)
        : d(new KVariableManagerPrivate)
{
    d->inlineObjectManager = inlineObjectManager;
}

KVariableManager::~KVariableManager()
{
    delete d;
}

void KVariableManager::setValue(const QString &name, const QString &value)
{
    int key;
    // we store the mapping from name to key
    if (d->variableMapping.contains(name))
        key = d->variableMapping.value(name);
    else {
        key = d->lastId++;
        d->variableMapping.insert(name, key);
    }
    // the variable manager stores the actual value of the variable.
    d->inlineObjectManager->setProperty(static_cast<KInlineObject::Property>(key), value);
    emit valueChanged();
}

QString KVariableManager::value(const QString &name) const
{
    int key = d->variableMapping.value(name);
    if (key == 0)
        return QString();
    return d->inlineObjectManager->stringProperty(static_cast<KInlineObject::Property>(key));
}

void KVariableManager::remove(const QString &name)
{
    int key = d->variableMapping.value(name);
    if (key == 0)
        return;
    d->variableMapping.remove(name);
    d->inlineObjectManager->removeProperty(static_cast<KInlineObject::Property>(key));
}

int KVariableManager::usageCount(const QString &name) const
{
    Q_UNUSED(name);
    // TODO
    return 0;
}

KVariable *KVariableManager::createVariable(const QString &name) const
{
    int key = d->variableMapping.value(name);
    if (key == 0)
        return 0;
    return new KNamedVariable(static_cast<KInlineObject::Property>(key), name);
}

void KVariableManager::saveOdf(KXmlWriter* writer) const 
{
	writer->startElement("text:user-field-decls");
	foreach (const QString &name, variables()) {
		writer->startElement("text:user-field-decl");
		writer->addAttribute("text:name", name);
		// TODO : Support other value types based on the actual variable type
		writer->addAttribute("office:value-type", "string");
		writer->addAttribute("office:string-value", value(name));
		writer->endElement(); //text:user-field-decl
	}
	writer->endElement(); //text:user-field-decls
}

QList<QString> KVariableManager::variables() const
{
    return d->variableMapping.keys();
}
