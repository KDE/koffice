/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Reginald Stadlbauer <reggie@kde.org>

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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include <kptextobject.h>
#include <kpgradient.h>
#include <kprcommand.h>

#include <qwidget.h>
#include <qpicture.h>
#include <qpainter.h>
#include <qwmatrix.h>
#include <qdom.h>
#include <qapplication.h>

#include <klocale.h>
#include <kdebug.h>

#include <kpresenter_view.h>
#include <kpresenter_doc.h>

#include <qrichtext_p.h>
#include <kotextobject.h>
#include <kostyle.h>
#include <kotextdocument.h>
#include <kotextformatter.h>
#include <kotextformat.h>
#include <kozoomhandler.h>

#include <qfont.h>

#include "kptextobject.moc"
#include "kprcanvas.h"
#include <koAutoFormat.h>
#include <koparagcounter.h>
#include <kaction.h>
#include <kotextparag.h>
#include <qpopupmenu.h>
#include <koVariable.h>
#include <koVariableDlgs.h>
#include "kprvariable.h"
#include <koRuler.h>
#include "kprdrag.h"
#include <qclipboard.h>

using namespace std;

/******************************************************************/
/* Class: KPTextObject                                            */
/******************************************************************/

const QString &KPTextObject::tagTEXTOBJ=KGlobal::staticQString("TEXTOBJ");
const QString &KPTextObject::attrLineSpacing=KGlobal::staticQString("lineSpacing");
const QString &KPTextObject::attrParagSpacing=KGlobal::staticQString("paragSpacing");
const QString &KPTextObject::attrMargin=KGlobal::staticQString("margin");
const QString &KPTextObject::attrBulletType1=KGlobal::staticQString("bulletType1");
const QString &KPTextObject::attrBulletType2=KGlobal::staticQString("bulletType2");
const QString &KPTextObject::attrBulletType3=KGlobal::staticQString("bulletType3");
const QString &KPTextObject::attrBulletType4=KGlobal::staticQString("bulletType4");
const QString &KPTextObject::attrBulletColor1=KGlobal::staticQString("bulletColor1");
const QString &KPTextObject::attrBulletColor2=KGlobal::staticQString("bulletColor2");
const QString &KPTextObject::attrBulletColor3=KGlobal::staticQString("bulletColor3");
const QString &KPTextObject::attrBulletColor4=KGlobal::staticQString("bulletColor4");
const QString &KPTextObject::attrObjType=KGlobal::staticQString("objType");
const QString &KPTextObject::tagP=KGlobal::staticQString("P");
const QString &KPTextObject::attrAlign=KGlobal::staticQString("align");
const QString &KPTextObject::attrType=KGlobal::staticQString("type");
const QString &KPTextObject::attrDepth=KGlobal::staticQString("depth");
const QString &KPTextObject::tagTEXT=KGlobal::staticQString("TEXT");
const QString &KPTextObject::attrFamily=KGlobal::staticQString("family");
const QString &KPTextObject::attrPointSize=KGlobal::staticQString("pointSize");
const QString &KPTextObject::attrBold=KGlobal::staticQString("bold");
const QString &KPTextObject::attrItalic=KGlobal::staticQString("italic");
const QString &KPTextObject::attrUnderline=KGlobal::staticQString("underline");
const QString &KPTextObject::attrStrikeOut=KGlobal::staticQString("strikeOut");
const QString &KPTextObject::attrColor=KGlobal::staticQString("color");
const QString &KPTextObject::attrWhitespace=KGlobal::staticQString("whitespace");
const QString &KPTextObject::attrTextBackColor=KGlobal::staticQString("textbackcolor");
const QString &KPTextObject::attrVertAlign=KGlobal::staticQString("VERTALIGN");


/*================ default constructor ===========================*/
KPTextObject::KPTextObject(  KPresenterDoc *doc )
    : KP2DObject()
{
    m_doc=doc;

    KPrTextDocument * textdoc = new KPrTextDocument( this ,
                                                     new KoTextFormatCollection( doc->defaultFont() ));

    m_textobj = new KoTextObject( textdoc, doc->standardStyle());

    brush = Qt::NoBrush;
    brush.setColor(QColor());
    pen = QPen( Qt::black, 1, Qt::NoPen );
    drawEditRect = true;
    drawEmpty = true;

    connect( m_textobj, SIGNAL( newCommand( KCommand * ) ),
             SLOT( slotNewCommand( KCommand * ) ) );
    connect( m_textobj, SIGNAL( availableHeightNeeded() ),
             SLOT( slotAvailableHeightNeeded() ) );
    connect( m_textobj, SIGNAL( repaintChanged( KoTextObject* ) ),
             SLOT( slotRepaintChanged() ) );

    // Send our "repaintChanged" signals to the document.
    connect( this, SIGNAL( repaintChanged( KPTextObject * ) ),
             m_doc, SLOT( slotRepaintChanged( KPTextObject * ) ) );
    connect(m_textobj, SIGNAL( showFormatObject(const KoTextFormat &) ),
             SLOT( slotFormatChanged(const KoTextFormat &)) );
}

KPTextObject::~KPTextObject()
{
}

QBrush KPTextObject::getBrush() const
{
    QBrush tmpBrush(brush);
    if(!tmpBrush.color().isValid())
        tmpBrush.setColor(QApplication::palette().color( QPalette::Active, QColorGroup::Base ));
    return tmpBrush;
}

/*======================= set size ===============================*/
void KPTextObject::setSize( double _width, double _height )
{
    KPObject::setSize( _width, _height );

    //kdDebug() << " KPTextObject::setSize -> setting doc width to " << m_doc->zoomHandler()->pixelToLayoutUnitX( _width ) << endl;
    textDocument()->setWidth( m_doc->zoomHandler()->ptToLayoutUnitPixX( _width ) );
    m_textobj->setLastFormattedParag( textDocument()->firstParag() );
    slotAvailableHeightNeeded();
    m_textobj->formatMore();
    if ( fillType == FT_GRADIENT && gradient )
        gradient->setSize( getSize().toQSize() );
}

/*======================= set size ===============================*/
void KPTextObject::resizeBy( double _dx, double _dy )
{
    KPObject::resizeBy( _dx, _dy );
    //kdDebug() << " KPTextObject::resizeBy -> setting doc width to " << m_doc->zoomHandler()->pixelToLayoutUnitX( getSize().width() ) << endl;
    textDocument()->setWidth( m_doc->zoomHandler()->ptToLayoutUnitPixX( getSize().width() ) );
    m_textobj->setLastFormattedParag( textDocument()->firstParag() );
    slotAvailableHeightNeeded();
    m_textobj->formatMore();
    if ( fillType == FT_GRADIENT && gradient )
        gradient->setSize( getSize().toQSize() );
}

/*========================= save =================================*/
QDomDocumentFragment KPTextObject::save( QDomDocument& doc, int offset )
{
    QDomDocumentFragment fragment=KP2DObject::save(doc, offset);
    fragment.appendChild(saveKTextObject( doc ));
    return fragment;
}

/*========================== load ================================*/
int KPTextObject::load(const QDomElement &element)
{
    int offset=KP2DObject::load(element);
    QDomElement e=element.namedItem(tagTEXTOBJ).toElement();
    if(!e.isNull()) {

#if 0
        ktextobject.document()->setLineSpacing( e.attribute( attrLineSpacing ).toInt() );
        ktextobject.document()->setParagSpacing( e.attribute( attrParagSpacing ).toInt() );
        ktextobject.document()->setMargin( e.attribute( attrMargin ).toInt() );
        KTextEditDocument::TextSettings settings = ktextobject.document()->textSettings();
        settings.bulletColor[0] = QColor( e.attribute( attrBulletColor1, Qt::black.name() ) );
        settings.bulletColor[1] = QColor( e.attribute( attrBulletColor2, Qt::black.name() ) );
        settings.bulletColor[2] = QColor( e.attribute( attrBulletColor3, Qt::black.name() ) );
        settings.bulletColor[3] = QColor( e.attribute( attrBulletColor4, Qt::black.name() ) );
        settings.bulletType[0] = (KTextEditDocument::Bullet)e.attribute( attrBulletType1, 0 ).toInt();
        settings.bulletType[1] = (KTextEditDocument::Bullet)e.attribute( attrBulletType2, 0 ).toInt();
        settings.bulletType[2] = (KTextEditDocument::Bullet)e.attribute( attrBulletType3, 0 ).toInt();
        settings.bulletType[3] = (KTextEditDocument::Bullet)e.attribute( attrBulletType4, 0 ).toInt();
        ktextobject.document()->setTextSettings( settings );
#endif
        //  <P ....> .... </P>
        QString type;
        if(e.hasAttribute(attrObjType ))
            type = e.attribute( attrObjType );
        int t = -1;
        if ( !type.isEmpty() )
        {
#if 0
            if ( type == "1" )
                t = KTextEdit::EnumList;
            if ( type == "2" )
                t = KTextEdit::BulletList;
#endif
            if ( type == "1" )
                t = 1;
            if ( type == "2" )
                t = 2;
        }
        loadKTextObject( e, t );
        //loadKTextObject( e, -1 /*TODO*/ );
    }
    setSize( ext.width(), ext.height() ); // this will to formatMore()
    return offset;
}

/*========================= draw =================================*/
void KPTextObject::draw( QPainter *_painter, KoZoomHandler*_zoomHandler, bool drawSelection )
{
    draw( _painter,_zoomHandler, false, 0L, true, drawSelection );
}

void KPTextObject::draw( QPainter *_painter, KoZoomHandler*_zoomHandler,
                         bool onlyChanged, QTextCursor* cursor, bool resetChanged,
                         bool drawSelection )
{
    _painter->save();
#if 0 //FIXME
    setupClipRegion( _painter, getBoundingRect(  ) );
#endif
    double ox = orig.x();// - _diffx;
    double oy = orig.y();// - _diffy;
    //kdDebug() << "Painting text object at " << ox << "," << oy << ":" << m_textobj->textDocument()->text() << endl;
    double ow = ext.width();
    double oh = ext.height();

    _painter->setPen( pen );
    _painter->setBrush( brush );

    // Handle the rotation, draw the background/border, then call drawText()
    int penw = pen.width() / 2;
    _painter->translate( _zoomHandler->zoomItX(ox),_zoomHandler->zoomItY( oy) );
    if ( angle == 0 )
    {
      if ( fillType == FT_BRUSH || !gradient )
        _painter->drawRect( penw, penw, _zoomHandler->zoomItX( ext.width() - 2 * penw), _zoomHandler->zoomItY( ext.height() - 2 * penw) );
      else
        _painter->drawPixmap( penw, penw, *gradient->getGradient(), 0, 0, _zoomHandler->zoomItX( ow - 2 * penw ), _zoomHandler->zoomItY( oh - 2 * penw ) );

      drawText( _painter, _zoomHandler, onlyChanged, cursor, resetChanged );
    }
    else
    {
      KoRect br = KoRect( 0, 0, ow, oh );
      double pw = br.width();
      double ph = br.height();
      KoRect rr = br;
      double yPos = -rr.y();
      double xPos = -rr.x();
      br.moveTopLeft( KoPoint( -br.width() / 2, -br.height() / 2 ) );
      rr.moveTopLeft( KoPoint( -rr.width() / 2, -rr.height() / 2 ) );

      QWMatrix m;
      m.translate( pw / 2, ph / 2 );
      m.rotate( angle );

      _painter->setWorldMatrix( m, true );


      if ( fillType == FT_BRUSH || !gradient )
        _painter->drawRect( _zoomHandler->zoomItX(rr.left() + xPos + penw), _zoomHandler->zoomItY(rr.top() + yPos + penw), _zoomHandler->zoomItX(ext.width() - 2 * penw), _zoomHandler->zoomItY(ext.height() - 2 * penw) );
      else
        _painter->drawPixmap( _zoomHandler->zoomItX( rr.left() + xPos + penw ), _zoomHandler->zoomItY( rr.top() + yPos + penw ), *gradient->getGradient(), 0, 0, _zoomHandler->zoomItX( ow - 2 * penw ), _zoomHandler->zoomItY( oh - 2 * penw ) );

      _painter->translate( _zoomHandler->zoomItX(rr.left() + xPos), _zoomHandler->zoomItY( rr.top() + yPos) );
      drawText( _painter, _zoomHandler, onlyChanged, cursor, resetChanged );
    }
    _painter->restore();

    KPObject::draw( _painter, _zoomHandler, drawSelection );
}

// This method simply draws the paragraphs in the given painter
// Assumes the painter is already set up correctly.
void KPTextObject::drawText( QPainter* _painter, KoZoomHandler *zoomHandler, bool onlyChanged, QTextCursor* cursor, bool resetChanged )
{
    //kdDebug() << "KPTextObject::drawText onlyChanged=" << onlyChanged << " cursor=" << cursor << " resetChanged=" << resetChanged << endl;
    QColorGroup cg = QApplication::palette().active();
    //// ### Transparent background - TODO use configuration ?
    cg.setBrush( QColorGroup::Base, NoBrush );
    QRect r = zoomHandler->zoomRect( KoRect( 0, 0, ext.width(), ext.height() ) );

    if ( specEffects )
    {
        switch ( effect2 )
        {
        case EF2T_PARA:
            kdDebug(33001) << "KPTextObject::draw onlyCurrStep=" << onlyCurrStep << " subPresStep=" << subPresStep << endl;
            drawParags( _painter, zoomHandler, cg, ( onlyCurrStep ? subPresStep : 0 ), subPresStep );
            break;
        default:
            /*Qt3::QTextParag * lastFormatted =*/ textDocument()->drawWYSIWYG(
                _painter, r.x(), r.y(), r.width(), r.height(),
                cg, zoomHandler,
                onlyChanged, cursor != 0, cursor, resetChanged );
        }
    }
    else
    {
        //kdDebug() << "KPTextObject::drawText r=" << DEBUGRECT(r) << endl;
        /*Qt3::QTextParag * lastFormatted = */ textDocument()->drawWYSIWYG(
            _painter, r.x(), r.y(), r.width(), r.height(),
            cg, zoomHandler,
            onlyChanged, cursor != 0, cursor, resetChanged );
    }
}

int KPTextObject::getSubPresSteps() const
{
    int paragraphs = 0;
    Qt3::QTextParag * parag = m_textobj->textDocument()->firstParag();
    for ( ; parag ; parag = parag->next() )
        paragraphs++;
    return paragraphs;
}

/*================================================================*/
void KPTextObject::extendObject2Contents( KPresenterView */*view*/ )
{
#if 0
    QSize s( ktextobject.neededSize() );
    setSize( s.width(), s.height() );
#endif
}

/*=========================== save ktextobject ===================*/
QDomElement KPTextObject::saveKTextObject( QDomDocument& doc )
{
#if 0
    KTextEditParag *parag = ktextobject.document()->firstParag();
    KTextEditDocument::TextSettings textSettings = ktextobject.document()->textSettings();
#endif

    QDomElement textobj=doc.createElement(tagTEXTOBJ);
#if 0
    textobj.setAttribute(attrLineSpacing, ktextobject.document()->lineSpacing());
    textobj.setAttribute(attrParagSpacing, ktextobject.document()->paragSpacing());
    textobj.setAttribute(attrMargin, ktextobject.document()->margin());
    textobj.setAttribute(attrBulletType1, (int)textSettings.bulletType[0]);
    textobj.setAttribute(attrBulletType2, (int)textSettings.bulletType[1]);
    textobj.setAttribute(attrBulletType3, (int)textSettings.bulletType[2]);
    textobj.setAttribute(attrBulletType4, (int)textSettings.bulletType[3]);
    textobj.setAttribute(attrBulletColor1, textSettings.bulletColor[0].name());
    textobj.setAttribute(attrBulletColor2, textSettings.bulletColor[1].name());
    textobj.setAttribute(attrBulletColor3, textSettings.bulletColor[2].name());
    textobj.setAttribute(attrBulletColor4, textSettings.bulletColor[3].name());
#endif
    KoTextParag *parag = static_cast<KoTextParag*> (textDocument()->firstParag());
    // ### fix this loop (Werner)
    while ( parag ) {
        saveParagraph( doc, parag, textobj, 0, parag->length()-2 );
        parag = static_cast<KoTextParag*>( parag->next());
    }
    return textobj;
}

QDomElement KPTextObject::saveHelper(const QString &tmpText,KoTextFormat*lastFormat , QDomDocument &doc)
{

    QDomElement element=doc.createElement(tagTEXT);
    QString tmpFamily, tmpColor, tmpTextBackColor;
    int tmpPointSize=10;
    unsigned int tmpBold=false, tmpItalic=false, tmpUnderline=false,tmpStrikeOut=false;
    int tmpVerticalAlign=-1;

    tmpFamily=lastFormat->font().family();
    tmpPointSize=static_cast<int>(KoZoomHandler::layoutUnitPtToPt( lastFormat->font().pointSize()));
    tmpBold=static_cast<unsigned int>(lastFormat->font().bold());
    tmpItalic=static_cast<unsigned int>(lastFormat->font().italic());
    tmpUnderline=static_cast<unsigned int>(lastFormat->font().underline());
    tmpStrikeOut=static_cast<unsigned int>(lastFormat->font().strikeOut());
    tmpColor=lastFormat->color().name();
    tmpVerticalAlign=static_cast<unsigned int>(lastFormat->vAlign());
    if(lastFormat->textBackgroundColor().isValid())
        tmpTextBackColor=lastFormat->textBackgroundColor().name();

    element.setAttribute(attrFamily, tmpFamily);
    element.setAttribute(attrPointSize, tmpPointSize);

    if(tmpBold)
        element.setAttribute(attrBold, tmpBold);
    if(tmpItalic)
        element.setAttribute(attrItalic, tmpItalic);
    if(tmpUnderline)
        element.setAttribute(attrUnderline, tmpUnderline);
    if(tmpStrikeOut)
        element.setAttribute(attrStrikeOut, tmpStrikeOut);
    element.setAttribute(attrColor, tmpColor);

    if(!tmpTextBackColor.isEmpty())
        element.setAttribute(attrTextBackColor, tmpTextBackColor);
    if(tmpVerticalAlign!=-1)
        element.setAttribute(attrVertAlign,tmpVerticalAlign);


    if(tmpText.stripWhiteSpace().isEmpty())
        // working around a bug in QDom
        element.setAttribute(attrWhitespace, tmpText.length());
    element.appendChild(doc.createTextNode(tmpText));
    return element;
}

/*====================== load ktextobject ========================*/
void KPTextObject::loadKTextObject( const QDomElement &elem, int type )
{
    QDomElement e = elem.firstChild().toElement();
    KoTextParag *lastParag = static_cast<KoTextParag *>(textDocument()->firstParag());
    int i = 0;
    int listNum = 0;
    int lineSpacing = 0, paragSpacing = 0;
    while ( !e.isNull() ) {
        if ( e.tagName() == tagP ) {
            QDomElement n = e.firstChild().toElement();

#if 0
            if ( type != -1 )
                lastParag->setType( (KTextEditParag::Type)type );
            else
                lastParag->setType( (KTextEditParag::Type)e.attribute( attrType ).toInt() );
#endif
            KoParagLayout paragLayout = loadParagLayout(e);
            //compatibility
            if(type!=-1)
            {
                if(!paragLayout.counter)
                {
                    paragLayout.counter = new KoParagCounter;
                }
                paragLayout.counter->setNumbering(KoParagCounter::NUM_LIST);
                if ( type == 1 )
                {
                    //t = KTextEdit::EnumList;
                    paragLayout.counter->setStyle(KoParagCounter::STYLE_NUM);
                }
                if ( type == 2 )
                {
                    //t = KTextEdit::BulletList;
                    paragLayout.counter->setStyle(KoParagCounter::STYLE_DISCBULLET);
                }
            }
            lastParag->setParagLayout( paragLayout );


            if(e.hasAttribute(attrAlign))
            {
                int tmpAlign=e.attribute( attrAlign ).toInt();
                if(tmpAlign==1)
                    lastParag->setAlignment(Qt::AlignLeft);
                else if(tmpAlign==2)
                    lastParag->setAlignment(Qt::AlignRight);
                else if(tmpAlign==4)
                    lastParag->setAlignment(Qt::AlignCenter);
                else if(tmpAlign==8)
                    lastParag->setAlignment(Qt::AlignJustify);
                else
                    kdDebug()<<"Error in e.attribute( attrAlign ).toInt()\n";
            }
            // ## lastParag->setListDepth( e.attribute( attrDepth ).toInt() );
            // TODO check/convert values
            lineSpacing = QMAX( e.attribute( attrLineSpacing ).toInt(), lineSpacing );
            paragSpacing = QMAX( QMAX( e.attribute( "distBefore" ).toInt(), e.attribute( "distAfter" ).toInt() ), paragSpacing );
            bool firstTextTag = true;
            while ( !n.isNull() ) {
                if ( n.tagName() == tagTEXT ) {
                    if ( firstTextTag ) {
                        lastParag->remove( 0, 1 ); // Remove current trailing space
                        firstTextTag = false;
                    }
                    KoTextFormat fm = loadFormat( n );

                    QString txt = n.firstChild().toText().data();
                    if(n.hasAttribute(attrWhitespace)) {
                        int ws=n.attribute(attrWhitespace).toInt();
                        txt.fill(' ', ws);
                    }
                    n=n.nextSibling().toElement();
                    if ( txt.isEmpty() )
                        txt = ' ';
                    if ( ( !txt[txt.length()-1].isSpace()  && n.isNull() ) )
                        txt += ' '; // trailing space at end of paragraph
                    lastParag->append( txt, true );
                    lastParag->setFormat( i, txt.length(), textDocument()->formatCollection()->format( &fm ) );
                    //kdDebug()<<"setFormat :"<<txt<<" i :"<<i<<" txt.length() "<<txt.length()<<endl;
                    i += txt.length();
                }
                else
                    n = n.nextSibling().toElement();
            }
        } else if ( e.tagName() == "UNSORTEDLISTTYPE" ) {
            if ( listNum < 4 ) {
                QColor c( e.attribute( "red" ).toInt(), e.attribute( "green" ).toInt(), e.attribute( "blue" ).toInt() );
                // ## settings.bulletColor[ listNum++ ] = c;
            }
        }
        e = e.nextSibling().toElement();
        if ( e.isNull() )
            break;
        i = 0;
        if ( !lastParag->length() == 0 )
            lastParag = new KoTextParag( textDocument(), lastParag, 0 );
    }

#if 0
    settings.lineSpacing = lineSpacing;
    settings.paragSpacing = QMAX( ktextobject.document()->paragSpacing(), paragSpacing );
    ktextobject.document()->setTextSettings( settings );
    ktextobject.updateCurrentFormat();
#endif
}

KoTextFormat KPTextObject::loadFormat( QDomElement &n )
{
    KoTextFormat format;
    QString family = n.attribute( attrFamily );
    int size = n.attribute( attrPointSize ).toInt();
    bool bold=false;
    if(n.hasAttribute(attrBold))
        bold = (bool)n.attribute( attrBold ).toInt();
    bool italic = false;
    if(n.hasAttribute(attrItalic))
        italic=(bool)n.attribute( attrItalic ).toInt();
    bool underline=false;
    if(n.hasAttribute( attrUnderline ))
        underline = (bool)n.attribute( attrUnderline ).toInt();
    bool strikeOut=false;
    if(n.hasAttribute(attrStrikeOut))
        strikeOut = (bool)n.attribute( attrStrikeOut ).toInt();

    QString color = n.attribute( attrColor );
    QFont fn( family );
    fn.setPointSize( KoTextZoomHandler::ptToLayoutUnitPt( size ) );
    fn.setBold( bold );
    fn.setItalic( italic );
    fn.setUnderline( underline );
    fn.setStrikeOut( strikeOut );
    //kdDebug() << "KPTextObject::loadFormat: family=" << family << " size=" << fn.pointSize() << endl;
    QColor col( color );

    format.setFont( fn );
    format.setColor( col );
    QString textBackColor=n.attribute(attrTextBackColor);
    if(!textBackColor.isEmpty())
    {
        QColor tmpCol(textBackColor);
        tmpCol=tmpCol.isValid() ? tmpCol : QApplication::palette().color( QPalette::Active, QColorGroup::Base );
        format.setTextBackgroundColor(tmpCol);
    }
    //TODO FIXME : value is correct, but format is not good :(
    if(n.hasAttribute(attrVertAlign))
        format.setVAlign( static_cast<QTextFormat::VerticalAlignment>(n.attribute(attrVertAlign).toInt() ) );


    //kdDebug()<<"loadFormat :"<<format.key()<<endl;
    return format;
}

KoParagLayout KPTextObject::loadParagLayout( QDomElement & parentElem)
{
    KoParagLayout layout;

    QDomElement element = parentElem.namedItem( "INDENTS" ).toElement();
    if ( !element.isNull() )
    {
        double val=0.0;
        if(element.hasAttribute("first"))
            val=element.attribute("first").toDouble();
        layout.margins[QStyleSheetItem::MarginFirstLine] = val;
        val=0.0;
        if(element.hasAttribute( "left"))
            val=element.attribute( "left").toDouble();
        layout.margins[QStyleSheetItem::MarginLeft] = val;
        val=0.0;
        if(element.hasAttribute("right"))
            val=element.attribute("right").toDouble();
        layout.margins[QStyleSheetItem::MarginRight] = val;
    }
    element = parentElem.namedItem( "LINESPACING" ).toElement();
    if ( !element.isNull() )
    {
        QString value = element.attribute( "value" );
        if ( value == "oneandhalf" )
            layout.lineSpacing = KoParagLayout::LS_ONEANDHALF;
        else if ( value == "double" )
            layout.lineSpacing = KoParagLayout::LS_DOUBLE;
        else
            layout.lineSpacing = value.toDouble();
    }

    element = parentElem.namedItem( "OFFSETS" ).toElement();
    if ( !element.isNull() )
    {
        double val =0.0;
        if(element.hasAttribute("before"))
            val=element.attribute("before").toDouble();
        layout.margins[QStyleSheetItem::MarginTop] = val;
        val = 0.0;
        if(element.hasAttribute("after"))
            val=element.attribute("after").toDouble();
        layout.margins[QStyleSheetItem::MarginBottom] = val;
    }



    element = parentElem.namedItem( "LEFTBORDER" ).toElement();
    if ( !element.isNull() )
        layout.leftBorder = KoBorder::loadBorder( element );
    else
        layout.leftBorder.ptWidth = 0;

    element = parentElem.namedItem( "RIGHTBORDER" ).toElement();
    if ( !element.isNull() )
        layout.rightBorder = KoBorder::loadBorder( element );
    else
        layout.rightBorder.ptWidth = 0;

    element = parentElem.namedItem( "TOPBORDER" ).toElement();
    if ( !element.isNull() )
        layout.topBorder = KoBorder::loadBorder( element );
    else
        layout.topBorder.ptWidth = 0;

    element = parentElem.namedItem( "BOTTOMBORDER" ).toElement();
    if ( !element.isNull() )
        layout.bottomBorder = KoBorder::loadBorder( element );
    else
        layout.bottomBorder.ptWidth = 0;

    element = parentElem.namedItem( "COUNTER" ).toElement();
    if ( !element.isNull() )
    {
        layout.counter = new KoParagCounter;
        layout.counter->load( element );
    }

    KoTabulatorList tabList;
    element = parentElem.firstChild().toElement();
    for ( ; !element.isNull() ; element = element.nextSibling().toElement() )
    {
        if ( element.tagName() == "TABULATOR" )
        {
            KoTabulator tab;
            tab.type=T_LEFT;
            if(element.hasAttribute("type"))
                tab.type = static_cast<KoTabulators>( element.attribute("type").toInt());
            tab.ptPos=0.0;
            if(element.hasAttribute("ptpos"))
                tab.ptPos=element.attribute("ptpos").toDouble();
            tabList.append( tab );
        }
    }
    layout.setTabList( tabList );


    element = parentElem.namedItem( "SHADOW" ).toElement();
    if ( !element.isNull() )
    {
        layout.shadowDistance=element.attribute("distance").toInt();
        layout.shadowDirection=element.attribute("direction").toInt();
        if ( element.hasAttribute("red") )
        {
            int r = element.attribute("red").toInt();
            int g = element.attribute("green").toInt();
            int b = element.attribute("blue").toInt();
            layout.shadowColor.setRgb( r, g, b );
        }
    }

    return layout;
}

void KPTextObject::saveParagLayout( const KoParagLayout& layout, QDomElement & parentElem )
{
    QDomDocument doc = parentElem.ownerDocument();
    QDomElement element;
    if ( layout.margins[QStyleSheetItem::MarginFirstLine] != 0 ||
         layout.margins[QStyleSheetItem::MarginLeft] != 0 ||
         layout.margins[QStyleSheetItem::MarginRight] != 0 )
    {
        element = doc.createElement( "INDENTS" );
        parentElem.appendChild( element );
        if ( layout.margins[QStyleSheetItem::MarginFirstLine] != 0 )
            element.setAttribute( "first", layout.margins[QStyleSheetItem::MarginFirstLine] );
        if ( layout.margins[QStyleSheetItem::MarginLeft] != 0 )
            element.setAttribute( "left", layout.margins[QStyleSheetItem::MarginLeft] );
        if ( layout.margins[QStyleSheetItem::MarginRight] != 0 )
            element.setAttribute( "right", layout.margins[QStyleSheetItem::MarginRight] );
    }


    if ( layout.margins[QStyleSheetItem::MarginTop] != 0 ||
         layout.margins[QStyleSheetItem::MarginBottom] != 0 )
    {
        element = doc.createElement( "OFFSETS" );
        parentElem.appendChild( element );
        if ( layout.margins[QStyleSheetItem::MarginTop] != 0 )
            element.setAttribute( "before", layout.margins[QStyleSheetItem::MarginTop] );
        if ( layout.margins[QStyleSheetItem::MarginBottom] != 0 )
            element.setAttribute( "after", layout.margins[QStyleSheetItem::MarginBottom] );
    }

    if ( layout.lineSpacing != 0 )
    {
        element = doc.createElement( "LINESPACING" );
        parentElem.appendChild( element );
        if ( layout.lineSpacing == KoParagLayout::LS_ONEANDHALF )
            element.setAttribute( "value", "oneandhalf" );
        else if ( layout.lineSpacing == KoParagLayout::LS_DOUBLE )
            element.setAttribute( "value", "double" );
        else
            element.setAttribute( "value", layout.lineSpacing );
    }

    if ( layout.leftBorder.ptWidth > 0 )
    {
        element = doc.createElement( "LEFTBORDER" );
        parentElem.appendChild( element );
        layout.leftBorder.save( element );
    }
    if ( layout.rightBorder.ptWidth > 0 )
    {
        element = doc.createElement( "RIGHTBORDER" );
        parentElem.appendChild( element );
        layout.rightBorder.save( element );
    }
    if ( layout.topBorder.ptWidth > 0 )
    {
        element = doc.createElement( "TOPBORDER" );
        parentElem.appendChild( element );
        layout.topBorder.save( element );
    }
    if ( layout.bottomBorder.ptWidth > 0 )
    {
        element = doc.createElement( "BOTTOMBORDER" );
        parentElem.appendChild( element );
        layout.bottomBorder.save( element );
    }

    if ( layout.counter && layout.counter->numbering() != KoParagCounter::NUM_NONE )
    {
        element = doc.createElement( "COUNTER" );
        parentElem.appendChild( element );
        if (layout.counter )
            layout.counter->save( element );
    }

    KoTabulatorList tabList = layout.tabList();
    KoTabulatorList::ConstIterator it = tabList.begin();
    for ( ; it != tabList.end() ; it++ )
    {
        element = doc.createElement( "TABULATOR" );
        parentElem.appendChild( element );
        element.setAttribute( "type", (*it).type );
        element.setAttribute( "ptpos", (*it).ptPos );
    }

    if(layout.shadowDistance!=0 || layout.shadowDirection!=KoParagLayout::SD_RIGHT_BOTTOM)
    {
        element = doc.createElement( "SHADOW" );
        parentElem.appendChild( element );
        element.setAttribute( "distance", layout.shadowDistance );
        element.setAttribute( "direction", layout.shadowDirection );
        if (layout.shadowColor.isValid())
        {
            element.setAttribute("red", layout.shadowColor.red());
            element.setAttribute("green", layout.shadowColor.green());
            element.setAttribute("blue", layout.shadowColor.blue());
        }
    }
}

/*================================================================*/
void KPTextObject::recalcPageNum( KPresenterDoc *doc )
{
    int h = doc->getPageRect( ).height();
    int pgnum = -1;
    for ( unsigned int i = 0; i < doc->getPageNums(); ++i ) {
        if ( (int)orig.y() <= ( (int)i + 1 ) * h ) {
            pgnum = i + 1;
            break;
        }
    }

    if ( pgnum == -1 )
        pgnum = doc->getPageNums();

    QPtrListIterator<Qt3::QTextCustomItem> cit( textDocument()->allCustomItems() );
    for ( ; cit.current() ; ++cit )
    {
        KPrPgNumVariable * var = dynamic_cast<KPrPgNumVariable *>( cit.current() );
        if ( var && !var->isDeleted() && var->subtype() == KPrPgNumVariable::VST_PGNUM_CURRENT )
        {
            var->setPgNum( pgnum + kPresenterDocument()->getVariableCollection()->variableSetting()->startingPage()-1);
            var->resize();
            var->paragraph()->invalidate( 0 ); // size may have changed -> need reformatting !
            var->paragraph()->setChanged( true );
        }
    }


#if 0
    ktextobject.setPageNum( pgnum );
#endif
}

void KPTextObject::drawParags( QPainter *painter, KoZoomHandler* zoomHandler, const QColorGroup& cg, int from, int to )
{
    // The fast and difficult way would be to call drawParagWYSIWYG
    // only on the paragraphs to be drawn. Then we have duplicate quite some code
    // (or lose double-buffering).
    // Easy (and not so slow) way:
    // we call KoTextDocument::drawWYSIWYG with a cliprect.
    Q_ASSERT( from <= to );
    int i = 0;
    QRect r = zoomHandler->zoomRect( KoRect( 0, 0, ext.width(), ext.height() ) );
    Qt3::QTextParag *parag = textDocument()->firstParag();
    while ( parag ) {
        if ( !parag->isValid() )
            parag->format();
        if ( i == from ) {
            r.setTop( m_doc->zoomHandler()->layoutUnitToPixelY( parag->rect().top() ) );
        }
        if ( i == to ) {
            r.setBottom( m_doc->zoomHandler()->layoutUnitToPixelY( parag->rect().bottom() ) );
            break;
        }
        ++i;
        parag = parag->next();
    }

    textDocument()->drawWYSIWYG(
        painter, r.x(), r.y(), r.width(), r.height(),
        cg, m_doc->zoomHandler(), // TODO (long term) the view's zoomHandler
        false /*onlyChanged*/, false /*cursor != 0*/, 0 /*cursor*/ /*, resetChanged*/ );
}

void KPTextObject::drawCursor( QPainter *p, QTextCursor *cursor, bool cursorVisible, KPrCanvas* canvas )
{
    KoZoomHandler *zh = m_doc->zoomHandler();
    QPoint origPix = zh->zoomPoint( orig );
    // Painter is already translated for diffx/diffy, but not for the object yet
    p->translate( origPix.x(), origPix.y() );
    KoTextParag* parag = static_cast<KoTextParag *>(cursor->parag());

    QPoint topLeft = cursor->topParag()->rect().topLeft();         // in QRT coords
    int lineY;
    // Cursor height, in pixels
    int cursorHeight = zh->layoutUnitToPixelY( topLeft.y(), parag->lineHeightOfChar( cursor->index(), 0, &lineY ) );
    QPoint iPoint( topLeft.x() - cursor->totalOffsetX() + cursor->x(),
                   topLeft.y() - cursor->totalOffsetY() + lineY );
    iPoint = zh->layoutUnitToPixel( iPoint );

    QPoint vPoint = iPoint + origPix;
    int xadj = parag->at( cursor->index() )->pixelxadj;
    iPoint.rx() += xadj;
    vPoint.rx() += xadj;
    // very small clipping around the cursor
    QRect clip( vPoint.x() - 5, vPoint.y() - canvas->diffy(), 10, cursorHeight );
    setupClipRegion( p, clip );

    QPixmap *pix = 0;
    QColorGroup cg = QApplication::palette().active();

    bool wasChanged = parag->hasChanged();
    parag->setChanged( TRUE );      // To force the drawing to happen
    textDocument()->drawParagWYSIWYG(
        p, parag,
        iPoint.x() - 5, iPoint.y(), clip.width(), clip.height(),
        pix, cg, m_doc->zoomHandler(),
        cursorVisible, cursor, false /*m_doc->viewFormattingChars()*/ );
    parag->setChanged( wasChanged );      // Maybe we have more changes to draw!

    //XIM Position
    QPoint ximPoint = vPoint;
    QFont f = parag->at( cursor->index() )->format()->font();
    int line;
    parag->lineStartOfChar( cursor->index(), 0, &line );
    m_doc->getKPresenterView()->getCanvas()->setXimPosition( ximPoint.x(), ximPoint.y(),
                                                           0, cursorHeight - parag->lineSpacing( line ),
                                                           &f );
}

KPrTextDocument * KPTextObject::textDocument() const
{
    return static_cast<KPrTextDocument*>(m_textobj->textDocument());
}

void KPTextObject::slotNewCommand( KCommand * cmd)
{
    m_doc->addCommand(cmd);
}

void KPTextObject::slotAvailableHeightNeeded()
{
    int ah = m_doc->zoomHandler()->ptToLayoutUnitPixY( getSize().height() );
    m_textobj->setAvailableHeight( ah );
    //kdDebug()<<"slotAvailableHeightNeeded: height=:"<<ah<<endl;
}

void KPTextObject::slotRepaintChanged()
{
    emit repaintChanged( this );
}

KPTextView * KPTextObject::createKPTextView( KPrCanvas * _canvas )
{
    return new KPTextView( this, _canvas );
}


void KPTextObject::removeHighlight ()
{
    m_textobj->removeHighlight();
}

void KPTextObject::highlightPortion( Qt3::QTextParag * parag, int index, int length, KPrCanvas*/*_canvas*/ )
{
    m_textobj->highlightPortion( parag, index, length );
#if 0
    QRect expose = canvas->viewMode()->normalToView( paragRect( parag ) );
    canvas->ensureVisible( (expose.left()+expose.right()) / 2,  // point = center of the rect
                           (expose.top()+expose.bottom()) / 2,
                           (expose.right()-expose.left()) / 2,  // margin = half-width of the rect
                           (expose.bottom()-expose.top()) / 2);
#endif
}

KCommand * KPTextObject::pasteKPresenter( QTextCursor * cursor, const QCString & data, bool removeSelected )
{
    // Having data as a QCString instead of a QByteArray seems to fix the trailing 0 problem
    // I tried using QDomDocument::setContent( QByteArray ) but that leads to parse error at the end

    //kdDebug(32001) << "KWTextFrameSet::pasteKWord" << endl;
    KMacroCommand * macroCmd = new KMacroCommand( i18n("Paste Text") );
    if ( removeSelected && textDocument()->hasSelection( KoTextDocument::Standard ) )
        macroCmd->addCommand( m_textobj->removeSelectedTextCommand( cursor, KoTextDocument::Standard ) );
    m_textobj->emitHideCursor();
    m_textobj->setLastFormattedParag( cursor->parag()->prev() ?
                           cursor->parag()->prev() : cursor->parag() );

    // We have our own command for this.
    // Using insert() wouldn't help storing the parag stuff for redo
    KPrPasteTextCommand * cmd = new KPrPasteTextCommand( textDocument(), cursor->parag()->paragId(), cursor->index(), data );
    textDocument()->addCommand( cmd );

    macroCmd->addCommand( new KoTextCommand( m_textobj, /*cmd, */QString::null ) );
    *cursor = *( cmd->execute( cursor ) );

    m_textobj->formatMore();
    emit repaintChanged( this );
    m_textobj->emitEnsureCursorVisible();
    m_textobj->emitUpdateUI( true );
    m_textobj->emitShowCursor();
    m_textobj->selectionChangedNotify();
    return macroCmd;
}


void KPTextObject::setShadowParameter(int _distance,ShadowDirection _direction,QColor _color)
{
    //todo apply to all parag
    shadowDistance = _distance;
    shadowDirection = _direction;
    shadowColor = _color;
    Qt3::QTextParag *parag = textDocument()->firstParag();
    while ( parag ) {
        // The double->int conversion for shadowDistance assumes pt=pixel. Bah.
        static_cast<KoTextParag *>(parag)->setShadow( (int)shadowDistance, shadowDirection, shadowColor );
        parag = parag->next();
    }
}

void KPTextObject::slotFormatChanged(const KoTextFormat &_format)
{
    m_doc->getKPresenterView()->showFormat( _format );
}

KPTextView::KPTextView( KPTextObject * txtObj,KPrCanvas *_canvas )
    : KoTextView( txtObj->textObject() )
{
    m_canvas=_canvas;
    m_kptextobj=txtObj;
    connect( txtObj->textObject(), SIGNAL( selectionChanged(bool) ), m_canvas, SIGNAL( selectionChanged(bool) ) );
    KoTextView::setReadWrite( txtObj->kPresenterDocument()->isReadWrite() );
    connect( textView(), SIGNAL( cut() ), SLOT( cut() ) );
    connect( textView(), SIGNAL( copy() ), SLOT( copy() ) );
    connect( textView(), SIGNAL( paste() ), SLOT( paste() ) );
    updateUI( true, true );
#if 0
    if( m_canvas->getView() && m_canvas->getView()->getHRuler())
    {
        m_canvas->getView()->getHRuler()->changeFlags(KoRuler::F_INDENTS | KoRuler::F_TABS);
        m_canvas->getView()->getHRuler()->repaint();
    }
#endif

}

KPTextView::~KPTextView()
{
#if 0
    if( m_canvas->getView() && m_canvas->getView()->getHRuler())
    {
        m_canvas->getView()->getHRuler()->changeFlags(0);
        m_canvas->getView()->getHRuler()->repaint();
    }
#endif
}

void KPTextView::terminate()
{
    disconnect( textView()->textObject(), SIGNAL( selectionChanged(bool) ), m_canvas, SIGNAL( selectionChanged(bool) ) );
    textView()->terminate();
}

void KPTextView::cut()
{
    if ( textDocument()->hasSelection( KoTextDocument::Standard ) ) {
        copy();
        textObject()->removeSelectedText( cursor() );
    }
}

void KPTextView::copy()
{
    if ( textDocument()->hasSelection( KoTextDocument::Standard ) ) {
        KPrTextDrag *kd = newDrag( 0L );
        QApplication::clipboard()->setData( kd );
    }
}

void KPTextView::paste()
{
    QMimeSource *data = QApplication::clipboard()->data();
    if ( data->provides( KPrTextDrag::selectionMimeType() ) )
    {
        QByteArray arr = data->encodedData( KPrTextDrag::selectionMimeType() );
        if ( arr.size() )
        {
            //kdDebug()<<"QCString( arr ) :"<<QCString( arr )<<endl;
            kpTextObject()->kPresenterDocument()->addCommand(kpTextObject()->pasteKPresenter( cursor(), QCString( arr ), true ));
        }
    }
    else
    {
        // Note: QClipboard::text() seems to do a better job than encodedData( "text/plain" )
        // In particular it handles charsets (in the mimetype).
        QString text = QApplication::clipboard()->text();
        if ( !text.isEmpty() )
            textObject()->pasteText( cursor(), text, currentFormat(), true );
    }
}

void KPTextView::updateUI( bool updateFormat, bool force  )
{
    KoTextView::updateUI( updateFormat, force  );
    // Paragraph settings
    KoTextParag * parag = static_cast<KoTextParag*>( cursor()->parag());
    if ( m_paragLayout.alignment != parag->alignment() || force ) {
        m_paragLayout.alignment = parag->alignment();
        m_canvas->getView()->alignChanged(  m_paragLayout.alignment );
    }

    // Counter
    if ( !m_paragLayout.counter )
        m_paragLayout.counter = new KoParagCounter; // we can afford to always have one here
    KoParagCounter::Style cstyle = m_paragLayout.counter->style();
    if ( parag->counter() )
        *m_paragLayout.counter = *parag->counter();
    else
    {
        m_paragLayout.counter->setNumbering( KoParagCounter::NUM_NONE );
        m_paragLayout.counter->setStyle( KoParagCounter::STYLE_NONE );
    }
    if ( m_paragLayout.counter->style() != cstyle || force )
    {
        m_canvas->getView()->showCounter( * m_paragLayout.counter );
    }
    if(m_paragLayout.leftBorder!=parag->leftBorder() ||
       m_paragLayout.rightBorder!=parag->rightBorder() ||
       m_paragLayout.topBorder!=parag->topBorder() ||
       m_paragLayout.bottomBorder!=parag->bottomBorder() || force )
    {
        m_paragLayout.leftBorder = parag->leftBorder();
        m_paragLayout.rightBorder = parag->rightBorder();
        m_paragLayout.topBorder = parag->topBorder();
        m_paragLayout.bottomBorder = parag->bottomBorder();
        //todo
        //m_canvas->gui()->getView()->showParagBorders( m_paragLayout.leftBorder, m_paragLayout.rightBorder, m_paragLayout.topBorder, m_paragLayout.bottomBorder );
    }

    if ( !parag->style() )
        kdWarning() << "Paragraph " << parag->paragId() << " has no style" << endl;
    else if ( m_paragLayout.style != parag->style() || force )
    {
        m_paragLayout.style = parag->style();
        //todo
        //m_canvas->gui()->getView()->showStyle( m_paragLayout.style->name() );
    }

    if( m_paragLayout.margins[QStyleSheetItem::MarginLeft] != parag->margin(QStyleSheetItem::MarginLeft)
        || m_paragLayout.margins[QStyleSheetItem::MarginFirstLine] != parag->margin(QStyleSheetItem::MarginFirstLine)
        || m_paragLayout.margins[QStyleSheetItem::MarginRight] != parag->margin(QStyleSheetItem::MarginRight)
        || force )
    {
        m_paragLayout.margins[QStyleSheetItem::MarginFirstLine] = parag->margin(QStyleSheetItem::MarginFirstLine);
        m_paragLayout.margins[QStyleSheetItem::MarginLeft] = parag->margin(QStyleSheetItem::MarginLeft);
        m_paragLayout.margins[QStyleSheetItem::MarginRight] = parag->margin(QStyleSheetItem::MarginRight);
        m_canvas->getView()->showRulerIndent( m_paragLayout.margins[QStyleSheetItem::MarginLeft], m_paragLayout.margins[QStyleSheetItem::MarginFirstLine], m_paragLayout.margins[QStyleSheetItem::MarginRight] );
    }
    m_paragLayout.setTabList( parag->tabList() );
    KoRuler * hr = m_canvas->getView()->getHRuler();
    if ( hr )
        hr->setTabList( parag->tabList() );
}

void KPTextView::ensureCursorVisible()
{
    kdDebug()<<"KPTextView::ensureCursorVisible() : not implemented\n";
}

void KPTextView::doAutoFormat( QTextCursor* cursor, KoTextParag *parag, int index, QChar ch )
{
    KoAutoFormat * autoFormat = m_kptextobj->kPresenterDocument()->getAutoFormat();
    if ( autoFormat )
        autoFormat->doAutoFormat( cursor, parag, index, ch, textObject());
}

void KPTextView::startDrag()
{
    textView()->dragStarted();
    m_canvas->dragStarted();
    KPrTextDrag *drag = newDrag( m_canvas );
    if ( !kpTextObject()->kPresenterDocument()->isReadWrite() )
        drag->dragCopy();
    else {
        if ( drag->drag() && QDragObject::target() != m_canvas  ) {
            textObject()->removeSelectedText( cursor() );
        }
    }
}


void KPTextView::showFormat( KoTextFormat *format )
{
    m_canvas->getView()->showFormat( *format );
}

void KPTextView::pgUpKeyPressed()
{
    QTextCursor *cursor = textView()->cursor();
    Qt3::QTextParag *s = cursor->parag();
    s = textDocument()->firstParag();

    textView()->cursor()->setParag( s );
    textView()->cursor()->setIndex( 0 );
}

void KPTextView::pgDownKeyPressed()
{
    QTextCursor *cursor = textView()->cursor();
    Qt3::QTextParag *s = cursor->parag();
    s = textDocument()->lastParag();
    cursor->setParag( s );
    cursor->setIndex( s->length() - 1 );
}

void KPTextView::keyPressEvent( QKeyEvent *e )
{
    handleKeyPressEvent(e);
}

void KPTextView::keyReleaseEvent( QKeyEvent *e )
{
    handleKeyReleaseEvent(e);
}

void KPTextView::clearSelection()
{
    if ( textDocument()->hasSelection( KoTextDocument::Standard ) )
    {
        textDocument()->removeSelection(KoTextDocument::Standard );
    }
}

void KPTextView::selectAll()
{
    textObject()->selectAll( true );
}

void KPTextView::drawCursor( bool b )
{
    KoTextView::drawCursor( b );
    if ( !cursor()->parag() )
        return;
    if ( !kpTextObject()->kPresenterDocument()->isReadWrite() )
        return;

    // We repaint the whole object.
    // TODO a kword-like painting method (many changes required though)
    //kpTextObject()->kPresenterDocument()->repaint( kpTextObject() );

    QPainter painter( m_canvas );
    painter.translate( -m_canvas->diffx(), -m_canvas->diffy() );
    painter.setBrushOrigin( -m_canvas->diffx(), -m_canvas->diffy() );

    kpTextObject()->drawCursor( &painter, cursor(), b, m_canvas );
}

void KPTextView::mousePressEvent( QMouseEvent *e, const QPoint &_pos)
{
    handleMousePressEvent( e, _pos );
}

void KPTextView::mouseDoubleClickEvent( QMouseEvent *e, const QPoint &pos)
{
    handleMouseDoubleClickEvent( e, pos  );
}

void KPTextView::mouseMoveEvent( QMouseEvent *e, const QPoint &_pos )
{
    handleMouseMoveEvent(e, _pos );
}

void KPTextView::mouseReleaseEvent( QMouseEvent *, const QPoint & )
{
    handleMouseReleaseEvent();
}

void KPTextView::showPopup( KPresenterView *view, const QPoint &point, QPtrList<KAction>& actionList )
{
    view->unplugActionList( "datatools" );
    view->unplugActionList( "datatools_link" );
    actionList.clear();
    actionList = dataToolActionList(view->kPresenterDoc()->instance());
    //kdDebug() << "KWView::openPopupMenuInsideFrame plugging actionlist with " << actionList.count() << " actions" << endl;
    if(refLink().isNull())
    {
        view->plugActionList( "datatools", actionList );
        QPopupMenu * popup = view->popupMenu("text_popup");
        Q_ASSERT(popup);
        if (popup)
            popup->popup( point ); // using exec() here breaks the spellcheck tool (event loop pb)
    }
    else
    {
        view->plugActionList( "datatools_link", actionList );
        QPopupMenu * popup = view->popupMenu("text_popup_link");
        Q_ASSERT(popup);
        if (popup)
            popup->popup( point ); // using exec() here breaks the spellcheck tool (event loop pb)
    }
}

void KPTextView::insertCustomVariable( const QString &name)
{
     KoVariable * var = 0L;
     KPresenterDoc * doc = kpTextObject()->kPresenterDocument();
     var = new KoCustomVariable( textObject()->textDocument(), name, doc->variableFormatCollection()->format( "STRING" ),  doc->getVariableCollection());
     insertVariable( var);
}

void KPTextView::insertLink(const QString &_linkName, const QString & hrefName)
{
    KoVariable * var = 0L;
    KPresenterDoc * doc = kpTextObject()->kPresenterDocument();
    var = new KoLinkVariable( textObject()->textDocument(),_linkName,hrefName , doc->variableFormatCollection()->format( "STRING" ),  doc->getVariableCollection());
    insertVariable( var);
}


void KPTextView::insertVariable( int type, int subtype )
{
    //kdDebug() << "KPTextView::insertVariable " << type << endl;
    KPresenterDoc * doc = kpTextObject()->kPresenterDocument();

    KoVariable * var = 0L;
    if ( type == VT_CUSTOM )
    {
        // Choose an existing variable
        KoVariableNameDia dia( m_canvas, doc->getVariableCollection()->getVariables() );
        if ( dia.exec() == QDialog::Accepted )
            var = new KoCustomVariable( textObject()->textDocument(), dia.getName(), doc->variableFormatCollection()->format( "STRING" ),doc->getVariableCollection() );
    }
    else
        var = doc->getVariableCollection()->createVariable( type, subtype,  doc->variableFormatCollection(), 0L, textObject()->textDocument(),doc);

    insertVariable( var );
}

void KPTextView::insertVariable( KoVariable *var )
{
    if ( var )
    {
        CustomItemsMap customItemsMap;
        customItemsMap.insert( 0, var );
        //kdDebug() << "KPTextView::insertVariable inserting into paragraph" << endl;
#ifdef DEBUG_FORMATS
        kdDebug() << "KPTextView::insertVariable currentFormat=" << currentFormat() << endl;
#endif
        textObject()->insert( cursor(), currentFormat(), KoTextObject::customItemChar(),
                                false, true, i18n("Insert Variable"),
                                customItemsMap );
        var->recalc();
        cursor()->parag()->invalidate(0);
        cursor()->parag()->setChanged( true );

        kpTextObject()->kPresenterDocument()->refreshMenuCustomVariable();
        kpTextObject()->kPresenterDocument()->repaint( kpTextObject() );
    }
}

KPrTextDrag * KPTextView::newDrag( QWidget * parent ) const
{
    QTextCursor c1 = textDocument()->selectionStartCursor( KoTextDocument::Standard );
    QTextCursor c2 = textDocument()->selectionEndCursor( KoTextDocument::Standard );

    QString text;

    QDomDocument domDoc( "PARAGRAPHS" );
    QDomElement elem = domDoc.createElement( "TEXTOBJ" );
    domDoc.appendChild( elem );
    if ( c1.parag() == c2.parag() )
    {
        text = c1.parag()->string()->toString().mid( c1.index(), c2.index() - c1.index() );
        m_kptextobj->saveParagraph( domDoc,static_cast<KoTextParag*>(c1.parag()),elem, c1.index(), c2.index()-1);
    }
    else
    {
        text += c1.parag()->string()->toString().mid( c1.index() ) + "\n";
        m_kptextobj->saveParagraph( domDoc,static_cast<KoTextParag*>(c1.parag()),elem, c1.index(), c1.parag()->length()-2);
        Qt3::QTextParag *p = c1.parag()->next();
        while ( p && p != c2.parag() ) {
            text += p->string()->toString() + "\n";
            m_kptextobj->saveParagraph( domDoc,static_cast<KoTextParag*>(p),elem, 0, p->length()-2);
            p = p->next();
        }
        text += c2.parag()->string()->toString().left( c2.index() );
        m_kptextobj->saveParagraph( domDoc,static_cast<KoTextParag*>(c2.parag()),elem, 0, c2.index()-1);
    }
    KPrTextDrag *kd = new KPrTextDrag( parent );
    kd->setPlain( text );
    kd->setKPresenter( domDoc.toCString() );
    kdDebug() << "KPTextView::newDrag " << domDoc.toCString() << endl;
    return kd;
}

void KPTextView::dragEnterEvent( QDragEnterEvent *e )
{
    if ( !kpTextObject()->kPresenterDocument()->isReadWrite() || !KPrTextDrag::canDecode( e ) )
    {
        e->ignore();
        return;
    }
    e->acceptAction();
}
void KPTextView::dragMoveEvent( QDragMoveEvent *e, const QPoint & )
{
    if ( !kpTextObject()->kPresenterDocument()->isReadWrite() || !KPrTextDrag::canDecode( e ) )
    {
        e->ignore();
        return;
    }
#if 0 //FIXME
    QPoint iPoint=e->pos() - kpTextObject()->getOrig();
    iPoint=kpTextObject()->kPresenterDocument()->zoomHandler()->pixelToLayoutUnit( QPoint(iPoint.x()+ m_canvas->diffx(),iPoint.y()+m_canvas->diffy()) );

    textObject()->emitHideCursor();
    placeCursor( iPoint );
    textObject()->emitShowCursor();
#endif
    e->acceptAction(); // here or out of the if ?
}
void KPTextView::dragLeaveEvent( QDragLeaveEvent * )
{
}



void KPTextView::dropEvent( QDropEvent * e )
{
#if 0 //FIXME
    if ( kpTextObject()->kPresenterDocument()->isReadWrite() && KPrTextDrag::canDecode( e ) )
    {
        e->acceptAction();
        QTextCursor dropCursor( textDocument() );
        QPoint dropPoint=e->pos() - kpTextObject()->getOrig();
        dropPoint=kpTextObject()->kPresenterDocument()->zoomHandler()->pixelToLayoutUnit( QPoint(dropPoint.x()+ m_canvas->diffx(),dropPoint.y()+m_canvas->diffy()) );

        // ####### Factorize code in libkotext!
        KMacroCommand *macroCmd=new KMacroCommand(i18n("Paste Text"));
        dropCursor.place( dropPoint, textDocument()->firstParag() );
        kdDebug(32001) << "KPTextView::dropEvent dropCursor at parag=" << dropCursor.parag()->paragId() << " index=" << dropCursor.index() << endl;

        if ( ( e->source() == m_canvas ) &&
             e->action() == QDropEvent::Move ) {
            //kdDebug()<<"decodeFrameSetNumber( QMimeSource *e ) :"<<numberFrameSet<<endl;;
            if ( textDocument()->hasSelection( KoTextDocument::Standard ) )
            {
                // Dropping into the selection itself ?
                QTextCursor startSel = textDocument()->selectionStartCursor( KoTextDocument::Standard );
                QTextCursor endSel = textDocument()->selectionEndCursor( KoTextDocument::Standard );
                bool inSelection = false;
                if ( startSel.parag() == endSel.parag() )
                    inSelection = ( dropCursor.parag() == startSel.parag() )
                                    && dropCursor.index() >= startSel.index()
                                    && dropCursor.index() <= endSel.index();
                else
                {
                    // Looking at first line first:
                    inSelection = dropCursor.parag() == startSel.parag() && dropCursor.index() >= startSel.index();
                    if ( !inSelection )
                    {
                        // Look at all other paragraphs except last one
                        Qt3::QTextParag *p = startSel.parag()->next();
                        while ( !inSelection && p && p != endSel.parag() )
                        {
                            inSelection = ( p == dropCursor.parag() );
                            p = p->next();
                        }
                        // Look at last paragraph
                        if ( !inSelection )
                            inSelection = dropCursor.parag() == endSel.parag() && dropCursor.index() <= endSel.index();
                    }
                }
                if ( inSelection )
                {
                    delete macroCmd;
                    textDocument()->removeSelection( KoTextDocument::Standard );
                    textObject()->selectionChangedNotify();
                    hideCursor();
                    *cursor() = dropCursor;
                    showCursor();
                    ensureCursorVisible();
                    return;
                }

                // Tricky. We don't want to do the placeCursor after removing the selection
                // (the user pointed at some text with the old selection in place).
                // However, something got deleted in our parag, dropCursor's index needs adjustment.
                if ( endSel.parag() == dropCursor.parag() )
                {
                    // Does the selection starts before (other parag or same parag) ?
                    if ( startSel.parag() != dropCursor.parag() || startSel.index() < dropCursor.index() )
                    {
                        // If other -> endSel.parag() will get deleted. The final position is in startSel.parag(),
                        // where the selection started + how much after the end we are. Make a drawing :)
                        // If same -> simply move back by how many chars we've deleted. Funny thing is, it's the same formula.
                        int dropIndex = dropCursor.index();
                        dropCursor.setParag( startSel.parag() );
                        // If dropCursor - endSel < 0, selection ends after, we're dropping into selection (no-op)
                        dropCursor.setIndex( dropIndex - QMIN( endSel.index(), dropIndex ) + startSel.index() );
                    }
                    kdDebug(32001) << "dropCursor: parag=" << dropCursor.parag()->paragId() << " index=" << dropCursor.index() << endl;
                }
                macroCmd->addCommand(textObject()->removeSelectedTextCommand( cursor(), KoTextDocument::Standard ));
            }
            hideCursor();
            *cursor() = dropCursor;
            showCursor();
            kdDebug(32001) << "cursor set back to drop cursor: parag=" << cursor()->parag()->paragId() << " index=" << cursor()->index() << endl;

        } else
        {   // drop coming from outside -> forget about current selection
            textDocument()->removeSelection( KoTextDocument::Standard );
            textObject()->selectionChangedNotify();
        }

        if ( e->provides( KPrTextDrag::selectionMimeType() ) )
        {
            QByteArray arr = e->encodedData( KPrTextDrag::selectionMimeType() );
            if ( arr.size() )
                macroCmd->addCommand(kpTextObject()->pasteKPresenter( cursor(), QCString(arr), false ));
        }
        else
        {
            QString text;
            if ( QTextDrag::decode( e, text ) )
                textObject()->pasteText( cursor(), text, currentFormat(), false );
        }
        kpTextObject()->kPresenterDocument()->addCommand(macroCmd);
    }
#endif
}


void KPTextObject::saveParagraph( QDomDocument& doc,KoTextParag * parag,QDomElement &parentElem,
                         int from /* default 0 */,
                         int to /* default length()-2 */ )
{
    if(!parag)
        return;
    QDomElement paragraph=doc.createElement(tagP);
    int tmpAlign=0;
    switch(parag->alignment())
    {
    case Qt::AlignLeft:
        tmpAlign=1;
        break;
    case Qt::AlignRight:
        tmpAlign=2;
        break;
    case Qt::AlignCenter:
        tmpAlign=4;
        break;
    case Qt::AlignJustify:
        tmpAlign=8;
    }
    if(tmpAlign!=1)
        paragraph.setAttribute(attrAlign, tmpAlign);

    saveParagLayout( parag->paragLayout(), paragraph );
    KoTextFormat *lastFormat = 0;
    QString tmpText;
    for ( int i = from; i <= to; ++i ) {
        KoTextStringChar &c = parag->string()->at(i);
        if ( !lastFormat || c.format()->key() != lastFormat->key() ) {
            if ( lastFormat )
                paragraph.appendChild(saveHelper(tmpText, lastFormat, doc));
            lastFormat = static_cast<KoTextFormat*> (c.format());
            tmpText=QString::null;
        }
        tmpText+=c.c;
    }
    if ( lastFormat ) {
        paragraph.appendChild(saveHelper(tmpText, lastFormat, doc));
    }
    parentElem.appendChild(paragraph);
}
