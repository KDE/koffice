/*!
 * Sets initial default values for the parameters. The user can
 * override all of these.
 */
#include "kchartparams.h"
#include "kglobal.h"

KChartParameters::KChartParameters() :
	BGColor( 0, 0, 0 ),
	GridColor( 160, 160, 160 ),
	LineColor( 0, 0, 0 ),
	PlotColor( 0, 0, 0 ),
	VolColor( 160, 160, 255 ),
	TitleColor( 0, 0, 0 ),
	XTitleColor( 0, 0, 0 ),
	YTitleColor( 0, 0, 0 ),
	YTitle2Color( 0, 0, 0 ),
	XLabelColor( 0, 0, 0 ),
	YLabelColor( 0, 0, 0 ),
	YLabel2Color( 0, 0, 0 ),
	EdgeColor() // default: no color
{
	ExtColor.setColor( 0, Qt::red );
	ExtColor.setColor( 1, Qt::green );
	ExtColor.setColor( 2, Qt::blue );
	ExtColor.setColor( 3, Qt::cyan );
	ExtColor.setColor( 4, Qt::magenta );
	ExtColor.setColor( 5, Qt::yellow );
	ExtColor.setColor( 6, Qt::darkRed );
	ExtColor.setColor( 7, Qt::darkGreen );
	ExtColor.setColor( 8, Qt::darkBlue );
	ExtColor.setColor( 9, Qt::darkCyan );
	ExtColor.setColor( 10, Qt::darkMagenta );
	ExtColor.setColor( 11, Qt::darkYellow );
	
	setTitleFont( QFont( "Helvetica", 12 ) );
	setYTitleFont( QFont( "Helvetica", 12 ) );
	setXTitleFont( QFont( "Helvetica", 12 ) );
	setYAxisFont( QFont( "Helvetica", 10 ) );
	setXAxisFont( QFont( "Helvetica", 10 ) );
	setLabelFont( QFont( "Helvetica", 10 ) );
	setAnnotationFont( QFont( "Helvetica", 10 ) );

	label_dist = 1+8/2;
	label_line = false;
	xlabel_spacing = 5;
	ylabel_density = 80;
	requested_ymin = MAXDOUBLE;
	//	requested_ymax = -MAXDOUBLE;
	requested_ymax = -MAXDOUBLE;
	requested_yinterval = -MAXDOUBLE;
	shelf = true;
	grid = true;
	xaxis = true;
	yaxis = true;
	yaxis2 = true;
	yval_style = true;
	stack_type = KCHARTSTACKTYPE_DEPTH;
	_3d_depth = 5.0;
	_3d_angle = 45;
	bar_width = 75;
	hlc_style = KCHARTHLCSTYLE_CLOSECONNECTED;
	hlc_cap_width = 25;
	annotation = 0;
	num_scatter_pts = 0;
	scatter = 0;
	thumbnail = false;
	thumbval = -MAXFLOAT;
	border = false;
	transparent_bg = false;
	percent_labels = KCHARTPCTTYPE_NONE;
	type = KCHARTTYPE_3DBAR;	
	hard_size = false;
	hard_graphheight = 0;
	hard_graphwidth = 0;
	hard_xorig = 0;
	hard_yorig = 0;
}

bool KChartParameters::do_vol() {
  bool val = ( type == KCHARTTYPE_COMBO_HLC_BAR   ||		// aka: combo
	     type == KCHARTTYPE_COMBO_HLC_AREA  ||
	     type == KCHARTTYPE_COMBO_LINE_BAR  ||
	     type == KCHARTTYPE_COMBO_LINE_AREA ||
	     type == KCHARTTYPE_3DCOMBO_HLC_BAR ||
	     type == KCHARTTYPE_3DCOMBO_HLC_AREA||
	     type == KCHARTTYPE_3DCOMBO_LINE_BAR||
	     type == KCHARTTYPE_3DCOMBO_LINE_AREA );
  return val;
}

bool KChartParameters::threeD() {
  bool val = ( type == KCHARTTYPE_3DAREA          ||
	       type == KCHARTTYPE_3DLINE          ||
	       type == KCHARTTYPE_3DBAR           ||
	       type == KCHARTTYPE_3DHILOCLOSE     ||
	       type == KCHARTTYPE_3DCOMBO_HLC_BAR ||
	       type == KCHARTTYPE_3DCOMBO_HLC_AREA||
	       type == KCHARTTYPE_3DCOMBO_LINE_BAR||
	       type == KCHARTTYPE_3DCOMBO_LINE_AREA );
  return val;
}

bool KChartParameters::has_hlc_sets() {
  bool val =
		( type == KCHARTTYPE_COMBO_HLC_BAR   ||
		  type == KCHARTTYPE_COMBO_HLC_AREA  ||
		  type == KCHARTTYPE_3DCOMBO_HLC_BAR ||
		  type == KCHARTTYPE_3DCOMBO_HLC_AREA||
		  type == KCHARTTYPE_3DHILOCLOSE     ||
		  type == KCHARTTYPE_HILOCLOSE );
 return val;
}

bool KChartParameters::do_bar() {
  bool val = ( type == KCHARTTYPE_3DBAR || // offset X objects to leave
	       type == KCHARTTYPE_BAR );  //  room at X(0) and X(n)
  return val;
}



void KChartParameters::saveColorArray(KConfig *conf, 
				      KChartColorArray *arr,
				      QString name) {
  QString confname;
  confname.sprintf("%s_count",name.ascii());
  conf->writeEntry(confname, arr->count());
  for(unsigned int i=0; i!=arr->count(); i++) {
    confname.sprintf("%s_%d", name.ascii(), i);
    conf->writeEntry(confname, arr->color(i));
  }
}

void KChartParameters::loadColorArray(KConfig *conf, 
				      KChartColorArray *arr,
				      QString name) {
  QString confname;
  QColor defcolor(255,0,0);
  confname.sprintf("%s_count",name.ascii());
  int count = conf->readNumEntry(confname, 0);
  for(int i=0; i!=count; i++) {
    confname.sprintf("%s_%d", name.ascii(), i);
    arr->setColor(i, conf->readColorEntry(confname, &defcolor));
  }
}




void KChartParameters::saveConfig(KConfig *conf) {
  conf->setGroup("ChartParameters");

  conf->writeEntry("title", title);
  conf->writeEntry("xtitle", xtitle);
  conf->writeEntry("ytitle", ytitle);
  conf->writeEntry("ytitle2", ytitle2);
  // the fonts
  conf->writeEntry("titlefont", titlefont);
  conf->writeEntry("ytitlefont", ytitlefont);
  conf->writeEntry("xtitlefont", xtitlefont);
  conf->writeEntry("yaxisfont", yaxisfont);
  conf->writeEntry("xaxisfont", xaxisfont);
  conf->writeEntry("labelfont", labelfont);

  conf->writeEntry("ylabel_fmt", ylabel_fmt);
  conf->writeEntry("ylabel2_fmt", ylabel2_fmt);
  conf->writeEntry("xlabel_spacing", xlabel_spacing);
  conf->writeEntry("ylabel_density", ylabel_density);
  conf->writeEntry("requested_ymin", requested_ymin);
  conf->writeEntry("requested_ymax", requested_ymax);
  conf->writeEntry("requested_yinterval", requested_yinterval);

  conf->writeEntry("shelf", shelf);
  conf->writeEntry("grid", grid );
  conf->writeEntry("xaxis", xaxis);
  conf->writeEntry("yaxis", yaxis);
  conf->writeEntry("yaxis2", yaxis2);
  conf->writeEntry("yval_style", yval_style );
  conf->writeEntry("stack_type", stack_type);
  conf->writeEntry("_3d_depth", _3d_depth);
  conf->writeEntry("_3d_angle", _3d_angle);
  conf->writeEntry("bar_width", bar_width);
  conf->writeEntry("hlc_style", hlc_style);
  conf->writeEntry("hlc_cap_width", hlc_cap_width );
  // TODO: Annotation type!!!
  conf->writeEntry("annotationfont", annotationfont);
  conf->writeEntry("num_scatter_pts", num_scatter_pts);
  // TODO: Scatter type!!!
  conf->writeEntry("thumbnail", thumbnail);
  conf->writeEntry("thumblabel", thumblabel);
  conf->writeEntry("thumbval", thumbval);
  conf->writeEntry("border", border);
  conf->writeEntry("BGColor", BGColor);
  conf->writeEntry("GridColor", GridColor);
  conf->writeEntry("LineColor", LineColor);
  conf->writeEntry("PlotColor", PlotColor);
  conf->writeEntry("VolColor", VolColor);
  conf->writeEntry("TitleColor", TitleColor);
  conf->writeEntry("XTitleColor", XTitleColor);
  conf->writeEntry("YTitleColor", YTitleColor );
  conf->writeEntry("XLabelColor", XLabelColor );
  conf->writeEntry("YLabelColor", YLabelColor);
  conf->writeEntry("YLabel2Color", YLabel2Color);
  conf->writeEntry("EdgeColor", EdgeColor);
  saveColorArray(conf, &SetColor, "SetColor");
  saveColorArray(conf, &ExtColor, "ExtColor");
  saveColorArray(conf, &ExtVolColor, "ExtVolColor");


  conf->writeEntry("transparent_bg", transparent_bg);
  conf->writeEntry("BGImage", BGImage);
  // TODO: explode, missing
  conf->writeEntry("percent_labels", percent_labels );
  conf->writeEntry("label_dist", label_dist);
  conf->writeEntry("label_line", label_line);
  conf->writeEntry("type", type );
  conf->writeEntry("other_threshold", other_threshold);
}

void KChartParameters::loadConfig(KConfig *conf) {
  conf->setGroup("ChartParameters");

  title = conf->readEntry("title", title);
  xtitle = conf->readEntry("xtitle", xtitle);
  ytitle = conf->readEntry("ytitle", ytitle );
  ytitle2 = conf->readEntry("ytitle2", ytitle2);
  // TODO: the fonts
  QFont tempfont;
  tempfont = conf->readFontEntry("titlefont", &titlefont);
  setTitleFont(tempfont);
  tempfont = conf->readFontEntry("ytitlefont", &ytitlefont);
  setYTitleFont(tempfont);
  tempfont = conf->readFontEntry("xtitlefont", &xtitlefont);
  setXTitleFont(tempfont);
  tempfont = conf->readFontEntry("yaxisfont", &yaxisfont);
  setYAxisFont(tempfont);
  tempfont = conf->readFontEntry("xaxisfont", &xaxisfont);
  setXAxisFont(tempfont);
  tempfont = conf->readFontEntry("labelfont", &labelfont);
  setLabelFont(tempfont);
  tempfont = conf->readFontEntry("annotationfont", &annotationfont);
  setAnnotationFont(tempfont);



  ylabel_fmt = conf->readEntry("ylabel_fmt", ylabel_fmt );
  ylabel2_fmt = conf->readEntry("ylabel2_fmt", ylabel2_fmt);
  xlabel_spacing = conf->readNumEntry("xlabel_spacing");
  ylabel_density = conf->readNumEntry("ylabel_density", ylabel_density);
  requested_ymin = conf->readNumEntry("requested_ymin", requested_ymin);
  requested_ymax = conf->readNumEntry("requested_ymax", requested_ymax );
  requested_yinterval = conf->readNumEntry("requested_yinterval",
					requested_yinterval);
  shelf = conf->readBoolEntry("shelf", shelf);
  grid = conf->readBoolEntry("grid", grid);
  xaxis = conf->readBoolEntry("xaxis", xaxis);
  yaxis = conf->readBoolEntry("yaxis", yaxis);
  yaxis2 = conf->readBoolEntry("yaxis2", yaxis);
  yval_style = conf->readNumEntry("yval_style", yval_style);
  stack_type = (KChartStackType)conf->readNumEntry("stack_type", stack_type);
  _3d_depth = conf->readDoubleNumEntry("_3d_depth", _3d_depth );
  _3d_angle = conf->readNumEntry("_3d_angle", _3d_angle);
  bar_width = conf->readNumEntry("bar_width", bar_width);
  hlc_style = (KChartHLCStyle)conf->readNumEntry("hlc_style", hlc_style);
  hlc_cap_width = conf->readNumEntry("hlc_cap_width", hlc_cap_width);
  // TODO: Annotation font
  num_scatter_pts = conf->readNumEntry("num_scatter_pts", num_scatter_pts);
  // TODO: Scatter type
  thumbnail = conf->readBoolEntry("thumbnail", thumbnail);
  thumblabel = conf->readEntry("thumblabel", thumblabel);
  border = conf->readBoolEntry("border", border);
  BGColor = conf->readColorEntry("BGColor", &BGColor);
  GridColor = conf->readColorEntry("GridColor", &GridColor);
  LineColor = conf->readColorEntry("LineColor", &LineColor);
  PlotColor = conf->readColorEntry("PlotColor", &PlotColor);
  VolColor = conf->readColorEntry("VolColor", &VolColor);
  TitleColor = conf->readColorEntry("TitleColor", &TitleColor);
  XTitleColor = conf->readColorEntry("XTitleColor", &XTitleColor);
  YTitleColor = conf->readColorEntry("YTitleColor", &YTitleColor);
  YTitle2Color = conf->readColorEntry("YTitle2Color", &YTitle2Color);
  XLabelColor = conf->readColorEntry("XLabelColor", &XLabelColor);
  YLabelColor = conf->readColorEntry("YLabelColor", &YLabelColor);
  YLabel2Color = conf->readColorEntry("YLabel2Color", &YLabel2Color);
  EdgeColor = conf->readColorEntry("EdgeColor", &EdgeColor);
  loadColorArray(conf, &SetColor, "SetColor");
  loadColorArray(conf, &ExtColor, "ExtColor");
  loadColorArray(conf, &ExtVolColor, "ExtVolColor");
  transparent_bg = conf->readBoolEntry("transparent_bg", transparent_bg);
  BGImage = conf->readEntry("BGImage", BGImage);
  // TODO: explode, missing
  percent_labels = (KChartPercentType)conf->readNumEntry("percent_labels",
							 percent_labels);
  label_dist = conf->readNumEntry("label_dist", label_dist);
  label_line = conf->readBoolEntry("label_line", label_line);
  type = (KChartType)conf->readNumEntry("type", type);
  other_threshold = conf->readNumEntry("other_threshold", other_threshold);
}

