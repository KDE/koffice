/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Reginald Stadlbauer <reggie@kde.org>
   Copyright (C) 2002, 2003 Ariya Hidayat <ariya@kde.org>
   Copyright (C) 2004, 2005 Laurent Montel <montel@kde.org>

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

#include "KPrPgConfDia.h"
#include "KPrDocument.h"
#include "KPrPage.h"

#include <qbuttongroup.h>
#include <qhbuttongroup.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qhbox.h>
#include <qvbox.h>
#include <qvgroupbox.h>
#include <qheader.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlistview.h>
#include <qpen.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qvaluelist.h>
#include <qvbuttongroup.h>
#include <qwhatsthis.h>

#include <kcolorbutton.h>
#include <kglobal.h>
#include <klocale.h>
#include <knuminput.h>
#include <qslider.h>

KPrPgConfDia::KPrPgConfDia( QWidget* parent, KPrDocument* doc )
    : KDialogBase( KDialogBase::Tabbed, i18n("Configure Slide Show"),
                   Ok|Cancel, Ok, parent, "pgConfDia", true ),
      m_doc( doc )
{
    setupPageGeneral();
    setupPageSlides();

    connect( this, SIGNAL( okClicked() ), this, SLOT( confDiaOk() ) );
    connect( this, SIGNAL( okClicked() ), this, SLOT( accept() ) );
}

void KPrPgConfDia::setupPageGeneral()
{
    QFrame* generalPage = addPage( i18n("&General") );
    QWhatsThis::add( generalPage, i18n("<p>This dialog allows you to configure how the slideshow "
				       "will be displayed, including whether the slides are "
				       "automatically sequenced or manually controlled, and also "
				       "allows you to configure a <em>drawing pen</em> that can "
				       "be used during the display of the presentation to add "
				       "additional information or to emphasise particular points.</p>") );
    QVBoxLayout *generalLayout = new QVBoxLayout( generalPage, 0, KDialog::spacingHint() );

    QVButtonGroup *switchGroup = new QVButtonGroup( i18n("&Transition Type"), generalPage );
    generalLayout->addWidget( switchGroup );
    QWhatsThis::add( switchGroup, i18n("<li><p>If you select <b>Manual transition to next step or slide</b> "
					  "then each transition and effect on a slide, or transition from "
					  "one slide to the next, will require an action. Typically this "
					  "action will be a mouse click, or the space bar.</p></li>"
					  "<li><p>If you select <b>Automatic transition to next step or slide</b> "
					  "then the presentation will automatically sequence each transition "
					  "and effect on a slide, and will automatically transition to the "
					  "next slide when the current slide is fully displayed. The speed "
					  "of sequencing is controlled using the slider below. This also "
					  "enables the option to automatically loop back to the first "
					  "slide after the last slide has been shown.</p></li>") );
    m_manualButton = new QRadioButton( i18n("&Manual transition to next step or slide"), switchGroup );
    m_manualButton->setChecked( m_doc->spManualSwitch() );
    m_autoButton = new QRadioButton( i18n("&Automatic transition to next step or slide"), switchGroup );
    m_autoButton->setChecked( !m_doc->spManualSwitch() );

    infiniteLoop = new QCheckBox( i18n( "&Infinite loop" ), generalPage );
    generalLayout->addWidget( infiniteLoop );
    QWhatsThis::add( infiniteLoop, i18n("<p>If this checkbox is selected, then the slideshow "
					"will restart at the first slide after the last slide "
					"has been displayed. It is only available if the "
					"<b>Automatic transition to next step or slide</b> "
					"button is selected above.</p> <p>This option may be "
					"useful if you are running a promotional display.</p>") );

    infiniteLoop->setEnabled( !m_doc->spManualSwitch() );
    connect( m_autoButton, SIGNAL( toggled(bool) ), infiniteLoop, SLOT( setEnabled(bool) ) );
    connect( m_autoButton, SIGNAL( toggled(bool) ), infiniteLoop, SLOT( setChecked(bool) ) );

    endOfPresentationSlide = new QCheckBox( i18n( "&Show 'End of presentation' slide" ), generalPage );
    generalLayout->addWidget( endOfPresentationSlide );
    QWhatsThis::add( endOfPresentationSlide, i18n("<p>If this checkbox is selected, when the slideshow "
					"has finished a black slideshow containing the "
					"message 'End of presentation. Click to exit' will "
					"be shown.") );
    endOfPresentationSlide->setChecked( m_doc->spShowEndOfPresentationSlide() );
    endOfPresentationSlide->setDisabled( infiniteLoop->isEnabled() && getInfiniteLoop() );
    connect( infiniteLoop, SIGNAL( toggled(bool) ), endOfPresentationSlide, SLOT( setDisabled(bool) ) );

    presentationDuration = new QCheckBox( i18n( "Measure presentation &duration" ), generalPage );
    generalLayout->addWidget( presentationDuration );
    QWhatsThis::add( presentationDuration, i18n("<p>If this checkbox is selected, the time that "
						"each slide was displayed for, and the total time "
						"for the presentation will be measured.</p> "
						"<p>The times will be displayed at the end of the "
						"presentation.</p> "
						"<p>This can be used during rehearsal to check "
						"coverage for each issue in the presentation, "
						"and to verify that the presentation duration "
						"is correct.</p>" ) );
    presentationDuration->setChecked( m_doc->presentationDuration() );

    // presentation pen (color and width)

    QGroupBox* penGroup = new QGroupBox( 2, Qt::Horizontal, i18n("Presentation Pen") , generalPage );
    generalLayout->addWidget( penGroup );
    QWhatsThis::add( penGroup, i18n("<p>This part of the dialog allows you to configure the "
				    "<em>drawing mode</em>, which allows you to add additional "
				    "information, emphasise particular content, or to correct "
				    "errors during the presentation by drawing on the slides "
				    "using the mouse.</p>"
				    "<p>You can configure the color of the drawing pen and the "
				    "width of the pen.</p>" ) );
    penGroup->layout()->setSpacing(KDialog::marginHint());
    penGroup->layout()->setMargin(KDialog::spacingHint());
    //QGridLayout *grid = new QGridLayout(penGroup->layout(), 3, 2 );

    QLabel* label = new QLabel( i18n( "Color:" ), penGroup );
    //grid->addWidget( label, 0, 0 );
    penColor = new KColorButton( m_doc->presPen().color(), m_doc->presPen().color(), penGroup );
    //grid->addWidget( penColor, 0, 1 );

    label = new QLabel( i18n( "Width:" ), penGroup );
    // grid->addWidget( label, 1, 0 );
    penWidth = new QSpinBox( 1, 10, 1, penGroup );
    penWidth->setSuffix( i18n(" pt") );
    penWidth->setValue( m_doc->presPen().width() );
    //grid->addWidget( penWidth, 1, 1 );

    generalLayout->addStretch();
}

void KPrPgConfDia::setupPageSlides()
{
    QFrame* slidesPage = addPage( i18n("&Slides") );
    QWhatsThis::add( slidesPage, i18n("<p>This dialog allows you to configure which slides "
				      "are used in the presentation. Slides that are not "
				      "selected will not be displayed during the slide "
				      "show.</p>") );
    QGridLayout *slidesLayout = new QGridLayout( slidesPage,7 , 2, 0, KDialog::spacingHint());


    QButtonGroup *group=new QVButtonGroup( slidesPage );
    group->setRadioButtonExclusive( true );

    m_customSlide = new QRadioButton( i18n( "Custom slide show" ), group, "customslide" );

    connect( m_customSlide, SIGNAL( clicked () ), this, SLOT( radioButtonClicked() ) );

    QHBox *box = new QHBox( group );

    m_labelCustomSlide = new QLabel( i18n( "Custom slide:" ),box );

    m_customSlideCombobox = new QComboBox( box );
    m_customSlideCombobox->insertStringList( m_doc->presentationList() );

    m_selectedSlide = new QRadioButton( i18n( "Selected pages:" ), group, "selectedslide" );
    slidesLayout->addMultiCellWidget( group, 0,2,0,1 );
    connect( m_selectedSlide, SIGNAL( clicked () ), this, SLOT( radioButtonClicked() ) );

    slides = new QListView( slidesPage );
    slidesLayout->addMultiCellWidget( slides, 3, 3, 0, 1 );
    slidesLayout->setRowStretch( 3, 10 );
    slides->addColumn( i18n("Slide") );
    slides->setSorting( -1 );
    slides->header()->hide();

    for ( int i = m_doc->getPageNums() - 1; i >= 0; --i )
    {
        KPrPage *page=m_doc->pageList().at( i );
        QCheckListItem* item = new QCheckListItem( slides,
                                                   page->pageTitle(),
                                                   QCheckListItem::CheckBox );
        item->setOn( page->isSlideSelected() );
    }

    QHBox* buttonGroup = new QHBox( slidesPage );
    buttonGroup->setSpacing( KDialog::spacingHint() );

    QPushButton* selectAllButton = new QPushButton( i18n( "Select &All" ), buttonGroup );
    connect( selectAllButton, SIGNAL( clicked() ), this, SLOT( selectAllSlides() ) );

    QPushButton* deselectAllButton = new QPushButton( i18n( "&Deselect All" ), buttonGroup );
    connect( deselectAllButton, SIGNAL( clicked() ), this, SLOT( deselectAllSlides() ) );

    QWidget* spacer = new QWidget( buttonGroup );

    spacer->setSizePolicy( QSizePolicy( QSizePolicy::Minimum, QSizePolicy::Expanding ) );
    slidesLayout->addMultiCellWidget( buttonGroup, 4, 4, 0, 1 );

    if ( !m_doc->presentationName().isEmpty() )
    {
        m_customSlide->setChecked( true );
        m_customSlideCombobox->setCurrentText( m_doc->presentationName() );
    }
    else
        m_selectedSlide->setChecked( true );

    if ( m_customSlideCombobox->count()==0 )
    {
        m_customSlide->setEnabled( false );
        m_labelCustomSlide->setEnabled( false );
        m_customSlideCombobox->setEnabled( false );
    }
    radioButtonClicked();
}

KPrPgConfDia::~KPrPgConfDia()
{
}

void KPrPgConfDia::radioButtonClicked()
{
    if ( m_customSlide->isChecked() )
    {
        m_labelCustomSlide->setEnabled( true );
        m_customSlideCombobox->setEnabled( true );
        slides->setEnabled( false );
    }
    else
    {
        m_labelCustomSlide->setEnabled( false );
        m_customSlideCombobox->setEnabled( false );
        slides->setEnabled( true );
    }
}

bool KPrPgConfDia::getInfiniteLoop() const
{
    return infiniteLoop->isChecked();
}

bool KPrPgConfDia::getShowEndOfPresentationSlide() const
{
    return endOfPresentationSlide->isChecked();
}

bool KPrPgConfDia::getManualSwitch() const
{
    return m_manualButton->isChecked();
}

bool KPrPgConfDia::getPresentationDuration() const
{
    return presentationDuration->isChecked();
}

QPen KPrPgConfDia::getPen() const
{
    return QPen( penColor->color(), penWidth->value() );
}

QValueList<bool> KPrPgConfDia::getSelectedSlides() const
{
    QValueList<bool> selectedSlides;

    QListViewItem *item = slides->firstChild();
    while( item )
    {
        QCheckListItem *checkItem = dynamic_cast<QCheckListItem*>( item );
        bool selected = false;
        if( checkItem ) selected = checkItem->isOn();
        item = item->nextSibling();
        selectedSlides.append( selected );
    }
    return selectedSlides;
}

void KPrPgConfDia::selectAllSlides()
{
    QListViewItem *item = slides->firstChild();
    while( item )
    {
        QCheckListItem *checkItem = dynamic_cast<QCheckListItem*>( item );
        if( checkItem ) checkItem->setOn( true );
        item = item->nextSibling();
    }
}

void KPrPgConfDia::deselectAllSlides()
{
    QListViewItem *item = slides->firstChild();
    while( item )
    {
        QCheckListItem *checkItem = dynamic_cast<QCheckListItem*>( item );
        if( checkItem ) checkItem->setOn( false );
        item = item->nextSibling();
    }
}

QString KPrPgConfDia::presentationName() const
{
    if ( m_customSlide->isChecked() )
        return m_customSlideCombobox->currentText();
    else
        return QString::null;
}

#include "KPrGradient.h"
#include "KPrPgConfDia.moc"
