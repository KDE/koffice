/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>
   Copyright (C) 1999 Simon Hausmann <hausmann@kde.org>
   Copyright (C) 2000 David Faure <faure@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/
#ifndef __koshell_window_h__
#define __koshell_window_h__

#include <koMainWindow.h>
#include <koQueryTrader.h>
#include <qlist.h>
#include <qmap.h>
#include <qvaluelist.h>

class QHBox;
class KoKoolBar;
class KoDocumentEntry;
class KoShellFrame;
class KoView;

class KoShellWindow : public KoMainWindow
{
  Q_OBJECT

public:

  KoShellWindow();
  virtual ~KoShellWindow();

  virtual bool openDocument( const KURL & url );
  virtual void setRootDocument( KoDocument *doc );
  /**
   * Update caption from document info - call when document info
   * (title in the about page) changes.
   */
  virtual void updateCaption();

  virtual QString configFile() const;

  virtual QString nativeFormatPattern() const;
  virtual QString nativeFormatName() const;

/*
  bool saveAllPages();
*/

protected slots:

  virtual void slotFileNew();
  virtual void slotFileClose();

  void slotKoolBar( int _grp, int _item );

protected:

  virtual bool queryClose();

  void closeDocument();

  struct Page
  {
    KoDocument *m_pDoc;
    KoView *m_pView;
    int m_id;
  };

  QValueList<Page> m_lstPages;
  QValueList<Page>::Iterator m_activePage;

  void switchToPage( QValueList<Page>::Iterator it );

  KoKoolBar* m_pKoolBar;

  int m_grpFile;
  int m_grpDocuments;

  // Map of available parts (the int is the koolbar item id)
  QMap<int,KoDocumentEntry> m_mapComponents;

  QString m_filter;

  // Saved between openDocument and setRootDocument
  KoDocumentEntry m_documentEntry;

  KoShellFrame *m_pFrame;

  QHBox *m_pLayout;
};

class KoShellFrame : public QWidget
{
  Q_OBJECT
public:
  KoShellFrame( QWidget *parent );

  void setView( KoView *view );

protected:
  virtual void resizeEvent( QResizeEvent * );

private:
  KoView *m_pView;
};

#endif // __koshell_window_h__

