/* This file is part of the KDE project
   Copyright 2010 Marijn Kruisselbrink <m.kruisselbrink@student.tue.nl>
   Copyright 2007 Stefan Nikolaus <stefan.nikolaus@kdemail.net>
   Copyright 2007 Thorsten Zachmann <zachmann@kde.org>
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
#include "KCDocBase.h"
#include "KCDocBase.moc"
#include "DocBase_p.h"

#include <KOdfSettings.h>
#include <KOdfLoadingContext.h>
#include <KOdfStoreReader.h>
#include <KOdfWriteStore.h>
#include <KoProgressUpdater.h>
#include <KResourceManager.h>
#include <KShapeRegistry.h>
#include <KShapeSavingContext.h>
#include <KOdfStorageDevice.h>
#include <KoUpdater.h>
#include <KOdfXmlNS.h>
#include <KXmlWriter.h>

#include "kcells_limits.h"
#include "KCBindingModel.h"
#include "KCCalculationSettings.h"
#include "KCMap.h"
#include "KCSheetAccessModel.h"

#include "part/KCView.h" // TODO: get rid of this dependency

QList<KCDocBase*> KCDocBase::Private::s_docs;
int KCDocBase::Private::s_docId = 0;

Q_DECLARE_METATYPE(QPointer<QAbstractItemModel>)

KCDocBase::KCDocBase(QWidget *parentWidget, QObject* parent, bool singleViewMode)
    : KoDocument(parentWidget, parent, singleViewMode)
    , d(new Private)
{
    d->resourceManager = new KResourceManager();
    d->map = new KCMap(this, CURRENT_SYNTAX_VERSION);

    // Document Url for FILENAME function and page header/footer.
    d->map->calculationSettings()->setFileName(url().prettyUrl());

    KShapeRegistry *registry = KShapeRegistry::instance();
    for (KGenericRegistry<KShapeFactoryBase*>::const_iterator it = registry->constBegin(); 
		    it != registry->constEnd(); ++it) {
        KShapeFactoryBase *shapeFactory = it.value();
        shapeFactory->newDocumentResourceManager(d->resourceManager);
    }

    d->configLoadFromFile = false;

    documents().append(this);

    d->sheetAccessModel = new KCSheetAccessModel(d->map);
}

KCDocBase::~KCDocBase()
{
    delete d->map;
    delete d->sheetAccessModel;
    delete d->resourceManager;
    delete d;
}

QList<KCDocBase*> KCDocBase::documents()
{
    return Private::s_docs;
}

void KCDocBase::setReadWrite(bool readwrite)
{
    map()->setReadWrite(readwrite);
    KoDocument::setReadWrite(readwrite);
}

KCMap *KCDocBase::map() const
{
    return d->map;
}

int KCDocBase::syntaxVersion() const
{
    return d->map->syntaxVersion();
}

KResourceManager* KCDocBase::resourceManager() const
{
    return d->resourceManager;
}

KCSheetAccessModel *KCDocBase::sheetAccessModel() const
{
    return d->sheetAccessModel;
}

void KCDocBase::initConfig()
{
}

bool KCDocBase::saveOdf(SavingContext &documentContext)
{
    KCElapsedTime et("OpenDocument Saving", KCElapsedTime::PrintOnlyTime);
    return saveOdfHelper(documentContext, SaveAll);
}

bool KCDocBase::saveOdfHelper(SavingContext & documentContext, SaveFlag saveFlag,
                        QString* /*plainText*/)
{
    Q_UNUSED(saveFlag);
    KOdfStore * store = documentContext.odfStore.store();
    KXmlWriter * manifestWriter = documentContext.odfStore.manifestWriter();

    KOdfStorageDevice dev(store);
    KOdfGenericStyles mainStyles;//for compile

    KXmlWriter* contentWriter = documentContext.odfStore.contentWriter();

    KXmlWriter* bodyWriter = documentContext.odfStore.bodyWriter();
    KShapeSavingContext savingContext(*bodyWriter, mainStyles, documentContext.embeddedSaver);

    //todo fixme just add a element for testing saving content.xml
    bodyWriter->startElement("office:body");
    bodyWriter->startElement("office:spreadsheet");

    // Saving the map.
    map()->saveOdf(*contentWriter, savingContext);

    savingContext.writeConnectors();

    bodyWriter->endElement(); ////office:spreadsheet
    bodyWriter->endElement(); ////office:body

    // Done with writing out the contents to the tempfile, we can now write out the automatic styles
    mainStyles.saveOdfStyles(KOdfGenericStyles::DocumentAutomaticStyles, contentWriter);

    documentContext.odfStore.closeContentWriter();

    //add manifest line for content.xml
    manifestWriter->addManifestEntry("content.xml",  "text/xml");

    mainStyles.saveOdfStylesDotXml(store, manifestWriter);

    if (!store->open("settings.xml"))
        return false;

    KXmlWriter* settingsWriter = KOdfWriteStore::createOasisXmlWriter(&dev, "office:document-settings");
    settingsWriter->startElement("office:settings");
    settingsWriter->startElement("config:config-item-set");
    settingsWriter->addAttribute("config:name", "view-settings");

    saveUnitOdf(settingsWriter);

    saveOdfSettings(*settingsWriter);

    settingsWriter->endElement(); // config:config-item-set

    settingsWriter->startElement("config:config-item-set");
    settingsWriter->addAttribute("config:name", "configuration-settings");
    settingsWriter->addConfigItem("SpellCheckerIgnoreList", d->spellListIgnoreAll.join(","));
    settingsWriter->endElement(); // config:config-item-set
    settingsWriter->endElement(); // office:settings
    settingsWriter->endElement(); // Root:element
    settingsWriter->endDocument();
    delete settingsWriter;

    if (!store->close())
        return false;

    if (!savingContext.saveDataCenter(store, manifestWriter)) {
        return false;
    }

    manifestWriter->addManifestEntry("settings.xml", "text/xml");

    setModified(false);

    return true;
}

bool KCDocBase::loadOdf(KOdfStoreReader & odfStore)
{
    QPointer<KoUpdater> updater;
    if (progressUpdater()) {
        updater = progressUpdater()->startSubtask(1, "KCells::KCDoc::loadOdf");
        updater->setProgress(0);
    }

    d->spellListIgnoreAll.clear();

    KXmlElement content = odfStore.contentDoc().documentElement();
    KXmlElement realBody(KoXml::namedItemNS(content, KOdfXmlNS::office, "body"));
    if (realBody.isNull()) {
        setErrorMessage(i18n("Invalid OASIS OpenDocument file. No office:body tag found."));
        map()->deleteLoadingInfo();
        return false;
    }
    KXmlElement body = KoXml::namedItemNS(realBody, KOdfXmlNS::office, "spreadsheet");

    if (body.isNull()) {
        kError(32001) << "No office:spreadsheet found!" << endl;
        KXmlElement childElem;
        QString localName;
        forEachElement(childElem, realBody) {
            localName = childElem.localName();
        }
        if (localName.isEmpty())
            setErrorMessage(i18n("Invalid OASIS OpenDocument file. No tag found inside office:body."));
        else
            setErrorMessage(i18n("This document is not a spreadsheet, but %1. Please try opening it with the appropriate application." , KoDocument::tagNameToDocumentType(localName)));
        map()->deleteLoadingInfo();
        return false;
    }

    KOdfLoadingContext context(odfStore.styles(), odfStore.store());

    // TODO check versions and mimetypes etc.

    // all <sheet:sheet> goes to workbook
    if (!map()->loadOdf(body, context)) {
        map()->deleteLoadingInfo();
        return false;
    }

    if (!odfStore.settingsDoc().isNull()) {
        loadOdfSettings(odfStore.settingsDoc());
    }
    initConfig();

    //update plugins that rely on bindings, as loading order can mess up the data of the plugins
    KCSheetAccessModel* sheetModel = sheetAccessModel();
    QList< KCSheet* > sheets = map()->sheetList();
    Q_FOREACH( KCSheet* sheet, sheets ){
        // This region contains the entire sheet
        const QRect region (0, 0, KS_colMax - 1, KS_rowMax - 1);
        QModelIndex index = sheetModel->index( 0, map()->indexOf( sheet ) );
          QVariant bindingModelValue = sheetModel->data( index , Qt::DisplayRole );
          KCBindingModel *curBindingModel = dynamic_cast< KCBindingModel* >( qvariant_cast< QPointer< QAbstractItemModel > >( bindingModelValue ).data() );
          if ( curBindingModel ){
              curBindingModel->emitDataChanged( region );
          }
    }

    if (updater) updater->setProgress(100);

    return true;
}

void KCDocBase::loadOdfSettings(const KXmlDocument&settingsDoc)
{
    KOdfSettings settings(settingsDoc);
    KOdfSettings::Items viewSettings = settings.itemSet("view-settings");
    if (!viewSettings.isNull()) {
        setUnit(KUnit::unit(viewSettings.parseConfigItemString("unit")));
    }
    map()->loadOdfSettings(settings);
    loadOdfIgnoreList(settings);
}

void KCDocBase::saveOdfSettings(KXmlWriter &settingsWriter)
{
    settingsWriter.startElement("config:config-item-map-indexed");
    settingsWriter.addAttribute("config:name", "Views");
    settingsWriter.startElement("config:config-item-map-entry");
    settingsWriter.addConfigItem("ViewId", QString::fromLatin1("View1"));
    saveOdfViewSettings(settingsWriter);
    //<config:config-item-map-named config:name="Tables">
    settingsWriter.startElement("config:config-item-map-named");
    settingsWriter.addAttribute("config:name", "Tables");
    foreach (KCSheet *sheet, map()->sheetList()) {
        settingsWriter.startElement("config:config-item-map-entry");
        settingsWriter.addAttribute("config:name", sheet->sheetName());
        saveOdfViewSheetSettings(sheet, settingsWriter);
        sheet->saveOdfSettings(settingsWriter);
        settingsWriter.endElement();
    }
    settingsWriter.endElement();
    settingsWriter.endElement();
    settingsWriter.endElement();
}

void KCDocBase::loadOdfIgnoreList(const KOdfSettings& settings)
{
    KOdfSettings::Items configurationSettings = settings.itemSet("configuration-settings");
    if (!configurationSettings.isNull()) {
        const QString ignorelist = configurationSettings.parseConfigItemString("SpellCheckerIgnoreList");
        //kDebug()<<" ignorelist :"<<ignorelist;
        d->spellListIgnoreAll = ignorelist.split(',', QString::SkipEmptyParts);
    }
}

void KCDocBase::paintContent(QPainter &, const QRect &)
{
}

bool KCDocBase::loadXML(const KXmlDocument &, KOdfStore *)
{
    return false;
}

KoView* KCDocBase::createViewInstance(QWidget *)
{
    return 0;
}

void KCDocBase::saveOdfViewSettings(KXmlWriter&)
{
}

void KCDocBase::saveOdfViewSheetSettings(KCSheet *, KXmlWriter&)
{
}
