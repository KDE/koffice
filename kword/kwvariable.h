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

#ifndef variable_h
#define variable_h

#include <qstring.h>
#include <qdatetime.h>
#include <qasciidict.h>

#include "defs.h"

#include <kovariable.h>
#include <koparagcounter.h>

class KWDocument;
class KWTextFrameSet;
class KWFootNoteFrameSet;
class KoVariable;
class KoPageVariable;
class KoMailMergeVariable;
class QDomElement;
class KoTextFormat;


class KWVariableSettings : public KoVariableSettings
{
 public:
    KWVariableSettings();
    virtual ~KWVariableSettings() {}
    virtual void save( QDomElement &parentElem );
    virtual void load( QDomElement &elem );
    void changeFootNoteCounter( KoParagCounter _c );
    void changeEndNoteCounter( KoParagCounter _c );
    KoParagCounter endNoteCounter() const { return m_endNoteCounter;}
    KoParagCounter footNoteCounter() const { return m_footNoteCounter;}
 private:
    KoParagCounter m_footNoteCounter;
    KoParagCounter m_endNoteCounter;
};

class KWVariableCollection : public KoVariableCollection
{
 public:
    KWVariableCollection(KWVariableSettings *_settings, KoVariableFormatCollection* coll);
    virtual KoVariable *createVariable( int type, short int subtype, KoVariableFormatCollection * coll, KoVariableFormat *varFormat,KoTextDocument *textdoc, KoDocument * doc, int _correct, bool _forceDefaultFormat=false, bool loadFootNote= true );
    virtual KoVariable* loadOasisField( KoTextDocument* textdoc, const QDomElement& tag, KoOasisContext& context );

 private:
    KWDocument *m_doc;
};

/**
 * "current page number" and "number of pages" variables
 */
class KWPgNumVariable : public KoPageVariable
{
public:
    KWPgNumVariable( KoTextDocument *textdoc, int subtype, KoVariableFormat *varFormat ,KoVariableCollection *_varColl, KWDocument *doc );

    virtual void recalc();
    virtual QString text(bool realValue=false);

private:
    KWDocument *m_doc;
};


/**
 * Mail Merge variable
 */
class KWMailMergeVariable : public KoMailMergeVariable
{
public:
    KWMailMergeVariable( KoTextDocument *textdoc, const QString &name, KoVariableFormat *varFormat,KoVariableCollection *_varColl, KWDocument *doc );

    virtual QString text(bool realValue=false);
    virtual QString value() const;
    virtual void recalc();
 private:
    KWDocument *m_doc;
};

/**
 * The variable showing the footnote number in superscript, in the text.
 */
class KWFootNoteVariable : public KoVariable
{
public:
    KWFootNoteVariable( KoTextDocument *textdoc, KoVariableFormat *varFormat, KoVariableCollection *varColl, KWDocument *doc );
    virtual VariableType type() const
    { return VT_FOOTNOTE; }
    enum Numbering {Auto, Manual};

    void setNoteType( NoteType _noteType ) { m_noteType = _noteType;}
    NoteType noteType() const {return m_noteType; }

    void setNumberingType( Numbering _type );
    Numbering numberingType() const { return m_numberingType;}

    void setManualString( const QString & _str ) { m_varValue=QVariant(_str);}
    QString manualString() const { return m_varValue.toString();}

    virtual void resize();
    virtual void drawCustomItem( QPainter* p, int x, int y, int wpix, int hpix, int ascentpix, int /*cx*/, int /*cy*/, int /*cw*/, int /*ch*/, const QColorGroup& cg, bool selected, int offset, bool drawingShadow );

    /** The frameset that contains the text for this footnote */
    KWFootNoteFrameSet * frameSet() const { return m_frameset; }
    void setFrameSet( KWFootNoteFrameSet* fs ) { Q_ASSERT( !m_frameset ); m_frameset = fs; }

    virtual void saveVariable( QDomElement &parentElem );
    virtual void load( QDomElement &elem );

    virtual void loadOasis( const QDomElement &elem, KoOasisContext& context );
    virtual void saveOasis( KoXmlWriter& writer, KoSavingContext& context ) const;


    virtual QString text(bool realValue=false);
    // Nothing to do here. Numbering done by KWTextFrameSet::renumberFootNotes
    virtual void recalc() { }

    // This is a sequence number, to order footnotes. It is always set, and different for all footnotes.
    void setNum( int _num ) { m_num = _num; }
    int num() const { return m_num; }

    // The number being displayed - for auto-numbered footnotes only.
    void setNumDisplay( int val );
    int numDisplay() const { return m_numDisplay; }

    virtual void finalize();

    // The page this var is on
    int pageNum() const;
    // The current Y position of the var (in doc pt)
    double varY() const;

    virtual void setDeleted( bool del );

    void formatedNote();
    virtual QString fieldCode();
protected:
    QString applyStyle();

private:
    KWDocument *m_doc;
    NoteType m_noteType;
    KWFootNoteFrameSet* m_frameset;
    Numbering m_numberingType;
    int m_num;
    int m_numDisplay;
};


class KWStatisticVariable : public KoVariable
{
public:
    KWStatisticVariable( KoTextDocument *textdoc, int subtype, KoVariableFormat *varFormat,KoVariableCollection *_varColl, KWDocument *doc );
    enum { VST_STATISTIC_NB_FRAME = 0, VST_STATISTIC_NB_PICTURE = 1, VST_STATISTIC_NB_TABLE = 2, VST_STATISTIC_NB_EMBEDDED = 3, VST_STATISTIC_NB_WORD = 4, VST_STATISTIC_NB_SENTENCE = 5, VST_STATISTIC_NB_LINES = 6, VST_STATISTIC_NB_CHARACTERE = 7 };

    virtual VariableType type() const
    { return VT_STATISTIC; }
    static QStringList actionTexts();

    virtual QStringList subTypeText();

    virtual void saveVariable( QDomElement &parentElem );
    virtual void load( QDomElement &elem );
    virtual void loadOasis( const QDomElement &elem, KoOasisContext& context );
    virtual void saveOasis( KoXmlWriter& writer, KoSavingContext& context ) const;

    virtual short int subType() const { return m_subtype; }

    virtual void setVariableSubType( short int subtype );

    QString name() const { return m_varValue.toString(); }
    virtual void recalc();
    virtual QString fieldCode();

    virtual QString text(bool realValue=false);

    QString value() const;
    void setValue( const QString &v );
protected:
    KWDocument *m_doc;
    short int m_subtype;
};

#endif
