/* This file is part of the KDE project
   Copyright (c) 2003 Lukas Tinkl <lukas@kde.org>
   Copyright (c) 2003 David Faure <faure@kde.org>

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

#include "KOdfStyleStack.h"
#include "KUnit.h"
#include "KOdfXmlNS.h"

#include <kdebug.h>

//#define DEBUG_STYLESTACK

class KOdfStyleStack::KOdfStyleStackPrivate
{
};

KOdfStyleStack::KOdfStyleStack()
        : m_styleNSURI(KOdfXmlNS::style), m_foNSURI(KOdfXmlNS::fo), d(0)
{
    clear();
}

KOdfStyleStack::KOdfStyleStack(const char* styleNSURI, const char* foNSURI)
        : m_propertiesTagName("properties"), m_styleNSURI(styleNSURI), m_foNSURI(foNSURI), d(0)
{
    clear();
}

KOdfStyleStack::~KOdfStyleStack()
{
    delete d;
}

void KOdfStyleStack::clear()
{
    m_stack.clear();
#ifdef DEBUG_STYLESTACK
    kDebug(30003) << "clear!";
#endif
}

void KOdfStyleStack::save()
{
    m_marks.push(m_stack.count());
#ifdef DEBUG_STYLESTACK
    kDebug(30003) << "save (level" << m_marks.count() << ") -> index" << m_stack.count();
#endif
}

void KOdfStyleStack::restore()
{
    Q_ASSERT(!m_marks.isEmpty());
    int toIndex = m_marks.pop();
#ifdef DEBUG_STYLESTACK
    kDebug(30003) << "restore (level" << m_marks.count() + 1 << ") -> to index" << toIndex;
#endif
    Q_ASSERT(toIndex > -1);
    Q_ASSERT(toIndex <= (int)m_stack.count());   // If equal, nothing to remove. If greater, bug.
    for (int index = (int)m_stack.count() - 1; index >= toIndex; --index)
        m_stack.pop_back();
}

void KOdfStyleStack::pop()
{
    Q_ASSERT(!m_stack.isEmpty());
    m_stack.pop_back();
#ifdef DEBUG_STYLESTACK
    kDebug(30003) << "pop -> count=" << m_stack.count();
#endif
}

void KOdfStyleStack::push(const KXmlElement& style)
{
    m_stack.append(style);
#ifdef DEBUG_STYLESTACK
    kDebug(30003) << "pushed" << style.attributeNS(m_styleNSURI, "name", QString()) << " -> count=" << m_stack.count();
#endif
}

QString KOdfStyleStack::property(const QString &nsURI, const QString &name) const
{
    return property(nsURI, name, 0);
}
QString KOdfStyleStack::property(const QString &nsURI, const QString &name, const QString &detail) const
{
    return property(nsURI, name, &detail);
}

inline QString KOdfStyleStack::property(const QString &nsURI, const QString &name, const QString *detail) const
{
    QString fullName(name);
    if (detail) {
        fullName += '-';
        fullName += *detail;
    }
    QList<KXmlElement>::ConstIterator it = m_stack.end();
    while (it != m_stack.begin()) {
        --it;
        KXmlElement properties = KoXml::namedItemNS(*it, m_styleNSURI, m_propertiesTagName);
        if (detail) {
            QString attribute(properties.attributeNS(nsURI, fullName));
            if (!attribute.isEmpty()) {
                return attribute;
            }
        }
        QString attribute(properties.attributeNS(nsURI, name));
        if (!attribute.isEmpty()) {
            return attribute;
        }
    }
    return QString();
}

bool KOdfStyleStack::hasProperty(const QString &nsURI, const QString &name) const
{
    return hasProperty(nsURI, name, 0);
}

bool KOdfStyleStack::hasProperty(const QString &nsURI, const QString &name, const QString &detail) const
{
    return hasProperty(nsURI, name, &detail);
}

inline bool KOdfStyleStack::hasProperty(const QString &nsURI, const QString &name, const QString *detail) const
{
    QString fullName(name);
    if (detail) {
        fullName += '-';
        fullName += *detail;
    }
    QList<KXmlElement>::ConstIterator it = m_stack.end();
    while (it != m_stack.begin()) {
        --it;
        const KXmlElement properties = KoXml::namedItemNS(*it, m_styleNSURI, m_propertiesTagName);
        if (properties.hasAttributeNS(nsURI, name) ||
                (detail && properties.hasAttributeNS(nsURI, fullName)))
            return true;
    }
    return false;
}

// Font size is a bit special. "115%" applies to "the fontsize of the parent style".
// This can be generalized though (hasPropertyThatCanBePercentOfParent() ? :)
qreal KOdfStyleStack::fontSize(const qreal defaultFontPointSize) const
{
    const QString name = "font-size";
    qreal percent = 1;
    QList<KXmlElement>::ConstIterator it = m_stack.end(); // reverse iterator

    while (it != m_stack.begin()) {
        --it;
        KXmlElement properties = KoXml::namedItemNS(*it, m_styleNSURI, m_propertiesTagName).toElement();
        if (properties.hasAttributeNS(m_foNSURI, name)) {
            const QString value = properties.attributeNS(m_foNSURI, name, QString());
            if (value.endsWith('%')) {
                //sebsauer, 20070609, the specs don't say that we have to calc them together but
                //just that we are looking for a valid parent fontsize. So, let's only take the
                //first percent definition into account and keep on to seek for a valid parent,
                //percent *= value.left( value.length() - 1 ).toDouble() / 100.0;
                if (percent == 1)
                    percent = value.left(value.length() - 1).toDouble() / 100.0;
            } else
                return percent * KUnit::parseValue(value);   // e.g. 12pt
        }
    }

    //if there was no valid parent, we return the default fontsize together with an optional calculated percent-value.
    return percent * defaultFontPointSize;
}

bool KOdfStyleStack::hasChildNode(const QString &nsURI, const QString &localName) const
{
    QList<KXmlElement>::ConstIterator it = m_stack.end();
    while (it != m_stack.begin()) {
        --it;
        KXmlElement properties = KoXml::namedItemNS(*it, m_styleNSURI, m_propertiesTagName);
        if (!KoXml::namedItemNS(properties, nsURI, localName).isNull())
            return true;
    }

    return false;
}

KXmlElement KOdfStyleStack::childNode(const QString &nsURI, const QString &localName) const
{
    QList<KXmlElement>::ConstIterator it = m_stack.end();

    while (it != m_stack.begin()) {
        --it;
        KXmlElement properties = KoXml::namedItemNS(*it, m_styleNSURI, m_propertiesTagName);
        KXmlElement e = KoXml::namedItemNS(properties, nsURI, localName);
        if (!e.isNull())
            return e;
    }

    return KXmlElement();          // a null element
}

bool KOdfStyleStack::isUserStyle(const KXmlElement& e, const QString& family) const
{
    if (e.attributeNS(m_styleNSURI, "family", QString()) != family)
        return false;
    const KXmlElement parent = e.parentNode().toElement();
    //kDebug(30003) <<"tagName=" << e.tagName() <<" parent-tagName=" << parent.tagName();
    return parent.localName() == "styles" /*&& parent.namespaceURI() == KOdfXmlNS::office*/;
}

QString KOdfStyleStack::userStyleName(const QString& family) const
{
    QList<KXmlElement>::ConstIterator it = m_stack.end();
    while (it != m_stack.begin()) {
        --it;
        //kDebug(30003) << (*it).attributeNS( m_styleNSURI,"name", QString());
        if (isUserStyle(*it, family))
            return (*it).attributeNS(m_styleNSURI, "name", QString());
    }
    // Can this ever happen?
    return "Standard";
}

QString KOdfStyleStack::userStyleDisplayName(const QString& family) const
{
    QList<KXmlElement>::ConstIterator it = m_stack.end();
    while (it != m_stack.begin()) {
        --it;
        //kDebug(30003) << (*it).attributeNS( m_styleNSURI,"display-name");
        if (isUserStyle(*it, family))
            return (*it).attributeNS(m_styleNSURI, "display-name", QString());
    }
    return QString(); // no display name, this can happen since it's optional
}

void KOdfStyleStack::setTypeProperties(const char* typeProperties)
{
    m_propertiesTagName = typeProperties == 0 ? QString("properties") : (QString(typeProperties) + "-properties");
}
