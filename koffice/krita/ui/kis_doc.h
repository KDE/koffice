/*
 *  Copyright (c) 1999-2000 Matthias Elter  <me@kde.org>
 *  Copyright (c) 2001 Toshitaka Fujioka  <fujioka@kde.org>
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#ifndef KIS_DOC_H_
#define KIS_DOC_H_

#include <kdebug.h>

#include <KoDocument.h>

#include "kis_types.h"
#include "kis_undo_adapter.h"

#include <koffice_export.h>

class QImage;
class QString;

class DCOPObject;
class KCommand;

class KoCommandHistory;
class KMacroCommand;

class KisProfile;
class KisView;
class KisNameServer;
class KisChildDoc;
class KisColorSpace;
class KisColor;
class KisCompositeOp;

class KRITACORE_EXPORT KisDoc : public KoDocument, private KisUndoAdapter {

    typedef KoDocument super;
    Q_OBJECT

public:
    KisDoc(QWidget *parentWidget = 0, const char *widgetName = 0, QObject* parent = 0, const char* name = 0, bool singleViewMode = false);
    virtual ~KisDoc();

public:
    // Overide KoDocument
    virtual bool wantExportConfirmation() const { return false; };
    virtual bool completeLoading(KoStore *store);
    virtual bool completeSaving(KoStore*);
    virtual DCOPObject* dcopObject();
    virtual bool initDoc(InitDocFlags flags, QWidget* parentWidget=0);
    virtual bool loadOasis( const QDomDocument&, KoOasisStyles&, const QDomDocument&, KoStore* );
    virtual bool saveOasis( KoStore*, KoXmlWriter* );
    virtual bool loadChildren( KoStore* store);
    virtual bool loadXML(QIODevice *, const QDomDocument& doc);
    virtual QCString mimeType() const;
    virtual QWidget* createCustomDocumentWidget(QWidget *parent);
    virtual KoDocument* hitTest(const QPoint &pos, const QWMatrix& matrix = QWMatrix());

    /**
     * Draw the image embedded in another KOffice document
     *
     * XXX: Use of transparent, zoomX and zoomY is not supported
     *      by Krita because we appear to be doing our zooming
     *      elsewhere. This may affect KOffice compatibility.
     */
    virtual void paintContent(QPainter& painter, const QRect& rect, bool /*transparent*/, double /*zoomX*/, double /*zoomY*/);

    virtual QDomDocument saveXML();

public slots:
            
        
    /**
     * Initialize an empty document using default values
     * @since 1.5
     */
     virtual void initEmpty();

private: // Undo adapter
    
    virtual void setCommandHistoryListener(const KisCommandHistoryListener *);
    virtual void removeCommandHistoryListener(const KisCommandHistoryListener *);

    virtual KCommand * presentCommand();
    virtual void addCommand(KCommand *cmd);
    virtual void setUndo(bool undo);
    virtual bool undo() const;
    virtual void beginMacro(const QString& macroName);
    virtual void endMacro();


public:


    Q_INT32 undoLimit() const;
    void setUndoLimit(Q_INT32 limit);

    Q_INT32 redoLimit() const;
    void setRedoLimit(Q_INT32 limit);

    /**
     * Create a new image that has this document as a parent and
     * replace the current image with this image.
     */
    bool newImage(const QString& name, Q_INT32 width, Q_INT32 height, KisColorSpace * cs, const KisColor &bgColor, const QString &imgDescription, const double imgResolution);

    /**
     * Create a new image that has this document as a parent and
     * replace the current image with this image.
     */
    KisImageSP newImage(const QString& name, Q_INT32 width, Q_INT32 height, KisColorSpace * colorstrategy);

    void renameImage(const QString& oldName, const QString& newName);


    /**
     * Adds the specified child document to this document; this
     * is not done with KoDocument::insertChild() because that
     * is protected and cannot be called from KisView.
     */
    KisChildDoc * createChildDoc( const QRect& rect, KoDocument* childDoc );

    /**
     * Makes an otherwise empty document ready for import/export
     */
    void prepareForImport();

    KisImageSP currentImage();

    /**
     * Set the current image to the specified image and turn undo on.
     */
    void setCurrentImage(KisImageSP image);

    KisUndoAdapter * undoAdapter() { return this; }

public slots:
    void slotImageUpdated();
    void slotImageUpdated(const QRect& rect);
    void slotDocumentRestored();
    void slotCommandExecuted(KCommand *command);

signals:
    void docUpdated();
    void docUpdated(QRect rect);
    void loadingFinished();

    /*
     * Emitted every time a command is added to the undo history, or executed
     * due to an undo or redo action.
     */
    void sigCommandExecuted();

protected:
    // Overide KoDocument
    virtual KoView* createViewInstance(QWidget *parent, const char *name);

protected slots:
    // Overide KoDocument
    virtual void openExistingFile(const QString& file);
    virtual void openTemplate(const QString& file);

private slots:
    void slotUpdate(KisImageSP img, Q_UINT32 x, Q_UINT32 y, Q_UINT32 w, Q_UINT32 h);
    void slotIOProgress(Q_INT8 percentage);
    
private:
    
    QDomElement saveImage(QDomDocument& doc, KisImageSP img);
    KisImageSP loadImage(const QDomElement& elem);
    void loadLayers(const QDomElement& element, KisImageSP img, KisGroupLayerSP parent);
    KisLayerSP loadLayer(const QDomElement& elem, KisImageSP img);
    KisLayerSP loadPaintLayer(const QDomElement& elem, KisImageSP img,
                QString name, Q_INT32 x, Q_INT32 y, Q_INT32 opacity, bool visible, bool locked,
                KisCompositeOp compositeOp);
    KisGroupLayerSP loadGroupLayer(const QDomElement& elem, KisImageSP img,
                QString name, Q_INT32 x, Q_INT32 y, Q_INT32 opacity, bool visible, bool locked,
                KisCompositeOp compositeOp);
    KisAdjustmentLayerSP loadAdjustmentLayer(const QDomElement& elem, KisImageSP img,
                                             QString name, Q_INT32 x, Q_INT32 y, Q_INT32 opacity, bool visible, bool locked,
                                             KisCompositeOp compositeOp);
    KisPartLayerSP loadPartLayer(const QDomElement& elem, KisImageSP img,
                                  QString name, Q_INT32 x, Q_INT32 y, Q_INT32 opacity,
                                  bool visible, bool locked, KisCompositeOp compositeOp);
    bool init();

    void setIOSteps(Q_INT32 nsteps);
    void IOCompletedStep();
    void IODone();

private:

    bool m_undo;
    KoCommandHistory *m_cmdHistory;
    QPtrList<KisCommandHistoryListener> m_undoListeners;
    KisImageSP m_currentImage;
    DCOPObject *m_dcop;
    KisNameServer *m_nserver;
    KMacroCommand *m_currentMacro;
    Q_INT32 m_macroNestDepth;
    Q_INT32 m_conversionDepth;
    int m_ioProgressTotalSteps;
    int m_ioProgressBase;
    QMap<KisLayerSP, QString> m_layerFilenames; // temp storage during load
    
};

#endif // KIS_DOC_H_

