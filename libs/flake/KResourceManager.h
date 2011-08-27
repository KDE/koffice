/*
   Copyright (c) 2006 Boudewijn Rempt (boud@valdyas.org)
   Copyright (C) 2007, 2009-2011 Thomas Zander <zander@kde.org>
   Copyright (c) 2008 Carlos Licea <carlos.licea@kdemail.net>

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
   Boston, MA 02110-1301, USA.
 */
#ifndef K_RESOURCEMANAGER_H
#define K_RESOURCEMANAGER_H

#include <QObject>
#include <QList>
#include <QVariant>
#include <QSizeF>
#include <QColor>

#include "flake_export.h"

#include <KUnit.h>

class KShape;
class KLineBorder;
class KUndoStack;
class KImageCollection;
class KOdfDocumentBase;
class QTextDocument;

/**
 * The KoCanvasResource contains a set of per-canvas
 * properties, like current foreground color, current background
 * color and more.
 * \sa KResourceManager KCanvasBase::resourceManager()
 */
namespace KoCanvasResource
{

/**
 * This enum holds identifiers to the resources that can be stored in here.
 */
enum CanvasResource {
    ForegroundColor,    ///< The active forground color selected for this canvas.
    BackgroundColor,    ///< The active background (KShapeBackgroundBase) selected for this canvas.
    HandleRadius,       ///< The handle radius (in pixels) used for drawing handles of any kind
    GrabSensitivity,    ///< The grab sensitivity (in pixels) used for grabbing handles of any kind
    PageSize,           ///< The size of the (current) page in postscript points (QSizeF).
    Unit,               ///< The unit of this canvas (KUnit)
    CurrentPage,        ///< The current page number (int)
    ActiveColorTarget,    ///< the active color target type see KFlake::StyleType for valid values
    ArtworkStart = 1000,      ///< Base number for artwork specific values.
    KivioStart = 3000,       ///< Base number for kivio specific values.
    ShowcaseStart = 5000,  ///< Base number for Showcase specific values.
    KCellsStart = 7000,     ///< Base number for kcells specific values.
    KWordStart = 8000,        ///< Base number for kword specific values.
    KoPageAppStart = 9000    ///< Base number for KoPageApp specific values.
};

}

/**
 * The KoDocumentResource contains a set of per-document
 * properties.
 * \sa KResourceManager KoShapeCollection::resourceManager()
 */
namespace KoDocumentResource
{
/**
 * This enum holds identifiers to the resources that can be stored in here.
 */
enum DocumentResource {
    UndoStack,              ///< The document-wide undo stack (KUndoStack)
    ImageCollection,        ///< The KImageCollection for the document
    OdfDocument,            ///< The document this canvas shows (KOdfDocumentBase)
    TextDocuments,          ///< A list of all the QTextDocument instances of a document.

    ArtworkStart = 1000,      ///< Base number for artwork specific values.
    KivioStart = 3000,       ///< Base number for kivio specific values.
    ShowcaseStart = 5000,  ///< Base number for Showcase specific values.
    KCellsStart = 7000,     ///< Base number for kcells specific values.
    KWordStart = 8000,       ///< Base number for kword specific values.
    KoPageAppStart = 9000,   ///< Base number for KoPageApp specific values.
    KoTextStart = 10000      ///< Base number for KOdfText specific values.
};
}

/**
 * The KResourceManager contains a set of per-canvas
 * properties, like current foreground color, current background
 * color and more. All tools belonging to the current canvas are
 * notified when a Resource changes (is set).
 * The properties come from the KoCanvasResource::CanvasResource enum or the
 * KoDocumentResource::DocumentResource depending on which manager you got.
 * See KCanvasBase::resourceManager KShapeController::resourceManager
 *
 * The manager can contain all sorts of variable types and there are accessors
 * for the most common ones.  All variables are always stored inside a QVariant
 * instance internally and you can always just use the resource() method to get
 * that directly.
 * The way to store arbitairy data objects that are stored as pointers you can use
 * the following code snippets;
 * @code
 *  QVariant variant;
 *  variant.setValue<void*>(textShapeData->document());
 *  resourceManager->setResource(KOdfText::CurrentTextDocument, variant);
 *  // and get it out again.
 *  QVariant var = resourceManager->resource(KOdfText::CurrentTextDocument);
 *  document = static_cast<QTextDocument*>(var.value<void*>());
 * @endcode
 */
class FLAKE_EXPORT KResourceManager : public QObject
{
    Q_OBJECT

public:

    /**
     * Constructor.
     * @param parent the parent QObject, used for memory management.
     */
    explicit KResourceManager(QObject *parent = 0);
    ~KResourceManager();

    /**
     * Set a resource of any type.
     * @param key the integer key
     * @param value the new value for the key.
     * @see KoCanvasResource::CanvasResource KoDocumentResource::DocumentResource
     */
    void setResource(int key, const QVariant &value);

    /**
     * Set a resource of type KShape*.
     * @param key the integer key
     * @param shape the new value for the key.
     * @see KoCanvasResource::CanvasResource KoDocumentResource::DocumentResource
     */
    void setResource(int key, KShape *shape);

    /**
     * Set a resource of type KUnit
     * @param key the integer key
     * @param unit the new value for the key.
     * @see KoCanvasResource::CanvasResource KoDocumentResource::DocumentResource
     */
    void setResource(int key, const KUnit &unit);

    /**
     * Returns a qvariant containing the specified resource or a standard one if the
     * specified resource does not exist.
     * @param key the key
     * @see KoCanvasResource::CanvasResource KoDocumentResource::DocumentResource
     */
    QVariant resource(int key) const;

    /**
     * Set the foregroundColor resource.
     * @param color the new foreground color
     */
    void setForegroundColor(const QColor &color);

    /**
     * Return the foregroundColor
     */
    QColor foregroundColor() const;

    /**
     * Set the backgroundColor resource.
     * @param color the new background color
     */
    void setBackgroundColor(const QColor &color);
    /**
     * Return the backgroundColor
     */
    QColor backgroundColor() const;

    /**
     * Tools that provide a handle for controlling the content that the tool can edit can
     * use this property to alter the radius that a circular handle should have on screen.
     * @param handleSize the radius in pixels.
     */
    void setHandleRadius(int handleSize);
    /// Returns the actual handle radius
    int handleRadius() const;

    /**
     * Tools that are used to grab handles or similar with the mouse
     * should use this value to determine if the mouse is near enough
     * @param grabSensitivity the grab sensitivity in pixels
     */
    void setGrabSensitivity(int grabSensitivity);
    /// Returns the actual grab sensitivity
    int grabSensitivity() const;

    /**
     * Return the resource determined by param key as a boolean.
     * @param key the indentifying key for the resource
     * @see KoCanvasResource::CanvasResource KoDocumentResource::DocumentResource
     */
    bool boolResource(int key) const;

    /**
     * Return the resource determined by param key as an integer.
     * @param key the indentifying key for the resource
     * @see KoCanvasResource::CanvasResource KoDocumentResource::DocumentResource
     */
    int intResource(int key) const;

    /**
     * Return the resource determined by param key as a QColor.
     * @param key the indentifying key for the resource
     * @see KoCanvasResource::CanvasResource KoDocumentResource::DocumentResource
     */
    QColor colorResource(int key) const;

    /**
     * Return the resource determined by param key as a pointer to a KShape.
     * @param key the indentifying key for the resource
     * @see KoCanvasResource::CanvasResource KoDocumentResource::DocumentResource
     */
    KShape *koShapeResource(int key) const;

    /**
     * Return the resource determined by param key as a QString .
     * @param key the indentifying key for the resource
     * @see KoCanvasResource::CanvasResource KoDocumentResource::DocumentResource
     */
    QString stringResource(int key) const;

    /**
     * Return the resource determined by param key as a QSizeF.
     * @param key the indentifying key for the resource
     * @see KoCanvasResource::CanvasResource KoDocumentResource::DocumentResource
     */
    QSizeF sizeResource(int key) const;

    /**
     * Return the resource determined by param key as a KUnit.
     * @param key the indentifying key for the resource
     * @see KoCanvasResource::CanvasResource KoDocumentResource::DocumentResource
     */
    KUnit unitResource(int key) const;

    /**
     * Returns true if there is a resource set with the requested key.
     * @param key the indentifying key for the resource
     * @see KoCanvasResource::CanvasResource KoDocumentResource::DocumentResource
     */
    bool hasResource(int key) const;

    /**
     * Remove the resource with @p key from the provider.
     * @param key the key that will be used to remove the resource
     * There will be a signal emitted with a variable that will return true on QVariable::isNull();
     * @see KoCanvasResource::CanvasResource KoDocumentResource::DocumentResource
     */
    void clearResource(int key);

    /**
     * Register a key with a slot to fetch on demand.
     * For usecases where the creation of a resource is expensive the actual
     * value of a resource can be lazily calculated by using a callback-slot.
     * How this works is that instead of setting the value using setValue(),
     * the caller calls setLazyResourceSlot() instead. The @param key is unchanged
     * but the object/slot arguments are used to call a method on the target object
     * the first time the resource is requested.
     * A typicall implementation would be like;
     * @code
     *    manager->setLazyResourceSlot(KoDocumentResource::ImageCollection,
     *        this, "createImageCollection");
     * @endcode
     * And the object would create a slot using the normal Qt concepts;
     * @code
     * public slots:
     *      void createImageCollection(KResourceManager *manager) {
     *          manager->setImageCollection(new KImageCollection(manager));
     *      }
     * @endcode
     * Notice that the method is required to take an argumennt of KResourceManager* and
     * the name of the method is passed without arguments (which is notably different
     * from usage of the SLOT macro)
     *
     * @param key the key that will be used to trigger the slot.
     * @param object the target object that will be used for the callback.
     * @param slot the name of the method, without arguments, to call.
     */
    void setLazyResourceSlot(int key, QObject *object, const char *slot);

    KUndoStack *undoStack() const;
    void setUndoStack(KUndoStack *undoStack);

    KImageCollection *imageCollection() const;
    void setImageCollection(KImageCollection *ic);

    KOdfDocumentBase *odfDocument() const;
    void setOdfDocument(KOdfDocumentBase *currentDocument);

    void setTextDocumentList(const QList<QTextDocument *> &allDocuments);
    QList<QTextDocument *> textDocumentList() const;

signals:
    /**
     * This signal is emitted every time a resource is set that is either
     * new or different from the previous set value.
     * @param key the indentifying key for the resource
     * @param value the variants new value.
     * @see KoCanvasResource::CanvasResource KoDocumentResource::DocumentResource
     */
    void resourceChanged(int key, const QVariant &value);

private:
    KResourceManager(const KResourceManager&);
    KResourceManager& operator=(const KResourceManager&);

    class Private;
    Private *const d;
};

#endif
