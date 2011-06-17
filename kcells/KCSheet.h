/* This file is part of the KDE project
   Copyright 2010 Marijn Kruisselbrink <m.kruisselbrink@student.tue.nl>
   Copyright 2007 Stefan Nikolaus <stefan.nikolaus@kdemail.net>
   Copyright 1998,1999 Torben Weis <weis@kde.org>
   Copyright 1999-2007 The KCells Team <koffice-devel@kde.org>

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
#include <QRect>
//#include <QWidget>

#include <KoDocument.h>
#include <KOdfSettings.h> // for KOdfSettings::NamedMap
#include <KShapeControllerBase.h>
#include <KShapeUserData.h>
#include <KXmlReader.h>

#include "KCCell.h"
#include "KCStyle.h"
#include "Global.h"
#include "KCProtectableObject.h"

#include "kcells_export.h"

class QAbstractItemModel;
class QDomElement;
class QUndoCommand;
class QWidget;

class KDataCenterBase;
class KoDocumentEntry;
class KOdfStyleStack;
class KOdfGenericStyles;
class KOdfSettings;
class KOdfStylesReader;
class KShape;
class KShapeSavingContext;
class KXmlWriter;

class KCCell;
class KCCellStorage;
class KCColumnFormat;
class CommentStorage;
class KCConditionsStorage;
class KCFormulaStorage;
class KCDocBase;
class FusionStorage;
class LinkStorage;
class KCHeaderFooter;
class KCMap;
class KCOdfLoadingContext;
class KCOdfSavingContext;
class KCPrintSettings;
class KCRegion;
class KCRowFormat;
class KCSheet;
class KCSheetPrint;
class KCStyle;
class KCStyleStorage;
class KCValidity;
class KCValidityStorage;
class KCValueStorage;
class KCView;
template<typename T> class IntervalMap;

/**
 * A sheet contains several cells.
 */
class KCELLS_EXPORT KCSheet : public KShapeUserData, public KShapeControllerBase,
        public KCProtectableObject
{
    Q_OBJECT
    Q_PROPERTY(QString sheetName READ sheetName)
    Q_PROPERTY(bool autoCalc READ isAutoCalculationEnabled WRITE setAutoCalculationEnabled)
    Q_PROPERTY(bool showGrid READ getShowGrid WRITE setShowGrid)

public:
    enum ChangeRef       { ColumnInsert, ColumnRemove, RowInsert, RowRemove };
    enum TestType        { Text, KCValidity, Comment, ConditionalCellAttribute };

    /**
     * Creates a sheet in \p map with the name \p sheetName.
     */
    KCSheet(KCMap* map, const QString& sheetName);

    /**
     * Copy constructor.
     * Creates a sheet with the contents and the settings of \p other.
     */
    KCSheet(const KCSheet& other);

    /**
     * Destructor.
     */
    ~KCSheet();

    /**
     * \return a model for this sheet
     */
    QAbstractItemModel *model() const;

    /**
     * \return the map this sheet belongs to
     */
    KCMap* map() const;

    /**
     * \return the document this sheet belongs to
     */
    KCDocBase* doc() const;

    // KShapeControllerBase interface
    virtual void addShape(KShape* shape);
    virtual void removeShape(KShape* shape);
    virtual KResourceManager* resourceManager() const;

    /**
     * Deletes all shapes without emitting shapeRemoved()
     */
    void deleteShapes();

    /**
     * \ingroup Embedding
     * Returns the sheet's shapes.
     * \return the shapes this sheet contains
     */
    QList<KShape*> shapes() const;

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
     * @see KCTabBar::renameTab
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
    void setLayoutDirection(Qt::LayoutDirection dir);

    /**
     * Returns, if the grid shall be shown on the screen
     */
    bool getShowGrid() const;

    /**
     * Sets, if the grid shall be shown on the screen
     */
    void setShowGrid(bool _showGrid);

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
    void setShowCommentIndicator(bool b);

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
    void setHidden(bool hidden);

    /**
     * @return a flag that indicates whether the sheet should paint the page breaks.
     *
     * @see setShowPageBorders
     * @see KCSheet::Private::showPageBorders
     */
    bool isShowPageBorders() const;

    /**
     * Turns the page break lines on or off.
     *
     * @see isShowPageBorders
     * @see KCSheet::Private::showPageBorders
     */
    void setShowPageBorders(bool _b);

    struct BackgroundImageProperties {
        BackgroundImageProperties()
        : repeat(Repeat)
        , opacity(1.0)
        , horizontalPosition(HorizontalCenter)
        , verticalPosition(VerticalCenter)
        {}

        enum Repetition {
            NoRepeat,
            Repeat,
            Stretch
        };
        Repetition repeat;

        float opacity;

        enum HorizontalPosition {
            Left,
            HorizontalCenter,
            Right
        };
        HorizontalPosition horizontalPosition;

        enum VerticalPosition {
            Top,
            VerticalCenter,
            Bottom
        };
        VerticalPosition verticalPosition;

        //TODO filterName
    };

    /**
     * Set background image for this sheet
     */
    void setBackgroundImage( const QImage& image );

    /**
     * @return The QImage used as the background picture for this sheet
     */
    QImage backgroundImage() const;

    void setBackgroundImageProperties( const BackgroundImageProperties& properties );

    BackgroundImageProperties backgroundImageProperties() const;

    //
    //END Methods related to sheet properties
    //
    //////////////////////////////////////////////////////////////////////////
    //
    //BEGIN Methods related to KCells's old file format
    //

    /**
     * \ingroup NativeFormat
     * Saves the sheet and all it's children in XML format
     */
    QDomElement saveXML(QDomDocument&);

    /**
     * \ingroup NativeFormat
     * Loads the sheet and all it's children in XML format
     */
    bool loadXML(const KXmlElement&);

    /**
     * \ingroup NativeFormat
     * Saves a children
     */
    bool saveChildren(KOdfStore* _store, const QString &_path);

    /**
     * \ingroup NativeFormat
     * Loads a children
     */
    bool loadChildren(KOdfStore* _store);

    //
    //END Methods related to KCells's old file format
    //
    //////////////////////////////////////////////////////////////////////////
    //
    //BEGIN Methods related to the OpenDocument file format
    //

    /**
     * \ingroup OpenDocument
     */
    bool loadOdf(const KXmlElement& sheet,
                 KCOdfLoadingContext& odfContext,
                 const Styles& autoStyles,
                 const QHash<QString, KCConditions>& conditionalStyles);

    /**
     * \ingroup OpenDocument
     */
    bool saveOdf(KCOdfSavingContext& tableContext);

    /**
     * \ingroup OpenDocument
     */
    void saveOdfHeaderFooter(KXmlWriter &xmlWriter) const;

    /**
     * \ingroup OpenDocument
     */
    void saveOdfBackgroundImage(KXmlWriter& xmlWriter) const;

    /**
     * \ingroup OpenDocument
     */
    void loadOdfSettings(const KOdfSettings::NamedMap &settings);

    /**
     * \ingroup OpenDocument
     */
    void saveOdfSettings(KXmlWriter &settingsWriter) const;

    void loadOdfObject(const KXmlElement& element, KShapeLoadingContext& shapeContext);
    //
    //END Methods related to the OpenDocument file format
    //
    //////////////////////////////////////////////////////////////////////////
    //
    //BEGIN Methods related to row formats
    //

    /**
     * \ingroup ColumnRowFormat
     * \return the row format of row \p _row . The default row format,
     * if no special one exists.
     */
    const KCRowFormat* rowFormat(int _row) const;

    /**
     * \ingroup ColumnRowFormat
     * If no special KCRowFormat exists for this row, then a new one is created.
     *
     * @return a non default KCRowFormat for this row.
     */
    KCRowFormat* nonDefaultRowFormat(int _row, bool force_creation = true);

    /**
     * \ingroup ColumnRowFormat
     * \return the first non-default row format
     */
    KCRowFormat* firstRow() const;

    /**
     * \ingroup ColumnRowFormat
     */
    void setDefaultHeight(double height);

    //
    //END Methods related to row formats
    //
    //////////////////////////////////////////////////////////////////////////
    //
    //BEGIN Methods related to column formats
    //

    /**
     * \ingroup ColumnRowFormat
     * \return the column format of column \p _column . The default column format,
     * if no special one exists.
     */
    const KCColumnFormat* columnFormat(int _column) const;

    /**
     * \ingroup ColumnRowFormat
     * If no special KCColumnFormat exists for this column, then a new one is created.
     *
     * @return a non default KCColumnFormat for this column.
     */
    KCColumnFormat* nonDefaultColumnFormat(int _column, bool force_creation = true);

    /**
     * \ingroup ColumnRowFormat
     * \return the first non-default row format
     */
    KCColumnFormat* firstCol() const;

    /**
     * \ingroup ColumnRowFormat
     */
    void setDefaultWidth(double width);

    //
    //END Methods related to column formats
    //
    //////////////////////////////////////////////////////////////////////////
    //
    //BEGIN Methods for Storage access
    //

    /**
     * \ingroup Storage
     * \return the cell storage
     */
    KCCellStorage* cellStorage() const;

    const CommentStorage* commentStorage() const;
    const KCConditionsStorage* conditionsStorage() const;
    const KCFormulaStorage* formulaStorage() const;
    const FusionStorage* fusionStorage() const;
    const LinkStorage* linkStorage() const;
    const KCStyleStorage* styleStorage() const;
    const KCValidityStorage* validityStorage() const;
    const KCValueStorage* valueStorage() const;

    /**
     * \ingroup Coordinates
     * \ingroup Storage
     * Determines the used area, i.e. the area spanning from A1 to the maximum
     * occupied column and row.
     * \return the used area
     */
    QRect usedArea(bool onlyContent = false) const;

    //
    //END Methods for Storage access
    //
    //////////////////////////////////////////////////////////////////////////
    //
    //BEGIN UNSORTED METHODS !!!
    //

    /**
     * \ingroup Coordinates
     * Determines the row for a given position \p _ypos . If the position is
     * on the border between two cells, the upper row is returned. Also, the offset
     * between the coordinate system root and the upper row border is determined.
     *
     * \param _ypos the position for which the row should be determined
     * \param _top the offset between the coordinate system root and the upper row border
     *
     * \return the row for the given position \p _ypos
     */
    int topRow(double _ypos, double &_top) const;

    /**
     * \ingroup Coordinates
     * Determines the row for a given position \p _ypos . If the position is
     * on the border between two cells, the lower row is returned.
     *
     * \param _ypos the position for which the row should be determined
     *
     * \return the row for the given position \p _ypos
     */
    int bottomRow(double _ypos) const;

    /**
     * \ingroup Coordinates
     * Determines the column for a given position \p _xpos . If the position is
     * on the border between two cells, the left column is returned. Also, the offset
     * between the coordinate system root and the left column border is determined.
     *
     * \param _xpos the position for which the column should be determined
     * \param _left the offset between the coordinate system root and the left column border
     *
     * \return the column for the given position \p _xpos
     */
    int leftColumn(double _xpos, double &_left) const;

    /**
     * \ingroup Coordinates
     * Determines the column for a given position \p _xpos . If the position is
     * on the border between two cells, the right column is returned.
     *
     * \param _xpos the position for which the column should be determined
     *
     * \return the column for the given position \p _xpos
     */
    int rightColumn(double _xpos) const;

    /**
     * \ingroup Coordinates
     * Calculates the region in document coordinates occupied by a range of cells.
     * \param cellRange the range of cells
     * \return the document area covered by the cells
     */
    QRectF cellCoordinatesToDocument(const QRect& cellRange) const;

    /**
     * \ingroup Coordinates
     * Calculates the cell range covering a document area.
     * \param area the document area
     * \return the cell range covering the area
     */
    QRect documentToCellCoordinates(const QRectF& area) const;

    /**
     * \ingroup Coordinates
     * @return the left corner of the column as double.
     * Use this method, when you later calculate other positions depending on this one
     * to avoid rounding problems
     * @param col the column's index
     */
    double columnPosition(int col) const;

    /**
     * \ingroup Coordinates
     * @return the top corner of the row as double.
     * Use this method, when you later calculate other positions depending on this one
     * to avoid rounding problems
     * @param _row the row's index
     */
    double rowPosition(int _row) const;

    /**
     * \ingroup Coordinates
     * \return the document size
     */
    QSizeF documentSize() const;

    /**
     * \ingroup Coordinates
     * Adjusts the internal reference of the sum of the widths of all columns.
     * Used in resizing of columns.
     */
    void adjustDocumentWidth(double deltaWidth);

    /**
     * \ingroup Coordinates
     * Adjusts the internal reference of the sum of the heights of all rows.
     * Used in resizing of rows.
     */
    void adjustDocumentHeight(double deltaHeight);

    //
    //END UNSORTED METHODS
    //
    //////////////////////////////////////////////////////////////////////////
    //
    //BEGIN Methods related to manipulations of selected cells
    //

    /**
     * \ingroup Commands
     */
    bool areaIsEmpty(const KCRegion& area, TestType _type = Text) ;

    //
    //END Methods related to manipulations of selected cells
    //
    //////////////////////////////////////////////////////////////////////////
    //
    //BEGIN Methods related to column/row operations
    //

    /**
     * \ingroup Commands
     * Helper method.
     * \see ShiftManipulator
     */
    void insertShiftRight(const QRect& rect);

    /**
     * \ingroup Commands
     * Helper method.
     * \see ShiftManipulator
     */
    void insertShiftDown(const QRect& rect);

    /**
     * \ingroup Commands
     * Helper method.
     * \see ShiftManipulator
     */
    void removeShiftUp(const QRect& rect);

    /**
     * \ingroup Commands
     * Helper method.
     * \see ShiftManipulator
     */
    void removeShiftLeft(const QRect& rect);

    /**
     * \ingroup ColumnRowFormat
     * Helper method.
     * \see InsertDeleteColumnManipulator
     * Moves all columns which are >= \p col \p number positions to the right
     * and inserts a new and empty column.
     */
    void insertColumns(int row, int numbers);

    /**
     * Helper method.
     * \see InsertDeleteRowManipulator
     * Moves all rows which are >= \p row \p number positions down
     * and inserts a new and empty row.
     */
    void insertRows(int row, int numbers);

    /**
     * \ingroup ColumnRowFormat
     * Helper method.
     * \see InsertDeleteColumnManipulator
     * Deletes \p number columns beginning at \p col .
     */
    void removeColumns(int row, int numbers);

    /**
     * \ingroup ColumnRowFormat
     * Helper method.
     * \see InsertDeleteRowManipulator
     * Deletes \p number rows beginning at \p row .
     */
    void removeRows(int row, int number);

    //
    //END Methods related column/row operations
    //
    //////////////////////////////////////////////////////////////////////////
    //
    //BEGIN UNSORTED METHODS !!!
    //

    void hideSheet(bool _hide);

    /**
     * \ingroup KCValue
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
     * \ingroup ColumnRowFormat
     * Insert the non-default column format \p columnFormat.
     */
    void insertColumnFormat(KCColumnFormat* columnFormat);

    /**
     * \ingroup ColumnRowFormat
     * Inserts the non-default row format \p rowFormat.
     */
    void insertRowFormat(KCRowFormat* rowFormat);

    /**
     * \ingroup ColumnRowFormat
     * Deletes the column format at \p column.
     */
    void deleteColumnFormat(int column);

    /**
     * \ingroup ColumnRowFormat
     * Deletes the row format at \p row.
     */
    void deleteRowFormat(int row);

    //
    //END UNSORTED METHODS
    //
    //////////////////////////////////////////////////////////////////////////
    //
    //BEGIN UNSORTED METHODS !!!
    //

    /**
     * Shows a status \p message in the status bar for \p timeout msecs.
     */
    void showStatusMessage(const QString &message, int timeout = 3000);

    void updateLocale();


    /**
     * \ingroup Page
     * The page layout manager.
     */
    KCSheetPrint *print() const;

    /**
     * \ingroup Page
     * Print settings.
     */
    KCPrintSettings* printSettings() const;

    /**
     * \ingroup Page
     * Sets the print settings.
     */
    void setPrintSettings(const KCPrintSettings& settings);

    /**
     * \ingroup Page
     * \return the header & footer object
     */
    KCHeaderFooter *headerFooter() const;

#ifndef NDEBUG
    void printDebug();
#endif

    //
    //END UNSORTED METHODS
    //
    //////////////////////////////////////////////////////////////////////////

signals:
    /**
     * Emitted, if the document size changed.
     * E.g. if some columns were inserted.
     * \param size new size
     */
    void documentSizeChanged(const QSizeF &size);

    /**
     * Emitted, if the visible size changed.
     * E.g. if the document size changed or the user selected an area,
     * which was not visible before.
     */
    void visibleSizeChanged();

    /**
     * Emitted, if a status \p message should be shown in the status bar
     * for \p timeout msecs.
     */
    void statusMessage(const QString& message, int timeout);

    /**
     * \ingroup Embedding
     * Emitted, if a \p shape was added.
     * \param sheet this sheet (for the KCView to determine, if it's the active one)
     */
    void shapeAdded(KCSheet *sheet, KShape *shape);

    /**
     * \ingroup Embedding
     * Emitted, if a \p shape was removed.
     * \param sheet this sheet (for the KCView to determine, if it's the active one)
     */
    void shapeRemoved(KCSheet *sheet, KShape *shape);

protected:
    /**
     * \ingroup KCValue
     * Change the name of a sheet in all formulas.
     * When you change name sheet Sheet1 -> Price
     * for all cell which refere to Sheet1, this function changes the name.
     */
    void changeCellTabName(QString const & old_name, QString const & new_name);

    //
    //////////////////////////////////////////////////////////////////////////
    //
    //BEGIN Methods related to the OpenDocument file format
    //

    void loadColumnNodes(const KXmlElement& parent, int& indexCol,
                            int& maxColumn, KOdfLoadingContext& odfContext,
                            QHash<QString, QRegion>& columnStyleRegions,
                            IntervalMap<QString>& columnStyles);
    void loadRowNodes(const KXmlElement& parent, int& rowIndex,
                            int& maxColumn, KCOdfLoadingContext& tableContext,
                            QHash<QString, QRegion>& rowStyleRegions,
                            QHash<QString, QRegion>& cellStyleRegions,
                            const IntervalMap<QString>& columnStyles,
                            const Styles& autoStyles);

    /**
     * \ingroup OpenDocument
     */
    int loadRowFormat(const KXmlElement& row, int &rowIndex,
                       KCOdfLoadingContext& odfContext,
                       QHash<QString, QRegion>& rowStyleRegions,
                       QHash<QString, QRegion>& cellStyleRegions,
                       const IntervalMap<QString>& columnStyles,
                       const Styles& autoStyles);

    /**
     * \ingroup OpenDocument
     * Loads the properties of a column from a table:table-column element in an OASIS XML file
     * defaultColumnCellStyles is a map from column indicies to the default cell style for that column
     */
    bool loadColumnFormat(const KXmlElement& row,
                          const KOdfStylesReader& stylesReader, int & indexCol,
                          QHash<QString, QRegion>& columnStyleRegions,
                          IntervalMap<QString>& columnStyles);

    /**
     * \ingroup OpenDocument
     * Inserts the styles contained in \p styleRegions into the style storage.
     * Looks automatic styles up in the map of preloaded automatic styles,
     * \p autoStyles , and custom styles in the KCStyleManager.
     * The region is restricted to \p usedArea .
     */
    void loadOdfInsertStyles(const Styles& autoStyles,
                             const QHash<QString, QRegion>& styleRegions,
                             const QHash<QString, KCConditions>& conditionalStyles,
                             const QRect& usedArea,
                             QList<QPair<QRegion, KCStyle> >& outStyleRegions,
                             QList<QPair<QRegion, KCConditions> >& outConditionalStyles);

    /**
     * \ingroup OpenDocument
     */
    bool loadSheetStyleFormat(KXmlElement *style);

    /**
     * \ingroup OpenDocument
     */
    void loadOdfMasterLayoutPage(KOdfStyleStack &styleStack);

    /**
     * \ingroup OpenDocument
     */
    QString saveOdfSheetStyleName(KOdfGenericStyles &mainStyles);

    /**
     * \ingroup OpenDocument
     */
    void saveOdfColRowCell(KXmlWriter& xmlWriter, KOdfGenericStyles &mainStyles,
                           int maxCols, int maxRows, KCOdfSavingContext& tableContext);

    /**
     * \ingroup OpenDocument
     */
    void saveOdfCells(KXmlWriter& xmlWriter, KOdfGenericStyles &mainStyles, int row, int maxCols,
                      KCOdfSavingContext& tableContext);

    /**
     * \ingroup OpenDocument
     */
    void convertPart(const QString & part, KXmlWriter & writer) const;

    /**
     * \ingroup OpenDocument
     */
    void addText(const QString & text, KXmlWriter & writer) const;

    /**
     * \ingroup OpenDocument
     */
    bool compareRows(int row1, int row2, int & maxCols, KCOdfSavingContext& tableContext) const;

    /**
     * \ingroup OpenDocument
     */
    QString getPart(const KXmlNode & part);

    /**
     * \ingroup OpenDocument
     */
    void replaceMacro(QString & text, const QString & old, const QString & newS);

    //
    //END Methods related to the OpenDocument file format
    //
    //////////////////////////////////////////////////////////////////////////
    //

    /**
     * \ingroup Commands
     * \see areaIsEmpty()
     */
    bool cellIsEmpty(const KCCell& cell, TestType _type);

    /**
     * \ingroup KCValue
     * \see changeNameCellRef()
     */
    QString changeNameCellRefHelper(const QPoint& pos, bool fullRowOrColumn, ChangeRef ref,
                                    int NbCol, const QPoint& point, bool isColumnFixed,
                                    bool isRowFixed);

private:
    /**
     * \ingroup NativeFormat
     */
    void convertObscuringBorders();

    /**
     * \ingroup NativeFormat
     */
    void checkContentDirection(QString const & name);

    // disable assignment operator
    void operator=(const KCSheet& other);

    class Private;
    Private * const d;
};

#endif  // KSPREAD_SHEET
