/* $Id$ */

#include "kchartWizardSelectChartSubTypePage.h"
#include "kchart_view.h"


#include <qpushbutton.h>
#include <qbuttongroup.h>
#include <qlabel.h>
#include <qcollection.h>
#include <qobjectlist.h>

#include <kiconloader.h>
#include <kglobal.h>
#include <klocale.h>
#include <qlayout.h>

kchartWizardSelectChartSubTypePage::kchartWizardSelectChartSubTypePage( QWidget* parent,
						    KChartPart* chart ) :
  QWidget( parent ),
  _chart( chart )
{
    //  _charttype = _chart->chartType();
    chartSubType=true;

    QGridLayout *grid1 = new QGridLayout(this,2,2,15,15);

    QVBoxLayout *lay1 = new QVBoxLayout( this );
    lay1->setMargin( 5 );
    lay1->setSpacing( 10 );

    QButtonGroup *grp = new QButtonGroup( 1, QGroupBox::Horizontal, i18n( "Chart Sub Type" ),this );
    grp->setRadioButtonExclusive( TRUE );
    grp->layout();
    lay1->addWidget(grp);
    depth=new QRadioButton( i18n("Depth"), grp ); ;
    sum=new QRadioButton( i18n("Sum"), grp );
    beside=new QRadioButton( i18n("Beside"), grp );
    layer=new QRadioButton( i18n("Layer"), grp );
    percent=new QRadioButton( i18n("Percent (only bar2D and bar3D)"), grp );
    switch((int)_chart->params()->stack_type)
        {
         case (int)KCHARTSTACKTYPE_DEPTH:
                {
                 depth->setChecked(true);
                 break;
                }
         case (int)KCHARTSTACKTYPE_SUM:
                {
                 sum->setChecked(true);
                 break;
                }
         case (int)KCHARTSTACKTYPE_BESIDE:
                {
                 beside->setChecked(true);
                 break;
                }
          case (int)KCHARTSTACKTYPE_LAYER:
                {
                 layer->setChecked(true);
                 break;
                 }
          case (int)KCHARTSTACKTYPE_PERCENT:
                {
                 percent->setChecked(true);
                 break;
                }
          default:
                {
                 cout <<"Error in stack_type\n";
                 break;
                }
        }
if(!chartSubType)
        grp->setEnabled(false);

grid1->addWidget(grp,0,0);
}



void kchartWizardSelectChartSubTypePage::apply()
{
if(chartSubType)
{
 if(depth->isChecked())
        {
        _chart->params()->stack_type = KCHARTSTACKTYPE_DEPTH;
        }
 else if(sum->isChecked())
        {
        _chart->params()->stack_type = KCHARTSTACKTYPE_SUM;
        }
 else if(beside->isChecked())
        {
        _chart->params()->stack_type = KCHARTSTACKTYPE_BESIDE;
        }
 else if(layer->isChecked())
        {
        _chart->params()->stack_type = KCHARTSTACKTYPE_LAYER;
        }
 else if(percent->isChecked())
        {
        _chart->params()->stack_type = KCHARTSTACKTYPE_PERCENT;
        }

 else
        {
        cout <<"Error in groupbutton\n";
        }
 }
else
 {
 _chart->params()->stack_type = KCHARTSTACKTYPE_DEPTH;
 }
}


#include "kchartWizardSelectChartSubTypePage.moc"
