/* This file is part of the KDE project
   Copyright (c) 2000 Simon Hausmann <hausmann@kde.org>

   $Id$

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

#ifndef __koDocumentInfoDlg_h__
#define __koDocumentInfoDlg_h__

#include <qobject.h>

#include <kpropsdlg.h>

class KDialogBase;
class KoDocumentInfo;
class KoDocumentInfoAuthor;
class KoDocumentInfoAbout;
class KArchiveEntry;

class KoDocumentInfoDlg : public QObject
{
  Q_OBJECT
public:
  KoDocumentInfoDlg( KoDocumentInfo *docInfo, QWidget *parent = 0, const char *name = 0,
		     KDialogBase *dialog = 0 );
  virtual ~KoDocumentInfoDlg();

  int exec();
  KDialogBase *dialog() const;

  void save();

signals:
  void changed();

private:
  void addAuthorPage( KoDocumentInfoAuthor *authorInfo );
  void addAboutPage( KoDocumentInfoAbout *aboutInfo );

  void save( KoDocumentInfoAuthor *authorInfo );
  void save( KoDocumentInfoAbout *aboutInfo );

  class KoDocumentInfoDlgPrivate;
  KoDocumentInfoDlgPrivate *d;
};

class KoDocumentInfoPropsPage : public KPropsDlgPlugin
{
  Q_OBJECT
public:
  KoDocumentInfoPropsPage( KPropertiesDialog *props );
  virtual ~KoDocumentInfoPropsPage();

  virtual void applyChanges();

private:
  void copy( const QString &path, const KArchiveEntry *entry );
  class KoDocumentInfoPropsPagePrivate;
  KoDocumentInfoPropsPagePrivate *d;
};

#endif
