#include "engine.h"
#include "engine2.h"
#include "enginehelper.h"
#include "enginedraw.h"

#include "kchartparams.h"
#include "kchartEngine.h"

#include <qfont.h>
#include <qcolor.h>
#include <qpainter.h>
#include <qimage.h>

#include <stdlib.h>
#include <stdarg.h>
#include <math.h>
#include <kdebug.h>
#include <kpixmapeffect.h>

void KChartEngine::drawAnnotation() {
  int x1 = PX(params->annotation->point+(params->do_bar()?1:0));
  int y1 = PY(highest);
  int x2=0;
  // front line
  p->setPen( AnnoteColor );
  p->drawLine( x1, PY(lowest)+1, x1, y1 );
  if( params->threeD() )
        { // on back plane
        setno = params->stack_type==KCHARTSTACKTYPE_DEPTH? num_hlc_sets? num_hlc_sets: num_sets: 1;
        x2 = PX(params->annotation->point+(params->do_bar()?1:0));
        // prspective line
        p->setPen( AnnoteColor );
        p->drawLine( x1, y1, x2, PY(highest) );
        }
  else
        { // for 3D done with back line
        x2 = PX(params->annotation->point+(params->do_bar()?1:0));
        p->setPen( AnnoteColor );
        p->drawLine( x1, y1, x1, y1-2 );
        }

  /* line-to and note */
  if( !params->annotation->note.isEmpty() )
        {  // any note?
        if( params->annotation->point >= (num_points/2) )
                {/* note to the left */
                p->setPen( AnnoteColor );
                p->drawLine( x2,PY(highest)-2,x2-annote_hgt/2, PY(highest)-2-annote_hgt/2 );
                p->setFont( params->annotationFont() );
                QRect br = QFontMetrics( params->annotationFont() ).boundingRect( 0, 0, MAXINT,
								   MAXINT,
								   Qt::AlignRight,
								   params->annotation->note );
                p->drawText(   x2-annote_hgt/2-1-annote_len - 1,
		     PY(highest)-annote_hgt+1,
		     br.width(), br.height(),
		     Qt::AlignRight, params->annotation->note );
                }
        else
                { /* note to right */
                p->setPen( AnnoteColor );
                p->drawLine( x2, PY(highest)-2,x2+annote_hgt/2, PY(highest)-2-annote_hgt/2 );
                p->setFont( params->annotationFont() );
                QRect br = QFontMetrics( params->annotationFont() ).boundingRect( 0, 0,
									MAXINT,
									MAXINT,
									Qt::AlignLeft,
									params->annotation->note );
                p->drawText( x2+annote_hgt/2+1 + 1,
		   PY(highest)-annote_hgt+1,
		   br.width(), br.height(),
		   Qt::AlignLeft, params->annotation->note );
                }
        }
  setno = 0;
}

void KChartEngine::titleText() {
    if( !params->title.isEmpty() )
        {
        int tlen=0;
        QColor	titlecolor = params->TitleColor;
        cnt_nl( params->title.latin1(), &tlen );
        p->setFont( params->titleFont() );
        p->setPen( titlecolor );
        QRect br = QFontMetrics( params->titleFont() ).boundingRect( 0, 0,
				MAXINT,
				MAXINT,
				Qt::AlignCenter,
				params->title );
        p->drawText( imagewidth/2 - tlen*params->titleFontWidth()/2, // x
				0, // y
				br.width(), br.height(),
				Qt::AlignCenter, params->title );
        }

    kdDebug(35001) << "done with the title text" << endl;

    if( !params->xtitle.isEmpty() )
        {
        //QColor	titlecolor = params->XTitleColor == Qt::black ?
        //                PlotColor: params->XTitleColor;
        
        QColor	titlecolor = params->XTitleColor;
        p->setPen( titlecolor );
        p->setFont( params->xTitleFont() );
        p->drawText( imagewidth/2 - params->xtitle.length()*params->xTitleFontWidth()/2,
                        imageheight-params->xTitleFontHeight()-1, params->xtitle );
    	}
}



void KChartEngine::drawThumbnails() {
#ifdef THUMB_VALS
    /* put thmbl and thumbval over vol and plot lines */
    int n=0, d=0, w=0;
    char thmbl[32];
    char *price_to_str( float, int*, int*, int* );
    char nmrtr[3+1], dmntr[3+1], whole[8];

    char *dbg = price_to_str( ABS(thumbval),&n,&d,&w );
    sprintf( nmrtr, "%d", n );
    sprintf( dmntr, "%d", d );
    sprintf( whole, "%d", w );

    p->setPen( ThumbLblColor );
    p->setFont( gdFontSmall );
    p->drawText( graphwidth/2-strlen(thumblabel)*SFONTWDTH/2,1,thumblabel );

    if( w || n )
        {
        int chgcolor  = thumbval>0.0? ThumbUColor: ThumbDColor;
        int thmbvalwidth = SFONTWDTH +	// up/down arrow
	        (w?strlen(whole)*SFONTWDTH: 0) +	// whole
	        (n?strlen(nmrtr)*TFONTWDTH+// numerator
	        SFONTWDTH +	// /
	        strlen(dmntr)*TFONTWDTH:// denominator
	        0);// no frac part

        smallarrow( p, graphwidth/2-thmbvalwidth/2, SFONTHGT, thumbval>0.0, chgcolor );
        if( w )
                {
	        p->setFont( gdFontSmall );
	        p->setPen( chgcolor );
	        p->drawText( (graphwidth/2-thmbvalwidth/2)+SFONTWDTH,
		     SFONTHGT+2,
		     whole );
                }
        if( n )
                {
	        p->setFont( gdFontTiny );
	        p->setPen( chgcolor );
	        p->drawText( (graphwidth/2-thmbvalwidth/2)   +	// start
		     SFONTWDTH	+	// arrow
		     (w? strlen(whole)*SFONTWDTH: 0) +	// whole
		     2,
		     SFONTHGT+2-2,
		     nmrtr );
	        p->setFont( gdFontSmall );
	        p->drawText( (graphwidth/2-thmbvalwidth/2)  +	// start
		     SFONTWDTH	 +	// arrow
		     (w? strlen(whole)*SFONTWDTH: 0) +	// whole
		     strlen(nmrtr)*TFONTWDTH,	// numerator
		     SFONTHGT+2,
		     '/' );
	        p->setFont( gdFontTiny );
	        p->drawText( (graphwidth/2-thmbvalwidth/2)  +	// start
		     SFONTWDTH	 +		// arrow
		     (w? strlen(whole)*SFONTWDTH: 0) +	// whole
		     strlen(nmrtr)*TFONTWDTH +	// numerator
		     SFONTWDTH - 3,
		     SFONTHGT+2+4,
		     dmntr );
                }
        }
    // thumblabel, thumbval
#endif
}


void KChartEngine::drawBackgroundImage()
{
  static KPixmap backpix;

  if( params->backgroundPixmap.isNull() )
	return;

  if( params->backgroundPixmapIsDirty ) {
	
	// scale loaded pixmap if necessary
	if( params->backgroundPixmapScaled ) {
	  QImage img = backpix.convertToImage();
	  img.smoothScale( imagewidth, imageheight );
	  backpix.convertFromImage( img );
	}
	  
	// tone lighter
	KPixmapEffect::intensity( backpix, params->backgroundPixmapIntensity );
  }

  // Now draw, either NW or centered
  int xpos = 0, ypos = 0;
  if( params->backgroundPixmapCentered ) {
	xpos = ( imagewidth - backpix.width() ) / 2;
	ypos = ( imageheight - backpix.height() ) / 2;
  }
  
  p->drawPixmap( xpos, ypos, backpix );
}


void KChartEngine::prepareColors()
{
    //for(int j=0; j<num_sets; ++j )
      for(int i=0; i<num_points; ++i )
        for(int j=0; j<num_sets; ++j )
          if( params->ExtColor.count() )
                {
	        //kdDebug(35001) << "Ext color" << endl;
	        // changed by me, BL
	        //QColor ext_clr = params->ExtColor.color( num_points*j+i );
                //Montel change to allow that all same think has the same color
                //QColor ext_clr = params->ExtColor.color( (num_points*j+i) % params->ExtColor.count());
                QColor ext_clr = params->ExtColor.color( (j) % params->ExtColor.count());
                ExtColor[j][i] = ext_clr;
                if( params->threeD() )
                        ExtColorShd[j][i] = QColor( ext_clr.red() / 2, ext_clr.green() / 2, ext_clr.blue() / 2 );
	        }
	   else if( params->SetColor.count() )
                {
	        QColor set_clr = params->SetColor.color( j );
	        ExtColor[j][i] = QColor( set_clr );

                if( params->threeD() )
                        ExtColorShd[j][i] = QColor( set_clr.red() / 2, set_clr.green() / 2, set_clr.blue() / 2 );
	        }
	   else
                {
	        ExtColor[j][i] = PlotColor;
                if( params->threeD() )
                        ExtColorShd[j][i] = QColor( params->PlotColor.red() / 2, params->PlotColor.green() / 2, params->PlotColor.blue() / 2 );
	        }
}
