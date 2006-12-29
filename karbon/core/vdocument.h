/* This file is part of the KDE project
   Copyright (C) 2001-2005, The Karbon Developers
   Copyright (C) 2005-2006 Jan Hambecht <jaham@gmx.net>

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

#ifndef VDOCUMENT_H
#define VDOCUMENT_H

#include <KoUnit.h>

#include <QString>
#include <QMap>
#include <q3ptrlist.h>
#include <q3ptrdict.h>
#include <QRectF>

#include "vobject.h"

#include <koffice_export.h>
#include <KoShapeControllerBase.h>

class QDomDocument;
class QDomElement;
class VSelection;
class KoPageLayout;
class KoShape;

class KoShapeLayer;

typedef QList<KoShapeLayer*> VLayerList;

/**
 * All non-visual, static doc info is in here.
 * The karbon part uses this class.
 * Filters can use this class as well instead of
 * the visually oriented karbon part.
 */

class KARBONBASE_EXPORT VDocument : public VObject
{
public:
	/** The different selection modes */
	enum VSelectionMode {
		ActiveLayer,	/**< selection within the active layer */
		VisibleLayers,	/**< selection within all visible layers */
		SelectedLayers,	/**< selection within all selected layers */
		AllLayers		/**< selection within all layers */
	};

	/**
	 * Constructs a new document.
	 */
	VDocument();

	/**
	 * Copy constructor.
	 *
	 * @param document the document to copy properties from
	 */
	VDocument( const VDocument& document );

	/** 
	 * Destroys the document and all of the layers.
	 */
	virtual ~VDocument();

	virtual void draw( VPainter* painter, const QRectF* rect ) const;
	
	/**
	 * Draw the document frame to a painting device.
	 *
	 * @param painter abstraction that is used to render to a painting device.
	 * @param pl layout describing the page to draw on (restricting the painter)
	 * @param drawPageMargins if @c true, also draw the crop marks for the page margins,
	 *        otherwise, don't draw them.
	 */
	void drawPage( VPainter *painter, const KoPageLayout &pl, bool drawPageMargins ) const;

	/**
	 * Returns document width.
	 *
	 * @return the document's width
	 */
	double width() const { return m_width; }

	/**
	 * Returns document height.
	 *
	 * @return the document's height
	 */
	double height() const { return m_height; }

	/**
	 * Sets document width.
	 *
	 * @param width the new document width
	 */
	void setWidth( double width ) { m_width = width; m_boundingBox.setWidth( width ); }

	/**
	 * Sets document height.
	 *
	 * @param height the new document height
	 */
	void setHeight( double height ) { m_height = height; m_boundingBox.setHeight( height ); }

	/**
	 * Returns document unit.
	 *
	 * @return the document's unit
	 */
	KoUnit unit() const
		{ return m_unit; }

	/**
	 * Sets document unit.
	 *
	 * @param unit the new document unit
	 */
	void setUnit( KoUnit unit )
		{ m_unit = unit; }

	/**
	 * Checks if specified layer can be raised. 
	 *
	 * A layer can be raised if there is more than one layer and the specified layer
	 * is not already at the top.
	 *
	 * @param layer the layer to check
	 * @return true if layer can be raised, else false
	 */
	bool canRaiseLayer( KoShapeLayer* layer );

	/**
	 * Checks if specified layer can be lowered. 
	 *
	 * A layer can be lowered if there is more than one layer and the specified layer
	 * is not already at the bottom.
	 *
	 * @param layer the layer to check
	 * @return true if layer can be lowered, else false
	 */
	bool canLowerLayer( KoShapeLayer* layer );

	/**
	 * Raises the layer.
	 * 
	 * @param layer the layer to raise
	 */
	void raiseLayer( KoShapeLayer* layer );

	/**
	 * Lowers the layer.
	 * 
	 * @param layer the layer to lower
	 */
	void lowerLayer( KoShapeLayer* layer );

	/**
	 * Returns the position of the specified layer.
	 *
	 * @param layer the layer to retrieve the position for
	 * @return the layer position
	 */
	int layerPos( KoShapeLayer* layer );

	/**
	 * Inserts a new layer.
	 * 
	 * The layer is appended at the end, on top of all other layers, and is activated.
	 *
	 * @param layer the layer to insert
	 */
	void insertLayer( KoShapeLayer* layer );

	/**
	 * Removes the layer.
	 *
	 * If there is no layer left, a new layer is created, inserted and activated.
	 *
	 * @param layer the layer to remove
	 */
	void removeLayer( KoShapeLayer* layer );

	/**
	 * Returns the list of layers.
     * The layer list provides a hierarchical view/access of the document data.
     * All the documents shapes are children of a shape container, where a layer
     * resembles a root container which can contain other containers in an
     * arbitrary nesting depth.
	 */
	const VLayerList& layers() const { return m_layers; }

	/**
	 * Returns the list of all shapes of the document.
     * This list provides a flat view/access to all the documents shapes.
     * For an hierarchical view/access one should retrieve the documents
     * layers with layers().
	 */
	const QList<KoShape*> shapes() const;

	QDomDocument saveXML() const;
	virtual void saveOasis( KoStore *store, KoXmlWriter *docWriter, KoGenStyles &mainStyles ) const;
	enum { STYLE_GRAPHICAUTO = 20, STYLE_LINEAR_GRADIENT, STYLE_RADIAL_GRADIENT, STYLE_STROKE };
	bool loadXML( const QDomElement& doc );
	virtual bool loadOasis( const QDomElement &element, KoOasisLoadingContext &context );
	virtual void save( QDomElement& element ) const;
	virtual void load( const QDomElement& element );
	void loadDocumentContent( const QDomElement& doc );

	virtual VDocument* clone() const;

	virtual void accept( VVisitor& visitor );


	/**
	 * Returns a pointer to the selection.
	 *
	 * @return the document's selection
	 */
	VSelection* selection() const
		{ return m_selection; }

	/**
	 * Returns the selection mode.
	 * 
	 * @return the actual selection mode
	 */
	VSelectionMode selectionMode() 
		{ return m_selectionMode; }

	/**
	 * Sets the selection mode.
	 *
	 * @param mode the new selection mode
	 */
	void setSelectionMode( VSelectionMode mode ) 
		{ m_selectionMode = mode; }

	/**
	 * Adds a new object to the active layer.
	 *
	 * @param object the object to append
	 */
	void add( KoShape* shape );

	/**
	 * Removes an object from the document.
	 *
	 * @param object the object to append
	 */
	void remove( KoShape* shape );
	
	/**
	 * Returns custom name of specified object.
	 *
	 * @param obj the object to retrieve name for
	 * @return the custom name of the object or an empty string if no custom name is set
	 */
	QString objectName( const KoShape *obj ) const;

	/**
	 * Sets custom name of specified object.
	 *
	 * By default all object have generic names like path, rectangle or text that
	 * is defined within the object's class.
	 * 
	 * @param obj the object to set custom name for
	 * @param name the the custom name to set
	 */
	void setObjectName( const KoShape *obj, const QString name );

	bool saveAsPath() const { return m_saveAsPath; }
	void saveAsPath( bool b ) { m_saveAsPath = b; }

    QRectF boundingRect();

private:
	/**
	 * Document width.
	 */
	double m_width;

	/**
	 * Document height.
	 */
	double m_height;

    QList<KoShape*> m_objects;   ///< The list of all object of the document.
	VLayerList m_layers;         ///< The layers in this document.

	/// The selection. A list of selected objects.
	VSelection* m_selection;
	/// The selectionMode
	VSelectionMode m_selectionMode;

	/**
	 * The unit.
	 */
	KoUnit m_unit;

	QMap<const KoShape *, QString>	m_objectNames;

	// TODO this flag is used nowhere, can we remove it?
	bool m_saveAsPath;
};

/*
class KARBONBASE_EXPORT KarbonDocument : public QObject, public KoShapeControllerInterface
{
    Q_OBJECT

public:
    FlakeDocument();
    virtual ~FlakeDocument();
    /// returns the list of objects
    const QList<KoShape *> & objects() const { return m_objects; }

    // inherited from KoShapeControllerInterface
    virtual void addShape( KoShape* shape );
    virtual void removeShape( KoShape* shape );

    /// add a new view
    void addView( KoCanvasBase* view );
    /// remove a connected view
    void removeView( KoCanvasBase* view );

    KoShape* createShape(const QRectF &rect) const;

private:
    /// the list of the documents objects
    QList<KoShape *> m_objects;
    /// the list of the connected views
    QList<KoCanvasBase*> m_views;
};*/

#endif

