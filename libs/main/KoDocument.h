/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>
   Copyright (C) 2000-2005 David Faure <faure@kde.org>
   Copyright (C) 2007 Thorsten Zachmann <zachmann@kde.org>

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

#ifndef KODOCUMENT_H
#define KODOCUMENT_H

#include <QtCore/QDateTime>
#include <QtGui/QMatrix>
#include <Qt3Support/Q3PtrList>

#include <kparts/part.h>
#include <kservice.h>
#include <kcomponentdata.h>

#include "KoUnit.h"
#include "KoPageLayout.h"
#include "komain_export.h"
#include "KoGridData.h"
#include "KoGuidesData.h"
#include <KoXmlReader.h>

class QXmlSimpleReader;
class QUndoCommand;

class KoStore;
class KoOdfReadStore;
class KoOdfWriteStore;
class KoMainWindow;
class KoChild;
class KoDocumentChild;
class KoView;
class KoDocumentInfo;
class KoOpenPane;
class KoEmbeddedDocumentSaver;


class KoVersionInfo
{
public:
    QDateTime date;
    QString saved_by;
    QString comment;
    QString title;

    QByteArray data; //the content of the compressed version
};

/**
 *  The %KOffice document class
 *
 *  This class provides some functionality each %KOffice document should have.
 *
 *  @short The %KOffice document class
 */
class KOMAIN_EXPORT KoDocument : public KParts::ReadWritePart
{
    Q_OBJECT
//     Q_PROPERTY( QByteArray dcopObjectId READ dcopObjectId)
    Q_PROPERTY(bool backupFile READ backupFile WRITE setBackupFile)

public:
    // context passed on saving to saveOdf
    struct SavingContext {
        SavingContext(KoOdfWriteStore & odfStore, KoEmbeddedDocumentSaver & embeddedSaver)
                : odfStore(odfStore)
                , embeddedSaver(embeddedSaver) {}

        KoOdfWriteStore & odfStore;
        KoEmbeddedDocumentSaver & embeddedSaver;
    };

    /**
     *  Constructor.
     * The first 4 arguments are the same as the ones passed to KParts::Factory::createPart.
     *
     * @param parentWidget the parent widget, in case we create a wrapper widget
     *        (in single view mode).
     *        Usually the first argument passed by KParts::Factory::createPart.
     * @param parent may be another KoDocument, or anything else.
     *        Usually the third argument of KParts::Factory::createPart.
     * @param singleViewMode determines whether the document may only have one view. In this case
     *        the @p parent must be a QWidget derived class. KoDocument will then create a wrapper widget
     *        (KoViewWrapperWidget) which is a child of @p parentWidget.
     *        This widget can be retrieved by calling widget().
     */
    KoDocument(QWidget* parentWidget,
               QObject* parent,
               bool singleViewMode = false);

    /**
     *  Destructor.
     *
     * The destructor does not delete any attached KoView objects and it does not
     * delete the attached widget as returned by widget().
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
     * of @p element is of interest here. The method searches first in the
     * KActionCollection of the first view and then in the KActionCollection of this
     * document.
     * This allows %KOffice applications to define actions in both the view and the document.
     * They should only define view-actions (like zooming and stuff) in the view.
     * Every action which changes the document should be defined in the document.
     *
     * Please notice that KoDocument indirectly inherits KXMLGUIClient.
     *
     * @see KXMLGUIClient
     * @see KXMLGUIClient::actionCollection
     * @see KoView::action
     */
    virtual QAction *action(const QDomElement &element) const;

    /**
     * Returns the DOM document which describes the GUI of the
     * first view.
     */
    virtual QDomDocument domDocument() const;

    /**
     * @internal
     */
    virtual void setManager(KParts::PartManager *manager);

    /**
     * Reimplemented from KParts::ReadWritePart for internal reasons
     * (for the autosave functionality)
     */
    virtual bool openUrl(const KUrl & url);

    /**
     * Opens the document given by @p url, without storing the URL
     * in the KoDocument.
     * Call this instead of openUrl() to implement KoMainWindow's
     * File --> Import feature.
     *
     * @note This will call openUrl(). To differentiate this from an ordinary
     *       Open operation (in any reimplementation of openUrl() or openFile())
     *       call isImporting().
     */
    bool importDocument(const KUrl &url);

    /**
     * Saves the document as @p url without changing the state of the
     * KoDocument (URL, modified flag etc.). Call this instead of
     * KParts::ReadWritePart::saveAs() to implement KoMainWindow's
     * File --> Export feature.
     *
     * @note This will call KoDocument::saveAs(). To differentiate this
     *       from an ordinary Save operation (in any reimplementation of
     *       saveFile()) call isExporting().
     */
    bool exportDocument(const KUrl &url);

    /**
     * @brief Sets whether the document can be edited or is read only.
     *
     * This recursively applied to all child documents and
     * KoView::updateReadWrite is called for every attached
     * view.
     */
    virtual void setReadWrite(bool readwrite = true);

    /**
     * @brief Used by KoApplication, and by KoMainWindow, when no document exists yet.
     *
     * With the help of @p instance or KApplication::componentData() this
     * method figures out which .desktop file matches this application. In this
     * file it searches for the "X-KDE-NativeMimeType" entry and returns it.
     *
     * @see KService
     * @see KDesktopFile
     */
    static QByteArray readNativeFormatMimeType(const KComponentData &instance = KComponentData());

    /**
     * Used by KoMainWindow, when no document exists yet.
     *
     * With the help of @p instance or KApplication::componentData() this
     * method figures out which .desktop file matches this application. In this
     * file it searches for the "X-KDE-ExtraNativeMimeTypes" entry and returns it.
     *
     * @see KService
     * @see KDesktopFile
     */
    static QStringList readExtraNativeMimeTypes(const KComponentData &instance = KComponentData());

    /**
     * With the help of @p instance or KApplication::componentData() this
     * method figures out which .desktop file matches this application,
     * and returns the KService instance for it.
     */
    static KService::Ptr readNativeService(const KComponentData &instance = KComponentData());

    /**
     * To be preferred when a document exists. It is fast when calling
     * it multiple times since it caches the result that readNativeFormatMimeType()
     * delivers.
     * This comes from the X-KDE-NativeMimeType key in the .desktop file
     * You do NOT have to reimplement this (it is only virtual for kounavail).
     */
    virtual QByteArray nativeFormatMimeType() const;

    /**
     * Returns the OASIS OpenDocument mimetype of the document, if supported
     * This comes from the X-KDE-NativeOasisMimeType key in the .desktop file
     */
    QByteArray nativeOasisMimeType() const;

    /// Checks whether a given mimetype can be handled natively.
    bool isNativeFormat(const QByteArray& mimetype) const;

    /// Returns a list of the mimetypes considered "native", i.e. which can
    /// be saved by KoDocument without a filter, in *addition* to the main one
    virtual QStringList extraNativeMimeTypes() const;

    /// Enum values used by specialOutputFlag - note that it's a bitfield for supportedSpecialFormats
    enum { /*SaveAsKOffice1dot1 = 1,*/ // old and removed
        SaveAsDirectoryStore = 2,
        SaveAsFlatXML = 4,
        SaveEncrypted = 8
                        // bitfield! next value is 16
    };

    /**
     * Return the set of SupportedSpecialFormats that the application wants to
     * offer in the "Save" file dialog.
     */
    virtual int supportedSpecialFormats() const;

    /**
     * Returns the actual mimetype of the document
     */
    QByteArray mimeType() const;

    /**
     * @brief Sets the mime type for the document.
     *
     * When choosing "save as" this is also the mime type
     * selected by default.
     */
    void setMimeType(const QByteArray & mimeType);

    /**
     * @brief Set the format in which the document should be saved.
     *
     * This is called on loading, and in "save as", so you shouldn't
     * have to call it.
     *
     * @param mimeType the mime type (format) to use.
     * @param specialOutputFlag is for "save as older version" etc.
     */
    void setOutputMimeType(const QByteArray & mimeType, int specialOutputFlag = 0);
    QByteArray outputMimeType() const;
    int specialOutputFlag() const;

    /**
     * Returns true if this document was the result of opening a foreign
     * file format and if the user hasn't yet saved the document (in any
     * format).
     *
     * Used by KoMainWindow to warn the user when s/he lazily presses
     * CTRL+S to save in the same foreign format, putting all his/her
     * formatting at risk (normally an export confirmation only comes up
     * with Save As).
     *
     * @param exporting specifies whether this is the setting for a
     * File --> Export or File --> Save/Save As operation.
     */
    bool confirmNonNativeSave(const bool exporting) const;
    void setConfirmNonNativeSave(const bool exporting, const bool on);

    virtual bool wantExportConfirmation() const;

    /**
     * Sets the error message to be shown to the user (use i18n()!)
     * when loading or saving fails.
     * If you asked the user about something and he chose "Cancel",
     * set the message to the magic string "USER_CANCELED", to skip the error dialog.
     */
    void setErrorMessage(const QString& errMsg);

    /**
     * Return the last error message. Usually KoDocument takes care of
     * showing it; this method is mostly provided for non-interactive use.
     */
    QString errorMessage() const;

    /**
     * Show the last error message in a message box.
     * The dialog box will mention a saving problem.
     * Note that save/saveFile takes care of doing it.
     */
    void showSavingErrorDialog();

    /**
     * Show the last error message in a message box.
     * The dialog box will mention a loading problem.
     * openUrl/openFile takes care of doing it, but not loadNativeFormat itself,
     * so this is often called after loadNativeFormat returned false.
     */
    void showLoadingErrorDialog();

    /**
     *  Create a new view for the document.
     */
    KoView *createView(QWidget *parent = 0);

    /**
     * Adds a view to the document.
     *
     * This calls KoView::updateReadWrite to tell the new view
     * whether the document is readonly or not.
     */
    virtual void addView(KoView *view);

    /**
     * Removes a view of the document.
     */
    virtual void removeView(KoView *view);

    /**
     * @return a list of views this document is displayed in
     */
    QList<KoView*> views() const;

    /**
     * @return number of views this document is displayed in
     */
    int viewCount() const;

    /**
     * Reimplemented from KParts::Part
     */
    virtual KParts::Part *hitTest(QWidget *widget, const QPoint &globalPos);

    /**
     *  Find the most nested child document which contains the
     *  requested point. The point is in the coordinate system
     *  of this part. If no child document contains this point, then
     *  a pointer to this document is returned.
     *
     *  This method can to be reimplemented if the document features child documents.
     *
     *  @param pos is in (unzoomed) document coordinates
     *  @param view the view in which we are testing for a hit. This is used to implement
     *         logic like "only hit an embedded object if it's selected in the current view"
     *  @param matrix transforms points from the documents coordinate system
     *         to the coordinate system of the requested point. This is used by
     *         transformed child documents, see KoDocumentChild/KoChild.
     *
     *  @return Pointer to the document under the mouse at that position
     */
    virtual KoDocument *hitTest(const QPoint &pos, KoView* view, const QMatrix& matrix = QMatrix());

    /**
     *  Paints the whole document into the given painter object.
     *
     *  @param painter     The painter object onto which will be drawn.
     *  @param rect        The rect that should be used in the painter object.
     *  @param view        The KoView is needed to fiddle about with the active widget, when painting children.
     */
    virtual void paintEverything(QPainter &painter, const QRect &rect, KoView *view = 0);

    /**
     * @brief Generates a preview picture of the document
     * @note The preview is used in the File Dialog and also to create the Thumbnail
     */
    virtual QPixmap generatePreview(const QSize& size);

    /**
     *  Paints all of the documents children into the given painter object.
     *
     *  @param painter     The painter object onto which will be drawn.
     *  @param rect        The rect that should be used in the painter object.
     *  @param view        The KoView is needed to fiddle about with the active widget.
     *
     *  @see #paintChild #paintEverything #paintContent
     */
    virtual void paintChildren(QPainter &painter, const QRect &rect, KoView *view);

    /**
     *  Paint a given child. Normally called by paintChildren().
     *
     *  @param child       The child to be painted.
     *  @param painter     The painter object onto which will be drawn.
     *  @param view        The KoView is needed to fiddle about with the active widget.
     *
     *  @see #paintEverything #paintChildren #paintContent
     */
    virtual void paintChild(KoDocumentChild *child, QPainter &painter, KoView *view);

    /**
     *  Paints the data itself. Normally called by paintEverything(). It does not
     *  paint the children.
     *  It's this method that %KOffice Parts have to implement.
     *
     *  @param painter     The painter object onto which will be drawn.
     *  @param rect        The rect that should be used in the painter object.
     *
     *  @see #paintEverything
     */
    virtual void paintContent(QPainter &painter, const QRect &rect) = 0;

    /**
     * Called by koApplication to check for an autosave file in $HOME
     */
    bool checkAutoSaveFile();

    /**
     * Creates and shows the start up widget.
     * @param parent the KoMainWindow used as parent for the widget.
     * @param alwaysShow always show the widget even if the user has configured it to not show.
     */
    virtual void showStartUpWidget(KoMainWindow* parent, bool alwaysShow = false);

    /**
     *  Tells the document that its title has been modified, either because
     *  the modified status changes (this is done by setModified() ) or
     *  because the URL or the document-info's title changed.
     */
    virtual void setTitleModified();

    /**
     *  @return true if the document is empty.
     */
    virtual bool isEmpty() const {
        return m_bEmpty;
    }

    /**
     *  @brief Sets the document to empty.
     *
     *  Used after loading a template
     *  (which is not empty, but not the user's input).
     *
     *  @see isEmpty()
     */
    virtual void setEmpty() {
        m_bEmpty = true;
    }

    /**
     *  @brief Loads a document from a store.
     *
     *  You should never have to reimplement.
     *
     *  @param store The store to load from
     *  @param url An internal url, like tar:/1/2
     */
    virtual bool loadFromStore(KoStore* store, const QString& url);

    /**
     *  @brief Loads an OASIS document from a store.
     *  This is used for both the main document and embedded objects.
     */
    virtual bool loadOasisFromStore(KoStore* store);

    /**
     *  @brief Saves a document to a store.
     *
     *  You should not have to reimplement this - but call it in saveChildren().
     */
    virtual bool saveToStore(KoStore* store, const QString& path);

    /**
     *  Reimplement this method to load the contents of your KOffice document,
     *  from the XML document. This is for the pre-Oasis file format (maindoc.xml).
     */
    virtual bool loadXML(const KoXmlDocument & doc, KoStore * store) = 0;

    /**
     *  Reimplement this method to load the contents of your %KOffice document,
     *  from the XML document ("content.xml"). The styles have been parsed already,
     *  you can find them in the odfStore.styles(). The store can be used
     *  to load images and embedded documents.
     */
    virtual bool loadOdf(KoOdfReadStore & odfStore) = 0;

    /**
     *  Reimplement this method to save the contents of your %KOffice document,
     *  using the ODF format.
     */
    virtual bool saveOdf(SavingContext & documentContext) = 0;

    /**
     *  Reimplement this to save the contents of the %KOffice document into
     *  a QDomDocument. The framework takes care of saving it to the store.
     */
    virtual QDomDocument saveXML();

    /**
     *  Return a correctly created QDomDocument for this KoDocument,
     *  including processing instruction, complete DOCTYPE tag (with systemId and publicId), and root element.
     *  @param tagName the name of the tag for the root element
     *  @param version the DTD version (usually the application's version).
     *  @deprecated use KoOdfWriteStore::createOasisXmlWriter instead
     */
    KDE_DEPRECATED QDomDocument createDomDocument(const QString& tagName, const QString& version) const;

    /**
     *  Return a correctly created QDomDocument for an old (1.3-style) %KOffice document,
     *  including processing instruction, complete DOCTYPE tag (with systemId and publicId), and root element.
     *  This static method can be used e.g. by filters.
     *  @param appName the app's instance name, e.g. kword, kspread, kpresenter etc.
     *  @param tagName the name of the tag for the root element, e.g. DOC for kword/kpresenter.
     *  @param version the DTD version (usually the application's version).
     */
    static QDomDocument createDomDocument(const QString& appName, const QString& tagName, const QString& version);

    /**
     *  The first thing to do in loadOasis is get hold of the office:body tag, then its child.
     *  If the child isn't the expected one, the error message can indicate what it is instead.
     *  This method returns a translated name for the type of document,
     *  e.g. i18n("Word Processing") for office:text.
     */
    static QString tagNameToDocumentType(const QString& localName);

    /**
     *  Save the document. The default implementation is to call
     *  saveXML(). This method exists only for applications that
     *  don't use QDomDocument for saving, i.e. kword and kpresenter.
     */
    virtual bool saveToStream(QIODevice * dev);

    /**
     *  Loads a document in the native format from a given URL.
     *  Reimplement if your native format isn't XML.
     *
     *  @param file the file to load - usually KReadOnlyPart::m_file or the result of a filter
     */
    virtual bool loadNativeFormat(const QString & file);

    /**
     *  Saves the document in native format, to a given file
     *  You should never have to reimplement.
     *  Made public for writing templates.
     */
    virtual bool saveNativeFormat(const QString & file);

    /**
     * Activate/deactivate/configure the autosave feature.
     * @param delay in seconds, 0 to disable
     */
    void setAutoSave(int delay);

    /**
     * Checks whether the document is currently in the process of autosaving
     */
    bool isAutosaving() const;

    /**
     * Set whether the next openUrl call should check for an auto-saved file
     * and offer to open it. This is usually true, but can be turned off
     * (e.g. for the preview module).
     */
    void setCheckAutoSaveFile(bool b);

    /**
     * Set whether the next openUrl call should show error message boxes in case
     * of errors. This is usually the case, but e.g. not when generating thumbnail
     * previews.
     */
    void setAutoErrorHandlingEnabled(bool b);

    /**
     * Checks whether error message boxes should be shown.
     */
    bool isAutoErrorHandlingEnabled() const;

    /**
     * Retrieve the default value for autosave in seconds.
     * Called by the applications to use the correct default in their config
     */
    static int defaultAutoSave() {
        return s_defaultAutoSave;
    }

    /**
     * @return the list of all children. Do not modify the
     *         returned list.
     */
    const Q3PtrList<KoDocumentChild>& children() const;

    /**
     * @return the KoDocumentChild associated with the given Document, but only if
     *         @p doc is a direct child of this document.
     *
     * This is a convenience function. You could get the same result
     * by traversing the list returned by children().
     */
    KoDocumentChild *child(KoDocument *doc);

    /**
     * @return the information concerning this document.
     * @see KoDocumentInfo
     */
    KoDocumentInfo *documentInfo() const;

    void setViewBuildDocument(KoView *view, const QDomDocument &doc);
    QDomDocument viewBuildDocument(KoView *view);

    /**
     * Appends the shell to the list of shells which show this
     * document as their root document.
     *
     * This method is automatically called from KoMainWindow::setRootDocument,
     * so you do not need to call it.
     */
    virtual void addShell(KoMainWindow *shell);

    /**
     * Removes the shell from the list. That happens automatically if the shell changes its
     * root document. Usually you do not need to call this method.
     */
    virtual void removeShell(KoMainWindow *shell);

    /**
     * @return the list of shells for the main window
     */
    const Q3PtrList<KoMainWindow>& shells() const;

    /**
     * @return the number of shells for the main window
     */
    int shellCount() const;

    /**
     * @return the list of all the currently opened documents
     */
    static Q3PtrList<KoDocument> *documentList() {
        return s_documentList;
    }

    /**
     * @brief Return a DCOP interface for this document
     *
     * %KOffice parts are strongly recommended to reimplement this method,
     * so that their DCOP interface provides more functionality than the basic KoDocumentIface
     */
//     virtual DCOPObject * dcopObject();

    /**
     * @return the ID of the DCOP interface for this document.
     **/
//     QByteArray dcopObjectId() const;

    /**
     * Signal the progress of operations such as loading or saving.
     */
    void emitProgress(int value) {
        emit sigProgress(value);
    }

    bool isInOperation() const;
    virtual void emitBeginOperation();
    virtual void emitEndOperation();

    /**
     * Return true if url() is a real filename, false if url() is
     * an internal url in the store, like "tar:/..."
     */
    virtual bool isStoredExtern() const;

    /**
     * @return the page layout associated with this document (margins, pageSize, etc).
     * Override this if you want to provide different sized pages.
     *
     * @see KoPageLayout
     */
    virtual KoPageLayout pageLayout(int pageNumber = 0) const;

    /**
     * Performs a cleanup of unneeded backup files
     */
    void removeAutoSaveFiles();

    void setBackupFile(bool _b);

    bool backupFile()const;

    /**
     * Returns true if this document or any of its internal child documents are modified.
     */
    bool isModified() const;

    /**
     * Returns true during loading (openUrl can be asynchronous)
     */
    bool isLoading() const;

    int queryCloseExternalChildren();
    int queryCloseDia();

    /**
     * @brief Set when we do not want to save external children when saving our 'main' doc.
     *
     * This makes it possible to save 'main' doc + all its internal children first, then
     * go on to save external children. Typically used by query close.
     * Use:
     * @code
     *      doc->setDoNotSaveExtDoc();
     *      doc->save();    // saves doc and its internal children,
     *                            //also calls saveExternalChildren() which sets setDoNotSaveExtDoc(false)
     *      doc->saveExternalChildren();
     * @endcode
     */
    void setDoNotSaveExtDoc(bool on = true);

    /**
     * Sets the backup path of the document
     */
    void setBackupPath(const QString & _path);

    /**
     * @return path to the backup document
     */
    QString backupPath()const;

    /**
     * Indicates that this document is currently viewed
     * and thus should control the title caption.
     * Also resets current flag for all parents.
     */
    void setCurrent(bool on = true);

    /**
     * Sets current flag for this document and all its parents
     */
    void forceCurrent(bool on);
    bool isCurrent() const;

    void setTitleModified(const QString &caption, bool mod);

    /**
     * Sets the document URL to empty URL
     * KParts doesn't allow this, but %KOffice apps have e.g. templates
     * After using loadNativeFormat on a template, one wants
     * to set the url to KUrl()
     */
    void resetURL() {
        setUrl(KUrl()); setLocalFilePath(QString());
    }

    /**
     * Set when you want an external embedded document to be stored internally
     */
    void setStoreInternal(bool i);

    /**
     * @return true when external embedded documents are stored internally
     */
    bool storeInternal() const;

    bool hasExternURL() const;

    /**
     * @internal (public for KoMainWindow)
     */
    void setMimeTypeAfterLoading(const QString& mimeType);

    /**
     * @return returns the number of pages in the document.
     */
    virtual int pageCount() const {
        return 1;
    }

    /**
     * Returns the unit used to display all measures/distances.
     */
    KoUnit unit() const;

    /**
     * Sets the unit used to display all measures/distances.
     */
    void setUnit(KoUnit u);

    /**
     * Returns the name of the unit used to display all measures/distances.
     * Use this method for displaying it in the user interface, but use
     * unit() for everything else (conversions etc.)
     */
    QString unitName() const;

    /**
     * Set the template type used. This is used by the start up widget to show
     * the correct templates.
     */
    void setTemplateType(const QString& _templateType);
    /**
     * Template type used. This is used by the start up widget to show
     * the correct templates.
     */
    QString templateType() const;

    /**
     * Shows the init dialog when embeding
     * @param parent the parent widget
     */
    virtual bool showEmbedInitDialog(QWidget* parent);


    QList<KoVersionInfo> &versionList();

    bool loadNativeFormatFromStore(QByteArray &data);

    /**
    Adds a new version and then saves the whole document.
    * @param comment the comment for the version
    * @return true on success, otherwise false
    */
    bool addVersion(const QString& comment);

    /// return the grid data for this document.
    KoGridData &gridData();

    /// returns the guides data for this document.
    KoGuidesData &guidesData();

    void clearUndoHistory();

    using ReadWritePart::setUrl;
    using ReadWritePart::localFilePath;
    using ReadWritePart::setLocalFilePath;

public slots:
    /**
     * Initialize an empty document using default values
     */
    virtual void initEmpty();

    /**
     * Adds a command to the undo stack and executes it by calling the redo() function.
     * @param command command to add to the undo stack
     */
    virtual void addCommand(QUndoCommand* command);

    /**
     * Begins recording of a macro command. At the end endMacro needs to be called.
     * @param text command description
     */
    virtual void beginMacro(const QString & text);

    /**
     * Ends the recording of a macro command.
     */
    virtual void endMacro();

    /**
     *  Sets the modified flag on the document. This means that it has
     *  to be saved or not before deleting it.
     */
    virtual void setModified(bool _mod);

    /**
     * Called by the undo stack when the document is saved or all changes has been undone
     * @param clean if the document's undo stack is clean or not
     */
    virtual void setDocumentClean(bool clean);

signals:

    /**
     * This signal is emitted when the unit is changed by setUnit().
     * It is common to connect views to it, in order to change the displayed units
     * (e.g. in the rulers)
     */
    void unitChanged(KoUnit);

    /**
     * This signal is emitted when a direct or indirect child document changes
     * and needs to be updated in all views.
     *
     * If one of your child documents emits the childChanged signal, then you may
     * usually just want to redraw this child. In this case you can ignore the parameter
     * passed by the signal.
     */
    void childChanged(KoDocumentChild *child);

    /**
     * Progress info while loading or saving. The value is in percents (i.e. a number between 0 and 100)
     * Your KoDocument-derived class should emit the signal now and then during load/save.
     * KoMainWindow will take care of displaying a progress bar automatically.
     */
    void sigProgress(int value);

    /**
     * Emitted e.g. at the beginning of a save operation
     * This is emitted by KoDocument and used by KoView to display a statusbar message
     */
    void statusBarMessage(const QString& text);

    /**
     * Emitted e.g. at the end of a save operation
     * This is emitted by KoDocument and used by KoView to clear the statusbar message
     */
    void clearStatusBarMessage();

    void beginOperation();
    void endOperation();

    /**
    * Emitted when the document is modified
    */
    void modified(bool);

    void closeEmbedInitDialog();

protected slots:
    /**
     * This slot loads an existing file and deletes the start up widget.
     * @param url the file to load
     */
    virtual void openExistingFile(const KUrl& url);
    /**
     * This slot loads a template and deletes the start up widget.
     * @param url the template to load
     */
    virtual void openTemplate(const KUrl& url);

protected:
    /**
     * Struct used in the list created by createCustomDocumentWidgets()
     */
    struct CustomDocumentWidgetItem {
        /// Pointer to the custom document widget
        QWidget* widget;
        /// title used in the sidebar. If left empty it will be displayed as "Custom Document"
        QString title;
        /// icon used in the sidebar. If left empty it will use the unknown icon
        QString icon;
    };

    /**
        Generate a name for the document.
    */
    QString newObjectName();

    QString autoSaveFile(const QString & path) const;

    virtual KoView *createViewInstance(QWidget *parent) = 0;

    /**
     *  Loads a document from KReadOnlyPart::m_file (KParts takes care of downloading
     *  remote documents).
     *  Applies a filter if necessary, and calls loadNativeFormat in any case
     *  You should not have to reimplement, except for very special cases.
     *
     * This method is called from the KReadOnlyPart::openUrl method.
     */
    virtual bool openFile();

    /**
     *  Saves a document to KReadOnlyPart::m_file (KParts takes care of uploading
     *  remote documents)
     *  Applies a filter if necessary, and calls saveNativeFormat in any case
     *  You should not have to reimplement, except for very special cases.
     */
    virtual bool saveFile();

    /**
     * Override this method in your derived class to show a widget in the startup 'dialog'.
     * This widget should allow the user to set settings for a custom document (i.e. one
     * not based on a template).
     * The returned widget should provide its own button (preferrably 'Create') and
     * implement the logic to implement the document instance correctly.
     * After initializing the widget should emit a signal called 'documentSelected()' which
     * will remove the startupWidget and show the document.
     * @param parent the parent of the to be created widget.
     * @return a list of KoDocument::CustomDocumentWidgetItem.
     */
    virtual QList<CustomDocumentWidgetItem> createCustomDocumentWidgets(QWidget *parent);

    /**
     *  OLD XML method. For OASIS just call KoDocumentChild::loadOasisDocument
     *  after KoDocumentChild::loadOasis.
     *
     *  You need to overload this function if your document may contain
     *  embedded documents. This function is called to load embedded documents.
     *
     *  An example implementation may look like this:
     *  @code
     *  QPtrListIterator<KoDocumentChild> it( children() );
     *  for( ; it.current(); ++it )
     *  {
     *    if ( !it.current()->loadDocument( _store ) )
     *    {
     *      return false;
     *    }
     *  }
     *  return true;
     *  @endcode
     */
    virtual bool loadChildren(KoStore*);

    /**
     *  Saves all internal children (only!).
     *  @see saveExternalChildren if you have external children.
     *  Returns true on success.
     */
    virtual bool saveChildren(KoStore* store);

    /**
     *  Saves all internal children (only!), to the store, using the OASIS format.
     *  This is called automatically during saveNativeFormat.
     *  @see saveExternalChildren if you have external children.
     *  Returns true on success.
     */
    virtual bool saveChildrenOdf(SavingContext & documentContext);

    /**
     *  Overload this function if you have to load additional files
     *  from a store. This function is called after loadXML()
     *  and after loadChildren() have been called.
     */
    virtual bool completeLoading(KoStore* store);

    /**
     *  If you want to write additional files to a store,
     *  then you must do it here.
     *  In the implementation, you should prepend the document
     *  url (using url().url()) before the filename, so that everything is kept relative
     *  to this document. For instance it will produce urls such as
     *  tar:/1/pictures/picture0.png, if the doc url is tar:/1
     *  But do this ONLY if the document is not stored extern (see isStoredExtern() ).
     *  If it is, then the pictures should be saved to tar:/pictures.
     */
    virtual bool completeSaving(KoStore* store);

    /**
     * Inserts the new child in the list of children and emits the
     * childChanged() signal.
     *
     * At the same time this method marks this document as modified.
     *
     * To remove a child, just delete it. KoDocument will detect this
     * and remove the child from its lists.
     *
     * @see #isModified
     */
    virtual void insertChild(KoDocumentChild *child);

    /** @internal */
    virtual void setModified() {
        KParts::ReadWritePart::setModified();
    }

    KoPageLayout m_pageLayout;

    /**
     *  Saves all externally stored children.
     *  Returns true on success.
     * @see #saveChildren for internal children
     */
    virtual bool saveExternalChildren();

    /**
     *  Returns whether or not the current openUrl() or openFile() call is
     *  actually an import operation (like File --> Import).
     *  This is for informational purposes only.
     */
    bool isImporting() const;

    /**
     *  Returns whether or not the current saveFile() call is actually an export
     *  operation (like File --> Export).
     *  If this function returns true during saveFile() and you are changing
     *  some sort of state, you _must_ restore it before the end of saveFile();
     *  otherwise, File --> Export will not work properly.
     */
    bool isExporting() const;

    /**
     * Creates the open widget showed at application start up.
     * @param parent the parent widget
     * @param instance the KComponentData to be used for KConfig data
     * @param templateType the template-type (group) that should be selected on creation.
     */
    KoOpenPane* createOpenPane(QWidget* parent, const KComponentData &instance,
                               const QString& templateType = QString());

private slots:
    void slotChildChanged(KoChild *c);
    void slotChildDestroyed();
    void slotAutoSave();
    void slotStarted(KIO::Job*);
    void startCustomDocument();
    /**
     * Removes the open widget showed at application start up.
     */
    void deleteOpenPane();

private:
    virtual void insertChild(QObject *) {
        Q_ASSERT(0);
    } // avoid compiler warning, but don't call deprecated insertChild() in QObject

    KService::Ptr nativeService();
    bool oldLoadAndParse(KoStore* store, const QString& filename, KoXmlDocument& doc);
    bool loadNativeFormatFromStore(const QString& file);
    bool loadNativeFormatFromStoreInternal(KoStore * store);

    bool savePreview(KoStore* store);
    bool saveOasisPreview(KoStore* store, KoXmlWriter* manifestWriter);
    class Private;
    Private * const d;
    KService::Ptr m_nativeService;
    bool m_bEmpty;
    static Q3PtrList<KoDocument> *s_documentList;
    static const int s_defaultAutoSave;
};

#endif
