/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Reginald Stadlbauer <reggie@kde.org>
   Copyright (C) 2001, S.R.Haque <srhaque@iee.org>
   Copyright (C) 2001, David Faure <david@mandrakesoft.com>

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

#ifndef kosearchdia_h
#define kosearchdia_h

#include <koFind.h>
#include <koReplace.h>
#include <qcolor.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qrichtext_p.h>

class QPushButton;
class QRadioButton;
class QGridLayout;
class QCheckBox;
class QComboBox;
class QSpinBox;
class KColorButton;
class KMacroCommand;
class KoTextView;
class KoTextObject;
class KCommand;
class KoTextDocument;
class KoTextFind;
class KoTextReplace;
class KFontCombo;
//
// This class represents the KWord-specific search extension items, and also the
// corresponding replace items.
//
class KoSearchContext
{
public:

    // Options.

    typedef enum
    {
        Family = 1 * KoFindDialog::MinimumUserOption,
        Color = 2 * KoFindDialog::MinimumUserOption,
        Size = 4 * KoFindDialog::MinimumUserOption,
        Bold = 8 * KoFindDialog::MinimumUserOption,
        Italic = 16 * KoFindDialog::MinimumUserOption,
        Underline = 32 * KoFindDialog::MinimumUserOption,
        VertAlign = 64 * KoFindDialog::MinimumUserOption,
        StrikeOut = 128 * KoFindDialog::MinimumUserOption,
        BgColor = 256 *KoFindDialog::MinimumUserOption
    } Options;

    KoSearchContext();
    ~KoSearchContext();

    QString m_family;
    QColor m_color;
    QColor m_backGroungColor;
    int m_size;
    KoTextFormat::VerticalAlignment m_vertAlign;
    KoTextFormat::UnderlineLineType m_underline;
    KoTextFormat::StrikeOutLineType m_strikeOut;

    QStringList m_strings; // history
    long m_optionsMask;
    long m_options;
};

//
// This class represents the GUI elements that correspond to KWSearchContext.
//
class KoSearchContextUI : public QObject
{
    Q_OBJECT
public:
    KoSearchContextUI( KoSearchContext *ctx, QWidget *parent );
    void setCtxOptions( long options );
    void setCtxHistory( const QStringList & history );
    KoSearchContext *context() const { return m_ctx;}
    bool optionSelected() const { return m_bOptionsShown;}
private slots:
    void slotShowOptions();

private:
    KoSearchContext *m_ctx;
    QGridLayout *m_grid;
    bool m_bOptionsShown;
    QPushButton *m_btnShowOptions;
    QWidget *m_parent;
};

//
// This class is the KWord search dialog.
//
class KoSearchDia:
    public KoFindDialog
{
    Q_OBJECT

public:
    KoSearchDia( QWidget *parent, const char *name, KoSearchContext *find, bool hasSelection );
    KoSearchContext * searchContext() {
        return m_findUI->context();
    }
    bool optionSelected() const { return m_findUI->optionSelected();}

protected slots:
    void slotOk();

private:
    KoSearchContextUI *m_findUI;
};

//
// This class is the kotext replace dialog.
//
class KoReplaceDia:
    public KoReplaceDialog
{
    Q_OBJECT

public:

    KoReplaceDia( QWidget *parent, const char *name, KoSearchContext *find, KoSearchContext *replace, bool hasSelection );
    KoSearchContext * searchContext() {
        return m_findUI->context();
    }
    KoSearchContext * replaceContext() {
        return m_replaceUI->context();
    }
    bool optionFindSelected() const { return m_findUI->optionSelected();}
    bool optionSearchSelected() const { return m_replaceUI->optionSelected();}
protected slots:
    void slotOk();

private:

    KoSearchContextUI *m_findUI;
    KoSearchContextUI *m_replaceUI;
};

/**
 * This class implements the 'find' functionality ( the "search next, prompt" loop )
 * and the 'replace' functionality. Same class, to allow centralizing the code that
 * finds the framesets and paragraphs to look into.
 */
class KoFindReplace : public QObject
{
    Q_OBJECT
public:
    KoFindReplace( QWidget * parent, KoSearchDia * dialog, KoTextView *textView, const QPtrList<KoTextObject> & lstObject);
    KoFindReplace( QWidget * parent, KoReplaceDia * dialog, KoTextView *textView, const QPtrList<KoTextObject> & lstObject);
    ~KoFindReplace();

    KoTextParag *currentParag() {
        return m_currentParag;
    }

    bool isReplace() const { return m_replace != 0L; }

    bool shouldRestart();

    //int numMatches() const;
    //int numReplacements() const;

    /** Do the complete loop for find or replace. When it exits, we're done.
     * Returns false if the user aborted the search/replace operation. */
    bool proceed();

    /** Bring to front (e.g. when menuitem called twice) */
    void setActiveWindow();

    /** Abort - when closing the view */
    void abort();
    bool aborted() const { return m_destroying; }

    virtual void emitNewCommand(KCommand *) = 0;
    virtual void highlightPortion(KoTextParag * parag, int index, int length, KoTextDocument *textdoc) = 0;

    void changeListObject(const QPtrList<KoTextObject> & lstObject);

    /** For KoTextFind and KoTextReplace */
    bool validateMatch( const QString &text, int index, int matchedlength );

protected:
    bool findInTextObject( KoTextObject * textObj, KoTextParag * firstParag, int firstIndex,
                           KoTextParag * lastParag, int lastIndex );
    bool process( const QString &text );
    void replaceWithAttribut( KoTextCursor * cursor, int index );
    KMacroCommand* macroCommand();
    long options() const;

protected slots:
    void highlight( const QString &text, int matchingIndex, int matchingLength, const QRect & );
    void replace( const QString &text, int replacementIndex, int replacedLength,int searchLength, const QRect & );

private:
    // Only one of those two will be set
    KoTextFind * m_find;
    KoTextReplace * m_replace;

    // Only one of those two will be set
    KoSearchDia * m_findDlg;
    KoReplaceDia * m_replaceDlg;

    KoTextObject *m_currentTextObj;
    KoTextParag *m_currentParag;
    KMacroCommand *m_macroCmd;
    int m_offset;
    KoTextView *m_textView;
    QPtrList<KoTextObject> m_lstObject;
    bool m_destroying;
};

/**
 * Reimplement KoFind to provide our own validateMatch - for the formatting options
 */
class KoTextFind : public KoFind
{
    Q_OBJECT
public:
    KoTextFind(const QString &pattern, long options, KoFindReplace *_findReplace, QWidget *parent = 0);
    ~KoTextFind();
    virtual bool validateMatch( const QString &text, int index, int matchedlength );
private:
    KoFindReplace * m_findReplace;
};

/**
 * Reimplement KoReplace to provide our own validateMatch - for the formatting options
 */
class KoTextReplace : public KoReplace
{
    Q_OBJECT
public:
    KoTextReplace(const QString &pattern, const QString &replacement, long options, KoFindReplace *_findReplace, QWidget *parent = 0);
    ~KoTextReplace();
    virtual bool validateMatch( const QString &text, int index, int matchedlength );
private:
    KoFindReplace * m_findReplace;
};

class KoFormatDia: public KDialogBase
{
    Q_OBJECT
public:
    KoFormatDia( QWidget* parent, const QString & _caption, KoSearchContext *_ctx, const char* name=0L);
    //apply to parameter to context !
    void ctxOptions( );

protected slots:
    void slotReset();
    void slotClear();
private:
    QCheckBox *m_checkFamily;
    QCheckBox *m_checkSize;
    QCheckBox *m_checkColor;
    QCheckBox *m_checkBgColor;
    QCheckBox *m_checkBold;
    QCheckBox *m_checkItalic;
    QCheckBox *m_checkUnderline;
    QCheckBox *m_checkVertAlign;
    QCheckBox *m_checkStrikeOut;
    KFontCombo *m_familyItem;
    QSpinBox *m_sizeItem;
    KColorButton *m_colorItem;
    KColorButton *m_bgColorItem;
    QRadioButton *m_boldYes;
    QRadioButton *m_boldNo;
    QRadioButton *m_italicYes;
    QRadioButton *m_italicNo;

    QComboBox *m_vertAlignItem;
    QComboBox *m_underlineItem;
    QComboBox *m_strikeOutItem;

    KoSearchContext *m_ctx;
};

#endif
