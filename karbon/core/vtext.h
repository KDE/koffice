/* This file is part of the KDE project
   Copyright (C) 2002, The Karbon Developers
*/

#ifndef __VTEXT_H__
#define __VTEXT_H__

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <qptrlist.h>
#include <qstring.h>
#include <qfont.h>

#include "vobjectlist.h"

class KarbonView;

class VText : public VObject
{
public:
	VText();
	VText( KarbonView *view, const QFont &font );
	VText( KarbonView *view, const QFont &font, const QString& text );
	VText( const VText& text );
	virtual ~VText();

	virtual void draw( VPainter *painter, const KoRect& rect );

	virtual void transform( const QWMatrix& m );

	virtual KoRect boundingBox( const double zoomFactor ) const;
	virtual bool intersects( const KoRect& rect, const double zoomFactor ) const;

	virtual VObject* clone();

	virtual void save( QDomElement& element ) const;
	virtual void load( const QDomElement& element );

	void setState( const VState state );

private:
#ifdef HAVE_FREETYPE
	void traceText( const QString &text );
#endif // HAVE_FREETYPE

	QString m_text;
	VObjectList m_glyphs;
	const KarbonView *m_view;
	QFont m_font;
};

#endif
