/* This file is part of the KDE project
   Copyright (C) 2002, The Karbon Developers

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

#ifndef __VPATTERN_H__
#define __VPATTERN_H__

#include <QPointF>
#include <QTableWidgetItem>
#include <qimage.h>
#include <qpixmap.h>
#include <karbon_export.h>

#include <KoXmlReader.h>


class KARBONBASE_EXPORT VPattern : public QTableWidgetItem
{
public:
	VPattern();
	VPattern( const QString &tilename );

	unsigned char *pixels();
	unsigned int tileWidth() const;
	unsigned int tileHeight() const;

	QPointF origin() const { return m_origin; }
	void setOrigin( const QPointF &origin ) { m_origin = origin; }

	QPointF vector() const { return m_vector; }
	void setVector( const QPointF &vector ) { m_vector = vector; }

	void load( const QString &tilename );

	void save( QDomElement& element ) const;
	void load( const KoXmlElement& element );

	void transform( const QMatrix& m );

        QPixmap& pixmap() const ;

	bool isValid() const { return m_valid; }

	QString tilename() const { return m_tilename; }

private:
	// coordinates:
	QPointF m_origin;
	QPointF m_vector;
	QImage m_image;
	QPixmap m_pixmap;
	QPixmap m_pixmapThumb;
	QString m_tilename;
	bool m_valid;
};

#endif
