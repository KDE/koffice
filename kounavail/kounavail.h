/* This file is part of the KDE project
   Copyright (C) 2001 David Faure <david@mandrakesoft.com>

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

#ifndef KOUNAVAIL_H
#define KOUNAVAIL_H

#include <koDocument.h>

class KoUnavailPart : public KoDocument
{
    Q_OBJECT
    Q_PROPERTY( QCString mimetype READ nativeFormatMimeType WRITE setMimeType )
    Q_PROPERTY( QString unavailReason READ unavailReason WRITE setUnavailReason )
    Q_PROPERTY( QString realURL READ realURL WRITE setRealURL )
public:
    KoUnavailPart( QWidget *parentWidget = 0, const char *widgetName = 0, QObject* parent = 0, const char* name = 0 );

    virtual void paintContent( QPainter& painter, const QRect& rect, bool transparent = FALSE, double zoomX = 1.0, double zoomY = 1.0 );

    virtual bool initDoc() { return true; }

    virtual bool loadXML( QIODevice *, const QDomDocument & );
    virtual bool saveFile();
    virtual QDomDocument saveXML();
    virtual bool saveChildren( KoStore* /*_store*/, const QString& /*_path*/ ) { return true; }

    /** This is called by KoDocumentChild::save */
    virtual QCString nativeFormatMimeType() const { return m_mimetype; }
    /** This is called by KoDocumentChild::createUnavailDocument */
    void setMimeType( const QCString& mime );
    // keep in sync with koDocumentChild.h
    enum UnavailReason { DocumentNotFound, HandlerNotFound };
    /** This is called by KoDocumentChild::createUnavailDocument */
    void setUnavailReason( const QString& reason ) { m_reason = reason; }
    // stupid moc - I want a write-only property !
    QString unavailReason() const { return m_reason; }
    /** This is called by KoDocumentChild::createUnavailDocument
     * Note the trick: we directly modify the URL of the document,
     * the one returned by KPart's url()
     */
    void setRealURL( const QString& u ) { m_url = u; }
    // stupid moc again
    QString realURL() const { return m_url.url(); }

protected:
    virtual KoView* createViewInstance( QWidget* parent, const char* name );

    QDomDocument m_doc;
    QCString m_mimetype;
    QString m_reason;
};

#include <koView.h>

class KoUnavailView : public KoView
{
    Q_OBJECT
public:
    KoUnavailView( KoUnavailPart* part, QWidget* parent = 0, const char* name = 0 );

protected:
    virtual void paintEvent( QPaintEvent* );
    virtual void updateReadWrite( bool ) {}
};

#include <koFactory.h>

class KInstance;
class KAboutData;

class KoUnavailFactory : public KoFactory
{
    Q_OBJECT
public:
    KoUnavailFactory( QObject* parent = 0, const char* name = 0 );
    ~KoUnavailFactory();

    virtual KParts::Part *createPartObject( QWidget *parentWidget = 0, const char *widgetName = 0, QObject *parent = 0, const char *name = 0, const char *classname = "KoDocument", const QStringList &args = QStringList() );

    static KInstance* global();

    // _Creates_ a KAboutData but doesn't keep ownership
    static KAboutData* aboutData();

private:
    static KInstance* s_global;
    static KAboutData* s_aboutData;
};

#endif
