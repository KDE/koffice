// -*- Mode: c++-mode; c-basic-offset: 4; indent-tabs-mode: nil; tab-width: 4; -*-
/* This file is part of the KDE project
   Copyright (c) 2003 Rob Buis <buis@kde.org>

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

#ifndef OODRAW_IMPORT_H__
#define OODRAW_IMPORT_H__

#include <koFilter.h>
#include <koStore.h>

#include <qdom.h>
#include <qdict.h>
#include <qcolor.h>
#include <stylestack.h>
#include <core/vdocument.h>
#include <core/vcomposite.h>
#include <core/vstroke.h>

class VGroup;

class OoDrawImport : public KoFilter
{
    Q_OBJECT
public:
    OoDrawImport( KoFilter *parent, const char *name, const QStringList & );
    virtual ~OoDrawImport();

    virtual KoFilter::ConversionStatus convert( QCString const & from, QCString const & to );

private:
	void createStyleMap( QDomDocument &docstyles );
	void insertStyles( const QDomElement& styles );
	void insertDraws( const QDomElement& styles );
	void fillStyleStack( const QDomElement& object );
	void addStyles( const QDomElement* style );
	void storeObjectStyles( const QDomElement& object );
	void appendPen( VObject &obj );
	void appendBrush( VObject &obj );
	void appendPoints(VPath &path, const QDomElement& object);
	void convert();
	void parseGroup( VGroup *parent, const QDomElement& object );
	void parseColor( VColor &color, const QString &s );
	double ymirror( double y );
	KoRect parseViewBox( const QDomElement& object );

	KoFilter::ConversionStatus openFile();

	VDocument			m_document;
	QDomDocument			m_content;
	QDomDocument			m_meta;
	QDomDocument			m_settings;
	QDict<QDomElement>		m_styles, m_draws;
	StyleStack			m_styleStack;
};

#endif
