#include "kchartWizardSetupAxesPage.h"
#include "kchart_view.h"
#include "kchart_part.h"

#include <qlabel.h>
#include <qframe.h>
#include <qlineedit.h>
#include <qcheckbox.h>
#include <qgroupbox.h>
#include <qlayout.h>
#include <klocale.h>
#include <kfontdialog.h>
#include <knumvalidator.h>
#include <math.h>
#include <float.h>


#include <qspinbox.h>
#include <qpushbutton.h>
#include <qcolor.h>



#include "kchart_params.h"

KChartWizardSetupAxesPage::KChartWizardSetupAxesPage( QWidget* parent,
                                                      KChartPart* chart ) :
    QWidget( parent ),
    _chart( chart )
{
    chart3d=true;

    QGridLayout *grid1 = new QGridLayout(this,2,2,15,15);

    QGroupBox* tmpQGroupBox;
    tmpQGroupBox = new QGroupBox( this, "GroupBox_1" );
    tmpQGroupBox->setFrameStyle( 49 );

    QGridLayout *grid2 = new QGridLayout(tmpQGroupBox,2,2,15,7);

    grid = new QCheckBox( i18n( "Has grid" ),tmpQGroupBox  );

    grid->setChecked(_chart->params()->showGrid());
    grid2->addWidget(grid,0,0);

    gridColor=new KColorButton(tmpQGroupBox);
    // PENDING(kalle) Put back in
    //     colorGrid=_chart->params()->GridColor;
    gridColor->setColor( colorGrid );
    grid2->addWidget(gridColor,0,1);

    border = new QCheckBox( i18n( "Border" ), tmpQGroupBox );
    // PENDING(kalle) Put back in
    //     border->setChecked( _chart->params()->border );
    grid2->addWidget(border,1,0);
    border->setEnabled(false);

    borderColor=new KColorButton(tmpQGroupBox);
    // PENDING(kalle) Put back in
    //     colorBorder=_chart->params()->LineColor;
    borderColor->setColor( colorBorder );
    grid2->addWidget(borderColor,1,1);
    borderColor->setEnabled(false);

    grid1->addWidget(tmpQGroupBox,0,0);


    tmpQGroupBox = new QGroupBox( this, "GroupBox_2" );
    tmpQGroupBox->setFrameStyle( 49 );

    grid2 = new QGridLayout(tmpQGroupBox,3,2,15,7);

    QLabel *tmpLabel;
    tmpLabel=new QLabel(tmpQGroupBox);
    tmpLabel->setText(i18n("Angle 3D: "));
    grid2->addWidget(tmpLabel,0,0);
    angle = new QSpinBox(1, 90, 1,tmpQGroupBox );
    angle->setValue( _chart->params()->threeDBarAngle() );
    grid2->addWidget(angle,0,1);

    tmpLabel=new QLabel(tmpQGroupBox);
    tmpLabel->setText(i18n("Depth 3D: "));
    grid2->addWidget(tmpLabel,1,0);
    depth = new QSpinBox(1, 40, 1, tmpQGroupBox);
    depth->setValue( static_cast<int>( _chart->params()->threeDBarDepth() ) );
    grid2->addWidget(depth,1,1);

    // Bar width is computed automatically in KDChart (and can be
    //     changed via gaps)
    //     tmpLabel=new QLabel(tmpQGroupBox);
//     tmpLabel->setText(i18n("bar width: "));
//     grid2->addWidget(tmpLabel,2,0);
//     barWidth = new QSpinBox(1, 200, 1, tmpQGroupBox);
//     barWidth->setValue(_chart->params()->bar_width);
//     grid2->addWidget(barWidth,2,1);

    grid1->addWidget(tmpQGroupBox,0,1);

    tmpQGroupBox = new QGroupBox( this, "GroupBox_3" );
    tmpQGroupBox->setFrameStyle( 49 );

    grid2 = new QGridLayout(tmpQGroupBox,3,2,15,7);

    tmpLabel=new QLabel(tmpQGroupBox);
    tmpLabel->setText(i18n("Y interval: "));
    grid2->addWidget(tmpLabel,0,0);

    QString tmp;
    y_interval=new QLineEdit(tmpQGroupBox);
    y_interval->setValidator(new KFloatValidator( y_interval));
    grid2->addWidget(y_interval,0,1);
    // PENDING(kalle) Put back in
    //     if( _chart->params()->requested_yinterval != -DBL_MAX)
//         y_interval->setText( tmp.setNum(_chart->params()->requested_yinterval));

    tmpLabel=new QLabel(tmpQGroupBox);
    tmpLabel->setText(i18n("Y min: "));
    grid2->addWidget(tmpLabel,1,0);
    y_min=new QLineEdit(tmpQGroupBox);
    y_min->setValidator(new KFloatValidator( y_min));
    grid2->addWidget(y_min,1,1);
    // PENDING(kalle) Put back in
    //     if( _chart->params()->requested_ymin != DBL_MAX)
//         y_min->setText( tmp.setNum(_chart->params()->requested_ymin));

    tmpLabel=new QLabel(tmpQGroupBox);
    tmpLabel->setText(i18n("Y max: "));
    grid2->addWidget(tmpLabel,2,0);
    y_max=new QLineEdit(tmpQGroupBox);
    y_max->setValidator(new KFloatValidator( y_max));
    grid2->addWidget(y_max,2,1);
    // PENDING(kalle) Put back in
    //     if( _chart->params()->requested_ymax != -DBL_MAX)
//         y_max->setText( tmp.setNum(_chart->params()->requested_ymax));

    grid1->addWidget(tmpQGroupBox,1,0);
    tmpQGroupBox->setEnabled(false);

    tmpQGroupBox = new QGroupBox( this, "GroupBox_3" );
    tmpQGroupBox->setFrameStyle( 49 );

    grid2 = new QGridLayout(tmpQGroupBox,3,2,15,7);

    tmpLabel=new QLabel(tmpQGroupBox);
    tmpLabel->setText(i18n("YLabel format: "));
    grid2->addWidget(tmpLabel,0,0);
    ylabel_fmt=new QLineEdit(tmpQGroupBox);
    grid2->addWidget(ylabel_fmt,0,1);

    // PENDING(kalle) Put back in
    //     if( !_chart->params()->ylabel_fmt.isEmpty()) {
//         int len=_chart->params()->ylabel_fmt.length();
//         ylabel_fmt->setText(_chart->params()->ylabel_fmt.right(len-3));
//     }
    ylabelFont = new QPushButton( tmpQGroupBox);
    grid2->addWidget(ylabelFont,1,0);
    ylabelFont->setText(i18n("Font"));
    // PENDING(kalle) Put back in
    //     ylabel=_chart->params()->yAxisFont();

    ylabelColor=new KColorButton(tmpQGroupBox);
    grid2->addWidget(ylabelColor,1,1);
    // PENDING(kalle) Put back in
    //     ycolor=_chart->params()->YLabelColor;
    ylabelColor->setColor( ycolor );


    tmpLabel=new QLabel(tmpQGroupBox);
    tmpLabel->setText(i18n("YLabel2 format: "));
    grid2->addWidget(tmpLabel,2,0);

    ylabel2_fmt=new QLineEdit(tmpQGroupBox);
    grid2->addWidget(ylabel2_fmt,2,1);
    // PENDING(kalle) Put back in
    //     if( !_chart->params()->ylabel2_fmt.isEmpty()) {
//         int len=_chart->params()->ylabel2_fmt.length();
//         ylabel2_fmt->setText(_chart->params()->ylabel2_fmt.right(len-3));
//     }
    tmpQGroupBox->setEnabled(false);
    grid1->addWidget(tmpQGroupBox,1,1);



    connect(ylabelFont,SIGNAL(clicked()),this,SLOT(changeLabelFont()));


    connect(ylabelColor,SIGNAL(changed( const QColor & )),
            this,SLOT(changeLabelColor(const QColor &)));
    connect(borderColor,SIGNAL(changed( const QColor & )),
            this,SLOT(changeBorderColor(const QColor &)));
    connect(gridColor,SIGNAL(changed( const QColor & )),
            this,SLOT(changeGridColor(const QColor &)));
}


KChartWizardSetupAxesPage::~KChartWizardSetupAxesPage()
{
    // _chart->removeAutoUpdate( preview );
}

void KChartWizardSetupAxesPage::changeLabelColor(const QColor &_color)
{
    ycolor=_color;
}

void KChartWizardSetupAxesPage::changeBorderColor(const QColor &_color)
{
    colorBorder=_color;
}

void KChartWizardSetupAxesPage::changeGridColor(const QColor &_color)
{
    colorGrid=_color;
}

void KChartWizardSetupAxesPage::changeLabelFont()
{
    if( KFontDialog::getFont( ylabel,false,this ) == QDialog::Rejected )
        return;
}

void KChartWizardSetupAxesPage::paintEvent( QPaintEvent *)
{
    if(chart3d) {
        angle->setEnabled(true);
        depth->setEnabled(true);
        //barWidth->setEnabled(true);
    } else {
        angle->setEnabled(false);
        depth->setEnabled(false);
        //barWidth->setEnabled(false);
    }
}

void KChartWizardSetupAxesPage::apply()
{
    _chart->params()->setAxisShowGrid(KDChartAxisParams::AxisPosLeft,grid->isChecked() );
    _chart->params()->setAxisShowGrid(KDChartAxisParams::AxisPosBottom,grid->isChecked() );

    KDChartAxisParams leftparams( _chart->params()->axisParams( KDChartAxisParams::AxisPosLeft ) );
/*    if( !y_max->text().isEmpty())
    {
        leftparams.setAxisAreaMode(KDChartAxisParams::AxisAreaModeMinMaxSize);
        leftparams.setAxisAreaMax(y_max->text().toDouble());
    }
    else
    leftparams.setAxisAreaMode(KDChartAxisParams::AxisAreaModeAutoSize);*/
    leftparams.setAxisCalcMode(KDChartAxisParams::AxisCalcLogarithmic);
    _chart->params()->setAxisParams( KDChartAxisParams::AxisPosLeft, leftparams );
    // PENDING(kalle) Put back in
    //     _chart->params()->grid =grid->isChecked() ;
//     if( !y_interval->text().isEmpty())
//         _chart->params()->requested_yinterval=y_interval->text().toDouble();
//     else
//         _chart->params()->requested_yinterval=0;
//     if( !y_max->text().isEmpty())
//         _chart->params()->requested_ymax=y_max->text().toDouble();
//     else
//         _chart->params()->requested_ymax=0;
//     if( !y_min->text().isEmpty())
//         _chart->params()->requested_ymin=y_min->text().toDouble();
//     else
//         _chart->params()->requested_ymin=0;

    // PENDING(kalle) Put back in
//     _chart->params()->border =border->isChecked() ;

    _chart->params()->setThreeDBarAngle( angle->value() );
    // PENDING(kalle) Put back in
    //     if(! ylabel_fmt->text().isEmpty()) {
//         QString tmp="%g "+ylabel_fmt->text();
//         _chart->params()->ylabel_fmt=tmp;
//     } else {
//         _chart->params()->ylabel_fmt="";
//     }
//     _chart->params()->setYAxisFont(ylabel);
//     _chart->params()->YLabelColor=ycolor;
//     _chart->params()->GridColor=colorGrid;
//     _chart->params()->LineColor=colorBorder;
    _chart->params()->setThreeDBarDepth( depth->value() );
    // PENDING(kalle) Put back in
    //     if(! ylabel2_fmt->text().isEmpty()) {
//         QString tmp="%g "+ylabel2_fmt->text();
//         _chart->params()->ylabel2_fmt=tmp;
//     } else {
//         _chart->params()->ylabel2_fmt="";
//     }
}



#include "kchartWizardSetupAxesPage.moc"
