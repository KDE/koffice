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

#include "koAutoFormat.h"
//#include "kotextdocument.h"

#include <kdebug.h>
#include <klocale.h>
#include <kinstance.h>
#include <kconfig.h>
#include <kstandarddirs.h>

#include <qfile.h>

#include <kotextobject.h>
#include <qdom.h>
#include <kglobal.h>
#include <koDocument.h>
#include "koVariable.h"
#include "koparagcounter.h"
#include <kcommand.h>
//#include <kotextformat.h>
#include <kcompletion.h>
#include <kcommand.h>

/******************************************************************/
/* Class: KoAutoFormat						  */
/******************************************************************/
KoAutoFormat::KoAutoFormat( KoDocument *_doc, KoVariableCollection *_varCollection, KoVariableFormatCollection *_varFormatCollection )
    : m_doc( _doc ),
      m_varCollection(_varCollection),
      m_varFormatCollection(_varFormatCollection),
      m_configRead( false ),
      m_convertUpperCase( false ), m_convertUpperUpper( false ),
      m_advancedAutoCorrect( true ),
      m_autoDetectUrl( false ),
      m_ignoreDoubleSpace( false ),
      m_removeSpaceBeginEndLine( false ),
      m_useBulletStyle(false),
      m_autoChangeFormat(false),
      m_autoReplaceNumber(false),
      m_useAutoNumberStyle(false),
      m_completion(false),
      m_completionAppendSpace(false),
      m_addCompletionWord(true),
      m_includeTwoUpperLetterException(false),
      m_includeAbbreviation(false),
      m_ignoreUpperCase(false),
      m_bAutoFormatActive(true),
      m_bAutoSuperScript( false ),
      m_bulletStyle(),
      m_typographicSimpleQuotes(),
      m_typographicDoubleQuotes(),
      m_typographicDefaultDoubleQuotes(),
      m_typographicDefaultSimpleQuotes(),
      m_listCompletion( new KCompletion ),
      m_entries(),
      m_superScriptEntries(),
      m_upperCaseExceptions(),
      m_twoUpperLetterException(),
      m_maxFindLength( 0 ),
      m_minCompletionWordLength( 5 ),
      m_nbMaxCompletionWord( 500 )
{
    //load once this list not each time that we "readConfig"
    loadListOfWordCompletion();
}

KoAutoFormat::KoAutoFormat( const KoAutoFormat& format )
    : m_doc( format.m_doc ),
      m_varCollection( format.m_varCollection ),
      m_varFormatCollection( format.m_varFormatCollection ),
      m_configRead( format.m_configRead ),
      m_convertUpperCase( format.m_convertUpperCase ),
      m_convertUpperUpper( format.m_convertUpperUpper ),
      m_advancedAutoCorrect( format.m_advancedAutoCorrect ),
      m_autoDetectUrl( format.m_autoDetectUrl ),
      m_ignoreDoubleSpace( format.m_ignoreDoubleSpace ),
      m_removeSpaceBeginEndLine( format.m_removeSpaceBeginEndLine ),
      m_useBulletStyle( format.m_useBulletStyle ),
      m_autoChangeFormat( format.m_autoChangeFormat ),
      m_autoReplaceNumber( format.m_autoReplaceNumber ),
      m_useAutoNumberStyle( format.m_useAutoNumberStyle ),
      m_completion( format.m_completion ),
      m_completionAppendSpace( format.m_completionAppendSpace ),
      m_addCompletionWord( format.m_addCompletionWord ),
      m_includeTwoUpperLetterException( format.m_includeTwoUpperLetterException ),
      m_includeAbbreviation( format.m_includeAbbreviation ),
      m_ignoreUpperCase( format.m_ignoreUpperCase ),
      m_bAutoFormatActive( format.m_bAutoFormatActive ),
      m_bAutoSuperScript( format.m_bAutoSuperScript ),
      m_bulletStyle( format.m_bulletStyle ),
      m_typographicSimpleQuotes( format.m_typographicSimpleQuotes ),
      m_typographicDoubleQuotes( format.m_typographicDoubleQuotes ),
      m_typographicDefaultDoubleQuotes( format.m_typographicDefaultDoubleQuotes),
      m_typographicDefaultSimpleQuotes( format.m_typographicDefaultSimpleQuotes),
      m_listCompletion( 0L ), // don't copy it!
      m_entries( format.m_entries ),
      m_superScriptEntries ( format.m_superScriptEntries ),
      m_upperCaseExceptions( format.m_upperCaseExceptions ),
      m_twoUpperLetterException( format.m_twoUpperLetterException ),
      m_maxFindLength( format.m_maxFindLength ),
      m_minCompletionWordLength( format.m_minCompletionWordLength ),
      m_nbMaxCompletionWord( format.m_nbMaxCompletionWord )
{
    //m_listCompletion=new KCompletion();
    //m_listCompletion->setItems( autoFormat.listCompletion() );
    //copyAutoFormatEntries( autoFormat );
}

KoAutoFormat::~KoAutoFormat()
{
    delete m_listCompletion;
}

void KoAutoFormat::loadListOfWordCompletion()
{
    KConfig config("kofficerc");
    KConfigGroupSaver cgs( &config, "Completion Word" );
    m_listCompletion->insertItems(config.readListEntry( "list" ));
}

void KoAutoFormat::readConfig()
{
    // Read the autoformat configuration
    // This is done on demand (when typing the first char, or when opening the config dialog)
    // so that loading is faster and to avoid doing it for readonly documents.
    if ( m_configRead )
        return;
    KConfig config("kofficerc");
    KConfigGroupSaver cgs( &config, "AutoFormat" );
    m_convertUpperCase = config.readBoolEntry( "ConvertUpperCase", false );
    m_convertUpperUpper = config.readBoolEntry( "ConvertUpperUpper", false );
    m_includeTwoUpperLetterException = config.readBoolEntry( "includeTwoLetterException", false );
    m_includeAbbreviation = config.readBoolEntry( "includeAbbreviation", false );

    m_advancedAutoCorrect = config.readBoolEntry( "AdvancedAutocorrect", true );
    m_autoDetectUrl = config.readBoolEntry("AutoDetectUrl",false);
    m_ignoreDoubleSpace = config.readBoolEntry("IgnoreDoubleSpace",false);
    m_removeSpaceBeginEndLine = config.readBoolEntry("RemoveSpaceBeginEndLine",false);

    m_useBulletStyle = config.readBoolEntry("UseBulletStyle",false);
    QString tmp = config.readEntry( "BulletStyle", "" );
    m_bulletStyle = tmp.isEmpty() ? QChar() : tmp[0];

    m_autoChangeFormat = config.readBoolEntry( "AutoChangeFormat", false );

    m_autoReplaceNumber = config.readBoolEntry( "AutoReplaceNumber", false );

    m_useAutoNumberStyle = config.readBoolEntry( "AutoNumberStyle", false );


    QString beginDoubleQuote = config.readEntry( "TypographicQuotesBegin" );
    QString endDoubleQuote = config.readEntry( "TypographicQuotesEnd" );

    m_typographicDoubleQuotes.replace = config.readBoolEntry( "TypographicQuotesEnabled", false );

    QString begin = config.readEntry( "TypographicSimpleQuotesBegin" );
    QString end = config.readEntry( "TypographicSimpleQuotesEnd" );
    m_typographicSimpleQuotes.replace = config.readBoolEntry( "TypographicSimpleQuotesEnabled", false );

    m_bAutoSuperScript = config.readBoolEntry( "AutoSuperScript", false );

    config.setGroup( "completion" );
    m_completion = config.readBoolEntry( "completion", false );

    m_completionAppendSpace = config.readBoolEntry( "CompletionAppendSpace", false );
    m_minCompletionWordLength = config.readUnsignedNumEntry( "CompletionMinWordLength", 5 );
    m_nbMaxCompletionWord = config.readUnsignedNumEntry( "NbMaxCompletionWord", 100 );
    m_addCompletionWord = config.readBoolEntry( "AddCompletionWord", true );


    Q_ASSERT( m_entries.isEmpty() ); // readConfig is only called once...
    config.setGroup( "AutoFormatEntries" );

    bool fileNotFound = false;
    QFile xmlFile;
    KLocale klocale(m_doc->instance()->instanceName());

    xmlFile.setName(locate( "data", "koffice/autocorrect/" + klocale.languageList().front() + ".xml", m_doc->instance() ));

    if(!xmlFile.open(IO_ReadOnly)) {
        xmlFile.setName(locate( "data", "koffice/autocorrect/autocorrect.xml", m_doc->instance() ));
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
              m_entries.insert( nl.item(i).toElement().attribute("find"), KoAutoFormatEntry(nl.item(i).toElement().attribute("replace")) );
          }
      }

      QDomElement upper = de.namedItem( "UpperCaseExceptions" ).toElement();
      if(!upper.isNull())
      {
          QDomNodeList nl = upper.childNodes();
          for(uint i = 0; i < nl.count(); i++)
          {
              m_upperCaseExceptions+= nl.item(i).toElement().attribute("exception");
          }
      }

      QDomElement twoUpper = de.namedItem( "TwoUpperLetterExceptions" ).toElement();
      if(!twoUpper.isNull())
      {
          QDomNodeList nl = twoUpper.childNodes();
          for(uint i = 0; i < nl.count(); i++)
          {
              m_twoUpperLetterException+= nl.item(i).toElement().attribute("exception");
          }
      }

      QDomElement superScript = de.namedItem( "SuperScript" ).toElement();
      if(!superScript.isNull())
      {
          QDomNodeList nl = superScript.childNodes();
          for(uint i = 0; i < nl.count() ; i++) {
              //bug in qmap we overwrite = false doesn't work
              //so we can't add multiple "othernb"
              m_superScriptEntries.insert( nl.item(i).toElement().attribute("find"), KoAutoFormatEntry(nl.item(i).toElement().attribute("super")),FALSE );
          }
      }

      QDomElement doubleQuote = de.namedItem( "DoubleQuote" ).toElement();
      if(!doubleQuote.isNull())
      {
          QDomNodeList nl = doubleQuote.childNodes();
          m_typographicDefaultDoubleQuotes.begin =  nl.item(0).toElement().attribute("begin")[0];
          m_typographicDefaultDoubleQuotes.end = nl.item(0).toElement().attribute("end")[0];
      }
      QDomElement simpleQuote = de.namedItem( "SimpleQuote" ).toElement();
      if(!simpleQuote.isNull())
      {
          QDomNodeList nl = simpleQuote.childNodes();
          m_typographicDefaultSimpleQuotes.begin =  nl.item(0).toElement().attribute("begin")[0];
          m_typographicDefaultSimpleQuotes.end = nl.item(0).toElement().attribute("end")[0];
      }
    }

    if( beginDoubleQuote.isEmpty())
    {
        if( m_typographicDefaultDoubleQuotes.begin.isNull())
            m_typographicDoubleQuotes.begin = QChar('�');
        else
            m_typographicDoubleQuotes.begin = m_typographicDefaultDoubleQuotes.begin;
    }
    else
        m_typographicDoubleQuotes.begin = beginDoubleQuote[0];

    if( endDoubleQuote.isEmpty() )
    {
        if( m_typographicDefaultDoubleQuotes.end.isNull())
            m_typographicDoubleQuotes.end = QChar('�');
        else
            m_typographicDoubleQuotes.end = m_typographicDefaultDoubleQuotes.end;
    }
    else
        m_typographicDoubleQuotes.end = endDoubleQuote[0];

    m_typographicDoubleQuotes.replace = m_typographicDoubleQuotes.replace
                                  && !m_typographicDoubleQuotes.begin.isNull()
                                  && !m_typographicDoubleQuotes.end.isNull();


    if( begin.isEmpty())
    {
        if( m_typographicDefaultSimpleQuotes.begin.isNull())
            m_typographicSimpleQuotes.begin = QChar('\'');
        else
            m_typographicSimpleQuotes.begin = m_typographicDefaultSimpleQuotes.begin;
    }
    else
        m_typographicSimpleQuotes.begin = begin[0];

    if( end.isEmpty() )
    {
        if( m_typographicDefaultSimpleQuotes.end.isNull())
            m_typographicSimpleQuotes.end = QChar('\'');
        else
            m_typographicSimpleQuotes.end = m_typographicDefaultSimpleQuotes.end;
    }
    else
        m_typographicSimpleQuotes.end = end[0];

    m_typographicSimpleQuotes.replace = m_typographicSimpleQuotes.replace
                                  && !m_typographicSimpleQuotes.end.isNull()
                                  && !m_typographicSimpleQuotes.begin.isNull();


    xmlFile.close();
    buildMaxLen();
    autoFormatIsActive();
    m_configRead = true;
}

void KoAutoFormat::saveConfig()
{
    KConfig config("kofficerc");
    KLocale klocale(m_doc->instance()->instanceName());
    KConfigGroupSaver cgs( &config, "AutoFormat" );
    config.writeEntry( "ConvertUpperCase", m_convertUpperCase );
    config.writeEntry( "ConvertUpperUpper", m_convertUpperUpper );
    config.writeEntry( "includeTwoLetterException", m_includeTwoUpperLetterException );
    config.writeEntry( "includeAbbreviation", m_includeAbbreviation );

    config.writeEntry( "TypographicQuotesBegin", QString( m_typographicDoubleQuotes.begin ) );
    config.writeEntry( "TypographicQuotesEnd", QString( m_typographicDoubleQuotes.end ) );
    config.writeEntry( "TypographicQuotesEnabled", m_typographicDoubleQuotes.replace );
    config.writeEntry( "TypographicSimpleQuotesBegin", QString( m_typographicSimpleQuotes.begin ) );
    config.writeEntry( "TypographicSimpleQuotesEnd", QString( m_typographicSimpleQuotes.end ) );
    config.writeEntry( "TypographicSimpleQuotesEnabled", m_typographicSimpleQuotes.replace );

    config.writeEntry( "AdvancedAutocorrect", m_advancedAutoCorrect );
    config.writeEntry( "AutoDetectUrl",m_autoDetectUrl);

    config.writeEntry( "IgnoreDoubleSpace",m_ignoreDoubleSpace );
    config.writeEntry( "RemoveSpaceBeginEndLine",m_removeSpaceBeginEndLine );

    config.writeEntry( "UseBulletStyle", m_useBulletStyle);
    config.writeEntry( "BulletStyle", QString(m_bulletStyle));

    config.writeEntry( "AutoChangeFormat", m_autoChangeFormat);

    config.writeEntry( "AutoReplaceNumber", m_autoReplaceNumber);

    config.writeEntry( "AutoNumberStyle", m_useAutoNumberStyle );

    config.writeEntry( "AutoSuperScript", m_bAutoSuperScript );

    config.setGroup( "completion" );
    config.writeEntry( "completion", m_completion );
    config.writeEntry( "CompletionAppendSpace", m_completionAppendSpace );
    config.writeEntry( "CompletionMinWordLength", m_minCompletionWordLength);
    config.writeEntry( "NbMaxCompletionWord", m_nbMaxCompletionWord);
    config.writeEntry( "AddCompletionWord", m_addCompletionWord );



    config.setGroup( "AutoFormatEntries" );
    KoAutoFormatEntryMap::Iterator it = m_entries.begin();

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
    for ( QStringList::Iterator it = m_upperCaseExceptions.begin(); it != m_upperCaseExceptions.end();++it )
    {
	data = doc.createElement("word");
	data.setAttribute("exception",(*it) );
	upper.appendChild(data);
    }
    begin.appendChild(upper);

    QDomElement twoUpper;
    twoUpper = doc.createElement("TwoUpperLetterExceptions");

    for ( QStringList::Iterator it = m_twoUpperLetterException.begin(); it != m_twoUpperLetterException.end();++it )
    {
	data = doc.createElement("word");
	data.setAttribute("exception",(*it) );
	twoUpper.appendChild(data);
    }
    begin.appendChild(twoUpper);

    QDomElement super;
    super = doc.createElement("SuperScript");
    KoAutoFormatEntryMap::Iterator it2 = m_superScriptEntries.begin();
    for ( ; it2 != m_superScriptEntries.end() ; ++it2 )
    {
	data = doc.createElement("superscript");
	data.setAttribute("find", it2.key());
	data.setAttribute("super", it2.data().replace());
	super.appendChild(data);
    }
    begin.appendChild(super);

    QDomElement doubleQuote;
    doubleQuote = doc.createElement("DoubleQuote");
    data = doc.createElement("doublequote");
    data.setAttribute("begin", QString(m_typographicDefaultDoubleQuotes.begin));
    data.setAttribute("end", QString(m_typographicDefaultDoubleQuotes.end));
    doubleQuote.appendChild(data);
    begin.appendChild(doubleQuote);


    QDomElement simpleQuote;
    simpleQuote = doc.createElement("simpleQuote");
    data = doc.createElement("simplequote");
    data.setAttribute("begin", QString(m_typographicDefaultSimpleQuotes.begin));
    data.setAttribute("end", QString(m_typographicDefaultSimpleQuotes.end));
    simpleQuote.appendChild(data);
    begin.appendChild(simpleQuote);


    QFile f(locateLocal("data", "koffice/autocorrect/"+klocale.languageList().front() + ".xml",m_doc->instance()));
    if(!f.open(IO_WriteOnly)) {
        kdWarning()<<"Error during saving autoformat to " << f.name() << endl;
	return;
    }
    QTextStream ts(&f);
    doc.save(ts, 2);
    f.close();
    autoFormatIsActive();
    config.sync();
}

QString KoAutoFormat::getLastWord(KoTextParag *parag, int index)
{
    QString lastWord;
    KoTextString *s = parag->string();
    for ( int i = index - 1; i >= 0; --i )
    {
        QChar ch = s->at( i ).c;
        if ( ch.isSpace() || ch.isPunct() )
            break;
        lastWord.prepend( ch );
    }
    return lastWord;
}

QString KoAutoFormat::getWordAfterSpace(KoTextParag *parag, int index)
{
    QString word;
    KoTextString *s = parag->string();
    for ( int i = index - 1; i >= 0; --i )
    {
        QChar ch = s->at( i ).c;
        if ( ch.isSpace() )
            break;
        word.prepend( ch );
    }
    return word;

}

void KoAutoFormat::doCompletion( KoTextCursor* textEditCursor, KoTextParag *parag, int index, KoTextObject *txtObj )
{
    if( m_completion )
    {
        QString lastWord = getLastWord(parag, index+1);
        QString word=m_listCompletion->makeCompletion( lastWord.lower() );
        if( !word.isEmpty() && word!=lastWord )
        {
            unsigned int length = lastWord.length();
            int start = index+1 - length;
            KMacroCommand *macro = new KMacroCommand( i18n("Completion word"));

            KoTextCursor cursor( parag->document() );
            cursor.setParag( parag );
            cursor.setIndex( start );
            KoTextDocument * textdoc = parag->textDocument();
            word=lastWord+word.right(word.length()-lastWord.length());
            if( m_completionAppendSpace )
                word+=" ";
            textdoc->setSelectionStart( KoTextObject::HighlightSelection, &cursor );
            cursor.setIndex( start + length );
            textdoc->setSelectionEnd( KoTextObject::HighlightSelection, &cursor );

            macro->addCommand( txtObj->replaceSelectionCommand( textEditCursor, word,
                                                              KoTextObject::HighlightSelection,
                                                              i18n("Completion word") ));
            if ( m_completionAppendSpace && !m_ignoreUpperCase && (m_convertUpperUpper || m_convertUpperCase) )
            {
                int newPos = word.length() + index - 3;
                lastWord = getLastWord(parag, newPos);
                KCommand *cmd = doUpperCase( textEditCursor, parag, newPos, lastWord, txtObj );
                if( cmd )
                    macro->addCommand( cmd );
            }
            txtObj->emitNewCommand( macro );

            // The space/tab/CR that we inserted is still there but delete/insert moved the cursor
            // -> go right
            txtObj->emitHideCursor();
            textEditCursor->gotoRight();
            txtObj->emitShowCursor();
        }
    }
}

void KoAutoFormat::autoFormatIsActive()
{
    m_bAutoFormatActive = m_useBulletStyle ||
                          m_removeSpaceBeginEndLine ||
                          m_autoDetectUrl ||
                          m_convertUpperUpper ||
                          m_convertUpperCase ||
                          m_autoReplaceNumber ||
                          m_autoChangeFormat ||
                          m_completion ||
                          m_typographicDoubleQuotes.replace ||
                          m_typographicSimpleQuotes.replace ||
       m_entries.count()!=0;
}

void KoAutoFormat::doAutoFormat( KoTextCursor* textEditCursor, KoTextParag *parag, int index, QChar ch,KoTextObject *txtObj )
{
    if ( !m_configRead )
        readConfig();

    if ( !m_bAutoFormatActive )
        return;

    if( ch.isSpace())
    {
        //a link doesn't have a space
        //=>m_ignoreUpperCase = false
        m_ignoreUpperCase=false;

        QString word=getWordAfterSpace(parag,index);

        if ( m_autoChangeFormat && index > 3)
        {
            doAutoChangeFormat( textEditCursor, parag,index, word, txtObj );

        }
        if ( m_autoDetectUrl && index > 0 )
        {
            doAutoDetectUrl( textEditCursor, parag,index, word, txtObj );
        }
        if ( m_autoReplaceNumber )
            doAutoReplaceNumber( textEditCursor, parag, index, word, txtObj );
    }

    if( ch =='\n' )
    {
        if( m_removeSpaceBeginEndLine && index > 1)
            doRemoveSpaceBeginEndLine( textEditCursor, parag, txtObj );
        if( m_useBulletStyle  && index > 3)
            doUseBulletStyle( textEditCursor, parag, txtObj, index );
        if( m_useAutoNumberStyle && index > 3 )
            doUseNumberStyle( textEditCursor, parag, txtObj, index );
        if( m_convertUpperUpper && m_includeTwoUpperLetterException )
            doAutoIncludeUpperUpper(textEditCursor, parag, txtObj );
        if( m_convertUpperCase && m_includeAbbreviation )
            doAutoIncludeAbbreviation(textEditCursor, parag, txtObj );
    }

    //kdDebug() << "KoAutoFormat::doAutoFormat ch=" << QString(ch) << endl;
    //if ( !m_enabled )
    //    return;
    // Auto-correction happens when pressing space, tab, CR, punct etc.
    if ( ( ch.isSpace() || ch.isPunct() ) && index > 0 )
    {
        QString lastWord = getLastWord(parag, index);
        //kdDebug()<<" m_listCompletion->items() :"<<m_listCompletion->items()<<endl;
        if( m_addCompletionWord && m_listCompletion->items().count() < m_nbMaxCompletionWord
            && lastWord.length()>= m_minCompletionWordLength )
            m_listCompletion->addItem( lastWord.lower() );

        detectStartOfLink(lastWord);
        //kdDebug() << "KoAutoFormat::doAutoFormat lastWord=" << lastWord << endl;
        KMacroCommand *macro =new KMacroCommand(i18n("Autocorrection"));
        bool cmdCreate=false;
        int newPos = index;
        KCommand *cmd = doAutoCorrect( textEditCursor, parag, newPos , txtObj );

        if( cmd )
        {
            macro->addCommand( cmd );
            cmdCreate = true;
        }

        if ( !m_ignoreUpperCase && (m_convertUpperUpper || m_convertUpperCase) )
        {
            lastWord = getLastWord(parag, newPos);
            cmd = doUpperCase( textEditCursor, parag, newPos, lastWord, txtObj );
            if( cmd )
            {
                macro->addCommand( cmd );
                cmdCreate = true;
            }
        }

        if ( cmdCreate )
            txtObj->emitNewCommand( macro );
        else
            delete macro;

        if( m_bAutoSuperScript && m_superScriptEntries.count()>0)
            doAutoSuperScript( textEditCursor, parag, newPos, lastWord, txtObj  );


    }
    if ( ch == '"' && m_typographicDoubleQuotes.replace )
    {
        doTypographicQuotes( textEditCursor, parag, index,txtObj, true /*double quote*/ );
    }
    else if ( ch == '\'' && m_typographicDoubleQuotes.replace )
    {
        doTypographicQuotes( textEditCursor, parag, index,txtObj, false /* simple quote*/ );
    }
}

KCommand *KoAutoFormat::doAutoCorrect( KoTextCursor* textEditCursor, KoTextParag *parag, int &index, KoTextObject *txtObj )
{
    if(!m_advancedAutoCorrect)
        return 0L;
    // Prepare an array with words of different lengths, all terminating at "index".
    // Obviously only full words are put into the array
    // But this allows 'find strings' with spaces and punctuation in them.
    QString * wordArray = new QString[m_maxFindLength+1];
    {
        QString word;
        KoTextString *s = parag->string();
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
    KoTextDocument * textdoc = parag->textDocument();

    // Now for each entry in the autocorrect list, look if
    // the word of the same size in wordArray matches.
    // This allows an o(n) behaviour instead of an o(n^2).
    for(int i=m_maxFindLength;i>0;--i)
    {
        KoAutoFormatEntryMap::ConstIterator it = m_entries.find((wordArray[i].lower()));
        if ( wordArray[i]!=0 && it!=m_entries.end() )
        {
            unsigned int length = wordArray[i].length();
            int start = index - length;
            KoTextCursor cursor( parag->document() );
            cursor.setParag( parag );
            cursor.setIndex( start );
            textdoc->setSelectionStart( KoTextObject::HighlightSelection, &cursor );
            cursor.setIndex( start + length );
            textdoc->setSelectionEnd( KoTextObject::HighlightSelection, &cursor );
            KCommand *cmd = txtObj->replaceSelectionCommand( textEditCursor, it.data().replace(),
                                                             KoTextObject::HighlightSelection,
                                                             i18n("Autocorrect word") );
            // The space/tab/CR that we inserted is still there but delete/insert moved the cursor
            // -> go right
            txtObj->emitHideCursor();
            textEditCursor->gotoRight();
            txtObj->emitShowCursor();
            delete [] wordArray;
            index = index - length +it.data().replace().length();
            return cmd;
        }
    }
    delete [] wordArray;
    return 0L;
}

void KoAutoFormat::doTypographicQuotes( KoTextCursor* textEditCursor, KoTextParag *parag, int index, KoTextObject *txtObj, bool doubleQuotes )
{
    //kdDebug() << "KoAutoFormat::doTypographicQuotes" << endl;
    KoTextDocument * textdoc = parag->textDocument();
    KoTextCursor cursor( parag->document() );
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
    {
        if( doubleQuotes )
            replacement = m_typographicDoubleQuotes.end;
        else
            replacement = m_typographicSimpleQuotes.end;
    }
    else
    {
        if( doubleQuotes )
            replacement = m_typographicDoubleQuotes.begin;
        else
            replacement = m_typographicSimpleQuotes.begin;
    }
    txtObj->emitNewCommand(txtObj->replaceSelectionCommand( textEditCursor, replacement,
                              KoTextObject::HighlightSelection,
                              i18n("Typographic quote") ));
}

KCommand * KoAutoFormat::doUpperCase( KoTextCursor *textEditCursor, KoTextParag *parag,
                                int index, const QString & word, KoTextObject *txtObj )
{
    KoTextDocument * textdoc = parag->textDocument();
    unsigned int length = word.length();
    int start = index - length;
    KoTextCursor backCursor( parag->document() );
    backCursor.setParag( parag );
    backCursor.setIndex( start );

    // backCursor now points at the first char of the word
    QChar firstChar = backCursor.parag()->at( backCursor.index() )->c;
    bool bNeedMove = false;
    KCommand *cmd = 0L;
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
            beginningOfSentence = isMark( backCursor.parag()->at( backCursor.index() )->c ) && backCursor.parag()->at( backCursor.index()+1 )->c.isSpace();

        // Now look for exceptions
        if ( beginningOfSentence )
        {
            QChar punct = backCursor.parag()->at( backCursor.index() )->c;
            QString text = getLastWord( backCursor.parag(), backCursor.index() )
                           + punct;
            // text has the word at the end of the 'sentence', including the termination. Example: "Mr."
            beginningOfSentence = (m_upperCaseExceptions.findIndex(text)==-1); // Ok if we can't find it
        }

        if ( beginningOfSentence )
        {
            KoTextCursor cursor( parag->document() );
            cursor.setParag( parag );
            cursor.setIndex( start );
            textdoc->setSelectionStart( KoTextObject::HighlightSelection, &cursor );
            cursor.setIndex( start + 1 );
            textdoc->setSelectionEnd( KoTextObject::HighlightSelection, &cursor );
            cmd = txtObj->replaceSelectionCommand( textEditCursor, QString( firstChar.upper() ),
                                      KoTextObject::HighlightSelection,
                                      i18n("Autocorrect (capitalize first letter)") );
            bNeedMove = true;
        }
    }
    else if ( m_convertUpperUpper && isUpper( firstChar ) && length > 2 )
    {
        backCursor.setIndex( backCursor.index() + 1 );
        QChar secondChar = backCursor.parag()->at( backCursor.index() )->c;
        //kdDebug()<<" secondChar :"<<secondChar<<endl;
        if ( isUpper( secondChar ) )
        {
            // Check next letter - we still want to be able to write fully uppercase words...
            backCursor.setIndex( backCursor.index() + 1 );
            QChar thirdChar = backCursor.parag()->at( backCursor.index() )->c;
            if ( isLower( thirdChar ) && (m_twoUpperLetterException.findIndex(word)==-1))
            {
                // Ok, convert
                KoTextCursor cursor( parag->document() );
                cursor.setParag( parag );
                cursor.setIndex( start + 1 ); // After all the first letter's fine, so only change the second letter
                textdoc->setSelectionStart( KoTextObject::HighlightSelection, &cursor );
                cursor.setIndex( start + 2 );
                textdoc->setSelectionEnd( KoTextObject::HighlightSelection, &cursor );

                QString replacement = word[1].lower();
                cmd = txtObj->replaceSelectionCommand( textEditCursor, replacement,KoTextObject::HighlightSelection,i18n("Autocorrect (Convert two Upper Case letters to one Upper Case and one Lower Case letter.)") );

                bNeedMove = true;
            }
        }
    }
    if ( bNeedMove )
    {
        txtObj->emitHideCursor();
        textEditCursor->setParag( parag );
        textEditCursor->setIndex( index );
        textEditCursor->gotoRight(); // not the same thing as index+1, in case of CR
        txtObj->emitShowCursor();
    }
    return cmd;
}

void KoAutoFormat::doAutoReplaceNumber( KoTextCursor* textEditCursor, KoTextParag *parag, int index, const QString & word , KoTextObject *txtObj )
{
    unsigned int length = word.length();
    if ( length != 3 )
        return;
    KoTextDocument * textdoc = parag->textDocument();
    int start = index - length;
    if( word == QString("1/2") || word == QString("1/4") || word == QString("3/4") )
    {
        KoTextCursor cursor( parag->document() );
        cursor.setParag( parag );
        cursor.setIndex( start );
        textdoc->setSelectionStart( KoTextObject::HighlightSelection, &cursor );
        cursor.setIndex( start + length );
        textdoc->setSelectionEnd( KoTextObject::HighlightSelection, &cursor );
        QString replacement;
        if( word == QString("1/2") )
            replacement=QString("�");
        else if (word == QString("1/4") )
            replacement=QString("�");
        else if (word == QString("3/4") )
            replacement=QString("�");
        QString cmdName=i18n("Autocorrect (replace 1/2... with ")+QString("�...)");
        txtObj->emitNewCommand(txtObj->replaceSelectionCommand( textEditCursor, replacement,
                                                           KoTextObject::HighlightSelection,
                                                                cmdName ));
        txtObj->emitHideCursor();
        textEditCursor->gotoRight();
        txtObj->emitShowCursor();
    }

}

void KoAutoFormat::detectStartOfLink(const QString &word)
{
    if (word.find("http")!=-1 || word.find("mailto")!=-1
        || word.find("ftp")!=-1 || word.find("file")!=-1
        || word.find("news")!=-1 )
        m_ignoreUpperCase=true;
}

void KoAutoFormat::doAutoDetectUrl( KoTextCursor *textEditCursor, KoTextParag *parag,int index, const QString & word, KoTextObject *txtObj )
{
    if (word.find("http://")!=-1 || word.find("mailto:")!=-1
        || word.find("ftp://")!=-1 || word.find("file:")!=-1
        || word.find("news:")!=-1)
    {
        unsigned int length = word.length();
        int start = index - length;
        KoTextCursor cursor( parag->document() );
        KoTextDocument * textdoc = parag->textDocument();
        cursor.setParag( parag );
        cursor.setIndex( start );
        textdoc->setSelectionStart( KoTextObject::HighlightSelection, &cursor );
        cursor.setIndex( start + length );
        textdoc->setSelectionEnd( KoTextObject::HighlightSelection, &cursor );
        KoVariable *var=new KoLinkVariable( textdoc, word, word ,m_varFormatCollection->format( "STRING" ), m_varCollection );

        CustomItemsMap customItemsMap;
        customItemsMap.insert( 0, var );
        KoTextFormat * lastFormat = parag->at( start )->format();
        txtObj->insert( textEditCursor, lastFormat, KoTextObject::customItemChar(), false, true, i18n("Insert Variable"), customItemsMap,KoTextObject::HighlightSelection );
        txtObj->emitHideCursor();
        textEditCursor->gotoRight();
        txtObj->emitShowCursor();
    }

}

void KoAutoFormat::doAutoIncludeUpperUpper(KoTextCursor* /*textEditCursor*/, KoTextParag *parag, KoTextObject* /*txtObj*/ )
{
    KoTextString *s = parag->string();

    if( s->length() < 2 )
        return;

    for (int i=0; i<=(s->length() - 1);i++)
    {
        QString word;
        for ( int j = i ; j < s->length() - 1; j++ )
        {
            QChar ch = s->at( j ).c;
            if ( ch.isSpace() )
                break;
            word.append( ch );
        }
        if( word.length() > 2 && word.left(2)==word.left(2).upper() && word.at(3)!=word.at(3).upper() )
        {
            if ( m_twoUpperLetterException.findIndex(word )==-1)
                m_twoUpperLetterException.append( word);
        }
        i+=word.length();
    }

}


void KoAutoFormat::doAutoIncludeAbbreviation(KoTextCursor* /*textEditCursor*/, KoTextParag *parag, KoTextObject* /*txtObj*/ )
{
    KoTextString *s = parag->string();

    if( s->length() < 2 )
        return;
    for (int i=0; i<=(s->length() - 1);i++)
    {
        QString wordAfter;
        QString word;

        for ( int j = i ; j < s->length() - 1; j++ )
        {
            QChar ch = s->at( j ).c;
            if ( ch.isSpace() )
                break;
            word.append( ch );
        }
        if ( isMark( word.at(word.length()-1)) )
        {
            for ( int j = i+word.length()+1 ; j < s->length() - 1; j++ )
            {
                QChar ch = s->at( j ).c;
                if ( ch.isSpace() )
                    break;
                wordAfter.append( ch );
            }
            if( word.length()>1 && !wordAfter.isEmpty() && wordAfter.at(0)==wordAfter.at(0).lower())
            {
                if ( m_upperCaseExceptions.findIndex(word )==-1)
                    m_upperCaseExceptions.append( word );
            }
        }
        i+=word.length();
        if( !wordAfter.isEmpty())
        {
            i+=wordAfter.length()+1;
        }
    }

}


void KoAutoFormat::doAutoChangeFormat( KoTextCursor *textEditCursor, KoTextParag *parag,int index, const QString & word, KoTextObject *txtObj )
{
    bool underline = (word.at(0)=='_' && word.at(word.length()-1)=='_');
    bool bold = (word.at(0)=='*' && word.at(word.length()-1)=='*');
    if( bold || underline)
    {
        QString replacement=word.mid(1,word.length()-2);
        int start = index - word.length();
        KoTextDocument * textdoc = parag->textDocument();
        KMacroCommand *macro=new KMacroCommand(i18n("Autocorrection: change format."));
        KoTextCursor cursor( parag->document() );

        cursor.setParag( parag );
        cursor.setIndex( start );
        textdoc->setSelectionStart( KoTextObject::HighlightSelection, &cursor );
        cursor.setIndex( start + word.length() );
        textdoc->setSelectionEnd( KoTextObject::HighlightSelection, &cursor );
        macro->addCommand(txtObj->replaceSelectionCommand( textEditCursor, replacement,
                                                           KoTextObject::HighlightSelection,
                                                           i18n("Autocorrect word") ));

        KoTextFormat * lastFormat = parag->at( start )->format();
        KoTextFormat * newFormat = new KoTextFormat(*lastFormat);
        cursor.setIndex( start );
        textdoc->setSelectionStart( KoTextObject::HighlightSelection, &cursor );
        cursor.setIndex( start + word.length()-2 );
        textdoc->setSelectionEnd( KoTextObject::HighlightSelection, &cursor );

        if( bold)
        {
            newFormat->setBold(true);
            macro->addCommand(txtObj->setFormatCommand( textEditCursor, 0L, newFormat, KoTextFormat::Bold , false,KoTextObject::HighlightSelection  ));
        }
        else if( underline )
        {
            newFormat->setUnderline(true);
            macro->addCommand(txtObj->setFormatCommand( textEditCursor, 0L, newFormat, KoTextFormat::Underline , false,KoTextObject::HighlightSelection  ));
        }
        txtObj->emitNewCommand(macro);
        txtObj->emitHideCursor();
        textEditCursor->gotoRight();
        txtObj->emitShowCursor();
    }
}

void KoAutoFormat::doUseBulletStyle(KoTextCursor * /*textEditCursor*/, KoTextParag *parag, KoTextObject *txtObj, int& index )
{
    KoTextDocument * textdoc = parag->textDocument();
    KoTextCursor cursor( parag->document() );
    KoTextString *s = parag->string();
    QChar ch = s->at( 0 ).c;

    if( m_useBulletStyle && (ch =='*' || ch == '-' || ch =='+') && (s->at(1).c).isSpace())
    {
        KMacroCommand *macroCmd = new KMacroCommand( i18n("Autocorrect (use bullet style)"));
        cursor.setParag( parag );
        cursor.setIndex( 0 );
        textdoc->setSelectionStart( KoTextObject::HighlightSelection, &cursor );
        cursor.setParag( parag );
        cursor.setIndex( 2 );
        textdoc->setSelectionEnd( KoTextObject::HighlightSelection, &cursor );
        KCommand *cmd=txtObj->removeSelectedTextCommand( &cursor, KoTextObject::HighlightSelection  );
        // Adjust index
        index -= 2;
        if(cmd)
            macroCmd->addCommand(cmd);

        cursor.setParag( parag );
        cursor.setIndex( 0 );
        textdoc->setSelectionStart( KoTextObject::HighlightSelection, &cursor );

        cursor.setIndex( 2 );
        textdoc->setSelectionEnd( KoTextObject::HighlightSelection, &cursor );


        KoParagCounter c;
        if( m_bulletStyle.isNull() && ch == '*' )
        {
            c.setNumbering( KoParagCounter::NUM_LIST );
            c.setStyle( KoParagCounter::STYLE_DISCBULLET );
        }
        else
        {
            c.setNumbering( KoParagCounter::NUM_LIST );
            c.setStyle( KoParagCounter::STYLE_CUSTOMBULLET );
            if( ch == '*')
                c.setCustomBulletCharacter( m_bulletStyle );
            else if ( ch == '+')
                c.setCustomBulletCharacter( '+' );
            else if ( ch == '-' )
                c.setCustomBulletCharacter( '-' );

        }
        c.setSuffix(QString::null);
        cmd=txtObj->setCounterCommand( &cursor, c ,KoTextObject::HighlightSelection );
        if( cmd)
            macroCmd->addCommand(cmd);
        cursor.setParag( parag->next() );
        cursor.setIndex( 0 );
        textdoc->setSelectionStart( KoTextObject::HighlightSelection, &cursor );
        cursor.setIndex( 0 );
        textdoc->setSelectionEnd( KoTextObject::HighlightSelection, &cursor );
        cmd=txtObj->setCounterCommand( &cursor, c ,KoTextObject::HighlightSelection );
        if(cmd)
            macroCmd->addCommand(cmd);
        txtObj->emitNewCommand(macroCmd);
    }

}

void KoAutoFormat::doUseNumberStyle(KoTextCursor * /*textEditCursor*/, KoTextParag *parag, KoTextObject *txtObj, int& index )
{
    KoTextDocument * textdoc = parag->textDocument();
    KoTextCursor cursor( parag->document() );
    KoTextString *s = parag->string();
    QString word;
    for ( int i = 0 ; i < s->length() - 1; i++ )
    {
        QChar ch = s->at( i ).c;
        if ( ch.isSpace() )
            break;
        word.append( ch );
    }
    QChar punct=word[word.length()-1];
    if( punct.isPunct() )
    {
        QString number=word.mid(0,word.length()-1);
        bool ok;
        uint val=number.toUInt(&ok);
        if( ok )
        {
            KMacroCommand *macroCmd = new KMacroCommand( i18n("Autocorrect (use number style)"));
            cursor.setParag( parag );
            cursor.setIndex( 0 );
            textdoc->setSelectionStart( KoTextObject::HighlightSelection, &cursor );
            cursor.setParag( parag );
            cursor.setIndex( word.length()+1 );
            textdoc->setSelectionEnd( KoTextObject::HighlightSelection, &cursor );
            KCommand *cmd=txtObj->removeSelectedTextCommand( &cursor, KoTextObject::HighlightSelection  );
            // Adjust index
            index -= 2;
            if(cmd)
                macroCmd->addCommand(cmd);

            // Apply counter to this paragraph
            cursor.setParag( parag );
            cursor.setIndex( 0 );
            textdoc->setSelectionStart( KoTextObject::HighlightSelection, &cursor );

            cursor.setIndex( 2 );
            textdoc->setSelectionEnd( KoTextObject::HighlightSelection, &cursor );

            KoParagCounter c;
            c.setNumbering( KoParagCounter::NUM_LIST );
            c.setStyle( KoParagCounter::STYLE_NUM );
            c.setSuffix(QString( punct ));
            c.setStartNumber( (int)val);
            cmd=txtObj->setCounterCommand( &cursor, c ,KoTextObject::HighlightSelection );
            if( cmd)
                macroCmd->addCommand(cmd);
            // Apply counter to next paragraph too
            cursor.setParag( parag->next() );
            cursor.setIndex( 0 );
            textdoc->setSelectionStart( KoTextObject::HighlightSelection, &cursor );
            cursor.setIndex( 0 );
            textdoc->setSelectionEnd( KoTextObject::HighlightSelection, &cursor );
            cmd=txtObj->setCounterCommand( &cursor, c ,KoTextObject::HighlightSelection );
            if(cmd)
                macroCmd->addCommand(cmd);
            txtObj->emitNewCommand(macroCmd);
        }
    }
}


void KoAutoFormat::doRemoveSpaceBeginEndLine( KoTextCursor *textEditCursor, KoTextParag *parag, KoTextObject *txtObj )
{
    KoTextString *s = parag->string();
    bool refreshCursor=false;
    KoTextDocument * textdoc = parag->textDocument();
    KoTextCursor cursor( parag->document() );

    KMacroCommand *macroCmd = new KMacroCommand( i18n("Autocorrect (remove start and end line space)"));
    for ( int i = parag->string()->length()-1; i >= 0; --i )
    {
        QChar ch = s->at( i ).c;
        if( !ch.isSpace())
        {
            if( i == parag->string()->length()-1 )
                break;
            cursor.setParag( parag );
            cursor.setIndex( i+1 );
            textdoc->setSelectionStart( KoTextObject::HighlightSelection, &cursor );
            cursor.setParag( parag );
            cursor.setIndex( parag->string()->length() );
            textdoc->setSelectionEnd( KoTextObject::HighlightSelection, &cursor );
            KCommand *cmd=txtObj->replaceSelectionCommand( &cursor, "",KoTextObject::HighlightSelection , QString::null );

            if(cmd)
                macroCmd->addCommand(cmd);

            refreshCursor=true;
            break;
        }
    }

    s = parag->string();

    for ( int i = 0 ; i < parag->string()->length() ; i++ )
    {
        QChar ch = s->at( i ).c;
        if( !ch.isSpace())
        {
            if( i == 0 )
                break;

            cursor.setParag( parag );
            cursor.setIndex( 0 );
            textdoc->setSelectionStart( KoTextObject::HighlightSelection, &cursor );
            cursor.setParag( parag );
            cursor.setIndex( i  );
            textdoc->setSelectionEnd( KoTextObject::HighlightSelection, &cursor );
            KCommand *cmd=txtObj->replaceSelectionCommand( &cursor, "",KoTextObject::HighlightSelection , QString::null );

            if(cmd)
                macroCmd->addCommand(cmd);
            refreshCursor=true;
            break;
        }
    }

    if( refreshCursor)
    {
        txtObj->emitNewCommand(macroCmd);
        txtObj->emitHideCursor();
        textEditCursor->setParag( parag->next() );
        //textEditCursor->cursorgotoRight();
        txtObj->emitShowCursor();
    }
    else
        delete macroCmd;
}

void KoAutoFormat::doAutoSuperScript( KoTextCursor* textEditCursor, KoTextParag *parag, int index, const QString & word , KoTextObject *txtObj )
{
    KoAutoFormatEntryMap::Iterator it = m_superScriptEntries.begin();
    bool found = false;
    QString replace;
    for ( ; it != m_superScriptEntries.end() ; ++it )
    {
        if( it.key()==word)
        {
            replace = it.data().replace();
            found = true;
            break;
        }
        else if ( it.key()=="othernb")
        {
            QString tmp = it.data().replace();
            int pos = word.find( tmp );
            if( pos != -1)
            {
                if( pos + tmp.length() == word.length())
                {
                    bool ok;
                    word.left( pos ).toInt( &ok);
                    if( ok )
                    {
                        replace = tmp;
                        found = true;
                        break;
                    }
                }
            }
        }
    }
    if (found )
    {
        KoTextDocument * textdoc = parag->textDocument();

        int start = index - replace.length();
        KoTextFormat * lastFormat = parag->at( start )->format();
        KoTextFormat * newFormat = new KoTextFormat(*lastFormat);
        KoTextCursor cursor( parag->document() );

        cursor.setParag( parag );
        cursor.setIndex( start );
        textdoc->setSelectionStart( KoTextObject::HighlightSelection, &cursor );
        cursor.setIndex( start + word.length() -1 );
        textdoc->setSelectionEnd( KoTextObject::HighlightSelection, &cursor );
        newFormat->setVAlign(KoTextFormat::AlignSuperScript);
        txtObj->emitNewCommand( txtObj->setFormatCommand( textEditCursor, 0L, newFormat, KoTextFormat::VAlign , false,KoTextObject::HighlightSelection  ));
    }
}

bool KoAutoFormat::doIgnoreDoubleSpace( KoTextParag *parag, int index,QChar ch )
{
    if( m_ignoreDoubleSpace && ch==' ' && index >=  0 )
    {
        KoTextString *s = parag->string();
        QChar ch = s->at( index ).c;
        if ( ch==' ' )
            return true;
    }
    return false;
}

void KoAutoFormat::configTypographicSimpleQuotes( TypographicQuotes _tq )
{
    m_typographicSimpleQuotes = _tq;
}

void KoAutoFormat::configTypographicDoubleQuotes( TypographicQuotes _tq )
{
    m_typographicDoubleQuotes = _tq;
}

void KoAutoFormat::configUpperCase( bool _uc )
{
    m_convertUpperCase = _uc;
}

void KoAutoFormat::configUpperUpper( bool _uu )
{
    m_convertUpperUpper = _uu;
}

void KoAutoFormat::configAdvancedAutocorrect( bool _aa )
{
    m_advancedAutoCorrect = _aa;
}

void KoAutoFormat::configAutoDetectUrl(bool _au)
{
    m_autoDetectUrl=_au;
}

void KoAutoFormat::configIgnoreDoubleSpace( bool _ids)
{
    m_ignoreDoubleSpace=_ids;
}

void KoAutoFormat::configRemoveSpaceBeginEndLine( bool _space)
{
    m_removeSpaceBeginEndLine=_space;
}

void KoAutoFormat::configUseBulletStyle( bool _ubs)
{
    m_useBulletStyle=_ubs;
}

void KoAutoFormat::configBulletStyle( QChar b )
{
    m_bulletStyle = b;
}

void KoAutoFormat::configAutoChangeFormat( bool b)
{
    m_autoChangeFormat = b;
}


void KoAutoFormat::configAutoReplaceNumber( bool b )
{
    m_autoReplaceNumber = b;
}

void KoAutoFormat::configAutoNumberStyle( bool b )
{
    m_useAutoNumberStyle = b;
}

void KoAutoFormat::configCompletion( bool b )
{
    m_completion = b;
}

void KoAutoFormat::configAppendSpace( bool b)
{
    m_completionAppendSpace= b;
}

void KoAutoFormat::configMinWordLength( uint val )
{
   m_minCompletionWordLength = val;
}

void KoAutoFormat::configNbMaxCompletionWord( uint val )
{
    m_nbMaxCompletionWord = val;
}


void KoAutoFormat::configAddCompletionWord( bool b )
{
    m_addCompletionWord= b;
}

bool KoAutoFormat::isUpper( const QChar &c )
{
    return c.lower() != c;
}

bool KoAutoFormat::isLower( const QChar &c )
{
    // Note that this is not the same as !isUpper !
    // For instance '1' is not lower nor upper,
    return c.upper() != c;
}

bool KoAutoFormat::isMark( const QChar &c )
{
    return ( c == QChar( '.' ) ||
	     c == QChar( '?' ) ||
	     c == QChar( '!' ) );
}

bool KoAutoFormat::isSeparator( const QChar &c )
{
    return ( !c.isLetter() && !c.isNumber() && !c.isDigit() );
}

void KoAutoFormat::buildMaxLen()
{
    QMap< QString, KoAutoFormatEntry >::Iterator it = m_entries.begin();

    m_maxFindLength = 0;
    for ( ; it != m_entries.end(); ++it )
	m_maxFindLength = QMAX( m_maxFindLength, it.key().length() );
}

QStringList KoAutoFormat::listCompletion() const
{
   return m_listCompletion->items();
}


void KoAutoFormat::configIncludeTwoUpperUpperLetterException( bool b)
{
    m_includeTwoUpperLetterException = b;
}

void KoAutoFormat::configIncludeAbbreviation( bool b )
{
    m_includeAbbreviation = b;
}

void KoAutoFormat::configAutoSuperScript( bool b )
{
    m_bAutoSuperScript = b;
}
