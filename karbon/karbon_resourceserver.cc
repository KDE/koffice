/* This file is part of the KDE project
   Copyright (C) 2002-2003 Rob Buis <buis@kde.org>
   Copyright (C) 2002 Beno� Vautrin <benoit.vautrin@free.fr>
   Copyright (C) 2002 Lennart Kudling <kudling@kde.org>
   Copyright (C) 2003 Anders Lund <anders@alweb.dk>
   Copyright (C) 2003,2006 Laurent Montel <montel@kde.org>
   Copyright (C) 2005 Peter Simonsson <psn@linux.se>
   Copyright (C) 2005-2006 Tim Beaulen <tbscope@gmail.com>
   Copyright (C) 2005-2007 Thomas Zander <zander@kde.org>
   Copyright (C) 2006-2007 Jan Hambrecht <jaham@gmx.net>
   Copyright (C) 2006 Casper Boemann <cbr@boemann.dk>
   Copyright (C) 2006 Dirk Mueller <mueller@kde.org>
   Copyright (C) 2007 Boudewijn Rempt <boud@valdyas.org>
   Copyright (C) 2007 David Faure <faure@kde.org>
   Copyright (C) 2007 Matthias Kretz <kretz@kde.org>

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

#include <config-karbon.h>
#include <QDir>
#include <qdom.h>
#include <QFile>
#include <qfileinfo.h>
#include <qstringlist.h>
#include <QPointF>
#include <QPixmap>
#include <Q3PtrList>
#include <QTextStream>

#include <kdebug.h>
#include <kglobal.h>
#include <kcomponentdata.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kiconloader.h>
#include <kogradientmanager.h>

#include "karbon_factory.h"
#include "karbon_resourceserver.h"
#include "vcomposite.h"
#include "vgradient.h"
#include "vgradienttabwidget.h"
#include "vgroup.h"
#include "vobject.h"
#include "vtext.h"
#include "vtransformcmd.h"
#include "shapes/vellipse.h"
#include "shapes/vsinus.h"
#include "shapes/vspiral.h"
#include "shapes/vstar.h"
#include "shapes/vpolyline.h"
#include "shapes/vpolygon.h"

#include <KoPattern.h>

#include <math.h>

KarbonResourceServer::KarbonResourceServer()
{
	kDebug(38000) <<"-- Karbon ResourceServer --";

	// PATTERNS
	kDebug(38000) <<"Loading patterns:";

	// image formats
	QStringList formats;
	formats << "*.png" << "*.tif" << "*.xpm" << "*.bmp" << "*.jpg" << "*.gif" << "*.pat";

	// init vars
	QStringList lst;
	QString format, file;

	// find patterns

	for( QStringList::Iterator it = formats.begin(); it != formats.end(); ++it )
	{
		format = *it;
		QStringList l = KarbonFactory::componentData().dirs()->findAllResources(
							"kis_pattern", format, KStandardDirs::NoDuplicates);
		lst += l;
	}

	// load patterns
	for( QStringList::Iterator it = lst.begin(); it != lst.end(); ++it )
	{
		file = *it;
		kDebug(38000) <<" -" << file;
		loadPattern( file );
	}

	kDebug(38000) << m_patterns.count() <<" patterns loaded.";

	// GRADIENTS
	kDebug(38000) <<"Loading gradients:";
	m_gradients = new Q3PtrList<VGradientListItem>();
	m_gradients->setAutoDelete( true );

	formats.clear();
	lst.clear();
	formats = KoGradientManager::filters();

	// find Gradients

	for( QStringList::Iterator it = formats.begin(); it != formats.end(); ++it )
	{
		format = *it;
		QStringList l = KarbonFactory::componentData().dirs()->findAllResources(
							"karbon_gradient", format, KStandardDirs::NoDuplicates);
		lst += l;
	}

	// load Gradients
	for( QStringList::Iterator it = lst.begin(); it != lst.end(); ++it )
	{
		file = *it;
		kDebug(38000) <<" -" << file;
		loadGradient( file );
	}

	kDebug(38000) << m_gradients->count() <<" gradients loaded.";

	// CLIPARTS
	kDebug(38000) <<"Loading cliparts:";
	m_cliparts = new Q3PtrList<VClipartIconItem>();
	m_cliparts->setAutoDelete( true );

	formats.clear();
	lst.clear();
	formats << "*.kclp";

	// find cliparts

	for( QStringList::Iterator it = formats.begin(); it != formats.end(); ++it )
	{
		format = *it;
		QStringList l = KarbonFactory::componentData().dirs()->findAllResources(
							"karbon_clipart", format, KStandardDirs::NoDuplicates);
		lst += l;
	}

	// load cliparts
	for( QStringList::Iterator it = lst.begin(); it != lst.end(); ++it )
	{
		file = *it;
		kDebug(38000) <<" -" << file;
		loadClipart( file );
	}

	m_pixmaps.setAutoDelete( true );

	kDebug(38000) << m_cliparts->count() <<" cliparts loaded.";
} // KarbonResourceServer::KarbonResourceServer

KarbonResourceServer::~KarbonResourceServer()
{
    qDeleteAll( m_patterns );
	m_gradients->clear();
	delete m_gradients;
	m_cliparts->clear();
	delete m_cliparts;
} // KarbonResourceServer::~KarbonResourceServer

int KarbonResourceServer::patternCount() const
{
    return m_patterns.count();
}

QList<KoPattern*> KarbonResourceServer::patterns()
{
    return m_patterns;
}

const KoPattern * KarbonResourceServer::loadPattern( const QString& filename )
{
    KoPattern * pattern = new KoPattern( filename );

    if( pattern->load() )
        m_patterns.append( pattern );
    else
    {
        delete pattern;
        pattern = 0L;
    }

    return pattern;
}

KoPattern * KarbonResourceServer::addPattern( const QString& tilename )
{
    int i = 1;
    QFileInfo fi;
    fi.setFile( tilename );

    if( fi.exists() == false )
        return 0L;

    QString name = fi.baseName();

    QString ext = '.' + fi.suffix();

    QString filename = KarbonFactory::componentData().dirs()->saveLocation(
                        "kis_pattern" ) + name + ext;

    i = 1;

    fi.setFile( filename );

    while( fi.exists() == true )
    {
        filename = KarbonFactory::componentData().dirs()->saveLocation("kis_pattern" ) + name + i + ext;
        fi.setFile( filename );
        kDebug(38000) << fi.fileName();
    }

    char buffer[ 1024 ];
    QFile in( tilename );
    in.open( QIODevice::ReadOnly );
    QFile out( filename );
    out.open( QIODevice::WriteOnly );

    while( !in.atEnd() )
        out.write( buffer, in.read( buffer, 1024 ) );

    out.close();
    in.close();

    const KoPattern* pattern = loadPattern( filename );

    if( pattern )
        return m_patterns.last();

    return 0;
} // KarbonResourceServer::addPattern

void KarbonResourceServer::removePattern( KoPattern* pattern )
{
    int index = m_patterns.indexOf( pattern );
    if( index < 0 )
        return;

    QFile file( pattern->filename() );

    if( file.remove() )
    {
        m_patterns.removeAt( index );
        delete pattern;
    }
} // KarbonResourceServer::removePattern

VGradientListItem*
KarbonResourceServer::addGradient( QGradient* gradient )
{
	int i = 1;
	char buffer[ 20 ];
	QFileInfo fi;

	sprintf( buffer, "%04d.kgr", i++ );
	fi.setFile( KarbonFactory::componentData().dirs()->saveLocation( "karbon_gradient" ) + buffer );

	while( fi.exists() == true )
	{
		sprintf( buffer, "%04d.kgr", i++ );
		fi.setFile( KarbonFactory::componentData().dirs()->saveLocation( "karbon_gradient" ) + buffer );
		kDebug(38000) << fi.fileName();
	}

	QString filename = KarbonFactory::componentData().dirs()->saveLocation( "karbon_gradient" ) + buffer;

    // TODO port 
	//saveGradient( gradient, filename );

	m_gradients->append( new VGradientListItem( gradient, filename ) );

	return m_gradients->last();
} // KarbonResourceServer::addGradient

void
KarbonResourceServer::removeGradient( VGradientListItem* gradient )
{
	QFile file( gradient->filename() );

	if( file.remove() )
		m_gradients->remove( gradient );
} // KarbonResourceServer::removeGradient

void
KarbonResourceServer::loadGradient( const QString& filename )
{
    KoGradientManager gradLoader;

    KoGradient* grad = gradLoader.loadGradient(filename);

    if( !grad )
        return;

    if( grad->colorStops.count() > 1 )
    {
        QGradient * gradient = 0;

        switch(grad->gradientType)
        {
            case KoGradientManager::gradient_type_linear:
            {
                QPointF start( grad->originX, grad->originY );
                QPointF stop( grad->vectorX, grad->vectorY );
                gradient = new QLinearGradient( start, stop );
                break;
            }
            case KoGradientManager::gradient_type_radial:
            {
                QPointF center( grad->originX, grad->originY );
                QPointF stop( grad->vectorX, grad->vectorY );
                QPointF focal( grad->focalpointX, grad->focalpointY );
                QPointF diff = stop-center;
                double radius = sqrt( diff.x()*diff.x() + diff.y()*diff.y() );
                gradient = new QRadialGradient( center, radius, focal );
                break;
            }
            case KoGradientManager::gradient_type_conic:
            {
                QPointF center( grad->originX, grad->originY );
                QPointF stop( grad->vectorX, grad->vectorY );
                QPointF diff = stop-center;
                double angle = atan2( center.y(), center.x() ) * 180.0 / M_PI;
                if( angle < 0.0 )
                    angle += 360.0;
                gradient = new QConicalGradient( center, angle );
                break;
            }
            default:
                delete gradient;
                return;
        }

        switch(grad->gradientRepeatMethod)
        {
            case KoGradientManager::repeat_method_none:
                gradient->setSpread(QGradient::PadSpread);
                break;
            case KoGradientManager::repeat_method_reflect:
                gradient->setSpread(QGradient::ReflectSpread);
                break;
            case KoGradientManager::repeat_method_repeat:
                gradient->setSpread(QGradient::RepeatSpread);
                break;
            default:
                delete gradient;
                return;
        }

        KoColorStop *colstop;
        for(colstop = grad->colorStops.first(); colstop; colstop = grad->colorStops.next())
        {
            QColor color;

            switch(colstop->colorType)
            {
                case KoGradientManager::color_type_hsv_ccw:
                case KoGradientManager::color_type_hsv_cw:
                    color.setHsvF( colstop->color1, colstop->color2, colstop->color3 );
                    break;
                case KoGradientManager::color_type_gray:
                    color.setRgbF( colstop->color1, colstop->color1, colstop->color1 );
                    break;
                case KoGradientManager::color_type_cmyk:
                    color.setCmykF( colstop->color1, colstop->color2, colstop->color3, colstop->color4);
                    break;
                case KoGradientManager::color_type_rgb:
                default:
                    color.setRgbF( colstop->color1, colstop->color2, colstop->color3 );
            }
            color.setAlphaF( colstop->opacity );

            gradient->setColorAt( colstop->offset, color );
        }
        m_gradients->append( new VGradientListItem( gradient, filename ) );
    }
} // KarbonResourceServer::loadGradient

void
KarbonResourceServer::saveGradient( VGradient* gradient, const QString& filename )
{
    QFile file( filename );
    QDomDocument doc;
    QDomElement me = doc.createElement( "PREDEFGRADIENT" );
    doc.appendChild( me );
    gradient->save( me );

    if( !( file.open( QIODevice::WriteOnly ) ) )
        return ;

    QTextStream ts( &file );

    doc.save( ts, 2 );

    file.flush();

    file.close();
} // KarbonResourceServer::saveGradient

VClipartIconItem*
KarbonResourceServer::addClipart( VObject* clipart, double width, double height )
{
	int i = 1;
	char buffer[ 20 ];
	sprintf( buffer, "%04d.kclp", i++ );

	while( KStandardDirs::exists( KarbonFactory::componentData().dirs()->saveLocation( "karbon_clipart" ) + buffer ) )
		sprintf( buffer, "%04d.kclp", i++ );

	QString filename = KarbonFactory::componentData().dirs()->saveLocation( "karbon_clipart" ) + buffer;

	saveClipart( clipart, width, height, filename );

	m_cliparts->append( new VClipartIconItem( clipart, width, height, filename ) );

	return m_cliparts->last();
} // KarbonResourceServer::addClipart

void
KarbonResourceServer::removeClipart( VClipartIconItem* clipart )
{
	QFile file( clipart->filename() );

	if( file.remove() )
		m_cliparts->remove
		( clipart );
}

void
KarbonResourceServer::loadClipart( const QString& filename )
{
	QFile f( filename );

	if( f.open( QIODevice::ReadOnly ) )
	{
		KoXmlDocument doc;

		if( !( doc.setContent( &f ) ) )
			f.close();
		else
		{
			KoXmlElement de = doc.documentElement();

			if( !de.isNull() && de.tagName() == "PREDEFCLIPART" )
			{
				VObject* clipart = 0L;
				double width = de.attribute( "width", "100.0" ).toFloat();
				double height = de.attribute( "height", "100.0" ).toFloat();

				KoXmlNode n = de.firstChild();

				if( !n.isNull() )
				{
					KoXmlElement e;
					e = n.toElement();

					if( !e.isNull() )
					{
						if( e.tagName() == "TEXT" )
							clipart = new VText( 0L );
						else if( e.tagName() == "COMPOSITE" || e.tagName() == "PATH" )
							clipart = new VPath( 0L );
						else if( e.tagName() == "GROUP" )
							clipart = new VGroup( 0L );
						else if( e.tagName() == "ELLIPSE" )
							clipart = new VEllipse( 0L );
						else if( e.tagName() == "POLYGON" )
							clipart = new VPolygon( 0L );
						else if( e.tagName() == "POLYLINE" )
							clipart = new VPolyline( 0L );
#if 0 
for now	
					else if( e.tagName() == "RECT" )
							clipart = new KarbonRectangle;
#endif
						else if( e.tagName() == "SINUS" )
							clipart = new VSinus( 0L );
						else if( e.tagName() == "SPIRAL" )
							clipart = new VSpiral( 0L );
						else if( e.tagName() == "STAR" )
							clipart = new VStar( 0L );
#ifdef HAVE_KARBONTEXT
						else if( e.tagName() == "TEXT" )
							clipart = new VText( 0L );
#endif
						if( clipart )
							clipart->load( e );
					}

					if( clipart )
						m_cliparts->append( new VClipartIconItem( clipart, width, height, filename ) );

					delete clipart;
				}
			}
		}
	}
}

void
KarbonResourceServer::saveClipart( VObject* clipart, double width, double height, const QString& filename )
{
	QFile file( filename );
	QDomDocument doc;
	QDomElement me = doc.createElement( "PREDEFCLIPART" );
	doc.appendChild( me );
	me.setAttribute( "width", width );
	me.setAttribute( "height", height );
	clipart->save( me );

	if( !( file.open( QIODevice::WriteOnly ) ) )
		return ;

	QTextStream ts( &file );

	doc.save( ts, 2 );

	file.flush();

	file.close();
}

QPixmap *
KarbonResourceServer::cachePixmap( const QString &key, int group_or_size )
{
	QPixmap *result = 0L;
	if( !( result = m_pixmaps[ key ] ) )
	{
		result = new QPixmap( KIconLoader::global()->iconPath( key, group_or_size ) );
		m_pixmaps.insert( key, result );
	}
	return result;
}

VClipartIconItem::VClipartIconItem( const VObject* clipart, double width, double height, const QString & filename )
		: m_filename( filename ), m_width( width ), m_height( height )
{
	m_clipart = clipart->clone();
	m_clipart->setState( VObject::normal );

	m_pixmap = QPixmap( 64, 64 );
    /*
	VQPainter p( &m_pixmap, 64, 64 );
	QMatrix mat( 64., 0, 0, 64., 0, 0 );

	VTransformCmd trafo( 0L, mat );
	trafo.visit( *m_clipart );

	m_clipart->draw( &p, &m_clipart->boundingBox() );

	trafo.setMatrix( mat.inverted() );
	trafo.visit( *m_clipart );

	p.end();

	m_thumbPixmap = QPixmap( 32, 32 );
	VQPainter p2( &m_thumbPixmap, 32, 32 );
	mat.setMatrix( 32., 0, 0, 32., 0, 0 );

	trafo.setMatrix( mat );
	trafo.visit( *m_clipart );

	m_clipart->draw( &p2, &m_clipart->boundingBox() );

	trafo.setMatrix( mat.inverted() );
	trafo.visit( *m_clipart );

	p2.end();
    */
	m_delete = QFileInfo( filename ).isWritable();
}


VClipartIconItem::VClipartIconItem( const VClipartIconItem& item )
		: QTableWidgetItem( item )
{
	m_clipart = item.m_clipart->clone();
	m_filename = item.m_filename;
	m_delete = item.m_delete;
	m_pixmap = item.m_pixmap;
	m_thumbPixmap = item.m_thumbPixmap;
	m_width = item.m_width;
	m_height = item.m_height;
}

VClipartIconItem::~VClipartIconItem()
{
	delete m_clipart;
}

VClipartIconItem* VClipartIconItem::clone() const
{
	return new VClipartIconItem( *this );
}

