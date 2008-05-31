/* This file is part of the KDE project
 * Copyright 2008 Stefan Nikolaus <stefan.nikolaus@kdemail.net>
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
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

#include "PrintJob.h"

#include "Canvas.h"
#include "Doc.h"
#include "Map.h"
#include "PrintManager.h"
#include "Sheet.h"
#include "SheetPrint.h"
#include "View.h"

#include <dialogs/SheetSelectPage.h>

#include <KoShapeManager.h>

#include <QPainter>

using namespace KSpread;

class PrintJob::Private
{
public:
    View* view;
    SheetSelectPage* sheetSelectPage;
    QList<Sheet*> selectedSheets;

public:
    int setupPages(const QPrinter& printer, bool forceRecreation = false);
    Sheet* getSheetPageNumber(int* sheetPageNumber) const;
};

int PrintJob::Private::setupPages(const QPrinter& printer, bool forceRecreation)
{
    // Create the page layout.
    KoPageLayout pageLayout;
    const QSizeF size = printer.paperSize(QPrinter::Point);
    pageLayout.format = KoPageFormat::guessFormat(POINT_TO_MM(size.width()), POINT_TO_MM(size.height()));
    pageLayout.orientation = (printer.orientation() == QPrinter::Landscape)
                           ? KoPageFormat::Landscape : KoPageFormat::Portrait;
    pageLayout.width = size.width();
    pageLayout.height = size.height();
    printer.getPageMargins(&pageLayout.left, &pageLayout.top, &pageLayout.right, &pageLayout.bottom,
                           QPrinter::Point);

    // Create the list of sheet, that should be printed.
    selectedSheets.clear();
    if (sheetSelectPage->allSheetsButton->isChecked())
        selectedSheets = view->doc()->map()->sheetList();
    else if (sheetSelectPage->activeSheetButton->isChecked())
        selectedSheets.append(view->activeSheet());
    else if (sheetSelectPage->selectedSheetsButton->isChecked())
    {
        const QStringList sheetNames = sheetSelectPage->selectedSheets();
        for (int i = 0; i < sheetNames.count(); ++i)
        {
            Sheet* sheet = view->doc()->map()->findSheet(sheetNames[i]);
            if (sheet == 0)
            {
                kWarning(36005) << i18n("Sheet %1 could not be found for printing", sheetNames[i]);
                continue;
            }
            selectedSheets.append(sheet);
        }
    }

    // (Re-)Create the pages of the sheets.
    int pageCount = 0;
    for (int i = 0; i < selectedSheets.count(); ++i)
    {
        // Use the defaults from each sheet and use the print dialog's page layout.
        PrintSettings settings = *selectedSheets[i]->printSettings();
        settings.setPageLayout(pageLayout);
        selectedSheets[i]->printManager()->setPrintSettings(settings, forceRecreation);
        pageCount += selectedSheets[i]->printManager()->pageCount();
    }
    return pageCount;
}

Sheet* PrintJob::Private::getSheetPageNumber(int* sheetPageNumber) const
{
    Q_ASSERT(sheetPageNumber);
    // Find the sheet specific page number.
    Sheet* sheet = 0;
    for (int i = 0; i < selectedSheets.count(); ++i)
    {
        sheet = selectedSheets[i];
        if (*sheetPageNumber <= sheet->printManager()->pageCount())
            break;
        *sheetPageNumber -= sheet->printManager()->pageCount();
    }
    return sheet;
}


PrintJob::PrintJob(View *view)
    : KoPrintingDialog(view)
    , d(new Private)
{
    d->view = view;
    d->sheetSelectPage = new SheetSelectPage();

    setShapeManager(static_cast<Canvas*>(d->view->canvas())->shapeManager());

    //apply page layout parameters
    const PrintSettings* settings = d->view->activeSheet()->printSettings();
    const KoPageLayout pageLayout = settings->pageLayout();
    const KoPageFormat::Format pageFormat = pageLayout.format;
    printer().setPaperSize( static_cast<QPrinter::PageSize>( KoPageFormat::printerPageSize( pageFormat ) ) );
    if (pageLayout.orientation == KoPageFormat::Landscape || pageFormat == KoPageFormat::ScreenSize)
        printer().setOrientation( QPrinter::Landscape );
    else
        printer().setOrientation( QPrinter::Portrait );
    printer().setPageMargins(pageLayout.left, pageLayout.top, pageLayout.right, pageLayout.bottom,
                             QPrinter::Point);
    printer().setFullPage( true );

    //kDebug(36005) <<"Iterating through available sheets and initializing list of available sheets.";
    QList<Sheet*> sheetList = d->view->doc()->map()->sheetList();
    for ( int i = sheetList.count()-1; i >= 0; --i )
    {
        Sheet* sheet = sheetList[ i ];
        //kDebug(36005) <<"Adding" << sheet->sheetName();
        d->sheetSelectPage->prependAvailableSheet(sheet->sheetName());
    }

    // Setup the pages.
    // Force the creation of pages.
    const int pageCount = d->setupPages(printer(), true);
    printer().setFromTo(1, pageCount);
}

PrintJob::~PrintJob()
{
//     delete d->sheetSelectPage; // QPrintDialog takes ownership
    delete d;
}

int PrintJob::documentFirstPage() const
{
    return d->selectedSheets.isEmpty() ? 0 : 1;
}

int PrintJob::documentLastPage() const
{
    const QList<Sheet*> sheets = d->selectedSheets;
    int pageCount = 0;
    for (int i = 0; i < sheets.count(); ++i)
        pageCount += sheets[i]->printManager()->pageCount();
    return pageCount;
}

void PrintJob::startPrinting(RemovePolicy removePolicy)
{
    // Setup the pages.
    // No recreation forced, because the sheet contents remained the same since the dialog was created.
    d->setupPages(printer());
    // Start the printing.
    KoPrintingDialog::startPrinting(removePolicy);
}

void PrintJob::preparePage(int pageNumber)
{
    Q_UNUSED(pageNumber)
    const int resolution = printer().resolution();
    const QRectF pageRect = printer().pageRect(QPrinter::Point);
    painter().translate(pageRect.left() * resolution, pageRect.top() * resolution);
    painter().setClipRect(0.0, 0.0, pageRect.width() * resolution, pageRect.height() * resolution);
}

void PrintJob::printPage(int pageNumber, QPainter &painter)
{
    kDebug(36004) << "Printing page" << pageNumber;
    int sheetPageNumber = pageNumber;
    Sheet* sheet = d->getSheetPageNumber(&sheetPageNumber);

    // Print the page.
    if (sheet)
        sheet->printManager()->printPage(sheetPageNumber, painter);

#if 0
    Q_UNUSED(pageNumber);
    // kDebug(36005) <<"Entering KSpread print.";
    //save the current active sheet for later, so we can restore it at the end
    Sheet* selectedsheet = d->view->activeSheet();

    //print all sheets in the order given by the print dialog (Sheet Selection)
    QStringList sheetlist = d->sheetSelectPage->selectedSheets();

    if (sheetlist.empty())
    {
      // kDebug(36005) <<"No sheet for printing selected, printing active sheet";
      sheetlist.append(d->view->activeSheet()->sheetName());
    }

    bool firstpage = true;

    QStringList::iterator sheetlistiterator;
    for (sheetlistiterator = sheetlist.begin(); sheetlistiterator != sheetlist.end(); ++sheetlistiterator)
    {
        // kDebug(36005) <<"  printing sheet" << *sheetlistiterator;
        Sheet* sheet = d->view->doc()->map()->findSheet(*sheetlistiterator);
        if (sheet == 0)
        {
          kWarning(36005) << i18n("Sheet %1 could not be found for printing",*sheetlistiterator);
          continue;
        }

        d->view->setActiveSheet(sheet,false);

        PrintSettings* settings = sheet->printSettings();

        if (firstpage)
          firstpage=false;
        else
        {
          // kDebug(36005) <<" inserting new page";
          printer().newPage();
        }

        if ( d->view->canvasWidget()->editor() )
        {
            d->view->canvasWidget()->deleteEditor( true ); // save changes
        }

        //store the current setting in a temporary variable
        KoPageFormat::Orientation _orient = settings->pageLayout().orientation;

        //use the current orientation from print dialog
        if ( printer().orientation() == QPrinter::Landscape )
        {
            settings->setPageOrientation( KoPageFormat::Landscape );
        }
        else
        {
            settings->setPageOrientation( KoPageFormat::Portrait );
        }

        kDebug() << "QPainter.device() =" << painter.device();

#if 0
        SheetPrint* print = sheet->print();
        bool result = print->print( painter, &printer() );
#else
        PrintManager printManager = (d->view->activeSheet());
        bool result = printManager.print(painter, &printer());
#endif

        //Restore original orientation
        settings->setPageOrientation( _orient );

        // Nothing to print
        if( !result )
        {
            // not required in Qt
            //if( !printer.previewOnly() )
            //{
                KMessageBox::information( 0,
                i18n("Nothing to print for sheet %1.",
                d->view->activeSheet()->sheetName()) );
                //@todo: make sure we really can comment this out,
                //       what to do with partially broken printouts?
//                 printer.abort();
            //}
        }
    }

    d->view->setActiveSheet(selectedsheet);
#endif
}

QList<KoShape*> PrintJob::shapesOnPage(int pageNumber)
{
    kDebug() << "determining shapes on page" << pageNumber;
    int sheetPageNumber = pageNumber;
    Sheet* sheet = d->getSheetPageNumber(&sheetPageNumber);
    if (!sheet)
        return QList<KoShape*>();

    const QRect cellRange = sheet->printManager()->cellRange(sheetPageNumber);
    kDebug() << "sheetPageNumber" << sheetPageNumber;
    kDebug() << "cellRange" << cellRange;
    kDebug() << "pageRange" << sheet->cellCoordinatesToDocument(cellRange);
    kDebug() << "shapeCount" << shapeManager()->shapesAt(sheet->cellCoordinatesToDocument(cellRange)).count();
    return shapeManager()->shapesAt(sheet->cellCoordinatesToDocument(cellRange));
}

QList<QWidget*> PrintJob::createOptionWidgets() const
{
    return QList<QWidget*>() << d->sheetSelectPage;
}
