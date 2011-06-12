/* This file is part of the KDE project
   Copyright 2007 Stefan Nikolaus <stefan.nikolaus@kdemail.net>
   Copyright 2005-2006 Inge Wallin <inge@lysator.liu.se>
   Copyright 2004 Ariya Hidayat <ariya@kde.org>
   Copyright 2002-2003 Norbert Andres <nandres@web.de>
   Copyright 2000-2002 Laurent Montel <montel@kde.org>
   Copyright 2002 John Dailey <dailey@vt.edu>
   Copyright 2002 Phillip Mueller <philipp.mueller@gmx.de>
   Copyright 2000 Werner Trobin <trobin@kde.org>
   Copyright 1999-2000 Simon Hausmann <hausmann@kde.org>
   Copyright 1999 David Faure <faure@kde.org>
   Copyright 1998-2000 Torben Weis <weis@kde.org>

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
   Boston, MA 02110-1301, USA.
*/

// Local
#include "KCNamedAreaManager.h"

// Qt
#include <QHash>

// KOffice
#include <KOdfXmlNS.h>
#include <KoXmlWriter.h>

// KCells
#include "KCCellStorage.h"
#include "KCFormulaStorage.h"
#include "KCLoadingInfo.h"
#include "KCMap.h"
#include "KCRegion.h"
#include "KCSheet.h"
#include "Util.h"

struct NamedArea {
    QString name;
    KCSheet* sheet;
    QRect range;
};

class KCNamedAreaManager::Private
{
public:
    const KCMap* map;
    QHash<QString, NamedArea> namedAreas;
};

KCNamedAreaManager::KCNamedAreaManager(const KCMap* map)
        : d(new Private)
{
    d->map = map;
    connect(this, SIGNAL(namedAreaAdded(const QString&)),
            this, SIGNAL(namedAreaModified(const QString&)));
    connect(this, SIGNAL(namedAreaRemoved(const QString&)),
            this, SIGNAL(namedAreaModified(const QString&)));
}

KCNamedAreaManager::~KCNamedAreaManager()
{
    delete d;
}

void KCNamedAreaManager::insert(const KCRegion& region, const QString& name)
{
    // NOTE Stefan: Only contiguous regions are supported (OpenDocument compatibility).
    NamedArea namedArea;
    namedArea.range = region.lastRange();
    namedArea.sheet = region.lastSheet();
    namedArea.name = name;
    namedArea.sheet->cellStorage()->setNamedArea(KCRegion(region.lastRange(), region.lastSheet()), name);
    d->namedAreas[name] = namedArea;
    emit namedAreaAdded(name);
}

void KCNamedAreaManager::remove(const QString& name)
{
    if (!d->namedAreas.contains(name))
        return;
    NamedArea namedArea = d->namedAreas.value(name);
    namedArea.sheet->cellStorage()->setNamedArea(KCRegion(namedArea.range, namedArea.sheet), QString());
    d->namedAreas.remove(name);
    emit namedAreaRemoved(name);
    const QList<KCSheet*> sheets = namedArea.sheet->map()->sheetList();
    foreach(KCSheet* sheet, sheets) {
        const QString tmp = '\'' + name + '\'';
        const KCFormulaStorage* const storage = sheet->formulaStorage();
        for (int c = 0; c < storage->count(); ++c) {
            if (storage->data(c).expression().contains(tmp)) {
                KCCell(sheet, storage->col(c), storage->row(c)).makeFormula();
            }
        }
    }
}

void KCNamedAreaManager::remove(KCSheet* sheet)
{
    const QList<NamedArea> namedAreas = d->namedAreas.values();
    for (int i = 0; i < namedAreas.count(); ++i) {
        if (namedAreas[i].sheet == sheet)
            remove(namedAreas[i].name);
    }
}

KCRegion KCNamedAreaManager::namedArea(const QString& name) const
{
    if (!d->namedAreas.contains(name))
        return KCRegion();
    const NamedArea namedArea = d->namedAreas.value(name);
    return KCRegion(namedArea.range, namedArea.sheet);
}

KCSheet* KCNamedAreaManager::sheet(const QString& name) const
{
    if (!d->namedAreas.contains(name))
        return 0;
    return d->namedAreas.value(name).sheet;
}

bool KCNamedAreaManager::contains(const QString& name) const
{
    return d->namedAreas.contains(name);
}

QList<QString> KCNamedAreaManager::areaNames() const
{
    return d->namedAreas.keys();
}

void KCNamedAreaManager::regionChanged(const KCRegion& region)
{
    KCSheet* sheet;
    QList< QPair<QRectF, QString> > namedAreas;
    KCRegion::ConstIterator end(region.constEnd());
    for (KCRegion::ConstIterator it = region.constBegin(); it != end; ++it) {
        sheet = (*it)->sheet();
        namedAreas = sheet->cellStorage()->namedAreas(KCRegion((*it)->rect(), sheet));
        for (int j = 0; j < namedAreas.count(); ++j) {
            d->namedAreas[namedAreas[j].second].range = namedAreas[j].first.toRect();
            emit namedAreaModified(namedAreas[j].second);
        }
    }
}

void KCNamedAreaManager::updateAllNamedAreas()
{
    QList< QPair<QRectF, QString> > namedAreas;
    const QRect rect(QPoint(1, 1), QPoint(KS_colMax, KS_rowMax));
    const QList<KCSheet*> sheets = d->map->sheetList();
    for (int i = 0; i < sheets.count(); ++i) {
        namedAreas = sheets[i]->cellStorage()->namedAreas(KCRegion(rect, sheets[i]));
        for (int j = 0; j < namedAreas.count(); ++j) {
            d->namedAreas[namedAreas[j].second].range = namedAreas[j].first.toRect();
            emit namedAreaModified(namedAreas[j].second);
        }
    }
}

void KCNamedAreaManager::loadOdf(const KoXmlElement& body)
{
    KoXmlNode namedAreas = KoXml::namedItemNS(body, KOdfXmlNS::table, "named-expressions");
    if (!namedAreas.isNull()) {
        kDebug(36003) << "Loading named areas...";
        KoXmlElement element;
        forEachElement(element, namedAreas) {
            if (element.namespaceURI() != KOdfXmlNS::table)
                continue;
            if (element.localName() == "named-range") {
                if (!element.hasAttributeNS(KOdfXmlNS::table, "name"))
                    continue;
                if (!element.hasAttributeNS(KOdfXmlNS::table, "cell-range-address"))
                    continue;

                // TODO: what is: table:base-cell-address
                const QString base = element.attributeNS(KOdfXmlNS::table, "base-cell-address", QString());

                // Handle the case where the table:base-cell-address does contain the referenced sheetname
                // while it's missing in the table:cell-range-address. See bug #194386 for an example.
                KCSheet* fallbackSheet = 0;
                if (!base.isEmpty()) {
                    KCRegion region(KCRegion::loadOdf(base), d->map);
                    fallbackSheet = region.lastSheet();
                }
                
                const QString name = element.attributeNS(KOdfXmlNS::table, "name", QString());
                const QString range = element.attributeNS(KOdfXmlNS::table, "cell-range-address", QString());
                kDebug(36003) << "Named area found, name:" << name << ", area:" << range;

                KCRegion region(KCRegion::loadOdf(range), d->map, fallbackSheet);
                if (!region.isValid() || !region.lastSheet()) {
                    kDebug(36003) << "invalid area";
                    continue;
                }

                insert(region, name);
            } else if (element.localName() == "named-expression") {
                kDebug(36003) << "Named expression found.";
                // TODO
            }
        }
    }
}

void KCNamedAreaManager::saveOdf(KoXmlWriter& xmlWriter) const
{
    if (d->namedAreas.isEmpty())
        return;
    KCRegion region;
    xmlWriter.startElement("table:named-expressions");
    const QList<NamedArea> namedAreas = d->namedAreas.values();
    for (int i = 0; i < namedAreas.count(); ++i) {
        region = KCRegion(namedAreas[i].range, namedAreas[i].sheet);
        xmlWriter.startElement("table:named-range");
        xmlWriter.addAttribute("table:name", namedAreas[i].name);
        xmlWriter.addAttribute("table:base-cell-address", KCRegion(1, 1, namedAreas[i].sheet).saveOdf());
        xmlWriter.addAttribute("table:cell-range-address", region.saveOdf());
        xmlWriter.endElement();
    }
    xmlWriter.endElement();
}

void KCNamedAreaManager::loadXML(const KoXmlElement& parent)
{
    KoXmlElement element;
    forEachElement(element, parent) {
        if (element.tagName() == "reference") {
            KCSheet* sheet = 0;
            QString tabname;
            QString refname;
            int left = 0;
            int right = 0;
            int top = 0;
            int bottom = 0;
            KoXmlElement sheetName = element.namedItem("tabname").toElement();
            if (!sheetName.isNull())
                sheet = d->map->findSheet(sheetName.text());
            if (!sheet)
                continue;
            KoXmlElement referenceName = element.namedItem("refname").toElement();
            if (!referenceName.isNull())
                refname = referenceName.text();
            KoXmlElement rect = element.namedItem("rect").toElement();
            if (!rect.isNull()) {
                bool ok;
                if (rect.hasAttribute("left-rect"))
                    left = rect.attribute("left-rect").toInt(&ok);
                if (rect.hasAttribute("right-rect"))
                    right = rect.attribute("right-rect").toInt(&ok);
                if (rect.hasAttribute("top-rect"))
                    top = rect.attribute("top-rect").toInt(&ok);
                if (rect.hasAttribute("bottom-rect"))
                    bottom = rect.attribute("bottom-rect").toInt(&ok);
            }
            insert(KCRegion(QRect(QPoint(left, top), QPoint(right, bottom)), sheet), refname);
        }
    }
}

QDomElement KCNamedAreaManager::saveXML(QDomDocument& doc) const
{
    QDomElement element = doc.createElement("areaname");
    const QList<NamedArea> namedAreas = d->namedAreas.values();
    for (int i = 0; i < namedAreas.count(); ++i) {
        QDomElement e = doc.createElement("reference");
        QDomElement tabname = doc.createElement("tabname");
        tabname.appendChild(doc.createTextNode(namedAreas[i].sheet->sheetName()));
        e.appendChild(tabname);

        QDomElement refname = doc.createElement("refname");
        refname.appendChild(doc.createTextNode(namedAreas[i].name));
        e.appendChild(refname);

        QDomElement rect = doc.createElement("rect");
        rect.setAttribute("left-rect", (namedAreas[i].range).left());
        rect.setAttribute("right-rect", (namedAreas[i].range).right());
        rect.setAttribute("top-rect", (namedAreas[i].range).top());
        rect.setAttribute("bottom-rect", (namedAreas[i].range).bottom());
        e.appendChild(rect);
        element.appendChild(e);
    }
    return element;
}

#include "KCNamedAreaManager.moc"
