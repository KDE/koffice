/*
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *                2006 Boudewijn Rempt <boud@valdyas.org>
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
#ifndef KIS_IMAGE_H_
#define KIS_IMAGE_H_

#include <qobject.h>
#include <qstring.h>
#include <qvaluevector.h>
#include <qmutex.h>

#include <config.h>

#include <ksharedptr.h>
#include <kurl.h>

#include "KoUnit.h"

#include "kis_composite_op.h"
#include "kis_global.h"
#include "kis_types.h"
#include "kis_annotation.h"
#include "kis_paint_device.h"

#include <koffice_export.h>


class DCOPObject;
class KCommand;

class KoCommandHistory;

class KisColorSpace;
class KisNameServer;
class KisUndoAdapter;
class KisPainter;
class KCommand;
class KisColor;
class KisFilterStrategy;
class KisImageIface;
class KisProfile;
class KisProgressDisplayInterface;
class KisPaintLayer;
class KisPerspectiveGrid;

class KRITACORE_EXPORT KisImage : public QObject, public KShared {
    Q_OBJECT

public:
    KisImage(KisUndoAdapter * adapter, Q_INT32 width, Q_INT32 height, KisColorSpace * colorSpace, const QString& name);
    KisImage(const KisImage& rhs);
    virtual ~KisImage();
    virtual DCOPObject *dcopObject();

public:
    typedef enum enumPaintFlags {
        PAINT_IMAGE_ONLY = 0,
        PAINT_BACKGROUND = 1,
        PAINT_SELECTION = 2,
        PAINT_MASKINACTIVELAYERS = 4,
        PAINT_EMBEDDED_RECT = 8 // If the current layer is an embedded part draw a rect around it
    } PaintFlags;

    /// Paint the specified rect onto the painter, adjusting the colors using the
    /// given profile. The exposure setting is used if the image has a high dynamic range.
    virtual void renderToPainter(Q_INT32 x1,
                    Q_INT32 y1,
                    Q_INT32 x2,
                    Q_INT32 y2,
                    QPainter &painter,
                    KisProfile *profile,
                    PaintFlags paintFlags,
                    float exposure = 0.0f);
    /**
    * Render the projection onto a QImage. In contrast with the above method, the
    * selection is not rendered.
    */
    virtual QImage convertToQImage(Q_INT32 x1,
                                    Q_INT32 y1,
                                    Q_INT32 x2,
                                    Q_INT32 y2,
                                    KisProfile * profile,
                                    float exposure = 0.0f);

    virtual QImage convertToQImage(const QRect& r, const QSize& fullImageSize, KisProfile *profile, PaintFlags paintFlags, float exposure = 0.0f);

    KisBackgroundSP background() const;
    KisSubstrateSP substrate() const;
    

public:

    /**
    * Lock the image to make sure no recompositing-causing signals get emitted
    * while you're messing with the layers. Don't forget to unlock again.
    */
    void lock();

    /**
    * Unlock the image to make sure the rest of Krita learns about changes in the image
    * again. If the rootLayer is dirty on unlocking, an imgUpdated signal is sent out
    * immediately.
    */
    void unlock();

    /**
    * Returns true if the image is locked.
    */
    bool locked() const;

    KisColor backgroundColor() const;
    void setBackgroundColor(const KisColor & color);

    QString name() const;
    void setName(const QString& name);

    QString description() const;
    void setDescription(const QString& description);

    QString nextLayerName() const;
    void rollBackLayerName();
    
    KisPerspectiveGrid* perspectiveGrid();
    void createPerspectiveGrid(QPoint topLeft, QPoint topRight, QPoint bottomRight, QPoint bottomLeft);
    /**
    * Resize the image to the specified width and height. The resize
    * method handles the creating on an undo step itself.
    *
    * @param w the new width of the image
    * @param h the new height of the image
    * @param x the x position of the crop on all layer if cropLayers is true
    * @param y the y position of the crop on all layer if cropLayers is true
    * @param cropLayers if true, all layers are cropped to the new size.
    */
    void resize(Q_INT32 w, Q_INT32 h, Q_INT32 x = 0, Q_INT32 y = 0,  bool cropLayers = false);

    /**
    * Resize the image to the specified width and height. The resize
    * method handles the creating on an undo step itself.
    *
    * @param rc the rect describing the new width and height of the image
    * @param cropLayers if true, all layers are cropped to the new rect
    */
    void resize(const QRect& rc, bool cropLayers = false);

    void scale(double sx, double sy, KisProgressDisplayInterface *m_progress, KisFilterStrategy *filterStrategy);
    void rotate(double angle, KisProgressDisplayInterface *m_progress);
    void shear(double angleX, double angleY, KisProgressDisplayInterface *m_progress);

    /**
    * Convert the image and all its layers to the dstColorSpace
    */
    void convertTo(KisColorSpace * dstColorSpace, Q_INT32 renderingIntent = INTENT_PERCEPTUAL);

    // Get the profile associated with this image
    KisProfile *  getProfile() const;

    /**
    * Set the profile of the image to the new profile and do the same for
    * all layers that have the same colorspace and profile as the image.
    * It doesn't do any pixel conversion.
    *
    * This is essential if you have loaded an image that didn't
    * have an embedded profile to which you want to attach the right profile.
    */

    void setProfile(const KisProfile * profile);

    /**
    * Replace the current undo adapter with the specified undo adapter.
    * The current undo adapter will _not_ be deleted.
    */
    void setUndoAdapter(KisUndoAdapter * undoAdapter);

    /**
    * Returns the current undo adapter. You can add new commands to the
    * undo stack using the adapter
    */
    KisUndoAdapter *undoAdapter() const;

    /**
    * Returns true if this image wants undo information, false otherwise
    */
    bool undo() const;
    /**
    * Tell the image it's modified; this emits the sigImageModified signal. This happens
    *  when the image needs to be saved
    */
    void setModified();

    KisColorSpace * colorSpace() const;

    // Resolution of the image == XXX: per inch?
    double xRes();
    double yRes();
    void setResolution(double xres, double yres);

    Q_INT32 width() const;
    Q_INT32 height() const;

    bool empty() const;

    /**
    *  returns a paintdevice that contains the merged layers of this image, within
    * the bounds of this image (with the colorspace and profile of this image)
    */
    KisPaintDeviceSP mergedImage();

    /*
    * Returns the colour of the merged image at pixel (x, y).
    */
    KisColor mergedPixel(Q_INT32 x, Q_INT32 y);

    /// Creates a new paint layer with the specified properties, adds it to the image, and returns it.
    KisLayerSP newLayer(const QString& name, Q_UINT8 opacity,
                            const KisCompositeOp& compositeOp = KisCompositeOp(), KisColorSpace * colorstrategy = 0);

    /// Get the active painting device. Returns 0 if the active layer does not have a paint device.
    KisPaintDeviceSP activeDevice();

    void setLayerProperties(KisLayerSP layer, Q_UINT8 opacity, const KisCompositeOp& compositeOp, const QString& name);

    KisGroupLayerSP rootLayer() const;
    KisLayerSP activeLayer() const;

    /// Return the projection; that is, the complete, composited representation
    /// of this image.
    KisPaintDeviceSP projection();
    
    KisLayerSP activate(KisLayerSP layer);
    KisLayerSP findLayer(const QString& name) const;
    KisLayerSP findLayer(int id) const;

    /// Move layer to specified position
    bool moveLayer(KisLayerSP layer, KisGroupLayerSP parent, KisLayerSP aboveThis);

    /**
    * Add an already existing layer to the image. The layer is put on top
    * of the layers in the specified layergroup
    * @param layer the layer to be added
    * @param parent the parent layer
    */
    bool addLayer(KisLayerSP layer, KisGroupLayerSP parent);
    
    /**
    * Add already existing layer to image.
    *
    * @param layer the layer to be added
    * @param parent the parent layer
    * @param aboveThis in the list with child layers of the specified
    *                  parent, add this layer above the specified sibling.
    *                  if 0, the layer is put in the lowermost position in
    *                  its group.
    * @param notify If true, the image is immediately recomposited, if false,
    *               no recomposition is done yet. The added layer is all
    *
    * returns false if adding the layer didn't work, true if the layer got added
    */
    bool addLayer(KisLayerSP layer, KisGroupLayerSP parent, KisLayerSP aboveThis);

    /// Remove layer
    bool removeLayer(KisLayerSP layer);

    /// Move layer up one slot
    bool raiseLayer(KisLayerSP layer);

    /// Move layer down one slot
    bool lowerLayer(KisLayerSP layer);

    /// Move layer to top slot
    bool toTop(KisLayerSP layer);

    /// Move layer to bottom slot
    bool toBottom(KisLayerSP layer);

    Q_INT32 nlayers() const;
    Q_INT32 nHiddenLayers() const;

    KCommand *raiseLayerCommand(KisLayerSP layer);
    KCommand *lowerLayerCommand(KisLayerSP layer);
    KCommand *topLayerCommand(KisLayerSP layer);
    KCommand *bottomLayerCommand(KisLayerSP layer);

    /**
    * Merge all visible layers and discard hidden ones.
    * The resulting layer will be activated.
    */
    void flatten();

    /**
    * Merge the specified layer with the layer
    * below this layer, remove the specified layer.
    */
    void mergeLayer(KisLayerSP l);

    QRect bounds() const;

    /// use if the layers have changed _completely_ (eg. when flattening)
    void notifyLayersChanged();

    void notifyPropertyChanged(KisLayerSP layer);

    void notifyImageLoaded();

    void notifyLayerUpdated(KisLayerSP layer, QRect rc);

    void setColorSpace(KisColorSpace * colorSpace);
    void setRootLayer(KisGroupLayerSP rootLayer);

    //KisGuideMgr *guides() const;

    /**
    * Add an annotation for this image. This can be anything: Gamma, EXIF, etc.
    * Note that the "icc" annotation is reserved for the colour strategies.
    * If the annotation already exists, overwrite it with this one.
    */
    void addAnnotation(KisAnnotationSP annotation);

    /** get the annotation with the given type, can return 0 */
    KisAnnotationSP annotation(QString type);

    /** delete the annotation, if the image contains it */
    void removeAnnotation(QString type);

    /**
    * Start of an iteration over the annotations of this image (including the ICC Profile)
    */
    vKisAnnotationSP_it beginAnnotations();

    /** end of an iteration over the annotations of this image */
    vKisAnnotationSP_it endAnnotations();

signals:

    void sigActiveSelectionChanged(KisImageSP image);
    void sigSelectionChanged(KisImageSP image);

    /// Emitted after a different layer is made active.
    void sigLayerActivated(KisLayerSP layer);

    /// Emitted after a layer is added: you can find out where by asking it for its parent(), et al.
    void sigLayerAdded(KisLayerSP layer);

    /** Emitted after a layer is removed.
        It's no longer in the image, but still exists, so @p layer is valid.

        @param layer the removed layer
        @param parent the parent of the layer, before it was removed
        @param wasAboveThis the layer it was above, before it was removed.
    */
    void sigLayerRemoved(KisLayerSP layer, KisGroupLayerSP wasParent, KisLayerSP wasAboveThis);

    /** Emitted after a layer is moved to a different position under its parent layer, or its parent changes.

        @param previousParent the parent of the layer, before it was moved
        @param wasAboveThis the layer it was above, before it was moved.
    */
    void sigLayerMoved(KisLayerSP layer, KisGroupLayerSP previousParent, KisLayerSP wasAboveThis);

    /// Emitted after a layer's properties (visible, locked, opacity, composite op, name, ...) change
    void sigLayerPropertiesChanged(KisLayerSP layer);

    /** Emitted when the list of layers has changed completely.
        This means e.g. when the image is flattened, but not when it is rotated,
        as the layers only change internally then.
    */
    void sigLayersChanged(KisGroupLayerSP rootLayer);

    /**
    * Emitted whenever an action has caused the image to be recomposited. This happens
    * after calls to recomposite().
    *
    * @param rc The rect that has been recomposited.
    */
    void sigImageUpdated(QRect rc);

    /**
    * Emitted whenever a layer is modified.
    *
    * @param layer The layer that has been modified.
    * @param rc The rectangle that has been modified.
    */
    void sigLayerUpdated(KisLayerSP layer, QRect rc);

    /**
    * Emitted whenever the image has been modified, so that it doesn't match with the version saved on disk.
    */
    void sigImageModified();

    void sigSizeChanged(Q_INT32 w, Q_INT32 h);
    void sigProfileChanged(KisProfile *  profile);
    void sigColorSpaceChanged(KisColorSpace*  cs);


public slots:
    void slotSelectionChanged();
    void slotSelectionChanged(const QRect& r);


private:
    KisImage& operator=(const KisImage& rhs);
    void init(KisUndoAdapter * adapter, Q_INT32 width, Q_INT32 height,  KisColorSpace * colorSpace, const QString& name);
    void emitSizeChanged();

private:

    KURL m_uri;
    QString m_name;
    QString m_description;

    Q_INT32 m_width;
    Q_INT32 m_height;

    double m_xres;
    double m_yres;

    KoUnit::Unit m_unit;

    KisColorSpace * m_colorSpace;

    bool m_dirty;
    QRect m_dirtyRect;

    KisBackgroundSP m_bkg;

    KisGroupLayerSP m_rootLayer; // The layers are contained in here
    KisLayerSP m_activeLayer;

    KisNameServer *m_nserver;
    KisUndoAdapter *m_adapter;
    //KisGuideMgr m_guides;

    DCOPObject *m_dcop;

    vKisAnnotationSP m_annotations;

    class KisImagePrivate;
    KisImagePrivate * m_private;
    
};

#endif // KIS_IMAGE_H_
