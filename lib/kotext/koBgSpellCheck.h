/* This file is part of the KDE project
   Copyright (C) 2002 David Faure <david@mandrakesoft.com>
                 2002 Laurent Montel <lmontel@mandrakesoft.com>

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

#ifndef kobgspellcheck_h
#define kobgspellcheck_h


#include <qobject.h>

class KoTextObject;
class KSpell;
class KoDocument;
class KSpellConfig;

namespace Qt3 { class QTextParag; }

class KoBgSpellCheck : public QObject
{
    Q_OBJECT
public:
    KoBgSpellCheck();
    virtual ~KoBgSpellCheck();
    void enableBackgroundSpellCheck( bool b );

    bool backgroundSpellCheckEnabled() const { return m_bSpellCheckEnabled; }

    void setIgnoreUpperWords( bool b);
    void setIgnoreTitleCase( bool b);

    void setKSpellConfig(KSpellConfig _kspell);

    //repaint object when we spell check
    virtual void slotRepaintChanged(KoTextObject *obj)=0;

    virtual void objectForSpellChecking(KoTextObject *obj)=0;

public slots:
    void startBackgroundSpellCheck();

protected slots:
    void spellCheckerReady();
    void spellCheckerMisspelling( const QString &, const QStringList &, unsigned int );
    void spellCheckerDone( const QString & );
    void spellCheckerFinished( );
    void spellCheckNextParagraph();

protected:
    void nextParagraphNeedingCheck();

    KoTextObject * currObj;
    // Structure holding the background spellcheck data
    struct KoBGSpell {
        KoBGSpell() : kspell(0L), currentTextObj(0L), currentParag(0L) {}

        // KSpell object for the background spellcheck
	KSpell *kspell;
        // The text frameset currently being checked
	// TODO change current text frameset, and implementing nextTextFrameSet, see kwview.cc
        // TODO implement "skip unchanged framesets" and "stop timer after all checked and until
        // user types something again"
        KoTextObject *currentTextObj;
        // The paragraph currently being checked
        Qt3::QTextParag *currentParag;
    };
    KoBGSpell m_bgSpell;

private:
    KSpellConfig * m_pKSpellConfig;

    bool m_bSpellCheckEnabled;
    bool m_bDontCheckUpperWord;
    bool m_bDontCheckTitleCase;
};
#endif
