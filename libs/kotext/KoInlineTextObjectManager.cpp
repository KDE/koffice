/* This file is part of the KDE project
 * Copyright (C) 2006-2007 Thomas Zander <zander@kde.org>
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
#include "KoInlineTextObjectManager.h"
#include "InsertNamedVariableAction.h"
#include "InsertTextReferenceAction.h"
#include "KoInlineObjectRegistry.h"
#include "KoTextLocator.h"

#include <QTextCursor>
#include <QPainter>
#include <kdebug.h>

KoInlineTextObjectManager::KoInlineTextObjectManager(QObject *parent)
    : QObject(parent),
    m_lastObjectId(0),
    m_variableManager(this)
{
}

KoInlineObject *KoInlineTextObjectManager::inlineTextObject(const QTextCharFormat &format) const {
    int id = format.intProperty(InlineInstanceId);
    if(id <= 0)
        return 0;
    return m_objects.value(id);
}

KoInlineObject *KoInlineTextObjectManager::inlineTextObject(const QTextCursor &cursor) const {
    return inlineTextObject(cursor.charFormat());
}

KoInlineObject *KoInlineTextObjectManager::inlineTextObject(int id) const {
    return m_objects.value(id);
}

void KoInlineTextObjectManager::insertInlineObject(QTextCursor &cursor, KoInlineObject *object) {
    QTextCharFormat cf;
    cf.setObjectType(1001);
    cf.setProperty(InlineInstanceId, ++m_lastObjectId);
    cursor.insertText(QString(0xFFFC), cf);
    object->setId(m_lastObjectId);
    m_objects.insert(m_lastObjectId, object);
    object->setManager(this);
    object->setup();
    if(object->propertyChangeListener())
        m_listeners.append(object);
}

void KoInlineTextObjectManager::setProperty(KoInlineObject::Property key, const QVariant &value) {
    if(m_properties.contains(key)) {
        if(value == m_properties.value(key))
            return;
        m_properties.remove(key);
    }
    m_properties.insert(key, value);
    foreach(KoInlineObject *obj, m_listeners)
        obj->propertyChanged(key, value);
}

QVariant KoInlineTextObjectManager::property(KoInlineObject::Property key) const {
    return m_properties.value(key);
}

int KoInlineTextObjectManager::intProperty(KoInlineObject::Property key) const {
    if(!m_properties.contains(key))
        return 0;
    return m_properties.value(key).toInt();
}

bool KoInlineTextObjectManager::boolProperty(KoInlineObject::Property key) const {
    if(!m_properties.contains(key))
        return false;
    return m_properties.value(key).toBool();
}

QString KoInlineTextObjectManager::stringProperty(KoInlineObject::Property key) const {
    if(!m_properties.contains(key))
        return QString();
    return qvariant_cast<QString>(m_properties.value(key));
}

const KoVariableManager *KoInlineTextObjectManager::variableManager() const {
    return &m_variableManager;
}

KoVariableManager *KoInlineTextObjectManager::variableManager() {
    return &m_variableManager;
}

void KoInlineTextObjectManager::removeProperty(KoInlineObject::Property key) {
    m_properties.remove(key);
}

QList<QAction*> KoInlineTextObjectManager::createInsertVariableActions(KoCanvasBase *host) const {
    QList<QAction *> answer = KoInlineObjectRegistry::instance()->createInsertVariableActions(host);
    int i=0;
    foreach(QString name, m_variableManager.variables()) {
        answer.insert(i++, new InsertNamedVariableAction(host, this, name));
    }

    answer.append(new InsertTextReferenceAction(host, this));
    return answer;
}

QList<KoTextLocator*> KoInlineTextObjectManager::textLocators() const {
    QList<KoTextLocator*> answers;
    foreach(KoInlineObject *object, m_objects) {
        KoTextLocator *tl = dynamic_cast<KoTextLocator*> (object);
        if(tl)
            answers.append(tl);
    }
    return answers;
}

#include "KoInlineTextObjectManager.moc"
