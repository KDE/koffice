/* This file is part of KOffice
   Copyright (C) 2004 Zack Rusin <zack@kde.org>

   Original version written by David Sweet <dsweet@kde.org> and
         Wolfram Diestel <wolfram@steloj.de>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include "config.h"
#ifdef HAVE_LIBKSPELL2
#include "kospell.h"

#include "kotextobject.h"
#include "kotextiterator.h"

#include <kspell2/broker.h>
#include <kspell2/filter.h>

#include <kglobal.h>
#include <klocale.h>
#include <kdebug.h>

#include <qtimer.h>

using namespace KSpell2;

class KoSpell::Private
{
public:
    KoTextIterator *itr;
    KoTextParag    *parag;
    bool            dialog;
    bool            needsIncrement;
};

static QString paragToText( KoTextParag *parag )
{
    if ( !parag || !parag->string()->needsSpellCheck() )
        return QString::null;

    parag->string()->setNeedsSpellCheck( false );
    if ( parag->length() <= 1 )
        return QString::null;

    QString str = parag->string()->toString();
    str.truncate( str.length() - 1 ); // remove trailing space
    return str;
}

KoSpell::KoSpell( const Broker::Ptr& broker,  QObject *parent,
                  const char *name )
    : BackgroundChecker( broker, parent, name )
{
    d = new Private;
    d->parag = 0;
    d->itr = 0;
    d->dialog = false;
    d->needsIncrement = false;
}

KoSpell::~KoSpell()
{
    delete d;
}

bool KoSpell::check( KoTextIterator *itr, bool dialog )
{
    bool ret = false;

    if ( !itr )
        return ret;
    d->itr = itr;
    d->needsIncrement = false;
    ret = !d->itr->atEnd();
    d->dialog = dialog;

    return ret;
}

bool KoSpell::check( KoTextParag *parag )
{
    if ( checking() || !parag )
        return false;

    d->parag = parag;

    start();

    return true;
}

bool KoSpell::checkWordInParagraph( KoTextParag *parag, int pos,
                                    QString& foundWord, int& start )
{
    if ( !parag ) {
        start = -1;
        return false;
    }

    d->parag = parag;
    QString str = paragToText( parag );
    Filter filter;
    filter.setBuffer( str );
    Word w = filter.wordAtPosition( pos );
    foundWord = w.word;
    start = w.start;
#ifdef DEBUG_SPELL
    kdDebug()<<"XXXXX WORD IS " << w.word << ", pos = "<< pos
             << ", start = "<< w.start <<endl;
#endif
    return checkWord( w.word );
}

QString KoSpell::getMoreText()
{
#ifdef DEBUG_SPELL
    kdDebug()<<"here 1 dialog = " << d->dialog << ", itr = "
             << d->itr << ", atEnd = "
             << ( ( d->itr ) ? d->itr->atEnd() : true )
             << endl;
#endif

    bool iteratorAtEnd = d->itr && d->itr->atEnd();

    if ( d->needsIncrement && !iteratorAtEnd ) {
        ++( *d->itr );
    }

    if ( !d->dialog && ( !d->itr || d->itr->atEnd() ) ) {
        QString str = paragToText( d->parag );
        if ( !str.isEmpty() )
            emit aboutToFeedText();
        return str;
    }

    if ( iteratorAtEnd )
        return QString::null;

    d->needsIncrement = true;

    QString text = d->itr->currentText();
    d->parag = d->itr->currentParag();

    emit aboutToFeedText();
    //kdDebug()<<"here 2"<<endl;
    while ( !d->dialog && d->parag ) {
        if ( d->parag->string()->needsSpellCheck() &&
             d->parag->length() > 1 )
            break;
        ++(*d->itr);
        if ( d->itr->atEnd() ) {
            d->needsIncrement = false;
            return QString::null;
        }
        d->parag = d->itr->currentParag();
        text = d->itr->currentText();
    }
    //kdDebug()<<"here 3"<<endl;
    d->parag->string()->setNeedsSpellCheck( false );

    return text;
}

void KoSpell::finishedCurrentFeed()
{
    emit paragraphChecked( d->parag );
}

KoTextParag  *KoSpell::currentParag() const
{
    return d->parag;
}

KoTextObject *KoSpell::currentTextObject() const
{
    if ( d->itr && !d->itr->atEnd() )
        return d->itr->currentTextObject();
    return 0;
}

int KoSpell::currentStartIndex() const
{
    if ( d->itr && !d->itr->atEnd() )
        return d->itr->currentStartIndex();
    return 0;
}

bool KoSpell::checking() const
{
#ifdef DEBUG_SPELL
    kdDebug()<< d->itr << ", "
             << ( ( d->itr ) ? d->itr->atEnd() : false ) << ", "
             << filter()->atEnd()
             << endl;
#endif
    if ( d->itr ) {
        if ( d->itr->atEnd() &&
             filter()->atEnd() )
            return false;
        else
            return true;
    } else
        return filter()->atEnd();
}

#include "kospell.moc"

#endif
