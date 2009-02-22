/* This file is part of the KDE project
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

#include "kspread_export.h"

#include <KoDataCenter.h>
#include <KoXmlReader.h>

class KoStore;
class KoOdfLoadingContext;
class KoEmbeddedDocumentSaver;

class QDomElement;
class QDomDocument;
class KoXmlWriter;
class KoGenStyles;
class KoOasisSettings;

namespace KSpread
{
class ApplicationSettings;
class BindingManager;
class CalculationSettings;
class ColumnFormat;
class Damage;
class DatabaseManager;
class DependencyManager;
class Doc;
class LoadingInfo;
class NamedAreaManager;
class RecalcManager;
class RowFormat;
class Sheet;
class Style;
class StyleManager;
class ValueParser;
class ValueConverter;
class ValueFormatter;
class ValueCalc;

/**
 * The "embedded document".
 * The Map holds all the document data.
 */
class KSPREAD_EXPORT Map : public QObject, public KoDataCenter
{
Q_OBJECT
public:
  /**
   * Created an empty map.
   */
  explicit Map(Doc* doc, const char* name = 0);

  /**
   * This deletes all sheets contained in this map.
   */
  virtual ~Map();

  /**
   * \return the document this map belongs to
   */
  Doc* doc() const;

    /**
     * \brief Sets whether the document can be edited or is read only.
     */
    void setReadWrite(bool readwrite = true);

    /**
     * \return Returns whether the document can be edited or is read only.
     */
    bool isReadWrite() const;

    // KoDataCenter interface
    virtual bool completeLoading(KoStore *store);
    virtual bool completeSaving(KoStore *store, KoXmlWriter *manifestWriter, KoShapeSavingContext * context);

    /**
     * \return a pointer to the binding manager
     */
    BindingManager* bindingManager() const;

    /**
     * \return a pointer to the database manager
     */
    DatabaseManager* databaseManager() const;

    /**
     * \return a pointer to the dependency manager
     */
    DependencyManager* dependencyManager() const;

    /**
     * \return a pointer to the named area manager
     */
    NamedAreaManager* namedAreaManager() const;

    /**
     * \return a pointer to the recalculation manager
     */
    RecalcManager* recalcManager() const;

    /**
     * @return the StyleManager of this Document
     */
    StyleManager* styleManager() const;

    /**
     * @return the value parser of this Document
     */
    ValueParser* parser() const;

    /**
     * @return the value formatter of this Document
     */
    ValueFormatter* formatter() const;

    /**
     * @return the value converter of this Document
     */
    ValueConverter* converter() const;

    /**
     * @return the value calculator of this Document
     */
    ValueCalc* calc() const;

    /**
     * \return the application settings
     */
    ApplicationSettings* settings() const;

    /**
     * \return the calculation settings
     */
    CalculationSettings* calculationSettings() const;

    /**
     * \return the default row format
     */
    const ColumnFormat* defaultColumnFormat() const;

    /**
     * \return the default row format
     */
    const RowFormat* defaultRowFormat() const;

    /**
     * Sets the default column width to \p width.
     */
    void setDefaultColumnWidth( double width );

    /**
     * Sets the default row height to \p height.
     */
    void setDefaultRowHeight( double height );

  /**
   * \ingroup OpenDocument
   */
  void saveOdfSettings( KoXmlWriter &settingsWriter );

  /**
   * \ingroup OpenDocument
   */
  void loadOdfSettings( KoOasisSettings &settings );

  /**
   * \ingroup OpenDocument
   */
  bool saveOdf( KoXmlWriter & xmlWriter, KoShapeSavingContext & savingContext );

  /**
   * \ingroup OpenDocument
   */
  bool loadOdf( const KoXmlElement& mymap, KoOdfLoadingContext& odfContext );

  /**
   * \ingroup NativeFormat
   */
  bool loadXML( const KoXmlElement& mymap );

  /**
   * \ingroup NativeFormat
   */
  QDomElement save( QDomDocument& doc );


  bool loadChildren( KoStore* _store );
  bool saveChildren( KoStore* _store );

  void password( QByteArray & passwd ) const;
  bool isProtected() const;
  void setProtected( QByteArray const & passwd );
  bool checkPassword( QByteArray const & passwd ) const;

  /**
   * The sheet named @p _from is being moved to the sheet @p _to.
   * If @p  _before is true @p _from is inserted before (after otherwise)
   * @p  _to.
   */
  void moveSheet( const QString & _from, const QString & _to, bool _before = true );

  /**
   * Searches for a sheet named @p name .
   * @return a pointer to the searched sheet
   * @return @c 0 if nothing was found
   */
  Sheet* findSheet( const QString& name ) const;

  /**
   * @return a pointer to the next sheet to @p sheet
   */
  Sheet* nextSheet( Sheet* sheet ) const;

  /**
   * @return a pointer to the previous sheet to @p sheet
   */
  Sheet* previousSheet( Sheet* ) const;

  /**
   * Creates a new sheet.
   * The sheet is not added to the map nor added to the GUI.
   * @return a pointer to a new Sheet
   */
  Sheet* createSheet();

  /**
   * Adds @p sheet to this map.
   * The sheet becomes the active sheet.
  */
  void addSheet( Sheet* sheet );

  /**
   * Creates a new sheet.
   * Adds a new sheet to this map.
   * @return a pointer to the new sheet
   */
  Sheet* addNewSheet();

  /**
   * @return a pointer to the sheet at index @p index in this map
   * @return @c 0 if the index exceeds the list boundaries
   */
  Sheet* sheet( int index ) const;

  /**
   * @return the list of sheets in this map
   */
  QList<Sheet*>& sheetList() const;

  /**
   * @return amount of sheets in this map
   */
  int count() const;

    void removeSheet(Sheet* sheet);
    void reviveSheet(Sheet* sheet);

  QStringList visibleSheets() const;
  QStringList hiddenSheets() const;

  void increaseLoadedRowsCounter(int i = 1);

    /**
     * \ingroup OpenDocument
     * \ingroup NativeFormat
     * \return true if the document is currently loading.
     */
    bool isLoading() const;

    /**
     * \ingroup OpenDocument
     * \ingroup NativeFormat
     * Creates the loading info, if it does not exist yet.
     * \return the loading info
     */
    LoadingInfo* loadingInfo() const;

    /**
     * \ingroup OpenDocument
     * \ingroup NativeFormat
     * Deletes the loading info. Called after loading is complete.
     */
    void deleteLoadingInfo();

    /**
     * \ingroup Operations
     */
    void addDamage(Damage* damage);

public Q_SLOTS:
    /**
     * \ingroup Operations
     */
    void flushDamages();

    /**
     * \ingroup Operations
     */
    void handleDamages(const QList<Damage*>& damages);

Q_SIGNALS:
    /**
     * \ingroup Operations
     */
    void damagesFlushed(const QList<Damage*>& damages);

    /**
     * Emitted, if a newly created sheet was added to the document.
     */
    void sheetAdded(Sheet* sheet);

    /**
     * Emitted, if a sheet was deleted from the document.
     */
    void sheetRemoved(Sheet* sheet);

    /**
     * Emitted, if a sheet was revived, i.e. a deleted sheet was reinserted.
     */
    void sheetRevived(Sheet* sheet);

private:
    Q_DISABLE_COPY( Map )

  class Private;
  Private * const d;
};

} // namespace KSpread

#endif
