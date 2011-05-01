/* This file is part of the KDE project
   Copyright 2010 Marijn Kruisselbrink <m.kruisselbrink@student.tue.nl>
   Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>

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

#ifndef __MAP_H__
#define __MAP_H__

#include <QList>
#include <QObject>
#include <QString>
#include <QStringList>

#include "KCProtectableObject.h"

#include "kcells_export.h"

#include <KoDataCenterBase.h>
#include <KoXmlReader.h>

class KoStore;
class KoOdfLoadingContext;
class KoEmbeddedDocumentSaver;
class KoStyleManager;
class KoResourceManager;

class KCompletion;

class QDomElement;
class QDomDocument;
class QUndoCommand;

class KoXmlWriter;
class KoGenStyles;
class KoOasisSettings;

class KCApplicationSettings;
class KCBindingManager;
class KCCalculationSettings;
class KCColumnFormat;
class KCDamage;
class DatabaseManager;
class KCDependencyManager;
class KCDocBase;
class KCLoadingInfo;
class KCNamedAreaManager;
class KCRecalcManager;
class KCRowFormat;
class KCSheet;
class KCStyle;
class KCStyleManager;
class KCValueParser;
class KCValueConverter;
class KCValueFormatter;
class KCValueCalc;

/**
 * The "embedded document".
 * The KCMap holds all the document data.
 */
class KCELLS_EXPORT KCMap : public QObject, public KoDataCenterBase, public KCProtectableObject
{
    Q_OBJECT
public:
    /**
     * Created an empty map.
     */
    explicit KCMap(KCDocBase* doc = 0, int syntaxVersion = 1);

    /**
     * This deletes all sheets contained in this map.
     */
    virtual ~KCMap();

    /**
     * \return the document this map belongs to
     */
    KCDocBase* doc() const;

    /**
     * \brief Sets whether the document can be edited or is read only.
     */
    void setReadWrite(bool readwrite = true);

    /**
     * \return Returns whether the document can be edited or is read only.
     */
    bool isReadWrite() const;

    // KoDataCenterBase interface
    virtual bool completeLoading(KoStore *store);
    virtual bool completeSaving(KoStore *store, KoXmlWriter *manifestWriter, KoShapeSavingContext * context);

    /**
     * \return a pointer to the binding manager
     */
    KCBindingManager* bindingManager() const;

    /**
     * \return a pointer to the database manager
     */
    DatabaseManager* databaseManager() const;

    /**
     * \return a pointer to the dependency manager
     */
    KCDependencyManager* dependencyManager() const;

    /**
     * \return a pointer to the named area manager
     */
    KCNamedAreaManager* namedAreaManager() const;

    /**
     * \return a pointer to the recalculation manager
     */
    KCRecalcManager* recalcManager() const;

    /**
     * @return the KCStyleManager of this Document
     */
    KCStyleManager* styleManager() const;

    /**
     * @return the KoStyleManager of this Document
     */
    KoStyleManager* textStyleManager() const;

    /**
     * @return the value parser of this Document
     */
    KCValueParser* parser() const;

    /**
     * @return the value formatter of this Document
     */
    KCValueFormatter* formatter() const;

    /**
     * @return the value converter of this Document
     */
    KCValueConverter* converter() const;

    /**
     * @return the value calculator of this Document
     */
    KCValueCalc* calc() const;

    /**
     * \return the application settings
     */
    KCApplicationSettings* settings() const;

    /**
     * \return the calculation settings
     */
    KCCalculationSettings* calculationSettings() const;

    /**
     * \return the default row format
     */
    const KCColumnFormat* defaultColumnFormat() const;

    /**
     * \return the default row format
     */
    const KCRowFormat* defaultRowFormat() const;

    /**
     * Sets the default column width to \p width.
     */
    void setDefaultColumnWidth(double width);

    /**
     * Sets the default row height to \p height.
     */
    void setDefaultRowHeight(double height);

    /**
     * \ingroup OpenDocument
     */
    void loadOdfSettings(KoOasisSettings &settings);

    /**
     * \ingroup OpenDocument
     */
    bool saveOdf(KoXmlWriter & xmlWriter, KoShapeSavingContext & savingContext);

    /**
     * \ingroup OpenDocument
     */
    bool loadOdf(const KoXmlElement& mymap, KoOdfLoadingContext& odfContext);

    /**
     * \ingroup NativeFormat
     */
    bool loadXML(const KoXmlElement& mymap);

    /**
     * \ingroup NativeFormat
     */
    QDomElement save(QDomDocument& doc);


    bool loadChildren(KoStore* _store);
    bool saveChildren(KoStore* _store);

    /**
     * The sheet named @p _from is being moved to the sheet @p _to.
     * If @p  _before is true @p _from is inserted before (after otherwise)
     * @p  _to.
     */
    void moveSheet(const QString & _from, const QString & _to, bool _before = true);

    /**
     * Searches for a sheet named @p name .
     * @return a pointer to the searched sheet
     * @return @c 0 if nothing was found
     */
    KCSheet* findSheet(const QString& name) const;

    /**
     * @return a pointer to the next sheet to @p sheet
     */
    KCSheet* nextSheet(KCSheet* sheet) const;

    /**
     * @return a pointer to the previous sheet to @p sheet
     */
    KCSheet* previousSheet(KCSheet*) const;

    /**
     * Creates a new sheet.
     * The sheet is not added to the map nor added to the GUI.
     * @return a pointer to a new KCSheet
     */
    KCSheet* createSheet(const QString& name = QString());

    /**
     * Adds @p sheet to this map.
     * The sheet becomes the active sheet.
    */
    void addSheet(KCSheet* sheet);

    /**
     * Creates a new sheet.
     * Adds a new sheet to this map.
     * @return a pointer to the new sheet
     */
    KCSheet* addNewSheet(const QString& name = QString());

    /**
     * @return a pointer to the sheet at index @p index in this map
     * @return @c 0 if the index exceeds the list boundaries
     */
    KCSheet* sheet(int index) const;

    /**
     * @return index of @p sheet in this map
     * @return @c 0 if the index exceeds the list boundaries
     */
    int indexOf(KCSheet* sheet) const;

    /**
     * @return the list of sheets in this map
     */
    QList<KCSheet*>& sheetList() const;

    /**
     * @return amount of sheets in this map
     */
    int count() const;

    void removeSheet(KCSheet* sheet);
    void reviveSheet(KCSheet* sheet);

    QStringList visibleSheets() const;
    QStringList hiddenSheets() const;

    int increaseLoadedRowsCounter(int i = 1);

    /**
     * \ingroup OpenDocument
     * \ingroup NativeFormat
     * \return true if the document is currently loading.
     */
    bool isLoading() const;

    /**
     * \return the document's syntax version
     * \ingroup NativeFormat
     */
    int syntaxVersion() const;

    /**
     * Sets the document's syntax \p version.
     * \ingroup NativeFormat
     */
    void setSyntaxVersion(int version);

    /**
     * \ingroup OpenDocument
     * \ingroup NativeFormat
     * Creates the loading info, if it does not exist yet.
     * \return the loading info
     */
    KCLoadingInfo* loadingInfo() const;

    /**
     * \ingroup OpenDocument
     * \ingroup NativeFormat
     * Deletes the loading info. Called after loading is complete.
     */
    void deleteLoadingInfo();

    /**
     * \return the KCompletion object, that allows user input completions.
     */
    KCompletion &stringCompletion();

    /**
     * Adds \p string to the list of string values in order to be able to
     * complete user inputs.
     */
    void addStringCompletion(const QString &string);

    /**
     * \ingroup Damages
     */
    void addDamage(KCDamage* damage);

    /**
     * Return a pointer to the resource manager associated with the
     * document. The resource manager contains
     * document wide resources * such as variable managers, the image
     * collection and others.
     * @see KoCanvasBase::resourceManager()
     */
    KoResourceManager *resourceManager() const;
public Q_SLOTS:
    /**
     * \ingroup Damages
     */
    void flushDamages();

    /**
     * \ingroup Damages
     */
    void handleDamages(const QList<KCDamage*>& damages);

    /**
     * Emits the signal commandAdded(QUndoCommand *).
     * You have to connect the signal to the object holding the undo stack or
     * any relay object, that propagates \p command to the undo stack.
     */
    void addCommand(QUndoCommand *command);

Q_SIGNALS:
    /**
     * \ingroup Damages
     */
    void damagesFlushed(const QList<KCDamage*>& damages);

    /**
     * Emitted, if a command was added by addCommand(QUndoCommand *).
     */
    void commandAdded(QUndoCommand *command);

    /**
     * Emitted, if a newly created sheet was added to the document.
     */
    void sheetAdded(KCSheet* sheet);

    /**
     * Emitted, if a sheet was deleted from the document.
     */
    void sheetRemoved(KCSheet* sheet);

    /**
     * Emitted, if a sheet was revived, i.e. a deleted sheet was reinserted.
     */
    void sheetRevived(KCSheet* sheet);

    /**
     * Emitted, if a status \p message should be shown in the status bar
     * for \p timeout msecs.
     */
    void statusMessage(const QString &message, int timeout);

private:
    Q_DISABLE_COPY(KCMap)

    class Private;
    Private * const d;
};

#endif
