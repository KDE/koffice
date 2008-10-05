/* This file is part of the KDE project
   Copyright 2007 Stefan Nikolaus <stefan.nikolaus@kdemail.net>
   Copyright 1998,1999 Torben Weis <weis@kde.org>
   Copyright 1999-2007 The KSpread Team <koffice-devel@kde.org>

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

#ifndef KSPREAD_SHEET
#define KSPREAD_SHEET

#include <QClipboard>
#include <QHash>
#include <QList>
#include <QPixmap>
#include <QRect>
//#include <QWidget>

#include <KoDocument.h>
#include <KoDocumentChild.h>
#include <KoOasisSettings.h> // for KoOasisSettings::NamedMap
#include <KoShapeControllerBase.h>
#include <KoShapeUserData.h>
#include <KoXmlReader.h>

#include "Cell.h"
#include "Style.h"
#include "Global.h"

#include "kspread_export.h"

class QDomElement;
class QUndoCommand;
class QWidget;

class KoDataCenter;
class KoDocumentEntry;
class KoStyleStack;
class KoGenStyles;
class KoOasisSettings;
class KoOdfStylesReader;
class KoShape;
class KoShapeSavingContext;
class KoXmlWriter;

namespace KSpread
{
class Cell;
class CellStorage;
class ColumnFormat;
class CommentStorage;
class ConditionsStorage;
class FormulaStorage;
class Doc;
class FusionStorage;
class LinkStorage;
class Map;
class OdfLoadingContext;
class OdfSavingContext;
class PrintManager;
class PrintSettings;
class Region;
class RowFormat;
class Selection;
class Sheet;
class SheetPrint;
class Style;
class StyleStorage;
class Validity;
class ValidityStorage;
class ValueStorage;
class View;

/**
 * A sheet contains several cells.
 */
class KSPREAD_EXPORT Sheet : public KoShapeUserData, public KoShapeControllerBase
{
    Q_OBJECT
    Q_PROPERTY( QString sheetName READ sheetName )
    Q_PROPERTY( bool autoCalc READ isAutoCalculationEnabled WRITE setAutoCalculationEnabled )
    Q_PROPERTY( bool showGrid READ getShowGrid WRITE setShowGrid )

public:
    enum ChangeRef       { ColumnInsert, ColumnRemove, RowInsert, RowRemove };
    enum TestType        { Text, Validity, Comment, ConditionalCellAttribute };

    /**
     * Creates a sheet in \p map with the name \p sheetName.
     */
    Sheet(Map* map, const QString& sheetName);

    /**
     * Copy constructor.
     * Creates a sheet with the contents and the settings of \p other.
     */
    Sheet(const Sheet& other);

    /**
     * Destructor.
     */
    ~Sheet();

    /**
     * \return the map this sheet belongs to
     */
    Map* map() const;

    /**
     * \return the document this sheet belongs to
     */
    Doc* doc() const;

    // KoShapeControllerBase interface
    virtual void addShape(KoShape* shape);
    virtual void removeShape(KoShape* shape);
    virtual QMap<QString, KoDataCenter*> dataCenterMap() const;

    /**
     * \ingroup Embedding
     * Returns the sheet's shapes.
     * \return the shapes this sheet contains
     */
    QList<KoShape*> shapes() const;

    //////////////////////////////////////////////////////////////////////////
    //
    //BEGIN Methods related to sheet properties
    //

    /**
     * \return the name of this sheet
     */
    QString sheetName() const;

    /**
     * Renames a sheet. This will automatically adapt all formulas
     * in all sheets and all cells to reflect the new name.
     *
     * If the name really changed then sig_nameChanged is emitted
     * and the GUI will reflect the change.
     *
     * @param name The new sheet name.
     * @param init If set to true then no formula will be changed and no signal
     *             will be emitted and no undo action created. Usually you do not
     *             want to do that.
     *
     * @return @c true if the sheet was renamed successfully
     * @return @c false if the sheet could not be renamed. Usually the reason is
     * that this name is already used.
     *
     * @see changeCellTabName
     * @see TabBar::renameTab
     * @see sheetName
     */
    bool setSheetName(const QString& name, bool init = false);

    /**
     * \return \c true , if a document is currently loading
     */
    bool isLoading();

    /**
     * Returns the layout direction of the sheet.
     */
    Qt::LayoutDirection layoutDirection() const;

    /**
     * Sets the layout direction of the sheet. For example, for Arabic or Hebrew
     * documents, it is possibly to layout the sheet from right to left.
     */
    void setLayoutDirection( Qt::LayoutDirection dir );

    /**
     * Returns, if the grid shall be shown on the screen
     */
    bool getShowGrid() const;

    /**
     * Sets, if the grid shall be shown on the screen
     */
    void setShowGrid( bool _showGrid );

    /**
     * Sets, if formula shall be shown instead of the result
     */
    bool getShowFormula() const;

    void setShowFormula(bool _showFormula);

    /**
     * Sets, if indicator must be shown when the cell holds a formula
     */
    bool getShowFormulaIndicator() const;

    void setShowFormulaIndicator(bool _showFormulaIndicator);

    /**
     * Returns true if comment indicator is visible.
     */
    bool getShowCommentIndicator() const;

    /**
     * If b is true, comment indicator is visible, otherwise
     * it will be hidden.
     */
    void setShowCommentIndicator( bool b );

    bool getLcMode() const;

    void setLcMode(bool _lcMode);

    bool isAutoCalculationEnabled() const;

    void setAutoCalculationEnabled(bool enable);

    bool getShowColumnNumber() const;

    void setShowColumnNumber(bool _showColumnNumber);

    bool getHideZero() const;

    void setHideZero(bool _hideZero);

    bool getFirstLetterUpper() const;

    void setFirstLetterUpper(bool _firstUpper);

    /**
     * @return true if this sheet is hidden
     */
    bool isHidden()const;

    /**
     * Hides or shows this sheets
     */
    void setHidden( bool hidden );

    /**
     * @return a flag that indicates whether the sheet should paint the page breaks.
     *
     * @see setShowPageBorders
     * @see Sheet::Private::showPageBorders
     */
    bool isShowPageBorders() const;

    /**
     * Turns the page break lines on or off.
     *
     * @see isShowPageBorders
     * @see Sheet::Private::showPageBorders
     */
    void setShowPageBorders( bool _b );

    //
    //END Methods related to sheet properties
    //
    //////////////////////////////////////////////////////////////////////////
    //
    //BEGIN Methods related to KSpread's old file format
    //

    /**
     * \ingroup NativeFormat
     * Saves the sheet and all it's children in XML format
     */
    QDomElement saveXML( QDomDocument& );

    /**
     * \ingroup NativeFormat
     * Loads the sheet and all it's children in XML format
     */
    bool loadXML( const KoXmlElement& );

    /**
     * \ingroup NativeFormat
     * Saves a children
     */
    bool saveChildren( KoStore* _store, const QString &_path );

    /**
     * \ingroup NativeFormat
     * Loads a children
     */
    bool loadChildren( KoStore* _store );

    //
    //END Methods related to KSpread's old file format
    //
    //////////////////////////////////////////////////////////////////////////
    //
    //BEGIN Methods related to the OpenDocument file format
    //

    /**
     * \ingroup OpenDocument
     */
    bool loadOdf( const KoXmlElement& sheet,
                    OdfLoadingContext& odfContext,
                    const Styles& autoStyles,
                    const QHash<QString, Conditions>& conditionalStyles );

    /**
     * \ingroup OpenDocument
     */
    bool saveOdf(OdfSavingContext& tableContext);

    /**
     * \ingroup OpenDocument
     */
    void saveOdfHeaderFooter( KoXmlWriter &xmlWriter ) const;

    /**
     * \ingroup OpenDocument
     */
    void loadOdfSettings( const KoOasisSettings::NamedMap &settings );

    /**
     * \ingroup OpenDocument
     */
    void saveOdfSettings( KoXmlWriter &settingsWriter ) const;

    /**
     * \ingroup OpenDocument
     */
    void saveOdfPrintStyleLayout( KoGenStyle &style ) const;

    //
    //END Methods related to the OpenDocument file format
    //
    //////////////////////////////////////////////////////////////////////////
    //
    //BEGIN Methods related to protection
    //

    /**
     * \ingroup Protection
     */
    void password( QByteArray & passwd ) const ;

    /**
     * \ingroup Protection
     */
    bool isProtected() const;

    /**
     * \ingroup Protection
     */
    void setProtected( QByteArray const & passwd );

    /**
     * \ingroup Protection
     */
    bool checkPassword( QByteArray const & passwd ) const;

    //
    //END Methods related to protection
    //
    //////////////////////////////////////////////////////////////////////////
    //
    //BEGIN Methods related to row formats
    //

    /**
     * \return the row format of row \p _row . The default row format,
     * if no special one exists.
     */
    const RowFormat* rowFormat( int _row ) const;

    /**
     * If no special RowFormat exists for this row, then a new one is created.
     *
     * @return a non default RowFormat for this row.
     */
    RowFormat* nonDefaultRowFormat( int _row, bool force_creation = true );

    /**
     * \return the first non-default row format
     */
    RowFormat* firstRow() const;

    void setDefaultHeight( double height );

    //
    //END Methods related to row formats
    //
    //////////////////////////////////////////////////////////////////////////
    //
    //BEGIN Methods related to column formats
    //

    /**
     * \return the column format of column \p _column . The default column format,
     * if no special one exists.
     */
    const ColumnFormat* columnFormat( int _column ) const;

    /**
     * If no special ColumnFormat exists for this column, then a new one is created.
     *
     * @return a non default ColumnFormat for this column.
     */
    ColumnFormat* nonDefaultColumnFormat( int _column, bool force_creation = true );

    /**
     * \return the first non-default row format
     */
    ColumnFormat* firstCol() const;

    void setDefaultWidth( double width );

    //
    //END Methods related to column formats
    //
    //////////////////////////////////////////////////////////////////////////
    //
    //BEGIN Methods for Storage access
    //

    /**
     * \return the cell storage
     */
    CellStorage* cellStorage() const;

    const CommentStorage* commentStorage() const;
    const ConditionsStorage* conditionsStorage() const;
    const FormulaStorage* formulaStorage() const;
    const FusionStorage* fusionStorage() const;
    const LinkStorage* linkStorage() const;
    StyleStorage* styleStorage() const;
    const ValidityStorage* validityStorage() const;
    const ValueStorage* valueStorage() const;

    /**
     * Determines the used area, i.e. the area spanning from A1 to the maximum
     * occupied column and row.
     * \return the used area
     */
    QRect usedArea() const;

    //
    //END Methods for Storage access
    //
    //////////////////////////////////////////////////////////////////////////
    //
    //BEGIN UNSORTED METHODS !!!
    //

    /**
     * Determines the row for a given position \p _ypos . If the position is
     * on the border between two cells, the upper row is returned. Also, the offset
     * between the coordinate system root and the upper row border is determined.
     *
     * \param _ypos the position for which the row should be determined
     * \param _top the offset between the coordinate system root and the upper row border
     *
     * \return the row for the given position \p _ypos
     */
    int topRow( double _ypos, double &_top ) const;

    /**
     * Determines the row for a given position \p _ypos . If the position is
     * on the border between two cells, the lower row is returned.
     *
     * \param _ypos the position for which the row should be determined
     *
     * \return the row for the given position \p _ypos
     */
    int bottomRow( double _ypos ) const;

    /**
     * Determines the column for a given position \p _xpos . If the position is
     * on the border between two cells, the left column is returned. Also, the offset
     * between the coordinate system root and the left column border is determined.
     *
     * \param _xpos the position for which the column should be determined
     * \param _left the offset between the coordinate system root and the left column border
     *
     * \return the column for the given position \p _xpos
     */
    int leftColumn( double _xpos, double &_left ) const;

    /**
     * Determines the column for a given position \p _xpos . If the position is
     * on the border between two cells, the right column is returned.
     *
     * \param _xpos the position for which the column should be determined
     *
     * \return the column for the given position \p _xpos
     */
    int rightColumn( double _xpos ) const;

    /**
     * Calculates the region in document coordinates occupied by a range of cells.
     * \param cellRange the range of cells
     * \return the document area covered by the cells
     */
    QRectF cellCoordinatesToDocument(const QRect& cellRange) const;

    /**
     * Calculates the cell range covering a document area.
     * \param area the document area
     * \return the cell range covering the area
     */
    QRect documentToCellCoordinates(const QRectF& area) const;

    /**
     * @return the left corner of the column as double.
     * Use this method, when you later calculate other positions depending on this one
     * to avoid rounding problems
     * @param col the column's index
     */
    double columnPosition( int col ) const;

    /**
     * @return the top corner of the row as double.
     * Use this method, when you later calculate other positions depending on this one
     * to avoid rounding problems
     * @param _row the row's index
     */
    double rowPosition( int _row ) const;

    /**
     * \return the document size
     */
    QSizeF documentSize() const;

    /**
     * Adjusts the internal reference of the sum of the widths of all columns.
     * Used in resizing of columns.
     */
    void adjustDocumentWidth( double deltaWidth );

    /**
     * Adjusts the internal reference of the sum of the heights of all rows.
     * Used in resizing of rows.
     */
    void adjustDocumentHeight( double deltaHeight );

    /**
     * Attempts to guess the title (or 'header') of a column, within a given area of the sheet
     * This is used, for example, by the Data Sort dialog, to guess the names of columns
     * within the selected area.  An empty string may be returned if guessColumnTitle does not think
     * that column @p col has a title.
     * @param area The area within the sheet to guess from
     * @param col The column to find the title (or 'header') for.
     */
    QString guessColumnTitle(QRect& area, int col);

    /**
     * Attempts to guess the title (or 'header') of a row, within a given area of the sheet
     * This is used, for example, by the Data Sort dialog, to guess the names of rows within the selected area.
     * An empty string may be returned if guessRowTitle does not think that row @p row has a title.
     * @param area The area within the sheet to guess from
     * @param row The row to find the title (or 'header') for.
     */
    QString guessRowTitle(QRect& area, int row);

    //
    //END UNSORTED METHODS
    //
    //////////////////////////////////////////////////////////////////////////
    //
    //BEGIN Methods related to manipulations of selected cells
    //

    /**
     * @param selection the selection of cells to work on
     */
    void copySelection( Selection* selection );
    /**
     * @param selection the selection of cells to work on
     */
    void cutSelection( Selection* selection );

    /**
     * @return @c true if there are text value in cell
     * so you can create list selection
     * @param selection the selection of cells to work on
     */
    bool testListChoose(Selection* selection);

    /**
     * returns the text to be copied to the clipboard
     * @param selection the selection of cells to work on
     */
    QString copyAsText(Selection* selection);

    bool areaIsEmpty(const Region& area, TestType _type = Text) ;

    /**
     * Deletes all cells in the given rectangle.
     * The display is NOT updated by this function.
     * This function can be used to clear an area before you paste something from the clipboard
     * in this area.
     *
     * @param region The region that contains the cells that should be deleted
     */
    void deleteCells(const Region& region);

    //
    //END Methods related to manipulations of selected cells
    //
    //////////////////////////////////////////////////////////////////////////
    //
    //BEGIN Methods related to cut & paste
    //

    /**
     * A convenience function which retrieves the data to be pasted
     * from the clipboard.
     */
    void paste( const QRect & pasteArea, bool makeUndo = true,
                Paste::Mode = Paste::Normal, Paste::Operation = Paste::OverWrite,
                bool insert = false, int insertTo = 0, bool pasteFC = false,
                QClipboard::Mode clipboardMode = QClipboard::Clipboard );

    void paste( const QByteArray & data, const QRect & pasteArea,
                bool makeUndo = false, Paste::Mode= Paste::Normal, Paste::Operation = Paste::OverWrite,
                bool insert = false, int insertTo = 0, bool pasteFC = false );

    /**
     * A function which allows to paste a text plain from the clipboard
     */
    void pasteTextPlain(const QString& _text, const QRect& pasteArea);

    //
    //END Methods related to cut & paste
    //
    //////////////////////////////////////////////////////////////////////////
    //
    //BEGIN Methods related to column/row operations
    //

    /**
     * Helper method.
     * \see ShiftManipulator
     */
    void insertShiftRight( const QRect& rect );

    /**
     * Helper method.
     * \see ShiftManipulator
     */
    void insertShiftDown( const QRect& rect );

    /**
     * Helper method.
     * \see ShiftManipulator
     */
    void removeShiftUp( const QRect& rect );

    /**
     * Helper method.
     * \see ShiftManipulator
     */
    void removeShiftLeft( const QRect& rect );

    /**
     * Helper method.
     * \see InsertDeleteColumnManipulator
     * Moves all columns which are >= \p col \p number positions to the right
     * and inserts a new and empty column.
     */
    void insertColumns( int row, int numbers );

    /**
     * Helper method.
     * \see InsertDeleteRowManipulator
     * Moves all rows which are >= \p row \p number positions down
     * and inserts a new and empty row.
     */
    void insertRows( int row, int numbers );

    /**
     * Helper method.
     * \see InsertDeleteColumnManipulator
     * Deletes \p number columns beginning at \p col .
     */
    void removeColumns( int row, int numbers );

    /**
     * Helper method.
     * \see InsertDeleteRowManipulator
     * Deletes \p number rows beginning at \p row .
     */
    void removeRows( int row, int numbers );

    /**
     * Updates vertical border and view.
     */
    void emitHideRow();

    /**
     * Updates horizontal border and view.
     */
    void emitHideColumn();

    //
    //END Methods related column/row operations
    //
    //////////////////////////////////////////////////////////////////////////
    //
    //BEGIN UNSORTED METHODS !!!
    //

    void hideSheet(bool _hide);

    /**
     * Change name of reference when the user inserts or removes a column,
     * a row or a cell (= insertion of a row [or column] on a single column [or row]).
     * For example the formula =Sheet1!A1 is changed into =Sheet1!B1 if a Column
     * is inserted before A.
     *
     * @param pos the point of insertion (only one coordinate may be used, depending
     * on the other parameters).
     * @param fullRowOrColumn if true, a whole row or column has been inserted/removed.
     *                        if false, we inserted or removed a cell
     * @param ref see ChangeRef
     * @param sheetName completes the pos specification by giving the sheet name
     * @param number number of columns which were inserted
     */
    void changeNameCellRef(const QPoint& pos, bool fullRowOrColumn, ChangeRef ref,
                           const QString& sheetName, int number);

    /**
     * Insert the non-default column format \p columnFormat.
     */
    void insertColumnFormat( ColumnFormat* columnFormat );

    /**
     * Inserts the non-default row format \p rowFormat.
     */
    void insertRowFormat( RowFormat* rowFormat );

    /**
     * Deletes the column format at \p column.
     */
    void deleteColumnFormat( int column );

    /**
     * Deletes the row format at \p row.
     */
    void deleteRowFormat( int row );

    /**
     * @param era set this to true if you want to encode relative references
     *            absolutely (they will be switched back to relative
     *            references during decoding) - used for cut to clipboard
     */
    QDomDocument saveCellRegion(const Region&, bool era = false);

    /**
     * insertTo defined if you insert to the bottom or right
     * insert to bottom if insertTo==1
     * insert to right if insertTo ==-1
     * insertTo used just for insert/paste an area
     * @see paste
     */
    bool loadSelection( const KoXmlDocument& doc, const QRect &pasteArea,
                        int _xshift, int _yshift, bool makeUndo,
                        Paste::Mode = Paste::Normal, Paste::Operation = Paste::OverWrite,
                        bool insert = false, int insertTo = 0, bool paste = false );

    void loadSelectionUndo( const KoXmlDocument & doc, const QRect &loadArea,
                            int _xshift, int _yshift,bool insert,int insertTo);

    /**
     * Used when you insert and paste cell
     * return true if it's a area
     * false if it's a column/row
     * it's used to select if you want to insert at the bottom or right
     * @see paste
     */
    bool testAreaPasteInsert()const;

    //
    //END UNSORTED METHODS
    //
    //////////////////////////////////////////////////////////////////////////
    //
    //BEGIN Methods related to painting
    //

    /**
     * \ingroup Painting
     * set a region of the spreadsheet to be 'paint dirty' meaning it
     * needs repainted.  This is not a flag on the cell itself since quite
     * often this needs set on a default cell
     */
    void setRegionPaintDirty(const Region & region);

    void setRegionPaintDirty(const QRect & rect);

    /**
     * \ingroup Painting
     * repaints all visible cells
     */
    void updateView();

    /**
     * \ingroup Painting
     * repaints all visible cells in \p region
     */
    void updateView(const Region& region);

    //
    //END Methods related to painting
    //
    //////////////////////////////////////////////////////////////////////////
    //
    //BEGIN UNSORTED METHODS !!!
    //

    void emit_updateRow( RowFormat* rowFormat, int _row, bool repaint = true );
    void emit_updateColumn( ColumnFormat* columnFormat, int _column );

    void updateLocale();


  SheetPrint * print() const;
    PrintSettings* printSettings() const;
    void setPrintSettings(const PrintSettings& settings);

#ifndef NDEBUG
    void printDebug();
#endif

    //
    //END UNSORTED METHODS
    //
    //////////////////////////////////////////////////////////////////////////

signals:
    void sig_refreshView();
    void sig_updateView( Sheet *_sheet );
    void sig_updateView( Sheet *_sheet, const Region& );
    void sig_updateColumnHeader( Sheet *_sheet );
    void sig_updateRowHeader( Sheet *_sheet );
    /**
     * @see setSheetName
     */
    void sig_nameChanged( Sheet* sheet, const QString& old_name );

    void sig_SheetHidden( Sheet* sheet);
    void sig_SheetShown( Sheet* sheet);
    void sig_SheetRemoved( Sheet* sheet);
    void sig_SheetActivated( Sheet* );
    void sig_RefreshView( Sheet* );
    void documentSizeChanged( const QSizeF& );
    void visibleSizeChanged();

protected:
    /**
     * Change the name of a sheet in all formulas.
     * When you change name sheet Sheet1 -> Price
     * for all cell which refere to Sheet1, this function changes the name.
     */
    void changeCellTabName( QString const & old_name,QString const & new_name );

    //
    //////////////////////////////////////////////////////////////////////////
    //
    //BEGIN Methods related to the OpenDocument file format
    //

    /**
     * \ingroup OpenDocument
     */
    bool loadRowFormat( const KoXmlElement& row, int &rowIndex,
                        OdfLoadingContext& odfContext,
                        QHash<QString, QRegion>& rowStyleRegions,
                        QHash<QString, QRegion>& cellStyleRegions );

    /**
     * \ingroup OpenDocument
     * Loads the properties of a column from a table:table-column element in an OASIS XML file
     * defaultColumnCellStyles is a map from column indicies to the default cell style for that column
     */
    bool loadColumnFormat(const KoXmlElement& row,
                          const KoOdfStylesReader& stylesReader, int & indexCol,
                          QHash<QString, QRegion>& columnStyleRegions );

    /**
     * \ingroup OpenDocument
     * Inserts the styles contained in \p styleRegions into the style storage.
     * Looks automatic styles up in the map of preloaded automatic styles,
     * \p autoStyles , and custom styles in the StyleManager.
     * The region is restricted to \p usedArea .
     */
    void loadOdfInsertStyles( const Styles& autoStyles,
                                const QHash<QString, QRegion>& styleRegions,
                                const QHash<QString, Conditions>& conditionalStyles,
                                const QRect& usedArea );

    /**
     * \ingroup OpenDocument
     */
    bool loadSheetStyleFormat( KoXmlElement *style );

    /**
     * \ingroup OpenDocument
     */
    void loadOdfMasterLayoutPage( KoStyleStack &styleStack );

    /**
     * \ingroup OpenDocument
     */
    QString saveOdfSheetStyleName( KoGenStyles &mainStyles );

    /**
     * \ingroup OpenDocument
     */
    void saveOdfColRowCell( KoXmlWriter& xmlWriter, KoGenStyles &mainStyles,
                              int maxCols, int maxRows, OdfSavingContext& tableContext);

    /**
     * \ingroup OpenDocument
     */
    void saveOdfCells(KoXmlWriter& xmlWriter, KoGenStyles &mainStyles, int row, int maxCols,
                        OdfSavingContext& tableContext);

    /**
     * \ingroup OpenDocument
     */
    void convertPart( const QString & part, KoXmlWriter & writer ) const;

    /**
     * \ingroup OpenDocument
     */
    void addText( const QString & text, KoXmlWriter & writer ) const;

    /**
     * \ingroup OpenDocument
     */
    bool compareRows(int row1, int row2, int & maxCols, OdfSavingContext& tableContext) const;

    /**
     * \ingroup OpenDocument
     */
    QString getPart( const KoXmlNode & part );

    /**
     * \ingroup OpenDocument
     */
    void replaceMacro( QString & text, const QString & old, const QString & newS );

    //
    //END Methods related to the OpenDocument file format
    //
    //////////////////////////////////////////////////////////////////////////
    //

    // helper function for areaIsEmpty
    bool cellIsEmpty( const Cell& cell, TestType _type );

    QString changeNameCellRefHelper(const QPoint& pos, bool fullRowOrColumn, ChangeRef ref,
                                    int NbCol, const QPoint& point, bool isColumnFixed,
                                    bool isRowFixed);

    static Sheet* find( int _id );
    static int s_id;
    static QHash<int,Sheet*>* s_mapSheets;

private:
    /**
     * \ingroup NativeFormat
     */
    void convertObscuringBorders();

    /**
     * \ingroup NativeFormat
     */
    void checkContentDirection( QString const & name );

    // disable assignment operator
    void operator=(const Sheet& other);

    class Private;
    Private * const d;
};

} // namespace KSpread

#endif  // KSPREAD_SHEET
