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

#include <kparts/partmanager.h>

#include <koDocumentChild.h>
#include <koDocument.h>
#include <koQueryTrader.h>
#include <kmimetype.h>
#include <klocale.h>
#include <kmessagebox.h>

#include <qapplication.h>

#include <kdebug.h>
#include <assert.h>

// Define the protocol used here for embedded documents' URL
// This used to "store" but KURL didn't like it,
// so let's simply make it "tar" !
#define STORE_PROTOCOL "tar"
#define INTERNAL_PROTOCOL "intern"
// Warning, keep it sync in koStore.cc and koDocument.cc

/**********************************************************
 *
 * KoDocumentChild
 *
 **********************************************************/

class KoDocumentChildPrivate
{
public:
  KoDocumentChildPrivate()
  {
  }
  ~KoDocumentChildPrivate()
  {
  }

  KoDocument *m_parent;
  KoDocument *m_doc;
};

KoDocumentChild::KoDocumentChild( KoDocument* parent, KoDocument* doc, const QRect& geometry )
    : KoChild( parent )
{
  d = new KoDocumentChildPrivate;
  d->m_parent = parent;
  d->m_doc = doc;
  setGeometry( geometry );
}

KoDocumentChild::KoDocumentChild( KoDocument* parent )
    : KoChild( parent )
{
  d = new KoDocumentChildPrivate;
  d->m_parent = parent;
  d->m_doc = 0L;
}

void KoDocumentChild::setDocument( KoDocument *doc, const QRect &geometry )
{
  d->m_doc = doc;
  setGeometry( geometry );

  updateMatrix();
}

KoDocument *KoDocumentChild::document() const
{
  return d->m_doc;
}

KoDocument *KoDocumentChild::parentDocument() const
{
  return d->m_parent;
}

KoDocument *KoDocumentChild::hitTest( const QPoint &p, const QWMatrix &_matrix )
{
  if ( !region( _matrix ).contains( p ) || !document() )
    return 0L;

  QWMatrix m( _matrix );
  m = matrix() * m;
  m.scale( xScaling(), yScaling() );

  return document()->hitTest( p, m );
}

bool KoDocumentChild::load( const QDomElement& element, bool uppercase )
{
    if ( element.hasAttribute( "url" ) )
        m_tmpURL = element.attribute("url");
    if ( element.hasAttribute("mime") )
        m_tmpMimeType = element.attribute("mime");

    if ( m_tmpURL.isEmpty() )
    {
        kdDebug(30003) << "Empty 'url' attribute in OBJECT" << endl;
        return false;
    }
    if ( m_tmpMimeType.isEmpty() )
    {
        kdDebug(30003) << "Empty 'mime' attribute in OBJECT" << endl;
        return false;
    }

    bool brect = FALSE;
    QDomElement e = element.firstChild().toElement();
    for( ; !e.isNull(); e = e.nextSibling().toElement() )
    {
        if ( e.tagName() == "rect" || ( uppercase && e.tagName() == "RECT" ) )
        {
            brect = true;
            int x, y, w, h;
            x=y=w=h=0;
            if ( e.hasAttribute( "x" ) )
                x = e.attribute( "x" ).toInt(&brect);
            if ( e.hasAttribute( "y" ) )
                y = e.attribute( "y" ).toInt(&brect);
            if ( e.hasAttribute( "w" ) )
                w = e.attribute( "w" ).toInt(&brect);
            if ( e.hasAttribute( "h" ) )
                h = e.attribute( "h" ).toInt(&brect);
            m_tmpGeometry = QRect(x, y, w, h);
            setGeometry(m_tmpGeometry);
        }
    }

    if ( !brect )
    {
        kdDebug(30003) << "Missing RECT in OBJECT" << endl;
        return false;
    }

    return true;
}

bool KoDocumentChild::loadDocument( KoStore* store )
{
    assert( !m_tmpURL.isEmpty() );

    kdDebug(30003) << "KoDocumentChild::loadDocument: trying to load " << m_tmpURL << endl;

    // Backwards compatibility
    if ( m_tmpMimeType == "application/x-killustrator" )
        m_tmpMimeType = "application/x-kontour";

    KoDocumentEntry e = KoDocumentEntry::queryByMimeType( m_tmpMimeType );
    if ( e.isEmpty() )
    {
        kdWarning(30003) << "Could not create child document with type " << m_tmpMimeType << endl;
        bool res = createUnavailDocument( store, true );
        if ( res )
        {
            // Try to turn the mimetype name into its comment
            QString mimeName = m_tmpMimeType;
            KMimeType::Ptr mime = KMimeType::mimeType( m_tmpMimeType );
            if ( mime->name() != KMimeType::defaultMimeType() )
                mimeName = mime->comment();
            d->m_doc->setProperty( "unavailReason", i18n( "No handler found for %1" ).arg( mimeName ) );
        }
        return res;
    }

    return loadDocumentInternal( store, e );
}

bool KoDocumentChild::loadDocumentInternal( KoStore* _store, const KoDocumentEntry& e, bool doOpenURL )
{
    kdDebug(30003) << "KoDocumentChild::loadDocumentInternal doOpenURL=" << doOpenURL << " m_tmpURL=" << m_tmpURL << endl;
    KoDocument * doc = e.createDoc( d->m_parent );
    if (!doc) {
        kdWarning(30003) << "createDoc failed" << endl;
        return false;
    }
    setDocument( doc, m_tmpGeometry );

    bool res = true;
    if ( doOpenURL )
    {
        bool internalURL = false;
        if ( m_tmpURL.startsWith( STORE_PROTOCOL ) || KURL::isRelativeURL( m_tmpURL ) )
        {
            res = document()->loadFromStore( _store, m_tmpURL );
            internalURL = true;
        }
        else
        {
            // Reference to an external document. Hmmm...
            KURL url( m_tmpURL );
            if ( !url.isLocalFile() )
            {
                QApplication::restoreOverrideCursor();
                // For security reasons we need to ask confirmation if the url is remote
                int result = KMessageBox::warningYesNoCancel(
                    0, i18n( "This document contains an external link to a remote document\n%1").arg(m_tmpURL),
                    i18n( "Confirmation required" ), i18n( "Download" ), i18n( "Skip" ) );

                if ( result == KMessageBox::Cancel )
                {
                    d->m_parent->setErrorMessage("USER_CANCELED");
                    return false;
                }
                if ( result == KMessageBox::Yes )
                    res = document()->openURL( url );
                // and if == No, res will still be false so we'll use a kounavail below
            }
            else
                res = document()->openURL( url );
        }
        if ( !res )
        {
            delete d->m_doc;
            d->m_doc = 0;
            QString tmpURL = m_tmpURL; // keep a copy, createUnavailDocument will erase it
            // Not found -> use a kounavail instead
            res = createUnavailDocument( _store, false /* the URL doesn't exist, don't try to open it */ );
            if ( res )
            {
                d->m_doc->setProperty( "realURL", tmpURL ); // so that it gets saved correctly
                if ( internalURL )
                    d->m_doc->setProperty( "unavailReason", i18n( "Couldn't load embedded object" ) );
                else
                    d->m_doc->setProperty( "unavailReason", i18n( "External document not found:\n%1" ).arg( tmpURL ) );
            }
            return res;
        }
        // Still waiting...
        QApplication::setOverrideCursor( waitCursor );
    }

    m_tmpURL = QString::null;

    // see KoDocument::insertChild for an explanation what's going on
    // now :-)
    if ( parentDocument() )
    {
        KoDocument *parent = parentDocument();

        if ( parent->manager() && parent->manager()->parts() )
        {
            KParts::PartManager *manager = parent->manager();

            if ( !manager->parts()->containsRef( document() ) &&
                 !parent->isSingleViewMode() )
                manager->addPart( document(), false );
        }
    }

    QApplication::restoreOverrideCursor();

    return res;
}

bool KoDocumentChild::createUnavailDocument( KoStore* store, bool doOpenURL )
{
    // We don't need a trader query here. We're looking for a very specific component.
    KService::Ptr serv = KService::serviceByDesktopName( "kounavail" );
    if ( serv == 0L )
    {
        kdWarning(30003) << "ERROR: service kounavail not found " << endl;
        return false;
    }
    KoDocumentEntry e( serv );
    if ( !loadDocumentInternal( store, e, doOpenURL ) )
        return false;
    d->m_doc->setProperty( "mimetype", m_tmpMimeType );
    return true;
}

QDomElement KoDocumentChild::save( QDomDocument& doc, bool uppercase )
{
    if( document() )
    {
        QDomElement e = doc.createElement( ( uppercase ? "OBJECT" : "object" ) );
        if ( document()->url().protocol() != INTERNAL_PROTOCOL ) {
            e.setAttribute( "url", document()->url().url() );
            kdDebug() << "KoDocumentChild::save url=" << document()->url().url() << endl;
        }
        else {
            e.setAttribute( "url", document()->url().path().mid( 1 ) );
            kdDebug() << "KoDocumentChild::save url=" << document()->url().path().mid( 1 ) << endl;
        }
        e.setAttribute( "mime", document()->nativeFormatMimeType() );
        kdDebug() << "KoDocumentChild::save mime=" << document()->nativeFormatMimeType() << endl;
        QDomElement rect = doc.createElement( ( uppercase ? "RECT" : "rect" ) );
        rect.setAttribute( "x", geometry().left() );
        rect.setAttribute( "y", geometry().top() );
        rect.setAttribute( "w", geometry().width() );
        rect.setAttribute( "h", geometry().height() );
        e.appendChild(rect);
        return e;
    }
    return QDomElement();
}

bool KoDocumentChild::isStoredExtern()
{
  const KURL & url = document()->url();
  if ( !url.hasPath() )
    return false;
  else if ( url.protocol() == STORE_PROTOCOL )
    return false;
  else if ( url.protocol() == INTERNAL_PROTOCOL )
    return false;

  return true;
}

KURL KoDocumentChild::url()
{
    return ( document() ? document()->url() : KURL() );
}

KoDocumentChild::~KoDocumentChild()
{
  if ( d->m_doc ) {
    delete d->m_doc;
    d->m_doc=0L;
  }
  delete d;
}

#include <koDocumentChild.moc>
