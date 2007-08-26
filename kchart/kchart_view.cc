/**
 *
 * Kalle Dalheimer <kalle@kde.org>
 */

#include <kicon.h>
#include <QFile>
#include <qpainter.h>
#include <qcursor.h>
#include <QMenu>
//Added by qt3to4:
#include <QMouseEvent>
#include <QPixmap>
#include <QPaintEvent>

#include <kaction.h>
#include <kglobal.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kdebug.h>
#include <kprinter.h>
#include <kstandarddirs.h>
#include <ktemporaryfile.h>
#include <kcomponentdata.h>
#include <kxmlguifactory.h>
#include <kfiledialog.h>
#include <kmessagebox.h>
#include <ktoggleaction.h>
#include <kactioncollection.h>

#include <KoCsvImportDialog.h>
#include <KoTemplateCreateDia.h>
#include <KoViewAdaptor.h>

//#include "KDChart.h"

#include "kchart_view.h"
#include "kchart_factory.h"
#include "kchart_part.h"
#if 0
#include "kchart_params.h"
#endif
#include "KChartViewAdaptor.h"

#include "KCConfigDialog.h"
#include "KCPageLayout.h"
#include "KCPrinterDialog.h"

#include "commands/ChartTypeCommand.h"

using namespace std;


//#include "sheetdlg.h"

namespace KChart
{


KChartView::KChartView( KChartPart* part, QWidget* parent )
    : KoView( part, parent )
{
    setComponentData( KChartFactory::global() );
    if ( koDocument()->isReadWrite() )
        setXMLFile( "kchart.rc" );
    else
        setXMLFile( "kchart_readonly.rc" );

    m_dbus = new ViewAdaptor(this);

    m_importData  = new KAction(i18n("Import Data..."), this);
    actionCollection()->addAction("import_data", m_importData );
    connect(m_importData, SIGNAL(triggered(bool)), SLOT( importData() ));
    KAction *actionExtraCreateTemplate  = new KAction(i18n("&Create Template From Document..."), this);
    actionCollection()->addAction("extra_template", actionExtraCreateTemplate );
    connect(actionExtraCreateTemplate, SIGNAL(triggered(bool)), SLOT( extraCreateTemplate() ));

    m_edit  = new KAction(KIcon("edit"), i18n("Edit &Data..."), this);
    actionCollection()->addAction("editdata", m_edit );
    connect(m_edit, SIGNAL(triggered(bool) ), SLOT( editData() ));
    m_config  = new KAction(KIcon("configure"), i18n("&Chart..."), this);
    actionCollection()->addAction("config", m_config );
    connect(m_config, SIGNAL(triggered(bool) ), SLOT( slotConfig() ));

    // One KToggleAction per chart type
    m_chartbars  = new KToggleAction(KIcon("chart_bar_3d"), i18n("&Bar"), this);
    actionCollection()->addAction("barschart", m_chartbars );
    connect(m_chartbars, SIGNAL(triggered(bool)), SLOT( barsChart() ));
    QActionGroup *charttypes = new QActionGroup( this );
	charttypes->addAction(m_chartbars);

    m_chartline  = new KToggleAction(KIcon("chart_line"), i18n("&Line"), this);
    actionCollection()->addAction("linechart", m_chartline );
    connect(m_chartline, SIGNAL(triggered(bool)), SLOT( lineChart() ));
	charttypes->addAction(m_chartline);

    m_chartareas  = new KToggleAction(KIcon("chart_area"), i18n("&Area"), this);
    actionCollection()->addAction("areaschart", m_chartareas );
    connect(m_chartareas, SIGNAL(triggered(bool)), SLOT( areasChart() ));
	charttypes->addAction(m_chartareas);
    m_chartareas->setEnabled(false); // TODO not supported yet

    m_charthilo  = new KToggleAction(KIcon("chart_hilo"), i18n("&HiLo"), this);
    actionCollection()->addAction("hilochart", m_charthilo );
    connect(m_charthilo, SIGNAL(triggered(bool)), SLOT( hiLoChart() ));
	charttypes->addAction(m_charthilo);
    m_charthilo->setEnabled(false); // TODO not supported yet

    m_chartbw  = new KToggleAction(KIcon("chart_boxwhisker"), i18n("Bo&x && Whiskers"), this);
    actionCollection()->addAction("bwchart", m_chartbw );
 connect(m_chartbw, SIGNAL(triggered(bool)), SLOT( bwChart() ));
	charttypes->addAction(m_chartbw);
    m_chartbw->setEnabled(false); // TODO not supported yet

    m_chartpie  = new KToggleAction(KIcon("chart_pie"), i18n("&Pie"), this);
    actionCollection()->addAction("piechart", m_chartpie );
 connect(m_chartpie, SIGNAL(triggered(bool)), SLOT( pieChart() ));
    charttypes->addAction(m_chartpie);
    m_chartring  = new KToggleAction(KIcon("chart_ring"), i18n("&Ring"), this);
    actionCollection()->addAction("ringchart", m_chartring );
 connect(m_chartring, SIGNAL(triggered(bool)), SLOT( ringChart() ));
	charttypes->addAction(m_chartring);
    m_chartpolar  = new KToggleAction(KIcon("chart_polar"), i18n("&Polar"), this);
    actionCollection()->addAction("polarchart", m_chartpolar );
 connect(m_chartpolar, SIGNAL(triggered(bool)), SLOT( polarChart() ));
	charttypes->addAction(m_chartpolar);
    // Configuration KActions
    m_colorConfig  = new KAction(i18n("&Colors..."), this);
    actionCollection()->addAction("color_config", m_colorConfig );
    connect(m_colorConfig, SIGNAL(triggered(bool)), SLOT( slotConfigColor() ));
    m_fontConfig  = new KAction(i18n("&Font..."), this);
    actionCollection()->addAction("font_config", m_fontConfig );
 connect(m_fontConfig, SIGNAL(triggered(bool)), SLOT( slotConfigFont() ));
    m_backConfig  = new KAction(i18n("&Background..."), this);
    actionCollection()->addAction("back_config", m_backConfig );
 connect(m_backConfig, SIGNAL(triggered(bool)), SLOT( slotConfigBack() ));
    m_legendConfig  = new KAction(i18n("&Legend..."), this);
    actionCollection()->addAction("legend_config", m_legendConfig );
 connect(m_legendConfig, SIGNAL(triggered(bool)), SLOT( slotConfigLegend() ));
    m_subTypeChartConfig  = new KAction(i18n("Chart &Sub-type..."), this);
    actionCollection()->addAction("legend_subtype", m_subTypeChartConfig );
 connect(m_subTypeChartConfig, SIGNAL(triggered(bool)), SLOT( slotConfigSubTypeChart() ));
    m_dataFormatConfig  = new KAction(i18n("&Data Format..."), this);
    actionCollection()->addAction("data_format", m_dataFormatConfig );
 connect(m_dataFormatConfig, SIGNAL(triggered(bool)), SLOT( slotConfigDataFormat() ));
    m_headerFooterConfig  = new KAction(i18n("&Header && Footer..."), this);
    actionCollection()->addAction("headerfooter_subtype", m_headerFooterConfig );
 connect(m_headerFooterConfig, SIGNAL(triggered(bool)), SLOT( slotConfigHeaderFooterChart() ));
    m_pageLayoutConfig  = new KAction(i18n("Page Layout..."), this);
    actionCollection()->addAction("page_layout", m_pageLayoutConfig );
 connect(m_pageLayoutConfig, SIGNAL(triggered(bool)), SLOT( slotConfigPageLayout() ));

    // initialize the configuration
    //    loadConfig();

    // Disable some things if we can't change the data, e.g. because
    // we are inside another application that provides the data for us.
    if (!((KChartPart*)koDocument())->canChangeValue()) {
	m_edit->setEnabled(false);
	m_importData->setEnabled(false);
    }

    updateGuiTypeOfChart();
}


KChartView::~KChartView()
{
//     delete m_dcop;
}


ViewAdaptor* KChartView::dbusObject()
{
    return m_dbus;
}


void KChartView::paintEvent( QPaintEvent* /*ev*/ )
{
    QPainter painter( this );

    // ### TODO: Scaling

    // Let the document do the drawing
    // This calls KChartPart::paintContent, basically.
    koDocument()->paintEverything( painter, rect(), this );
}


void KChartView::updateReadWrite( bool /*readwrite*/ )
{
#ifdef __GNUC__
#warning TODO
#endif
}



void KChartView::updateGuiTypeOfChart()
{
#if 0
    KDChartParams* params = ((KChartPart*)koDocument())->params();
#else
    KChartPart *part = qobject_cast<KChartPart*>( koDocument() );
#endif

    switch(part->chartType()) {
    case BarChartType:
	m_chartbars->setChecked(true);
	break;
    case LineChartType:
	m_chartline->setChecked(true);
	break;
    case AreaChartType:
	m_chartareas->setChecked(true);
	break;
    case PieChartType:
	m_chartpie->setChecked(true);
	break;
    case HiLoChartType:
	m_charthilo->setChecked(true);
	break;
    case RingChartType:
	m_chartring->setChecked(true);
	break;
    case PolarChartType:
        m_chartpolar->setChecked(true);
        break;
    case BoxWhiskerChartType:
        m_chartbw->setChecked( true );
        break;
    default:
	//todo
	break;
    }

    // Disable subtype configuration button if appropriate.
    updateButton();
}


void KChartView::slotConfig()
{
#if 0
    config(KCConfigDialog::KC_ALL);
#endif
}


void KChartView::config(int flags)
{
    Q_UNUSED( flags );
#if 0
    // open a config dialog depending on the chart type
    KChartParams      *params = ((KChartPart*)koDocument())->params();
    KDChartTableData  *dat    = ((KChartPart*)koDocument())->data();

    KCConfigDialog    *d      = new KCConfigDialog( params, this, flags, dat );

    connect( d, SIGNAL( dataChanged() ),
             this, SLOT( slotRepaint() ) );
    d->exec();
    delete d;
#endif
}


void KChartView::slotRepaint()
{
    ((KChartPart*)koDocument())->setModified(true);
    update();
}


void KChartView::saveConfig()
{
    kDebug(35001) <<"Save config...";
    ((KChartPart*)koDocument())->saveConfig( KGlobal::config().data() );
}


void KChartView::loadConfig()
{
    kDebug(35001) <<"Load config...";

    KGlobal::config()->reparseConfiguration();
    ((KChartPart*)koDocument())->loadConfig( KGlobal::config().data() );

    updateGuiTypeOfChart();
    //refresh chart when you load config
    update();
}


void KChartView::defaultConfig()
{
    ((KChartPart*)koDocument())->defaultConfig(  );
    updateGuiTypeOfChart();
    update();
}


void KChartView::pieChart()
{
#if 0
    if ( m_chartpie->isChecked() ) {
	forceAxisParams(false);
	KChartParams  *params = ((KChartPart*)koDocument())->params();

	params->setChartType( KChartParams::Pie );
	params->setThreeDPies(params->threeDBars());
	params->setExplodeFactor( 0 );
	params->setExplode( true );

	updateButton();
	update();
	((KChartPart*)koDocument())->setModified(true);
    }
    else
        m_chartpie->setChecked( true ); // always one has to be checked !
#else
    ChartTypeCommand* command = new ChartTypeCommand(qobject_cast<KChartPart*>(koDocument())->chart());
    command->setChartType(PieChartType);
    koDocument()->addCommand(command);
    update();
#endif
}

void KChartView::forceAxisParams(bool lineMode)
{
    Q_UNUSED( lineMode );
#if 0
    KChartParams  *params = ((KChartPart*)koDocument())->params();
    KDChartAxisParams  axisParams;
    axisParams = params->axisParams( KDChartAxisParams::AxisPosLeft );
    if(params->chartType() == KChartParams::Line)
        m_logarithmicScale = axisParams.axisCalcMode();
    if(lineMode) {
        if(m_logarithmicScale)
            axisParams.setAxisCalcMode(KDChartAxisParams::AxisCalcLogarithmic);
    } else
        axisParams.setAxisCalcMode(KDChartAxisParams::AxisCalcLinear);
    params->setAxisParams( KDChartAxisParams::AxisPosLeft, axisParams );
#endif
}

void KChartView::lineChart()
{
#if 0
    if ( m_chartline->isChecked() ) {
	forceAxisParams(true);
	KChartParams* params = ((KChartPart*)koDocument())->params();

	params->setChartType( KChartParams::Line );
	params->setLineChartSubType( KDChartParams::LineNormal );

	updateButton();
	update();
	((KChartPart*)koDocument())->setModified(true);
    }
    else
	m_chartline->setChecked( true ); // always one has to be checked !
#else
    ChartTypeCommand* command = new ChartTypeCommand(qobject_cast<KChartPart*>(koDocument())->chart());
    command->setChartType(LineChartType);
    koDocument()->addCommand(command);
    update();
#endif
}


void KChartView::barsChart()
{
#if 0
    if ( m_chartbars->isChecked() ) {
	forceAxisParams(false);
	KChartParams* params = ((KChartPart*)koDocument())->params();

	params->setChartType( KChartParams::Bar );
	params->setBarChartSubType( KDChartParams::BarNormal );

	updateButton();
    params->setThreeDBars( params->threeDPies() );
	update();
	((KChartPart*)koDocument())->setModified(true);
    }
    else
	m_chartbars->setChecked( true ); // always one has to be checked !
#else
    ChartTypeCommand* command = new ChartTypeCommand(qobject_cast<KChartPart*>(koDocument())->chart());
    command->setChartType(BarChartType);
    koDocument()->addCommand(command);
    update();
#endif
}


void KChartView::areasChart()
{
#if 0
    if ( m_chartareas->isChecked() ) {
	forceAxisParams(false);
	KChartParams* params = ((KChartPart*)koDocument())->params();

	params->setChartType( KChartParams::Area );
	params->setAreaChartSubType( KDChartParams::AreaNormal );

	updateButton();
	update();
	((KChartPart*)koDocument())->setModified(true);
    }
    else
	m_chartareas->setChecked( true ); // always one has to be checked !
#else
    ChartTypeCommand* command = new ChartTypeCommand(qobject_cast<KChartPart*>(koDocument())->chart());
    command->setChartType(AreaChartType);
    koDocument()->addCommand(command);
    update();
#endif
}


void KChartView::hiLoChart()
{
#if 0
    if ( m_charthilo->isChecked() ) {
	forceAxisParams(false);
	KChartParams* params = ((KChartPart*)koDocument())->params();

	params->setChartType( KChartParams::HiLo );
	params->setHiLoChartSubType( KDChartParams::HiLoNormal );

	updateButton();
	update();
	((KChartPart*)koDocument())->setModified(true);
    }
    else
	m_charthilo->setChecked( true ); // always one has to be checked !
#else
    ChartTypeCommand* command = new ChartTypeCommand(qobject_cast<KChartPart*>(koDocument())->chart());
    command->setChartType(HiLoChartType);
    koDocument()->addCommand(command);
    update();
#endif
}


void KChartView::ringChart()
{
#if 0
    if ( m_chartring->isChecked() ) {
	forceAxisParams(false);
	KChartParams* params = ((KChartPart*)koDocument())->params();

	params->setChartType( KChartParams::Ring );

	updateButton();
	update();
	((KChartPart*)koDocument())->setModified(true);
    }
    else
	m_chartring->setChecked( true ); // always one has to be checked !
#else
    ChartTypeCommand* command = new ChartTypeCommand(qobject_cast<KChartPart*>(koDocument())->chart());
    command->setChartType(RingChartType);
    koDocument()->addCommand(command);
    update();
#endif
}


void KChartView::polarChart()
{
#if 0
    if ( m_chartpolar->isChecked() ) {
	forceAxisParams(false);
        KDChartParams* params = ((KChartPart*)koDocument())->params();

        params->setChartType( KDChartParams::Polar );
        params->setPolarChartSubType( KDChartParams::PolarNormal );

        update();
	((KChartPart*)koDocument())->setModified(true);
    }
    else
        m_chartpolar->setChecked( true ); // always one has to be checked !
#else
    ChartTypeCommand* command = new ChartTypeCommand(qobject_cast<KChartPart*>(koDocument())->chart());
    command->setChartType(PolarChartType);
    koDocument()->addCommand(command);
    update();
#endif
}


void KChartView::bwChart()
{
#if 0
    if ( m_chartbw->isChecked() ) {
	forceAxisParams(false);
        KDChartParams* params = ((KChartPart*)koDocument())->params();

        params->setChartType( KDChartParams::BoxWhisker );
        params->setBWChartSubType( KDChartParams::BWNormal );

        update();
	((KChartPart*)koDocument())->setModified(true);
    }
    else
        m_chartbw->setChecked( true ); // always one has to be checked !
#else
    ChartTypeCommand* command = new ChartTypeCommand(qobject_cast<KChartPart*>(koDocument())->chart());
    command->setChartType(BoxWhiskerChartType);
    koDocument()->addCommand(command);
    update();
#endif
}


void KChartView::mousePressEvent ( QMouseEvent *e )
{
    if (!koDocument()->isReadWrite() || !factory())
        return;
    if ( e->button() == Qt::RightButton )
        ((QMenu*)factory()->container("action_popup",this))->popup(QCursor::pos());
}


void KChartView::slotConfigColor()
{
    config(KCConfigDialog::KC_COLORS);
}


void KChartView::slotConfigFont()
{
    config(KCConfigDialog::KC_FONT);
}


void KChartView::slotConfigBack()
{
    config(KCConfigDialog::KC_BACK);
}


void KChartView::slotConfigLegend()
{
   config(KCConfigDialog::KC_LEGEND);
}

void KChartView::slotConfigDataFormat()
{
    config(KCConfigDialog::KC_DATAFORMAT);
}

void KChartView::slotConfigSubTypeChart()
{
    config(KCConfigDialog::KC_SUBTYPE);
}


void KChartView::slotConfigHeaderFooterChart()
{
    config(KCConfigDialog::KC_HEADERFOOTER);
}


// FIXME: Rename into something suitable.
void KChartView::updateButton()
{
    // Disable sub chart config item.
    KChartPart* part = qobject_cast<KChartPart*>(koDocument());

    bool state=(part->chartType() == BarChartType ||
                part->chartType() == AreaChartType ||
                part->chartType() == LineChartType ||
                part->chartType() == HiLoChartType ||
                part->chartType() == PolarChartType);
    m_subTypeChartConfig->setEnabled(state);
}


void KChartView::slotConfigPageLayout()
{
    KChartPart    *part = qobject_cast<KChartPart*>( koDocument() );
    KCPageLayout  *dialog = new KCPageLayout(part, this);

    connect( dialog, SIGNAL( dataChanged() ),
             this,   SLOT( slotRepaint() ) );

    dialog->exec();
    delete dialog;
}


void KChartView::setupPrinter( KPrinter &printer )
{
  if ( !printer.previewOnly() )
    printer.addDialogPage( new KCPrinterDialog( 0, "KChart page" ) );
}


void KChartView::print(KPrinter &printer)
{
    Q_UNUSED( printer );
#if 0                           // Disable printing for now.
    printer.setFullPage( false );

    QPainter painter;
    painter.begin(&printer);

    int  height;
    int  width;
    if ( !printer.previewOnly() ) {
	int const scalex = printer.option("kde-kchart-printsizex").toInt();
	int const scaley = printer.option("kde-kchart-printsizey").toInt();

	width  = printer.width()  * scalex / 100;
	height = printer.height() * scaley / 100;
    }
    else {
	// Fill the whole page.
	width  = printer.width();
	height = printer.height();
    }

    QRect  rect(0, 0, width, height);
    KDChart::print(&painter,
		   ((KChartPart*)koDocument())->params(),
		   ((KChartPart*)koDocument())->data(),
		   0, 		// regions
		   &rect);
    painter.end();
#endif
}


// Import data from a Comma Separated Values file.
//

void KChartView::importData()
{
    // Get the name of the file to open.
    QString filename = KFileDialog::getOpenFileName(KUrl(QString()),// startDir
						    QString(),// filter
						    0,
						    i18n("Import Data"));
    kDebug(35001) <<"Filename = <" << filename <<">";
    if (filename.isEmpty())
      return;

    // Check to see if we can read the file.
    QFile  inFile(filename);
    if (!inFile.open(QIODevice::ReadOnly)) {
	KMessageBox::sorry( 0, i18n("The file %1 could not be read.",
                                    filename) );
	inFile.close();
	return;
    }

    // Let the CSV dialog structure the data in the file.
    QByteArray  inData( inFile.readAll() );
    inFile.close();
    KoCsvImportDialog *dialog = new KoCsvImportDialog(0L);
    dialog->setData(inData);

    if ( !dialog->exec() ) {
	// kDebug(35001) <<"Cancel was pressed";
	return;
    }

    //kDebug(35001) <<"OK was pressed";

#if 0
    uint  rows = dialog->rows();
    uint  cols = dialog->cols();

    //kDebug(35001) <<"Rows:" << rows <<"  Cols:" << cols;

    bool  hasRowHeaders = ( rows > 1 && dialog->firstRowContainHeaders() );
    bool  hasColHeaders = ( cols > 1 && dialog->firstColContainHeaders() );

    KDChartTableData  data( rows, cols );
    data.setUsedRows( rows );
    data.setUsedCols( cols );
    for (uint row = 0; row < rows; row++) {
	for (uint col = 0; col < cols; col++) {
	    bool     ok;
	    QString  tmp;
	    double   val;

	    // Get the text and convert to double unless in the headers.
	    tmp = dialog->text( row, col );
	    if ( ( row == 0 && hasRowHeaders )
		 || ( col == 0 && hasColHeaders ) ) {
		kDebug(35001) <<"Setting header (" << row <<"," << col
			       << ") to value " << tmp << endl;
		data.setCell( row, col, tmp );
	    }
	    else {
		val = tmp.toDouble(&ok);
		if (!ok)
		    val = 0.0;

		kDebug(35001) <<"Setting (" << row <<"," << col
			       << ") to value " << val << endl;

		// and do the actual setting.
		data.setCell( row, col, val );
	    }
	}
    }

    ((KChartPart*)koDocument())->doSetData( data,
					    hasRowHeaders, hasColHeaders );
#else
    TableModel        data();
#endif
}


void KChartView::extraCreateTemplate()
{
    int width = 60;
    int height = 60;
    QPixmap pix = koDocument()->generatePreview(QSize(width, height));

    KTemporaryFile tempFile;
    tempFile.setSuffix(".chrt");
    tempFile.open();

    koDocument()->saveNativeFormat( tempFile.fileName() );

    KoTemplateCreateDia::createTemplate( "kchart_template", KChartFactory::global(),
                                         tempFile.fileName(), pix, this );

    KChartFactory::global().dirs()->addResourceType("kchart_template",
                                                    KStandardDirs::kde_default( "data" ) +
                                                    "kchart/templates/");
}


}  //KChart namespace

#include "kchart_view.moc"
