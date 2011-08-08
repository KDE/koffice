/* This file is part of the KDE project
   Copyright (C) 2002 Lennart Kudling <kudling@kde.org>
   Copyright (C) 2002 Benoît Vautrin <benoit.vautrin@free.fr>
   Copyright (C) 2005 Thomas Zander <zander@kde.org>
   Copyright (C) 2006-2007 Jan Hambrecht <jaham@gmx.net>
   Copyright (C) 2006 Stephan Kulow <coolo@kde.org>
   Copyright (C) 2006 Peter Simonsson <psn@linux.se>

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
 * Boston, MA 02110-1301, USA.
*/

#ifndef ARTWORKLAYERREORDERCOMMAND_H
#define ARTWORKLAYERREORDERCOMMAND_H

#include <QUndoCommand>
#include <artworkcommon_export.h>

class ArtworkDocument;
class KShapeLayer;

/// Command for raising or lowering layers
class ARTWORKCOMMON_EXPORT ArtworkLayerReorderCommand : public QUndoCommand
{
public:
    /// The different types of layer commands.
    enum ReorderType {
        RaiseLayer, ///< raises layer in z-order
        LowerLayer  ///< lowers layer in z-order
    };

    /**
     * Layer command which works on a single layer.
     * @param document the document containing the layer
     * @param layer the layer which is subject to the command
     * @param commandType the type of the command to redo
     */
    ArtworkLayerReorderCommand(ArtworkDocument* document, KShapeLayer* layer, ReorderType commandType, QUndoCommand* parent = 0);

    /**
     * Layer command which works on a single layer.
     * @param document the document containing the layer
     * @param layers the list of layers which are subject to the command
     * @param commandType the type of the command to redo
     */
    ArtworkLayerReorderCommand(ArtworkDocument* document, QList<KShapeLayer*> layers, ReorderType commandType, QUndoCommand* parent = 0);

    virtual ~ArtworkLayerReorderCommand();

    /// redo the command
    virtual void redo();
    /// revert the actions done in redo
    virtual void undo();

private:
    ArtworkDocument *m_document;         ///< the document to work on
    QList<KShapeLayer*> m_layers; ///< the list of layers subject to the command
    ReorderType m_cmdType;         ///< the type of the command to redo
};

#endif // ARTWORKLAYERREORDERCOMMAND_H

