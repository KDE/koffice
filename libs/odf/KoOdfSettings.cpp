/* This file is part of the KDE project
   Copyright (C) 2004 Laurent Montel <montel@kde.org>

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

#include "KoOdfSettings.h"
#include "KOdfXmlNS.h"
#include <kdebug.h>

class KoOdfSettings::Private
{
};

KoOdfSettings::KoOdfSettings(const KoXmlDocument& doc)
        : m_settingsElement(KoXml::namedItemNS(doc.documentElement(), KOdfXmlNS::office, "settings")),
        m_configNsUri(KOdfXmlNS::config)
        , d(0)
{
    const KoXmlElement contents = doc.documentElement();
    if (m_settingsElement.isNull())
        kDebug(30003) << " document doesn't have tag 'office:settings'";
}

KoOdfSettings::KoOdfSettings(const KoXmlDocument& doc, const char* officeNSURI, const char* configNSURI)
        : m_settingsElement(KoXml::namedItemNS(doc.documentElement(), officeNSURI, "settings")),
        m_configNsUri(configNSURI)
        , d(0)
{
    const KoXmlElement contents = doc.documentElement();
    if (m_settingsElement.isNull())
        kDebug(30003) << " document doesn't have tag 'office:settings'";
}

KoOdfSettings::~KoOdfSettings()
{
    delete d;
}

KoOdfSettings::Items KoOdfSettings::itemSet(const QString& itemSetName) const
{
    KoXmlElement e;
    forEachElement(e, m_settingsElement) {
        if (e.localName() == "config-item-set" &&
                e.namespaceURI() == m_configNsUri &&
                e.attributeNS(m_configNsUri, "name", QString()) == itemSetName) {
            return Items(e, this);
        }
    }

    return Items(KoXmlElement(), this);
}

KoOdfSettings::IndexedMap KoOdfSettings::Items::indexedMap(const QString& itemMapName) const
{
    KoXmlElement configItem;
    forEachElement(configItem, m_element) {
        if (configItem.localName() == "config-item-map-indexed" &&
                configItem.namespaceURI() == m_settings->m_configNsUri &&
                configItem.attributeNS(m_settings->m_configNsUri, "name", QString()) == itemMapName) {
            return IndexedMap(configItem, m_settings);
        }
    }
    return IndexedMap(KoXmlElement(), m_settings);
}

KoOdfSettings::NamedMap KoOdfSettings::Items::namedMap(const QString& itemMapName) const
{
    KoXmlElement configItem;
    forEachElement(configItem, m_element) {
        if (configItem.localName() == "config-item-map-named" &&
                configItem.namespaceURI() == m_settings->m_configNsUri &&
                configItem.attributeNS(m_settings->m_configNsUri, "name", QString()) == itemMapName) {
            return NamedMap(configItem, m_settings);
        }
    }
    return NamedMap(KoXmlElement(), m_settings);
}

KoOdfSettings::Items KoOdfSettings::IndexedMap::entry(int entryIndex) const
{
    int i = 0;
    KoXmlElement entry;
    forEachElement(entry, m_element) {
        if (entry.localName() == "config-item-map-entry" &&
                entry.namespaceURI() == m_settings->m_configNsUri) {
            if (i == entryIndex)
                return Items(entry, m_settings);
            else
                ++i;
        }
    }
    return Items(KoXmlElement(), m_settings);
}

KoOdfSettings::Items KoOdfSettings::NamedMap::entry(const QString& entryName) const
{
    KoXmlElement entry;
    forEachElement(entry, m_element) {
        if (entry.localName() == "config-item-map-entry" &&
                entry.namespaceURI() == m_settings->m_configNsUri &&
                entry.attributeNS(m_settings->m_configNsUri, "name", QString()) == entryName) {
            return Items(entry, m_settings);
        }
    }
    return Items(KoXmlElement(), m_settings);
}

// helper method
QString KoOdfSettings::Items::findConfigItem(const KoXmlElement& element,
        const QString& item, bool* ok) const
{
    KoXmlElement it;
    forEachElement(it, element) {
        if (it.localName() == "config-item" &&
                it.namespaceURI() == m_settings->m_configNsUri &&
                it.attributeNS(m_settings->m_configNsUri, "name", QString()) == item) {
            *ok = true;
            return it.text();
        }
    }
    *ok = false;
    return QString();
}


QString KoOdfSettings::Items::findConfigItem(const QString& item, bool* ok) const
{
    return findConfigItem(m_element, item, ok);
}

#if 0 // does anyone need this one? passing a default value does the job, too
bool KoOdfSettings::Items::hasConfigItem(const QString& configName) const
{
    bool ok;
    (void)findConfigItem(configName, &ok);
    return ok;
}
#endif

QString KoOdfSettings::Items::parseConfigItemString(const QString& configName, const QString& defValue) const
{
    bool ok;
    const QString str = findConfigItem(configName, &ok);
    return ok ? str : defValue;
}

int KoOdfSettings::Items::parseConfigItemInt(const QString& configName, int defValue) const
{
    bool ok;
    const QString str = findConfigItem(configName, &ok);
    int value;
    if (ok) {
        value = str.toInt(&ok);
        if (ok)
            return value;
    }
    return defValue;
}

qreal KoOdfSettings::Items::parseConfigItemDouble(const QString& configName, qreal defValue) const
{
    bool ok;
    const QString str = findConfigItem(configName, &ok);
    qreal value;
    if (ok) {
        value = str.toDouble(&ok);
        if (ok)
            return value;
    }
    return defValue;
}

bool KoOdfSettings::Items::parseConfigItemBool(const QString& configName, bool defValue) const
{
    bool ok;
    const QString str = findConfigItem(configName, &ok);
    if (! ok)
        return defValue;
    if (str == "true")
        return true;
    else if (str == "false")
        return false;
    return defValue;
}

short KoOdfSettings::Items::parseConfigItemShort(const QString& configName, short defValue) const
{
    bool ok;
    const QString str = findConfigItem(configName, &ok);
    short value;
    if (ok) {
        value = str.toShort(&ok);
        if (ok)
            return value;
    }
    return defValue;
}

long KoOdfSettings::Items::parseConfigItemLong(const QString& configName, long defValue) const
{
    bool ok;
    const QString str = findConfigItem(configName, &ok);
    long value;
    if (ok) {
        value = str.toLong(&ok);
        if (ok)
            return value;
    }
    return defValue;
}
