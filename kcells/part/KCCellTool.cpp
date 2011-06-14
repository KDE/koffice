/* This file is part of the KDE project
   Copyright 2006-2007 Stefan Nikolaus <stefan.nikolaus@kdemail.net>
   Copyright 2006 Robert Knight <robertknight@gmail.com>
   Copyright 2006 Inge Wallin <inge@lysator.liu.se>
   Copyright 1999-2002,2004 Laurent Montel <montel@kde.org>
   Copyright 2002-2005 Ariya Hidayat <ariya@kde.org>
   Copyright 1999-2004 David Faure <faure@kde.org>
   Copyright 2004-2005 Meni Livne <livne@kde.org>
   Copyright 2001-2003 Philipp Mueller <philipp.mueller@gmx.de>
   Copyright 2002-2003 Norbert Andres <nandres@web.de>
   Copyright 2003 Hamish Rodda <rodda@kde.org>
   Copyright 2003 Joseph Wenninger <jowenn@kde.org>
   Copyright 2003 Lukas Tinkl <lukas@kde.org>
   Copyright 2000-2002 Werner Trobin <trobin@kde.org>
   Copyright 2002 Harri Porten <porten@kde.org>
   Copyright 2002 John Dailey <dailey@vt.edu>
   Copyright 2002 Daniel Naber <daniel.naber@t-online.de>
   Copyright 1999-2000 Torben Weis <weis@kde.org>
   Copyright 1999-2000 Stephan Kulow <coolo@kde.org>
   Copyright 2000 Bernd Wuebben <wuebben@kde.org>
   Copyright 2000 Wilco Greven <greven@kde.org>
   Copyright 2000 Simon Hausmann <hausmann@kde.org
   Copyright 1999 Michael Reiher <michael.reiher@gmx.de>
   Copyright 1999 Boris Wedl <boris.wedl@kfunigraz.ac.at>
   Copyright 1999 Reginald Stadlbauer <reggie@kde.org>

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
#include "KCCellTool.h"

#include <QPainter>

#include <KAction>
#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kmimetype.h>
#include <krun.h>
#include <KSelectAction>

#include <KCanvasBase.h>
#include <KoCanvasController.h>
#include <KoPointerEvent.h>
#include <KoSelection.h>
#include <KoShapeManager.h>
#include <KoViewConverter.h>

#include "KCCell.h"
#include "KCCanvas.h"
#include "KCDoc.h"
#include "kcells_limits.h"
#include "KCPrintSettings.h"
#include "KCSheet.h"
#include "KCStyleManager.h"
#include "Util.h"
#include "KCView.h"

#include "commands/KCAutoFillCommand.h"
#include "commands/DefinePrintRangeCommand.h"

#include "ui/CellView.h"
#include "ui/Selection.h"
#include "ui/SheetView.h"

class KCCellTool::Private
{
public:
    KCCanvas* canvas;
};

KCCellTool::KCCellTool(KCanvasBase* canvas)
        : CellToolBase(canvas)
        , d(new Private)
{
    d->canvas = static_cast<KCCanvas*>(canvas);

    KAction* action = 0;

    // -- misc actions --

    action = new KAction(i18n("Define Print Range"), this);
    addAction("definePrintRange", action);
    connect(action, SIGNAL(triggered(bool)), this, SLOT(definePrintRange()));
    action->setToolTip(i18n("Define the print range in the current sheet"));
}

KCCellTool::~KCCellTool()
{
    delete d;
}

void KCCellTool::paint(QPainter &painter, const KoViewConverter &viewConverter)
{
    KoShape::applyConversion(painter, viewConverter);
    const double xOffset = viewConverter.viewToDocumentX(canvas()->canvasController()->canvasOffsetX());
    const double yOffset = viewConverter.viewToDocumentY(canvas()->canvasController()->canvasOffsetY());
    // The visible area in document coordinates:
    const QRectF paintRect = QRectF(QPointF(-xOffset, -yOffset), size());

    /* paint the selection */
    paintReferenceSelection(painter, paintRect);
    paintSelection(painter, paintRect);
}

void KCCellTool::activate(ToolActivation toolActivation, const QSet<KoShape*> &shapes)
{
    canvas()->shapeManager()->selection()->deselectAll();
    CellToolBase::activate(toolActivation, shapes);
}

Selection* KCCellTool::selection()
{
    return d->canvas->selection();
}

QPointF KCCellTool::offset() const
{
    return QPointF(0.0, 0.0);
}

QSizeF KCCellTool::size() const
{
    return canvas()->viewConverter()->viewToDocument(d->canvas->size());
}

QPointF KCCellTool::canvasOffset() const
{
    return d->canvas->offset();
}

int KCCellTool::maxCol() const
{
    return KS_colMax;
}

int KCCellTool::maxRow() const
{
    return KS_rowMax;
}

SheetView* KCCellTool::sheetView(const KCSheet* sheet) const
{
    return d->canvas->view()->sheetView(sheet);
}

void KCCellTool::definePrintRange()
{
    DefinePrintRangeCommand* command = new DefinePrintRangeCommand();
    command->setSheet(selection()->activeSheet());
    command->add(*selection());
    d->canvas->doc()->addCommand(command);
}

#include "KCCellTool.moc"
