/* This file is part of the KDE project
   Copyright (C) 1999, 2000, 2001 Montel Laurent <lmontel@mandrakesoft.com>

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

#ifndef __kspread_dlg_preference__
#define __kspread_dlg_preference__

#include <kdialogbase.h>
#include <kcolorbutton.h>

class KSpreadView;
class KSpreadTable;
class KConfig;
class KIntNumInput;
class KDoubleNumInput;
class KSpellConfig;
class QCheckBox;
class QComboBox;
class QPushButton;


class preference : public QObject
{
  Q_OBJECT
public:
  preference( KSpreadView* _view,QVBox *box, char *name = 0 );
  void apply();
  void slotDefault();

protected:
  QCheckBox *m_pFormula;
  QCheckBox *m_pAutoCalc;
  QCheckBox *m_pGrid;
  QCheckBox *m_pColumn;
  QCheckBox *m_pLcMode;
  QCheckBox *m_pHideZero;
  QCheckBox *m_pFirstLetterUpper;
  KSpreadView* m_pView;
} ;

class parameterLocale :  public QObject
{
 Q_OBJECT
public:
   parameterLocale( KSpreadView* _view,QVBox *box, char *name = 0);
 void apply();
public slots:
   void updateDefaultSystemConfig();
 protected:
   QLabel *m_shortDate,*m_time,*m_money,*m_date,*m_language,*m_number;
   QPushButton *m_updateButton;
   KSpreadView* m_pView;	
   bool m_bUpdateLocale;
};

class configure : public QObject
{
  Q_OBJECT
public:
  configure( KSpreadView* _view,QVBox *box, char *name = 0 );
  void apply();
  void slotDefault();
protected:
  KSpreadView* m_pView;
  KIntNumInput  *nbPage;
  KIntNumInput* nbRecentFile;
  KIntNumInput* autoSaveDelay;
  QCheckBox *showVScrollBar;
  QCheckBox *showHScrollBar;
  QCheckBox *showColHeader;
  QCheckBox *showRowHeader;
  QCheckBox *showTabBar;
  QCheckBox *showFormulaBar;
  QCheckBox *showStatusBar;
  KConfig* config;
  int oldRecent;
  int oldAutoSaveValue;
} ;


class miscParameters : public QObject
{
  Q_OBJECT
public:
  miscParameters( KSpreadView* _view, QVBox *box, char *name = 0 );
  void apply();
  void slotDefault();

  void initComboBox();

public slots:
  void slotTextComboChanged(const QString &);

protected:
  KSpreadView* m_pView;
  KIntNumInput  *valIndent;
  KConfig* config;
  QComboBox *typeCompletion;
  QComboBox *typeCalc;
  QComboBox *typeOfMove;
  QCheckBox *msgError;
  QCheckBox *commentIndicator;
  bool comboChanged;
} ;

class colorParameters : public QObject
{
  Q_OBJECT
public:
  colorParameters( KSpreadView* _view, QVBox *box, char *name = 0 );
  void apply();
  void slotDefault();
protected:
  KSpreadView* m_pView;
  KColorButton* gridColor;
  KConfig* config;
} ;

class configureLayoutPage : public QObject
{
  Q_OBJECT
public:
  configureLayoutPage( KSpreadView* _view,QVBox *box, char *name = 0 );
  void apply();
  void slotDefault();
  void initCombo();
protected:
  KSpreadView* m_pView;
  QComboBox *defaultOrientationPage;
  QComboBox *defaultSizePage;
  QComboBox *defaultUnit;
  //store old config
  int paper;
  int orientation;
  int unit;
  
  KConfig* config;
} ;

class configureSpellPage : public QObject
{
  Q_OBJECT
public:
  configureSpellPage( KSpreadView* _view, QVBox *box, char *name = 0 );
  void apply();
  void slotDefault();
protected:
  KSpreadView * m_pView;
  KSpellConfig * _spellConfig;
  KConfig * config;
  QCheckBox * m_dontCheckUpperWord;
  QCheckBox * m_dontCheckTitleCase;
} ;

class KSpreadpreference : public KDialogBase
{
  Q_OBJECT
public:
  enum { KS_PREFERENCES = 1, KS_LOCALE = 2, KS_INTERFACE = 4,
         KS_MISC = 8, KS_COLOR = 16, KS_LAYOUT = 32, KS_SPELLING = 64 };
  KSpreadpreference( KSpreadView* parent, const char* name);
public slots:
  void slotApply();
  void slotDefault();
  void openPage(int flags);
private :
  KSpreadView* m_pView;
  preference *_preferenceConfig;
  configure * _configure;
  miscParameters *_miscParameter;
  colorParameters *_colorParameter;
  configureLayoutPage *_layoutPage;
  configureSpellPage *_spellPage;
  parameterLocale *_localePage;
};



#endif
