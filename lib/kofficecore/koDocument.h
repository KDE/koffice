// -*- c-basic-offset: 4 -*-
/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>
   Copyright (C) 2000, 2001 David Faure <david@mandrakesoft.com>

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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef __ko_document_h__
#define __ko_document_h__

namespace std { };
using namespace std;
#include <qwmatrix.h>

#include <kparts/part.h>
#include <kurl.h>
#include <kservice.h>
#include <koGlobal.h>

class QDomElement;
class QDomDocument;

class KoStore;
class KoMainWindow;

class KoChild;
class KoDocumentChild;
class KoView;
class KoDocumentInfo;
class DCOPObject;


/**
 *  The KOffice document class
 *
 *  This class provides some functionality each KOffice document should have.
 *
 *  @short The KOffice document class
 */
class KoDocument : public KParts::ReadWritePart
{
    Q_OBJECT
    Q_PROPERTY( QCString dcopObjectId READ dcopObjectId)
    
public:

    /**
     *  Constructor.
     * The first 4  arguments are the same as the ones passed to KParts::Factory::createPart.
     *
     * @param parentWidget the parent widget, in case we create a wrapper widget
     *        (in single view mode).
     *        Usually the first argument passed by KParts::Factory::createPart.
     * @param parent may be another KoDocument, or anything else.
     *        Usually the third argument of KParts::Factory::createPart.
     * @param name is used to identify this document via DCOP so you may want to
     *        pass a meaningful name here which matches the pattern [A-Za-z_][A-Za-z_0-9]*.
     * @param singleViewMode determines whether the document may only have one view. In this case
     *        the @param parent must be a QWidget derived class. KoDocument will then create a wrapper widget
     *        (@ref KoViewWrapperWidget) which is a child of @param parentWidget.
     *        This widget can be retrieved by calling @ref #widget.
     */
    KoDocument( QWidget* parentWidget,
                const char* widgetName,
                QObject* parent,
                const char* name,
                bool singleViewMode = false );

    /**
     *  Destructor.
     *
     * The destructor does not delete any attached @ref KoView objects and it does not
     * delete the attached widget as returned by @ref #widget.
     */
    virtual ~KoDocument();

    /**
     * Tells whether this document is in singleview mode. This mode can only be set
     * in the constructor.
     */
    bool isSingleViewMode() const;

    /**
     * Is the document embedded?
     */
    bool isEmbedded() const;

    /**
     * Returns the action described action object. In fact only the "name" attribute
     * of @ref #element is of interest here. The method searches first in the
     * @ref KActionCollection of the first view and then in the KActionCollection of this
     * document.
     * This allows KOffice applications to define actions in both the view and the document.
     * They should only define view-actions (like zooming and stuff) in the view.
     * Every action which changes the document should be defined in the document.
     *
     * Please notice that KoDocument indirectly inherits KXMLGUIClient.
     *
     * @see KXMLGUIClient
     * @see KXMLGUIClient::actionCollection
     * @see KoView::action
     */
    virtual KAction *action( const QDomElement &element ) const;

    /**
     * Returns the DOM document which describes the GUI of the
     * first view.
     */
    virtual QDomDocument domDocument() const;

    /**
     * @internal
     */
    virtual void setManager( KParts::PartManager *manager );

    /**
     * Reimplemented from KParts::ReadWritePart for internal reasons
     * (for the autosave functionality)
     */
    virtual bool openURL( const KURL & url );

    /**
     * Sets whether the document can be edited or is read only.
     * This recursively applied to all child documents and
     * @ref KoView::updateReadWrite is called for every attached
     * view.
     */
    virtual void setReadWrite( bool readwrite = true );

    /**
     * Used by KoApplication, when no document exists yet.
     *
     * With the help of @param instance or @ref KApplication::instance() this
     * method figures out which .desktop file matches this application. In this
     * file it searches for the "X-KDE-NativeMimeType" entry and returns it.
     *
     * @see KService
     * @see KDesktopFile
     */
    static QCString readNativeFormatMimeType( KInstance *instance = 0L );

    static KService::Ptr readNativeService( KInstance *instance = 0L );

    /**
     * To be preferred when a document exists. It is fast when calling
     * it multiple times since it caches the result that @ref #readNativeFormatMimeType
     * delivers.
     * You do NOT have to reimplement this (it is only virtual for kounavail).
     */
    virtual QCString nativeFormatMimeType() const;

    KService::Ptr nativeService();

    enum { SaveAsKOffice1dot1 = 1, SaveAsDirectoryStore = 2 };

    /**
     * Set the format in which the document should be saved.
     * This is called on loading, and in "save as", so you shouldn't
     * have to call it.
     * @param specialOutputFlag is for "save as older version" etc.
     */
    void setOutputMimeType( const QCString & mimeType, int specialOutputFlag = 0 );
    QCString outputMimeType() const;
    int specialOutputFlag() const;

    /**
     * Sets the error message to be shown to the user (use i18n()!)
     * when loading or saving fails.
     * If you asked the user about something and he chose "Cancel",
     * set the message to the magic string "USER_CANCELED", to skip the error dialog.
     */
    void setErrorMessage( const QString& errMsg );

    /**
     *  Create a new view for the document.
     */
    KoView *createView( QWidget *parent = 0, const char *name = 0 );

    /**
     * Adds a view to the document.
     *
     * This calls @ref KoView::updateReadWrite to tell the new view
     * whether the document is readonly or not.
     */
    virtual void addView( KoView *view );

    virtual void removeView( KoView *view );

    const QPtrList<KoView> & views() const;

    int viewCount() const;

    /**
     * Reimplemented from @ref KParts::Part
     */
    virtual KParts::Part *hitTest( QWidget *widget, const QPoint &globalPos );

    /**
     *  Find the most nested child document which contains the
     *  questionable point. The point is in the coordinate system
     *  of this part. If no child document contains this point, then
     *  a pointer to this document is returned.
     *
     *  This function has to be overloaded if the document features child documents.
     *
     *  @param matrix transforms points from the documents coordinate system
     *         to the coordinate system of the questionable point.
     *  @param pos is in some unknown coordinate system, but the matrix can
     *         be used to transform a point of this parts coordinate system
     *         to the coordinate system of p.
     *
     *  @return Pointer to the document, that was hit.
     */
    virtual KoDocument *hitTest( const QPoint &pos, const QWMatrix &matrix = QWMatrix() );

    /**
     *  Paints the whole document into the given painter object.
     *
     *  @param painter     The painter object into that should be drawn.
     *  @param rect        The rect that should be used in the painter object.
     *  @param transparent If true then the entire rectangle is erased before painting.
     *  @param view        The KoView is needed to fiddle about with the active widget, when painting children.
     *  @param zoomX       The zoom value to be applied to X coordinates when painting.
     *  @param zoomY       The zoom value to be applied to Y coordinates when painting.
     */
    virtual void paintEverything( QPainter &painter, const QRect &rect, bool transparent = false,
                                  KoView *view = 0L, double zoomX = 1.0, double zoomY = 1.0 );

    virtual QPixmap generatePreview( const QSize& size );

    /**
     *  Paints all of the documents children into the given painter object.
     *
     *  @param painter     The painter object into that should be drawn.
     *  @param rect        The rect that should be used in the painter object.
     *  @param view        The KoView is needed to fiddle about with the active widget.
     *  @param zoomX       The zoom value to be applied to X coordinates when painting.
     *  @param zoomY       The zoom value to be applied to Y coordinates when painting.
     *
     *  @see #paintChild #paintEverything #paintContent
     */
    virtual void paintChildren( QPainter &painter, const QRect &rect, KoView *view, double zoomX = 1.0, double zoomY = 1.0 );

    /**
     *  Paint a given child. Normally called by @ref paintChildren.
     *
     *  @param child       The child to be painted.
     *  @param painter     The painter object into that should be drawn.
     *  @param view        The KoView is needed to fiddle about with the active widget.
     *  @param zoomX       The zoom value to be applied to X coordinates when painting.
     *  @param zoomY       The zoom value to be applied to Y coordinates when painting.
     *
     *  @see #paintEverything #paintChildren #paintContent
     */
    virtual void paintChild( KoDocumentChild *child, QPainter &painter, KoView *view, double zoomX = 1.0, double zoomY = 1.0 );

    /**
     *  Paints the data itself. Normally called by @ref paintEverthing. It does not
     *  paint the children.
     *  It's this method that KOffice Parts have to implement.
     *
     *  @param painter     The painter object into that should be drawn.
     *  @param rect        The rect that should be used in the painter object.
     *  @param transparent If true then the entire rectangle is erased before painting.
     *  @param zoomX       The zoom value to be applied to X coordinates when painting.
     *  @param zoomY       The zoom value to be applied to Y coordinates when painting.
     *
     *  @see #paintEverything
     */
    virtual void paintContent( QPainter &painter, const QRect &rect, bool transparent = false, double zoomX = 1.0, double zoomY = 1.0 ) = 0;

    /**
     * Called by koApplication to check for an autosave file in $HOME
     */
    bool checkAutoSaveFile();

    /**
     *  Initializes an empty document (display the template dialog!).
     *  You have to overload this method to initalize all your document variables.
     */
    virtual bool initDoc() = 0;

    /**
     *  Sets the modified flag on the document. This means that it has
     *  to be saved or not before deleting it.
     */
    virtual void setModified( bool _mod );

    /**
     *  Tells the document that its title has been modified, either because
     *  the modified status changes (this is done by @ref setModified) or
     *  because the URL or the document-info's title changed.
     */
    virtual void setTitleModified();

    /**
     *  @return true if the document is empty.
     */
    virtual bool isEmpty() const { return m_bEmpty; }

    /**
     *  Sets the document to empty. Used after loading a template
     *  (which is not empty, but not the user's input).
     *
     * @ref #isEmpty
     */
    virtual void setEmpty() { m_bEmpty = true; }

    /**
     *  Loads a document from a store.
     *  You should never have to reimplement.
     *  @param url An internal url, like tar:/1/2
     */
    virtual bool loadFromStore( KoStore* store, const QString& url );

    /**
     *  Saves a document to a store.
     *  You should not have to reimplement this - but call it in @ref saveChildren.
     */
    virtual bool saveToStore( KoStore* store, const QString& path );

    /**
     *  Reimplement this method to load the contents of your KOffice document,
     *  from the XML document.
     *
     *  You are supposed to use the QDomDocument. The QIODevice is provided only
     *  for the cases where some pre-processing is needed, like kpresenter's kprconverter.
     *  Note that the QIODevice could be 0L, when called from an import filter.
     */
    virtual bool loadXML( QIODevice *, const QDomDocument & doc ) = 0;

    /**
     *  Reimplement this to save the contents of the KOffice document into
     *  a QDomDocument. The framework takes care of saving it to the store.
     */
    virtual QDomDocument saveXML();

    /**
     *  Return a correctly created QDomDocument for this KoDocument,
     *  including processing instruction, complete DOCTYPE tag (with systemId and publicId), and root element.
     *  @param tagName the name of the tag for the root element
     *  @param version the DTD version (usually the application's version).
     */
    QDomDocument createDomDocument( const QString& tagName, const QString& version ) const;

    /**
     *  Return a correctly created QDomDocument for a KOffice document,
     *  including processing instruction, complete DOCTYPE tag (with systemId and publicId), and root element.
     *  This static method can be used e.g. by filters.
     *  @param appName the app's instance name, e.g. kword, kspread, kpresenter etc.
     *  @param tagName the name of the tag for the root element, e.g. DOC for kword/kpresenter.
     *  @param version the DTD version (usually the application's version).
     */
    static QDomDocument createDomDocument( const QString& appName, const QString& tagName, const QString& version );

    /**
     *  Save the document. The default implementation is to call
     *  @ref saveXML. This method exists only for applications that
     *  don't use QDomDocument for saving, i.e. kword and kpresenter.
     */
    virtual bool saveToStream( QIODevice * dev );

    /**
     *  Loads a document in the native format from a given URL.
     *  Reimplement if your native format isn't XML.
     *
     *  @param file the file to load - usually @ref KReadOnlyPart::m_file or the result of a filter
     */
    virtual bool loadNativeFormat( const QString & file );

    /**
     *  Saves the document in native format, to a given file
     *  You should never have to reimplement.
     *  Made public for writing templates.
     */
    virtual bool saveNativeFormat( const QString & file );

    /**
     * Activate/deactivate/configure the autosave feature.
     * @param delay in seconds, 0 to disable
     */
    void setAutoSave( int delay );

    /**
     * Checks whether the document is currently in the process of autosaving
     */
    bool isAutosaving();

    /**
     * Set whether the next openURL call should check for an auto-saved file
     * and offer to open it. This is usually true, but can be turned off
     * (e.g. for the preview module).
     */
    void setCheckAutoSaveFile( bool b );

    /**
     * Set whether the next openURL call should show error message boxes in case
     * of errors. This is usually the case, but e.g. not when generating thumbnail
     * previews.
     */
    void setAutoErrorHandlingEnabled( bool b );

    /**
     * Retrieve the default value for autosave in seconds.
     * Called by the applications to use the correct default in their config
     */
    static int defaultAutoSave() { return s_defaultAutoSave; }

    /**
     * @return the list of all children. Do not modify the
     *         returned list.
     */
    const QPtrList<KoDocumentChild>& children() const;

    /**
     * @return the KoDocumentChild associated with the given Document, but only if
     *         "doc" is a direct child of this document.
     *
     * This is a convenience function. You could get the same result
     * by traversing the list returned by @ref #children.
     */
    KoDocumentChild *child( KoDocument *doc );

    /**
     * @return the information concerning this document.
     * @see KoDocumentInfo
     */
    KoDocumentInfo *documentInfo() const;

    void setViewBuildDocument( KoView *view, const QDomDocument &doc );
    QDomDocument viewBuildDocument( KoView *view );

    /**
     * Appends the shell to the list of shells which show this
     * document as their root document.
     *
     * This method is automatically called from @ref KoMainWindow::setRootDocument,
     * so you dont need to call it.
     */
    virtual void addShell( KoMainWindow *shell );

    /**
     * Removes the shell from the list. That happens automatically if the shell changes its
     * root document. Usually you dont need to call this method.
     */
    virtual void removeShell( KoMainWindow *shell );

    const QPtrList<KoMainWindow>& shells() const;

    int shellCount() const;

    /**
     * Returns the list of all the currently opened documents
     */
    static QPtrList<KoDocument> *documentList() { return s_documentList; }

    /**
     * Return a DCOP interface for this document
     * KOffice parts are strongly recommended to reimplement this method,
     * so that their dcop interface provides more functionality than the basic KoDocumentIface
     */
    virtual DCOPObject * dcopObject();
    
    /**
     * return the ID of the dcop interface for this document.
     **/
    QCString dcopObjectId() const;

    void emitProgress( int value ) { emit sigProgress( value ); }

    bool isInOperation();
    virtual void emitBeginOperation();
    virtual void emitEndOperation();

    /**
     * Return true if url() is a real filename, false if url() is
     * an internal url in the store, like "tar:/..."
     */
    virtual bool isStoredExtern();

    KoPageLayout pageLayout() const { return m_pageLayout; }
    
    void removeAutoSaveFiles();
    
signals:
    /**
     * This signal is emitted, if a direct or indirect child document changes
     * and needs to be updated in all views.
     *
     * If one of your child documents emits the childChanged signal, then you may
     * usually just want to redraw this child. In this case you can ignore the parameter
     * passed by the signal.
     */
    void childChanged( KoDocumentChild *child );

    void sigProgress(int value);

    void sigBeginOperation();
    void sigEndOperation();

protected:

    QString autoSaveFile( const QString & path ) const;

    virtual KoView *createViewInstance( QWidget *parent, const char *name ) = 0;

    /**
     *  Loads a document from @ref KReadOnlyPart::m_file (KParts takes care of downloading
     *  remote documents).
     *  Applies a filter if necessary, and calls loadNativeFormat in any case
     *  You should not have to reimplement, except for very special cases.
     *
     * This method is called from the @ref KReadOnlyPart::openURL method.
     */
    virtual bool openFile();

    /**
     *  Saves a document to @ref KReadOnlyPart::m_file (KParts takes care of uploading
     *  remote documents)
     *  Applies a filter if necessary, and calls saveNativeFormat in any case
     *  You should not have to reimplement, except for very special cases.
     */
    virtual bool saveFile();

    /**
     *  You need to overload this function if your document may contain
     *  embedded documents. This function is called to load embedded documents.
     *
     *  An example implementation may look like this:
     *  <PRE>
     *  QPtrListIterator<KoDocumentChild> it( children() );
     *  for( ; it.current(); ++it )
     *  {
     *    if ( !it.current()->loadDocument( _store ) )
     *    {
     *      return false;
     *    }
     *  }
     *  return true;
     *  </PRE>
     */
    virtual bool loadChildren( KoStore* );

    /**
     *  Saves all children. If your document supports embedding, then you have
     *  to overload this function. An implementation may look like this:
     *  <PRE>
     *  int i = 0;
     *
     *  QPtrListIterator<KoDocumentChild> it( children() );
     *  for( ; it.current(); ++it ) {
     *    KoDocument* childDoc = static_cast<KoDocumentChild*>(it.current())->document();
     *    if ( childDoc )
     *      if ( childDoc->isStoredExtern() )
     *      {
     *          if ( !childDoc->save() )
     *              return FALSE;
     *      }
     *      else {
     *          QString internURL = QString( "%1/%2" ).arg( _path ).arg( i++ );
     *          if ( !childDoc->saveToStore( _store, internURL ) )
     *              return FALSE;
     *      }
     *  }
     *  return true;
     *  </PRE>
     */
    virtual bool saveChildren( KoStore* store );

    /**
     *  Overload this function if you have to load additional files
     *  from a store. This function is called after @ref #loadXML
     *  and after @ref #loadChildren have been called.
     */
    virtual bool completeLoading( KoStore* store );

    /**
     *  If you want to write additional files to a store,
     *  then you must do it here.
     *  In the implementation, you should prepend the document
     *  url (using url().url()) before the filename, so that everything is kept relative
     *  to this document. For instance it will produce urls such as
     *  tar:/1/pictures/picture0.png, if the doc url is tar:/1
     *  But do this ONLY if the document is not stored extern (see @ref #isStoredExtern).
     *  If it is, then the pictures should be saved to tar:/pictures.
     */
    virtual bool completeSaving( KoStore* store );

    /**
     * Sets the document URL to empty URL
     * KParts doesn't allow this, but KOffice apps have e.g. templates
     * After using loadNativeFormat on a template, one wants
     * to set the url to KURL()
     */
    void resetURL() { m_url = KURL(); m_file = QString::null; }

    /**
     * Inserts the new child in the list of children and emits the
     * @ref #childChanged signal.
     *
     * At the same time this method marks this document as modified.
     *
     * To remove a child, just delete it. KoDocument will detect this
     * and remove the child from its lists.
     *
     * @see #isModified
     */
    virtual void insertChild( KoDocumentChild *child );

    /** @internal */
    virtual void setModified() { KParts::ReadWritePart::setModified(); }

    /** @internal */
    virtual void insertChild(QObject *o) { QObject::insertChild(o); }

    KoPageLayout m_pageLayout;

private slots:
    void slotChildChanged( KoChild *c );
    void slotChildDestroyed();
    void slotAutoSave();

private:
    void savePreview( KoStore* store );

    class Private;
    Private *d;
    KService::Ptr m_nativeService;
    bool m_bEmpty;
    static QPtrList<KoDocument> *s_documentList;
    static const int s_defaultAutoSave;
};

#endif
