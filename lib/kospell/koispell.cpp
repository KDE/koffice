/* This file is part of the KDE libraries
   Copyright (C) 2002-2003 Laurent Montel <lmontel@mandrakesoft.com>

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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <ctype.h>
#include <stdlib.h> // atoi

#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif

#include <qtextcodec.h>
#include <qtimer.h>
#include <kapplication.h>
#include <kdebug.h>
#include <klocale.h>

#include "koSpell.h"
#include "koSpelldlg.h"
#include "koispell.moc"
#include "koispell.h"
#include "koSconfig.h"

#include <kwin.h>
#include <kprocio.h>

#define MAXLINELENGTH 10000

enum {
	GOOD=     0,
	IGNORE=   1,
	REPLACE=  2,
	MISTAKE=  3
};


//TODO
//Parse stderr output
//e.g. -- invalid dictionary name

/*
  Things to put in KSpellConfigDlg:
    make root/affix combinations that aren't in the dictionary (-m)
    don't generate any affix/root combinations (-P)
    Report  run-together  words   with   missing blanks as spelling errors.  (-B)
    default dictionary (-d [dictionary])
    personal dictionary (-p [dictionary])
    path to ispell -- NO: ispell should be in $PATH
    */


//  Connects a slot to KProcIO's output signal
#define OUTPUT(x) (connect (proc, SIGNAL (readReady(KProcIO *)), this, SLOT (x(KProcIO *))))

// Disconnect a slot from...
#define NOOUTPUT(x) (disconnect (proc, SIGNAL (readReady(KProcIO *)), this, SLOT (x(KProcIO *))))



KOISpell::KOISpell (QWidget *_parent, const QString &_caption,
		QObject *obj, const char *slot, KOSpellConfig *_ksc,
		bool _progressbar, bool _modal)
    :KOSpell(_parent,_caption,_ksc,_modal,/*_autocorrect*/false)
{

  m_bIgnoreUpperWords=false;
  m_bIgnoreTitleCase=false;

  autoDelete = false;
  modaldlg = _modal;
  progressbar = _progressbar;

  proc=0;
  ksdlg=0;

  texmode=dlgon=FALSE;

  dialogsetup = FALSE;
  progres=10;
  curprog=0;

  dialogwillprocess=FALSE;
  dialog3slot="";

  personaldict=FALSE;
  dlgresult=-1;

  caption=_caption;

  parent=_parent;

  trystart=0;
  maxtrystart=2;

  if ( obj && slot )
      // caller wants to know when kspell is ready
      connect (this, SIGNAL (ready(KOSpell *)), obj, slot);
  else
      // Hack for modal spell checking
      connect (this, SIGNAL (ready(KOSpell *)), this, SLOT( slotModalReady() ) );
  proc=new KProcIO(codec);

  startIspell();
}

void KOISpell::startIspell()
  //trystart = {0,1,2}
{

  kdDebug(750) << "Try #" << trystart << endl;
  if (trystart>0)
    proc->resetAll();
#if 0 //fixem !!!!!!!!!!!!!
  switch (ksconfig->client())
    {
    case KS_CLIENT_ISPELL:
      *proc << "ispell";
      kdDebug(750) << "Using ispell" << endl;
      break;
    case KS_CLIENT_ASPELL:
      *proc << "aspell";
      kdDebug(750) << "Using aspell" << endl;
      break;
    }
#endif
  *proc << "ispell";
  kdDebug(750) << "Using ispell" << endl;

  // TODO: add option -h to ignore HTML (XML) code
  *proc << "-a" << "-S";
  if (ksconfig->noRootAffix())
    {
      *proc<<"-m";
    }
  if (ksconfig->runTogether())
    {
      *proc << "-B";
    }
  else
    {
      *proc << "-C";
    }

  if (trystart<2)
    {
      if (! ksconfig->dictionary().isEmpty())
	{
	  kdDebug(750) << "using dictionary [" << ksconfig->dictionary() << "]" << endl;
	  *proc << "-d";
	  *proc << ksconfig->dictionary();
	}
    }

  //Note to potential debuggers:  -Tlatin2 _is_ being added on the
  //  _first_ try.  But, some versions of ispell will fail with this
  // option, so kspell tries again without it.  That's why as 'ps -ax'
  // shows "ispell -a -S ..." withou the "-Tlatin2" option.

  if (trystart<1)
    switch (ksconfig->encoding())
      {
      case KOS_E_LATIN1:
	*proc << "-Tlatin1";
	break;
      case KOS_E_LATIN2:
	*proc << "-Tlatin2";
	break;
      case KOS_E_LATIN3:
        *proc << "-Tlatin3";
        break;

      // add the other charsets here
      case KOS_E_LATIN4:
      case KOS_E_LATIN5:
      case KOS_E_LATIN7:
      case KOS_E_LATIN8:
      case KOS_E_LATIN9:
      case KOS_E_LATIN13:
      case KOS_E_LATIN15:

	// will work, if this is the default charset in the dictionary
	kdError(750) << "charsets iso-8859-4 .. iso-8859-15 not supported yet" << endl;
	break;

      case KOS_E_UTF8:
        *proc << "-Tutf8";
        break;

      case KOS_E_KOI8U:
	*proc << "-w'"; // add ' as a word char
	break;

      }




  /*
  if (ksconfig->personalDict()[0]!='\0')
    {
      kdDebug(750) << "personal dictionary [" << ksconfig->personalDict() << "]" << endl;
      *proc << "-p";
      *proc << ksconfig->personalDict();
    }
    */


  // -a : pipe mode
  // -S : sort suggestions by probable correctness
  if (trystart==0) //don't connect these multiple times
    {
      connect (proc, SIGNAL (  receivedStderr (KProcess *, char *, int)),
	       this, SLOT (ispellErrors (KProcess *, char *, int)));


      connect(proc, SIGNAL(processExited(KProcess *)),
	      this, SLOT (ispellExit (KProcess *)));

      OUTPUT(KSpell2);
    }

  if (proc->start ()==FALSE )
  {
      m_status = Error;
      QTimer::singleShot( 0, this, SLOT(emitDeath()));
  }
}

QStringList KOISpell::resultCheckWord( const QString &/*_word*/ )
{
    return QStringList();
}


void KOISpell::ispellErrors (KProcess *, char *buffer, int buflen)
{
  buffer [buflen-1] = '\0';
  //  kdDebug(750) << "ispellErrors [" << buffer << "]\n" << endl;
}

void KOISpell::KSpell2 (KProcIO *)

{
  kdDebug(750) << "KSpell::KSpell2" << endl;
  trystart=maxtrystart;  //We've officially started ispell and don't want
       //to try again if it dies.
  QString line;

  if (proc->fgets (line, TRUE)==-1)
  {
     QTimer::singleShot( 0, this, SLOT(emitDeath()));
     return;
  }


  if (line[0]!='@') //@ indicates that ispell is working fine
  {
     QTimer::singleShot( 0, this, SLOT(emitDeath()));
     return;
  }

  //We want to recognize KDE in any text!
  if (ignore ("kde")==FALSE)
  {
     kdDebug(750) << "@KDE was FALSE" << endl;
     QTimer::singleShot( 0, this, SLOT(emitDeath()));
     return;
  }

  //We want to recognize linux in any text!
  if (ignore ("linux")==FALSE)
  {
     kdDebug(750) << "@Linux was FALSE" << endl;
     QTimer::singleShot( 0, this, SLOT(emitDeath()));
     return;
  }

  NOOUTPUT (KSpell2);

  m_status = Running;
  emit ready(this);
}

void
KOISpell::setUpDialog (bool reallyuseprogressbar)
{
  if (dialogsetup)
    return;

  //Set up the dialog box
  ksdlg=new KOSpellDlg (parent, ksconfig, "dialog",
		       progressbar && reallyuseprogressbar, modaldlg );
  ksdlg->setCaption (caption);
  connect (ksdlg, SIGNAL (command (int)), this,
		SLOT (slotStopCancel (int)) );
  connect (this, SIGNAL ( progress (unsigned int) ),
	   ksdlg, SLOT ( slotProgress (unsigned int) ));
#ifdef Q_WS_X11 // FIXME(E): Implement for Qt/Embedded
  KWin::setIcons (ksdlg->winId(), kapp->icon(), kapp->miniIcon());
#endif
  if ( modaldlg )
      ksdlg->setFocus();
  dialogsetup = TRUE;
}

bool KOISpell::addPersonal (const QString & word)
{
  QString qs = word.simplifyWhiteSpace();

  //we'll let ispell do the work here b/c we can
  if (qs.find (' ')!=-1 || qs.isEmpty())    // make sure it's a _word_
    return FALSE;

  qs.prepend ("*");
  personaldict=TRUE;

  return proc->fputs(qs);
}

bool KOISpell::writePersonalDictionary ()
{
  return proc->fputs ("#");
}

bool KOISpell::ignore (const QString & word)
{
  QString qs = word.simplifyWhiteSpace();

  //we'll let ispell do the work here b/c we can
  if (qs.find (' ')!=-1 || qs.isEmpty())    // make sure it's a _word_
    return FALSE;

  qs.prepend ("@");

  return proc->fputs(qs);
}

bool
KOISpell::cleanFputsWord (const QString & s, bool appendCR)
{
  QString qs(s);
  //bool firstchar = TRUE;
  bool empty = TRUE;

  for (unsigned int i=0; i<qs.length(); i++)
  {
    //we need some punctuation for ornaments
    if (qs[i] != '\'' && qs[i] != '\"' && qs[i] != '-'
	&& qs[i].isPunct() || qs[i].isSpace())
    {
      qs.remove(i,1);
      i--;
    } else {
      if (qs[i].isLetter()) empty=FALSE;
    }
  }

  // don't check empty words, otherwise synchronisation will lost
  if (empty) return FALSE;

  return proc->fputs("^"+qs, appendCR);
}

bool
KOISpell::cleanFputs (const QString & s, bool appendCR)
{
  QString qs(s);
  unsigned l = qs.length();

  // some uses of '$' (e.g. "$0") cause ispell to skip all following text
  for(unsigned int i = 0; i < l; ++i)
  {
    if(qs[i] == '$')
      qs[i] = ' ';
  }

  if (l<MAXLINELENGTH)
    {
      if (qs.isEmpty())
	qs="";

      return proc->fputs ("^"+qs, appendCR);
    }
  else
    return proc->fputs ("^\n",appendCR);
}

bool KOISpell::checkWord (const QString & buffer, bool _usedialog)
{
  QString qs = buffer.simplifyWhiteSpace();

  if (qs.find (' ')!=-1 || qs.isEmpty())    // make sure it's a _word_
    return FALSE;

  ///set the dialog signal handler
  dialog3slot = SLOT (checkWord3());

  usedialog=_usedialog;
  setUpDialog(FALSE);
  if (_usedialog)
    {
      emitProgress();
      ksdlg->show();
    }
  else
    ksdlg->hide();

  OUTPUT (checkWord2);
  //  connect (this, SIGNAL (dialog3()), this, SLOT (checkWord3()));

  proc->fputs ("%"); // turn off terse mode
  proc->fputs (buffer); // send the word to ispell

  return TRUE;
}

void KOISpell::checkWord2 (KProcIO *)
{
  QString word;

  QString line;
  proc->fgets (line, TRUE); //get ispell's response

/* ispell man page: "Each sentence of text input is terminated with an
   additional blank line,  indicating that ispell has completed processing
   the input line." */
  QString blank_line;
  proc->fgets(blank_line, TRUE); // eat the blank line

  NOOUTPUT(checkWord2);

  bool mistake = (parseOneResponse(line, word, sugg) == MISTAKE);
  if ( mistake && usedialog )
    {
      cwword=word;
      dialog (word, sugg, SLOT (checkWord3()));
      return;
    }
  else if( mistake )
    {
      emit misspelling (word, sugg, lastpos);
    }

  //emits a "corrected" signal _even_ if no change was made
  //so that the calling program knows when the check is complete
  emit corrected (word, word, 0L);
}

void KOISpell::checkWord3 ()
{
  disconnect (this, SIGNAL (dialog3()), this, SLOT (checkWord3()));

  emit corrected (cwword, replacement(), 0L);
}

QString KOISpell::funnyWord (const QString & word)
  // composes a guess from ispell to a readable word
  // e.g. "re+fry-y+ies" -> "refries"
{
  QString qs;
  unsigned int i=0;

  for (i=0; word [i]!='\0';i++)
    {
      if (word [i]=='+')
	continue;
      if (word [i]=='-')
	{
	  QString shorty;
	  unsigned int j;
	  int k;

	  for (j=i+1;word [j]!='\0' && word [j]!='+' &&
		 word [j]!='-';j++)
	    shorty+=word [j];
	  i=j-1;

	  if ((k=qs.findRev (shorty))==0 || k!=-1)
	    qs.remove (k,shorty.length());
	  else
	    {
              qs+='-';
              qs+=shorty;  //it was a hyphen, not a '-' from ispell
            }
	}
      else
	qs+=word [i];
    }
  return qs;
}


int KOISpell::parseOneResponse (const QString &buffer, QString &word, QStringList & sugg)
  // buffer is checked, word and sugg are filled in
  // returns
  //   GOOD    if word is fine
  //   IGNORE  if word is in ignorelist
  //   REPLACE if word is in replacelist
  //   MISTAKE if word is misspelled
{
  word = "";
  posinline=0;

  sugg.clear();

  if (buffer [0]=='*' || buffer[0] == '+' || buffer[0] == '-')
    {
      return GOOD;
    }

  if (buffer [0]=='&' || buffer [0]=='?' || buffer [0]=='#')
    {
      int i,j;


      word = buffer.mid (2,buffer.find (' ',3)-2);
      //check() needs this
      orig=word;

      if(m_bIgnoreTitleCase && word==word.upper())
          return IGNORE;

      if(m_bIgnoreUpperWords && word[0]==word[0].upper())
      {
          QString text=word[0]+word.right(word.length()-1).lower();
          if(text==word)
              return IGNORE;
      }

      /////// Ignore-list stuff //////////
      //We don't take advantage of ispell's ignore function because
      //we can't interrupt ispell's output (when checking a large
      //buffer) to add a word to _it's_ ignore-list.
      if (ignorelist.findIndex(word.lower())!=-1)
	return IGNORE;

      //// Position in line ///
      QString qs2;

      if (buffer.find(':')!=-1)
	qs2=buffer.left (buffer.find (':'));
      else
	qs2=buffer;

      posinline = qs2.right( qs2.length()-qs2.findRev(' ') ).toInt()-1;

      ///// Replace-list stuff ////
      QStringList::Iterator it = replacelist.begin();
      for(;it != replacelist.end(); ++it, ++it) // Skip two entries at a time.
      {
         if (word == *it) // Word matches
         {
            ++it;
            word = *it;   // Replace it with the next entry
            return REPLACE;
	 }
      }

      /////// Suggestions //////
      if (buffer [0] != '#')
	{
	  QString qs = buffer.mid(buffer.find(':')+2, buffer.length());
	  qs+=',';
	  sugg.clear();
	  i=j=0;
	  while ((unsigned int)i<qs.length())
	    {
	      QString temp = qs.mid (i,(j=qs.find (',',i))-i);
	      sugg.append (funnyWord (temp));

	      i=j+2;
	    }
	}

      if ((sugg.count()==1) && (sugg.first() == word))
	return GOOD;

      return MISTAKE;
    }


  kdError(750) << "HERE?: [" << buffer << "]" << endl;
  kdError(750) << "Please report this to dsweet@kde.org" << endl;
  kdError(750) << "Thank you!" << endl;
  emit done((bool)FALSE);
  emit done (KOISpell::origbuffer);
  return MISTAKE;
}

bool KOISpell::checkList (QStringList *_wordlist, bool _usedialog)
  // prepare check of string list
{
  wordlist=_wordlist;
  if ((totalpos=wordlist->count())==0)
    return FALSE;
  wlIt = wordlist->begin();
  usedialog=_usedialog;

  // prepare the dialog
  setUpDialog();

  //set the dialog signal handler
  dialog3slot = SLOT (checkList4 ());

  proc->fputs ("%"); // turn off terse mode & check one word at a time

  //lastpos now counts which *word number* we are at in checkListReplaceCurrent()
  lastpos = -1;
  checkList2();

  // when checked, KProcIO calls checkList3a
  OUTPUT(checkList3a);

  return TRUE;
}

void KOISpell::checkList2 ()
  // send one word from the list to KProcIO
  // invoked first time by checkList, later by checkListReplaceCurrent and checkList4
{
  // send next word
  if (wlIt != wordlist->end())
    {
      kdDebug(750) << "KS::cklist2 " << lastpos << ": " << *wlIt << endl;

      endOfResponse = FALSE;
      bool put;
      lastpos++; offset=0;
      put = cleanFputsWord (*wlIt);
      ++wlIt;

      // when cleanFPutsWord failed (e.g. on empty word)
      // try next word; may be this is not good for other
      // problems, because this will make read the list up to the end
      if (!put) {
	checkList2();
      }
    }
  else
    // end of word list
    {
      NOOUTPUT(checkList3a);
      ksdlg->hide();
      emit done(TRUE);
    }
}

void KOISpell::checkList3a (KProcIO *)
  // invoked by KProcIO, when data from ispell are read
{
  //kdDebug(750) << "start of checkList3a" << endl;

  // don't read more data, when dialog is waiting
  // for user interaction
  if (dlgon) {
    //kdDebug(750) << "dlgon: don't read more data" << endl;
    return;
  }

  int e, tempe;

  QString word;
  QString line;

    do
      {
	tempe=proc->fgets (line, TRUE); //get ispell's response

	//kdDebug(750) << "checkList3a: read bytes [" << tempe << "]" << endl;


	if (tempe == 0) {
	  endOfResponse = TRUE;
	  //kdDebug(750) << "checkList3a: end of resp" << endl;
	} else if (tempe>0) {
	  if ((e=parseOneResponse (line, word, sugg))==MISTAKE ||
	      e==REPLACE)
	    {
	      dlgresult=-1;

	      if (e==REPLACE)
		{
		  QString old = *(--wlIt); ++wlIt;
		  dlgreplacement=word;
		  checkListReplaceCurrent();
		  // inform application
		  emit corrected (old, *(--wlIt), lastpos); ++wlIt;
		}
	      else if( usedialog )
		{
		  cwword=word;
		  dlgon=TRUE;
		  // show the dialog
		  dialog (word, sugg, SLOT (checkList4()));
		  return;
		}
	      else
		{
		  emit misspelling (word, sugg, lastpos);
 		}
	    }

	}
      	emitProgress (); //maybe

	// stop when empty line or no more data
      } while (tempe > 0);

    //kdDebug(750) << "checkList3a: exit loop with [" << tempe << "]" << endl;

    // if we got an empty line, t.e. end of ispell/aspell response
    // and the dialog isn't waiting for user interaction, send next word
    if (endOfResponse && !dlgon) {
      //kdDebug(750) << "checkList3a: send next word" << endl;
      checkList2();
    }
}

void KOISpell::checkListReplaceCurrent () {

  // go back to misspelled word
  wlIt--;

  QString s = *wlIt;
  s.replace(posinline+offset,orig.length(),replacement());
  offset += replacement().length()-orig.length();
  wordlist->insert (wlIt, s);
  wlIt = wordlist->remove (wlIt);
  // wlIt now points to the word after the repalced one

}

void KOISpell::checkList4 ()
  // evaluate dialog return, when a button was pressed there
{
  dlgon=FALSE;
  QString old;

  disconnect (this, SIGNAL (dialog3()), this, SLOT (checkList4()));

  //others should have been processed by dialog() already
  switch (dlgresult)
    {
    case KS_REPLACE:
    case KS_REPLACEALL:
      kdDebug(750) << "KS: cklist4: lastpos: " << lastpos << endl;
      old = *(--wlIt); ++wlIt;
      // replace word
      checkListReplaceCurrent();
      emit corrected (old, *(--wlIt), lastpos); ++wlIt;
      break;
    case KS_CANCEL:
      ksdlg->hide();
      emit done ((bool)FALSE);
      return;
    case KS_STOP:
      ksdlg->hide();
      emit done (TRUE);
      break;
    };

  // read more if there is more, otherwise send next word
  if (!endOfResponse) {
    //kdDebug(750) << "checkList4: read more from response" << endl;
      checkList3a(NULL);
  }
}

bool KOISpell::check( const QString &_buffer, bool _usedialog )
{
  QString qs;

  usedialog=_usedialog;
  setUpDialog ();
  //set the dialog signal handler
  dialog3slot = SLOT (check3 ());

  kdDebug(750) << "KS: check" << endl;
  origbuffer = _buffer;
  if ( ( totalpos = origbuffer.length() ) == 0 )
    {
      emit done(origbuffer);
      return FALSE;
    }


  // Torben: I corrected the \n\n problem directly in the
  //         origbuffer since I got errors otherwise
  if ( origbuffer.right(2) != "\n\n" )
    {
      if (origbuffer.at(origbuffer.length()-1)!='\n')
	{
	  origbuffer+='\n';
	  origbuffer+='\n'; //shouldn't these be removed at some point?
	}
      else
	origbuffer+='\n';
    }

  newbuffer=origbuffer;

  // KProcIO calls check2 when read from ispell
  OUTPUT(check2);
  proc->fputs ("!");

  //lastpos is a position in newbuffer (it has offset in it)
  offset=lastlastline=lastpos=lastline=0;

  emitProgress ();

  // send first buffer line
  int i = origbuffer.find('\n', 0)+1;
  qs=origbuffer.mid (0,i);
  cleanFputs (qs,FALSE);

  lastline=i; //the character position, not a line number

  if (usedialog)
    {
      emitProgress();
      ksdlg->show();
    }
  else
    ksdlg->hide();

  return TRUE;
}

void KOISpell::check2 (KProcIO *)
  // invoked by KProcIO when read from ispell
{
  int e, tempe;
  QString word;
  QString line;

  do
    {
      tempe=proc->fgets (line); //get ispell's response
      kdDebug(750) << "KSpell::check2 (" << tempe << "b)" << endl;

      if (tempe>0)
	{
	  if ((e=parseOneResponse (line, word, sugg))==MISTAKE ||
	      e==REPLACE)
	    {
	      dlgresult=-1;

	      // for multibyte encoding posinline needs correction
	      if (ksconfig->encoding() == KOS_E_UTF8) {
		// kdDebug(750) << "line: " << origbuffer.mid(lastlastline,
		// lastline-lastlastline) << endl;
		// kdDebug(750) << "posinline uncorr: " << posinline << endl;

		// convert line to UTF-8, cut at pos, convert back to UCS-2
		// and get string length
		posinline = (QString::fromUtf8(
		   origbuffer.mid(lastlastline,lastline-lastlastline).utf8(),
		   posinline)).length();
		// kdDebug(750) << "posinline corr: " << posinline << endl;
	      }

	      lastpos=posinline+lastlastline+offset;

	      //orig is set by parseOneResponse()

	      if (e==REPLACE)
		{
		  dlgreplacement=word;
		  emit corrected (orig, replacement(), lastpos);
		  offset+=replacement().length()-orig.length();
		  newbuffer.replace (lastpos, orig.length(), word);
		}
	      else  //MISTAKE
		{
		  cwword=word;
		  //kdDebug(750) << "(Before dialog) word=[" << word << "] cwword =[" << cwword << "]\n" << endl;
                  if ( usedialog ) {
                      // show the word in the dialog
                      dialog (word, sugg, SLOT (check3()));
                  } else {
                      // No dialog, just emit misspelling and continue
                      emit misspelling (word, sugg, lastpos);
                      dlgresult = KS_IGNORE;
                      check3();
                  }
		  return;
		}
	    }

	  }

      emitProgress (); //maybe

    } while (tempe>0);

  proc->ackRead();


  if (tempe==-1) //we were called, but no data seems to be ready...
    return;

  //If there is more to check, then send another line to ISpell.
  if ((unsigned int)lastline<origbuffer.length())
    {
      int i;
      QString qs;

      //kdDebug(750) << "[EOL](" << tempe << ")[" << temp << "]" << endl;

      lastpos=(lastlastline=lastline)+offset; //do we really want this?
      i=origbuffer.find('\n', lastline)+1;
      qs=origbuffer.mid (lastline, i-lastline);
      cleanFputs (qs,FALSE);
      lastline=i;
      return;
    }
  else
  //This is the end of it all
    {
      ksdlg->hide();
      //      kdDebug(750) << "check2() done" << endl;
      newbuffer.truncate (newbuffer.length()-2);
      emitProgress();
      emit done (newbuffer);
    }
}

void KOISpell::check3 ()
  // evaluates the return value of the dialog
{
  disconnect (this, SIGNAL (dialog3()), this, SLOT (check3()));

  kdDebug(750) << "check3 [" << cwword << "] [" << replacement() << "] " << dlgresult << endl;

  //others should have been processed by dialog() already
  switch (dlgresult)
    {
    case KS_REPLACE:
    case KS_REPLACEALL:
      offset+=replacement().length()-cwword.length();
      newbuffer.replace (lastpos, cwword.length(),
			 replacement());
      emit corrected (dlgorigword, replacement(), lastpos);
      break;
    case KS_CANCEL:
    //      kdDebug(750) << "cancelled\n" << endl;
      ksdlg->hide();
      emit done (origbuffer);
      return;
    case KS_STOP:
      ksdlg->hide();
      //buffer=newbuffer);
      emitProgress();
      emit done (newbuffer);
      return;
    };

  proc->ackRead();
}

void
KOISpell::slotStopCancel (int result)
{
  if (dialogwillprocess)
    return;

  kdDebug(750) << "KSpell::slotStopCancel [" << result << "]" << endl;

  if (result==KS_STOP || result==KS_CANCEL)
    if (!dialog3slot.isEmpty())
      {
	dlgresult=result;
	connect (this, SIGNAL (dialog3()), this, dialog3slot.ascii());
	emit dialog3();
      }
}


void KOISpell::dialog(const QString & word, QStringList & sugg, const char *_slot)
{
  dlgorigword=word;

  dialog3slot=_slot;
  dialogwillprocess=TRUE;
  connect (ksdlg, SIGNAL (command (int)), this, SLOT (dialog2(int)));
  ksdlg->init (word, &sugg);
  emit misspelling (word, sugg, lastpos);

  emitProgress();
  ksdlg->show();
}

void KOISpell::dialog2 (int result)
{
  QString qs;

  disconnect (ksdlg, SIGNAL (command (int)), this, SLOT (dialog2(int)));
  dialogwillprocess=FALSE;
  dlgresult=result;
  ksdlg->standby();

  dlgreplacement=ksdlg->replacement();

  //process result here
  switch (dlgresult)
    {

    case KS_IGNORE:
      emit ignoreword(dlgorigword);
      break;
    case KS_IGNOREALL:
      // would be better to lower case only words with beginning cap
      ignorelist.prepend(dlgorigword.lower());
      emit ignoreall (dlgorigword);
      break;
    case KS_ADD:
      addPersonal (dlgorigword);
      personaldict=TRUE;
      emit addword (dlgorigword);
      // adding to pesonal dict takes effect at the next line, not the current
      ignorelist.prepend(dlgorigword.lower());
      break;
    case KS_REPLACEALL:
      replacelist.append (dlgorigword);
      QString _replacement = replacement();
      replacelist.append (_replacement);
      emit replaceall( dlgorigword ,  _replacement );
      break;
    }

  connect (this, SIGNAL (dialog3()), this, dialog3slot.ascii());
  emit dialog3();
}


KOISpell:: ~KOISpell ()
{
    delete proc;
}


void KOISpell::cleanUp ()
{
  if (m_status == Cleaning) return; // Ignore
  if (m_status == Running)
  {
    if (personaldict)
       writePersonalDictionary();
    m_status = Cleaning;
  }
  proc->closeStdin();
}

void KOISpell::ispellExit (KProcess *)
{
  kdDebug() << "KSpell::ispellExit() " << m_status << endl;

  if ((m_status == Starting) && (trystart<maxtrystart))
  {
    trystart++;
    startIspell();
    return;
  }

  if (m_status == Starting)
     m_status = Error;
  else if (m_status == Cleaning)
     m_status = Finished;
  else if (m_status == Running)
     m_status = Crashed;
  else // Error, Finished, Crashed
     return; // Dead already

  kdDebug(750) << "Death" << endl;
  QTimer::singleShot( 0, this, SLOT(emitDeath()));
}

// This is always called from the event loop to make
// sure that the receiver can safely delete the
// KOISpell object.
void KOISpell::emitDeath()
{
  bool deleteMe = autoDelete; // Can't access object after next call!
  emit death();
  if (deleteMe)
     delete this;
}

void KOISpell::setProgressResolution (unsigned int res)
{
  progres=res;
}

void KOISpell::emitProgress ()
{
  uint nextprog = (uint) (100.*lastpos/(double)totalpos);

  if (nextprog>=curprog)
    {
      curprog=nextprog;
      emit progress (curprog);
    }
}

// --------------------------------------------------
// Stuff for modal (blocking) spell checking
//
// Written by Torben Weis <weis@kde.org>. So please
// send bug reports regarding the modal stuff to me.
// --------------------------------------------------

int
KOISpell::modalCheck( QString& text )
{
    return modalCheck( text,0 );
}

int
KOISpell::modalCheck( QString& text, KOSpellConfig* _kcs )
{
    modalreturn = 0;
    modaltext = text;

    /*modalWidgetHack = new QWidget(0,0,WType_Modal);
    modalWidgetHack->setGeometry(-10,-10,2,2);
    */

    // kdDebug() << "KOISpell1" << endl;
    KOISpell* spell = new KOISpell( 0L, i18n("Spell Checker"), 0 ,
				0, _kcs, true, true );
    //modalWidgetHack->show();
    //qApp->enter_loop();

    while (spell->status()!=Finished)
      kapp->processEvents();

    text = modaltext;

    //delete modalWidgetHack;
    //modalWidgetHack = 0;

    delete spell;
    return modalreturn;
}

void KOISpell::slotSpellCheckerCorrected( const QString & oldText, const QString & newText, unsigned int pos )
{
    modaltext=modaltext.replace(pos,oldText.length(),newText);
}


void KOISpell::slotModalReady()
{
    //kdDebug() << qApp->loopLevel() << endl;
    //kdDebug(750) << "MODAL READY------------------" << endl;

    Q_ASSERT( m_status == Running );
    connect( this, SIGNAL( done( const QString & ) ),
             this, SLOT( slotModalDone( const QString & ) ) );
    QObject::connect( this, SIGNAL( corrected( const QString&, const QString&, unsigned int ) ),
                      this, SLOT( slotSpellCheckerCorrected( const QString&, const QString &, unsigned int ) ) );
     QObject::connect( this, SIGNAL( death() ),
                      this, SLOT( slotModalSpellCheckerFinished( ) ) );
    check( modaltext );
}

void KOISpell::slotModalDone( const QString &/*_buffer*/ )
{
    //kdDebug(750) << "MODAL DONE " << _buffer << endl;
    //modaltext = _buffer;
    cleanUp();

    //kdDebug() << "ABOUT TO EXIT LOOP" << endl;
    //qApp->exit_loop();

    //modalWidgetHack->close(true);
    slotModalSpellCheckerFinished();
}

void KOISpell::slotModalSpellCheckerFinished( )
{
    modalreturn=(int)this->status();
}

QString KOISpell::modaltext;
int KOISpell::modalreturn = 0;
QWidget* KOISpell::modalWidgetHack = 0;


