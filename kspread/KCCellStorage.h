/* This file is part of the KDE project
   Copyright 2007 Stefan Nikolaus <stefan.nikolaus@kdemail.net>

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

#ifndef KSPREAD_CELL_STORAGE
#define KSPREAD_CELL_STORAGE

#include <QPair>
#include <QRect>
#include <QTextDocument>

#include "KCCell.h"
#include "kspread_limits.h"
#include "KCPointStorage.h"

#include "database/Database.h"

class KoXmlWriter;

class QUndoCommand;

class KCBinding;
class KCBindingStorage;
class KCCell;
class CommentStorage;
class Conditions;
class KCConditionsStorage;
class KCFormula;
class KCFormulaStorage;
class FusionStorage;
class LinkStorage;
class KCRegion;
class RichTextStorage;
class KCSheet;
class KCStyleStorage;
class KCValidity;
class ValidityStorage;
class KCValue;
class ValueStorage;

/**
 * \ingroup Storage
 * The cell storage.
 * A wrapper around a couple of storages, which hold the cell data.
 * Provides methods to iterate over the non-empty cells.
 * Emits Damages on changes.
 * Capable of recording the old data for undoing.
 *
 * \author Stefan Nikolaus <stefan.nikolaus@kdemail.net>
 *
 * \note If you fill the storage, do it row-wise. That's more performant.
 */
class KSPREAD_EXPORT KCCellStorage : public QObject
{
    Q_OBJECT
public:
    enum Visiting {
        Values          = 0x01,
        Formulas        = 0x02,
        Comments        = 0x04,
        Links           = 0x08,
        Styles          = 0x10,
        ConditionStyles = 0x20,
        Validities      = 0x40,
        VisitContent    = 0x03, ///< just visit the cell contents: values, formulas
        VisitAll        = 0xFF  ///< visit all: cell contents, styles, comments, ...
    };

    /**
     * Constructor.
     * Creates an empty storage for \p sheet.
     */
    KCCellStorage(KCSheet* sheet);

    /**
     * Copy constructor.
     * \note Take care: does not perform a deep copy!
     */
    KCCellStorage(const KCCellStorage& other);

    /**
     * Copy constructor.
     * Creates a KCCellStorage for \p sheet and copies the data from \p other.
     */
    KCCellStorage(const KCCellStorage& other, KCSheet* sheet);

    /**
     * Destructor.
     */
    ~KCCellStorage();

    /**
     * \return the sheet this KCCellStorage is for.
     */
    KCSheet* sheet() const;

    /**
     * Removes all data at \p col , \p row .
     */
    void take(int col, int row);

    /**
     * \return the binding associated with the KCCell at \p column , \p row .
     */
    KCBinding binding(int column, int row) const;
    void setBinding(const KCRegion& region, const KCBinding& binding);
    void removeBinding(const KCRegion& region, const KCBinding& binding);

    /**
     * \return the comment associated with the KCCell at \p column , \p row .
     */
    QString comment(int column, int row) const;
    void setComment(const KCRegion& region, const QString& comment);

    /**
     * \return the conditional formattings associated with the KCCell at \p column , \p row .
     */
    Conditions conditions(int column, int row) const;
    void setConditions(const KCRegion& region, Conditions conditions);

    /**
     * \return the database associated with the KCCell at \p column , \p row .
     */
    Database database(int column, int row) const;
    QList< QPair<QRectF, Database> > databases(const KCRegion& region) const;
    void setDatabase(const KCRegion& region, const Database& database);

    /**
     * \return the formula associated with the KCCell at \p column , \p row .
     */
    KCFormula formula(int column, int row) const;
    void setFormula(int column, int row, const KCFormula& formula);

    /**
     * \return the hyperlink associated with the KCCell at \p column , \p row .
     */
    QString link(int column, int row) const;
    void setLink(int column, int row, const QString& link);

    /**
     * \return the named area's name associated with the KCCell at \p column , \p row .
     */
    QString namedArea(int column, int row) const;
    QList< QPair<QRectF, QString> > namedAreas(const KCRegion& region) const;
    void setNamedArea(const KCRegion& region, const QString& namedArea);
    void emitInsertNamedArea(const KCRegion &region, const QString &namedArea);

    /**
     * \return the KCStyle associated with the KCCell at \p column , \p row .
     */
    KCStyle style(int column, int row) const;

    /**
     * \return the KCStyle associated with \p rect.
     */
    KCStyle style(const QRect& rect) const;
    void setStyle(const KCRegion& region, const KCStyle& style);
    void insertSubStyle(const QRect& rect, const SharedSubStyle& subStyle);

    /**
     * \return the user input associated with the KCCell at \p column , \p row .
     */
    QString userInput(int column, int row) const;
    void setUserInput(int column, int row, const QString& input);

    /**
     * \return the validity checks associated with the KCCell at \p column , \p row .
     */
    KCValidity validity(int column, int row) const;
    void setValidity(const KCRegion& region, KCValidity validity);

    /**
     * \return the value associated with the KCCell at \p column , \p row .
     */
    KCValue value(int column, int row) const;

    /**
     * Creates a value array containing the values in \p region.
     */
    KCValue valueRegion(const KCRegion& region) const;
    void setValue(int column, int row, const KCValue& value);

    QSharedPointer<QTextDocument> richText(int column, int row) const;
    void setRichText(int column, int row, QSharedPointer<QTextDocument> text);

    /**
     */
    bool doesMergeCells(int column, int row) const;
    bool isPartOfMerged(int column, int row) const;

    /**
     * Merge the cell at \p column, \p row with the \p numXCells adjacent cells in horizontal
     * direction and with the \p numYCells adjacent cells in vertical direction. I.e. the
     * resulting cell spans \p numXCells + 1 columns and \p numYCells + 1 rows. Passing \c 0
     * as \p numXCells and \p numYCells unmerges the cell at \p column, \p row.
     *
     * \param column the master cell's column
     * \param row the master cell's row
     * \param numXCells number of horizontal cells to be merged in
     * \param numYCells number of vertical cells to be merged in
     *
     */
    void mergeCells(int column, int row, int numXCells, int numYCells);
    KCCell masterCell(int column, int row) const;
    int mergedXCells(int column, int row) const;
    int mergedYCells(int column, int row) const;
    QList<KCCell> masterCells(const KCRegion& region) const;

    /**
     * \return \c true, if the cell's value is a matrix and obscures other cells
     */
    bool locksCells(int column, int row) const;
    bool isLocked(int column, int row) const;
    bool hasLockedCells(const KCRegion& region) const;
    void lockCells(const QRect& rect);
    void unlockCells(int column, int row);
    QRect lockedCells(int column, int row) const;

    /**
     * Insert \p number columns at \p position .
     * \return the data, that became out of range (shifted over the end)
     */
    void insertColumns(int position, int number = 1);

    /**
     * Removes \p number columns at \p position .
     * \return the removed data
     */
    void removeColumns(int position, int number = 1);

    /**
     * Insert \p number rows at \p position .
     * \return the data, that became out of range (shifted over the end)
     */
    void insertRows(int position, int number = 1);

    /**
     * Removes \p number rows at \p position .
     * \return the removed data
     */
    void removeRows(int position, int number = 1);

    /**
     * Shifts the data right of \p rect to the left by the width of \p rect .
     * The data formerly contained in \p rect becomes overridden.
     */
    void removeShiftLeft(const QRect& rect);

    /**
     * Shifts the data in and right of \p rect to the right by the width of \p rect .
     */
    void insertShiftRight(const QRect& rect);

    /**
     * Shifts the data below \p rect to the top by the height of \p rect .
     * The data formerly contained in \p rect becomes overridden.
     */
    void removeShiftUp(const QRect& rect);

    /**
     * Shifts the data in and below \p rect to the bottom by the height of \p rect .
     */
    void insertShiftDown(const QRect& rect);

    /**
     * Retrieve the first used data in \p col .
     * Can be used in conjunction with nextInColumn() to loop through a column.
     * \return the first used data in \p col or the default data, if the column is empty.
     */
    KCCell firstInColumn(int col, Visiting visiting = VisitAll) const;

    /**
     * Retrieve the first used data in \p row .
     * Can be used in conjunction with nextInRow() to loop through a row.
     * \return the first used data in \p row or the default data, if the row is empty.
     */
    KCCell firstInRow(int row, Visiting visiting = VisitAll) const;

    /**
     * Retrieve the last used data in \p col .
     * Can be used in conjunction with prevInColumn() to loop through a column.
     * \return the last used data in \p col or the default data, if the column is empty.
     */
    KCCell lastInColumn(int col, Visiting visiting = VisitAll) const;

    /**
     * Retrieve the last used data in \p row .
     * Can be used in conjunction with prevInRow() to loop through a row.
     * \return the last used data in \p row or the default data, if the row is empty.
     */
    KCCell lastInRow(int row, Visiting visiting = VisitAll) const;

    /**
     * Retrieve the next used data in \p col after \p row .
     * Can be used in conjunction with firstInColumn() to loop through a column.
     * \return the next used data in \p col or the default data, there is no further data.
     */
    KCCell nextInColumn(int col, int row, Visiting visiting = VisitAll) const;

    /**
     * Retrieve the next used data in \p row after \p col .
     * Can be used in conjunction with firstInRow() to loop through a row.
     * \return the next used data in \p row or the default data, if there is no further data.
     */
    KCCell nextInRow(int col, int row, Visiting visiting = VisitAll) const;

    /**
     * Retrieve the previous used data in \p col after \p row .
     * Can be used in conjunction with lastInColumn() to loop through a column.
     * \return the previous used data in \p col or the default data, there is no further data.
     */
    KCCell prevInColumn(int col, int row, Visiting visiting = VisitAll) const;

    /**
     * Retrieve the previous used data in \p row after \p col .
     * Can be used in conjunction with lastInRow() to loop through a row.
     * \return the previous used data in \p row or the default data, if there is no further data.
     */
    KCCell prevInRow(int col, int row, Visiting visiting = VisitAll) const;

    /**
     * The maximum occupied column, i.e. the horizontal storage dimension.
     * \return the maximum column
     */
    int columns(bool includeStyles = true) const;

    /**
     * The maximum occupied row, i.e. the vertical storage dimension.
     * \return the maximum row
     */
    int rows(bool includeStyles = true) const;

    /**
     * The number of rows that are consecutive to, and identical to \p row. This includes the row
     * itself.
     */
    int rowRepeat(int row) const;

    /**
     * The first row in the block of consecutive identical rows \p row is in.
     */
    int firstIdenticalRow(int row) const;

    /**
     * Set how often the specified row is repeated. \p row is the index of the first row in a block,
     * \p count is the number of times it is repeated (including the first one). This method is used
     * during loading.
     */
    void setRowsRepeated(int row, int count);

    /**
     * Creates a substorage consisting of the values in \p region.
     * \return a subset of the storage stripped down to the values in \p region
     */
    KCCellStorage subStorage(const KCRegion& region) const;

    const KCBindingStorage* bindingStorage() const;
    const CommentStorage* commentStorage() const;
    const KCConditionsStorage* conditionsStorage() const;
    const KCFormulaStorage* formulaStorage() const;
    const FusionStorage* fusionStorage() const;
    const LinkStorage* linkStorage() const;
    const KCStyleStorage* styleStorage() const;
    const ValidityStorage* validityStorage() const;
    const ValueStorage* valueStorage() const;

    void loadConditions(const QList<QPair<QRegion, Conditions> >& conditions);
    void loadStyles(const QList<QPair<QRegion, KCStyle> >& styles);

    void invalidateStyleCache();

    /**
     * Starts the undo recording.
     * While recording the undo data of each storage operation is saved in
     * an undo command, that can be retrieved when the recording is stopped.
     * \see stopUndoRecording
     */
    void startUndoRecording();

    /**
     * Stops the undo recording.
     * An undo command has to be passed as \p parent command and
     * for each sub-storage an undo-capable command is attached to \p parent.
     * \see startUndoRecording
     */
    void stopUndoRecording(QUndoCommand *parent);

Q_SIGNALS:
    void insertNamedArea(const KCRegion&, const QString&);
    void namedAreaRemoved(const QString&);

private:
    // do not allow assignment
    KCCellStorage& operator=(const KCCellStorage&);

    class Private;
    Private * const d;
};

class UserInputStorage : public KCPointStorage<QString>
{
public:
    UserInputStorage& operator=(const KCPointStorage<QString>& o) {
        KCPointStorage<QString>::operator=(o);
        return *this;
    }
};

class LinkStorage : public KCPointStorage<QString>
{
public:
    LinkStorage& operator=(const KCPointStorage<QString>& o) {
        KCPointStorage<QString>::operator=(o);
        return *this;
    }
};

class RichTextStorage : public KCPointStorage<QSharedPointer<QTextDocument> >
{
public:
    RichTextStorage& operator=(const KCPointStorage<QSharedPointer<QTextDocument> >& o) {
        KCPointStorage<QSharedPointer<QTextDocument> >::operator=(o);
        return *this;
    }
};

#endif // KSPREAD_CELL_STORAGE
