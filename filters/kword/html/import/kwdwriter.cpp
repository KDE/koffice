/***************************************************************************
                          kwdwriter.cpp  -  description
                             -------------------
    begin                : Wed Sep 5 2001
    copyright            : (C) 2001 by Frank Dekervel
    email                : Frank.Dekervel@student.kuleuven.ac.be
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License           *
 *   as published by                                                       *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "kwdwriter.h"
#include <stdlib.h>
#include "qrect.h"

KWDWriter::KWDWriter(KoStore *store){
	_store=store;
	_doc= new QDomDocument("DOC");

	_doc->appendChild( _doc->createProcessingInstruction( "xml", "version=\"1.0\" encoding=\"UTF-8\"" ) );
	tableNo=1;
	QDomElement kwdoc = _doc->createElement( "DOC" );
	kwdoc.setAttribute( "editor", "RTF Import Filter" );
	kwdoc.setAttribute( "mime", "application/x-kword" );
	_doc->appendChild( kwdoc );

    QDomElement paper = _doc->createElement( "PAPER" );
    kwdoc.appendChild( paper );
    paper.setAttribute( "format", 1 );
    paper.setAttribute( "width", 595 );
    paper.setAttribute( "height", 841 );
    paper.setAttribute( "orientation", 0 );
    paper.setAttribute( "columns", 1 );
    paper.setAttribute( "columnspacing", 3 );
    paper.setAttribute( "hType", 0 );
    paper.setAttribute( "fType", 0 );

    QDomElement borders = _doc->createElement( "PAPERBORDERS" );
    paper.appendChild( borders );
    borders.setAttribute( "left", 20 );
    borders.setAttribute( "top", 10 );
    borders.setAttribute( "right", 10 );
    borders.setAttribute( "bottom", 10 );

    QDomElement docattrs = _doc->createElement( "ATTRIBUTES" );
    kwdoc.appendChild( docattrs );
    docattrs.setAttribute( "processing", 0 );
    docattrs.setAttribute( "standardpage", 1 );
    docattrs.setAttribute( "hasHeader", 0 );
    docattrs.setAttribute( "hasFooter", 0 );
    docattrs.setAttribute( "unit", "mm" );

    QDomElement framesets = _doc->createElement("FRAMESETS");
    kwdoc.appendChild(framesets);
    QDomElement rootframeset = addFrameSet(framesets);
    QDomElement mainframe= addFrame(rootframeset);

    QDomElement styles=_doc->createElement("STYLES");
    kwdoc.appendChild(styles);

    QDomElement standard=_doc->createElement("STYLE");
    styles.appendChild(standard);

    QDomElement  tmp;
    	tmp=_doc->createElement("NAME");
    	tmp.setAttribute("value","Standard");
    	standard.appendChild(tmp);
    	
    	tmp=_doc->createElement("FOLLOWING");
    	tmp.setAttribute("name","Standard");
    	standard.appendChild(tmp);
    	QDomElement fmt;
    	
    	fmt=_doc->createElement("FORMAT");
    	fmt.setAttribute("id","1");
    	standard.appendChild(fmt);
    	
    	tmp=_doc->createElement("SIZE");
    	tmp.setAttribute("value","12"); // HACK !
    	fmt.appendChild(tmp);


}

int KWDWriter::createTable() {
 return tableNo++;
}


QDomElement KWDWriter::createInline(QDomElement paragraph, QDomElement toInline) {
 if (toInline.tagName() == "FRAMESET") {
     formatAttribute(paragraph,"ANCHOR","type","frameset");
     // fixme: support other kinds of inlines.
 }
 if (!toInline.attribute("grpMgr").isEmpty()) {
	 formatAttribute(paragraph,"ANCHOR","instance",toInline.attribute("grpMgr"));
 }
 addText(paragraph,"#",6); // the anchor.
}

QDomElement KWDWriter::currentLayout(QDomElement paragraph) {
	return paragraph.elementsByTagName("LAYOUT").item(0).toElement();
}


QDomElement KWDWriter::createTableCell(int tableno, int nrow,
				int ncol, int colspan, int x, int y, int w, int h) {
	QDomElement parent=docroot().elementsByTagName("FRAMESETS").item(0).toElement();

	QDomElement fs=addFrameSet(parent,1,0,
			QString("Table %1 - %2,%3").arg(tableno).arg(nrow).arg(ncol),
			1);
	fs.setAttribute("grpMgr",QString("Table %1").arg(tableno));
	fs.setAttribute("row",nrow);
	fs.setAttribute("col",ncol);
	fs.setAttribute("cols",colspan); // FIXME do colspan in finishTable
					 // so we don't have to give it as an argument
	fs.setAttribute("rows",1);	// FIXME support rowspan ?
	addFrame(fs,0/*runaround*/,0/*copy*/,
		y,x,h+y,w+x); // FIXME
	return fs;
}

QDomElement KWDWriter::fetchTableCell(int tableno, int rowno, int colno) {
	QDomNodeList e=docroot().elementsByTagName("FRAMESET");
	for (unsigned int i=0;i<e.count();i++) {
	     QDomElement k=e.item(i).toElement();
	     if (k.attribute("grpMgr") == QString("Table %1").arg(tableno))
		     if (k.attribute("row") == QString("%1").arg(rowno))
		     	if (k.attribute("col") == QString("%1").arg(colno))
		     		return k;
	}
	QDomElement dummy;
	return dummy;
}

#define MAX(x,y) ((x > y) ? x : y)
#define MIN(x,y) ((x < y) ? x : y)

void KWDWriter::finishTable(int tableno,int x, int y, int w, int h) {
	int ncols=0;
	int nrows=0;
	QDomNodeList nl=docroot().elementsByTagName("FRAMESET");
	   //FIXME calculate nrows and stuff.
	   //and add empty cells for missing ones.
	
	// first, see how big the table is (cols & rows)
	for (unsigned i=0;i<nl.count();i++) {
	    QDomElement k=nl.item(i).toElement();
	    	if (k.attribute("grpMgr") == QString("Table %1").arg(tableno)) {
	    	  ncols=MAX(ncols,k.attribute("col").toInt()+1);
	    	  nrows=MAX(nrows,k.attribute("row").toInt()+1);	    	
	    	}
	}
	int curcol=0;
	int currow=0;
	int currow_inc=0;
	if (ncols == 0) ncols=1; // FIXME (floating point division by zero)
	if (nrows == 0) nrows=1;
	
	int step_x=(w-x)/ncols;
	int step_y=(h-y)/nrows;
	
	
	// then, let's create the missing cells and resize them if needed.
	bool must_resize=false;
	if (w>0) must_resize=true;
	while (currow < nrows) {
	   curcol=0;
	   while (curcol < ncols) {
	      QDomElement e=fetchTableCell(tableno,currow,curcol);
	      if (e.isNull()) {
	              // a missing cell !
	              qWarning(QString("creating %1 %2").arg(currow).arg(curcol).latin1());
	              createTableCell(tableno,currow,curcol,1,x+step_x*curcol,y+step_y*currow,step_x,step_y);
	              // fixme: what to do if we don't have to resize ?
	      }
	
	      // resize this one FIXME optimize this routine
	      if (must_resize == true) {
	      QDomElement ee=e.firstChild().toElement(); // the frame in the frameset
	          int cs=e.attribute("cols").toInt();
	          int rs=e.attribute("cols").toInt();
	          qWarning("resizing");
	          addRect(ee,x+step_x*curcol,0,step_x*cs,step_y*rs);
	      }
	      if (curcol==0) currow_inc=e.attribute("rows").toInt();
	      curcol +=e.attribute("cols").toInt();

	      	
	   }
	   currow+=currow_inc;
	}
	
	
}


QDomElement KWDWriter::addFrameSet(QDomElement parent, int frametype,
				   int frameinfo, QString name, int visible) {
	QDomElement frameset=_doc->createElement("FRAMESET");
	parent.appendChild(frameset);
	frameset.setAttribute("frameType",frametype);
	frameset.setAttribute("frameInfo",frameinfo);
	if (!name.isNull())
		frameset.setAttribute("name",name);
	else
		frameset.setAttribute("name","Text-frameset 1");
	frameset.setAttribute("visible",visible);
	return frameset;
}

QDomElement KWDWriter::addParagraph(QDomElement parent) {
	QDomElement k;
	return addParagraph(parent,k);
}

QDomElement KWDWriter::addParagraph(QDomElement parent, QDomElement layoutToClone) {

	QDomElement paragraph=_doc->createElement("PARAGRAPH");
	QDomElement formats=_doc->createElement("FORMATS");
	QDomElement layout;
	if (layoutToClone.isNull()) {
		layout=_doc->createElement("LAYOUT");
	}
	else {
		layout=layoutToClone.cloneNode().toElement();
	}
	//QDomElement format=_doc->createElement("FORMAT");
	QDomElement text=_doc->createElement("TEXT");
	QDomText t=_doc->createTextNode(QString(""));
	text.appendChild(t);
	paragraph.appendChild(formats);
	//formats.appendChild(format);
	paragraph.appendChild(text);
	parent.appendChild(paragraph);
	paragraph.appendChild(layout);
	return paragraph;
}

QDomElement KWDWriter::formatAttribute(QDomElement paragraph, QString name, QString attrName, QString attr) {
	QDomElement lastformat=currentFormat(paragraph,true);
	QDomNodeList qdnl= lastformat.elementsByTagName(name);
	if (qdnl.length()) {
	  QDomElement el;
	  el=qdnl.item(0).toElement();
	  el.setAttribute(attrName,attr);
	  return el;
	} else {
	  QDomElement al=_doc->createElement(name);
	  lastformat.appendChild(al);
	  al.setAttribute(attrName,attr);
	  return al;
	}

}

QString KWDWriter::getLayoutAttribute(QDomElement paragraph, QString name, QString attrName) {
	QDomElement currentLayout=paragraph.elementsByTagName("LAYOUT").item(0).toElement();
	QDomNodeList qdnl= currentLayout.elementsByTagName(name);
	if (qdnl.length()) {
	  QDomElement el=qdnl.item(0).toElement();
	  return el.attribute(attrName);
	}
	return QString::null;
}

QDomElement KWDWriter::layoutAttribute(QDomElement paragraph, QString name, QString attrName, QString attr) {
	QDomElement currentLayout=paragraph.elementsByTagName("LAYOUT").item(0).toElement();
	QDomNodeList qdnl= currentLayout.elementsByTagName(name);

	if (qdnl.length()) {
	  QDomElement el;
	  el=qdnl.item(0).toElement();
	  el.setAttribute(attrName,attr);
	  return el;
	} else {
	  QDomElement al=_doc->createElement(name);
	  currentLayout.appendChild(al);
	  al.setAttribute(attrName,attr);
	  return al;
	}
}

void KWDWriter::addText(QDomElement paragraph, QString text, int format_id) {
	QDomNode temp=paragraph.elementsByTagName("TEXT").item(0).firstChild();
	QDomText currentText=temp.toText();
	if (temp.isNull()) { qWarning("no text"); exit(0); }
	int oldLength=currentText.data().length();
	currentText.setData(currentText.data()+text);
	//qWarning(currentText.data());
	int newLength=text.length();
	QDomElement lastformat=currentFormat(paragraph,true);
	lastformat.setAttribute("id",format_id);
	lastformat.setAttribute("pos",QString("%1").arg(oldLength));
	lastformat.setAttribute("len",QString("%1").arg(newLength));
}

QDomElement KWDWriter::startFormat(QDomElement paragraph) {
        if (paragraph.isNull()) { qWarning("startFormat on empty paragraph"); exit(0); }
	QDomElement format=_doc->createElement("FORMAT");
	paragraph.elementsByTagName("FORMATS").item(0).appendChild(format);
	//formatAttribute(paragraph,"SIZE","VALUE","12"); // FIXME hack hack hack
	return format;
}


QDomElement KWDWriter::startFormat(QDomElement paragraph, QDomElement formatToClone) {
	QDomElement format=formatToClone.cloneNode().toElement();
	if (format.isNull()) { qWarning("startFormat: null format cloned"); exit(0); }
        if (paragraph.isNull()) { qWarning("startFormat on empty paragraph"); exit(0); }

	format.removeAttribute("len");
	format.removeAttribute("pos");
	format.removeAttribute("id");
	for (QDomElement a=format.firstChild().toElement();!a.isNull();a=a.nextSibling().toElement()) {
		if (a.tagName() == "ANCHOR") {
		      format.removeChild(a);
		}
	}
	paragraph.elementsByTagName("FORMATS").item(0).appendChild(format);
	return format;
}

QDomElement KWDWriter::currentFormat(QDomElement paragraph, bool start_new_one) {

	QDomElement e=paragraph.elementsByTagName("FORMATS").item(0).lastChild().toElement();
	if (e.isNull()) {
	   // no current format, start a new one
	   if (start_new_one) return startFormat(paragraph);
	   else { qWarning("warning: returning null format"); }
	}
	if (e.attribute("len") != QString::null) {
	   // current format already has length, clone it.
	   if (start_new_one) return startFormat(paragraph,e);
	}
	return e;
}


void KWDWriter::cleanUpParagraph(QDomElement paragraph) {
	QDomElement e=paragraph.elementsByTagName("FORMATS").item(0).toElement();
	if (e.isNull()) { qWarning("cleanup : no valid paragraph"); exit(0); }
	for (QDomElement k=e.firstChild().toElement();!k.isNull();k=k.nextSibling().toElement()) {
	     if (k.attribute("len",QString::null) == QString::null) {
	         e.removeChild(k);
	         cleanUpParagraph(paragraph); // sloooow
	         return;
	     }
	}
}


QDomElement KWDWriter::addFrame(QDomElement frameset, int runaround, int copy,
                                int top, int left, int bottom, int right,
                                int newFrameBehaviour, int runaroundGap ) {
	QDomElement frame = _doc->createElement("FRAME");
	frameset.appendChild(frame);
	frame.setAttribute("runaround",runaround);
	frame.setAttribute("copy",copy);
	frame.setAttribute("newFrameBehaviour",newFrameBehaviour);
	frame.setAttribute("runaroundGap",runaroundGap);
	addRect(frame,left,top,right-left,bottom-top);
	return frame;
}

QDomElement KWDWriter::docroot() {
	return _doc->elementsByTagName("DOC").item(0).toElement();
}

bool KWDWriter::writeDoc() {
	QCString str=_doc->toCString();
	qWarning(str);
	if (!_store->open("root")) {
	    qWarning("warning, cannot open root in store");
	    return false;
	}
	_store->write((const char *)str, str.length());
	_store->close();
	return true;
}


QDomElement KWDWriter::currentFrameset() {
	return _doc->elementsByTagName("FRAMESETS").item(0).lastChild().toElement();
}


void KWDWriter::addRect(QDomElement e,QRect rect) {
     e.setAttribute("top",rect.top());
     e.setAttribute("left",rect.left());
     e.setAttribute("bottom",rect.bottom());
     e.setAttribute("right",rect.right());
}

void KWDWriter::addRect(QDomElement e,int x, int y, int w, int h) {
	addRect(e,QRect(x,y,w,h));
}



QDomElement KWDWriter::currentParagraph(QDomElement frameset) {
	QDomNodeList e=frameset.elementsByTagName("FRAMESET");
	return e.item(e.length()-1).toElement(); // FIXME
}




KWDWriter::~KWDWriter(){
}
