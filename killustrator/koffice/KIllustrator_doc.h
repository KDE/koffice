/* -*- C++ -*-

  $Id$

  This file is part of KIllustrator.
  Copyright (C) 1998 Kai-Uwe Sattler (kus@iti.cs.uni-magdeburg.de)

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU Library General Public License as
  published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

*/

#ifndef KIllustrator_doc_h_
#define KIllustrator_doc_h_

#include <koDocument.h>
#include <koDocumentChild.h>

#include <GDocument.h>
#include <qcstring.h>

class KIllustratorDocument;
class KIllustratorView;
class KoDocumentEntry;
class KoStore;
class GDocument;
class GPart;
class QDomDocument;

class KIllustratorChild : public KoDocumentChild
{
public:
    KIllustratorChild( KIllustratorDocument* );
    KIllustratorChild( KIllustratorDocument* killu, KoDocument* doc, const QRect& geometry );
    ~KIllustratorChild();

    KIllustratorDocument* killuParent () { return (KIllustratorDocument*)parent(); }
};

class KIllustratorDocument : public KoDocument
{
    Q_OBJECT
public:
    KIllustratorDocument( QWidget *parentWidget = 0, const char *widgetName = 0, QObject* parent = 0, const char* name = 0, bool singleViewMode = false );
    ~KIllustratorDocument ();

    // Overloaded methods from KoDocument

    virtual bool saveChildren (KoStore* _store, const char *_path);
    virtual QDomDocument saveXML() { return m_gdocument->saveToXml(); }
    //virtual bool save() { return KParts::ReadWritePart::save(); }

    //bool completeSaving (KoStore* store); // not needed? - what about embedded wmf files?

    virtual bool loadXML (QIODevice *, const QDomDocument &document);
    bool loadChildren (KoStore* store);

    /**
     * Overloaded @ref Part::createShell
     */
    KoMainWindow* createShell();

    /**
     * Overloaded @ref KoDocument::initDoc.
     */
    bool initDoc ();

    /**
     * Overloaded @ref KoDocument::mimeType.
     */
    QCString mimeType() const { return "application/x-killustrator"; }

    /**
     * Overloaded @ref ContainerPart::insertChild.
     */
    void insertChild( KoDocumentChild* child );
	
    /**
     * Overloaded @ref Part::paintContent
     */
    void paintContent( QPainter& painter, const QRect& rect, bool transparent );
	
    // Killustrator stuff
    GDocument* gdoc();

    void insertPart (const QRect& rect, KoDocumentEntry& e);
    void changeChildGeometry (KIllustratorChild* child, const QRect& r);

  bool insertNewTemplate (int, int, bool clean = false);

signals:
    void partInserted (KIllustratorChild* child, GPart* part);
    void childGeometryChanged (KIllustratorChild* child);

protected:
    /**
     * Overloaded @ref KoDocument::createViewInstance
     */
    KoView* createViewInstance( QWidget* parent, const char* name );

private:
    GDocument* m_gdocument;
};

#endif
