/***************************************************************************
              mreportviewer.cpp  -  Kugar report viewer widget
              -------------------
    begin     : Fri Aug 13 1999                                           
    copyright : (C) 1999 by Mutiny Bay Software                         
    email     : info@mutinybaysoftware.com                                     
 ***************************************************************************/

#include <qapplication.h>

#include "mreportviewer.h"


/** Constructor */
MReportViewer::MReportViewer(QWidget *parent, const char *name ) : QWidget(parent,name) {
	// Create the scrollview
  scroller = new QScrollView(this);

  // Create the report engine
  rptEngine = new MReportEngine();
  report = 0;

	// Connect the rendering update signal and slot
	connect(rptEngine,SIGNAL(signalRenderStatus(int)),
			  SLOT(slotRenderProgress(int)));

	connect(rptEngine,SIGNAL(preferedTemplate(const QString &)),
			  SIGNAL(preferedTemplate(const QString &)));

  // Get the current color palette
  QPalette p = palette();
  QColorGroup g = p.normal();

  // Set the scroller's background color
  scroller->viewport()->setBackgroundColor(g.mid());

  // Create the report display widget
  display = new MPageDisplay(scroller->viewport());

  // Set the display's default background color
  display->setBackgroundColor(white);

  // Add the display to the scrollview
  scroller->addChild(display);

  //Hide the display, we don't have a report yet ..
  display->hide();
}

/** Destructor */
MReportViewer::~MReportViewer(){
  clearReport();
  delete rptEngine;
}

/** Report viewer's paint event */
void MReportViewer::paintEvent(QPaintEvent* event) {		
}

/** Report viewer's resize event */
void MReportViewer::resizeEvent(QResizeEvent* event){
  scroller->resize(event->size());
}


// Set the report's data from an in-line string.

bool MReportViewer::setReportData(const QString &data)
{
	return rptEngine -> setReportData(data);
}


// Set the report's data from an i/o device.

bool MReportViewer::setReportData(QIODevice *dev)
{
	return rptEngine -> setReportData(dev);
}


// Set the report's template from an in-line string.

bool MReportViewer::setReportTemplate(const QString &tpl)
{
	return rptEngine -> setReportTemplate(tpl);
}


// Set the report's template from an i/o device.

bool MReportViewer::setReportTemplate(QIODevice *dev)
{
	return rptEngine -> setReportTemplate(dev);
}


/** Generates the report's page collection */
bool MReportViewer::renderReport()
{
  // Check if a previous report exists and
  // if so de-allocated it
  if(report != 0)
    delete report;

  // Render the report
  report = rptEngine->renderReport();

  // Display the first page of the report
  if(report != 0 && report->getFirstPage() != 0){
    display->setPageDimensions(report->pageDimensions());
    display->setPage(report->getFirstPage());
    display->show();

    return true;
  }

  return false;
}

/** Clears the report's page collection */
void MReportViewer::clearReport(){
  // Hide the display
  display->hide();

  // De-Allocate any report
  if (report)
  {
  	delete report;
  	report = 0;
  }
}

/** Prints the rendered report to the selected printer - displays Qt print dialog */
void MReportViewer::printReport(){
  // Check for a report
	if(report == 0) return;

	// Get the page count
	int cnt = report->pageCount();

	// Check if there is a report or any pages to print
	if(cnt == 0) {
    qWarning(QObject::tr("There are no pages in the\nreport to print."));
    return;
  }

  // Set the printer dialog
	printer = new QPrinter();
  printer->setPageSize((QPrinter::PageSize)report->pageSize());
  printer->setOrientation((QPrinter::Orientation)report->pageOrientation());
	printer->setMinMax(1, cnt);
	printer->setFromTo(1, cnt);
  printer->setFullPage(true);

  if (printer->setup(this)) {
		QPicture* page;
  	QPainter painter;
		bool printRev;

		// Save the viewer's page index
		int viewIdx = report->getCurrentIndex();

		// Check the order we are printing the pages
		if (printer->pageOrder() == QPrinter::FirstPageFirst)
			printRev = false;
		else
    	printRev = true;

		// Get the count of pages and copies to print
		int printFrom = printer->fromPage() - 1;
		int printTo = printer->toPage();		
		int printCnt = (printTo - printFrom);
		int printCopies = printer->numCopies();
		int totalSteps = printCnt * printCopies;
		int currentStep = 1;

		// Set copies to 1, QPrinter copies does not appear to work ...
		printer->setNumCopies(1);

		// Setup the progress dialog
		QProgressDialog progress( QObject::tr("Printing report..."),
															QObject::tr("Cancel"),
		          								totalSteps, this, "progress", true );
		progress.setMinimumDuration(M_PROGRESS_DELAY);
		QObject::connect(&progress, SIGNAL(cancelled()), this, SLOT(slotCancelPrinting()));
		progress.setProgress(0);	
		qApp->processEvents();

		// Start the printer
   	painter.begin(printer);

		// Print each copy
		for(int j = 0; j < printCopies; j++){
      // Print each page in the collection
      for(int i = printFrom ; i < printTo; i++, currentStep++){
  			if(!printer->aborted()){
  				progress.setProgress(currentStep);
  				qApp->processEvents();

					if (printRev)
  					report->setCurrentPage((printCnt == 1) ? i : (printCnt - 1) - i);
					else
						report->setCurrentPage(i);

        	page = report->getCurrentPage();
        	page->play(&painter);
        	if(i < printCnt - 1)
          	printer->newPage();
  			}
  			else{
					j = printCopies;
  				break;
				}
      }
			if(j < printCopies - 1)
				printer->newPage();
		}

		// Cleanup printing
		setCursor(arrowCursor);
    painter.end();
    report->setCurrentPage(viewIdx);
  }
	delete printer;
}

/** Shows the first page in the report */
void MReportViewer::slotFirstPage(){
  QPicture* page;

	if(report == 0) return;

  if((page = report->getFirstPage()) != 0){
    display->setPage(page);
    display->repaint();
  }
}

/** Shows the next page in the report */
void MReportViewer::slotNextPage(){
  QPicture* page;

	if(report == 0) return;

  int index = report->getCurrentIndex();

  if((page = report->getNextPage()) != 0){
    display->setPage(page);
    display->repaint();
  }
  else
    report->setCurrentPage(index);
}

/** Shows the prevoius page in the report */
void MReportViewer::slotPrevPage(){
  QPicture* page;

	if(report == 0) return;

  int index = report->getCurrentIndex();

  if((page = report->getPreviousPage()) != 0){
    display->setPage(page);
    display->repaint();
  }
  else
    report->setCurrentPage(index);
}

/** Shows the last page in the report */
void MReportViewer::slotLastPage(){
  QPicture* page;

	if(report == 0) return;

  if((page = report->getLastPage()) != 0){
    display->setPage(page);
    display->repaint();
  }
}

/** Cancel printing of the report */
void MReportViewer::slotCancelPrinting(){
	printer->abort();
}

/** Updates rendering progress */
void MReportViewer::slotRenderProgress(int p){
	static QProgressDialog* progress;
	static int totalSteps;

	// Check if the dialog was created
	if (progress == 0){
		totalSteps = rptEngine->getRenderSteps();
		progress = new QProgressDialog( QObject::tr("Creating report..."), QObject::tr("Cancel"),
                                    totalSteps, this, "progress", true );
		progress->setMinimumDuration(M_PROGRESS_DELAY);	
	}

	// Update the dialog
	progress->setProgress(p);
	qApp->processEvents();

	// Check if the action was canceled
	if (progress->wasCancelled()){
		progress->setProgress(totalSteps);
    rptEngine->slotCancelRendering();
	}

	// Cleanup dialog if necessary
	if (progress->progress() == -1)
		delete progress;
}


// Return the preferred size.

QSize MReportViewer::sizeHint() const
{
	return scroller -> sizeHint();
}
