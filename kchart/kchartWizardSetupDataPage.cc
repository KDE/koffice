/* $Id$ */

#include "kchartWizardSetupDataPage.h"
#include "kchart_view.h"
#include "kchartWizard.h"

#include <qradiobutton.h>
#include <qcheckbox.h>
#include <qbuttongroup.h>
#include <qlabel.h>

kchartWizardSetupDataPage::kchartWizardSetupDataPage( QWidget* parent,
													  KChartPart* chart) :
  QWidget( parent ),
  _chart( chart ),
  _parent( (kchartWizard*)parent )
{
  QButtonGroup* tmpQGroupBox;
  tmpQGroupBox = new QButtonGroup( this, "GroupBox_1" );
  tmpQGroupBox->setGeometry( 260, 10, 130, 80 );
  tmpQGroupBox->setFrameStyle( 49 );
  tmpQGroupBox->setAlignment( 1 );
  
  datarow = new QRadioButton( this, "RadioButton_1" );
  datarow->setGeometry( 270, 40, 100, 20 );
  datarow->setText( i18n("Rows") );
  connect( datarow, SIGNAL( clicked() ), 
		   this, SLOT( dataInRowsClicked() ) );
  tmpQGroupBox->insert( datarow );
  
  datacol = new QRadioButton( this, "RadioButton_2" );
  datacol->setGeometry( 270, 60, 100, 20 );
  datacol->setText( i18n("Columns") );
  datacol->setChecked( true );
  connect( datarow, SIGNAL( clicked() ),
		   this, SLOT( dataInColsClicked() ) );
  tmpQGroupBox->insert( datacol );
  
  QLabel* tmpQLabel;
  tmpQLabel = new QLabel( this, "Label_3" );
  tmpQLabel->setGeometry( 270, 20, 100, 20 );
  tmpQLabel->setText( i18n("Data is in:") );
  
  QButtonGroup* descrBG = new QButtonGroup( this );
  descrBG->hide();
  coldescript = new QCheckBox( this, "CheckBox_3" );
  coldescript->setGeometry( 260, 110, 20, 30 );
  coldescript->setChecked( true );
  connect( coldescript, SIGNAL( toggled( bool ) ),
		   this, SLOT( firstColumnIsDescriptionToggled( bool ) ) );
  coldescript->setText( "" );
  descrBG->insert( coldescript );
  
  tmpQLabel = new QLabel( this, "Label_4" );
  tmpQLabel->setGeometry( 280, 110, 100, 30 );
  tmpQLabel->setText( i18n("Use first row as description") );
  tmpQLabel->setAlignment( 1313 );
  
  rowdescript = new QCheckBox( this, "CheckBox_4" );
  rowdescript->setGeometry( 260, 150, 20, 30 );
  connect( rowdescript, SIGNAL( toggled( bool ) ),
		   this, SLOT( firstRowIsDescriptionToggled( bool ) ) );
  rowdescript->setText( "" );
  descrBG->insert( rowdescript );
  
  tmpQLabel = new QLabel( this, "Label_5" );
  tmpQLabel->setGeometry( 280, 150, 100, 30 );
  tmpQLabel->setText( i18n("Use first column as description") );
  tmpQLabel->setAlignment( 1313 );
  
  QFrame* tmpQFrame;
  tmpQFrame = new QFrame( this, "Frame_1" );
  tmpQFrame->setGeometry( 10, 10, 240, 220 );
  tmpQFrame->setFrameStyle( QFrame::Panel | QFrame::Sunken );
  tmpQFrame->setLineWidth( 2 );

  /*
  preview = new kchartWidget( _chart, tmpQFrame );
  preview->show();
  _chart->addAutoUpdate( preview );
  preview->resize( tmpQFrame->contentsRect().width(),
				   tmpQFrame->contentsRect().height() );
  */
  parent->resize( 400, 350 );
}


kchartWizardSetupDataPage::~kchartWizardSetupDataPage()
{
    /*
  _chart->removeAutoUpdate( preview );
    */
}


void kchartWizardSetupDataPage::dataInRowsClicked()
{
  _parent->emitNeedNewData( _parent->dataArea(), kchartWizard::Row,
								 coldescript->isChecked(), 
								 rowdescript->isChecked() );
}

void kchartWizardSetupDataPage::dataInColsClicked()
{
  _parent->emitNeedNewData( _parent->dataArea(), kchartWizard::Col,
							coldescript->isChecked(),
							rowdescript->isChecked() );
}

void kchartWizardSetupDataPage::firstColumnIsDescriptionToggled( bool )
{
  _parent->emitNeedNewData( _parent->dataArea(), 
							( datarow->isChecked() ? kchartWizard::Row :
							  kchartWizard::Col ),
							coldescript->isChecked(),
							rowdescript->isChecked() );
}

void kchartWizardSetupDataPage::firstRowIsDescriptionToggled( bool )
{
  _parent->emitNeedNewData( _parent->dataArea(), 
							( datarow->isChecked() ? kchartWizard::Row :
							  kchartWizard::Col ),
							coldescript->isChecked(),
							rowdescript->isChecked() );
}



#include "kchartWizardSetupDataPage.moc"
