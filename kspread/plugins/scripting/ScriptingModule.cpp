/*
 * This file is part of KCells
 *
 * Copyright (c) 2005 Cyrille Berger <cberger@cberger.net>
 * Copyright (c) 2006 Isaac Clerencia <isaac@warp.es>
 * Copyright (c) 2006 Sebastian Sauer <mail@dipe.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "ScriptingModule.h"
#include "ScriptingFunction.h"
#include "ScriptingWidgets.h"
#include "ScriptingReader.h"
#include "ScriptingWriter.h"
#include "ScriptingListener.h"

#include <QPointer>
#include <QLayout>
#include <kdebug.h>

#include <part/KCDoc.h>
#include <part/KCView.h>
#include <interfaces/ViewAdaptor.h>
#include <KCSheet.h>
#include <interfaces/SheetAdaptor.h>
#include <KCMap.h>
#include <interfaces/MapAdaptor.h>
#include <KoDocumentAdaptor.h>
#include <KoApplicationAdaptor.h>

extern "C" {
    KDE_EXPORT QObject* krossmodule() {
        return new ScriptingModule();
    }
}

/// \internal d-pointer class.
class ScriptingModule::Private
{
public:
    QPointer<KCDoc> doc;
    QHash<QString, ScriptingFunction* > functions;
    QStringList functionnames;
};

ScriptingModule::ScriptingModule(QObject* parent)
        : KoScriptingModule(parent, "KCells")
        , d(new Private())
{
    d->doc = 0;
}

ScriptingModule::~ScriptingModule()
{
    kDebug() << "ScriptingModule::~ScriptingModule()";
    delete d;
}

KCView* ScriptingModule::kcellsView()
{
    return dynamic_cast<KCView* >(KoScriptingModule::view());
}

KCDoc* ScriptingModule::kcellsDoc()
{
    if (! d->doc) {
        if (KCView* v = kcellsView())
            d->doc = v->doc();
        if (! d->doc)
            d->doc = new KCDoc(0, this);
    }
    return d->doc;
}

KoDocument* ScriptingModule::doc()
{
    return kcellsDoc();
}

QObject* ScriptingModule::map()
{
    return kcellsDoc()->map()->findChild<MapAdaptor* >();
}

QObject* ScriptingModule::view()
{
    KCView* v = kcellsView();
    return v ? v->findChild<ViewAdaptor* >() : 0;
}

QObject* ScriptingModule::currentSheet()
{
    KCView* v = kcellsView();
    KCSheet* s = v ? v->activeSheet() : 0;
    return s ? s->findChild<SheetAdaptor* >() : 0;
}

QObject* ScriptingModule::sheetByName(const QString& name)
{
    if (kcellsDoc()->map())
        foreach(KCSheet* sheet, kcellsDoc()->map()->sheetList()) {
        if (sheet->sheetName() == name) {
            return sheet->findChild<SheetAdaptor* >(); {
            }
        }
    }
    return 0;
}

QStringList ScriptingModule::sheetNames()
{
    QStringList names;
    foreach(KCSheet* sheet, kcellsDoc()->map()->sheetList()) {
        names.append(sheet->sheetName());
    }
    return names;
}

bool ScriptingModule::hasFunction(const QString& name)
{
    return d->functions.contains(name);
}

QObject* ScriptingModule::function(const QString& name)
{
    if (d->functions.contains(name))
        return d->functions[name];
    ScriptingFunction* function = new ScriptingFunction(this);
    function->setName(name);
    d->functions.insert(name, function);
    d->functionnames.append(name);
    return function;
}

QObject* ScriptingModule::createListener(const QString& sheetname, const QString& range)
{
    KCSheet* sheet = kcellsDoc()->map()->findSheet(sheetname);
    if (! sheet) return 0;
    KCRegion region(range, kcellsDoc()->map(), sheet);
    QRect r = region.firstRange();
    return new ScriptingCellListener(sheet, r.isNull() ? sheet->usedArea() : r);
}

bool ScriptingModule::fromXML(const QString& xml)
{
    KoXmlDocument xmldoc;
    if (! xmldoc.setContent(xml, true))
        return false;
    return kcellsDoc()->loadXML(xmldoc, 0);
}

QString ScriptingModule::toXML()
{
    return kcellsDoc()->saveXML().toString(2);
}

bool ScriptingModule::openUrl(const QString& url)
{
    return kcellsDoc()->openUrl(url);
}

bool ScriptingModule::saveUrl(const QString& url)
{
    return kcellsDoc()->saveAs(url);
}

bool ScriptingModule::importUrl(const QString& url)
{
    return kcellsDoc()->importDocument(url);
}

bool ScriptingModule::exportUrl(const QString& url)
{
    return kcellsDoc()->exportDocument(url);
}

QObject* ScriptingModule::reader()
{
    return new ScriptingReader(this);
}

QObject* ScriptingModule::writer()
{
    return new ScriptingWriter(this);
}

QWidget* ScriptingModule::createSheetsListView(QWidget* parent)
{
    ScriptingSheetsListView* listview = new ScriptingSheetsListView(this, parent);
    if (parent && parent->layout())
        parent->layout()->addWidget(listview);
    return listview;
}

#include "ScriptingModule.moc"
