/***************************************************************************
              mreportengine.h  -  Kugar report engine
              -------------------
    begin     : Sun Aug 15 1999                                           
    copyright : (C) 1999 by Mutiny Bay Software                         
    email     : info@mutinybaysoftware.com                                     
 ***************************************************************************/

#ifndef MREPORTENGINE_H
#define MREPORTENGINE_H

#include <qobject.h>
#include <qpicture.h>
#include <kprinter.h>
#include <qpaintdevicemetrics.h>
#include <qsize.h>
#include <qarray.h>
#include <qdom.h>

#include "mlineobject.h"
#include "mlabelobject.h"
#include "mspecialobject.h"
#include "mfieldobject.h"
#include "mreportsection.h"
#include "mreportdetail.h"
#include "mpagecollection.h"

/**Kugar report engine
  *@author Mutiny Bay Software
  */

class MReportEngine : public QObject
{
	Q_OBJECT

public:
	enum PageOrientation { Portrait, Landscape };

	enum PageSize {
		A4, B5, Letter, Legal, Executive,
		A0, A1, A2, A3, A5, A6, A7, A8, A9, B0, B1,
		B10, B2, B3, B4, B6, B7, B8, B9, C5E, Comm10E,
		DLE, Folio, Ledger, Tabloid, NPageSize
	};

	MReportEngine();
	MReportEngine(const MReportEngine& mReportEngine);
	MReportEngine operator=(const MReportEngine& mReportEngine);
	virtual ~MReportEngine();

	bool setReportData(const QString &);
	bool setReportData(QIODevice *);
	bool setReportTemplate(const QString &);
	bool setReportTemplate(QIODevice *);
	int getRenderSteps() {return records.length() / 2;}
	MPageCollection* renderReport();

public slots:
	void slotCancelRendering();

signals:
	void signalRenderStatus(int);	
	void preferedTemplate(const QString &);

private:
  /** Report data document */
  QDomDocument rd;
  /** Report template document */
  QDomDocument rt;

  /** Report painter */
  QPainter p;

  /** Report page size */
  int pageSize;
  /** Report page orientation */
  int pageOrientation;
  /** Report top margin */
  int topMargin;
  /** Report top margin */
  int bottomMargin;
  /** Report top margin */
  int leftMargin;
  /** Report top margin */
  int rightMargin;

  /** Report page width */
  int pageWidth;
  /** Report page height */
  int pageHeight;

  /** Report header */
  MReportSection rHeader;
  /** Page header */
  MReportSection pHeader;
  /** Detail section */
  MReportDetail detail;
  /** Page footer */
  MReportSection pFooter;
  /** Report footer */
  MReportSection rFooter;

  /** Current y position on page */
  int currY;
  /** Current height of page excluding margins */
  int currHeight;
  /** Current page number */
  int currPage;
  /** Current report date */
  QDate currDate;

  /** Grand total array */
  QList<QArray<double> > grandTotal;

	/** Cancel rendering flag */
	bool cancelRender;

private:
  // The set of records being rendered.
  QDomNodeList records;
  /** Clears report formatting */
  void clearFormatting();
  /** Starts a new page of the report */
  void startPage(MPageCollection* pages);
  /** Finishes the current page of the report */
  void endPage(MPageCollection* pages);
  /** Finishes the current page and adds a new page */
  void newPage(MPageCollection* pages);
  /** Draws the report header object to the current page */
  void drawReportHeader(MPageCollection* pages);
  /** Draws the page header to the current page */
  void drawPageHeader(MPageCollection* pages);
  /** Draws the page footer to the current page */
  void drawPageFooter(MPageCollection* pages);
  /** Draws the report footer to the current page */
  void drawReportFooter(MPageCollection* pages);

  /** Gets the metrics for the selected page size */
  QSize getPageMetrics(int size, int orientation);

  void initData();
  void initTemplate();

  /** Sets the main layout attributes for the report */
  void setReportAttributes(QDomNode* report);
  /** Sets the layout attributes for the given report section */
  void setSectionAttributes(MReportSection* section, QDomNode* report);
  /** Sets the layout attributes for the detail section */
  void setDetailAttributes(QDomNode* report);
	/** Sets a line's layout attributes */
  void setLineAttributes(MLineObject* line, QDomNamedNodeMap* attr);
  /** Sets a label's layout attributes */
  void setLabelAttributes(MLabelObject* label, QDomNamedNodeMap* attr);
  /** Sets a special field's layout attributes */
  void setSpecialAttributes(MSpecialObject* field, QDomNamedNodeMap* attr);
  /** Sets a field's layout attributes */
  void setFieldAttributes(MFieldObject* field, QDomNamedNodeMap* attr);
  /** Sets a calculated field's layout attributes */
  void setCalculatedFieldAttributes(MCalcObject* field, QDomNamedNodeMap* attr);

  /** Copies member data from one object to another.
    * Used by the copy constructor and assignment operator
    */
  void copy(const MReportEngine* mReportEngine);
};

#endif
