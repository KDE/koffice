/* This file is part of the KDE project
   Copyright (C)  2001 Montel Laurent <lmontel@mandrakesoft.com>

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

#ifndef __kwconfig__
#define __kwconfig__

#include <kdialogbase.h>

class KWView;
class QCheckBox;
class KIntNumInput;
class KDoubleNumInput;
class KSpellConfig;
class KConfig;
class QComboBox;

class ConfigureSpellPage : public QObject
{
    Q_OBJECT
public:
    ConfigureSpellPage( KWView *_view, QVBox *box, char *name = 0 );
    void apply();
    void slotDefault();
private:
    KWView* m_pView;
    KSpellConfig *_spellConfig;
    QCheckBox *_dontCheckUpperWord;
    QCheckBox *_dontCheckTitleCase;
    KConfig* config;
};

class ConfigureInterfacePage : public QObject
{
    Q_OBJECT
public:
    ConfigureInterfacePage( KWView *_view, QVBox *box, char *name = 0 );
    void apply();
    void slotDefault();
private:
    KWView* m_pView;
    KDoubleNumInput* gridX,*gridY;
    KDoubleNumInput* indent;
    KIntNumInput* recentFiles;
    KIntNumInput* autoSave;
    QCheckBox *showRuler;
    int oldNbRecentFiles;
    KConfig* config;
    int oldAutoSaveValue;
    KIntNumInput *m_nbPagePerRow;
};

class ConfigureMiscPage : public QObject
{
    Q_OBJECT
public:
    ConfigureMiscPage( KWView *_view, QVBox *box, char *name = 0 );
    void apply();
    void slotDefault();
private:
    KWView* m_pView;
    KConfig* config;
    QComboBox *m_unit;
    int m_oldUnit;
    KIntNumInput* m_undoRedoLimit;
    int m_oldNbRedo;
    int m_oldStartingPage;
    QLineEdit* m_variableNumberOffset;
};

class ConfigureDefaultDocPage : public QObject
{
    Q_OBJECT
public:
    ConfigureDefaultDocPage( KWView *_view, QVBox *box, char *name = 0 );
    void apply();
    void slotDefault();
public slots:
    void selectNewDefaultFont();
private:
    KWView* m_pView;
    KConfig* config;
    QComboBox *m_unit;
    QFont *font;
    QLabel *fontName;

    KDoubleNumInput* columnSpacing;
};

class KWConfig : public KDialogBase
{
    Q_OBJECT
public:
    enum { KW_KSPELL=1,KP_INTERFACE=2,KP_MISC=4, KP_DOCUMENT=8};
    KWConfig( KWView* parent );
    void openPage(int flags);
public slots:
    void slotApply();
    void slotDefault();
private:
    ConfigureSpellPage *m_spellPage;
    ConfigureInterfacePage *m_interfacePage;
    ConfigureMiscPage *m_miscPage;
    ConfigureDefaultDocPage *m_defaultDocPage;
};



#endif
