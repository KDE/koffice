/******************************************************************/
/* KPresenter - (c) by Reginald Stadlbauer 1997-1998              */
/* Version: 0.1.0                                                 */
/* Author: Reginald Stadlbauer                                    */
/* E-Mail: reggie@kde.org                                         */
/* Homepage: http://boch35.kfunigraz.ac.at/~rs                    */
/* needs c++ library Qt (http://www.troll.no)                     */
/* needs mico (http://diamant.vsb.cs.uni-frankfurt.de/~mico/)     */
/* needs OpenParts and Kom (weis@kde.org)                         */
/* written for KDE (http://www.kde.org)                           */
/* License: GNU GPL                                               */
/******************************************************************/
/* Module: PenBrush Command                                       */
/******************************************************************/

#include "kpresenter_doc.h"
#include "penbrushcmd.h"

#include "kplineobject.h"
#include "kprectobject.h"
#include "kpellipseobject.h"
#include "kpautoformobject.h"
#include "kpclipartobject.h"
#include "kptextobject.h"
#include "kppixmapobject.h"
#include "kppieobject.h"
#include "kppartobject.h"

/******************************************************************/
/* Class: PenBrushCmd                                             */
/******************************************************************/

/*======================== constructor ===========================*/
PenBrushCmd::PenBrushCmd( QString _name, QList<Pen> &_oldPen, QList<Brush> &_oldBrush,
                          Pen _newPen, Brush _newBrush, QList<KPObject> &_objects, KPresenterDoc *_doc, int _flags = 0 )
    : Command( _name ), oldPen( _oldPen ), oldBrush( _oldBrush ), objects( _objects )
{
    objects.setAutoDelete( false );
    oldPen.setAutoDelete( false );
    oldBrush.setAutoDelete( false );
    doc = _doc;
    newPen = _newPen;
    newBrush = _newBrush;
    flags = _flags;

    for ( unsigned int i = 0; i < objects.count(); i++ )
        objects.at( i )->incCmdRef();
}

/*======================== destructor ============================*/
PenBrushCmd::~PenBrushCmd()
{
    for ( unsigned int i = 0; i < objects.count(); i++ )
        objects.at( i )->decCmdRef();
    oldPen.setAutoDelete( true );
    oldPen.clear();
    oldBrush.setAutoDelete( true );
    oldBrush.clear();
}

/*====================== execute =================================*/
void PenBrushCmd::execute()
{
    KPObject *kpobject = 0L;
    Pen tmpPen = newPen;
    Brush tmpBrush = newBrush;

    for ( int i = 0; i < static_cast<int>( objects.count() ); i++ )
    {
        if ( flags & LB_ONLY )
        {
            newPen.pen = oldPen.at( i )->pen;
            newPen.lineEnd = oldPen.at( i )->lineEnd;
            newBrush = *oldBrush.at( i );
        }
        if ( flags & LE_ONLY )
        {
            newPen.pen = oldPen.at( i )->pen;
            newPen.lineBegin = oldPen.at( i )->lineBegin;
            newBrush = *oldBrush.at( i );
        }
        if ( flags & PEN_ONLY )
        {
            newPen.lineBegin = oldPen.at( i )->lineBegin;
            newPen.lineEnd = oldPen.at( i )->lineEnd;
            if ( newPen.pen != Qt::NoPen )
                newPen.pen = QPen( newPen.pen.color(), oldPen.at( i )->pen.width(),
                                   oldPen.at( i )->pen.style() != Qt::NoPen ? oldPen.at( i )->pen.style() : Qt::SolidLine );
            else
                newPen.pen = QPen( oldPen.at( i )->pen.color(), oldPen.at( i )->pen.width(), Qt::NoPen );
            newBrush = *oldBrush.at( i );
        }
        if ( flags & BRUSH_ONLY )
        {
            newBrush.gColor1 = oldBrush.at( i )->gColor1;
            newBrush.gColor2 = oldBrush.at( i )->gColor2;
            if ( newBrush.brush != Qt::NoBrush )
                newBrush.brush = QBrush( newBrush.brush.color(),
                                         oldBrush.at( i )->brush.style() != Qt::NoBrush ? oldBrush.at( i )->brush.style() : Qt::SolidPattern );
            else
                newBrush.brush = QBrush( oldBrush.at( i )->brush.color(), Qt::NoBrush );
            newBrush.gType = oldBrush.at( i )->gType;
            newPen = *oldPen.at( i );
        }

        kpobject = objects.at( i );
        switch ( kpobject->getType() )
        {
        case OT_LINE:
        {
            dynamic_cast<KPLineObject*>( kpobject )->setPen( newPen.pen );
            dynamic_cast<KPLineObject*>( kpobject )->setLineBegin( newPen.lineBegin );
            dynamic_cast<KPLineObject*>( kpobject )->setLineEnd( newPen.lineEnd );
            doc->repaint( kpobject );
        } break;
        case OT_RECT:
        {
            dynamic_cast<KPRectObject*>( kpobject )->setPen( newPen.pen );
            dynamic_cast<KPRectObject*>( kpobject )->setBrush( newBrush.brush );
            dynamic_cast<KPRectObject*>( kpobject )->setFillType( newBrush.fillType );
            dynamic_cast<KPRectObject*>( kpobject )->setGColor1( newBrush.gColor1 );
            dynamic_cast<KPRectObject*>( kpobject )->setGColor2( newBrush.gColor2 );
            dynamic_cast<KPRectObject*>( kpobject )->setGType( newBrush.gType );
            doc->repaint( kpobject );
        } break;
        case OT_ELLIPSE:
        {
            dynamic_cast<KPEllipseObject*>( kpobject )->setPen( newPen.pen );
            dynamic_cast<KPEllipseObject*>( kpobject )->setBrush( newBrush.brush );
            dynamic_cast<KPEllipseObject*>( kpobject )->setFillType( newBrush.fillType );
            dynamic_cast<KPEllipseObject*>( kpobject )->setGColor1( newBrush.gColor1 );
            dynamic_cast<KPEllipseObject*>( kpobject )->setGColor2( newBrush.gColor2 );
            dynamic_cast<KPEllipseObject*>( kpobject )->setGType( newBrush.gType );
            doc->repaint( kpobject );
        } break;
        case OT_AUTOFORM:
        {
            dynamic_cast<KPAutoformObject*>( kpobject )->setPen( newPen.pen );
            dynamic_cast<KPAutoformObject*>( kpobject )->setBrush( newBrush.brush );
            dynamic_cast<KPAutoformObject*>( kpobject )->setLineBegin( newPen.lineBegin );
            dynamic_cast<KPAutoformObject*>( kpobject )->setLineEnd( newPen.lineEnd );
            dynamic_cast<KPAutoformObject*>( kpobject )->setFillType( newBrush.fillType );
            dynamic_cast<KPAutoformObject*>( kpobject )->setGColor1( newBrush.gColor1 );
            dynamic_cast<KPAutoformObject*>( kpobject )->setGColor2( newBrush.gColor2 );
            dynamic_cast<KPAutoformObject*>( kpobject )->setGType( newBrush.gType );
            doc->repaint( kpobject );
        } break;
        case OT_PIE:
        {
            dynamic_cast<KPPieObject*>( kpobject )->setPen( newPen.pen );
            dynamic_cast<KPPieObject*>( kpobject )->setBrush( newBrush.brush );
            dynamic_cast<KPPieObject*>( kpobject )->setLineBegin( newPen.lineBegin );
            dynamic_cast<KPPieObject*>( kpobject )->setLineEnd( newPen.lineEnd );
            dynamic_cast<KPPieObject*>( kpobject )->setFillType( newBrush.fillType );
            dynamic_cast<KPPieObject*>( kpobject )->setGColor1( newBrush.gColor1 );
            dynamic_cast<KPPieObject*>( kpobject )->setGColor2( newBrush.gColor2 );
            dynamic_cast<KPPieObject*>( kpobject )->setGType( newBrush.gType );
            doc->repaint( kpobject );
        } break;
        case OT_PART:
        {
            dynamic_cast<KPPartObject*>( kpobject )->setPen( newPen.pen );
            dynamic_cast<KPPartObject*>( kpobject )->setBrush( newBrush.brush );
            dynamic_cast<KPPartObject*>( kpobject )->setFillType( newBrush.fillType );
            dynamic_cast<KPPartObject*>( kpobject )->setGColor1( newBrush.gColor1 );
            dynamic_cast<KPPartObject*>( kpobject )->setGColor2( newBrush.gColor2 );
            dynamic_cast<KPPartObject*>( kpobject )->setGType( newBrush.gType );
            doc->repaint( kpobject );
        } break;
        case OT_TEXT:
        {
            dynamic_cast<KPTextObject*>( kpobject )->setPen( newPen.pen );
            dynamic_cast<KPTextObject*>( kpobject )->setBrush( newBrush.brush );
            dynamic_cast<KPTextObject*>( kpobject )->setFillType( newBrush.fillType );
            dynamic_cast<KPTextObject*>( kpobject )->setGColor1( newBrush.gColor1 );
            dynamic_cast<KPTextObject*>( kpobject )->setGColor2( newBrush.gColor2 );
            dynamic_cast<KPTextObject*>( kpobject )->setGType( newBrush.gType );
            doc->repaint( kpobject );
        } break;
        case OT_PICTURE:
        {
            dynamic_cast<KPPixmapObject*>( kpobject )->setPen( newPen.pen );
            dynamic_cast<KPPixmapObject*>( kpobject )->setBrush( newBrush.brush );
            dynamic_cast<KPPixmapObject*>( kpobject )->setFillType( newBrush.fillType );
            dynamic_cast<KPPixmapObject*>( kpobject )->setGColor1( newBrush.gColor1 );
            dynamic_cast<KPPixmapObject*>( kpobject )->setGColor2( newBrush.gColor2 );
            dynamic_cast<KPPixmapObject*>( kpobject )->setGType( newBrush.gType );
            doc->repaint( kpobject );
        } break;
        case OT_CLIPART:
        {
            dynamic_cast<KPClipartObject*>( kpobject )->setPen( newPen.pen );
            dynamic_cast<KPClipartObject*>( kpobject )->setBrush( newBrush.brush );
            dynamic_cast<KPClipartObject*>( kpobject )->setFillType( newBrush.fillType );
            dynamic_cast<KPClipartObject*>( kpobject )->setGColor1( newBrush.gColor1 );
            dynamic_cast<KPClipartObject*>( kpobject )->setGColor2( newBrush.gColor2 );
            dynamic_cast<KPClipartObject*>( kpobject )->setGType( newBrush.gType );
            doc->repaint( kpobject );
        } break;
        default: break;
        }
    }

    newPen = tmpPen;
    newBrush = tmpBrush;
}

/*====================== unexecute ===============================*/
void PenBrushCmd::unexecute()
{
    KPObject *kpobject = 0L;

    for ( unsigned int i = 0; i < objects.count(); i++ )
    {
        kpobject = objects.at( i );
        switch ( kpobject->getType() )
        {
        case OT_LINE:
        {
            if ( oldPen.count() > i )
            {
                dynamic_cast<KPLineObject*>( kpobject )->setPen( oldPen.at( i )->pen );
                dynamic_cast<KPLineObject*>( kpobject )->setLineBegin( oldPen.at( i )->lineBegin );
                dynamic_cast<KPLineObject*>( kpobject )->setLineEnd( oldPen.at( i )->lineEnd );
                doc->repaint( kpobject );
            }
        } break;
        case OT_RECT:
        {
            if ( oldPen.count() > i )
                dynamic_cast<KPRectObject*>( kpobject )->setPen( oldPen.at( i )->pen );
            if ( oldBrush.count() > i )
            {
                dynamic_cast<KPRectObject*>( kpobject )->setBrush( oldBrush.at( i )->brush );
                dynamic_cast<KPRectObject*>( kpobject )->setFillType( oldBrush.at( i )->fillType );
                dynamic_cast<KPRectObject*>( kpobject )->setGColor1( oldBrush.at( i )->gColor1 );
                dynamic_cast<KPRectObject*>( kpobject )->setGColor2( oldBrush.at( i )->gColor2 );
                dynamic_cast<KPRectObject*>( kpobject )->setGType( oldBrush.at( i )->gType );
            }
            doc->repaint( kpobject );
        } break;
        case OT_ELLIPSE:
        {
            if ( oldPen.count() > i )
                dynamic_cast<KPEllipseObject*>( kpobject )->setPen( oldPen.at( i )->pen );
            if ( oldBrush.count() > i )
            {
                dynamic_cast<KPEllipseObject*>( kpobject )->setBrush( oldBrush.at( i )->brush );
                dynamic_cast<KPEllipseObject*>( kpobject )->setFillType( oldBrush.at( i )->fillType );
                dynamic_cast<KPEllipseObject*>( kpobject )->setGColor1( oldBrush.at( i )->gColor1 );
                dynamic_cast<KPEllipseObject*>( kpobject )->setGColor2( oldBrush.at( i )->gColor2 );
                dynamic_cast<KPEllipseObject*>( kpobject )->setGType( oldBrush.at( i )->gType );
            }   
            doc->repaint( kpobject );
        } break;
        case OT_AUTOFORM:
        {
            if ( oldPen.count() > i )
                dynamic_cast<KPAutoformObject*>( kpobject )->setPen( oldPen.at( i )->pen );
            if ( oldBrush.count() > i )
                dynamic_cast<KPAutoformObject*>( kpobject )->setBrush( oldBrush.at( i )->brush );
            if ( oldPen.count() > i )
            {
                dynamic_cast<KPAutoformObject*>( kpobject )->setLineBegin( oldPen.at( i )->lineBegin );
                dynamic_cast<KPAutoformObject*>( kpobject )->setLineEnd( oldPen.at( i )->lineEnd );
            }
            if ( oldBrush.count() > i )
            {
                dynamic_cast<KPAutoformObject*>( kpobject )->setFillType( oldBrush.at( i )->fillType );
                dynamic_cast<KPAutoformObject*>( kpobject )->setGColor1( oldBrush.at( i )->gColor1 );
                dynamic_cast<KPAutoformObject*>( kpobject )->setGColor2( oldBrush.at( i )->gColor2 );
                dynamic_cast<KPAutoformObject*>( kpobject )->setGType( oldBrush.at( i )->gType );
            }
            doc->repaint( kpobject );
        } break;
        case OT_PIE:
        {
            if ( oldPen.count() > i )
                dynamic_cast<KPPieObject*>( kpobject )->setPen( oldPen.at( i )->pen );
            if ( oldBrush.count() > i )
            {
                dynamic_cast<KPPieObject*>( kpobject )->setBrush( oldBrush.at( i )->brush );
                dynamic_cast<KPPieObject*>( kpobject )->setLineBegin( oldPen.at( i )->lineBegin );
                dynamic_cast<KPPieObject*>( kpobject )->setLineEnd( oldPen.at( i )->lineEnd );
                dynamic_cast<KPPieObject*>( kpobject )->setFillType( oldBrush.at( i )->fillType );
                dynamic_cast<KPPieObject*>( kpobject )->setGColor1( oldBrush.at( i )->gColor1 );
                dynamic_cast<KPPieObject*>( kpobject )->setGColor2( oldBrush.at( i )->gColor2 );
                dynamic_cast<KPPieObject*>( kpobject )->setGType( oldBrush.at( i )->gType );
            }
            doc->repaint( kpobject );
        } break;
        case OT_PART:
        {
            if ( oldPen.count() > i )
                dynamic_cast<KPPartObject*>( kpobject )->setPen( oldPen.at( i )->pen );
            if ( oldBrush.count() > i )
            {
                dynamic_cast<KPPartObject*>( kpobject )->setBrush( oldBrush.at( i )->brush );
                dynamic_cast<KPPartObject*>( kpobject )->setFillType( oldBrush.at( i )->fillType );
                dynamic_cast<KPPartObject*>( kpobject )->setGColor1( oldBrush.at( i )->gColor1 );
                dynamic_cast<KPPartObject*>( kpobject )->setGColor2( oldBrush.at( i )->gColor2 );
                dynamic_cast<KPPartObject*>( kpobject )->setGType( oldBrush.at( i )->gType );
            }
            doc->repaint( kpobject );
        } break;
        case OT_TEXT:
        {
            if ( oldPen.count() > i )
                dynamic_cast<KPTextObject*>( kpobject )->setPen( oldPen.at( i )->pen );
            if ( oldBrush.count() > i )
            {
                dynamic_cast<KPTextObject*>( kpobject )->setBrush( oldBrush.at( i )->brush );
                dynamic_cast<KPTextObject*>( kpobject )->setFillType( oldBrush.at( i )->fillType );
                dynamic_cast<KPTextObject*>( kpobject )->setGColor1( oldBrush.at( i )->gColor1 );
                dynamic_cast<KPTextObject*>( kpobject )->setGColor2( oldBrush.at( i )->gColor2 );
                dynamic_cast<KPTextObject*>( kpobject )->setGType( oldBrush.at( i )->gType );
            }
            doc->repaint( kpobject );
        } break;
        case OT_PICTURE:
        {
            if ( oldPen.count() > i )
                dynamic_cast<KPPixmapObject*>( kpobject )->setPen( oldPen.at( i )->pen );
            if ( oldBrush.count() > i )
            {
                dynamic_cast<KPPixmapObject*>( kpobject )->setBrush( oldBrush.at( i )->brush );
                dynamic_cast<KPPixmapObject*>( kpobject )->setFillType( oldBrush.at( i )->fillType );
                dynamic_cast<KPPixmapObject*>( kpobject )->setGColor1( oldBrush.at( i )->gColor1 );
                dynamic_cast<KPPixmapObject*>( kpobject )->setGColor2( oldBrush.at( i )->gColor2 );
                dynamic_cast<KPPixmapObject*>( kpobject )->setGType( oldBrush.at( i )->gType );
            }
            doc->repaint( kpobject );
        } break;
        case OT_CLIPART:
        {
            if ( oldPen.count() > i )
                dynamic_cast<KPClipartObject*>( kpobject )->setPen( oldPen.at( i )->pen );
            if ( oldBrush.count() > i )
            {
                dynamic_cast<KPClipartObject*>( kpobject )->setBrush( oldBrush.at( i )->brush );
                dynamic_cast<KPClipartObject*>( kpobject )->setFillType( oldBrush.at( i )->fillType );
                dynamic_cast<KPClipartObject*>( kpobject )->setGColor1( oldBrush.at( i )->gColor1 );
                dynamic_cast<KPClipartObject*>( kpobject )->setGColor2( oldBrush.at( i )->gColor2 );
                dynamic_cast<KPClipartObject*>( kpobject )->setGType( oldBrush.at( i )->gType );
            }
            doc->repaint( kpobject );
        } break;
        default: break;
        }
    }
}
