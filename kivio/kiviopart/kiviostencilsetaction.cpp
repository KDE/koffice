/* This file is part of the KDE project
   Copyright (C) 2003 Peter Simonsson <psn@linux.se>

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

#include "kiviostencilsetaction.h"

#include <kpopupmenu.h>
#include <kstandarddirs.h>
#include <kapplication.h>
#include <kdebug.h>
#include <ktoolbar.h>
#include <ktoolbarbutton.h>

#include <qfile.h>
#include <qdir.h>
#include <qpixmap.h>
#include <qwhatsthis.h>
#include <qmenubar.h>

#include "kivio_stencil_spawner_set.h"

static const char * default_plug_xpm[] = {
"16 16 5 1",
" 	c None",
".	c #000000",
"+	c #C4C7FF",
"@	c #5961FF",
"#	c #FFFFFF",
"                ",
"                ",
" ...            ",
" .++..          ",
" .+@++..        ",
" .+@@@++..      ",
" .+@@@@@++..    ",
" .+@@@@@@@++..  ",
" .+@@@@@@@@@++. ",
" .+@@@@@@@@@@@.#",
" .+@@@@@@@@@@@.#",
" .+@@@@@@@@@@@.#",
" .+@@@@@@@@@@@.#",
" .+@@@@@@@@@@@.#",
" ..............#",
"  ##############"};

KivioStencilSetAction::KivioStencilSetAction(const QString &text, const QString &pix,
  KActionCollection *parent, const char *name)
  : KAction(text, pix, 0, parent, name)
{
  setShortcutConfigurable( false );

  m_childMenuList.setAutoDelete(true);
  m_pathList.setAutoDelete(true);

  m_popup = new KPopupMenu(0L,"KivioStencilSetAction::popup");
  connect(m_popup, SIGNAL(aboutToShow()), SLOT(updateMenu()));
}

KivioStencilSetAction::~KivioStencilSetAction()
{
  delete m_popup;
  m_popup = 0;
}

KPopupMenu* KivioStencilSetAction::popupMenu() const
{
  return m_popup;
}

void KivioStencilSetAction::popup( const QPoint& global )
{
  popupMenu()->popup(global);
}

int KivioStencilSetAction::plug( QWidget* widget, int index)
{
  // This function is copied from KActionMenu::plug
  if (kapp && !kapp->authorizeKAction(name()))
    return -1;
  kdDebug(129) << "KAction::plug( " << widget << ", " << index << " )" << endl; // remove -- ellis
  if ( widget->inherits("QPopupMenu") )
  {
    QPopupMenu* menu = static_cast<QPopupMenu*>( widget );
    int id;

    if ( hasIconSet() )
      id = menu->insertItem( iconSet(), text(), popupMenu(), -1, index );
    else
      id = menu->insertItem( text(), popupMenu(), -1, index );

    if ( !isEnabled() )
      menu->setItemEnabled( id, false );

    addContainer( menu, id );
    connect( menu, SIGNAL( destroyed() ), this, SLOT( slotDestroyed() ) );

    return containerCount() - 1;
  }
  else if ( widget->inherits( "KToolBar" ) )
  {
    KToolBar *bar = static_cast<KToolBar *>( widget );

    int id_ = KAction::getToolButtonID();

    if ( icon().isEmpty() && !iconSet().isNull() )
      bar->insertButton( iconSet().pixmap(), id_, SIGNAL( clicked() ), this,
                          SLOT( slotActivated() ), isEnabled(), plainText(),
                          index );
    else
    {
      KInstance *instance;

      if ( m_parentCollection )
        instance = m_parentCollection->instance();
      else
        instance = KGlobal::instance();

      bar->insertButton( icon(), id_, SIGNAL( clicked() ), this,
                          SLOT( slotActivated() ), isEnabled(), plainText(),
                          index, instance );
    }

    addContainer( bar, id_ );

    if (!whatsThis().isEmpty())
      QWhatsThis::add( bar->getButton(id_), whatsThis() );

    connect( bar, SIGNAL( destroyed() ), this, SLOT( slotDestroyed() ) );

    bar->getButton(id_)->setPopup(popupMenu(), true );

    return containerCount() - 1;
  }
  else if ( widget->inherits( "QMenuBar" ) )
  {
    QMenuBar *bar = static_cast<QMenuBar *>( widget );

    int id;

    id = bar->insertItem( text(), popupMenu(), -1, index );

    if ( !isEnabled() )
      bar->setItemEnabled( id, false );

    addContainer( bar, id );
    connect( bar, SIGNAL( destroyed() ), this, SLOT( slotDestroyed() ) );

    return containerCount() - 1;
  }

  return -1;
}

void KivioStencilSetAction::updateMenu()
{
  m_id = 0;
  m_popup->clear();

  m_childMenuList.clear();
  m_pathList.clear();

  KStandardDirs *dirs = KGlobal::dirs();
  QStringList dirList = dirs->findDirs("data", "kivio/stencils");
  dirList.sort();

  for( QStringList::Iterator it = dirList.begin(); it != dirList.end(); ++it )
  {
    QString dir = (*it);
    loadCollections(dir);
  }
}

void KivioStencilSetAction::loadCollections( const QString& rootDirStr )
{
  QDir rootDir( rootDirStr );

  rootDir.setFilter( QDir::Dirs );
  rootDir.setSorting( QDir::Name );

  const QFileInfoList *colList = rootDir.entryInfoList();
  QFileInfoListIterator colIt( *colList );
  QFileInfo *colFInfo;

  while( (colFInfo = colIt.current()) )
  {
    if( colFInfo->fileName() != ".." &&
        colFInfo->fileName() != "." )
    {
      KPopupMenu* ch = new KPopupMenu;
      connect(ch,SIGNAL(activated(int)),SLOT(slotActivated(int)));
      loadSet( ch, rootDirStr + "/" + colFInfo->fileName() );
      m_popup->insertItem(QIconSet(dirtPixmap(colFInfo->absFilePath())),
        KivioStencilSpawnerSet::readTitle(colFInfo->absFilePath()),ch);
      m_childMenuList.append(ch);
    }
    ++colIt;
  }
}

void KivioStencilSetAction::loadSet( KPopupMenu* menu, const QString& rootDirStr )
{
  QDir rootDir( rootDirStr );

  rootDir.setFilter( QDir::Dirs );
  rootDir.setSorting( QDir::Name );

  const QFileInfoList *setList = rootDir.entryInfoList();
  QFileInfoListIterator setIt( *setList );
  QFileInfo *setFInfo;

  while( (setFInfo = setIt.current()) )
  {
    if( setFInfo->fileName() != ".." && setFInfo->fileName() != "." )
    {
      menu->insertItem(QIconSet(dirtPixmap(setFInfo->absFilePath())),
        KivioStencilSpawnerSet::readTitle(setFInfo->absFilePath()),m_id);
      m_pathList.insert( m_id, new QString(rootDirStr + "/" + setFInfo->fileName()) );
      m_id++;
    }
    ++setIt;
  }
}

QPixmap KivioStencilSetAction::dirtPixmap( const QString& dir )
{
    QFile file( dir + "/icon.xpm" );
    QFileInfo finfo( file );

    if( finfo.exists()==false )
    {
        return QPixmap( (const char **)default_plug_xpm );
    }

    return QPixmap( finfo.absFilePath() );
}

void KivioStencilSetAction::slotActivated(int id)
{
  if (id<0 || m_pathList.at((uint)id) == 0L )
    return;

  QString path = QString(*m_pathList.at((uint)id));
  m_pathList.clear();

  emit activated(path);
}
