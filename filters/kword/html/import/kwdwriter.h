/***************************************************************************
                          kwdwriter.h  -  description
                             -------------------
    begin                : Wed Sep 5 2001
    copyright            : (C) 2001 by Frank Dekervel
    email                : Frank.Dekervel@student.kuleuven.ac.be
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License as        *
 *   published by                                                          *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KWDWRITER_H
#define KWDWRITER_H


#include <QRect>
#include <qdom.h>

#include <KoStore.h>

/**
  *@author Frank Dekervel
  */
class KWDWriter {
public: 
	explicit KWDWriter(KoStore *store);
	~KWDWriter();


	/**
	 * writes the document to the koStore
	 **/
	bool writeDoc();


	/**
	 * adds a frameset to parent FIXME
	 **/
	QDomElement addFrameSet(QDomElement &parent, int frametype=1,
				int frameinfo=0, const QString&name=QString(),
				int visible=1);

	/**
	 * \returns true if we are currently in a table (\a createTable() got called,
	 * but we are still waiting for a \a finishTable() ).
	 */
	bool isInTable() const;

	/**
	 * creates a table
	 **/
	int createTable();


	/**
	 * creates a table cell
	 **/
	QDomElement createTableCell(int tableno, int nrow,
				int ncol, int colspan, QRect rect);



	/**
	 * fetches the cell of a table
	 **/
	QDomElement fetchTableCell(int tableno, int rowno, int colno);


	/**
	 * finishes a table
	 * if the arguments x,y,w,h are given, each cell is resized to have a 'right'
	 * table. otherwise, the cell sizes are not touched.
	 **/
	void finishTable(int tableno, QRect rect);
	void finishTable(int tableno);

	/**
	 * inlines something in a paragraph
	 * @param paragraph: the paragraph the anchor should be placed in
	 * @param toInline: the element that should be inlined
	 **/
	void createInline(const QDomElement &paragraph, const QDomElement &toInline);


	/**
	 * create a horizontal ruler layout
	 **/
	void createHR(const QDomElement &paragraph, int width=1);

	/**
	 *
	 **/
	QDomElement currentLayout(const QDomElement &paragraph);

	/**
	 * adds a frame to frameset FIXME
	 **/
	QDomElement addFrame(QDomElement &frameset, QRect rect, int runaround=0, int copy=0,
                                //int top=42, int left=28, int bottom=799, int right=567,
                                int newFrameBehaviour=0, int runaroundGap=2
				);


	/**
	 * adds a paragraph
	 **/
	QDomElement addParagraph(QDomElement &parent);
	QDomElement addParagraph(QDomElement &parent, const QDomElement &layout);

	/**
	 * adds/changes an attribute to/of the current format
	 **/
        QDomElement formatAttribute(const QDomElement &paragraph, const QString& name, const QString& attrName, const QString& attr);

        /**
         * get a layout attribute
         **/
	QString getLayoutAttribute(const QDomElement &paragraph, const QString& name, const QString& attrName);


        /**
         * adds/changes an attribute to/of the current layout
         **/
        QDomElement layoutAttribute(const QDomElement &paragraph, const QString& name, const QString& attrName, const QString& attr);

        /**
         * creates a new format in the current paragraph. do this before adding text
         * FIXME: you can only do addText once per format
         **/
        QDomElement startFormat(const QDomElement &paragraph);
        QDomElement startFormat(const QDomElement &paragraph, const QDomElement &formatToClone);


	/**
	 * cleans up the current paragraph (throw away unused formats)
	 * FIXME: find a better solution
	 **/
	 void cleanUpParagraph(const QDomElement &paragraph);

	/**
	 * adds some text to the current format in this paragraph
	 **/
	void addText(const QDomElement &paragraph, const QString& text, int format_id, bool keep_formatting=false);

	/**
	 * returns the current format
	 * if start_new_one is true, a new format will be started if needed
	 **/
	QDomElement currentFormat(const QDomElement &paragraph, bool start_new_one=false);

	/**
	 * create a Link (URL)
	 **/
	QDomElement createLink(const QDomElement &paragraph, const QString& linkName, const QString& hrefName);

	/**
	 * copy the given layout, and set it as layout of the given paragraph
	 **/
	QDomElement setLayout(QDomElement &paragraph, const QDomElement &layout);

	/**
	 * returns the text of this paragraph.
	 **/
	QString getText(const QDomElement &paragraph);

	/**
	 * returns the rectangle of the first frame of this frameset
	 **/
	QRect getRect(const QDomElement &frameset);

	/**
	 * returns the 'main' frameset of this document.
	 **/
        QDomElement mainFrameset();

	/**
	 * mark document as being written by author, and having title title
	 **/
	void createDocInfo(const QString& author, const QString& title);

        /**
         * returns the document root
         **/
        QDomElement docroot();

        /**
         * creates a KWord Variable (Link, ...)
         **/
	void appendKWordVariable(QDomDocument& doc, QDomElement& format,
		const QString& text, const QString& key, int type, QDomElement& child);

private:
	/**
	 * creates a rectangle
	 **/
	void addRect(QDomElement &e, QRect rect);

       	
protected:
	KoStore *_store;
	QDomElement _mainFrameset;
	QDomDocument *_doc;
	QDomDocument *_docinfo;
	QDomElement _docinfoMain;
	int tableNo;
	bool insidetable;
	
};

#endif
