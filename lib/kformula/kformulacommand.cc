/* This file is part of the KDE project
   Copyright (C) 2001 Andrea Rizzi <rizzi@kde.org>
	              Ulrich Kuettler <ulrich.kuettler@mailbox.tu-dresden.de>

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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include <iostream>
#include <klocale.h>  //This is for undo descriptions

#include <qlist.h>

#include "kformulacommand.h"
#include "formulacursor.h"
#include "indexelement.h"
#include "matrixelement.h"
#include "sequenceelement.h"
#include "textelement.h"

KFORMULA_NAMESPACE_BEGIN

int KFormulaCommand::evilDestructionCount = 0;

KFormulaCommand::KFormulaCommand(const QString &name, KFormulaContainer* document)
        : KCommand(name), cursordata(0), undocursor(0), doc(document)
{
    evilDestructionCount++;
}

KFormulaCommand::~KFormulaCommand()
{
    evilDestructionCount--;
    delete undocursor;
    delete cursordata;
}

FormulaCursor* KFormulaCommand::getExecuteCursor()
{
    FormulaCursor* cursor = getActiveCursor();
    if (cursordata == 0) {
        setExecuteCursor(getActiveCursor());
    }
    else {
        cursor->setCursorData(cursordata);
    }
    return cursor;
}

void KFormulaCommand::setExecuteCursor(FormulaCursor* cursor)
{
    // assert(cursordata == 0);
    cursordata = cursor->getCursorData();
}

FormulaCursor* KFormulaCommand::getUnexecuteCursor()
{
    FormulaCursor* cursor = getActiveCursor();
    cursor->setCursorData(undocursor);
    destroyUndoCursor();
    return cursor;
}

void KFormulaCommand::setUnexecuteCursor(FormulaCursor* cursor)
{
    // assert(undocursor == 0);
    undocursor = cursor->getCursorData();
}


// ******  Generic Add command

KFCAdd::KFCAdd(const QString &name, KFormulaContainer *document)
        : KFormulaCommand(name, document)
{
    addList.setAutoDelete( true );
}


void KFCAdd::execute()
{
    FormulaCursor* cursor = getExecuteCursor();
    cursor->insert(addList, BasicElement::beforeCursor);
    setUnexecuteCursor(cursor);
    cursor->setSelection(false);
    testDirty();
}


void KFCAdd::unexecute()
{
    FormulaCursor* cursor = getUnexecuteCursor();
    cursor->remove(addList, BasicElement::beforeCursor);
    //cursor->setSelection(false);
    cursor->normalize();
    testDirty();
}



// ******  Remove selection command

KFCRemoveSelection::KFCRemoveSelection(KFormulaContainer *document,
                                       BasicElement::Direction direction)
        : KFormulaCommand(i18n("Remove selected text"), document),
          dir(direction)
{
    removedList.setAutoDelete( true );
}

void KFCRemoveSelection::execute()
{
    FormulaCursor* cursor = getExecuteCursor();
    cursor->remove(removedList, dir);
    setUnexecuteCursor(cursor);
    testDirty();
}

void KFCRemoveSelection::unexecute()
{
    FormulaCursor* cursor = getUnexecuteCursor();
    cursor->insert(removedList);
    cursor->setSelection(false);
    testDirty();
}



KFCReplace::KFCReplace(const QString &name, KFormulaContainer* document)
        : KFCAdd(name, document), removeSelection(0)
{
}

KFCReplace::~KFCReplace()
{
    delete removeSelection;
}

void KFCReplace::execute()
{
    if (getActiveCursor()->isSelection() && (removeSelection == 0)) {
        removeSelection = new KFCRemoveSelection(getDocument());
    }
    if (removeSelection != 0) {
        removeSelection->execute();
    }
    KFCAdd::execute();
}

void KFCReplace::unexecute()
{
    KFCAdd::unexecute();
    if (removeSelection != 0) {
        removeSelection->unexecute();
    }
}



KFCRemove::KFCRemove(KFormulaContainer *document,
                     BasicElement::Direction direction)
        : KFormulaCommand(i18n("Remove selected text"), document),
          element(0), simpleRemoveCursor(0), dir(direction)
{
    removedList.setAutoDelete( true );
}

KFCRemove::~KFCRemove()
{
    delete simpleRemoveCursor;
    delete element;
}

void KFCRemove::execute()
{
    FormulaCursor* cursor = getExecuteCursor();
    cursor->remove(removedList, dir);
    if (cursor->elementIsSenseless()) {
        simpleRemoveCursor = cursor->getCursorData();
        element = cursor->replaceByMainChildContent();
    }
    setUnexecuteCursor(cursor);
    cursor->normalize();
    testDirty();
}

void KFCRemove::unexecute()
{
    FormulaCursor* cursor = getUnexecuteCursor();
    if (element != 0) {
        cursor->replaceSelectionWith(element);
        element = 0;

        cursor->setCursorData(simpleRemoveCursor);
        delete simpleRemoveCursor;
        simpleRemoveCursor = 0;
    }
    cursor->insert(removedList, dir);
    cursor->setSelection(false);
    testDirty();
}


KFCRemoveEnclosing::KFCRemoveEnclosing(KFormulaContainer* document,
                                       BasicElement::Direction dir)
        : KFormulaCommand(i18n("Remove enclosing element"), document),
          element(0), direction(dir)
{
}

KFCRemoveEnclosing::~KFCRemoveEnclosing()
{
    delete element;
}

void KFCRemoveEnclosing::execute()
{
    FormulaCursor* cursor = getExecuteCursor();
    element = cursor->removeEnclosingElement(direction);
    setUnexecuteCursor(cursor);
    //cursor->normalize();
    cursor->setSelection(false);
    testDirty();
}

void KFCRemoveEnclosing::unexecute()
{
    FormulaCursor* cursor = getUnexecuteCursor();
    cursor->replaceSelectionWith(element);
    cursor->normalize();
    cursor->setSelection(false);
    element = 0;
    testDirty();
}


// ******  Add root, bracket etc command

KFCAddReplacing::KFCAddReplacing(const QString &name, KFormulaContainer* document)
        : KFormulaCommand(name, document), element(0)
{
}

KFCAddReplacing::~KFCAddReplacing()
{
    delete element;
}


void KFCAddReplacing::execute()
{
    FormulaCursor* cursor = getExecuteCursor();
    cursor->replaceSelectionWith(element);
    setUnexecuteCursor(cursor);
    cursor->goInsideElement(element);
    element = 0;
    testDirty();
}


void KFCAddReplacing::unexecute()
{
    FormulaCursor* cursor = getUnexecuteCursor();
    element = cursor->replaceByMainChildContent();
    cursor->normalize();
    testDirty();
}


// ******  Add index command

KFCAddGenericIndex::KFCAddGenericIndex(KFormulaContainer* document, ElementIndexPtr _index)
        : KFCAdd(i18n("Add index"), document), index(_index)
{
    addElement(new SequenceElement());
}

void KFCAddGenericIndex::execute()
{
    index->setToIndex(getActiveCursor());
    KFCAdd::execute();
}


KFCAddIndex::KFCAddIndex(KFormulaContainer* document,
                         IndexElement* element, ElementIndexPtr index)
        : KFCAddReplacing(i18n("Add index"), document),
          addIndex(document, index)
{
    setElement(element);
}

KFCAddIndex::~KFCAddIndex()
{
}

void KFCAddIndex::execute()
{
    KFCAddReplacing::execute();
    addIndex.execute();
}

void KFCAddIndex::unexecute()
{
    addIndex.unexecute();
    KFCAddReplacing::unexecute();
}

KFORMULA_NAMESPACE_END
