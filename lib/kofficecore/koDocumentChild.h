/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>

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
#ifndef __koDocumentChild_h__
#define __koDocumentChild_h__

#include <qtextstream.h>
#include <qwmatrix.h>
#include <komlParser.h>
#include <koChild.h>

class QDomDocument;
class QDomElement;
class KURL;
class KoStore;
class KoDocument;
class KoDocumentChildPrivate;

/**
 * Holds an embedded object.
 *
 * In addition to its base class @ref KoChild it cares about the content
 * of an embedded document. That means it supports operations like
 * loading and saving.
 *
 * If you need a representation for embedded documents in your KOffice
 * component then you should inherit from this class.
 */
class KoDocumentChild : public KoChild
{
  Q_OBJECT
public:
  KoDocumentChild( KoDocument* parent, KoDocument* doc, const QRect& geometry );

  /**
   * When using this constructor you must call @ref setDocument before
   * you can call any other function of this class.
   */
  KoDocumentChild( KoDocument* parent );

  virtual ~KoDocumentChild();

  /**
   * Call this function only directly after calling the constructor
   * that takes only a parent as argument.
   */
  virtual void setDocument( KoDocument *doc, const QRect &geometry );

  virtual KoDocument *document() const;

  virtual KoDocument *parentDocument() const;

  virtual KoDocument* hitTest( const QPoint& p, const QWMatrix& _matrix = QWMatrix() );

  /**
   * Can be empty (which is why it doesn't return a const KURL &)
   */
  virtual KURL url();

  /**
   *  Writes the "object" tag, but does NOT write the content of the
   *  embedded documents. Saving the embedded documents themselves
   *  is done in @ref KoDocument::saveChildren. This function just stores information
   *  about the position and id of the embedded document and should be
   *  called from within KoDocument::saveXML.
   *
   *  The "object" element is not added to the document. It is just created
   *  and returned.
   *
   *  @return the element containing the "object" tag.
   */
  virtual QDomElement save( QDomDocument& doc );

  /**
   *  Parses the "object" tag. This does NOT mean creating the child documents.
   *  AFTER the 'parser' finished parsing, you must use @ref #loadDocument
   *  to actually load the embedded documents.
   *
   *  What you hsould have in mind is that this method is called from within
   *  @ref KoDocument::loadXML while @ref #loadDocument is called from within
   *  @ref KoDocument::loadChildren, respectively from your implementation
   *  of these methods.
   */
  virtual bool load( const QDomElement& element );

  /**
   *  Actually loads the document from the disk/net or from the store,
   *  depending on @ref #url
   */
  virtual bool loadDocument( KoStore* );

  virtual bool isStoredExtern();

protected:
  // #### Torben: Is that still needed ?
  /**
   * Called if @ref #load finds a tag that it does not understand.
   *
   * @return TRUE if the tag could be handled. The default implementation
   *         returns FALSE.
   */
  virtual bool loadTag( KOMLParser& parser, const string& tag, std::vector<KOMLAttrib>& lst2 );

protected: // TODO private: when KOML is dead

  /**
   *  Holds the source of this object, for example "file:/home/weis/image.gif"
   *  or "tar:/table1/2" if it is stored in a koffice store. This variable is
   *  set after parsing the OBJECT tag in @ref #load and is reset after
   *  calling @ref #loadDocument.
   */
  QString m_tmpURL;

  /**
   * This variable is
   *  set after parsing the OBJECT tag in @ref #load and is reset after
   *  calling @ref #loadDocument.
   */
  QRect m_tmpGeometry;

  /**
   * This variable is
   *  set after parsing the OBJECT tag in @ref #load and is reset after
   *  calling @ref #loadDocument.
   */
  QString m_tmpMimeType;

  KoDocumentChildPrivate *d;
};

#endif
