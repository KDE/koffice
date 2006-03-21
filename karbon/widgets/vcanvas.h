/* This file is part of the KDE project
   Copyright (C) 2001, 2002, 2003 The Karbon Developers

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

#ifndef __VCANVAS_H__
#define __VCANVAS_H__


#include <qscrollview.h>
#include <koffice_export.h>
class KarbonPart;
class KarbonView;
class KoRect;
class KoPoint;
class VPainter;

// The canvas is a QScrollView.

class KARBONCOMMON_EXPORT VCanvas : public QScrollView
{
	Q_OBJECT
public:
	VCanvas( QWidget *parent, KarbonView* view, KarbonPart* part );
	virtual ~VCanvas();

	void repaintAll( const KoRect & );
	void repaintAll( bool drawVObjects = true );

	QPixmap *pixmap() { return m_pixmap; }

	/**
	 * Sets mouse position to point p.
	 */
	void setPos( const KoPoint& p );

	KoPoint toViewport( const KoPoint & ) const;
	KoPoint toContents( const KoPoint & ) const;
	KoRect boundingBox() const;

	/**
	 * Adjusts the viewport top-left position. This doesn't change the zoom level.
	 * Note that centerX and centerY is a value between 0.0 and 1.0, indicating a
	 * percentage of the total width/height. Thus centerX/centerY indicates the
	 * center of the viewport.
	 */
	void setViewport( double centerX, double centerY );

	/**
	 * Sets the canvas viewport rectangle to rect. The zoom level is adjusted for this, if
	 * needed.
	 */
	void setViewportRect( const KoRect &rect );

	int pageOffsetX() const;
	int pageOffsetY() const;

	KoPoint snapToGrid( const KoPoint & );

protected:
	virtual void dragEnterEvent( QDragEnterEvent * );
	virtual void dropEvent( QDropEvent * );
	virtual void focusInEvent( QFocusEvent * );
	virtual void viewportPaintEvent( QPaintEvent* );
	virtual void drawContents( QPainter* painter, int clipx, int clipy,
		int clipw, int cliph  );
	void drawDocument( QPainter* painter, const KoRect& rect, bool drawVObjects = true );

	virtual void resizeEvent( QResizeEvent* event );

	virtual bool eventFilter( QObject* object, QEvent* event );

	void setYMirroring( VPainter * );

private slots:
	void slotContentsMoving( int , int );

signals:
	void viewportChanged();

private:
	QPixmap *m_pixmap;
	KarbonPart* m_part;
	KarbonView* m_view;
};

#endif
