// -*- Mode: c++; c-basic-offset: 4; indent-tabs-mode: nil; tab-width: 4; -*-
/* This file is part of the KDE project
   Copyright (C) 2001 Laurent Montel <lmontel@mandrakesoft.com>

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

#include "kprtextdocument.h"
#include "kpresenter_doc.h"
#include "kptextobject.h"
#include <kdebug.h>

#include "kprvariable.h"

#include <kooasiscontext.h>

KPrTextDocument::KPrTextDocument( KPTextObject * textobj, KoTextFormatCollection *fc, KoTextFormatter *formatter )
    : KoTextDocument( textobj->kPresenterDocument()->zoomHandler(), fc, formatter, true ), m_textobj( textobj )
{
    //kdDebug(33001) << "KPrTextDocument constructed " << this << "  KPTextObject:" << textobj << endl;
}

KPrTextDocument::~KPrTextDocument()
{
}


bool KPrTextDocument::loadSpanTag( const QDomElement& tag, KoOasisContext& context,
                                  KoTextParag* parag, uint pos,
                                  QString& textData, KoTextCustomItem* & customItem )
{
    const QString tagName( tag.tagName() );
    const bool textFoo = tagName.startsWith( "text:" );
    kdDebug() << "KWTextDocument::loadSpanTag: " << tagName << endl;

    if ( textFoo )
    {
        if ( tagName == "text:a" )
        {
            QString href( tag.attribute("xlink:href") );
            if ( href.startsWith("#") )
            {
                context.styleStack().save();
                // We have a reference to a bookmark (### TODO)
                // As we do not support it now, treat it as a <text:span> without formatting
                parag->loadOasisSpan( tag, context, pos ); // recurse
                context.styleStack().restore();
            }
            else
            {
                // The text is contained in a text:span inside the text:a element. In theory
                // we could have multiple spans there, but OO ensures that there is always only one,
                // splitting the hyperlink if necessary (at format changes).
                // Note that we ignore the formatting of the span.
                QDomElement spanElem = tag.namedItem( "text:span" ).toElement();
                QString text;
                if( spanElem.isNull() )
                    text = tag.text();
                else {
                    // The save/restore of the stack is done by the caller (KoTextParag::loadOasisSpan)
                    // This allows to use the span's format for the variable.
                    //kdDebug(32500) << "filling stack with " << spanElem.attribute( "text:style-name" ) << endl;
                    context.fillStyleStack( spanElem, "text:style-name" );
                    text = spanElem.text();
                }
                textData = '#'; // hyperlink placeholder
                // unused tag.attribute( "office:name" )
                KoVariableCollection& coll = context.variableCollection();
                customItem = new KoLinkVariable( this, text, href,
                                                 coll.formatCollection()->format( "STRING" ),
                                                 &coll );
            }
            return true;
        }
    }
    else // non "text:" tags
        {
            kdDebug()<<"Extension found tagName : "<<tagName<<endl;
        }
    return false;
}


#include "kprtextdocument.moc"
