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

#include <koparagcounter.h>

#include "contents.h"
#include "kwdoc.h"
#include "kwtextframeset.h"
#include "kwtextdocument.h"
#include "kwtextparag.h"

#include <klocale.h>
#include <kdebug.h>

KWInsertTOCCommand::KWInsertTOCCommand( KWTextFrameSet * fs, KoTextParag *parag )
    : KoTextDocCommand( fs->textDocument() ), m_paragId( parag->paragId() )
{
}

KoTextCursor * KWInsertTOCCommand::execute( KoTextCursor *c )
{
    KWTextDocument * textdoc = static_cast<KWTextDocument *>(doc);
    KWTextFrameSet * fs = textdoc->textFrameSet();

    fs->kWordDocument()->renameButtonTOC(true);

    KoTextParag *insertionParag = textdoc->paragAt( m_paragId );
    if ( !insertionParag ) {
        qWarning( "KWInsertTOCCommand:: can't locate parag at %d, last parag: %d", m_paragId, textdoc->lastParag()->paragId() );
        return c;
    }
    KWTextParag *body = static_cast<KWTextParag *>( insertionParag );
    // Create new very first paragraph
    KWTextParag *parag = static_cast<KWTextParag *>( textdoc->createParag( textdoc, body->prev() /*prev*/, body /*next*/, true ) );
    parag->append( i18n( "Table of Contents" ) );
    KWStyle * style = findOrCreateTOCStyle( fs, -1 ); // "Contents Title"
    parag->setParagLayout( style->paragLayout() );
    parag->setFormat( 0, parag->string()->length(), textdoc->formatCollection()->format( &style->format() ) );

    // Insert table and THEN set page numbers
    // Otherwise the page numbers are incorrect

    KWTextParag *p = static_cast<KWTextParag *>(textdoc->firstParag()/*parag->next()*/);
    //ASSERT( p == body );
    KWTextParag *prevTOCParag = parag;
    QMap<KWTextParag *, KWTextParag *> paragMap;     // Associate a paragraph form the TOC with the real one from the body
    while ( p ) {
        // We recognize headers by the "numbering" property of the paragraph
        // This way, we don't rely on a particular name (breaks when renaming)
        // nor on whether the user used styles or not.
        // It even lets a user create two styles for the same level of chapter.
        if ( p->counter() && p->counter()->numbering() == KoParagCounter::NUM_CHAPTER )
        {
            parag = static_cast<KWTextParag *>(textdoc->createParag( textdoc, prevTOCParag /*prev*/, body /*next*/, true ));
            QString txt = p->string()->toString();
            txt = txt.left( txt.length() - 1 ); // remove trailing space
            txt.prepend( p->counter()->text(p) );
            parag->append( txt );
            prevTOCParag = parag;

            paragMap.insert( parag, p );
        }
        p = static_cast<KWTextParag *>(p->next());
    }
    // Set a hard frame break after the last TOC parag
    kdDebug() << "KWInsertTOCCommand::execute setPageBreaking on " << prevTOCParag << " " << prevTOCParag->paragId() << endl;
    prevTOCParag->setPageBreaking( prevTOCParag->pageBreaking() | KWParagLayout::HardFrameBreakAfter );

    // Format paragraphs, to take this page break into account and update page numbers
    fs->layout();
    fs->updateFrames();

    //kdDebug() << "KWInsertTOCCommand::execute layouting done, setting page numbers" << endl;

    // Now add the page numbers, and apply the style
    QMap<KWTextParag *, KWTextParag *>::Iterator mapIt = paragMap.begin();
    for ( ; mapIt != paragMap.end() ; ++mapIt )
    {
        KWTextParag * parag = mapIt.key(); // Parag in the TOC
        KWTextParag * p = mapIt.data();    // Parag in the body

        // Find page number for paragraph
        KoPoint pt;
        KWFrame * frame = fs->internalToDocument( QPoint(0, p->rect().top()), pt );
        if ( frame ) // let's be safe
        {
            parag->append( "\t" );
            parag->append( QString::number( frame->pageNum() + 1 ) );
        }

        // Apply style
        int depth = p->counter()->depth();    // we checked for p->counter() before putting in the map
        KWStyle * tocStyle = findOrCreateTOCStyle( fs, depth );
        parag->setParagLayout( tocStyle->paragLayout() );
        parag->setFormat( 0, parag->string()->length(), & tocStyle->format() );
    }
    // The setParagLayout ruined it, so here it is again :)
    prevTOCParag->setPageBreaking( prevTOCParag->pageBreaking() | KWParagLayout::HardFrameBreakAfter );
    return c;
}

KoTextCursor *KWInsertTOCCommand::unexecute( KoTextCursor *c )
{
    KWTextDocument * textdoc = static_cast<KWTextDocument *>(doc);
    KWTextFrameSet * fs = textdoc->textFrameSet();

    removeTOC( fs, c, 0L );
    fs->kWordDocument()->renameButtonTOC(false);
    return c;
}

KoTextCursor * KWInsertTOCCommand::removeTOC( KWTextFrameSet *fs, KoTextCursor *cursor, KMacroCommand * /*macroCmd*/ )
{
    KoTextDocument * textdoc = fs->textDocument();
    // Remove existing table of contents, based on the style
    KoTextCursor start( textdoc );
    KoTextCursor end( textdoc );
    // We start from the end, to avoid the parag shifting problem
    KoTextParag *p = textdoc->lastParag();
    KoTextCursor *posOfTable=0L;
    KWTextParag *posOfToc=0L;

    while ( p )
    {
        KWTextParag * parag = static_cast<KWTextParag *>(p);
        if ( parag->style() && ( parag->style()->name().startsWith( "Contents Head" ) ||
            parag->style()->name() == "Contents Title" ) )
        {
            posOfToc=parag;

            kdDebug() << "KWContents::createContents Deleting paragraph " << p << " " << p->paragId() << endl;
            // This paragraph is part of the TOC -> remove

            /* This method aims to provide an "undo" that restores the previous version of the TOC.
               Problem is, it screws up the parag style (due to removeSelectedText calling join),
               and the first parag of the body ends up with the Contents Title style.
            start.setParag( p );
            start.setIndex( 0 );
            textdoc->setSelectionStart( KoTextDocument::Temp, &start );
            ASSERT( p->next() );
            end.setParag( p->next() );
            end.setIndex( 0 );
            textdoc->setSelectionEnd( KoTextDocument::Temp, &end );
            KCommand * cmd = fs->removeSelectedTextCommand( cursor, KoTextDocument::Temp );
            if ( macroCmd )
                macroCmd->addCommand( cmd );
            */

            // So instead, we do things by hand, and without undo....

            KoTextParag *prev = p->prev();
            KoTextParag *next = p->next();
            // Move cursor out
            if ( cursor->parag() == p )
                cursor->setParag( next ? next : prev );
            delete p;
            kdDebug() << "KWInsertTOCCommand::removeTOC " << p << " deleted" << endl;
            p = next;
            kdDebug() << "KWInsertTOCCommand::removeTOC prev=" << prev << " p=" << p << endl;
            // Fix parag chain
            if ( prev )
            {
                prev->setNext( p );
                if ( p )
                    p->setParagId( prev->paragId() + 1 );
            }
            else
            {
                textdoc->setFirstParag( p );
                if ( p )
                {
                    p->setParagId( 0 );
                } else // completely empty document !
                {
                    textdoc->clear( true ); // recreate empty parag.
                    cursor->setParag( textdoc->firstParag() );
                    break;
                }
            }
            p->setPrev( prev );
        }
        p = p->prev();
    }
    textdoc->invalidate();
     if(posOfToc)
     {
         posOfTable=new KoTextCursor( textdoc );
         posOfTable->setParag(posOfToc  );
         posOfTable->setIndex( 0 );//start of parag
     }
    // ### TODO propagate parag ID changes.
    return posOfTable;
}

KWStyle * KWInsertTOCCommand::findOrCreateTOCStyle( KWTextFrameSet *fs, int depth )
{
    // Determine style name.
    // NOTE: don't add i18n here ! This is translated using
    // the i18n calls in stylenames.cc !
    QString name;
    if ( depth >= 0 )
        name = QString( "Contents Head %1" ).arg( depth+1 );
    else
        name = "Contents Title";
    KWStyle * style = fs->kWordDocument()->styleCollection()->findStyle( name );
    if ( !style )
    {
        style = new KWStyle( name );
        style->format().setBold( ( ( depth==-1) || ( depth==0 ) ) ? true : false );
        style->format().setPointSize( depth==-1 ? 20 : 12 );
        if ( depth == -1 )
        {
            style->paragLayout().topBorder = KoBorder( Qt::black, KoBorder::SOLID, 1 );
            style->paragLayout().bottomBorder = KoBorder( Qt::black, KoBorder::SOLID, 1 );
            // Old kword had only top and bottom. But borders are drawn differently now
            // (not the whole line anymore), so we need the 4 borders.
            style->paragLayout().leftBorder = KoBorder( Qt::black, KoBorder::SOLID, 1 );
            style->paragLayout().rightBorder = KoBorder( Qt::black, KoBorder::SOLID, 1 );
            style->paragLayout().alignment = Qt::AlignCenter;
        }
        else
        {
            KoTabulatorList tabList;
            KoTabulator tab;
            tab.ptPos = KoUnit::ptFromUnit( floor( KoUnit::toMM( fs->frame( 0 )->width() )  ), KoUnit::unit("mm") );
            tab.type = T_RIGHT;
            tab.filling = TF_DOTS;
            tab.ptWidth = 0.5;
            tabList.append( tab );
            style->paragLayout().setTabList( tabList );
            style->paragLayout().margins[QStyleSheetItem::MarginLeft] = KoUnit::ptFromUnit( (depth*4.5), KoUnit::unit("mm") );
        }
        style = fs->kWordDocument()->styleCollection()->addStyleTemplate( style );     // register the new style
        fs->kWordDocument()->updateAllStyleLists();                 // show it in the UI
    }
    return style;
}
