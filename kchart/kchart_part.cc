/**
 * $Id$
 *
 * Kalle Dalheimer <kalle@kde.org>
 */

#include <qdom.h>
#include <qtextstream.h>
#include <qbuffer.h>
#include "kchart_part.h"
#include "kchart_view.h"
#include "kchart_shell.h"
#include "kchart_factory.h"
#include <kstddirs.h>

#include <engine.h>
#include <kchartparams.h>
#include <kglobal.h>
//#include <iostream> // only for cerr?
#include <kdebug.h> // "ported" to kdDebug(35001)

using namespace std;

// Some hardcoded data for a chart

/* ----- set some data ----- */
// float   a[6]  = { 0.5, 0.09, 0.6, 0.85, 0.0, 0.90 },
// b[6]  = { 1.9, 1.3,  0.6, 0.75, 0.1, -2.0 };
/* ----- X labels ----- */
// char    *t[6] = { "Chicago", "New York", "L.A.", "Atlanta", "Paris, MD\n(USA) ", "London" };
/* ----- data set colors (RGB) ----- */
// QColor   sc[2]    = { QColor( 255, 128, 128 ), QColor( 128, 128, 255 ) };


#include <qpainter.h>

KChartPart::KChartPart( QWidget *parentWidget, const char *widgetName, QObject* parent, const char* name, bool singleViewMode )
  : KoDocument( parentWidget, widgetName, parent, name, singleViewMode ),
    _params( 0 )
{
  m_bLoading = false;
  kdDebug(35001) << "Contstructor started!" << endl;
  initDoc();
  // hack
  setModified(true);
}

KChartPart::~KChartPart()
{
  kdDebug(35001) << "Part is going to be destroyed now!!!" << endl;
  if (_params != NULL)
    delete _params;
}


bool KChartPart::initDoc()
{
  // Initialize the parameter set for this chart document
  // PENDING(kalle,torben) Where to delete this?
  kdDebug(35001) << "InitDOC" << endl;
  _params = new KChartParameters;
  initRandomData();
  // PENDING(lotzi) This is where to start the wizard and fill the
  // params struct with the data the users enters there.

  return TRUE;
}

void KChartPart::initRandomData()
{
     // fill cells
    int col,row;
    // initialize some data, if there is none
    if (currentData.rows() == 0) {
      kdDebug(35001) << "Initialize with some data!!!" << endl;
      currentData.expand(4,4);
      for (row = 0;row < 4;row++)
	for (col = 0;col < 4;col++) {
	  KChartValue t;
	  t.exists= true;
	  t.value = (double)row+col;
	  // kdDebug(35001) << "Set cell for " << row << "," << col << endl;
	  currentData.setCell(row,col,t);
	 }
    }

}



QCString KChartPart::mimeType() const
{
    return "application/x-kchart";
}

KoView* KChartPart::createView( QWidget* parent, const char* name )
{
    return new KChartView( this, parent, name );
}

KoMainWindow* KChartPart::createShell()
{
    KoMainWindow* shell = new KChartShell();
    shell->setRootDocument( this );
    shell->show();

    return shell;
}


void KChartPart::paintContent( QPainter& painter, const QRect& rect, bool transparent )
{
  if (isLoading()) {
    kdDebug(35001) << "Loading... Do not paint!!!..." << endl;
    return;
  }
  // if params is 0, initDoc() has not been called
  ASSERT( _params != 0 );

  // ####### handle transparency
  if( !transparent )
    painter.eraseRect( rect );

  // kdDebug(35001) << "KChartPart::paintContent called, rows = "
  //                << currentData.rows() << ", cols = "
  //                << currentData.cols() << endl;

  // Need to draw only the document rectangle described in the parameter rect.
  //  return;
  out_graph( rect.width(),
	     rect.height(), // short width, height
	     &painter,        // Paint into this painter
	     _params,	      // the parameters of the chart,
	     // including the type
	     currentData );

}

void KChartPart::setPart( const KChartData& data )
{
  currentData = data;
  initLabelAndLegend();
  emit docChanged();
}

void KChartPart::initLabelAndLegend()
{

    if(_params->legend.isEmpty())
    	{
        for(unsigned int i=0;i<currentData.rows();i++)
                {
                QString tmp;
                tmp="Legend "+tmp.setNum(i);
                _params->legend+=tmp;
                }
        }

    if(_params->xlbl.isEmpty())
    	{
        for(unsigned int i=0;i<currentData.cols();i++)
                {
                QString tmp;
                tmp="Year 200"+tmp.setNum(i);
    	        _params->xlbl+=tmp;
                }
    	}

QArray<int> tmpExp(currentData.cols()*currentData.rows());
QArray<bool> tmpMissing(currentData.cols()*currentData.rows());

for(unsigned int i=0; i<(currentData.cols()*currentData.rows()); ++i )
  {
  tmpExp[i]=0;
  tmpMissing[i]=FALSE;
  }
if(_params->missing.isEmpty())
	{
  	_params->missing=tmpMissing;
  	}
if(_params->explode.isEmpty())
	{
  	_params->explode=tmpExp;
	}
}

void KChartPart::loadConfig( KConfig *conf ) {
    _params->loadConfig(conf);
}

void KChartPart::defaultConfig(  ) {
    _params->defaultConfig();
}

void KChartPart::saveConfig( KConfig *conf ) {
    _params->saveConfig(conf);
}

bool KChartPart::save( ostream& out, const char * /*_format*/ ) {
  kdDebug(35001) << "save kchart called!" << endl;
  QDomDocument doc( "chart" );
  doc.appendChild( doc.createProcessingInstruction( "xml", "version=\"1.0\" encoding=\"UTF-8\"" ) );
  QDomElement chart = doc.createElement( "chart" );
  chart.setAttribute( "author", "Kalle Dalheimer, Laszlo Boloni" );
  chart.setAttribute( "email", "kalle@dalheimer.org, boloni@cs.purdue.edu" );
  chart.setAttribute( "editor", "KChart" );
  chart.setAttribute( "mime", "application/x-kchart" );
  doc.appendChild( chart );
  // now save the data
  QDomElement data = doc.createElement("data");
  data.setAttribute("rows", currentData.rows());
  data.setAttribute("cols", currentData.cols());
  for (unsigned int row = 0;row < currentData.rows();row++) {
      for (unsigned int col = 0;col < currentData.cols();col++) {
	// later we need a value
	kdDebug(35001) << "Row " << row << endl;
	KChartValue t = currentData.cell(row, col);
	QDomElement e = doc.createElement("cell");
        e.setAttribute("hide", (int)_params->missing[currentData.cols()*col+row]);
        e.setAttribute("dist", _params->explode[currentData.cols()*col+row]);
        e.setAttribute("value", t.value.toDouble());
        	  /*
	  if ( e.isNull() )
	      return e;
	  */
	data.appendChild(e);
      }
  }
  // now save the parameters
  chart.appendChild(data);

  QDomElement params = doc.createElement("params");
  chart.appendChild(params);
  params.setAttribute("type",(int)_params->type);
  params.setAttribute("subtype",(int)_params->stack_type);
  params.setAttribute("hlc_style",(int)_params->hlc_style);

  if(!_params->title.isEmpty())
        {
        QDomElement title = doc.createElement( "title" );
        title.appendChild( doc.createTextNode( _params->title ) );
        params.appendChild( title );
        QDomElement titlefont = doc.createElement("titlefont");
        params.appendChild(titlefont);
        titlefont.appendChild( createElement( "font", _params->titleFont(), doc ) );

        }


  if(!_params->xtitle.isEmpty())
        {
        QDomElement xtitle = doc.createElement( "xtitle" );
        xtitle.appendChild( doc.createTextNode( _params->xtitle ) );
        params.appendChild( xtitle );
        QDomElement xtitlefont = doc.createElement("xtitlefont");
        xtitlefont.appendChild( createElement( "font", _params->xTitleFont(), doc  ) );
        params.appendChild(xtitlefont);
        }
  if(!_params->ytitle.isEmpty())
        {
        QDomElement ytitle = doc.createElement( "ytitle" );
        ytitle.appendChild( doc.createTextNode( _params->ytitle ) );
        params.appendChild( ytitle );
        QDomElement ytitlefont = doc.createElement("ytitlefont");
        ytitlefont.appendChild( createElement( "font", _params->yTitleFont(), doc ) );
        params.appendChild(ytitlefont);
        }
  if(!_params->ytitle2.isEmpty())
        {
        QDomElement ytitle2 = doc.createElement( "ytitle2" );
        ytitle2.appendChild( doc.createTextNode( _params->ytitle2 ) );
        params.appendChild( ytitle2 );
        }
   if((!_params->ylabel_fmt.isEmpty())||(!_params->ylabel2_fmt.isEmpty()))
        {
        if(!_params->ylabel_fmt.isEmpty())
                {
                QDomElement ylabelfmt = doc.createElement( "ylabelfmt" );
                ylabelfmt.appendChild( doc.createTextNode( _params->ylabel_fmt ) );
                params.appendChild( ylabelfmt );
                }
        if(!_params->ylabel2_fmt.isEmpty())
                {
                QDomElement ylabel2fmt = doc.createElement( "ylabel2fmt" );
                ylabel2fmt.appendChild( doc.createTextNode( _params->ylabel2_fmt ) );
                params.appendChild( ylabel2fmt );
                }

        }
  QDomElement labelfont = doc.createElement("labelfont");
  labelfont.appendChild( createElement( "font", _params->labelFont(), doc ) );
  params.appendChild(labelfont);

  QDomElement yaxisfont = doc.createElement("yaxisfont");
  yaxisfont.appendChild( createElement( "font", _params->yAxisFont(), doc ) );
  params.appendChild(yaxisfont);

  QDomElement xaxisfont = doc.createElement("xaxisfont");
  xaxisfont.appendChild( createElement( "font", _params->xAxisFont(), doc ) );
  params.appendChild(xaxisfont);

  QDomElement annotationFont = doc.createElement("annotationfont");
  annotationFont.appendChild( createElement( "font", _params->annotationFont(), doc ) );
  params.appendChild(annotationFont);

  QDomElement yaxis = doc.createElement("yaxis");
  yaxis.setAttribute("ymin",_params->requested_ymin);
  yaxis.setAttribute("ymax",_params->requested_ymax);
  yaxis.setAttribute("yinterval",_params->requested_yinterval);
  params.appendChild(yaxis);

  QDomElement graph = doc.createElement("graph");
  graph.setAttribute("grid",(int)_params->grid);
  graph.setAttribute("xaxis",(int)_params->xaxis);
  graph.setAttribute("yaxis",(int)_params->yaxis);
  graph.setAttribute("shelf",(int)_params->shelf);
  graph.setAttribute("yaxis2",(int)_params->yaxis2);
  graph.setAttribute("ystyle",(int)_params->yval_style);
  graph.setAttribute("border",(int)_params->border);
  graph.setAttribute("transbg",(int)_params->transparent_bg);
  graph.setAttribute("xlabel",(int)_params->hasxlabel);
  graph.setAttribute("line",(int)_params->label_line);
  graph.setAttribute("percent",(int)_params->percent_labels);
  graph.setAttribute("cross",(int)_params->cross);
  params.appendChild(graph);
  //graph params
  QDomElement graphparams = doc.createElement("graphparams");
  graphparams.setAttribute("depth3d",(double)_params->_3d_depth);
  graphparams.setAttribute("angle3d",(short)_params->_3d_angle);
  graphparams.setAttribute("barwidth",(short)_params->bar_width);
  graphparams.setAttribute("colpie",(int)_params->colPie);
  graphparams.setAttribute("labeldist",(int)_params->label_dist);
  params.appendChild(graphparams);
  //graph color
  QDomElement graphcolor = doc.createElement("graphcolor");
  graphcolor.setAttribute( "bgcolor", _params->BGColor.name() );
  graphcolor.setAttribute( "gridcolor", _params->GridColor.name() );
  graphcolor.setAttribute( "linecolor", _params->LineColor.name() );
  graphcolor.setAttribute( "plotcolor", _params->PlotColor.name() );
  graphcolor.setAttribute( "volcolor", _params->VolColor.name() );
  graphcolor.setAttribute( "titlecolor", _params->TitleColor.name() );
  graphcolor.setAttribute( "xtitlecolor", _params->XTitleColor.name() );
  graphcolor.setAttribute( "ytitlecolor", _params->YTitleColor.name() );
  graphcolor.setAttribute( "ytitle2color", _params->YTitle2Color.name() );
  graphcolor.setAttribute( "xlabelcolor", _params->XLabelColor.name() );
  graphcolor.setAttribute( "ylabelcolor", _params->YLabelColor.name() );
  graphcolor.setAttribute( "ylabel2color", _params->YLabel2Color.name() );
  params.appendChild(graphcolor);

  if(_params->annotation)
        {
        QDomElement annotation = doc.createElement("annotation");
        annotation.setAttribute("color", _params->annotation->color.name());
        annotation.setAttribute("point",(int)_params->annotation->point);
        params.appendChild(annotation);

        QDomElement note = doc.createElement( "note" );
        note.appendChild( doc.createTextNode( _params->annotation->note ) );
        params.appendChild( note );
        }

  QDomElement legend = doc.createElement("legend");
  chart.appendChild(legend);
  legend.setAttribute("number",_params->legend.count());
  for(QStringList::Iterator it = _params->legend.begin();it  != _params->legend.end();++it)
        {
        QDomElement name = doc.createElement( "name" );
        name.appendChild( doc.createTextNode( *it ) );
        legend.appendChild(name);
        }

  QDomElement xlbl = doc.createElement("xlbl");
  chart.appendChild(xlbl);
  xlbl.setAttribute("number",_params->xlbl.count());
  for(QStringList::Iterator it = _params->xlbl.begin();it  != _params->xlbl.end();++it)
        {
        QDomElement label = doc.createElement( "label" );
        label.appendChild( doc.createTextNode( *it ) );
        xlbl.appendChild(label);
        }

  QDomElement extColor = doc.createElement("extcolor");
  chart.appendChild(extColor);
  extColor.setAttribute("number",_params->ExtColor.count());
  for(unsigned int i=0;i<_params->ExtColor.count();i++)
        {
        QDomElement color = doc.createElement( "color" );
        color.setAttribute( "name", _params->ExtColor.color(i).name() );
        extColor.appendChild(color);
        }


  kdDebug(35001) << "Ok, till here!!!" << endl;
  QBuffer buffer;
  buffer.open( IO_WriteOnly );
  QTextStream str( &buffer );
  str << doc;
  buffer.close();

  out.write( buffer.buffer().data(), buffer.buffer().size() );

  //  setModified( false );
  return true;
};

bool KChartPart::loadChildren( KoStore* /*_store*/ ) {
  kdDebug(35001) << "kchart loadChildren called" << endl;
  return true;
};

bool KChartPart::loadXML( const QDomDocument& doc, KoStore* /*store*/ ) {
  kdDebug(35001) << "kchart loadXML called" << endl;
  // <spreadsheet>
  //  m_bLoading = true;
  if ( doc.doctype().name() != "chart" )
  {
    //m_bLoading = false;
    return false;
  }

  kdDebug(35001) << "Ok, it is a chart" << endl;

  QDomElement chart = doc.documentElement();
  if ( chart.attribute( "mime" ) != "application/x-kchart" )
    return false;

  kdDebug(35001) << "Mimetype ok" << endl;

  QDomElement data = chart.namedItem("data").toElement();
  bool ok;
  int cols = data.attribute("cols").toInt(&ok);
  kdDebug(35001) << "cols readed as:" << cols << endl;
  if (!ok)  { return false; }
  int rows = data.attribute("rows").toInt(&ok);
  if (!ok)  { return false; }
  kdDebug(35001) << rows << " x " << cols << endl;
  currentData.expand(rows, cols);
  kdDebug(35001) << "Expanded!" << endl;
  QDomNode n = data.firstChild();
  QArray<int> tmpExp(rows*cols);
  QArray<bool> tmpMissing(rows*cols);

  for (int i=0; i!=rows; i++)
   {
    for (int j=0; j!=cols; j++)
     {
      if (n.isNull())
      {
	kdDebug(35001) << "Some problems, there is less data than it should be!" << endl;
	break;
      }
      QDomElement e = n.toElement();
      if ( !e.isNull() && e.tagName() == "cell" )
      {
	  // add the cell to the corresponding place...
	  double val = e.attribute("value").toDouble(&ok);
	  if (!ok)  {  return false; }
	  kdDebug(35001) << i << " " << j << "=" << val << endl;
	  KChartValue t;
	  t.exists= true;
	  t.value = val;
	  // kdDebug(35001) << "Set cell for " << row << "," << col << endl;
	  currentData.setCell(i,j,t);
          if ( e.hasAttribute( "hide" ) )
                {
	        tmpMissing[cols*j+i] = (bool)e.attribute("hide").toInt( &ok );
	        if ( !ok )
	                return false;
                }
         else
                {
                tmpMissing[cols*j+i] = false;
                }
         if ( e.hasAttribute( "dist" ) )
                {
	        tmpExp[cols*j+i] = e.attribute("dist").toInt( &ok );
	        if ( !ok )
	                return false;
                }
         else
                {
                tmpExp[cols*j+i] = 0;
                }

         n = n.nextSibling();
      }
    }
  }
  _params->missing=tmpMissing;
  _params->explode=tmpExp;

  QDomElement params = chart.namedItem( "params" ).toElement();
  if ( params.hasAttribute( "type" ) )
        {
	 _params->type = (KChartType)params.attribute("type").toInt( &ok );
	 if ( !ok )
	        return false;
         }
  if ( params.hasAttribute( "subtype" ) )
        {
	 _params->stack_type = (KChartStackType)params.attribute("subtype").toInt( &ok );
	 if ( !ok )
	        return false;
         }
  if ( params.hasAttribute( "hlc_style" ) )
        {
	 _params->hlc_style = (KChartHLCStyle)params.attribute("hlc_style").toInt( &ok );
	 if ( !ok )
	        return false;
         }

  QDomElement title = params.namedItem( "title" ).toElement();
    if ( !title.isNull())
        {
         QString t = title.text();
         _params->title=t;
        }
  QDomElement titlefont = params.namedItem( "titlefont" ).toElement();
    if ( !titlefont.isNull())
        {
        QDomElement font = titlefont.namedItem( "font" ).toElement();
	    if ( !font.isNull() )
		_params->setTitleFont(toFont(font));
        }
  QDomElement xtitle = params.namedItem( "xtitle" ).toElement();
    if ( !xtitle.isNull())
        {
         QString t = xtitle.text();
         _params->xtitle=t;
        }
  QDomElement xtitlefont = params.namedItem( "xtitlefont" ).toElement();
    if ( !xtitlefont.isNull())
        {
        QDomElement font = xtitlefont.namedItem( "font" ).toElement();
	    if ( !font.isNull() )
		_params->setXTitleFont(toFont(font));
        }
  QDomElement ytitle = params.namedItem( "ytitle" ).toElement();
    if ( !ytitle.isNull())
        {
         QString t = ytitle.text();
         _params->ytitle=t;
        }
  QDomElement ytitle2 = params.namedItem( "ytitle2" ).toElement();
    if ( !ytitle2.isNull())
        {
         QString t = ytitle2.text();
         _params->ytitle2=t;
        }
  QDomElement ytitlefont = params.namedItem( "ytitlefont" ).toElement();
    if ( !ytitlefont.isNull())
        {
        QDomElement font = ytitlefont.namedItem( "font" ).toElement();
	    if ( !font.isNull() )
		_params->setYTitleFont(toFont(font));
        }
  QDomElement ylabelfmt = params.namedItem( "ylabelfmt" ).toElement();
    if ( !ylabelfmt.isNull())
        {
         QString t = ylabelfmt.text();
         _params->ylabel_fmt=t;
        }
  QDomElement ylabel2fmt = params.namedItem( "ylabel2fmt" ).toElement();
    if ( !ylabel2fmt.isNull())
        {
         QString t = ylabel2fmt.text();
         _params->ylabel2_fmt=t;
        }
  QDomElement labelfont = params.namedItem( "labelfont" ).toElement();
    if ( !labelfont.isNull())
        {
        QDomElement font = labelfont.namedItem( "font" ).toElement();
	    if ( !font.isNull() )
		_params->setLabelFont(toFont(font));
        }

  QDomElement yaxisfont = params.namedItem( "yaxisfont" ).toElement();
    if ( !yaxisfont.isNull())
        {
        QDomElement font = yaxisfont.namedItem( "font" ).toElement();
	    if ( !font.isNull() )
		_params->setYAxisFont(toFont(font));
        }

  QDomElement xaxisfont = params.namedItem( "xaxisfont" ).toElement();
    if ( !xaxisfont.isNull())
        {
        QDomElement font = xaxisfont.namedItem( "font" ).toElement();
	    if ( !font.isNull() )
		_params->setXAxisFont(toFont(font));
        }
  QDomElement annotationFont = params.namedItem("annotationfont").toElement();
    if ( !annotationFont.isNull())
        {
        QDomElement font = annotationFont.namedItem( "font" ).toElement();
	    if ( !font.isNull() )
		_params->setAnnotationFont(toFont(font));
        }

  QDomElement yaxis = params.namedItem( "yaxis" ).toElement();
    if ( !yaxis.isNull())
        {
        if(yaxis.hasAttribute( "yinterval" ))
                {
                _params->requested_yinterval= yaxis.attribute("yinterval").toDouble( &ok );
	        if ( !ok ) return false;
                }
        if(yaxis.hasAttribute( "ymin" ))
                {
                _params->requested_ymin= yaxis.attribute("ymin").toDouble( &ok );
	        if ( !ok ) return false;
                }
         if(yaxis.hasAttribute( "ymax" ))
                {
                _params->requested_ymax= yaxis.attribute("ymax").toDouble( &ok );
	        if ( !ok ) return false;
                }
        }
  QDomElement graph = params.namedItem( "graph" ).toElement();
  if(!graph.isNull())
        {
        if(graph.hasAttribute( "grid" ))
                {
                _params->grid=(bool) graph.attribute("grid").toInt( &ok );
                if(!ok) return false;
                }
         if(graph.hasAttribute( "xaxis" ))
                {
                _params->xaxis=(bool) graph.attribute("xaxis").toInt( &ok );
                if(!ok) return false;
                }
         if(graph.hasAttribute( "yaxis" ))
                {
                _params->yaxis=(bool) graph.attribute("yaxis").toInt( &ok );
                if(!ok) return false;
                }
          if(graph.hasAttribute( "shelf" ))
                {
                _params->shelf=(bool) graph.attribute("shelf").toInt( &ok );
                if(!ok) return false;
                }
          if(graph.hasAttribute( "yaxis2" ))
                {
                _params->yaxis2=(bool) graph.attribute("yaxis2").toInt( &ok );
                if(!ok) return false;
                }
          if(graph.hasAttribute( "ystyle" ))
                {
                _params->yval_style=(bool) graph.attribute("ystyle").toInt( &ok );
                if(!ok) return false;
                }
          if(graph.hasAttribute( "border" ))
                {
                _params->border=(bool) graph.attribute("border").toInt( &ok );
                if(!ok) return false;
                }
          if(graph.hasAttribute( "transbg" ))
                {
                _params->transparent_bg=(bool) graph.attribute("transbg").toInt( &ok );
                if(!ok) return false;
                }
          if(graph.hasAttribute( "xlabel" ))
                {
                _params->hasxlabel=(bool) graph.attribute("xlabel").toInt( &ok );
                if(!ok) return false;
                }
          if(graph.hasAttribute( "line"))
                {
                _params->label_line=(bool) graph.attribute("line").toInt( &ok );
                if(!ok) return false;
                }
          if(graph.hasAttribute( "percent"))
                {
                _params->percent_labels=(KChartPercentType) graph.attribute("percent").toInt( &ok );
                if(!ok) return false;
                }
          if(graph.hasAttribute("cross"))
                {
                _params->cross=(bool) graph.attribute("cross").toInt( &ok );
                if(!ok) return false;
                }
        }
  QDomElement graphparams = params.namedItem( "graphparams" ).toElement();
  if(!graphparams.isNull())
        {
         if(graphparams.hasAttribute( "dept3d" ))
                {
                _params->_3d_depth=graphparams.attribute("dept3d").toDouble( &ok );
                if(!ok) return false;
                }
         if(graphparams.hasAttribute( "angle3d" ))
                {
                _params->_3d_angle=graphparams.attribute("angle3d").toShort( &ok );
                if(!ok) return false;
                }
         if(graphparams.hasAttribute( "barwidth" ))
                {
                _params->bar_width=graphparams.attribute("barwidth").toShort( &ok );
                if(!ok) return false;
                }
         if(graphparams.hasAttribute( "colpie" ))
                {
                _params->colPie=graphparams.attribute("colpie").toInt( &ok );
                if(!ok) return false;
                }
         if(graphparams.hasAttribute( "labeldist" ))
                {
                _params->label_dist=graphparams.attribute("labeldist").toInt( &ok );
                if(!ok) return false;
                }
        }

   QDomElement graphcolor = params.namedItem( "graphcolor" ).toElement();
   if(!graphcolor.isNull())
        {
         if(graphcolor.hasAttribute( "bgcolor" ))
                {
                _params->BGColor= QColor( graphcolor.attribute( "bgcolor" ) );
                }
         if(graphcolor.hasAttribute( "gridcolor" ))
                {
                _params->GridColor= QColor( graphcolor.attribute( "gridcolor" ) );
                }
         if(graphcolor.hasAttribute( "linecolor" ))
                {
                _params->LineColor= QColor( graphcolor.attribute( "linecolor" ) );
                }
         if(graphcolor.hasAttribute( "plotcolor" ))
                {
                _params->PlotColor= QColor( graphcolor.attribute( "plotcolor" ) );
                }
         if(graphcolor.hasAttribute( "volcolor" ))
                {
                _params->VolColor= QColor( graphcolor.attribute( "volcolor" ) );
                }
         if(graphcolor.hasAttribute( "titlecolor" ))
                {
                _params->TitleColor= QColor( graphcolor.attribute( "titlecolor" ) );
                }
         if(graphcolor.hasAttribute( "xtitlecolor" ))
                {
                _params->XTitleColor= QColor( graphcolor.attribute( "xtitlecolor" ) );
                }
         if(graphcolor.hasAttribute( "ytitlecolor" ))
                {
                _params->YTitleColor= QColor( graphcolor.attribute( "ytitlecolor" ) );
                }
         if(graphcolor.hasAttribute( "ytitle2color" ))
                {
                _params->YTitle2Color= QColor( graphcolor.attribute( "ytitle2color" ) );
                }
         if(graphcolor.hasAttribute( "xlabelcolor" ))
                {
                _params->XLabelColor= QColor( graphcolor.attribute( "xlabelcolor" ) );
                }
         if(graphcolor.hasAttribute( "ylabelcolor" ))
                {
                _params->YLabelColor= QColor( graphcolor.attribute( "ylabelcolor" ) );
                }
         if(graphcolor.hasAttribute( "ylabel2color" ))
                {
                _params->YLabel2Color= QColor( graphcolor.attribute( "ylabel2color" ) );
                }
        }

   QDomElement annotation = params.namedItem( "annotation" ).toElement();
   if(!annotation.isNull())
        {
        _params->annotation=new KChartAnnotationType;
        if(annotation.hasAttribute( "color" ))
                {
                _params->annotation->color= QColor( annotation.attribute( "color" ) );
                }
        if(annotation.hasAttribute( "point" ))
                {
                _params->annotation->point=annotation.attribute("point").toDouble( &ok );
                if(!ok) return false;
                }
        }
   QDomElement note = params.namedItem( "note" ).toElement();
   if ( !note.isNull())
        {
         QString t = note.text();
         _params->annotation->note=t;
        }

  QDomElement legend = chart.namedItem("legend").toElement();
  if(!legend.isNull())
  {
  int number = legend.attribute("number").toInt(&ok);
  if (!ok)  { return false; }
  QDomNode name = legend.firstChild();
  _params->legend.clear();
  for (int i=0; i<number; i++)
  {
  if (name.isNull())
        {
	kdDebug(35001) << "Some problems, there is less data than it should be!" << endl;
	break;
        }
   QDomElement element = name.toElement();
   if ( !element.isNull() && element.tagName() == "name" )
        {
        QString t = element.text();
        _params->legend+=t;
        name = name.nextSibling();
        }
  }

  }
  QDomElement xlbl = chart.namedItem("xlbl").toElement();
  if(!xlbl.isNull())
  {
  int number = xlbl.attribute("number").toInt(&ok);
  if (!ok)  { return false; }
  QDomNode label = xlbl.firstChild();
  _params->xlbl.clear();
  for (int i=0; i<number; i++)
  {
  if (label.isNull())
        {
	kdDebug(35001) << "Some problems, there is less data than it should be!" << endl;
	break;
        }
   QDomElement element = label.toElement();
   if ( !element.isNull() && element.tagName() == "label" )
        {
        QString t = element.text();
        _params->xlbl+=t;
        label = label.nextSibling();
        }
   }
   }

  QDomElement extcolor = chart.namedItem("extcolor").toElement();
  if(!extcolor.isNull())
  {
  unsigned int number = extcolor.attribute("number").toInt(&ok);
  if (!ok)  { return false; }
  QDomNode color = extcolor.firstChild();

  for (unsigned int i=0; i<number; i++)
  {
  if (color.isNull())
        {
	kdDebug(35001) << "Some problems, there is less data than it should be!" << endl;
	break;
        }
   QDomElement element = color.toElement();
   if ( !element.isNull())
        {
        if(element.hasAttribute( "name" ))
                {
                _params->ExtColor.setColor(i,QColor( element.attribute( "name" ) ));
                }
        color = color.nextSibling();
        }
   }
   }



  return true;
};

bool KChartPart::load( istream& in, KoStore* store )
{
  kdDebug(35001) << "kchart load called" << endl;
  m_bLoading = true;
  _params = new KChartParameters;
    QBuffer buffer;
    buffer.open( IO_WriteOnly );

    char buf[ 4096 ];
    int anz;
    do
    {
	in.read( buf, 4096 );
	anz = in.gcount();
	buffer.writeBlock( buf, anz );
    } while( anz > 0 );

    buffer.close();

    buffer.open( IO_ReadOnly );
    QDomDocument doc;
    doc.setContent( &buffer );

    bool b = loadXML( doc, store );
    //bool b = true;
    //initDoc();

    buffer.close();
    // init the parameters
    m_bLoading = false;
    return b;
};

QDomElement KChartPart::createElement(const QString &tagName, const QFont &font, QDomDocument &doc) const {

    QDomElement e=doc.createElement( tagName );

    e.setAttribute( "family", font.family() );
    e.setAttribute( "size", font.pointSize() );
    e.setAttribute( "weight", font.weight() );
    if ( font.bold() )
	e.setAttribute( "bold", "yes" );
    if ( font.italic() )
	e.setAttribute( "italic", "yes" );

    return e;
}

QFont KChartPart::toFont(QDomElement &element) const {

    QFont f;
    f.setFamily( element.attribute( "family" ) );

    bool ok;
    f.setPointSize( element.attribute("size").toInt( &ok ) );
    if ( !ok ) return QFont();

    f.setWeight( element.attribute("weight").toInt( &ok ) );
    if ( !ok ) return QFont();

    if ( element.hasAttribute( "italic" ) )
	f.setItalic( TRUE );

    if ( element.hasAttribute( "bold" ) )
	f.setBold( TRUE );

    return f;
}
#include "kchart_part.moc"

/**
 * $Log$
 * Revision 1.35  2000/06/14 19:44:02  wtrobin
 * even more QDom
 *
 * Revision 1.34  2000/06/14 18:37:24  wtrobin
 * QDom fun
 *
 * Revision 1.33  2000/06/12 19:46:22  faure
 * Ported to new kofficecore
 *
 * Revision 1.32  2000/04/21 06:34:21  wtrobin
 * qDebug, debug, cerr, cout,... -> kdDebug(35001)
 *
 * Revision 1.31  2000/04/08 16:43:13  mlaurent
 * Now "Chart Combo*" works
 * Bug fix
 *
 * Revision 1.30  2000/04/02 16:41:46  mlaurent
 * Now you can add an annotation
 * Bug fix
 * Improved "Config Dialog"
 *
 * Revision 1.29  2000/02/23 05:50:06  mlaurent
 * Bug fix
 *
 * Revision 1.28  2000/02/20 22:39:54  mueller
 * compilation fix with new Qt
 *
 * Revision 1.27  2000/02/13 19:32:03  mlaurent
 * bug fix
 *
 * Revision 1.26  2000/02/13 14:34:18  hausmann
 * - fixed segfault in KoView destructor (use a QGuardedPtr on the manager)
 * - hacked KoDocument to hold a list of associated shells and kill them
 *   upon destruction of the document
 * - fixed KoMainWindow and all the apps to let each shell have a *correct*
 *   KInstance assigned
 * - ignore generated Makefiles in doc/katabase
 *
 * Revision 1.25  2000/02/08 13:12:30  coolo
 * some fixes for ANSI C++ (I gave up on kchart ;(
 *
 * Revision 1.24  2000/02/05 18:12:33  shausman
 * the port of KOffice to use kparts instead of koparts. What I basically did
 * was:
 * - merged the container/view functionality from koparts into kofficecore
 * - made KoDocument inherits KParts::ReadWritePart
 * - added support for single-view-mode to KoDocument and reimplemented
 *   some virtual methods of KParts::Part to redirect requests to the view
 *   (like action requests for example)
 * - ported the nested child activation/deactivation code to
 *   KParts::PartManager (which was a pain, as the old code just deleted
 *   child views upon deactivation, while in kparts we first have to
 *   deactivate a views GUI before we can delete it)
 * - removed the m_bDoPartActivation and partDoubleClick hacks from the
 *   KWord/KPresenter activation stuff ;-) and replaced with a more simple
 *   mechanism of adding the child parts to the tree just right before the
 *   activation
 *   (there are still bugs in the nested part activation/deactivation stuff
 *   though, but I'll look into that)
 * - got rid of all the duplicated shell xml documents and used a common
 *   shared shell gui
 * - added support for read-write state changing (as supported by kparts)
 *   I currently only implemented it in KSpread and added big TODO warnings
 *   to the other apps ;-) . In those methods just deactivate all actions
 *   and widgets which could possibly modify the document, in case we
 *   enter readonly state (this is *VERY* important, *please* support it
 *   properly)
 * - ported all the .rc files to the kparts standard and ported the
 *   calculator plugin of kspread to a real kparts plugin
 * - I *removed* support for inplace editing! (important, Torben, please read
 *   :-)
 *   (I commented out the inplace xml stuff in the .rc files)
 *   The reason for the disabled support is:
 *   I can't see how this is supposed to work at all!
 *   Imaginge I embed a KSpread document into KWord, rotate it and activate
 *   it. This is when the inplace-editing GUI is meant to be used, instead of
 *   the "usual" kspread gui. BUT: This cannot work, as the actions are
 *   only available (allocated, implemented, used, etc..) in a KoView.
 *   HOWEVER a KoView is *not* created in this case, so I cannot see how this
 *   is supposed to work?
 *   Perhaps I'm missing something, but the only situation I see where it
 *   worked was in kspread itself ;-) , as if I embed a kspread table into
 *   another kspread table, then the shell (in koparts) considered the
 *   embedded document to be active, but the active view (as this is where
 *   the user clicked) is *still* the parent KoView (kspread view). So
 *   the shell was able to find the actions, apparently from the completely
 *   wrong view though. Perhaps Torben can help here? :-)
 *   (perhaps we should move all actions from the view to the document?)
 * - added check if the global application object is a KoApplication, so that
 *   the filter stuff is only used inside KOffice (otherwise it used
 *   to crash horribly ;)
 *
 * Revision 1.23  2000/01/28 20:00:37  mlaurent
 * Improved save and load parameters (label,legend,extcolor)
 *
 * Revision 1.22  2000/01/23 14:48:48  mlaurent
 * Improved pie chart
 *
 * Revision 1.21  2000/01/22 19:55:19  mlaurent
 * Add new page for config pie chart
 * Now pie chart works (a lot :( there are against bug)
 *
 * Revision 1.20  2000/01/21 16:04:03  mlaurent
 * Add legend and label
 * Add new page for config font
 * bug fix
 *
 * Revision 1.19  2000/01/08 20:36:21  mlaurent
 * Now you can choose a "sub type chart"
 * Now there are a color for same think
 *
 * Revision 1.18  2000/01/07 20:37:42  mlaurent
 * *** empty log message ***
 *
 * Revision 1.17  2000/01/06 20:32:46  mlaurent
 * Bug fix
 * Now you can choose font and color in kchartWizard
 *
 * Revision 1.16  2000/01/05 20:09:51  mlaurent
 * Improved save parameters
 *
 * Revision 1.15  2000/01/05 07:50:22  mlaurent
 * Improved save parameters
 *
 * Revision 1.14  2000/01/04 21:02:31  mlaurent
 * Start save parameters in file
 *
 * Revision 1.13  2000/01/03 20:26:42  mlaurent
 * Improved kchartWizard and bugfix
 *
 * Revision 1.12  1999/12/21 22:36:17  faure
 * Porting to new QVariant. More like this and I write a script !
 *
 * Revision 1.11  1999/11/29 21:26:14  wtrobin
 * - fixed some ugly warnings
 * - made kchart compile with --enable-final
 *
 * Revision 1.10  1999/11/21 20:26:45  boloni
 * -multidocument view works - but it still freezes at load and new. But this
 * time it is probably my fault somewhere
 *
 * Revision 1.9  1999/11/21 17:43:29  boloni
 * -save and load works but the New and Load mechanisms from KoMainWindow hangs
 * it. Use loading from the command line.
 * -there is something wrong with that mechanism, because KWord is not working in multiple document mode either - it just overwrites the old one for New or Load.
 *
 * Revision 1.8  1999/11/21 16:40:13  boloni
 * save-load works
 * data files can be specified at the command line
 * the is some problem with load, still
 *
 * Revision 1.7  1999/11/21 15:27:14  boloni
 * ok
 *
 * Revision 1.6  1999/11/19 05:04:35  boloni
 * more work on saving
 *
 * Revision 1.5  1999/11/17 02:49:53  boloni
 * -started implementing save and load.
 *
 * Revision 1.4  1999/11/16 03:00:56  boloni
 * -enabling grid and label drawing. Some more small reorganizations
 * -one more page in the wizard.
 *
 * Revision 1.3  1999/11/14 18:02:06  boloni
 * auto-initialization for standalone startup
 * separate class for the kchart data editor
 *
 * Revision 1.2  1999/10/25 04:52:52  boloni
 * -ok, the gray rectangle which Reggie got was due to the fact that the
 * rc files were hardcoded so it worked only from the kchart dir.
 * -changed to the "locate" style and now it has menus if started from other dirs, too.
 * -and btw the kchart.rc was not installed anyhow
 *
 * Revision 1.1  1999/10/20 10:07:32  kulow
 * sync with canossa
 *
 * Revision 1.15  1999/10/18 08:15:10  kalle
 * Pulled the colors (and some other stuff) into KChartParameter
 *
 * Revision 1.14  1999/10/16 14:51:08  kalle
 * Accessor for params, pulled the fonts into KChartParameters (finally!)
 *
 * Revision 1.13  1999/10/15 00:54:16  boloni
 * more work
 *
 * Revision 1.11  1999/10/13 20:25:18  kalle
 * chart type is now taken from param struct
 *
 * Revision 1.10  1999/10/13 15:07:58  kalle
 * More parameter work. Compiles.
 *
 */
