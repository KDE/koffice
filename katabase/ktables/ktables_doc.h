/***************************************************************************
                          ktablesdoc.h  -  description                              
                             -------------------                                         
    begin                : Mi� J�l  7 17:04:49 CEST 1999
                                           
    copyright            : (C) 1999 by �rn E. Hansen                         
    email                : hanseno@mail.bip.net                                     
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   * 
 *                                                                         *
 ***************************************************************************/


#ifndef KTABLESDOC_H
#define KTABLESDOC_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif 

// include files for QT
#include <qobject.h>
#include <qstring.h>
#include <qlist.h>

#include <openparts_ui.h>
#include <koFrame.h>
#include <koDocument.h>
#include <koPrintExt.h>

#include "ktables.h"

// forward declaration of the Ktables classes
class KtablesView;
class KtablesServer;
class QueryDialog;

/**	KtablesDoc provides a document object for a document-view model.
  *
	* The KtablesDoc class provides a document object that can be used in conjunction with the classes KtablesApp and KtablesView
	* to create a document-view model for standard KDE applications based on KApplication and KTMainWindow. Thereby, the document object
	* is created by the KtablesApp instance and contains the document structure with the according methods for manipulation of the document
	* data by KtablesView objects. Also, KtablesDoc contains the methods for serialization of the document data from and to files.
	*
	* @author Source Framework Automatically Generated by KDevelop, (c) The KDevelop Team. 	
	* @version KDevelop version 0.4 code generation
	*/
class KtablesDoc :  public QObject,
										virtual public KoDocument,
										virtual public KoPrintExt,
										virtual public Ktables::Document_skel
{
  Q_OBJECT

 public:
  KtablesDoc(QWidget* parent=0, const char *name=0);
  virtual ~KtablesDoc();

  virtual void cleanUp();
  virtual CORBA::Boolean initDoc();
  virtual KOffice::MainWindow_ptr createMainWindow();

  virtual KtablesView *createTableView(QWidget *p=0);
  virtual OpenParts::View_ptr createView();
  virtual void viewList( OpenParts::Document::ViewList*& _list );
  virtual int viewCount();
  virtual bool isEmpty();
  virtual void draw(QPaintDevice *, CORBA::Long, CORBA::Long, CORBA::Float) { };

  virtual void addView(KtablesView *);
  virtual void removeView(KtablesView *);

	void deleteContents();

	void pathName(const char* path_name);
	const QString& getPathName() const;
	
	void title(const char* title);
	const QString& getTitle() const;
	
  char *mimeType() { return  CORBA::string_dup( "application/x-ktables" ); };
  virtual bool hasToWriteMultipart() { return false; };
  virtual bool loadXML( KOMLParser& parser, KOStore::Store_ptr _store );
  virtual bool save( ostream& out,const char * /* format */ );

 signals:
 	void sigUpdateView();
  void signalMsg(const char *);

 public slots:
 	void slotUpdateAllViews(KtablesView* pSender);
	 	
 private:
 	bool b_modified;
	QString m_title;
	QString m_path;

};

#endif // KTABLESDOC_H




































