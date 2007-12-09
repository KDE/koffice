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

#ifndef __KARBON_VIEW__
#define __KARBON_VIEW__

#include <KoView.h>
#include <QPointF>
#include <kxmlguibuilder.h>
#include <KoUnit.h>
#include <KoShapeAlignCommand.h>
#include <KoShapeDistributeCommand.h>
#include <KoZoomMode.h>
#include <karbon_export.h>
#include "KarbonBooleanCommand.h"

class QLabel;
class QDropEvent;
class QResizeEvent;
class QRectF;
class QPrinter;

class KAction;
class KarbonPart;
class KSelectAction;
class KToggleAction;

class KoContextHelpAction;
class KoCanvasController;
class KoRuler;
class KoZoomController;

class VDocumentTab;
class VColorDocker;
class VStyleDocker;
class KarbonTransformDocker;
class KarbonLayerDocker;

class VPainterFactory;
class KarbonCanvas;
class KarbonStylePreviewDocker;

class KARBONCOMMON_EXPORT KarbonView : public KoView, public KXMLGUIBuilder
{
    Q_OBJECT

public:
    explicit KarbonView( KarbonPart* part, QWidget* parent = 0 );
    virtual ~KarbonView();

    KarbonPart *part() const { return m_part; }

    void dropEvent( QDropEvent *e );

    KarbonCanvas* canvasWidget() const { return m_canvas; }

    KoContextHelpAction* contextHelpAction() const { return m_contextHelpAction; }

    void reorganizeGUI();
    void setNumberOfRecentFiles( unsigned int number );

    QLabel* statusMessage() const { return m_status; }

    void setCursor( const QCursor & );

public slots:
    // editing:
    void editCut();
    void editCopy();
    void editPaste();
    void editSelectAll();
    void editDeselectAll();
    void editDeleteSelection();

    void selectionDuplicate();
    void selectionBringToFront();
    void selectionSendToBack();
    void selectionMoveUp();
    void selectionMoveDown();
    void selectionAlignHorizontalLeft();
    void selectionAlignHorizontalCenter();
    void selectionAlignHorizontalRight();
    void selectionAlignVerticalTop();
    void selectionAlignVerticalCenter();
    void selectionAlignVerticalBottom();

    void selectionDistributeHorizontalCenter();
    void selectionDistributeHorizontalGap();
    void selectionDistributeHorizontalLeft();
    void selectionDistributeHorizontalRight();
    void selectionDistributeVerticalCenter();
    void selectionDistributeVerticalGap();
    void selectionDistributeVerticalBottom();
    void selectionDistributeVerticalTop();

    void fileImportGraphic();

    void groupSelection();
    void ungroupSelection();

    void closePath();
    void combinePath();
    void separatePath();
    void reversePath();

    void intersectPaths();
    void subtractPaths();
    void unitePaths();

    void configure();

    void pageLayout();

    void selectionChanged();

    void togglePageMargins(bool);
    void showRuler();
    void showGrid();
    void snapToGrid();

    void showSelectionPopupMenu( const QPoint &pos );

protected slots:
    // Object related operations.

    // View.
    void viewModeChanged();

    /// Called by the zoom action to set the zoom
    void zoomChanged( KoZoomMode::Mode mode, double zoom );

    void mousePositionChanged( const QPoint &position );
    void pageOffsetChanged();

    void documentViewRectChanged( const QRectF &viewRect );

    void updateUnit( KoUnit unit );
signals:
    void selectionChange();
    void pageLayoutChanged();

protected:
    virtual void updateReadWrite( bool readwrite );
    virtual void resizeEvent( QResizeEvent* event );

    void createDocumentTabDock();
    void createLayersTabDock();
    void createStrokeDock();
    void createColorDock();
    void createTransformDock();
    void createResourceDock();

    void addSelectionToClipboard() const;
    virtual KoPrintJob * createPrintJob();

private:
    void initActions();
    void updateRuler();

    void selectionAlign(KoShapeAlignCommand::Align align);
    void selectionDistribute(KoShapeDistributeCommand::Distribute distribute);

    void booleanOperation( KarbonBooleanCommand::BooleanOperation operation );

    /// Returns a list of all selected path shapes
    QList<KoPathShape*> selectedPathShapes();

    KarbonPart        *m_part;
    KarbonCanvas        *m_canvas;
    KoCanvasController    *m_canvasController;
    KoRuler            *m_horizRuler;
    KoRuler            *m_vertRuler;

    KarbonStylePreviewDocker * m_stylePreview;

    KAction            *m_groupObjects;
    KAction            *m_ungroupObjects;

    KAction            *m_closePath;
    KAction            *m_combinePath;
    KAction            *m_separatePath;
    KAction * m_reversePath;
    KAction * m_intersectPath;
    KAction * m_subtractPath;
    KAction * m_unitePath;

    // actions:
    KSelectAction        *m_viewAction;
    KAction                *m_configureAction;
    KToggleAction        *m_showRulerAction;
    KToggleAction        *m_showGridAction;
    KToggleAction        *m_snapGridAction;
    KToggleAction        *m_showPageMargins;
    KoContextHelpAction    *m_contextHelpAction;
    KAction                *m_deleteSelectionAction;

    //dockers
    VDocumentTab        *m_DocumentTab;
    VColorDocker        *m_ColorManager;
    VStyleDocker        *m_styleDocker;
    KarbonTransformDocker *m_TransformDocker;
    KarbonLayerDocker        *m_layerDocker;

    //Status Bar
    QLabel                *m_status;       // Ordinary status
    QLabel                *m_cursorCoords; // Cursor coordinates

    KoZoomController * m_zoomController;
};

#endif

