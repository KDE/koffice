/* This file is part of the KDE project
   Copyright (C) 2010 Thorsten Zachmann <zachmann@kde.org>

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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#ifndef SCPICTURESIMPORT_H
#define SCPICTURESIMPORT_H

#include <QObject>
#include <KUrl>

class KShapeFactoryBase;
class KoPAPageBase;
class KoPAMasterPage;
class SCDocument;
class SCView;
class KJob;
class QUndoCommand;

class SCPicturesImport : public QObject
{
    Q_OBJECT
public:
    SCPicturesImport();

    void import(SCView *view);

private slots:
    // starts the transfer of the next image
    void import();
    void pictureImported(KJob *job);

private:
    SCDocument *m_doc;
    KUrl::List m_urls;
    KoPAPageBase *m_currentPage;
    KoPAMasterPage *m_masterPage;
    KShapeFactoryBase *m_factory;
    QUndoCommand *m_cmd;
};

#endif /* SCPICTURESIMPORT_H */
