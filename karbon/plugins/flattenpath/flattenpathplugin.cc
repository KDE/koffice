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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include "flattenpathplugin.h"
#include "klocale.h"
#include <karbon_view_base.h>
#include <karbon_part_base.h>
#include <kgenericfactory.h>
#include <kdebug.h>
#include <qgroupbox.h>
#include <qlabel.h>

#include <knuminput.h>
#include <core/vgroup.h>
#include <core/vpath.h>
#include <core/vsegment.h>
#include <core/vselection.h>

typedef KGenericFactory<FlattenPathPlugin, KarbonViewBase> FlattenPathPluginFactory;
K_EXPORT_COMPONENT_FACTORY( karbon_flattenpathplugin, FlattenPathPluginFactory( "karbonflattenpathplugin" ) );

FlattenPathPlugin::FlattenPathPlugin( KarbonViewBase *parent, const char* name, const QStringList & )
: Plugin( parent, name )
{
	new KAction(
		i18n( "&Flatten Path..." ), 0, 0, this,
		SLOT( slotFlattenPath() ), actionCollection(), "path_flatten" );

	m_flattenPathDlg = new VFlattenDlg();
	m_flattenPathDlg->setFlatness( 0.2 );
}

void
FlattenPathPlugin::slotFlattenPath()
{
	KarbonPartBase *part = ((KarbonViewBase *)parent())->part();
	if( part && m_flattenPathDlg->exec() )
		part->addCommand( new VFlattenCmd( &part->document(), m_flattenPathDlg->flatness() ), true );
}

VFlattenDlg::VFlattenDlg( QWidget* parent, const char* name )
	: KDialogBase( parent, name, true,  i18n( "Polygonize" ), Ok | Cancel )
{
	// add input fields on the left:
	QGroupBox* group = new QGroupBox( 2, Qt::Horizontal, i18n( "Properties" ), this );
	new QLabel( i18n( "Flatness:" ), group );
	m_flatness = new KDoubleNumInput( group );
	group->setMinimumWidth( 300 );

	// signals and slots:
	connect( this, SIGNAL( okClicked() ), this, SLOT( accept() ) );
	connect( this, SIGNAL( cancelClicked() ), this, SLOT( reject() ) );

	setMainWidget( group );
	setFixedSize( baseSize() );
}

double
VFlattenDlg::flatness() const
{
	return m_flatness->value();
}

void
VFlattenDlg::setFlatness( double value )
{
	m_flatness->setValue( value);
}

// TODO: Think about if we want to adapt this:

/*
 * <cite from GNU ghostscript's gxpflat.c>
 *
 * To calculate how many points to sample along a path in order to
 * approximate it to the desired degree of flatness, we define
 *      dist((x,y)) = abs(x) + abs(y);
 * then the number of points we need is
 *      N = 1 + sqrt(3/4 * D / flatness),
 * where
 *      D = max(dist(p0 - 2*p1 + p2), dist(p1 - 2*p2 + p3)).
 * Since we are going to use a power of 2 for the number of intervals,
 * we can avoid the square root by letting
 *      N = 1 + 2^(ceiling(log2(3/4 * D / flatness) / 2)).
 * (Reference: DEC Paris Research Laboratory report #1, May 1989.)
 *
 * We treat two cases specially.  First, if the curve is very
 * short, we halve the flatness, to avoid turning short shallow curves
 * into short straight lines.  Second, if the curve forms part of a
 * character (indicated by flatness = 0), we let
 *      N = 1 + 2 * max(abs(x3-x0), abs(y3-y0)).
 * This is probably too conservative, but it produces good results.
 *
 * </cite from GNU ghostscript's gxpflat.c>
 */


VFlattenCmd::VFlattenCmd( VDocument *doc, double flatness )
	: VReplacingCmd( doc, i18n( "Flatten Curves" ) )
{
	m_flatness = flatness > 0.0 ? flatness : 1.0;
}

void
VFlattenCmd::visitVPath( VPath& path )
{
	path.first();

	// Ommit first segment.
	while( path.next() )
	{
		while( !path.current()->isFlat( m_flatness )  )
		{
			// Split at midpoint.
			path.insert(
				path.current()->splitAt( 0.5 ) );
		}

		// Convert to line.
		path.current()->setDegree( 1 );

		if( !success() )
			setSuccess();
	}
}

#include "flattenpathplugin.moc"

