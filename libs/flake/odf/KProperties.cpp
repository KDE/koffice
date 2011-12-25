/*
 *  Copyright (c) 2006 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (C) 2006-2008 Thomas Zander <zander@kde.org>
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

#include "KProperties.h"

#include <qdom.h>
#include <QString>

class KProperties::Private
{
public:
    QMap<QString, QVariant> properties;
};

KProperties::KProperties()
        : d(new Private())
{
}

KProperties::KProperties(const KProperties & rhs)
        : d(new Private())
{
    d->properties = rhs.d->properties;
}

KProperties::~KProperties()
{
    delete d;
}

QMapIterator<QString, QVariant> KProperties::propertyIterator() const
{
    return QMapIterator<QString, QVariant>(d->properties);
}


bool KProperties::isEmpty() const
{
    return d->properties.isEmpty();
}

void  KProperties::load(const QDomElement &root)
{
    d->properties.clear();

    QDomElement e = root;
    QDomNode n = e.firstChild();

    while (!n.isNull()) {
        // We don't nest elements.
        QDomElement e = n.toElement();
        if (!e.isNull()) {
            if (e.tagName() == "property") {
                const QString name = e.attribute("name");
                const QString type = e.attribute("type");
                const QString value = e.text();
                QDataStream in(value.toAscii());
                QVariant v;
                in >> v;
                d->properties[name] = v;
            }
        }
        n = n.nextSibling();
    }
}

bool KProperties::load(const QString & s)
{
    QDomDocument doc;

    if (!doc.setContent(s))
        return false;
    load(doc.documentElement());

    return true;
}

void KProperties::save(QDomElement &root) const
{
    QDomDocument doc = root.ownerDocument();
    QMap<QString, QVariant>::Iterator it;
    for (it = d->properties.begin(); it != d->properties.end(); ++it) {
        QDomElement e = doc.createElement("property");
        e.setAttribute("name", QString(it.key().toLatin1()));
        QVariant v = it.value();
        e.setAttribute("type", v.typeName());

        QByteArray bytes;
        QDataStream out(&bytes, QIODevice::WriteOnly);
        out << v;
        QDomText text = doc.createCDATASection(QString::fromAscii(bytes, bytes.size()));
        e.appendChild(text);
        root.appendChild(e);
    }
}

QString KProperties::store(const QString &s) const
{
    QDomDocument doc = QDomDocument(s);
    QDomElement root = doc.createElement(s);
    doc.appendChild(root);

    save(root);
    return doc.toString();
}

void KProperties::setProperty(const QString & name, const QVariant & value)
{
    // If there's an existing value for this name already, replace it.
    d->properties.insert(name, value);
}

bool KProperties::property(const QString & name, QVariant & value) const
{
    QMap<QString, QVariant>::const_iterator it = d->properties.constFind(name);
    if (it == d->properties.constEnd()) {
        return false;
    } else {
        value = *it;
        return true;
    }
}

QVariant KProperties::property(const QString & name) const
{
    return d->properties.value(name, QVariant());
}


int KProperties::intProperty(const QString & name, int def) const
{
    const QVariant v = property(name);
    if (v.isValid())
        return v.toInt();
    else
        return def;

}

qreal KProperties::doubleProperty(const QString & name, qreal def) const
{
    const QVariant v = property(name);
    if (v.isValid())
        return v.toDouble();
    else
        return def;
}

bool KProperties::boolProperty(const QString & name, bool def) const
{
    const QVariant v = property(name);
    if (v.isValid())
        return v.toBool();
    else
        return def;
}

QString KProperties::stringProperty(const QString & name, const QString & def) const
{
    const QVariant v = property(name);
    if (v.isValid())
        return v.toString();
    else
        return def;
}

bool KProperties::contains(const QString & key) const
{
    return d->properties.contains(key);
}

QVariant KProperties::value(const QString & key) const
{
    return d->properties.value(key);
}

bool KProperties::operator==(const KProperties &other) const
{
    if (d->properties.count() != other.d->properties.count())
        return false;
    
    for (QMap<QString, QVariant>::const_iterator it = d->properties.constBegin(); it != d->properties.constEnd(); ++it) {
    	const QString & key = it.key();
        if (other.d->properties.value(key) != it.value())
            return false;
    }
    return true;
}
