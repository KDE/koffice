/* This file is part of the KDE project
   Copyright 2010 Marijn Kruisselbrink <m.kruisselbrink@student.tue.nl>
   Copyright 2007, 2009 Stefan Nikolaus <stefan.nikolaus@kdemail.net>

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
#include "KCCellStorage.h"
#include "CellStorage_p.h"

// KDE
#include <klocale.h>

// KOffice
#include <KoXmlWriter.h>

// KCells
#include "KCBindingStorage.h"
#include "KCConditionsStorage.h"
#include "Damages.h"
#include "KCDependencyManager.h"
#include "KCFormulaStorage.h"
#include "KCMap.h"
#include "ModelSupport.h"
#include "KCRecalcManager.h"
#include "KCRectStorage.h"
#include "KCRowRepeatStorage.h"
#include "KCSheet.h"
#include "KCStyleStorage.h"
#include "KCValidityStorage.h"
#include "KCValueStorage.h"

// commands
#include "commands/KCPointStorageUndoCommand.h"
#include "commands/KCRectStorageUndoCommand.h"
#include "commands/StyleStorageCommand.h"

// database
#include "database/DatabaseStorage.h"
#include "database/DatabaseManager.h"

Q_DECLARE_METATYPE(QSharedPointer<QTextDocument>)

typedef KCRectStorage<QString> NamedAreaStorage;

class KCCellStorage::Private
{
public:
    Private(KCSheet* sheet)
            : sheet(sheet)
            , bindingStorage(new KCBindingStorage(sheet->map()))
            , commentStorage(new CommentStorage(sheet->map()))
            , conditionsStorage(new KCConditionsStorage(sheet->map()))
            , databaseStorage(new DatabaseStorage(sheet->map()))
            , formulaStorage(new KCFormulaStorage())
            , fusionStorage(new FusionStorage(sheet->map()))
            , linkStorage(new LinkStorage())
            , matrixStorage(new MatrixStorage(sheet->map()))
            , namedAreaStorage(new NamedAreaStorage(sheet->map()))
            , styleStorage(new KCStyleStorage(sheet->map()))
            , userInputStorage(new UserInputStorage())
            , validityStorage(new KCValidityStorage(sheet->map()))
            , valueStorage(new KCValueStorage())
            , richTextStorage(new RichTextStorage())
            , rowRepeatStorage(new KCRowRepeatStorage())
            , undoData(0) {}

    Private(const Private& other, KCSheet* sheet)
            : sheet(sheet)
            , bindingStorage(new KCBindingStorage(*other.bindingStorage))
            , commentStorage(new CommentStorage(*other.commentStorage))
            , conditionsStorage(new KCConditionsStorage(*other.conditionsStorage))
            , databaseStorage(new DatabaseStorage(*other.databaseStorage))
            , formulaStorage(new KCFormulaStorage(*other.formulaStorage))
            , fusionStorage(new FusionStorage(*other.fusionStorage))
            , linkStorage(new LinkStorage(*other.linkStorage))
            , matrixStorage(new MatrixStorage(*other.matrixStorage))
            , namedAreaStorage(new NamedAreaStorage(*other.namedAreaStorage))
            , styleStorage(new KCStyleStorage(*other.styleStorage))
            , userInputStorage(new UserInputStorage(*other.userInputStorage))
            , validityStorage(new KCValidityStorage(*other.validityStorage))
            , valueStorage(new KCValueStorage(*other.valueStorage))
            , richTextStorage(new RichTextStorage(*other.richTextStorage))
            , rowRepeatStorage(new KCRowRepeatStorage(*other.rowRepeatStorage))
            , undoData(0) {}

    ~Private() {
        delete bindingStorage;
        delete commentStorage;
        delete conditionsStorage;
        delete databaseStorage;
        delete formulaStorage;
        delete fusionStorage;
        delete linkStorage;
        delete matrixStorage;
        delete namedAreaStorage;
        delete styleStorage;
        delete userInputStorage;
        delete validityStorage;
        delete valueStorage;
        delete richTextStorage;
        delete rowRepeatStorage;
    }

    void createCommand(QUndoCommand *parent) const;

    KCSheet*                  sheet;
    KCBindingStorage*         bindingStorage;
    CommentStorage*         commentStorage;
    KCConditionsStorage*      conditionsStorage;
    DatabaseStorage*        databaseStorage;
    KCFormulaStorage*         formulaStorage;
    FusionStorage*          fusionStorage;
    LinkStorage*            linkStorage;
    MatrixStorage*          matrixStorage;
    NamedAreaStorage*       namedAreaStorage;
    KCStyleStorage*           styleStorage;
    UserInputStorage*       userInputStorage;
    KCValidityStorage*        validityStorage;
    KCValueStorage*           valueStorage;
    RichTextStorage*        richTextStorage;
    KCRowRepeatStorage*       rowRepeatStorage;
    KCCellStorageUndoData*    undoData;
};

void KCCellStorage::Private::createCommand(QUndoCommand *parent) const
{
    if (!undoData->bindings.isEmpty()) {
        KCRectStorageUndoCommand<KCBinding> *const command
        = new KCRectStorageUndoCommand<KCBinding>(sheet->model(), SourceRangeRole, parent);
        command->add(undoData->bindings);
    }
    if (!undoData->comments.isEmpty()) {
        KCRectStorageUndoCommand<QString> *const command
        = new KCRectStorageUndoCommand<QString>(sheet->model(), CommentRole, parent);
        command->add(undoData->comments);
    }
    if (!undoData->conditions.isEmpty()) {
        KCRectStorageUndoCommand<KCConditions> *const command
        = new KCRectStorageUndoCommand<KCConditions>(sheet->model(), ConditionRole, parent);
        command->add(undoData->conditions);
    }
    if (!undoData->databases.isEmpty()) {
        KCRectStorageUndoCommand<Database> *const command
        = new KCRectStorageUndoCommand<Database>(sheet->model(), TargetRangeRole, parent);
        command->add(undoData->databases);
    }
    if (!undoData->formulas.isEmpty()) {
        KCPointStorageUndoCommand<KCFormula> *const command
        = new KCPointStorageUndoCommand<KCFormula>(sheet->model(), FormulaRole, parent);
        command->add(undoData->formulas);
    }
    if (!undoData->fusions.isEmpty()) {
        KCRectStorageUndoCommand<bool> *const command
        = new KCRectStorageUndoCommand<bool>(sheet->model(), FusionedRangeRole, parent);
        command->add(undoData->fusions);
    }
    if (!undoData->links.isEmpty()) {
        KCPointStorageUndoCommand<QString> *const command
        = new KCPointStorageUndoCommand<QString>(sheet->model(), LinkRole, parent);
        command->add(undoData->links);
    }
    if (!undoData->matrices.isEmpty()) {
        KCRectStorageUndoCommand<bool> *const command
        = new KCRectStorageUndoCommand<bool>(sheet->model(), LockedRangeRole, parent);
        command->add(undoData->matrices);
    }
    if (!undoData->namedAreas.isEmpty()) {
        KCRectStorageUndoCommand<QString> *const command
        = new KCRectStorageUndoCommand<QString>(sheet->model(), NamedAreaRole, parent);
        command->add(undoData->namedAreas);
    }
    if (!undoData->richTexts.isEmpty()) {
        KCPointStorageUndoCommand<QSharedPointer<QTextDocument> > *const command
        = new KCPointStorageUndoCommand<QSharedPointer<QTextDocument> >(sheet->model(), RichTextRole, parent);
        command->add(undoData->richTexts);
    }
    if (!undoData->styles.isEmpty()) {
        StyleStorageCommand *const command
        = new StyleStorageCommand(styleStorage, parent);
        command->add(undoData->styles);
    }
    if (!undoData->userInputs.isEmpty()) {
        KCPointStorageUndoCommand<QString> *const command
        = new KCPointStorageUndoCommand<QString>(sheet->model(), UserInputRole, parent);
        command->add(undoData->userInputs);
    }
    if (!undoData->validities.isEmpty()) {
        KCRectStorageUndoCommand<KCValidity> *const command
        = new KCRectStorageUndoCommand<KCValidity>(sheet->model(), ValidityRole, parent);
        command->add(undoData->validities);
    }
    if (!undoData->values.isEmpty()) {
        KCPointStorageUndoCommand<KCValue> *const command
        = new KCPointStorageUndoCommand<KCValue>(sheet->model(), ValueRole, parent);
        command->add(undoData->values);
    }
}


KCCellStorage::KCCellStorage(KCSheet* sheet)
        : QObject(sheet)
        , d(new Private(sheet))
{
}

KCCellStorage::KCCellStorage(const KCCellStorage& other)
        : QObject(other.d->sheet)
        , d(new Private(*other.d))
{
}

KCCellStorage::KCCellStorage(const KCCellStorage& other, KCSheet* sheet)
        : QObject(sheet)
        , d(new Private(*other.d, sheet))
{
}

KCCellStorage::~KCCellStorage()
{
    delete d;
}

KCSheet* KCCellStorage::sheet() const
{
    return d->sheet;
}

void KCCellStorage::take(int col, int row)
{
    KCFormula oldFormula;
    QString oldLink;
    QString oldUserInput;
    KCValue oldValue;
    QSharedPointer<QTextDocument> oldRichText;

    oldFormula = d->formulaStorage->take(col, row);
    oldLink = d->linkStorage->take(col, row);
    oldUserInput = d->userInputStorage->take(col, row);
    oldValue = d->valueStorage->take(col, row);
    oldRichText = d->richTextStorage->take(col, row);

    if (!d->sheet->map()->isLoading()) {
        // Trigger a recalculation of the consuming cells.
        KCCellDamage::Changes changes = KCCellDamage:: KCBinding | KCCellDamage::KCFormula | KCCellDamage::KCValue;
        d->sheet->map()->addDamage(new KCCellDamage(KCCell(d->sheet, col, row), changes));

        d->rowRepeatStorage->setRowRepeat(row, 1);
    }
    // also trigger a relayout of the first non-empty cell to the left of this cell
    int prevCol;
    KCValue v = d->valueStorage->prevInRow(col, row, &prevCol);
    if (!v.isEmpty())
        d->sheet->map()->addDamage(new KCCellDamage(KCCell(d->sheet, prevCol, row), KCCellDamage::Appearance));


    // recording undo?
    if (d->undoData) {
        d->undoData->formulas   << qMakePair(QPoint(col, row), oldFormula);
        d->undoData->links      << qMakePair(QPoint(col, row), oldLink);
        d->undoData->userInputs << qMakePair(QPoint(col, row), oldUserInput);
        d->undoData->values     << qMakePair(QPoint(col, row), oldValue);
        d->undoData->richTexts  << qMakePair(QPoint(col, row), oldRichText);
    }
}

KCBinding KCCellStorage::binding(int column, int row) const
{
    return d->bindingStorage->contains(QPoint(column, row));
}

void KCCellStorage::setBinding(const KCRegion& region, const KCBinding& binding)
{
    // recording undo?
    if (d->undoData)
        d->undoData->bindings << d->bindingStorage->undoData(region);

    d->bindingStorage->insert(region, binding);
}

void KCCellStorage::removeBinding(const KCRegion& region, const KCBinding& binding)
{
    // recording undo?
    if (d->undoData) {
        d->undoData->bindings << d->bindingStorage->undoData(region);
    }
    d->bindingStorage->remove(region, binding);
}

QString KCCellStorage::comment(int column, int row) const
{
    return d->commentStorage->contains(QPoint(column, row));
}

void KCCellStorage::setComment(const KCRegion& region, const QString& comment)
{
    // recording undo?
    if (d->undoData)
        d->undoData->comments << d->commentStorage->undoData(region);

    d->commentStorage->insert(region, comment);
    if (!d->sheet->map()->isLoading()) {
        foreach (const QRect& r, region.rects()) {
            d->rowRepeatStorage->splitRowRepeat(r.top());
            d->rowRepeatStorage->splitRowRepeat(r.bottom()+1);
        }
    }
}

KCConditions KCCellStorage::conditions(int column, int row) const
{
    return d->conditionsStorage->contains(QPoint(column, row));
}

void KCCellStorage::setConditions(const KCRegion& region, KCConditions conditions)
{
    // recording undo?
    if (d->undoData)
        d->undoData->conditions << d->conditionsStorage->undoData(region);

    d->conditionsStorage->insert(region, conditions);
    if (!d->sheet->map()->isLoading()) {
        foreach (const QRect& r, region.rects()) {
            d->rowRepeatStorage->splitRowRepeat(r.top());
            d->rowRepeatStorage->splitRowRepeat(r.bottom()+1);
        }
    }
}

Database KCCellStorage::database(int column, int row) const
{
    QPair<QRectF, Database> pair = d->databaseStorage->containedPair(QPoint(column, row));
    if (pair.first.isEmpty())
        return Database();
    if (pair.second.isEmpty())
        return Database();
    // update the range, which might get changed
    Database database = pair.second;
    database.setRange(KCRegion(pair.first.toRect(), d->sheet));
    return database;
}

QList< QPair<QRectF, Database> > KCCellStorage::databases(const KCRegion& region) const
{
    return d->databaseStorage->intersectingPairs(region);
}

void KCCellStorage::setDatabase(const KCRegion& region, const Database& database)
{
    // recording undo?
    if (d->undoData)
        d->undoData->databases << d->databaseStorage->undoData(region);

    d->databaseStorage->insert(region, database);
}

KCFormula KCCellStorage::formula(int column, int row) const
{
    return d->formulaStorage->lookup(column, row, KCFormula::empty());
}

void KCCellStorage::setFormula(int column, int row, const KCFormula& formula)
{
    KCFormula old = KCFormula::empty();
    if (formula.expression().isEmpty())
        old = d->formulaStorage->take(column, row, KCFormula::empty());
    else
        old = d->formulaStorage->insert(column, row, formula);

    // formula changed?
    if (formula != old) {
        if (!d->sheet->map()->isLoading()) {
            // trigger an update of the dependencies and a recalculation
            d->sheet->map()->addDamage(new KCCellDamage(KCCell(d->sheet, column, row), KCCellDamage::KCFormula | KCCellDamage::KCValue));
            d->rowRepeatStorage->setRowRepeat(row, 1);
        }
        // recording undo?
        if (d->undoData) {
            d->undoData->formulas << qMakePair(QPoint(column, row), old);
            // Also store the old value, if there wasn't a formula before,
            // because the new value is calculated later by the damage
            // processing and is not recorded for undoing.
            if (old == KCFormula())
                d->undoData->values << qMakePair(QPoint(column, row), value(column, row));
        }
    }
}

QString KCCellStorage::link(int column, int row) const
{
    return d->linkStorage->lookup(column, row);
}

void KCCellStorage::setLink(int column, int row, const QString& link)
{
    QString old;
    if (link.isEmpty())
        old = d->linkStorage->take(column, row);
    else
        old = d->linkStorage->insert(column, row, link);

    // recording undo?
    if (d->undoData && link != old)
        d->undoData->links << qMakePair(QPoint(column, row), old);
    if (!d->sheet->map()->isLoading())
        d->rowRepeatStorage->setRowRepeat(row, 1);
}

QString KCCellStorage::namedArea(int column, int row) const
{
    QPair<QRectF, QString> pair = d->namedAreaStorage->containedPair(QPoint(column, row));
    if (pair.first.isEmpty())
        return QString();
    if (pair.second.isEmpty())
        return QString();
    return pair.second;
}

QList< QPair<QRectF, QString> > KCCellStorage::namedAreas(const KCRegion& region) const
{
    return d->namedAreaStorage->intersectingPairs(region);
}

void KCCellStorage::setNamedArea(const KCRegion& region, const QString& namedArea)
{
    // recording undo?
    if (d->undoData)
        d->undoData->namedAreas << d->namedAreaStorage->undoData(region);

    d->namedAreaStorage->insert(region, namedArea);
}

void KCCellStorage::emitInsertNamedArea(const KCRegion &region, const QString &namedArea)
{
    emit insertNamedArea(region, namedArea);
}

KCStyle KCCellStorage::style(int column, int row) const
{
    return d->styleStorage->contains(QPoint(column, row));
}

KCStyle KCCellStorage::style(const QRect& rect) const
{
    return d->styleStorage->contains(rect);
}

void KCCellStorage::setStyle(const KCRegion& region, const KCStyle& style)
{
    // recording undo?
    if (d->undoData)
        d->undoData->styles << d->styleStorage->undoData(region);

    d->styleStorage->insert(region, style);
    if (!d->sheet->map()->isLoading()) {
        foreach (const QRect& r, region.rects()) {
            d->rowRepeatStorage->splitRowRepeat(r.top());
            d->rowRepeatStorage->splitRowRepeat(r.bottom()+1);
        }
    }
}

void KCCellStorage::insertSubStyle(const QRect &rect, const KCSharedSubStyle &subStyle)
{
    d->styleStorage->insert(rect, subStyle);
    if (!d->sheet->map()->isLoading()) {
        d->rowRepeatStorage->splitRowRepeat(rect.top());
        d->rowRepeatStorage->splitRowRepeat(rect.bottom()+1);
    }
}

QString KCCellStorage::userInput(int column, int row) const
{
    return d->userInputStorage->lookup(column, row);
}

void KCCellStorage::setUserInput(int column, int row, const QString& userInput)
{
    QString old;
    if (userInput.isEmpty())
        old = d->userInputStorage->take(column, row);
    else
        old = d->userInputStorage->insert(column, row, userInput);

    // recording undo?
    if (d->undoData && userInput != old)
        d->undoData->userInputs << qMakePair(QPoint(column, row), old);
    if (!d->sheet->map()->isLoading())
        d->rowRepeatStorage->setRowRepeat(row, 1);
}

QSharedPointer<QTextDocument> KCCellStorage::richText(int column, int row) const
{
    return d->richTextStorage->lookup(column, row);
}

void KCCellStorage::setRichText(int column, int row, QSharedPointer<QTextDocument> text)
{
    QSharedPointer<QTextDocument> old;
    if (text.isNull())
        old = d->richTextStorage->take(column, row);
    else
        old = d->richTextStorage->insert(column, row, text);

    // recording undo?
    if (d->undoData && text != old)
        d->undoData->richTexts << qMakePair(QPoint(column, row), old);
}

KCValidity KCCellStorage::validity(int column, int row) const
{
    return d->validityStorage->contains(QPoint(column, row));
}

void KCCellStorage::setValidity(const KCRegion& region, KCValidity validity)
{
    // recording undo?
    if (d->undoData)
        d->undoData->validities << d->validityStorage->undoData(region);

    d->validityStorage->insert(region, validity);
    if (!d->sheet->map()->isLoading()) {
        foreach (const QRect& r, region.rects()) {
            d->rowRepeatStorage->splitRowRepeat(r.top());
            d->rowRepeatStorage->splitRowRepeat(r.bottom()+1);
        }
    }
}

KCValue KCCellStorage::value(int column, int row) const
{
    return d->valueStorage->lookup(column, row);
}

KCValue KCCellStorage::valueRegion(const KCRegion& region) const
{
    // create a subStorage with adjusted origin
    return KCValue(d->valueStorage->subStorage(region, false), region.boundingRect().size());
}

void KCCellStorage::setValue(int column, int row, const KCValue& value)
{
    // release any lock
    unlockCells(column, row);

    KCValue old;
    if (value.isEmpty())
        old = d->valueStorage->take(column, row);
    else
        old = d->valueStorage->insert(column, row, value);

    // value changed?
    if (value != old) {
        if (!d->sheet->map()->isLoading()) {
            // Always trigger a repainting and a binding update.
            KCCellDamage::Changes changes = KCCellDamage::Appearance | KCCellDamage::KCBinding;
            // Trigger a recalculation of the consuming cells, only if we are not
            // already in a recalculation process.
            if (!d->sheet->map()->recalcManager()->isActive())
                changes |= KCCellDamage::KCValue;
            d->sheet->map()->addDamage(new KCCellDamage(KCCell(d->sheet, column, row), changes));
            // Also trigger a relayouting of the first non-empty cell to the left of this one
            int prevCol;
            KCValue v = d->valueStorage->prevInRow(column, row, &prevCol);
            if (!v.isEmpty())
                d->sheet->map()->addDamage(new KCCellDamage(KCCell(d->sheet, prevCol, row), KCCellDamage::Appearance));
            d->rowRepeatStorage->setRowRepeat(row, 1);
        }
        // recording undo?
        if (d->undoData)
            d->undoData->values << qMakePair(QPoint(column, row), old);
    }
}

bool KCCellStorage::doesMergeCells(int column, int row) const
{
    const QPair<QRectF, bool> pair = d->fusionStorage->containedPair(QPoint(column, row));
    if (pair.first.isNull())
        return false;
    if (pair.second == false)
        return false;
    // master cell?
    if (pair.first.toRect().topLeft() != QPoint(column, row))
        return false;
    return true;
}

bool KCCellStorage::isPartOfMerged(int column, int row) const
{
    const QPair<QRectF, bool> pair = d->fusionStorage->containedPair(QPoint(column, row));
    if (pair.first.isNull())
        return false;
    if (pair.second == false)
        return false;
    // master cell?
    if (pair.first.toRect().topLeft() == QPoint(column, row))
        return false;
    return true;
}

void KCCellStorage::mergeCells(int column, int row, int numXCells, int numYCells)
{
    // Start by unmerging the cells that we merge right now
    const QPair<QRectF, bool> pair = d->fusionStorage->containedPair(QPoint(column, row));
    if (!pair.first.isNull())
        d->fusionStorage->insert(KCRegion(pair.first.toRect()), false);
    // Merge the cells
    if (numXCells != 0 || numYCells != 0)
        d->fusionStorage->insert(KCRegion(column, row, numXCells + 1, numYCells + 1), true);
    if (!d->sheet->map()->isLoading())
        d->rowRepeatStorage->setRowRepeat(row, 1);
}

KCCell KCCellStorage::masterCell(int column, int row) const
{
    const QPair<QRectF, bool> pair = d->fusionStorage->containedPair(QPoint(column, row));
    if (pair.first.isNull())
        return KCCell(d->sheet, column, row);
    if (pair.second == false)
        return KCCell(d->sheet, column, row);
    return KCCell(d->sheet, pair.first.toRect().topLeft());
}

int KCCellStorage::mergedXCells(int column, int row) const
{
    const QPair<QRectF, bool> pair = d->fusionStorage->containedPair(QPoint(column, row));
    if (pair.first.isNull())
        return 0;
    // Not the master cell?
    if (pair.first.topLeft() != QPoint(column, row))
        return 0;
    return pair.first.toRect().width() - 1;
}

int KCCellStorage::mergedYCells(int column, int row) const
{
    const QPair<QRectF, bool> pair = d->fusionStorage->containedPair(QPoint(column, row));
    if (pair.first.isNull())
        return 0;
    // Not the master cell?
    if (pair.first.topLeft() != QPoint(column, row))
        return 0;
    return pair.first.toRect().height() - 1;
}

QList<KCCell> KCCellStorage::masterCells(const KCRegion& region) const
{
    const QList<QPair<QRectF, bool> > pairs = d->fusionStorage->intersectingPairs(region);
    if (pairs.isEmpty())
        return QList<KCCell>();
    QList<KCCell> masterCells;
    for (int i = 0; i < pairs.count(); ++i) {
        if (pairs[i].first.isNull())
            continue;
        if (pairs[i].second == false)
            continue;
        masterCells.append(KCCell(d->sheet, pairs[i].first.toRect().topLeft()));
    }
    return masterCells;
}

bool KCCellStorage::locksCells(int column, int row) const
{
    const QPair<QRectF, bool> pair = d->matrixStorage->containedPair(QPoint(column, row));
    if (pair.first.isNull())
        return false;
    if (pair.second == false)
        return false;
    // master cell?
    if (pair.first.toRect().topLeft() != QPoint(column, row))
        return false;
    return true;
}

bool KCCellStorage::isLocked(int column, int row) const
{
    const QPair<QRectF, bool> pair = d->matrixStorage->containedPair(QPoint(column, row));
    if (pair.first.isNull())
        return false;
    if (pair.second == false)
        return false;
    // master cell?
    if (pair.first.toRect().topLeft() == QPoint(column, row))
        return false;
    return true;
}

bool KCCellStorage::hasLockedCells(const KCRegion& region) const
{
    typedef QPair<QRectF, bool> RectBoolPair;
    QList<QPair<QRectF, bool> > pairs = d->matrixStorage->intersectingPairs(region);
    foreach (const RectBoolPair& pair, pairs) {
        if (pair.first.isNull())
            continue;
        if (pair.second == false)
            continue;
        // more than just the master cell in the region?
        const QPoint topLeft = pair.first.toRect().topLeft();
        if (pair.first.width() >= 1) {
            if (region.contains(topLeft + QPoint(1, 0), d->sheet))
                return true;
        }
        if (pair.first.height() >= 1) {
            if (region.contains(topLeft + QPoint(0, 1), d->sheet))
                return true;
        }
    }
    return false;
}

void KCCellStorage::lockCells(const QRect& rect)
{
    // Start by unlocking the cells that we lock right now
    const QPair<QRectF, bool> pair = d->matrixStorage->containedPair(rect.topLeft());  // FIXME
    if (!pair.first.isNull())
        d->matrixStorage->insert(KCRegion(pair.first.toRect()), false);
    // Lock the cells
    if (rect.width() > 1 || rect.height() > 1)
        d->matrixStorage->insert(KCRegion(rect), true);
}

void KCCellStorage::unlockCells(int column, int row)
{
    const QPair<QRectF, bool> pair = d->matrixStorage->containedPair(QPoint(column, row));
    if (pair.first.isNull())
        return;
    if (pair.second == false)
        return;
    if (pair.first.toRect().topLeft() != QPoint(column, row))
        return;
    const QRect rect = pair.first.toRect();
    d->matrixStorage->insert(KCRegion(rect), false);
    // clear the values
    for (int r = rect.top(); r <= rect.bottom(); ++r) {
        for (int c = rect.left(); c <= rect.right(); ++c) {
            if (r != rect.top() || c != rect.left())
                setValue(c, r, KCValue());
        }
    }
    // recording undo?
    if (d->undoData)
        d->undoData->matrices << pair;
}

QRect KCCellStorage::lockedCells(int column, int row) const
{
    const QPair<QRectF, bool> pair = d->matrixStorage->containedPair(QPoint(column, row));
    if (pair.first.isNull())
        return QRect(column, row, 1, 1);
    if (pair.second == false)
        return QRect(column, row, 1, 1);
    if (pair.first.toRect().topLeft() != QPoint(column, row))
        return QRect(column, row, 1, 1);
    return pair.first.toRect();
}

void KCCellStorage::insertColumns(int position, int number)
{
    // Trigger a dependency update of the cells, which have a formula. (old positions)
    // FIXME Stefan: Would it be better to directly alter the dependency tree?
    // TODO Stefan: Optimize: Avoid the double creation of the sub-storages, but don't process
    //              formulas, that will get out of bounds after the operation.
    const KCRegion invalidRegion(QRect(QPoint(position, 1), QPoint(KS_colMax, KS_rowMax)), d->sheet);
    KCPointStorage<KCFormula> subStorage = d->formulaStorage->subStorage(invalidRegion);
    KCCell cell;
    for (int i = 0; i < subStorage.count(); ++i) {
        cell = KCCell(d->sheet, subStorage.col(i), subStorage.row(i));
        d->sheet->map()->addDamage(new KCCellDamage(cell, KCCellDamage::KCFormula));
    }
    // Trigger an update of the bindings and the named areas.
    d->sheet->map()->addDamage(new KCCellDamage(d->sheet, invalidRegion, KCCellDamage::KCBinding | KCCellDamage::NamedArea));

    QList< QPair<QRectF, KCBinding> > bindings = d->bindingStorage->insertColumns(position, number);
    QList< QPair<QRectF, QString> > comments = d->commentStorage->insertColumns(position, number);
    QList< QPair<QRectF, KCConditions> > conditions = d->conditionsStorage->insertColumns(position, number);
    QList< QPair<QRectF, Database> > databases = d->databaseStorage->insertColumns(position, number);
    QVector< QPair<QPoint, KCFormula> > formulas = d->formulaStorage->insertColumns(position, number);
    QList< QPair<QRectF, bool> > fusions = d->fusionStorage->insertColumns(position, number);
    QVector< QPair<QPoint, QString> > links = d->linkStorage->insertColumns(position, number);
    QList< QPair<QRectF, bool> > matrices = d->matrixStorage->insertColumns(position, number);
    QList< QPair<QRectF, QString> > namedAreas = d->namedAreaStorage->insertColumns(position, number);
    QList< QPair<QRectF, KCSharedSubStyle> > styles = d->styleStorage->insertColumns(position, number);
    QVector< QPair<QPoint, QString> > userInputs = d->userInputStorage->insertColumns(position, number);
    QVector< QPair<QPoint, QSharedPointer<QTextDocument> > > richTexts = d->richTextStorage->insertColumns(position, number);
    QList< QPair<QRectF, KCValidity> > validities = d->validityStorage->insertColumns(position, number);
    QVector< QPair<QPoint, KCValue> > values = d->valueStorage->insertColumns(position, number);
    // recording undo?
    if (d->undoData) {
        d->undoData->bindings   << bindings;
        d->undoData->comments   << comments;
        d->undoData->conditions << conditions;
        d->undoData->databases  << databases;
        d->undoData->formulas   << formulas;
        d->undoData->fusions    << fusions;
        d->undoData->links      << links;
        d->undoData->matrices   << matrices;
        d->undoData->namedAreas << namedAreas;
        d->undoData->styles     << styles;
        d->undoData->userInputs << userInputs;
        d->undoData->validities << validities;
        d->undoData->values     << values;
        d->undoData->richTexts  << richTexts;
    }

    // Trigger a dependency update of the cells, which have a formula. (new positions)
    subStorage = d->formulaStorage->subStorage(invalidRegion);
    for (int i = 0; i < subStorage.count(); ++i) {
        cell = KCCell(d->sheet, subStorage.col(i), subStorage.row(i));
        d->sheet->map()->addDamage(new KCCellDamage(cell, KCCellDamage::KCFormula));
    }
    // Trigger a recalculation only for the cells, that depend on values in the changed region.
    KCRegion providers = d->sheet->map()->dependencyManager()->reduceToProvidingRegion(invalidRegion);
    d->sheet->map()->addDamage(new KCCellDamage(d->sheet, providers, KCCellDamage::KCValue));
}

void KCCellStorage::removeColumns(int position, int number)
{
    // Trigger a dependency update of the cells, which have a formula. (old positions)
    const KCRegion invalidRegion(QRect(QPoint(position, 1), QPoint(KS_colMax, KS_rowMax)), d->sheet);
    KCPointStorage<KCFormula> subStorage = d->formulaStorage->subStorage(invalidRegion);
    KCCell cell;
    for (int i = 0; i < subStorage.count(); ++i) {
        cell = KCCell(d->sheet, subStorage.col(i), subStorage.row(i));
        d->sheet->map()->addDamage(new KCCellDamage(cell, KCCellDamage::KCFormula));
    }
    // Trigger an update of the bindings and the named areas.
    const KCRegion region(QRect(QPoint(position - 1, 1), QPoint(KS_colMax, KS_rowMax)), d->sheet);
    d->sheet->map()->addDamage(new KCCellDamage(d->sheet, region, KCCellDamage::KCBinding | KCCellDamage::NamedArea));

    QList< QPair<QRectF, KCBinding> > bindings = d->bindingStorage->removeColumns(position, number);
    QList< QPair<QRectF, QString> > comments = d->commentStorage->removeColumns(position, number);
    QList< QPair<QRectF, KCConditions> > conditions = d->conditionsStorage->removeColumns(position, number);
    QList< QPair<QRectF, Database> > databases = d->databaseStorage->removeColumns(position, number);
    QVector< QPair<QPoint, KCFormula> > formulas = d->formulaStorage->removeColumns(position, number);
    QList< QPair<QRectF, bool> > fusions = d->fusionStorage->removeColumns(position, number);
    QVector< QPair<QPoint, QString> > links = d->linkStorage->removeColumns(position, number);
    QList< QPair<QRectF, bool> > matrices = d->matrixStorage->removeColumns(position, number);
    QList< QPair<QRectF, QString> > namedAreas = d->namedAreaStorage->removeColumns(position, number);
    QList< QPair<QRectF, KCSharedSubStyle> > styles = d->styleStorage->removeColumns(position, number);
    QVector< QPair<QPoint, QString> > userInputs = d->userInputStorage->removeColumns(position, number);
    QList< QPair<QRectF, KCValidity> > validities = d->validityStorage->removeColumns(position, number);
    QVector< QPair<QPoint, KCValue> > values = d->valueStorage->removeColumns(position, number);
    QVector< QPair<QPoint, QSharedPointer<QTextDocument> > > richTexts = d->richTextStorage->removeColumns(position, number);
    // recording undo?
    if (d->undoData) {
        d->undoData->bindings   << bindings;
        d->undoData->comments   << comments;
        d->undoData->conditions << conditions;
        d->undoData->databases  << databases;
        d->undoData->formulas   << formulas;
        d->undoData->fusions    << fusions;
        d->undoData->links      << links;
        d->undoData->matrices   << matrices;
        d->undoData->namedAreas << namedAreas;
        d->undoData->styles     << styles;
        d->undoData->userInputs << userInputs;
        d->undoData->validities << validities;
        d->undoData->values     << values;
        d->undoData->richTexts  << richTexts;
    }

    // Trigger a dependency update of the cells, which have a formula. (new positions)
    subStorage = d->formulaStorage->subStorage(invalidRegion);
    for (int i = 0; i < subStorage.count(); ++i) {
        cell = KCCell(d->sheet, subStorage.col(i), subStorage.row(i));
        d->sheet->map()->addDamage(new KCCellDamage(cell, KCCellDamage::KCFormula));
    }
    // Trigger a recalculation only for the cells, that depend on values in the changed region.
    KCRegion providers = d->sheet->map()->dependencyManager()->reduceToProvidingRegion(invalidRegion);
    d->sheet->map()->addDamage(new KCCellDamage(d->sheet, providers, KCCellDamage::KCValue));
}

void KCCellStorage::insertRows(int position, int number)
{
    // Trigger a dependency update of the cells, which have a formula. (old positions)
    const KCRegion invalidRegion(QRect(QPoint(1, position), QPoint(KS_colMax, KS_rowMax)), d->sheet);
    KCPointStorage<KCFormula> subStorage = d->formulaStorage->subStorage(invalidRegion);
    KCCell cell;
    for (int i = 0; i < subStorage.count(); ++i) {
        cell = KCCell(d->sheet, subStorage.col(i), subStorage.row(i));
        d->sheet->map()->addDamage(new KCCellDamage(cell, KCCellDamage::KCFormula));
    }
    // Trigger an update of the bindings and the named areas.
    d->sheet->map()->addDamage(new KCCellDamage(d->sheet, invalidRegion, KCCellDamage::KCBinding | KCCellDamage::NamedArea));

    QList< QPair<QRectF, KCBinding> > bindings = d->bindingStorage->insertRows(position, number);
    QList< QPair<QRectF, QString> > comments = d->commentStorage->insertRows(position, number);
    QList< QPair<QRectF, KCConditions> > conditions = d->conditionsStorage->insertRows(position, number);
    QList< QPair<QRectF, Database> > databases = d->databaseStorage->insertRows(position, number);
    QVector< QPair<QPoint, KCFormula> > formulas = d->formulaStorage->insertRows(position, number);
    QList< QPair<QRectF, bool> > fusions = d->fusionStorage->insertRows(position, number);
    QVector< QPair<QPoint, QString> > links = d->linkStorage->insertRows(position, number);
    QList< QPair<QRectF, bool> > matrices = d->matrixStorage->insertRows(position, number);
    QList< QPair<QRectF, QString> > namedAreas = d->namedAreaStorage->insertRows(position, number);
    QList< QPair<QRectF, KCSharedSubStyle> > styles = d->styleStorage->insertRows(position, number);
    QVector< QPair<QPoint, QString> > userInputs = d->userInputStorage->insertRows(position, number);
    QList< QPair<QRectF, KCValidity> > validities = d->validityStorage->insertRows(position, number);
    QVector< QPair<QPoint, KCValue> > values = d->valueStorage->insertRows(position, number);
    QVector< QPair<QPoint, QSharedPointer<QTextDocument> > > richTexts = d->richTextStorage->insertRows(position, number);
    // recording undo?
    if (d->undoData) {
        d->undoData->bindings   << bindings;
        d->undoData->comments   << comments;
        d->undoData->conditions << conditions;
        d->undoData->databases  << databases;
        d->undoData->formulas   << formulas;
        d->undoData->fusions    << fusions;
        d->undoData->links      << links;
        d->undoData->matrices   << matrices;
        d->undoData->namedAreas << namedAreas;
        d->undoData->styles     << styles;
        d->undoData->userInputs << userInputs;
        d->undoData->validities << validities;
        d->undoData->values     << values;
        d->undoData->richTexts  << richTexts;
    }

    // Trigger a dependency update of the cells, which have a formula. (new positions)
    subStorage = d->formulaStorage->subStorage(invalidRegion);
    for (int i = 0; i < subStorage.count(); ++i) {
        cell = KCCell(d->sheet, subStorage.col(i), subStorage.row(i));
        d->sheet->map()->addDamage(new KCCellDamage(cell, KCCellDamage::KCFormula));
    }
    // Trigger a recalculation only for the cells, that depend on values in the changed region.
    KCRegion providers = d->sheet->map()->dependencyManager()->reduceToProvidingRegion(invalidRegion);
    d->sheet->map()->addDamage(new KCCellDamage(d->sheet, providers, KCCellDamage::KCValue));

    d->rowRepeatStorage->insertRows(position, number);
}

void KCCellStorage::removeRows(int position, int number)
{
    // Trigger a dependency update of the cells, which have a formula. (old positions)
    const KCRegion invalidRegion(QRect(QPoint(1, position), QPoint(KS_colMax, KS_rowMax)), d->sheet);
    KCPointStorage<KCFormula> subStorage = d->formulaStorage->subStorage(invalidRegion);
    KCCell cell;
    for (int i = 0; i < subStorage.count(); ++i) {
        cell = KCCell(d->sheet, subStorage.col(i), subStorage.row(i));
        d->sheet->map()->addDamage(new KCCellDamage(cell, KCCellDamage::KCFormula));
    }
    // Trigger an update of the bindings and the named areas.
    const KCRegion region(QRect(QPoint(1, position - 1), QPoint(KS_colMax, KS_rowMax)), d->sheet);
    d->sheet->map()->addDamage(new KCCellDamage(d->sheet, region, KCCellDamage::KCBinding | KCCellDamage::NamedArea));

    QList< QPair<QRectF, KCBinding> > bindings = d->bindingStorage->removeRows(position, number);
    QList< QPair<QRectF, QString> > comments = d->commentStorage->removeRows(position, number);
    QList< QPair<QRectF, KCConditions> > conditions = d->conditionsStorage->removeRows(position, number);
    QList< QPair<QRectF, Database> > databases = d->databaseStorage->removeRows(position, number);
    QVector< QPair<QPoint, KCFormula> > formulas = d->formulaStorage->removeRows(position, number);
    QList< QPair<QRectF, bool> > fusions = d->fusionStorage->removeRows(position, number);
    QVector< QPair<QPoint, QString> > links = d->linkStorage->removeRows(position, number);
    QList< QPair<QRectF, bool> > matrices = d->matrixStorage->removeRows(position, number);
    QList< QPair<QRectF, QString> > namedAreas = d->namedAreaStorage->removeRows(position, number);
    QList< QPair<QRectF, KCSharedSubStyle> > styles = d->styleStorage->removeRows(position, number);
    QVector< QPair<QPoint, QString> > userInputs = d->userInputStorage->removeRows(position, number);
    QList< QPair<QRectF, KCValidity> > validities = d->validityStorage->removeRows(position, number);
    QVector< QPair<QPoint, KCValue> > values = d->valueStorage->removeRows(position, number);
    QVector< QPair<QPoint, QSharedPointer<QTextDocument> > > richTexts = d->richTextStorage->removeRows(position, number);
    // recording undo?
    if (d->undoData) {
        d->undoData->bindings   << bindings;
        d->undoData->comments   << comments;
        d->undoData->conditions << conditions;
        d->undoData->databases  << databases;
        d->undoData->formulas   << formulas;
        d->undoData->fusions    << fusions;
        d->undoData->links      << links;
        d->undoData->matrices   << matrices;
        d->undoData->namedAreas << namedAreas;
        d->undoData->styles     << styles;
        d->undoData->userInputs << userInputs;
        d->undoData->validities << validities;
        d->undoData->values     << values;
        d->undoData->richTexts  << richTexts;
    }

    // Trigger a dependency update of the cells, which have a formula. (new positions)
    subStorage = d->formulaStorage->subStorage(invalidRegion);
    for (int i = 0; i < subStorage.count(); ++i) {
        cell = KCCell(d->sheet, subStorage.col(i), subStorage.row(i));
        d->sheet->map()->addDamage(new KCCellDamage(cell, KCCellDamage::KCFormula));
    }
    // Trigger a recalculation only for the cells, that depend on values in the changed region.
    KCRegion providers = d->sheet->map()->dependencyManager()->reduceToProvidingRegion(invalidRegion);
    d->sheet->map()->addDamage(new KCCellDamage(d->sheet, providers, KCCellDamage::KCValue));

    d->rowRepeatStorage->removeRows(position, number);
}

void KCCellStorage::removeShiftLeft(const QRect& rect)
{
    // Trigger a dependency update of the cells, which have a formula. (old positions)
    const KCRegion invalidRegion(QRect(rect.topLeft(), QPoint(KS_colMax, rect.bottom())), d->sheet);
    KCPointStorage<KCFormula> subStorage = d->formulaStorage->subStorage(invalidRegion);
    KCCell cell;
    for (int i = 0; i < subStorage.count(); ++i) {
        cell = KCCell(d->sheet, subStorage.col(i), subStorage.row(i));
        d->sheet->map()->addDamage(new KCCellDamage(cell, KCCellDamage::KCFormula));
    }
    // Trigger an update of the bindings and the named areas.
    const KCRegion region(QRect(rect.topLeft() - QPoint(1, 0), QPoint(KS_colMax, rect.bottom())), d->sheet);
    d->sheet->map()->addDamage(new KCCellDamage(d->sheet, region, KCCellDamage::KCBinding | KCCellDamage::NamedArea));

    QList< QPair<QRectF, KCBinding> > bindings = d->bindingStorage->removeShiftLeft(rect);
    QList< QPair<QRectF, QString> > comments = d->commentStorage->removeShiftLeft(rect);
    QList< QPair<QRectF, KCConditions> > conditions = d->conditionsStorage->removeShiftLeft(rect);
    QList< QPair<QRectF, Database> > databases = d->databaseStorage->removeShiftLeft(rect);
    QVector< QPair<QPoint, KCFormula> > formulas = d->formulaStorage->removeShiftLeft(rect);
    QList< QPair<QRectF, bool> > fusions = d->fusionStorage->removeShiftLeft(rect);
    QVector< QPair<QPoint, QString> > links = d->linkStorage->removeShiftLeft(rect);
    QList< QPair<QRectF, bool> > matrices = d->matrixStorage->removeShiftLeft(rect);
    QList< QPair<QRectF, QString> > namedAreas = d->namedAreaStorage->removeShiftLeft(rect);
    QList< QPair<QRectF, KCSharedSubStyle> > styles = d->styleStorage->removeShiftLeft(rect);
    QVector< QPair<QPoint, QString> > userInputs = d->userInputStorage->removeShiftLeft(rect);
    QList< QPair<QRectF, KCValidity> > validities = d->validityStorage->removeShiftLeft(rect);
    QVector< QPair<QPoint, KCValue> > values = d->valueStorage->removeShiftLeft(rect);
    QVector< QPair<QPoint, QSharedPointer<QTextDocument> > > richTexts = d->richTextStorage->removeShiftLeft(rect);
    // recording undo?
    if (d->undoData) {
        d->undoData->bindings   << bindings;
        d->undoData->comments   << comments;
        d->undoData->conditions << conditions;
        d->undoData->databases  << databases;
        d->undoData->formulas   << formulas;
        d->undoData->fusions    << fusions;
        d->undoData->links      << links;
        d->undoData->matrices   << matrices;
        d->undoData->namedAreas << namedAreas;
        d->undoData->styles     << styles;
        d->undoData->userInputs << userInputs;
        d->undoData->validities << validities;
        d->undoData->values     << values;
        d->undoData->richTexts  << richTexts;
    }

    // Trigger a dependency update of the cells, which have a formula. (new positions)
    subStorage = d->formulaStorage->subStorage(invalidRegion);
    for (int i = 0; i < subStorage.count(); ++i) {
        cell = KCCell(d->sheet, subStorage.col(i), subStorage.row(i));
        d->sheet->map()->addDamage(new KCCellDamage(cell, KCCellDamage::KCFormula));
    }
    // Trigger a recalculation only for the cells, that depend on values in the changed region.
    KCRegion providers = d->sheet->map()->dependencyManager()->reduceToProvidingRegion(invalidRegion);
    d->sheet->map()->addDamage(new KCCellDamage(d->sheet, providers, KCCellDamage::KCValue));

    d->rowRepeatStorage->removeShiftLeft(rect);
}

void KCCellStorage::insertShiftRight(const QRect& rect)
{
    // Trigger a dependency update of the cells, which have a formula. (old positions)
    const KCRegion invalidRegion(QRect(rect.topLeft(), QPoint(KS_colMax, rect.bottom())), d->sheet);
    KCPointStorage<KCFormula> subStorage = d->formulaStorage->subStorage(invalidRegion);
    KCCell cell;
    for (int i = 0; i < subStorage.count(); ++i) {
        cell = KCCell(d->sheet, subStorage.col(i), subStorage.row(i));
        d->sheet->map()->addDamage(new KCCellDamage(cell, KCCellDamage::KCFormula));
    }
    // Trigger an update of the bindings and the named areas.
    d->sheet->map()->addDamage(new KCCellDamage(d->sheet, invalidRegion, KCCellDamage::KCBinding | KCCellDamage::NamedArea));

    QList< QPair<QRectF, KCBinding> > bindings = d->bindingStorage->insertShiftRight(rect);
    QList< QPair<QRectF, QString> > comments = d->commentStorage->insertShiftRight(rect);
    QList< QPair<QRectF, KCConditions> > conditions = d->conditionsStorage->insertShiftRight(rect);
    QList< QPair<QRectF, Database> > databases = d->databaseStorage->insertShiftRight(rect);
    QVector< QPair<QPoint, KCFormula> > formulas = d->formulaStorage->insertShiftRight(rect);
    QList< QPair<QRectF, bool> > fusions = d->fusionStorage->insertShiftRight(rect);
    QVector< QPair<QPoint, QString> > links = d->linkStorage->insertShiftRight(rect);
    QList< QPair<QRectF, bool> > matrices = d->matrixStorage->insertShiftRight(rect);
    QList< QPair<QRectF, QString> > namedAreas = d->namedAreaStorage->insertShiftRight(rect);
    QList< QPair<QRectF, KCSharedSubStyle> > styles = d->styleStorage->insertShiftRight(rect);
    QVector< QPair<QPoint, QString> > userInputs = d->userInputStorage->insertShiftRight(rect);
    QList< QPair<QRectF, KCValidity> > validities = d->validityStorage->insertShiftRight(rect);
    QVector< QPair<QPoint, KCValue> > values = d->valueStorage->insertShiftRight(rect);
    QVector< QPair<QPoint, QSharedPointer<QTextDocument> > > richTexts = d->richTextStorage->insertShiftRight(rect);
    // recording undo?
    if (d->undoData) {
        d->undoData->bindings   << bindings;
        d->undoData->comments   << comments;
        d->undoData->conditions << conditions;
        d->undoData->databases  << databases;
        d->undoData->formulas   << formulas;
        d->undoData->fusions    << fusions;
        d->undoData->links      << links;
        d->undoData->matrices   << matrices;
        d->undoData->namedAreas << namedAreas;
        d->undoData->styles     << styles;
        d->undoData->userInputs << userInputs;
        d->undoData->validities << validities;
        d->undoData->values     << values;
        d->undoData->richTexts  << richTexts;
    }

    // Trigger a dependency update of the cells, which have a formula. (new positions)
    subStorage = d->formulaStorage->subStorage(invalidRegion);
    for (int i = 0; i < subStorage.count(); ++i) {
        cell = KCCell(d->sheet, subStorage.col(i), subStorage.row(i));
        d->sheet->map()->addDamage(new KCCellDamage(cell, KCCellDamage::KCFormula));
    }
    // Trigger a recalculation only for the cells, that depend on values in the changed region.
    KCRegion providers = d->sheet->map()->dependencyManager()->reduceToProvidingRegion(invalidRegion);
    d->sheet->map()->addDamage(new KCCellDamage(d->sheet, providers, KCCellDamage::KCValue));

    d->rowRepeatStorage->insertShiftRight(rect);
}

void KCCellStorage::removeShiftUp(const QRect& rect)
{
    // Trigger a dependency update of the cells, which have a formula. (old positions)
    const KCRegion invalidRegion(QRect(rect.topLeft(), QPoint(rect.right(), KS_rowMax)), d->sheet);
    KCPointStorage<KCFormula> subStorage = d->formulaStorage->subStorage(invalidRegion);
    KCCell cell;
    for (int i = 0; i < subStorage.count(); ++i) {
        cell = KCCell(d->sheet, subStorage.col(i), subStorage.row(i));
        d->sheet->map()->addDamage(new KCCellDamage(cell, KCCellDamage::KCFormula));
    }
    // Trigger an update of the bindings and the named areas.
    const KCRegion region(QRect(rect.topLeft() - QPoint(0, 1), QPoint(rect.right(), KS_rowMax)), d->sheet);
    d->sheet->map()->addDamage(new KCCellDamage(d->sheet, region, KCCellDamage::KCBinding | KCCellDamage::NamedArea));

    QList< QPair<QRectF, KCBinding> > bindings = d->bindingStorage->removeShiftUp(rect);
    QList< QPair<QRectF, QString> > comments = d->commentStorage->removeShiftUp(rect);
    QList< QPair<QRectF, KCConditions> > conditions = d->conditionsStorage->removeShiftUp(rect);
    QList< QPair<QRectF, Database> > databases = d->databaseStorage->removeShiftUp(rect);
    QVector< QPair<QPoint, KCFormula> > formulas = d->formulaStorage->removeShiftUp(rect);
    QList< QPair<QRectF, bool> > fusions = d->fusionStorage->removeShiftUp(rect);
    QVector< QPair<QPoint, QString> > links = d->linkStorage->removeShiftUp(rect);
    QList< QPair<QRectF, bool> > matrices = d->matrixStorage->removeShiftUp(rect);
    QList< QPair<QRectF, QString> > namedAreas = d->namedAreaStorage->removeShiftUp(rect);
    QList< QPair<QRectF, KCSharedSubStyle> > styles = d->styleStorage->removeShiftUp(rect);
    QVector< QPair<QPoint, QString> > userInputs = d->userInputStorage->removeShiftUp(rect);
    QList< QPair<QRectF, KCValidity> > validities = d->validityStorage->removeShiftUp(rect);
    QVector< QPair<QPoint, KCValue> > values = d->valueStorage->removeShiftUp(rect);
    QVector< QPair<QPoint, QSharedPointer<QTextDocument> > > richTexts = d->richTextStorage->removeShiftUp(rect);
    // recording undo?
    if (d->undoData) {
        d->undoData->bindings   << bindings;
        d->undoData->comments   << comments;
        d->undoData->conditions << conditions;
        d->undoData->databases  << databases;
        d->undoData->formulas   << formulas;
        d->undoData->fusions    << fusions;
        d->undoData->links      << links;
        d->undoData->matrices   << matrices;
        d->undoData->namedAreas << namedAreas;
        d->undoData->styles     << styles;
        d->undoData->userInputs << userInputs;
        d->undoData->validities << validities;
        d->undoData->values     << values;
        d->undoData->richTexts  << richTexts;
    }

    // Trigger a dependency update of the cells, which have a formula. (new positions)
    subStorage = d->formulaStorage->subStorage(invalidRegion);
    for (int i = 0; i < subStorage.count(); ++i) {
        cell = KCCell(d->sheet, subStorage.col(i), subStorage.row(i));
        d->sheet->map()->addDamage(new KCCellDamage(cell, KCCellDamage::KCFormula));
    }
    // Trigger a recalculation only for the cells, that depend on values in the changed region.
    KCRegion providers = d->sheet->map()->dependencyManager()->reduceToProvidingRegion(invalidRegion);
    d->sheet->map()->addDamage(new KCCellDamage(d->sheet, providers, KCCellDamage::KCValue));

    d->rowRepeatStorage->removeShiftUp(rect);
}

void KCCellStorage::insertShiftDown(const QRect& rect)
{
    // Trigger a dependency update of the cells, which have a formula. (old positions)
    const KCRegion invalidRegion(QRect(rect.topLeft(), QPoint(rect.right(), KS_rowMax)), d->sheet);
    KCPointStorage<KCFormula> subStorage = d->formulaStorage->subStorage(invalidRegion);
    KCCell cell;
    for (int i = 0; i < subStorage.count(); ++i) {
        cell = KCCell(d->sheet, subStorage.col(i), subStorage.row(i));
        d->sheet->map()->addDamage(new KCCellDamage(cell, KCCellDamage::KCFormula));
    }
    // Trigger an update of the bindings and the named areas.
    d->sheet->map()->addDamage(new KCCellDamage(d->sheet, invalidRegion, KCCellDamage::KCBinding | KCCellDamage::NamedArea));

    QList< QPair<QRectF, KCBinding> > bindings = d->bindingStorage->insertShiftDown(rect);
    QList< QPair<QRectF, QString> > comments = d->commentStorage->insertShiftDown(rect);
    QList< QPair<QRectF, KCConditions> > conditions = d->conditionsStorage->insertShiftDown(rect);
    QList< QPair<QRectF, Database> > databases = d->databaseStorage->insertShiftDown(rect);
    QVector< QPair<QPoint, KCFormula> > formulas = d->formulaStorage->insertShiftDown(rect);
    QList< QPair<QRectF, bool> > fusions = d->fusionStorage->insertShiftDown(rect);
    QVector< QPair<QPoint, QString> > links = d->linkStorage->insertShiftDown(rect);
    QList< QPair<QRectF, bool> > matrices = d->matrixStorage->insertShiftDown(rect);
    QList< QPair<QRectF, QString> > namedAreas = d->namedAreaStorage->insertShiftDown(rect);
    QList< QPair<QRectF, KCSharedSubStyle> > styles = d->styleStorage->insertShiftDown(rect);
    QVector< QPair<QPoint, QString> > userInputs = d->userInputStorage->insertShiftDown(rect);
    QList< QPair<QRectF, KCValidity> > validities = d->validityStorage->insertShiftDown(rect);
    QVector< QPair<QPoint, KCValue> > values = d->valueStorage->insertShiftDown(rect);
    QVector< QPair<QPoint, QSharedPointer<QTextDocument> > > richTexts = d->richTextStorage->insertShiftDown(rect);
    // recording undo?
    if (d->undoData) {
        d->undoData->bindings   << bindings;
        d->undoData->comments   << comments;
        d->undoData->conditions << conditions;
        d->undoData->databases  << databases;
        d->undoData->formulas   << formulas;
        d->undoData->fusions    << fusions;
        d->undoData->links      << links;
        d->undoData->matrices   << matrices;
        d->undoData->namedAreas << namedAreas;
        d->undoData->styles     << styles;
        d->undoData->userInputs << userInputs;
        d->undoData->validities << validities;
        d->undoData->values     << values;
        d->undoData->richTexts  << richTexts;
    }

    // Trigger a dependency update of the cells, which have a formula. (new positions)
    subStorage = d->formulaStorage->subStorage(invalidRegion);
    for (int i = 0; i < subStorage.count(); ++i) {
        cell = KCCell(d->sheet, subStorage.col(i), subStorage.row(i));
        d->sheet->map()->addDamage(new KCCellDamage(cell, KCCellDamage::KCFormula));
    }
    // Trigger a recalculation only for the cells, that depend on values in the changed region.
    KCRegion providers = d->sheet->map()->dependencyManager()->reduceToProvidingRegion(invalidRegion);
    d->sheet->map()->addDamage(new KCCellDamage(d->sheet, providers, KCCellDamage::KCValue));

    d->rowRepeatStorage->insertShiftDown(rect);
}

KCCell KCCellStorage::firstInColumn(int col, Visiting visiting) const
{
    Q_UNUSED(visiting);

    int newRow = 0;
    int tmpRow = 0;
    d->formulaStorage->firstInColumn(col, &tmpRow);
    newRow = tmpRow;
    d->valueStorage->firstInColumn(col, &tmpRow);
    if (tmpRow)
        newRow = newRow ? qMin(newRow, tmpRow) : tmpRow;
    if (!newRow)
        return KCCell();
    return KCCell(d->sheet, col, newRow);
}

KCCell KCCellStorage::firstInRow(int row, Visiting visiting) const
{
    int newCol = 0;
    int tmpCol = 0;
    d->formulaStorage->firstInRow(row, &tmpCol);
    newCol = tmpCol;
    d->valueStorage->firstInRow(row, &tmpCol);
    if (tmpCol)
        newCol = newCol ? qMin(newCol, tmpCol) : tmpCol;
    if (visiting == VisitAll) {
        tmpCol = d->styleStorage->firstColumnIndexInRow(row);
        if (tmpCol)
            newCol = newCol ? qMin(newCol, tmpCol) : tmpCol;
    }
    if (!newCol)
        return KCCell();
    return KCCell(d->sheet, newCol, row);
}

KCCell KCCellStorage::lastInColumn(int col, Visiting visiting) const
{
    Q_UNUSED(visiting);
    int newRow = 0;
    int tmpRow = 0;
    d->formulaStorage->lastInColumn(col, &tmpRow);
    newRow = tmpRow;
    d->valueStorage->lastInColumn(col, &tmpRow);
    newRow = qMax(newRow, tmpRow);
    if (!newRow)
        return KCCell();
    return KCCell(d->sheet, col, newRow);
}

KCCell KCCellStorage::lastInRow(int row, Visiting visiting) const
{
    Q_UNUSED(visiting);
    int newCol = 0;
    int tmpCol = 0;
    d->formulaStorage->lastInRow(row, &tmpCol);
    newCol = tmpCol;
    d->valueStorage->lastInRow(row, &tmpCol);
    newCol = qMax(newCol, tmpCol);
    if (!newCol)
        return KCCell();
    return KCCell(d->sheet, newCol, row);
}

KCCell KCCellStorage::nextInColumn(int col, int row, Visiting visiting) const
{
    Q_UNUSED(visiting);
    int newRow = 0;
    int tmpRow = 0;
    d->formulaStorage->nextInColumn(col, row, &tmpRow);
    newRow = tmpRow;
    d->valueStorage->nextInColumn(col, row, &tmpRow);
    if (tmpRow)
        newRow = newRow ? qMin(newRow, tmpRow) : tmpRow;
    if (!newRow)
        return KCCell();
    return KCCell(d->sheet, col, newRow);
}

KCCell KCCellStorage::nextInRow(int col, int row, Visiting visiting) const
{
    int newCol = 0;
    int tmpCol = 0;
    d->formulaStorage->nextInRow(col, row, &tmpCol);
    newCol = tmpCol;
    d->valueStorage->nextInRow(col, row, &tmpCol);
    if (tmpCol)
        newCol = newCol ? qMin(newCol, tmpCol) : tmpCol;
    if (visiting == VisitAll) {
        tmpCol = d->styleStorage->nextColumnIndexInRow(col, row);
        if (tmpCol)
            newCol = newCol ? qMin(newCol, tmpCol) : tmpCol;
    }
    if (!newCol)
        return KCCell();
    return KCCell(d->sheet, newCol, row);
}

KCCell KCCellStorage::prevInColumn(int col, int row, Visiting visiting) const
{
    Q_UNUSED(visiting);
    int newRow = 0;
    int tmpRow = 0;
    d->formulaStorage->prevInColumn(col, row, &tmpRow);
    newRow = tmpRow;
    d->valueStorage->prevInColumn(col, row, &tmpRow);
    newRow = qMax(newRow, tmpRow);
    if (!newRow)
        return KCCell();
    return KCCell(d->sheet, col, newRow);
}

KCCell KCCellStorage::prevInRow(int col, int row, Visiting visiting) const
{
    Q_UNUSED(visiting);
    int newCol = 0;
    int tmpCol = 0;
    d->formulaStorage->prevInRow(col, row, &tmpCol);
    newCol = tmpCol;
    d->valueStorage->prevInRow(col, row, &tmpCol);
    newCol = qMax(newCol, tmpCol);
    if (!newCol)
        return KCCell();
    return KCCell(d->sheet, newCol, row);
}

int KCCellStorage::columns(bool includeStyles) const
{
    int max = 0;
    max = qMax(max, d->commentStorage->usedArea().right());
    max = qMax(max, d->conditionsStorage->usedArea().right());
    max = qMax(max, d->fusionStorage->usedArea().right());
    if (includeStyles) max = qMax(max, d->styleStorage->usedArea().right());
    max = qMax(max, d->validityStorage->usedArea().right());
    max = qMax(max, d->formulaStorage->columns());
    max = qMax(max, d->linkStorage->columns());
    max = qMax(max, d->valueStorage->columns());

    // don't include bindings cause the bindingStorage does only listen to all cells in the sheet.
    //max = qMax(max, d->bindingStorage->usedArea().right());

    return max;
}

int KCCellStorage::rows(bool includeStyles) const
{
    int max = 0;
    max = qMax(max, d->commentStorage->usedArea().bottom());
    max = qMax(max, d->conditionsStorage->usedArea().bottom());
    max = qMax(max, d->fusionStorage->usedArea().bottom());
    if (includeStyles) max = qMax(max, d->styleStorage->usedArea().bottom());
    max = qMax(max, d->validityStorage->usedArea().bottom());
    max = qMax(max, d->formulaStorage->rows());
    max = qMax(max, d->linkStorage->rows());
    max = qMax(max, d->valueStorage->rows());

    // don't include bindings cause the bindingStorage does only listen to all cells in the sheet.
    //max = qMax(max, d->bindingStorage->usedArea().bottom());

    return max;
}

KCCellStorage KCCellStorage::subStorage(const KCRegion& region) const
{
    KCCellStorage subStorage(d->sheet);
    *subStorage.d->formulaStorage = d->formulaStorage->subStorage(region);
    *subStorage.d->linkStorage = d->linkStorage->subStorage(region);
    *subStorage.d->valueStorage = d->valueStorage->subStorage(region);
    return subStorage;
}

const KCBindingStorage* KCCellStorage::bindingStorage() const
{
    return d->bindingStorage;
}

const CommentStorage* KCCellStorage::commentStorage() const
{
    return d->commentStorage;
}

const KCConditionsStorage* KCCellStorage::conditionsStorage() const
{
    return d->conditionsStorage;
}

const KCFormulaStorage* KCCellStorage::formulaStorage() const
{
    return d->formulaStorage;
}

const FusionStorage* KCCellStorage::fusionStorage() const
{
    return d->fusionStorage;
}

const LinkStorage* KCCellStorage::linkStorage() const
{
    return d->linkStorage;
}

const KCStyleStorage* KCCellStorage::styleStorage() const
{
    return d->styleStorage;
}

const KCValidityStorage* KCCellStorage::validityStorage() const
{
    return d->validityStorage;
}

const KCValueStorage* KCCellStorage::valueStorage() const
{
    return d->valueStorage;
}

void KCCellStorage::startUndoRecording()
{
    // If undoData is not null, the recording wasn't stopped.
    // Should not happen, hence this assertion.
    Q_ASSERT(d->undoData == 0);
    d->undoData = new KCCellStorageUndoData();
}

void KCCellStorage::stopUndoRecording(QUndoCommand *parent)
{
    // If undoData is null, the recording wasn't started.
    // Should not happen, hence this assertion.
    Q_ASSERT(d->undoData != 0);
    // append sub-commands to the parent command
    d->createCommand(parent); // needs d->undoData
    for (int i = 0; i < d->undoData->namedAreas.count(); ++i) {
        emit namedAreaRemoved(d->undoData->namedAreas[i].second);
    }
    delete d->undoData;
    d->undoData = 0;
}

void KCCellStorage::loadConditions(const QList<QPair<QRegion, KCConditions> >& conditions)
{
    d->conditionsStorage->load(conditions);
}

void KCCellStorage::loadStyles(const QList<QPair<QRegion, KCStyle> > &styles)
{
    d->styleStorage->load(styles);
}

void KCCellStorage::invalidateStyleCache()
{
    d->styleStorage->invalidateCache();
}

int KCCellStorage::rowRepeat(int row) const
{
    return d->rowRepeatStorage->rowRepeat(row);
}

int KCCellStorage::firstIdenticalRow(int row) const
{
    return d->rowRepeatStorage->firstIdenticalRow(row);
}

void KCCellStorage::setRowsRepeated(int row, int count)
{
    d->rowRepeatStorage->setRowRepeat(row, count);
}

#include "KCCellStorage.moc"
