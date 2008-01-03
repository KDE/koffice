/* This file is part of the KDE project
  Copyright (C) 2002-2004 Alexander Dymo <cloudtemple@mksat.net>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MEm_viewHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/
#include "kudesigner_view.h"
#include "kudesigner_factory.h"
#include "kudesigner_doc.h"

#include <map>

#include <qpainter.h>
#include <qicon.h>
#include <qinputdialog.h>
#include <qevent.h>
#include <q3mainwindow.h>
#include <qaction.h>
#include <QLayout>
#include <q3dockwindow.h>
#include <qmenubar.h>
#include <qmessagebox.h>
#include <QSpinBox>
#include <QLabel>
//Added by qt3to4:
#include <Q3VBoxLayout>
#include <QResizeEvent>
#include <QPaintEvent>
#include <QDockWidget>
#include <kicon.h>
#include <kaction.h>
#include <kstandardaction.h>
#include <klocale.h>
#include <kdebug.h>
#include <kicon.h>
#include <ktoggleaction.h>
#include <kactioncollection.h>
#include <KoMainWindow.h>

#include <commdefs.h>
#include <view.h>
#include <structurewidget.h>
#include <canvas.h>
#include <command.h>

#include <field.h>
#include <calcfield.h>
#include <label.h>
#include <line.h>
#include <specialfield.h>

#include <kugartemplate.h>
#include <reportheader.h>
#include <reportfooter.h>
#include <pageheader.h>
#include <pagefooter.h>
#include <detailheader.h>
#include <detailfooter.h>
#include <detail.h>

#include <koproperty/editor.h>
#include <koproperty/property.h>

using namespace Kudesigner;

KudesignerView::KudesignerView( KudesignerDoc* part, QWidget* parent)
        : KoView( part, parent ), m_propertyEditor( 0 ), m_doc( part )
{
    setComponentData( KudesignerFactory::global() );
    if ( !part->isReadWrite() )  // readonly case, e.g. when embedded into konqueror
        setXMLFile( "kudesigner_readonly.rc" ); // simplified GUI
    else
        setXMLFile( "kudesignerui.rc" );

    Q3VBoxLayout *l = new Q3VBoxLayout( this, 0, 0 );
    m_view = new Kudesigner::View( part->canvas(), this );
    if ( part->plugin() )
    {
        m_view->setAcceptDrops( part->plugin() ->acceptsDrops() );
        m_view->viewport() ->setAcceptDrops( part->plugin() ->acceptsDrops() );
        m_view->setPlugin( part->plugin() );
    }
    l->addWidget( m_view );

    m_view->viewport() ->setFocusProxy( m_view );
    m_view->viewport() ->setFocusPolicy( Qt::WheelFocus );
    m_view->setFocus();

    m_view->itemToInsert = 0;

    QDockWidget  *dw1 = new QDockWidget( shell() );
    QDockWidget  *dw2 = new QDockWidget( shell() );
    m_structure = new Kudesigner::StructureWidget( dw1 );
    m_propertyEditor = new Editor( dw2 );
#ifdef __GNUC__
#warning "kde4: port it"
#endif
#if 0
	dw1->boxLayout() ->addWidget( m_structure, 1 );
    dw2->boxLayout() ->addWidget( m_propertyEditor, 1 );
    dw1->setFixedExtentWidth( 400 );
    dw1->setResizeEnabled( true );
    dw2->setFixedExtentWidth( 400 );
    dw2->setResizeEnabled( true );
#endif
    if ( m_doc->plugin() )
    {
        //                 connect( m_propertyEditor, SIGNAL(createPluggedInEditor(QWidget*&, Editor *, Property*, Box *)),
        //                          m_doc->plugin(), SLOT(createPluggedInEditor(QWidget*&, Editor *, Property*, Box *)));

        kDebug() <<"*************Property and plugin have been connected";
    }

    shell() ->addDockWidget( m_doc->propertyPosition(),dw1 );
    shell() ->addDockWidget( m_doc->propertyPosition(),dw2 );

    m_structure->setDocument( m_doc->canvas() );

    connect( m_doc, SIGNAL( canvasChanged( Kudesigner::Canvas * ) ),
             m_structure, SLOT( setDocument( Kudesigner::Canvas * ) ) );
    connect( m_doc->canvas(), SIGNAL( structureModified() ),
             m_structure, SLOT( refresh() ) );

    connect( m_view, SIGNAL( selectionMade( Buffer* ) ),
             this, SLOT( populateProperties( Buffer* ) ) );

    connect( m_view, SIGNAL( selectionClear() ),
             m_propertyEditor, SLOT( clear() ) );

    connect( m_view, SIGNAL( changed() ),
             m_doc, SLOT( setModified() ) );

    connect( m_view, SIGNAL( selectionMade( Buffer* ) ),
             m_structure, SLOT( selectionMade() ) );
    connect( m_view, SIGNAL( selectionClear() ),
             m_structure, SLOT( selectionClear() ) );

    connect( m_view, SIGNAL( selectedActionProcessed() ), this, SLOT( unselectItemAction() ) );
    connect( m_view, SIGNAL( modificationPerformed() ), part, SLOT( setModified() ) );
    connect( m_view, SIGNAL( itemPlaced( int, int, int, int ) ), this, SLOT( placeItem( int, int, int, int ) ) );

    gridLabel = new QLabel( i18n( "Grid size:" ), shell() );
    gridBox = new QSpinBox( 1, 100, 1, shell() );
    gridBox->setValue( 10 );
    connect( gridBox, SIGNAL( valueChanged( int ) ), m_view, SLOT( setGridSize( int ) ) );

    initActions();

    show();
    m_view->show();
    m_structure->refresh();
}

KudesignerView::~KudesignerView()
{
    delete gridLabel;
    delete gridBox;
}

void KudesignerView::paintEvent( QPaintEvent* ev )
{
    QPainter painter;
    painter.begin( this );

    // Let the document do the drawing
    koDocument() ->paintEverything( painter, ev->rect(), this );

    painter.end();
}

void KudesignerView::resizeEvent( QResizeEvent* /*_ev*/ )
{
    m_view->setGeometry( 0, 0, width(), height() );
}

void KudesignerView::initActions()
{
    cutAction = KStandardAction::cut( this, SLOT( cut() ), actionCollection() );
    copyAction = KStandardAction::copy( this, SLOT( copy() ), actionCollection() );
    pasteAction = KStandardAction::paste( this, SLOT( paste() ), actionCollection() );
    selectAllAction = KStandardAction::selectAll( this, SLOT( selectAll() ), actionCollection() );
    deleteAction  = new KAction(KIcon("edit-delete"), i18n("Delete"), this);
    actionCollection()->addAction("edit_delete", deleteAction );
    connect(deleteAction, SIGNAL(triggered(bool) ), SLOT( deleteItems() ));
    cutAction->setEnabled( false );
    copyAction->setEnabled( false );
    pasteAction->setEnabled( false );
    //    deleteAction->setEnabled(false);

    sectionsReportHeader  = new KAction(KIcon("irh"), i18n("Report Header"), this);
    actionCollection()->addAction("rheader", sectionsReportHeader );
    connect(sectionsReportHeader, SIGNAL(triggered(bool) ), SLOT( slotAddReportHeader() ));
    sectionsReportFooter  = new KAction(KIcon("irf"), i18n("Report Footer"), this);
    actionCollection()->addAction("rfooter", sectionsReportFooter );
    connect(sectionsReportFooter, SIGNAL(triggered(bool) ), SLOT( slotAddReportFooter() ));
    sectionsPageHeader  = new KAction(KIcon("iph"), i18n("Page Header"), this);
    actionCollection()->addAction("pheader", sectionsPageHeader );
    connect(sectionsPageHeader, SIGNAL(triggered(bool) ), SLOT( slotAddPageHeader() ));
    sectionsPageFooter  = new KAction(KIcon("ipf"), i18n("Page Footer"), this);
    actionCollection()->addAction("pfooter", sectionsPageFooter );
    connect(sectionsPageFooter, SIGNAL(triggered(bool) ), SLOT( slotAddPageFooter() ));
    sectionsDetailHeader  = new KAction(KIcon("idh"), i18n("Detail Header"), this);
    actionCollection()->addAction("dheader", sectionsDetailHeader );
    connect(sectionsDetailHeader, SIGNAL(triggered(bool) ), SLOT( slotAddDetailHeader() ));
    sectionsDetail  = new KAction(KIcon("id"), i18n("Detail"), this);
    actionCollection()->addAction("detail", sectionsDetail );
    connect(sectionsDetail, SIGNAL(triggered(bool) ), SLOT( slotAddDetail() ));
    sectionsDetailFooter  = new KAction(KIcon("idf"), i18n("Detail Footer"), this);
    actionCollection()->addAction("dfooter", sectionsDetailFooter );
    connect(sectionsDetailFooter, SIGNAL(triggered(bool) ), SLOT( slotAddDetailFooter() ));

    itemsNothing  = new KToggleAction(KIcon("select-rectangular"), i18n("Clear Selection"), this);
    actionCollection()->addAction("nothing", itemsNothing );
    connect(itemsNothing, SIGNAL(triggered(bool)), SLOT( slotAddItemNothing() ));
    itemsNothing->setChecked( true );
    itemsLabel  = new KToggleAction(KIcon("insert-object"), i18n("Label"), this);
    actionCollection()->addAction("label", itemsLabel );
    connect(itemsLabel, SIGNAL(triggered(bool)), SLOT( slotAddItemLabel() ));
    itemsField  = new KToggleAction(KIcon("frame_field"), i18n("Field"), this);
    actionCollection()->addAction("field", itemsField );
    connect(itemsField, SIGNAL(triggered(bool)), SLOT( slotAddItemField() ));
    itemsSpecial  = new KToggleAction(KIcon("frame_query"), i18n("Special Field"), this);
    actionCollection()->addAction("special", itemsSpecial );
    connect(itemsSpecial, SIGNAL(triggered(bool)), SLOT( slotAddItemSpecial() ));
    itemsCalculated  = new KToggleAction(KIcon("frame_formula"), i18n("Calculated Field"), this);
    actionCollection()->addAction("calcfield", itemsCalculated );
    connect(itemsCalculated, SIGNAL(triggered(bool)), SLOT( slotAddItemCalculated() ));
    itemsLine  = new KToggleAction(KIcon("frame_chart"), i18n("Line"), this);
    actionCollection()->addAction("line", itemsLine );
    connect(itemsLine, SIGNAL(triggered(bool)), SLOT( slotAddItemLine() ));

    QActionGroup *group = new QActionGroup( this );
    group->addAction( itemsNothing );
    group->addAction( itemsLabel);
    group->addAction( itemsField);
    group->addAction( itemsSpecial );
    group->addAction( itemsCalculated );
    group->addAction( itemsLine );

    gridActionLabel  = new KAction(i18n("Grid Label"), this);
    actionCollection()->addAction("gridlabel", gridActionLabel );
    gridActionLabel->setDefaultWidget( gridLabel );

    gridAction  = new KAction(i18n("Grid Size"), this);
    actionCollection()->addAction("gridaction", gridAction );
    gridAction->setDefaultWidget( gridBox );
}

void KudesignerView::updateReadWrite( bool /*readwrite*/ )
{
}

void KudesignerView::guiActivateEvent( KParts::GUIActivateEvent *ev )
{
    if ( ev->activated() )
        m_propertyEditor->show();
    else
        m_propertyEditor->hide();
    KoView::guiActivateEvent( ev );
}

void KudesignerView::populateProperties( Buffer *buf )
{
    connect( buf, SIGNAL( propertyChanged(KoProperty::Set&, KoProperty::Property&) ), m_doc->canvas(), SLOT( changed() ) );
    m_propertyEditor->changeSet( buf );
}

void KudesignerView::cut()
{
    //    kDebug(31000) <<"KudesignerView::cut(): CUT called";
}

void KudesignerView::copy()
{
    //    kDebug(31000) <<"KudesignerView::copy(): COPY called";
}

void KudesignerView::paste( )
{}

void KudesignerView::deleteItems( )
{
    if ( m_doc->canvas() ->selected.count() > 0 )
        m_doc->addCommand( new DeleteReportItemsCommand( m_doc->canvas(), m_doc->canvas() ->selected ) );
}

void KudesignerView::selectAll( )
{
    m_doc->canvas() ->selectAll();
}

void KudesignerView::slotAddReportHeader()
{
    if ( !( ( ( KudesignerDoc * ) ( koDocument() ) ) ) ->canvas() ->kugarTemplate() ->reportHeader )
    {
        m_doc->addCommand( new AddReportHeaderCommand( m_doc->canvas() ) );
    }
}

void KudesignerView::slotAddReportFooter()
{
    if ( !( ( ( KudesignerDoc * ) ( koDocument() ) ) ) ->canvas() ->kugarTemplate() ->reportFooter )
    {
        m_doc->addCommand( new AddReportFooterCommand( m_doc->canvas() ) );
    }
}

void KudesignerView::slotAddPageHeader()
{
    if ( !( ( ( KudesignerDoc * ) ( koDocument() ) ) ) ->canvas() ->kugarTemplate() ->pageHeader )
    {
        m_doc->addCommand( new AddPageHeaderCommand( m_doc->canvas() ) );
    }
}

void KudesignerView::slotAddPageFooter()
{
    if ( !( ( ( KudesignerDoc * ) ( koDocument() ) ) ) ->canvas() ->kugarTemplate() ->pageFooter )
    {
        m_doc->addCommand( new AddPageFooterCommand( m_doc->canvas() ) );
    }
}

void KudesignerView::slotAddDetailHeader()
{
    bool Ok = false;
    unsigned int level = QInputDialog::getInteger( tr( "Add Detail Header" ), tr( "Enter detail level:" ),
                         0, 0, 100, 1, &Ok, this );
    if ( !Ok )
        return ;
    if ( m_doc->canvas() ->kugarTemplate() ->detailsCount >= level )
    {
        m_doc->addCommand( new AddDetailHeaderCommand( level, m_doc->canvas() ) );
    }
}

void KudesignerView::slotAddDetail()
{
    bool Ok = false;
    unsigned int level = QInputDialog::getInteger( tr( "Add Detail" ), tr( "Enter detail level:" ),
                         0, 0, 100, 1, &Ok, this );
    if ( !Ok )
        return ;
    if ( ( ( level == 0 ) && ( m_doc->canvas() ->kugarTemplate() ->detailsCount == 0 ) )
            || ( m_doc->canvas() ->kugarTemplate() ->detailsCount == level ) )
    {
        m_doc->addCommand( new AddDetailCommand( level, m_doc->canvas() ) );
    }
}

void KudesignerView::slotAddDetailFooter()
{
    bool Ok = false;
    unsigned int level = QInputDialog::getInteger( tr( "Add Detail Footer" ), tr( "Enter detail level:" ),
                         0, 0, 100, 1, &Ok, this );
    if ( !Ok )
        return ;

    if ( m_doc->canvas() ->kugarTemplate() ->detailsCount >= level )
    {
        m_doc->addCommand( new AddDetailFooterCommand( level, m_doc->canvas() ) );
    }
}

void KudesignerView::slotAddItemNothing()
{
    if ( m_doc->canvas() )
    {
        if ( m_view->itemToInsert )
        {
            m_view->itemToInsert = 0;
        }
    }
}

void KudesignerView::slotAddItemLabel()
{
    if ( m_doc->canvas() )
    {
        m_view->itemToInsert = Rtti_Label;
    }
}

void KudesignerView::slotAddItemField()
{
    if ( m_doc->canvas() )
    {
        m_view->itemToInsert = Rtti_Field;
    }
}

void KudesignerView::slotAddItemSpecial()
{
    if ( m_doc->canvas() )
    {
        m_view->itemToInsert = Rtti_Special;
    }
}

void KudesignerView::slotAddItemCalculated()
{
    if ( m_doc->canvas() )
    {
        m_view->itemToInsert = Rtti_Calculated;
    }
}

void KudesignerView::slotAddItemLine()
{
    if ( m_doc->canvas() )
    {
        m_view->itemToInsert = Rtti_Line;
    }
}

void KudesignerView::unselectItemAction()
{
    /*    itemsNothing->setOn(true);*/
}

void KudesignerView::placeItem( int x, int y, int band, int bandLevel )
{
    m_doc->addCommand( new AddReportItemCommand( m_doc->canvas(), m_view, x, y, ( Kudesigner::RttiValues ) band, bandLevel ) );
}

#include "kudesigner_view.moc"
