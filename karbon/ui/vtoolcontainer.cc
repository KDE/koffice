/* This file is part of the KDE project
   Copyright (C) 2002, The Karbon Developers
*/

#include <qbuttongroup.h>
#include <kpixmap.h>
#include <kstandarddirs.h>
#include <qtoolbutton.h>
#include <kdualcolorbutton.h>
#include <koMainWindow.h>
#include <koView.h>

#include "vtoolcontainer.h"

VToolContainer::VToolContainer( KoView* parent, const char* /*name*/ )
	: QToolBar( parent->shell() )
{
	setOrientation(Vertical);
	setLabel("");

	QToolButton *button;

	btngroup = new QButtonGroup( 2, Horizontal, this );
	btngroup->setExclusive( true );
	button = new QToolButton(btngroup);
	QPixmap pixmap(locate("appdata", "icons/hicolor/22x22/actions/select.png"));
	button->setPixmap(pixmap);
	button->setToggleButton( true );
	button->toggle();
	connect( button, SIGNAL( clicked() ), this, SIGNAL( selectToolActivated() ) );
	btngroup->insert( button, Select );

	button = new QToolButton(btngroup);
	pixmap = QPixmap(locate("appdata", "icons/hicolor/22x22/actions/scale.png"));
	button->setPixmap(pixmap);
	button->setToggleButton( true );
	connect( button, SIGNAL( clicked() ), this, SIGNAL( scaleToolActivated() ) );
	btngroup->insert( button, Scale );

	button = new QToolButton(btngroup);
	pixmap = QPixmap(locate("appdata", "icons/hicolor/22x22/actions/rotate.png"));
	button->setPixmap(pixmap);
	button->setToggleButton( true );
	connect( button, SIGNAL( clicked() ), this, SIGNAL( rotateToolActivated() ) );
	btngroup->insert( button, Rotate );
	
	button = new QToolButton(btngroup);
	pixmap = QPixmap(locate("appdata", "icons/hicolor/22x22/actions/shear.png"));
	button->setPixmap(pixmap);
	button->setToggleButton( true );
	connect( button, SIGNAL( clicked() ), this, SIGNAL( shearToolActivated() ) );
	btngroup->insert( button, Shear );
	
	button = new QToolButton(btngroup);
	pixmap = QPixmap(locate("appdata", "icons/hicolor/22x22/actions/ellipse.png"));
	button->setPixmap(pixmap);
	button->setToggleButton( true );
	connect( button, SIGNAL( clicked() ), this, SIGNAL( ellipseToolActivated() ) );
	btngroup->insert( button, Ellipse );
	
	button = new QToolButton(btngroup);
	pixmap = QPixmap(locate("appdata", "icons/hicolor/22x22/actions/rectangle.png"));
	button->setPixmap(pixmap);
	button->setToggleButton( true );
	connect( button, SIGNAL( clicked() ), this, SIGNAL( rectangleToolActivated() ) );
	btngroup->insert( button, Rectangle );

	button = new QToolButton(btngroup);
	pixmap = QPixmap(locate("appdata", "icons/hicolor/22x22/actions/roundrect.png"));
	button->setPixmap(pixmap);
	button->setToggleButton( true );
	connect( button, SIGNAL( clicked() ), this, SIGNAL( roundRectToolActivated() ) );
	btngroup->insert( button, Roundrect );
	
	button = new QToolButton(btngroup);
	pixmap = QPixmap(locate("appdata", "icons/hicolor/22x22/actions/polygon.png"));
	button->setPixmap(pixmap);
	button->setToggleButton( true );
	connect( button, SIGNAL( clicked() ), this, SIGNAL( polygonToolActivated() ) );
	btngroup->insert( button, Polygon );
	
	button = new QToolButton(btngroup);
	pixmap = QPixmap(locate("appdata", "icons/hicolor/22x22/actions/star.png"));
	button->setPixmap(pixmap);
	button->setToggleButton( true );
	connect( button, SIGNAL( clicked() ), this, SIGNAL( starToolActivated() ) );
	btngroup->insert( button, Star );

	button = new QToolButton(btngroup);
	pixmap = QPixmap(locate("appdata", "icons/hicolor/22x22/actions/sinus.png"));
	button->setPixmap(pixmap);
	button->setToggleButton( true );
	connect( button, SIGNAL( clicked() ), this, SIGNAL( sinusToolActivated() ) );
	btngroup->insert( button, Sinus );

	button = new QToolButton(btngroup);
	pixmap = QPixmap(locate("appdata", "icons/hicolor/22x22/actions/spiral.png"));
	button->setPixmap(pixmap);
	button->setToggleButton( true );
	connect( button, SIGNAL( clicked() ), this, SIGNAL( spiralToolActivated() ) );
	btngroup->insert( button, Spiral );

	btngroup->setInsideSpacing(2);
	btngroup->setInsideMargin(5);
	connect(btngroup, SIGNAL(clicked(int)), this, SLOT(slotButton(int)));

	//dialog buttons
	dlggroup = new QButtonGroup( 2, Horizontal, this ); 
	// has to be a new non-toggle group (dialogs, not tools)
	button = new QToolButton(dlggroup);
	pixmap = QPixmap(locate("appdata", "icons/hicolor/22x22/actions/outlinedlg.png"));
	button->setPixmap(pixmap);
	dlggroup->insert( button, Outline);
	button = new QToolButton(dlggroup);
	pixmap = QPixmap(locate("appdata", "icons/hicolor/22x22/actions/solidfilldlg.png"));
	button->setPixmap(pixmap);
	connect( button, SIGNAL( clicked() ), this, SIGNAL( solidFillActivated() ) );
	dlggroup->insert( button, SolidFill);
	button = new QToolButton(dlggroup);
	pixmap = QPixmap(locate("appdata", "icons/hicolor/22x22/actions/gradientdlg.png"));
	button->setPixmap(pixmap);
	dlggroup->insert( button, Gradient);
	dlggroup->setInsideSpacing(2);
	dlggroup->setInsideMargin(5);
}

void VToolContainer::slotButton( int /*ID*/ )
{
}

VToolContainer::~VToolContainer() { }

#include "vtoolcontainer.moc"
