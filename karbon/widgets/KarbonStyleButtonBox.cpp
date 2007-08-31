/* This file is part of the KDE project
   Copyright (C) 2003 Tomislav Lukman <tomislav.lukman@ck.t-com.hr>
   Copyright (C) 2005 Rob Buis <buis@kde.org>
   Copyright (C) 2007 Jan Hambrecht <jaham@gmx.net>

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

#if 0

// 8x8 pixels

static const char* const buttonnone[]={
"8 8 7 1",
"c c #0a0000",
". c #0a0a0a",
"b c #330000",
"a c #331f1f",
"# c #333333",
"e c #ff0000",
"d c #ffffff",
".####abc",
"#ddddeeb",
"#dddeeea",
"#ddeeed#",
"#deeedd#",
"aeeeddd#",
"beedddd#",
"cba####."};

static const char* const buttonsolid[]={
"8 8 1 1",
". c #000000",
"........",
"........",
"........",
"........",
"........",
"........",
"........",
"........"};

static const char* const buttongradient[]={
"8 8 14 1",
"f c #000000",
"e c #040404",
". c #0a0a0a",
"d c #0f0f0f",
"c c #181818",
"b c #212121",
"a c #292929",
"# c #303030",
"g c #333333",
"l c #484848",
"k c #787878",
"j c #a7a7a7",
"i c #cdcdcd",
"h c #f1f1f1",
".#abcdef",
"ghijkl#f",
"ghijkl#f",
"ghijkl#f",
"ghijkl#f",
"ghijkl#f",
"ghijkl#f",
".#abcdef"};

static const char* const buttonpattern[]={
"8 8 4 1",
". c #0a0a0a",
"# c #333333",
"a c #a0a0a0",
"b c #ffffff",
".######.",
"#aabbaa#",
"#aabbaa#",
"#bbaabb#",
"#bbaabb#",
"#aabbaa#",
"#aabbaa#",
".######."};

#else

// 16x16 pixels

static const char* const buttonnone[]={
"16 16 3 1",
"# c #333333",
"e c #ff0000",
"- c #ffffff",
"################",
"#------------ee#",
"#-----------eee#",
"#----------eee-#",
"#---------eee--#",
"#--------eee---#",
"#-------eee----#",
"#------eee-----#",
"#-----eee------#",
"#----eee-------#",
"#---eee--------#",
"#--eee---------#",
"#-eee----------#",
"#eee-----------#",
"#ee------------#",
"################"};

static const char* const buttonsolid[]={
"16 16 2 1",
"# c #000000",
". c #969696",
"################",
"#..............#",
"#..............#",
"#..............#",
"#..............#",
"#..............#",
"#..............#",
"#..............#",
"#..............#",
"#..............#",
"#..............#",
"#..............#",
"#..............#",
"#..............#",
"#..............#",
"################"};


// FIXME: Smoother gradient button.

static const char* const buttongradient[]={
"16 16 15 1",
"# c #000000",
"n c #101010",
"m c #202020",
"l c #303030",
"k c #404040",
"j c #505050",
"i c #606060",
"h c #707070",
"g c #808080",
"f c #909090",
"e c #a0a0a0",
"d c #b0b0b0",
"c c #c0c0c0",
"b c #d0d0d0",
"a c #e0e0e0",
"################",
"#abcdefghijklmn#",
"#abcdefghijklmn#",
"#abcdefghijklmn#",
"#abcdefghijklmn#",
"#abcdefghijklmn#",
"#abcdefghijklmn#",
"#abcdefghijklmn#",
"#abcdefghijklmn#",
"#abcdefghijklmn#",
"#abcdefghijklmn#",
"#abcdefghijklmn#",
"#abcdefghijklmn#",
"#abcdefghijklmn#",
"#abcdefghijklmn#",
"################"};

static const char* const buttonpattern[]={
"16 16 4 1",
". c #0a0a0a",
"# c #333333",
"a c #a0a0a0",
"b c #ffffffff",
"################",
"#aaaaabbbbaaaaa#",
"#aaaaabbbbaaaaa#",
"#aaaaabbbbaaaaa#",
"#aaaaabbbbaaaaa#",
"#aaaaabbbbaaaaa#",
"#bbbbbaaaabbbbb#",
"#bbbbbaaaabbbbb#",
"#bbbbbaaaabbbbb#",
"#bbbbbaaaabbbbb#",
"#aaaaabbbbaaaaa#",
"#aaaaabbbbaaaaa#",
"#aaaaabbbbaaaaa#",
"#aaaaabbbbaaaaa#",
"#aaaaabbbbaaaaa#",
"################"};

static const char* const buttonevenodd[]={
"16 16 3 1",
"# c #0a0a0a",
". c #969696",
"o c #ffffffff",
"################",
"#..............#",
"#..............#",
"#..............#",
"#..............#",
"#....######....#",
"#....#oooo#....#",
"#....#oooo#....#",
"#....#oooo#....#",
"#....#oooo#....#",
"#....######....#",
"#..............#",
"#..............#",
"#..............#",
"#..............#",
"################"};

static const char* const buttonwinding[]={
"16 16 2 1",
"# c #0a0a0a",
". c #969696",
"################",
"#..............#",
"#..............#",
"#..............#",
"#..............#",
"#....######....#",
"#....#....#....#",
"#....#....#....#",
"#....#....#....#",
"#....#....#....#",
"#....######....#",
"#..............#",
"#..............#",
"#..............#",
"#..............#",
"################"};

#endif


#include "KarbonStyleButtonBox.h"
#include "karbon_part.h"

#include <KoCanvasBase.h>
#include <KoCanvasController.h>
#include <KoToolManager.h>
#include <KoSelection.h>
#include <KoShapeManager.h>
#include <KoShapeBackgroundCommand.h>
#include <KoShapeBorderCommand.h>
#include <KoCanvasResourceProvider.h>
#include <KoShapeBorderModel.h>
#include <KoLineBorder.h>
#include <KoColor.h>
#include <KoPathFillRuleCommand.h>
#include <KoPathShape.h>

#include <klocale.h>

#include <QtGui/QPixmap>
#include <QtGui/QPushButton>
#include <QtGui/QToolTip>
#include <QtGui/QButtonGroup>
#include <QtGui/QGridLayout>

KarbonStyleButtonBox::KarbonStyleButtonBox( QWidget* parent )
    : QFrame( parent ), m_isStrokeManipulator( false )
{
    setFrameStyle( QFrame::GroupBoxPanel | QFrame::Sunken );
    setMinimumSize( 45, 70 );

    QGridLayout * layout = new QGridLayout( this );
    QButtonGroup * group = new QButtonGroup( this );

    // The button for no fill
    QPushButton* button = new QPushButton( this );
    button->setIcon( QPixmap( (const char **) buttonnone ) );
    button->setMinimumHeight( 20 );
    button->setToolTip( i18n( "None" ) );
    group->addButton( button, None );
    layout->addWidget( button, 0, 0 );

    // The button for solid fill
    button = new QPushButton( this );
    button->setIcon( QPixmap( (const char **) buttonsolid ) );
    button->setMinimumHeight( 20 );
    button->setToolTip( i18n( "Solid" ) );
    group->addButton( button, Solid );
    layout->addWidget( button, 0, 1 );

    // The button for gradient fill
    button = new QPushButton( this );
    button->setIcon( QPixmap( (const char **) buttongradient ) );
    button->setMinimumHeight( 20 );
    button->setToolTip( i18n( "Gradient" ) );
    group->addButton( button, Gradient );
    layout->addWidget( button, 1, 0 );

    // The button for pattern fill
    button = new QPushButton( this );
    button->setIcon( QPixmap( (const char **) buttonpattern ) );
    button->setMinimumHeight( 20 );
    button->setToolTip( i18n( "Pattern" ) );
    group->addButton( button, Pattern );
    layout->addWidget( button, 1, 1 );

    // The button for even-odd fill rule
    button = new QPushButton( this );
    button->setIcon( QPixmap( (const char **) buttonevenodd ) );
    button->setMinimumHeight( 20 );
    button->setToolTip( i18n( "Even-Odd Fill" ) );
    group->addButton( button, EvenOdd );
    layout->addWidget( button, 2, 0 );

    // The button for winding fill-rule
    button = new QPushButton( this );
    button->setIcon( QPixmap( (const char **) buttonwinding ) );
    button->setMinimumHeight( 20 );
    button->setToolTip( i18n( "Winding Fill" ) );
    group->addButton( button, Winding );
    layout->addWidget( button, 2, 1 );

    layout->setMargin( 1 );
    layout->setSpacing( 1 );
    layout->setColumnStretch( 0, 1 );
    layout->setColumnStretch( 1, 1 );

    m_group = group;

    connect( group, SIGNAL( buttonClicked( int ) ), this, SLOT( slotButtonPressed( int ) ) );
}

void KarbonStyleButtonBox::slotButtonPressed( int id )
{
    KoCanvasController* canvasController = KoToolManager::instance()->activeCanvasController();
    KoSelection *selection = canvasController->canvas()->shapeManager()->selection();
    if( ! selection || ! selection->count() )
        return;

    if ( m_isStrokeManipulator )
        manipulateStrokes( id );
    else
        manipulateFills( id );
}

void KarbonStyleButtonBox::setStroke()
{
    m_isStrokeManipulator = true;
    m_group->button( EvenOdd )->setEnabled( false );
    m_group->button( Winding )->setEnabled( false );
}

void KarbonStyleButtonBox::setFill()
{
    m_isStrokeManipulator = false;
    m_group->button( EvenOdd )->setEnabled( true );
    m_group->button( Winding )->setEnabled( true );
}

bool KarbonStyleButtonBox::isStrokeManipulator() const
{
    return m_isStrokeManipulator;
}

void KarbonStyleButtonBox::manipulateFills( int id )
{
    KoCanvasController* canvasController = KoToolManager::instance()->activeCanvasController();
    KoSelection *selection = canvasController->canvas()->shapeManager()->selection();

    QUndoCommand * cmd = 0;

    switch( id )
    {
        case None:
            cmd = new KoShapeBackgroundCommand( selection->selectedShapes(), QBrush() );
            break;
        case Solid:
        {
            QBrush fill = QBrush( canvasController->canvas()->resourceProvider()->backgroundColor().toQColor() );
            cmd = new KoShapeBackgroundCommand( selection->selectedShapes(), fill );
            break;
        }
        case Gradient:
            //m_fill.setType( VFill::grad );
            break;
        case Pattern:
            //m_fill.setType( VFill::patt );
            break;
        case EvenOdd:
        case Winding:
        {
            Qt::FillRule fillRule = id == EvenOdd ? Qt::OddEvenFill : Qt::WindingFill;
            QList<KoPathShape*> shapes;
            foreach( KoShape * shape, selection->selectedShapes() )
            {
                KoPathShape * path = dynamic_cast<KoPathShape*>( shape );
                if( path && path->fillRule() != fillRule )
                    shapes.append( path );
            }
            if( shapes.count() )
                cmd = new KoPathFillRuleCommand( shapes, fillRule );
            break;
        }
    }
    if( cmd )
        canvasController->canvas()->addCommand( cmd );
}

void KarbonStyleButtonBox::manipulateStrokes( int id )
{
    KoCanvasController* canvasController = KoToolManager::instance()->activeCanvasController();
    // TODO get the actual border stroke from the ResourceProvider
    KoShapeBorderModel * border = 0;
    switch( id )
    {
        case None:
        break;
        case Solid:
        {
            QColor color = canvasController->canvas()->resourceProvider()->foregroundColor().toQColor();
            border = new KoLineBorder( 1.0, color );
            break;
        }
        case Gradient:
            //m_stroke.setType( VStroke::grad );
            break;
        case Pattern:
            //m_stroke.setType( VStroke::patt );
            break;
    }
    KoSelection *selection = canvasController->canvas()->shapeManager()->selection();
    canvasController->canvas()->addCommand( new KoShapeBorderCommand( selection->selectedShapes(), border ) );
}

#include "KarbonStyleButtonBox.moc"

