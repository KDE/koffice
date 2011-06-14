/* This file is part of the KDE project
 * Copyright (C) 2006-2009 Thomas Zander <zander@kde.org>
 * Copyright (C) 2008 Thorsten Zachmann <zachmann@kde.org>
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
#include "InsertNamedVariableAction_p.h"
#include "InsertTextReferenceAction_p.h"
#include "InsertTextLocator_p.h"
#include "KInlineObjectRegistry.h"
#include "KoTextLocator.h"
#include "KoBookmark.h"

#include <QTextCursor>
#include <QPainter>

KoInlineTextObjectManager::KoInlineTextObjectManager(QObject *parent)
        : QObject(parent),
        m_lastObjectId(0),
        m_variableManager(this)
{
}

KInlineObject *KoInlineTextObjectManager::inlineTextObject(const QTextCharFormat &format) const
{
    int id = format.intProperty(InlineInstanceId);
    if (id <= 0)
        return 0;
    return m_objects.value(id);
}

KInlineObject *KoInlineTextObjectManager::inlineTextObject(const QTextCursor &cursor) const
{
    return inlineTextObject(cursor.charFormat());
}

KInlineObject *KoInlineTextObjectManager::inlineTextObject(int id) const
{
    return m_objects.value(id);
}

void KoInlineTextObjectManager::insertInlineObject(QTextCursor &cursor, KInlineObject *object, QTextCharFormat charFormat)
{
    QTextCharFormat oldCf = cursor.charFormat();
    QTextCharFormat cf(charFormat);
    cf.setObjectType(QTextFormat::UserObject + 1);
    cf.setProperty(InlineInstanceId, ++m_lastObjectId);
    cursor.insertText(QString(QChar::ObjectReplacementCharacter), cf);
    object->setId(m_lastObjectId);
    m_objects.insert(m_lastObjectId, object);
    object->setManager(this);
    object->setDocument(cursor.document());
    object->setTextPosition(cursor.position() - 1);
    object->setup();
    if (object->propertyChangeListener()) {
        m_listeners.append(object);
        QHash<int, QVariant>::iterator i;
        for (i = m_properties.begin(); i != m_properties.end(); ++i)
            object->propertyChanged((KInlineObject::Property)(i.key()), i.value());
    }

    KoBookmark *bookmark = dynamic_cast<KoBookmark *>(object);
    if (bookmark
            && (bookmark->type() == KoBookmark::StartBookmark
                || bookmark->type() == KoBookmark::SinglePosition))
        m_bookmarkManager.insert(bookmark);

    // reset to use old format so that the InlineInstanceId is no longer set.
    cursor.setCharFormat(oldCf);
}

bool KoInlineTextObjectManager::removeInlineObject(QTextCursor &cursor)
{
    KInlineObject *object = inlineTextObject(cursor);
    if (object->propertyChangeListener()) {
        int position = m_listeners.indexOf(object);
        m_listeners.removeAt(position);
    }

    // what if a KoTextLocator is removed? what to do with KoTextReference?
    QTextCharFormat format = cursor.charFormat();
    int id = format.intProperty(InlineInstanceId);
    if (id <= 0)
        return false;

    int removed = m_objects.remove(id);

    KoBookmark *bookmark = dynamic_cast<KoBookmark *>(object);
    if (bookmark) {
        if (bookmark->type() == KoBookmark::StartBookmark) {
            m_bookmarkManager.remove(bookmark->name());
            KoBookmark *endBookmark = bookmark->endBookmark();
            endBookmark->setType(KoBookmark::SinglePosition);
            m_bookmarkManager.insert(endBookmark);
        } else if (bookmark->type() == KoBookmark::EndBookmark) {
            KoBookmark *startBookmark = m_bookmarkManager.bookmark(bookmark->name());
            startBookmark->setType(KoBookmark::SinglePosition);
        } else
            m_bookmarkManager.remove(bookmark->name());
    }

    delete object;
    object = 0;

    if (removed != 0) {
        cursor.deletePreviousChar();
        return true;
    }
    return false;
}

void KoInlineTextObjectManager::removeInlineObject(KInlineObject *object)
{
    //Q_ASSERT(object);
    if (object)
        m_objects.remove(object->id());
    // TODO dirty the document somehow
}

void KoInlineTextObjectManager::setProperty(KInlineObject::Property key, const QVariant &value)
{
    if (m_properties.contains(key)) {
        if (value == m_properties.value(key))
            return;
        m_properties.remove(key);
    }
    m_properties.insert(key, value);
    foreach (KInlineObject *obj, m_listeners)
        obj->propertyChanged(key, value);
}

QVariant KoInlineTextObjectManager::property(KInlineObject::Property key) const
{
    return m_properties.value(key);
}

int KoInlineTextObjectManager::intProperty(KInlineObject::Property key) const
{
    if (!m_properties.contains(key))
        return 0;
    return m_properties.value(key).toInt();
}

bool KoInlineTextObjectManager::boolProperty(KInlineObject::Property key) const
{
    if (!m_properties.contains(key))
        return false;
    return m_properties.value(key).toBool();
}

QString KoInlineTextObjectManager::stringProperty(KInlineObject::Property key) const
{
    if (!m_properties.contains(key))
        return QString();
    return qvariant_cast<QString>(m_properties.value(key));
}

const KoVariableManager *KoInlineTextObjectManager::variableManager() const
{
    return &m_variableManager;
}

KoVariableManager *KoInlineTextObjectManager::variableManager()
{
    return &m_variableManager;
}

KoBookmarkManager *KoInlineTextObjectManager::bookmarkManager()
{
    return &m_bookmarkManager;
}

void KoInlineTextObjectManager::removeProperty(KInlineObject::Property key)
{
    m_properties.remove(key);
}

QList<QAction*> KoInlineTextObjectManager::createInsertVariableActions(KoCanvasBase *host) const
{
    QList<QAction *> answer = KInlineObjectRegistry::instance()->createInsertVariableActions(host);
    int i = 0;
    foreach(const QString & name, m_variableManager.variables()) {
        answer.insert(i++, new InsertNamedVariableAction(host, this, name));
    }

    answer.append(new InsertTextLocator(host));
    answer.append(new InsertTextReferenceAction(host, this));
    return answer;
}

QList<KoTextLocator*> KoInlineTextObjectManager::textLocators() const
{
    QList<KoTextLocator*> answers;
    foreach(KInlineObject *object, m_objects) {
        KoTextLocator *tl = dynamic_cast<KoTextLocator*>(object);
        if (tl)
            answers.append(tl);
    }
    return answers;
}

void KoInlineTextObjectManager::documentInformationUpdated(const QString &info, const QString &data)
{
    if (info == "title")
        setProperty(KInlineObject::Title, data);
    else if (info == "description")
        setProperty(KInlineObject::Description, data);
    else if (info == "subject")
        setProperty(KInlineObject::Subject, data);
    else if (info == "keyword")
        setProperty(KInlineObject::Keywords, data);
    else if (info == "creator")
        setProperty(KInlineObject::AuthorName, data);
    else if (info == "initial")
        setProperty(KInlineObject::AuthorInitials, data);
    else if (info == "author-title")
        setProperty(KInlineObject::SenderTitle, data);
    else if (info == "email")
        setProperty(KInlineObject::SenderEmail, data);
    else if (info == "telephone")
        setProperty(KInlineObject::SenderPhonePrivate, data);
    else if (info == "telephone-work")
        setProperty(KInlineObject::SenderPhoneWork, data);
    else if (info == "fax")
        setProperty(KInlineObject::SenderFax, data);
    else if (info == "country")
        setProperty(KInlineObject::SenderCountry, data);
    else if (info == "postal-code")
        setProperty(KInlineObject::SenderPostalCode, data);
    else if (info == "city")
        setProperty(KInlineObject::SenderCity, data);
    else if (info == "street")
        setProperty(KInlineObject::SenderStreet, data);
    else if (info == "position")
        setProperty(KInlineObject::SenderPosition, data);
    else if (info == "company")
        setProperty(KInlineObject::SenderCompany, data);
}

QList<KInlineObject*> KoInlineTextObjectManager::inlineTextObjects() const
{
    return m_objects.values();
}

#include <KoInlineTextObjectManager.moc>
