/* This file is part of the KDE project
 * Copyright (C) 2009 Jan Hambrecht <jaham@gmx.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "ArtworkDocumentMergeCommand.h"
#include "ArtworkPart.h"
#include "ArtworkDocument.h"
#include "KShapeLayer.h"
#include <KLocale>

class ArtworkDocumentMergeCommand::Private
{
public:
    Private() : hasMerged(false)
    {
    }

    ~Private()
    {
        if (!hasMerged) {
            qDeleteAll(layers);
            qDeleteAll(shapes);
        }
    }

    ArtworkPart * targetPart;
    QList<KShapeLayer*> layers;
    QList<KShape*> shapes;
    bool hasMerged;
};

ArtworkDocumentMergeCommand::ArtworkDocumentMergeCommand(ArtworkPart * targetPart, ArtworkPart * sourcePart)
        : QUndoCommand(0), d(new Private())
{
    d->targetPart = targetPart;
    d->layers = sourcePart->document().layers();
    d->shapes = sourcePart->document().shapes();
    foreach(KShapeLayer * layer, d->layers) {
        sourcePart->removeShape(layer);
    }
    foreach(KShape * shape, d->shapes) {
        sourcePart->removeShape(shape);
    }
    setText(i18n("Insert graphics"));
}

ArtworkDocumentMergeCommand::~ArtworkDocumentMergeCommand()
{
    delete d;
}

void ArtworkDocumentMergeCommand::redo()
{
    if (!d->hasMerged) {
        foreach(KShapeLayer * layer, d->layers) {
            d->targetPart->addShape(layer);
        }
        foreach(KShape * shape, d->shapes) {
            d->targetPart->addShape(shape);
        }
        d->hasMerged = true;
    }

    QUndoCommand::redo();
}

void ArtworkDocumentMergeCommand::undo()
{
    QUndoCommand::undo();

    if (d->hasMerged) {
        foreach(KShapeLayer * layer, d->layers) {
            d->targetPart->removeShape(layer);
        }
        foreach(KShape * shape, d->shapes) {
            d->targetPart->removeShape(shape);
        }
        d->hasMerged = false;
    }
}
