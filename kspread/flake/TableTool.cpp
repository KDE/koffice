/* This file is part of the KDE project
   Copyright 2006 Stefan Nikolaus <stefan.nikolaus@kdemail.net>

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
#include "TableTool.h"

#include <QApplication>
#include <QGridLayout>
#include <QLabel>
#include <QPainter>
#include <QSpinBox>
#include <QAction>
#include <QToolBar>
#include <QComboBox>
#include <QPushButton>
#include <QListWidget>

#include <kdebug.h>
#include <klineedit.h>
#include <klocale.h>
#include <kicon.h>
#include <kaction.h>
#include <kfiledialog.h>
#include <kpagedialog.h>

#include <KoCanvasBase.h>
#include <KoPointerEvent.h>
#include <KoSelection.h>
#include <KoShapeManager.h>

#include "Cell.h"
#include "Doc.h"
#include "Global.h"
#include "Selection.h"
#include "Sheet.h"
#include "Map.h"

#include "commands/DataManipulators.h"

#include "TableShape.h"

using namespace KSpread;

class TableTool::Private
{
public:
    // If the user is dragging around with the mouse then this tells us what he is doing.
    // The user may want to mark cells or he started in the lower right corner
    // of the marker which is something special. The values for the 2 above
    // methods are called 'Mark' and 'Merge' or 'AutoFill' depending
    // on the mouse button used. By default this variable holds
    // the value 'None'.
    enum { None, Mark, Merge, AutoFill, Resize } mouseAction : 3;

    QString userInput;
    Selection* selection;
    TableShape* tableShape;

public:
    QRectF cellCoordinatesToDocument(const QRect& cellRange) const;
};

// TODO Stefan: Copied from Canvas. Share it somewhere.
QRectF TableTool::Private::cellCoordinatesToDocument( const QRect& cellRange ) const
{
    register Sheet * const sheet = tableShape->sheet();
    if (!sheet)
        return QRectF();

    QRectF rect;
    rect.setLeft  ( sheet->columnPosition( cellRange.left() ) );
    rect.setRight ( sheet->columnPosition( cellRange.right() + 1 ) );
    rect.setTop   ( sheet->rowPosition( cellRange.top() ) );
    rect.setBottom( sheet->rowPosition( cellRange.bottom() + 1 ) );
    return rect;
}


TableTool::TableTool( KoCanvasBase* canvas )
    : KoTool( canvas )
    , d( new Private )
{
    d->selection = new Selection(this);
    connect(d->selection, SIGNAL(changed(const Region&)), this, SLOT(changeSelection(const Region&)));
    d->tableShape = 0;

    KAction* importAction = new KAction(KIcon("file-import"), i18n("Import OpenDocument Spreadsheet File"), this);
    importAction->setIconText(i18n("Import"));
    addAction("import", importAction);
    connect(importAction, SIGNAL(triggered()), this, SLOT(importDocument()));

    KAction* exportAction = new KAction(KIcon("file-export"), i18n("Export OpenDocument Spreadsheet File"), this);
    exportAction->setIconText(i18n("Export"));
    addAction("export", exportAction);
    connect(exportAction, SIGNAL(triggered()), this, SLOT(exportDocument()));
}

TableTool::~TableTool()
{
    delete d->selection;
    delete d;
}

void TableTool::importDocument()
{
    QString file = KFileDialog::getOpenFileName(KUrl(), "application/vnd.oasis.opendocument.spreadsheet", 0, "Import");
    if (file.isEmpty()) return;
    d->tableShape->doc()->setModified(false);
    d->tableShape->doc()->import(file);
}

void TableTool::exportDocument()
{
    QString file = KFileDialog::getSaveFileName(KUrl(), "application/vnd.oasis.opendocument.spreadsheet", 0, "Export");
    if (file.isEmpty()) return;
    d->tableShape->doc()->exp0rt(file);
}

void TableTool::paint( QPainter& painter, const KoViewConverter& viewConverter )
{
    KoShape::applyConversion(painter, viewConverter);

    // get the transparent selection color
    QColor selectionColor(QApplication::palette().highlight().color());
    selectionColor.setAlpha(127);

    // draw the transparent selection background
    const QRectF rect = d->cellCoordinatesToDocument(d->selection->lastRange());
    painter.fillRect(rect.translated(d->tableShape->position()), selectionColor);
}

// TODO Stefan: Taken from DefaultTool. Share it somewhere.
void TableTool::mousePressEvent(KoPointerEvent* event)
{
    if (!d->tableShape->boundingRect().contains(event->point))
        return;
    const QPointF position = event->point - d->tableShape->position();
    Sheet* const sheet = d->tableShape->sheet();

    // In which cell did the user click ?
    double xpos;
    double ypos;
    int col = sheet->leftColumn(position.x(), xpos);
    int row = sheet->topRow(position.y(), ypos);
    // you cannot move marker when col > KS_colMax or row > KS_rowMax
    if (col > KS_colMax || row > KS_rowMax)
        return;

    // Extending an existing selection with the shift button ?
    if ((event->modifiers() & Qt::ShiftModifier) && sheet->doc()->isReadWrite() &&
         !d->selection->isColumnOrRowSelected())
    {
        d->selection->update(QPoint(col, row));
        return;
    }

    switch (event->button())
    {
        case Qt::LeftButton:
        {
            if (event->modifiers() & Qt::ControlModifier)
            {
                // Start a marking action
                d->mouseAction = Private::Mark;
                // extend the existing selection
                d->selection->extend(QPoint(col, row), sheet);
            }
            else
            {
                // Start a marking action
                d->mouseAction = Private::Mark;
                // reinitialize the selection
                d->selection->initialize(QPoint(col, row), sheet);
            }
            break;
        }
        case Qt::MidButton:
        {
        }
        case Qt::RightButton:
        {
            if (!d->selection->contains(QPoint(col, row)))
            {
                // No selection or the mouse press was outside of an existing selection?
                d->selection->initialize(QPoint(col, row), sheet);
            }
            break;
        }
        default:
            break;
    }
    d->tableShape->repaint();
}

void TableTool::mouseMoveEvent( KoPointerEvent* )
{
}

void TableTool::mouseReleaseEvent( KoPointerEvent* )
{
}

void TableTool::activate( bool temporary )
{
    Q_UNUSED( temporary );

    KoSelection* selection = m_canvas->shapeManager()->selection();
    foreach ( KoShape* shape, selection->selectedShapes() )
    {
        d->tableShape = dynamic_cast<TableShape*>( shape );
        if ( d->tableShape )
            break;
    }
    if ( !d->tableShape )
    {
        emit done();
        return;
    }
    d->selection->setActiveSheet(d->tableShape->sheet());
    d->selection->setOriginSheet(d->tableShape->sheet());
    useCursor( Qt::ArrowCursor, true );
    d->tableShape->repaint();
}

void TableTool::deactivate()
{
    d->tableShape = 0;
}

void TableTool::changeColumns( int num )
{
    d->tableShape->setColumns( num );
    d->tableShape->repaint();
}

void TableTool::changeRows( int num )
{
    d->tableShape->setRows( num );
    d->tableShape->repaint();
}

void TableTool::changeSelection(const Region& changedRegion)
{
    const Cell cell(d->tableShape->sheet(), d->selection->marker());
    emit userInputChanged(cell.userInput());
}

void TableTool::changeUserInput(const QString& content)
{
    d->userInput = content;
}

void TableTool::applyUserInput()
{
    if( ! d->tableShape )
        return;
    DataManipulator* manipulator = new DataManipulator();
    manipulator->setSheet(d->tableShape->sheet());
    manipulator->setValue(Value(d->userInput));
    manipulator->setParsing(true);
    manipulator->setExpandMatrix(false);
    manipulator->add(*d->selection);
    manipulator->execute();
}

void TableTool::sheetsBtnClicked()
{
    KPageDialog* dialog = new KPageDialog();
    dialog->setFaceType(KPageDialog::Plain);
    dialog->setCaption(i18n("Sheets"));
    QWidget *w = new QWidget();
    QHBoxLayout* l = new QHBoxLayout(w);
    w->setLayout(l);
    l->setMargin(0);
    QListWidget* list = new QListWidget(w);
    l->addWidget(list);
    dialog->setMainWidget(w);
    Map *map = d->tableShape->doc()->map();
    foreach(Sheet* s, map->sheetList())
        list->addItem(s->sheetName());
    QVBoxLayout* btnlayout = new QVBoxLayout(w);
    QPushButton* addbtn = new QPushButton(KIcon("edit-add"), i18n("Add"), w);
    addbtn->setEnabled(false);
    btnlayout->addWidget(addbtn);
    QPushButton* rembtn = new QPushButton(KIcon("edit-delete"), i18n("Remove"), w);
    rembtn->setEnabled(false);
    btnlayout->addWidget(rembtn);
    btnlayout->addStretch(1);
    l->addLayout(btnlayout);
    if( dialog->exec() == QDialog::Accepted ) {
        //TODO
    }
    delete dialog;
}

QWidget* TableTool::createOptionWidget()
{
    QWidget* optionWidget = new QWidget();
    QVBoxLayout* l = new QVBoxLayout( optionWidget );
    l->setMargin(0);
    optionWidget->setLayout(l);

    QGridLayout* layout = new QGridLayout( optionWidget );
    l->addLayout(layout);

    QLabel* label = 0;
    QSpinBox* spinBox = 0;

    QHBoxLayout* sheetlayout = new QHBoxLayout(optionWidget);
    sheetlayout->setMargin(0);
    sheetlayout->setSpacing(3);
    layout->addLayout(sheetlayout, 0, 1);
    QComboBox* sheetComboBox = new QComboBox(optionWidget);
    sheetlayout->addWidget(sheetComboBox, 1);
    Map *map = d->tableShape->doc()->map();
    foreach(Sheet* s, map->sheetList()) {
        sheetComboBox->addItem(s->sheetName());
        //sheetComboBox->setCurrentIndex( sheetComboBox->count()-1 );
    }
    QPushButton *sheetbtn = new QPushButton(KIcon("table"), QString(), optionWidget);
    sheetbtn->setFixedHeight( sheetComboBox->sizeHint().height() );
    connect(sheetbtn, SIGNAL(clicked()), this, SLOT(sheetsBtnClicked()));
    sheetlayout->addWidget(sheetbtn);
    label = new QLabel(i18n("Sheet:"), optionWidget);
    label->setBuddy(sheetComboBox);
    label->setToolTip(i18n("Selected Sheet"));
    layout->addWidget(label, 0, 0);

    KLineEdit* lineEdit = new KLineEdit(optionWidget);
    layout->addWidget(lineEdit, 1, 1);
    connect(lineEdit, SIGNAL(editingFinished()), this, SLOT(applyUserInput()));
    connect(lineEdit, SIGNAL(textChanged(const QString&)), this, SLOT(changeUserInput(const QString&)));
    connect(this, SIGNAL(userInputChanged(const QString&)), lineEdit, SLOT(setText(const QString&)));
    changeSelection(Region()); // initialize the lineEdit with the cell content

    label = new QLabel(i18n("Content:"), optionWidget);
    label->setBuddy(lineEdit);
    label->setToolTip(i18n("Cell content"));
    layout->addWidget(label, 1, 0);

    spinBox = new QSpinBox( optionWidget );
    spinBox->setRange( 1, KS_colMax );
    spinBox->setValue( d->tableShape->columns() );
    layout->addWidget( spinBox, 2, 1 );
    connect( spinBox, SIGNAL( valueChanged(int) ), this, SLOT( changeColumns(int) ) );

    label = new QLabel( i18n( "Columns:" ), optionWidget );
    label->setBuddy( spinBox );
    label->setToolTip( i18n( "Number of columns" ) );
    layout->addWidget( label, 2, 0 );

    spinBox = new QSpinBox( optionWidget );
    spinBox->setRange( 1, KS_rowMax );
    spinBox->setValue( d->tableShape->rows() );
    layout->addWidget( spinBox, 3, 1 );
    connect( spinBox, SIGNAL( valueChanged(int) ), this, SLOT( changeRows(int) ) );

    label = new QLabel( i18n( "Rows:" ), optionWidget );
    label->setBuddy( spinBox );
    label->setToolTip( i18n( "Number of rows" ) );
    layout->addWidget( label, 3, 0 );

//layout->setColumnStretch( 1, 1 );
    layout->setRowStretch( 4, 1 );

    QToolBar* tb = new QToolBar(optionWidget);
    l->addWidget(tb);
    tb->setMovable(false);
    tb->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    tb->addAction( action("import") );
    tb->addAction( action("export") );

    return optionWidget;
}

#include "TableTool.moc"
