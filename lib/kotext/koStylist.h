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

#ifndef kostylist_h
#define kostylist_h

#include <kdialogbase.h>
#include <qstringlist.h>

#include <koParagDia.h>
#include <koUnit.h>
#include <qptrlist.h>
#include <kostyle.h>

class KoFontChooser;
class KoStyle;
class KoStyleEditor;
class KoStyleManagerTab;
class QCheckBox;
class QComboBox;
class QGridLayout;
class QLineEdit;
class QListBox;
class QPushButton;
class QTabWidget;
class QWidget;
class KoTextDocument;

/******************************************************************/
/* Class: KoStyleManager                                          */
/******************************************************************/
class KoStyleManager : public KDialogBase
{
    Q_OBJECT

public:
    KoStyleManager( QWidget *_parent,KoUnit::Unit unit,const QPtrList<KoStyle> & style );

    virtual KoStyle* addStyleTemplate(KoStyle *style)=0;
    //virtual void applyStyleChange( KoStyle * changedStyle, int paragLayoutChanged, int formatChanged )=0;
    virtual void applyStyleChange( StyleChangeDefMap changed )=0;

    virtual void removeStyleTemplate( KoStyle *style )=0;
    virtual void updateAllStyleLists()=0;
    virtual void updateStyleListOrder( const QStringList & list)=0;

protected:
    void updateFollowingStyle( KoStyle *s );
    void setupWidget(const QPtrList<KoStyle> & style);
    void addGeneralTab();
    void apply();
    void updateGUI();
    void updatePreview();
    void save();
    int styleIndex( int pos );

    QTabWidget *m_tabs;
    QListBox *m_stylesList;
    QLineEdit *m_nameString;
    QComboBox *m_styleCombo;
    QPushButton *m_deleteButton;
    QPushButton *m_newButton;
    QPushButton *m_moveUpButton;
    QPushButton *m_moveDownButton;
    QComboBox *m_inheritCombo;
    KoStylePreview *preview;

    KoStyle *m_currentStyle;
    QPtrList<KoStyle> m_origStyles;      // internal list of orig styles we have modified
    QPtrList<KoStyle> m_changedStyles;   // internal list of changed styles.
    QPtrList<KoStyleManagerTab> m_tabsList;
    QStringList m_styleOrder;
    int numStyles;
    bool noSignals;

protected slots:
    virtual void slotOk();
    virtual void slotApply();
    void switchStyle();
    void switchTabs();
    void addStyle();
    void deleteStyle();
    void moveUpStyle();
    void moveDownStyle();
    void renameStyle(const QString &);
protected:
    KoStyle * style( const QString & _name );
    void addTab( KoStyleManagerTab * tab );
};

class KoStyleManagerTab : public QWidget {
    Q_OBJECT
public:
    KoStyleManagerTab(QWidget *parent) : QWidget(parent) {};

    /** the new style which is to be displayed */
    void setStyle(KoStyle *style) { m_style = style; }
    /**  update the GUI from the current Style*/
    virtual void update() = 0;
    /**  return the (i18n-ed) name of the tab */
    virtual QString tabName() = 0;
    /** save the GUI to the style */
    virtual void save() = 0;
protected:
    KoStyle *m_style;
};

// A tab to edit parts of the parag-layout of the style
// Acts as a wrapper around KoParagLayoutWidget [which doesn't know about styles].
class KoStyleParagTab : public KoStyleManagerTab
{
    Q_OBJECT
public:
    KoStyleParagTab( QWidget * parent )
        : KoStyleManagerTab( parent ) { m_widget = 0L; }

    // not a constructor parameter since 'this' is the parent of the widget
    void setWidget( KoParagLayoutWidget * widget );

    virtual void update();
    virtual void save();
    virtual QString tabName() { return m_widget->tabName(); }
protected:
    virtual void resizeEvent( QResizeEvent *e );
private:
    KoParagLayoutWidget * m_widget;
};

// The "font" tab. Maybe we should put the text color at the bottom ?
class KoStyleFontTab : public KoStyleManagerTab
{
    Q_OBJECT
public:
    KoStyleFontTab( QWidget * parent );
    ~KoStyleFontTab();
    virtual void update();
    virtual QString tabName();
    virtual void save();
protected:
    virtual void resizeEvent( QResizeEvent *e );
private:
    KoFontChooser* m_chooser;
    KoZoomHandler* m_zoomHandler;
};

/*
Font            simple font dia
Color           simple color dia
Spacing and Indents     paragraph spacing dia (KWParagDia)
alignments      KoParagDia alignment tab
borders         KoParagDia  borders tab
numbering       KoParagDia  tab numbering
tabulators      KoParagDia  tab tabs */

#endif
