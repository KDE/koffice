/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Reginald Stadlbauer <reggie@kde.org>
                 2001       Sven Leiber         <s.leiber@web.de>

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

#include "autoformat.h"
#include "kwdoc.h"
#include "kwtextparag.h"
#include "kwtextdocument.h"
#include "kwtextframeset.h"

#include <kdebug.h>
#include <klocale.h>
#include <kinstance.h>
#include <kconfig.h>
#include <kstddirs.h>

#include <qrichtext_p.h>
#include <qvector.h>
#include <qfile.h>
#include <qdom.h>

#include <kotextobject.h>


/******************************************************************/
/* Class: KWAutoFormat						  */
/******************************************************************/
KWAutoFormat::KWAutoFormat( KWDocument *_doc )
    : m_doc( _doc ), m_configRead( false ),
      m_convertUpperCase( false ), m_convertUpperUpper( false ),
      m_typographicQuotes(), /*m_enabled( true ),*/
      m_maxlen( 0 )
{
}

void KWAutoFormat::readConfig()
{
    // Read the autoformat configuration
    // This is done on demand (when typing the first char, or when opening the config dialog)
    // so that loading is faster and to avoid doing it for readonly documents.
    KLocale klocale("kword");
    if ( m_configRead )
        return;
    KConfig * config = m_doc->instance()->config();
    KConfigGroupSaver cgs( config, "AutoFormat" );
    m_convertUpperCase = config->readBoolEntry( "ConvertUpperCase", false );
    m_convertUpperUpper = config->readBoolEntry( "ConvertUpperUpper", false );
    QString begin = config->readEntry( "TypographicQuotesBegin", "�" );
    m_typographicQuotes.begin = begin[0];
    QString end = config->readEntry( "TypographicQuotesEnd", "�" );
    m_typographicQuotes.end = end[0];
    m_typographicQuotes.replace = config->readBoolEntry( "TypographicQuotesEnabled", false )
                                  && !begin.isEmpty()
                                  && !end.isEmpty();

    Q_ASSERT( m_entries.isEmpty() ); // readConfig is only called once...
    config->setGroup( "AutoFormatEntries" );

    bool fileNotFound = false;
    QFile xmlFile;

    xmlFile.setName(KWFactory::global()->dirs()->findResource("autocorrect", klocale.languageList().front() + ".xml"));
    if(!xmlFile.open(IO_ReadOnly)) {
        xmlFile.setName(KWFactory::global()->dirs()->findResource("autocorrect","autocorrect.xml"));
    if(!xmlFile.open(IO_ReadOnly)) {
	fileNotFound = true;
      }
    }

    if(!fileNotFound) {
      QDomDocument doc;
      if(!doc.setContent(&xmlFile)) {
        //return;
      }
      if(doc.doctype().name() != "autocorrection") {
        //return;
      }
      QDomElement de=doc.documentElement();
      QDomElement item = de.namedItem( "items" ).toElement();
      if(!item.isNull())
      {
          QDomNodeList nl = item.childNodes();
          m_maxFindLength=nl.count();
          for(uint i = 0; i < m_maxFindLength; i++) {
              m_entries.insert( nl.item(i).toElement().attribute("find"), KWAutoFormatEntry(nl.item(i).toElement().attribute("replace")) );
          }
      }

      QDomElement upper = de.namedItem( "UpperCaseExceptions" ).toElement();
      if(!upper.isNull())
      {
          QDomNodeList nl = upper.childNodes();
          for(uint i = 0; i < nl.count(); i++)
          {
              upperCaseExceptions+= nl.item(i).toElement().attribute("exception");
          }
      }

      QDomElement twoUpper = de.namedItem( "TwoUpperLetterExceptions" ).toElement();
      if(!twoUpper.isNull())
      {
          QDomNodeList nl = twoUpper.childNodes();
          for(uint i = 0; i < nl.count(); i++)
          {
              twoUpperLetterException+= nl.item(i).toElement().attribute("exception");
          }
      }
    }
    xmlFile.close();

    buildMaxLen();
    //compatibility with kword1.1
    if(config->hasKey( "UpperCaseExceptions" ) )
        upperCaseExceptions+=config->readListEntry( "UpperCaseExceptions" );

    if(config->hasKey( "TwoUpperLetterExceptions"))
        twoUpperLetterException+=config->readListEntry( "TwoUpperLetterExceptions" );
    m_configRead = true;
}

void KWAutoFormat::saveConfig()
{
    KConfig * config = m_doc->instance()->config();
    KLocale klocale("kword");
    KConfigGroupSaver cgs( config, "AutoFormat" );
    config->writeEntry( "ConvertUpperCase", m_convertUpperCase );
    config->writeEntry( "ConvertUpperUpper", m_convertUpperUpper );
    config->writeEntry( "TypographicQuotesBegin", QString( m_typographicQuotes.begin ) );
    config->writeEntry( "TypographicQuotesEnd", QString( m_typographicQuotes.end ) );
    config->writeEntry( "TypographicQuotesEnabled", m_typographicQuotes.replace );
    config->setGroup( "AutoFormatEntries" );
    KWAutoFormatEntryMap::Iterator it = m_entries.begin();

    //refresh m_maxFindLength
    m_maxFindLength=0;

    QDomDocument doc("autocorrection");

    QDomElement begin = doc.createElement( "Word" );
    doc.appendChild( begin );

    QDomElement items;
    items = doc.createElement("items");
    QDomElement data;
    for ( ; it != m_entries.end() ; ++it )
    {
	data = doc.createElement("item");
	data.setAttribute("find", it.key());
	data.setAttribute("replace", it.data().replace());
	items.appendChild(data);

        m_maxFindLength=QMAX(m_maxFindLength,it.key().length());
    }
    begin.appendChild(items);

    QDomElement upper;
    upper = doc.createElement("UpperCaseExceptions");
    for ( QStringList::Iterator it = upperCaseExceptions.begin(); it != upperCaseExceptions.end();++it )
    {
	data = doc.createElement("word");
	data.setAttribute("exception",(*it) );
	upper.appendChild(data);
    }
    begin.appendChild(upper);

    QDomElement twoUpper;
    twoUpper = doc.createElement("TwoUpperLetterExceptions");

    for ( QStringList::Iterator it = twoUpperLetterException.begin(); it != twoUpperLetterException.end();++it )
    {
	data = doc.createElement("word");
	data.setAttribute("exception",(*it) );
	twoUpper.appendChild(data);
    }
    begin.appendChild(twoUpper);

    QFile f(locateLocal("data", "kword/autocorrect/"+klocale.languageList().front() + ".xml"));
    if(!f.open(IO_WriteOnly)) {
        kdDebug()<<"Error during saving...........\n";
	return;
    }
    QTextStream ts(&f);
    doc.save(ts, 2);
    f.close();


    //config->writeEntry( "UpperCaseExceptions",upperCaseExceptions );

    //config->writeEntry( "TwoUpperLetterExceptions",twoUpperLetterException);

    config->sync();
}

QString KWAutoFormat::getLastWord(KoTextParag *parag, int index)
{
    QString lastWord;
    QTextString *s = parag->string();
    for ( int i = index - 1; i >= 0; --i )
    {
        QChar ch = s->at( i ).c;
        if ( ch.isSpace() || ch.isPunct() )
            break;
        lastWord.prepend( ch );
    }
    return lastWord;
}

void KWAutoFormat::doAutoFormat( QTextCursor* textEditCursor, KoTextParag *parag, int index, QChar ch )
{
    if ( !m_configRead )
        readConfig();

    if ( !m_convertUpperUpper && !m_convertUpperCase
         && !m_typographicQuotes.replace && m_entries.count()==0)
        return;

    //kdDebug() << "KWAutoFormat::doAutoFormat ch=" << QString(ch) << endl;
    //if ( !m_enabled )
    //    return;
    // Auto-correction happens when pressing space, tab, CR, punct etc.
    if ( ch.isSpace() || ch.isPunct() )
    {
        if ( index > 0 )
        {
            QString lastWord = getLastWord(parag, index);
            //kdDebug() << "KWAutoFormat::doAutoFormat lastWord=" << lastWord << endl;
            if ( !doAutoCorrect( textEditCursor, parag, index ) )
            {
                if ( m_convertUpperUpper || m_convertUpperCase )
                    doUpperCase( textEditCursor, parag, index, lastWord );
                // todo doSpellCheck( textEditCursor, parag, index, lastWord );
            }
        }
    }
    if ( ch == '"' && m_typographicQuotes.replace )
    {
        doTypographicQuotes( textEditCursor, parag, index );
    }
}

bool KWAutoFormat::doAutoCorrect( QTextCursor* textEditCursor, KoTextParag *parag, int index )
{
    // Prepare an array with words of different lengths, all terminating at "index".
    // Obviously only full words are put into the array
    // But this allows 'find strings' with spaces and punctuation in them.
    QString * wordArray = new QString[m_maxFindLength+1];
    {
        QString word;
        QTextString *s = parag->string();
        for ( int i = index - 1; i >= 0; --i )
        {
            QChar ch = s->at( i ).c;
            if ( ch.isSpace() || ch.isPunct() || i==0)
            {
                if(i==0 && word.length()<m_maxFindLength)
                   word.prepend( ch );
                wordArray[word.length()]=word;
            }
            word.prepend( ch );
            if (((index - 1)-i) == (int)m_maxFindLength)
                break;
        }
    }
    KWTextDocument * textdoc = static_cast<KWTextDocument *>(parag->textDocument());
    // Now for each entry in the autocorrect list, look if
    // the word of the same size in wordArray matches.
    // This allows an o(n) behaviour instead of an o(n^2).
    for(int i=m_maxFindLength;i>0;--i)
    {
        KWAutoFormatEntryMap::ConstIterator it = m_entries.find(wordArray[i]);
        if ( wordArray[i]!=0 && it!=m_entries.end() )
        {
            unsigned int length = wordArray[i].length();
            int start = index - length;
            QTextCursor cursor( parag->document() );
            cursor.setParag( parag );
            cursor.setIndex( start );
            textdoc->setSelectionStart( KoTextObject::HighlightSelection, &cursor );
            cursor.setIndex( start + length );
            textdoc->setSelectionEnd( KoTextObject::HighlightSelection, &cursor );

            KWTextFrameSet * textfs = textdoc->textFrameSet();
            m_doc->addCommand(textfs->textObject()->replaceSelectionCommand( textEditCursor, it.data().replace(),
                                      KoTextObject::HighlightSelection,
                                      i18n("Autocorrect word") ));
            // The space/tab/CR that we inserted is still there but delete/insert moved the cursor
            // -> go right
            textfs->textObject()->emitHideCursor();
            textEditCursor->gotoRight();
            textfs->textObject()->emitShowCursor();
            delete [] wordArray;
            return true;
        }
    }
    delete [] wordArray;
    return false;
}

void KWAutoFormat::doTypographicQuotes( QTextCursor* textEditCursor, KoTextParag *parag, int index )
{
    kdDebug() << "KWAutoFormat::doTypographicQuotes" << endl;
    KWTextDocument * textdoc = static_cast<KWTextDocument *>(parag->textDocument());
    QTextCursor cursor( parag->document() );
    cursor.setParag( parag );
    cursor.setIndex( index );
    textdoc->setSelectionStart( KoTextObject::HighlightSelection, &cursor );
    cursor.setIndex( index + 1 );
    textdoc->setSelectionEnd( KoTextObject::HighlightSelection, &cursor );

    // Need to determine if we want a starting or ending quote.
    // I see two solutions: either simply alternate, or depend on leading space.
    // MSWord does the latter afaics...
    QString replacement;
    if ( index > 0 && !parag->at( index - 1 )->c.isSpace() )
        replacement = m_typographicQuotes.end;
    else
        replacement = m_typographicQuotes.begin;

    KWTextFrameSet * textfs = textdoc->textFrameSet();
    m_doc->addCommand (textfs->textObject()->replaceSelectionCommand( textEditCursor, replacement,
                              KoTextObject::HighlightSelection,
                              i18n("Typographic quote") ));
}

void KWAutoFormat::doUpperCase( QTextCursor *textEditCursor, KoTextParag *parag,
                                int index, const QString & word )
{
    KWTextDocument * textdoc = static_cast<KWTextDocument *>(parag->textDocument());
    unsigned int length = word.length();
    int start = index - length;
    QTextCursor backCursor( parag->document() );
    backCursor.setParag( parag );
    backCursor.setIndex( start );

    // backCursor now points at the first char of the word
    QChar firstChar = backCursor.parag()->at( backCursor.index() )->c;
    bool bNeedMove = false;

    if ( m_convertUpperCase && isLower( firstChar ) )
    {
        bool beginningOfSentence = true; // true if beginning of text
        // Go back over any space/tab/CR
        while ( backCursor.index() > 0 || backCursor.parag()->prev() )
        {
            beginningOfSentence = false; // we could go back -> false unless we'll find '.'
            backCursor.gotoLeft();
            if ( !backCursor.parag()->at( backCursor.index() )->c.isSpace() )
                break;
        }
        // We are now at the first non-space char before the word
        if ( !beginningOfSentence )
            beginningOfSentence = isMark( backCursor.parag()->at( backCursor.index() )->c );

        // Now look for exceptions
        if ( beginningOfSentence )
        {
            QChar punct = backCursor.parag()->at( backCursor.index() )->c;
            QString text = getLastWord( static_cast<KoTextParag*>( backCursor.parag() ), backCursor.index() )
                           + punct;
            // text has the word at the end of the 'sentence', including the termination. Example: "Mr."
            beginningOfSentence = (upperCaseExceptions.findIndex(text)==-1); // Ok if we can't find it
        }

        if ( beginningOfSentence )
        {
            QTextCursor cursor( parag->document() );
            cursor.setParag( parag );
            cursor.setIndex( start );
            textdoc->setSelectionStart( KoTextObject::HighlightSelection, &cursor );
            cursor.setIndex( start + 1 );
            textdoc->setSelectionEnd( KoTextObject::HighlightSelection, &cursor );

            KWTextFrameSet * textfs = textdoc->textFrameSet();
            m_doc->addCommand(textfs->textObject()->replaceSelectionCommand( textEditCursor, QString( firstChar.upper() ),
                                      KoTextObject::HighlightSelection,
                                      i18n("Autocorrect (capitalize first letter)") ));
            bNeedMove = true;
        }
    }
    else if ( m_convertUpperUpper && isUpper( firstChar ) && length > 2 )
    {
        backCursor.setIndex( backCursor.index() + 1 );
        QChar secondChar = backCursor.parag()->at( backCursor.index() )->c;
        if ( isUpper( secondChar ) )
        {
            // Check next letter - we still want to be able to write fully uppercase words...
            backCursor.setIndex( backCursor.index() + 1 );
            QChar thirdChar = backCursor.parag()->at( backCursor.index() )->c;
            if ( isLower( thirdChar ) && (twoUpperLetterException.findIndex(word)==-1))
            {
                // Ok, convert
                QTextCursor cursor( parag->document() );
                cursor.setParag( parag );
                cursor.setIndex( start + 1 ); // After all the first letter's fine, so only change the second letter
                textdoc->setSelectionStart( KoTextObject::HighlightSelection, &cursor );
                cursor.setIndex( start + 2 );
                textdoc->setSelectionEnd( KoTextObject::HighlightSelection, &cursor );

                QString replacement = word[1].lower();
                KWTextFrameSet * textfs = textdoc->textFrameSet();
                m_doc->addCommand(textfs->textObject()->replaceSelectionCommand( textEditCursor, replacement,
                                          KoTextObject::HighlightSelection,
                                          i18n("Autocorrect uppercase-uppercase") )); // hard to describe....
                bNeedMove = true;
            }
        }
    }
    if ( bNeedMove )
    {
        // Back to where we were
        KWTextFrameSet * textfs = textdoc->textFrameSet();
        textfs->textObject()->emitHideCursor();
        textEditCursor->setParag( parag );
        textEditCursor->setIndex( index );
        textEditCursor->gotoRight(); // not the same thing as index+1, in case of CR
        textfs->textObject()->emitShowCursor();
    }
}

void KWAutoFormat::doSpellCheck( QTextCursor *,KoTextParag */*parag*/, int /*index*/, const QString & /*word*/ )
{
#if 0
    if ( !enabled || !doc->onLineSpellCheck() )
	return;
    if ( isSeparator( parag->string()->data()[ fc->getTextPos() ].c ) ) {
	if ( !spBuffer.isEmpty() && spBegin ) {
	    //qDebug( "spellcheck: %s", spBuffer.latin1() );
	    spBuffer = QString::null;
	    spBegin = 0;
	}
	return;
    }

    if ( spBuffer.isEmpty() )
	spBegin = &parag->string()->data()[ fc->getTextPos() ];
    spBuffer += parag->string()->data()[ fc->getTextPos() ].c;
#endif
}

void KWAutoFormat::configTypographicQuotes( TypographicQuotes _tq )
{
    m_typographicQuotes = _tq;
}

void KWAutoFormat::configUpperCase( bool _uc )
{
    m_convertUpperCase = _uc;
}

void KWAutoFormat::configUpperUpper( bool _uu )
{
    m_convertUpperUpper = _uu;
}

bool KWAutoFormat::isUpper( const QChar &c )
{
    return c.lower() != c;
}

bool KWAutoFormat::isLower( const QChar &c )
{
    // Note that this is not the same as !isUpper !
    // For instance '1' is not lower nor upper,
    return c.upper() != c;
}

bool KWAutoFormat::isMark( const QChar &c )
{
    return ( c == QChar( '.' ) ||
	     c == QChar( '?' ) ||
	     c == QChar( '!' ) );
}

bool KWAutoFormat::isSeparator( const QChar &c )
{
    return ( !c.isLetter() && !c.isNumber() && !c.isDigit() );
}

void KWAutoFormat::buildMaxLen()
{
    QMap< QString, KWAutoFormatEntry >::Iterator it = m_entries.begin();

    m_maxlen = 0;
    for ( ; it != m_entries.end(); ++it )
	m_maxlen = QMAX( m_maxlen, it.key().length() );
}
