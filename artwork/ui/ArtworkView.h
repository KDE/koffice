/* This file is part of the KDE project
   Copyright (C) 2001-2002 Lennart Kudling <kudling@kde.org>
   Copyright (C) 2001-2005,2007 Rob Buis <buis@kde.org>
   Copyright (C) 2002-2003,2005 Tomislav Lukman <tomislav.lukman@ck.t-com.hr>
   Copyright (C) 2002,2005 Laurent Montel <montel@kde.org>
   Copyright (C) 2002,2005,2007 David Faure <faure@kde.org>
   Copyright (C) 2002 Benoit Vautrin <benoit.vautrin@free.fr>
   Copyright (C) 2005-2006 Peter Simonsson <psn@linux.se>
   Copyright (C) 2005-2006 Tim Beaulen <tbscope@gmail.com>
   Copyright (C) 2005-2006 Thomas Zander <zander@kde.org>
   Copyright (C) 2005-2007 Jan Hambrecht <jaham@gmx.net>
   Copyright (C) 2005-2006 Inge Wallin <inge@lysator.liu.se>
   Copyright (C) 2005-2006 Casper Boemann <cbr@boemann.dk>
   Copyright (C) 2005-2006 Sven Langkamp <sven.langkamp@gmail.com>
   Copyright (C) 2006 Martin Ellis <martin.ellis@kdemail.net>
   Copyright (C) 2006 Boudewijn Rempt <boud@valdyas.org>

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

#ifndef ARTWORK_VIEW_H
#define ARTWORK_VIEW_H

#include <KoView.h>
#include <QPointF>
#include <kxmlguibuilder.h>
#include <KUnit.h>
#include <KShapeAlignCommand.h>
#include <KShapeDistributeCommand.h>
#include <KoZoomMode.h>
#include <artworkui_export.h>
#include "ArtworkBooleanCommand.h"

class QLabel;
class QDropEvent;
class QResizeEvent;
class QRectF;
class QPrinter;

class KAction;
class ArtworkPart;
class KSelectAction;
class KToggleAction;

class KCanvasController;
class KoRuler;

class ArtworkLayerDocker;
class ArtworkZoomController;

class ArtworkCanvas;
class ArtworkStylePreviewDocker;

class ARTWORKUI_EXPORT ArtworkView : public KoView
{
    Q_OBJECT

public:
    explicit ArtworkView(ArtworkPart* part, QWidget* parent = 0);
    virtual ~ArtworkView();

    /// Returns the view is attached to
    ArtworkPart * part() const;

    /// Returns the canvas widget of this view
    ArtworkCanvas * canvasWidget() const;

    void reorganizeGUI();
    void setNumberOfRecentFiles(uint number);

    /// Reimplemented from QWidget
    virtual void setCursor(const QCursor &);
    /// Reimplemented from QWidget
    virtual void dropEvent(QDropEvent *e);
    /// Reimplemented from KoView
    virtual KoZoomController *zoomController() const;


public slots:
    // editing:
    void editSelectAll();
    void editDeselectAll();
    void editDeleteSelection();

    void selectionDuplicate();
    void selectionDistributeHorizontalCenter();
    void selectionDistributeHorizontalGap();
    void selectionDistributeHorizontalLeft();
    void selectionDistributeHorizontalRight();
    void selectionDistributeVerticalCenter();
    void selectionDistributeVerticalGap();
    void selectionDistributeVerticalBottom();
    void selectionDistributeVerticalTop();

    void fileImportGraphic();

    void closePath();
    void combinePath();
    void separatePath();
    void reversePath();

    void intersectPaths();
    void subtractPaths();
    void unitePaths();
    void excludePaths();

    void pathSnapToGrid();

    void configure();

    void pageLayout();

    void selectionChanged();

    void togglePageMargins(bool);
    void showRuler();
    void showGuides();
    void snapToGrid();

protected slots:
    // Object related operations.

    // View
    void viewModeChanged(bool);
    void zoomSelection();
    void zoomDrawing();

    void mousePositionChanged(const QPoint &position);
    void pageOffsetChanged();

    void updateUnit(const KUnit &unit);

    void applyFillToSelection();
    void applyStrokeToSelection();

protected:
    virtual void updateReadWrite(bool readwrite);
    virtual void resizeEvent(QResizeEvent* event);

    void createLayersTabDock();
    void createStrokeDock();
    void createColorDock();

    virtual KoPrintJob * createPrintJob();

private:
    void initActions();
    void updateRuler();

    void selectionDistribute(KShapeDistributeCommand::Distribute distribute);

    void booleanOperation(ArtworkBooleanCommand::BooleanOperation operation);

    /// Returns a list of all selected path shapes
    QList<KPathShape*> selectedPathShapes();

    class Private;
    Private * const d;
};

#endif //ARTWORK_VIEW_H

