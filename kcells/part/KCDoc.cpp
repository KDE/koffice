/* This file is part of the KDE project
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

// Local
#include "KCDoc.h"
#include "../DocBase_p.h"

#include <unistd.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <pwd.h>

#include <QApplication>
#include <QFont>
#include <QTimer>
#include <QList>
#include <QPainter>
#include <QGraphicsItem>

#include <kstandarddirs.h>
#include <kdebug.h>
#include <kconfig.h>
#include <kmessagebox.h>
#include <ktemporaryfile.h>

#include <KoApplication.h>
#include <KoDataCenterBase.h>
#include <KoDocumentInfo.h>
#include <KoMainWindow.h>
#include <KoOdfSettings.h>
#include <KOdfLoadingContext.h>
#include <KoOdfStylesReader.h>
#include <KoOdfReadStore.h>
#include <KoOdfWriteStore.h>
#include <KoResourceManager.h>
#include <KoShapeConfigFactoryBase.h>
#include <KoShapeFactoryBase.h>
#include <KoShapeManager.h>
#include <KoShapeRegistry.h>
#include <KoStoreDevice.h>
#include <KoStyleStack.h>
#include <KoXmlNS.h>
#include <KoXmlWriter.h>
#include <KoZoomHandler.h>
#include <KoShapeSavingContext.h>
#include <KoUpdater.h>
#include <KoProgressUpdater.h>

#include "KCBindingManager.h"
#include "KCCalculationSettings.h"
#include "KCCanvas.h"
#include "KCDependencyManager.h"
#include "KCFactory.h"
#include "KCFormula.h"
#include "KCFunction.h"
#include "KCFunctionModuleRegistry.h"
#include "KCHeaderFooter.h"
#include "KCLoadingInfo.h"
#include "KCLocalization.h"
#include "KCMap.h"
#include "KCNamedAreaManager.h"
#include "KCPrintSettings.h"
#include "KCRecalcManager.h"
#include "KCSheet.h"
#include "KCSheetPrint.h"
#include "KCStyleManager.h"
#include "Util.h"
#include "KCView.h"
#include "KCSheetAccessModel.h"
#include "KCBindingModel.h"

// D-Bus
#include "interfaces/MapAdaptor.h"
#include "interfaces/SheetAdaptor.h"
#include <KoChartInterface.h>
#include <QtDBus/QtDBus>

// chart shape
#include "chart/KCChartDialog.h"

// ui
#include "ui/Selection.h"
#include "ui/SheetView.h"


class KCDoc::Private
{
public:
    KCMap *map;
    static QList<KCDoc*> s_docs;
    static int s_docId;

    // document properties
    bool configLoadFromFile       : 1;
    QStringList spellListIgnoreAll;
    SavedDocParts savedDocParts;
    KCSheetAccessModel *sheetAccessModel;
    KoResourceManager *resourceManager;
};

// Make sure an appropriate DTD is available in www/koffice/DTD if changing this value
static const char * CURRENT_DTD_VERSION = "1.2";

/*****************************************************************************
 *
 * KCDoc
 *
 *****************************************************************************/

QList<KCDoc*> KCDoc::Private::s_docs;
int KCDoc::Private::s_docId = 0;

KCDoc::KCDoc(QWidget *parentWidget, QObject* parent, bool singleViewMode)
        : KCDocBase(parentWidget, parent, singleViewMode)
        , dd(new Private)
{
    connect(d->map, SIGNAL(sheetAdded(KCSheet*)), this, SLOT(sheetAdded(KCSheet*)));
    new MapAdaptor(d->map);
    QDBusConnection::sessionBus().registerObject('/' + objectName() + '/' + d->map->objectName(), d->map);


    // Init chart shape factory with KCells's specific configuration panels.
    KoShapeFactoryBase *chartShape = KoShapeRegistry::instance()->value(ChartShapeId);
    if (chartShape) {
        QList<KoShapeConfigFactoryBase*> panels = KCChartDialog::panels(d->map);
        chartShape->setOptionPanels(panels);
    }

    connect(d->map, SIGNAL(commandAdded(QUndoCommand *)),
            this, SLOT(addCommand(QUndoCommand *)));

    setComponentData(KCFactory::global(), false);
    setTemplateType("kcells_template");

    // Load the function modules.
    KCFunctionModuleRegistry::instance()->loadFunctionModules();
}

KCDoc::~KCDoc()
{
    //don't save config when kword is embedded into konqueror
    if (isReadWrite())
        saveConfig();

    delete dd;
}

void KCDoc::openTemplate(const KUrl& url)
{
    map()->loadingInfo()->setLoadTemplate(true);
    KoDocument::openTemplate(url);
    map()->deleteLoadingInfo();
    initConfig();
}

void KCDoc::initEmpty()
{
    KSharedConfigPtr config = KCFactory::global().config();
    const int page = config->group("Parameters").readEntry("NbPage", 1);

    for (int i = 0; i < page; ++i)
        map()->addNewSheet();

    resetURL();
    initConfig();
    map()->styleManager()->createBuiltinStyles();

    KoDocument::initEmpty();
}

void KCDoc::saveConfig()
{
    if (isEmbedded() || !isReadWrite())
        return;
    KSharedConfigPtr config = KCFactory::global().config();
}

void KCDoc::initConfig()
{
    KSharedConfigPtr config = KCFactory::global().config();

    const int page = config->group("KCells Page Layout").readEntry("Default unit page", 0);
    setUnit(KoUnit((KoUnit::Unit) page));
}

KoView* KCDoc::createViewInstance(QWidget* parent)
{
    return new KCView(parent, this);
}

bool KCDoc::saveChildren(KoStore* _store)
{
    return map()->saveChildren(_store);
}

int KCDoc::supportedSpecialFormats() const
{
    return KoDocument::supportedSpecialFormats();
}

bool KCDoc::completeSaving(KoStore* _store)
{
    Q_UNUSED(_store);
    return true;
}


QDomDocument KCDoc::saveXML()
{
    /* don't pull focus away from the editor if this is just a background
       autosave */
    if (!isAutosaving()) {
        foreach(KoView* view, views())
        static_cast<KCView *>(view)->selection()->emitCloseEditor(true);
    }

    QDomDocument doc = KoDocument::createDomDocument("kcells", "spreadsheet", CURRENT_DTD_VERSION);
    QDomElement spread = doc.documentElement();
    spread.setAttribute("editor", "KCells");
    spread.setAttribute("mime", "application/x-kcells");
    spread.setAttribute("syntaxVersion", CURRENT_SYNTAX_VERSION);

    if (!d->spellListIgnoreAll.isEmpty()) {
        QDomElement spellCheckIgnore = doc.createElement("SPELLCHECKIGNORELIST");
        spread.appendChild(spellCheckIgnore);
        for (QStringList::Iterator it = d->spellListIgnoreAll.begin(); it != d->spellListIgnoreAll.end(); ++it) {
            QDomElement spellElem = doc.createElement("SPELLCHECKIGNOREWORD");
            spellCheckIgnore.appendChild(spellElem);
            spellElem.setAttribute("word", *it);
        }
    }

    SavedDocParts::const_iterator iter = d->savedDocParts.constBegin();
    SavedDocParts::const_iterator end  = d->savedDocParts.constEnd();
    while (iter != end) {
        // save data we loaded in the beginning and which has no owner back to file
        spread.appendChild(iter.value());
        ++iter;
    }

    QDomElement e = map()->save(doc);
    if (!views().isEmpty()) { // no view if embedded document
        // Save visual info for the first view, such as active sheet and active cell
        // It looks like a hack, but reopening a document creates only one view anyway (David)
        KCView *const view = static_cast<KCView*>(views().first());
        KCCanvas *const canvas = view->canvasWidget();
        e.setAttribute("activeTable",  canvas->activeSheet()->sheetName());
        e.setAttribute("markerColumn", view->selection()->marker().x());
        e.setAttribute("markerRow",    view->selection()->marker().y());
        e.setAttribute("xOffset",      canvas->xOffset());
        e.setAttribute("yOffset",      canvas->yOffset());
    }
    spread.appendChild(e);

    setModified(false);

    return doc;
}

bool KCDoc::loadChildren(KoStore* _store)
{
    return map()->loadChildren(_store);
}


bool KCDoc::loadXML(const KoXmlDocument& doc, KoStore*)
{
    QPointer<KoUpdater> updater;
    if (progressUpdater()) {
        updater = progressUpdater()->startSubtask(1, "KCells::KCDoc::loadXML");
        updater->setProgress(0);
    }

    d->spellListIgnoreAll.clear();
    // <spreadsheet>
    KoXmlElement spread = doc.documentElement();

    if (spread.attribute("mime") != "application/x-kcells" && spread.attribute("mime") != "application/vnd.kde.kcells") {
        setErrorMessage(i18n("Invalid document. Expected mimetype application/x-kcells or application/vnd.kde.kcells, got %1" , spread.attribute("mime")));
        return false;
    }

    bool ok = false;
    int version = spread.attribute("syntaxVersion").toInt(&ok);
    map()->setSyntaxVersion(ok ? version : 0);
    if (map()->syntaxVersion() > CURRENT_SYNTAX_VERSION) {
        int ret = KMessageBox::warningContinueCancel(
                      0, i18n("This document was created with a newer version of KCells (syntax version: %1)\n"
                              "When you open it with this version of KCells, some information may be lost.", map()->syntaxVersion()),
                      i18n("File Format Mismatch"), KStandardGuiItem::cont());
        if (ret == KMessageBox::Cancel) {
            setErrorMessage("USER_CANCELED");
            return false;
        }
    }

    // <locale>
    KoXmlElement loc = spread.namedItem("locale").toElement();
    if (!loc.isNull())
        static_cast<KCLocalization*>(map()->calculationSettings()->locale())->load(loc);

    if (updater) updater->setProgress(5);

    KoXmlElement defaults = spread.namedItem("defaults").toElement();
    if (!defaults.isNull()) {
        double dim = defaults.attribute("row-height").toDouble(&ok);
        if (!ok)
            return false;
        map()->setDefaultRowHeight(dim);

        dim = defaults.attribute("col-width").toDouble(&ok);

        if (!ok)
            return false;

        map()->setDefaultColumnWidth(dim);
    }

    KoXmlElement ignoreAll = spread.namedItem("SPELLCHECKIGNORELIST").toElement();
    if (!ignoreAll.isNull()) {
        KoXmlElement spellWord = spread.namedItem("SPELLCHECKIGNORELIST").toElement();

        spellWord = spellWord.firstChild().toElement();
        while (!spellWord.isNull()) {
            if (spellWord.tagName() == "SPELLCHECKIGNOREWORD") {
                d->spellListIgnoreAll.append(spellWord.attribute("word"));
            }
            spellWord = spellWord.nextSibling().toElement();
        }
    }

    if (updater) updater->setProgress(40);
    // In case of reload (e.g. from konqueror)
    qDeleteAll(map()->sheetList());
    map()->sheetList().clear();

    KoXmlElement styles = spread.namedItem("styles").toElement();
    if (!styles.isNull()) {
        if (!map()->styleManager()->loadXML(styles)) {
            setErrorMessage(i18n("Styles cannot be loaded."));
            return false;
        }
    }

    // <map>
    KoXmlElement mymap = spread.namedItem("map").toElement();
    if (mymap.isNull()) {
        setErrorMessage(i18n("Invalid document. No map tag."));
        return false;
    }
    if (!map()->loadXML(mymap)) {
        return false;
    }

    // named areas
    const KoXmlElement areaname = spread.namedItem("areaname").toElement();
    if (!areaname.isNull())
        map()->namedAreaManager()->loadXML(areaname);

    //Backwards compatibility with older versions for paper layout
    if (map()->syntaxVersion() < 1) {
        KoXmlElement paper = spread.namedItem("paper").toElement();
        if (!paper.isNull()) {
            loadPaper(paper);
        }
    }

    if (updater) updater->setProgress(85);

    KoXmlElement element(spread.firstChild().toElement());
    while (!element.isNull()) {
        QString tagName(element.tagName());

        if (tagName != "locale" && tagName != "map" && tagName != "styles"
                && tagName != "SPELLCHECKIGNORELIST" && tagName != "areaname"
                && tagName != "paper") {
            // belongs to a plugin, load it and save it for later use
            d->savedDocParts[ tagName ] = KoXml::asQDomElement(QDomDocument(), element);
        }

        element = element.nextSibling().toElement();
    }

    if (updater) updater->setProgress(90);
    initConfig();
    if (updater) updater->setProgress(100);

    return true;
}

void KCDoc::loadPaper(KoXmlElement const & paper)
{
    KoPageLayout pageLayout;
    pageLayout.format = KoPageFormat::formatFromString(paper.attribute("format"));
    pageLayout.orientation = (paper.attribute("orientation")  == "Portrait")
                             ? KoPageFormat::Portrait : KoPageFormat::Landscape;

    // <borders>
    KoXmlElement borders = paper.namedItem("borders").toElement();
    if (!borders.isNull()) {
        pageLayout.leftMargin   = MM_TO_POINT(borders.attribute("left").toFloat());
        pageLayout.rightMargin  = MM_TO_POINT(borders.attribute("right").toFloat());
        pageLayout.topMargin    = MM_TO_POINT(borders.attribute("top").toFloat());
        pageLayout.bottomMargin = MM_TO_POINT(borders.attribute("bottom").toFloat());
    }

    //apply to all sheet
    foreach(KCSheet* sheet, map()->sheetList()) {
        sheet->printSettings()->setPageLayout(pageLayout);
    }

    QString hleft, hright, hcenter;
    QString fleft, fright, fcenter;
    // <head>
    KoXmlElement head = paper.namedItem("head").toElement();
    if (!head.isNull()) {
        KoXmlElement left = head.namedItem("left").toElement();
        if (!left.isNull())
            hleft = left.text();
        KoXmlElement center = head.namedItem("center").toElement();
        if (!center.isNull())
            hcenter = center.text();
        KoXmlElement right = head.namedItem("right").toElement();
        if (!right.isNull())
            hright = right.text();
    }
    // <foot>
    KoXmlElement foot = paper.namedItem("foot").toElement();
    if (!foot.isNull()) {
        KoXmlElement left = foot.namedItem("left").toElement();
        if (!left.isNull())
            fleft = left.text();
        KoXmlElement center = foot.namedItem("center").toElement();
        if (!center.isNull())
            fcenter = center.text();
        KoXmlElement right = foot.namedItem("right").toElement();
        if (!right.isNull())
            fright = right.text();
    }
    //The macro "<sheet>" formerly was typed as "<table>"
    hleft   = hleft.replace("<table>", "<sheet>");
    hcenter = hcenter.replace("<table>", "<sheet>");
    hright  = hright.replace("<table>", "<sheet>");
    fleft   = fleft.replace("<table>", "<sheet>");
    fcenter = fcenter.replace("<table>", "<sheet>");
    fright  = fright.replace("<table>", "<sheet>");

    foreach(KCSheet* sheet, map()->sheetList()) {
        sheet->print()->headerFooter()->setHeadFootLine(hleft, hcenter, hright,
                fleft, fcenter, fright);
    }
}

bool KCDoc::completeLoading(KoStore* store)
{
    kDebug(36001) << "------------------------ COMPLETING --------------------";

    setModified(false);
    bool ok = map()->completeLoading(store);

    kDebug(36001) << "------------------------ COMPLETION DONE --------------------";
    return ok;
}


bool KCDoc::docData(QString const & xmlTag, QDomElement & data)
{
    SavedDocParts::iterator iter = d->savedDocParts.find(xmlTag);
    if (iter == d->savedDocParts.end())
        return false;
    data = iter.value();
    d->savedDocParts.erase(iter);
    return true;
}

void KCDoc::addIgnoreWordAllList(const QStringList & _lst)
{
    d->spellListIgnoreAll = _lst;
}

QStringList KCDoc::spellListIgnoreAll() const
{
    return d->spellListIgnoreAll;
}

void KCDoc::paintContent(QPainter& painter, const QRect& rect)
{
    paintContent(painter, rect, 0);
}

void KCDoc::paintContent(QPainter& painter, const QRect& rect, KCSheet* _sheet)
{
    if (rect.isEmpty()) {
        return;
    }
    KCSheet *const sheet = _sheet ? _sheet : d->map->sheet(0);

    const KoPageLayout pageLayout = sheet->printSettings()->pageLayout();
    QPixmap thumbnail(pageLayout.width, pageLayout.height);
    thumbnail.fill(Qt::white);

    SheetView sheetView(sheet);

    const qreal zoom = sheet->printSettings()->zoom();
    KoZoomHandler zoomHandler;
    zoomHandler.setZoom(zoom);
    sheetView.setViewConverter(&zoomHandler);

    sheetView.setPaintCellRange(sheet->print()->cellRange(1)); // first page

    QPainter pixmapPainter(&thumbnail);
    pixmapPainter.setClipRect(QRect(QPoint(0, 0), thumbnail.size()));
    sheetView.paintCells(pixmapPainter, QRect(0, 0, pageLayout.width, pageLayout.height), QPointF(0,0));

    // The pixmap gets scaled to fit the rectangle.
    painter.drawPixmap(rect & QRect(0, 0, 100, 100), thumbnail);
}

void KCDoc::updateAllViews()
{
    emit updateView();
}

void KCDoc::updateBorderButton()
{
    foreach(KoView* view, views())
    static_cast<KCView*>(view)->updateBorderButton();
}

void KCDoc::addIgnoreWordAll(const QString & word)
{
    if (d->spellListIgnoreAll.indexOf(word) == -1)
        d->spellListIgnoreAll.append(word);
}

void KCDoc::clearIgnoreWordAll()
{
    d->spellListIgnoreAll.clear();
}

void KCDoc::addView(KoView *_view)
{
    KoDocument::addView(_view);
    foreach(KoView* view, views())
    static_cast<KCView*>(view)->selection()->emitCloseEditor(true);
}

void KCDoc::loadConfigFromFile()
{
    d->configLoadFromFile = true;
}

bool KCDoc::configLoadFromFile() const
{
    return d->configLoadFromFile;
}

void KCDoc::sheetAdded(KCSheet* sheet)
{
    new SheetAdaptor(sheet);
    QString dbusPath('/' + sheet->map()->objectName() + '/' + objectName());
    if (sheet->parent()) {
        dbusPath.prepend('/' + sheet->parent()->objectName());
    }
    QDBusConnection::sessionBus().registerObject(dbusPath, sheet);

}

void KCDoc::saveOdfViewSettings(KoXmlWriter& settingsWriter)
{
    if (!views().isEmpty()) { // no view if embedded document
        // Save visual info for the first view, such as active sheet and active cell
        // It looks like a hack, but reopening a document creates only one view anyway (David)
        KCView *const view = static_cast<KCView*>(views().first());
        // save current sheet selection before to save marker, otherwise current pos is not saved
        view->saveCurrentSheetSelection();
        //<config:config-item config:name="ActiveTable" config:type="string">Feuille1</config:config-item>
        if (KCSheet *sheet = view->activeSheet()) {
            settingsWriter.addConfigItem("ActiveTable", sheet->sheetName());
        }
    }
}

void KCDoc::saveOdfViewSheetSettings(KCSheet *sheet, KoXmlWriter &settingsWriter)
{
    if (!views().isEmpty()) {
        KCView *const view = static_cast<KCView*>(views().first());
        QPoint marker = view->markerFromSheet(sheet);
        QPointF offset = view->offsetFromSheet(sheet);
        settingsWriter.addConfigItem("CursorPositionX", marker.x() - 1);
        settingsWriter.addConfigItem("CursorPositionY", marker.y() - 1);
        settingsWriter.addConfigItem("xOffset", offset.x());
        settingsWriter.addConfigItem("yOffset", offset.y());
    }
}

bool KCDoc::saveOdfHelper(SavingContext &documentContext, SaveFlag saveFlag, QString *plainText)
{
    /* don't pull focus away from the editor if this is just a background
       autosave */
    if (!isAutosaving()) {
        foreach(KoView* view, views())
            static_cast<KCView *>(view)->selection()->emitCloseEditor(true);
    }

    return KCDocBase::saveOdfHelper(documentContext, saveFlag, plainText);
}

#include "KCDoc.moc"
