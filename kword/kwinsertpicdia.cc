/* This file is part of the KDE project
   Copyright (C)  2001 David Faure <faure@kde.org>

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

#include "kwinsertpicdia.h"
#include <koPictureFilePreview.h>
#include <klocale.h>
#include <kfiledialog.h>
#include <qpainter.h>
#include <qpushbutton.h>
#include <qpicture.h>
#include <qbitmap.h>
#include <qlayout.h>
#include <qcheckbox.h>
#include <kimageio.h>
#include <kio/netaccess.h>
#include <koClipartCollection.h>
#include <kdebug.h>

/**
 * This is the preview that appears on the right of the "Insert picture" dialog.
 * Not to be confused with KoPictureFilePreview, the one inside the file dialog!
 * (Note: this one has to remain separate, for the day we add options like flipping, rotating, etc.)
 */
class KWInsertPicPreview : public QFrame
{
public:
    KWInsertPicPreview( QWidget *parent )
        : QFrame( parent )
    {
        setFrameStyle( WinPanel | Sunken );
        setMinimumSize( 300, 200 );
        m_type = IPD_IMAGE;
    }

    virtual ~KWInsertPicPreview() {}

    bool setPixmap( const QString & filename )
    {
        m_pixmap.load( filename );
        if ( m_pixmap.height() > 0 ) // we divide by the height in kwcanvas...
        {
            m_type = IPD_IMAGE;
            //kdDebug() << "setPixmap: m_pixmap is " << m_pixmap.width() << ", " << m_pixmap.height() << endl;
            const QBitmap nullBitmap;
            m_pixmap.setMask( nullBitmap );  //don't show transparency
            repaint(false);
            return true;
        }
        return false;
    }

    QSize pixmapSize() const { return m_type == IPD_IMAGE ? m_pixmap.size() : QSize(); }

    void setClipart( const QString & filename )
    {
        m_type = IPD_CLIPART;
        m_pixmap = QPixmap();
        (void)KoClipartCollection::loadFromFile( filename, &m_picture );
        repaint(false);
    }

    void drawContents( QPainter *p )
    {
        QFrame::drawContents( p );
        p->save();
        p->translate( contentsRect().x(), contentsRect().y() );
        if ( m_type == IPD_IMAGE )
        {
            p->drawPixmap( QPoint( 0, 0 ), m_pixmap );
        }
        else
        {
            QRect br = m_picture.boundingRect();
            if ( br.width() && br.height() )
                p->scale( (double)width() / (double)br.width(), (double)height() / (double)br.height() );
            p->drawPicture( m_picture );
        }
        p->restore();
    }
private:
    enum { IPD_IMAGE, IPD_CLIPART } m_type;
    QPixmap m_pixmap;
    QPicture m_picture;
};

//////////////

KWInsertPicDia::KWInsertPicDia( QWidget *parent, const char *name )
    : KDialogBase( Plain, i18n("Insert Picture"), Ok|Cancel, Ok, parent, name, true )
{
    setInitialSize( QSize(400, 300) );

    QWidget *page = plainPage();
    QGridLayout *grid = new QGridLayout( page, 5, 2, KDialog::marginHint(), KDialog::spacingHint() );

    QPushButton *pbImage = new QPushButton( i18n( "Choose &Image" ), page );
    grid->addWidget( pbImage, 0, 0 );
    connect( pbImage, SIGNAL( clicked() ), SLOT( slotChooseImage() ) );

    QPushButton *pbClipart = new QPushButton( i18n( "Choose &Clipart" ), page );
    grid->addWidget( pbClipart, 1, 0 );
    connect( pbClipart, SIGNAL( clicked() ), SLOT( slotChooseClipart() ) );

    m_cbInline = new QCheckBox( i18n( "Insert Picture Inline" ), page );
    grid->addWidget( m_cbInline, 2, 0 );

    m_cbKeepRatio= new QCheckBox( i18n("Retain original aspect ratio"),page);
    grid->addWidget( m_cbKeepRatio,3,0);

    m_preview = new KWInsertPicPreview( page );
    grid->addMultiCellWidget( m_preview, 0, 4, 1, 1 );

    // Stretch the buttons and checkboxes a little, but stretch the preview much more
    grid->setRowStretch( 0, 1 );
    grid->setRowStretch( 1, 1 );
    grid->setRowStretch( 2, 1 );
    grid->setRowStretch( 3, 1 );
    grid->setRowStretch( 4, 10);
    grid->setColStretch( 0, 1 );
    grid->setColStretch( 1, 10 );
    m_cbKeepRatio->setChecked(true);
    enableButtonOK( false );
    setFocus();
}

bool KWInsertPicDia::makeInline() const
{
    return m_cbInline->isChecked();
}

bool KWInsertPicDia::keepRatio() const
{
    return m_cbKeepRatio->isChecked();
}

void KWInsertPicDia::slotChooseImage()
{
    if ( KWInsertPicDia::selectPictureDia(m_filename ) )
    {
        if ( m_preview->setPixmap( m_filename ) )
        {
            m_type = IPD_IMAGE;
            enableButtonOK( true );
            m_cbKeepRatio->setEnabled( true );
            m_cbKeepRatio->setChecked( true );
        }
    }
}

bool KWInsertPicDia::selectPictureDia( QString &filename, const QString & _path)
{
    KFileDialog fd( _path, KImageIO::pattern(KImageIO::Reading), 0, 0, TRUE );
    fd.setCaption(i18n("Choose Image"));
    QString file = selectPicture( fd );
    if ( !file.isEmpty() )
    {
        filename = file;
        return true;
    }
    return false;
}



void KWInsertPicDia::slotChooseClipart()
{
    if ( KWInsertPicDia::selectClipartDia(m_filename ) )
    {
        m_type = IPD_CLIPART;
        m_preview->setClipart( m_filename );
        enableButtonOK( true );
        m_cbKeepRatio->setEnabled( false );
        m_cbKeepRatio->setChecked( false );
    }
}


bool KWInsertPicDia::selectClipartDia( QString &filename, const QString & _path)
{
    KFileDialog fd( _path, KoPictureFilePreview::clipartPattern(), 0, 0, true );
    fd.setCaption(i18n("Choose Clipart"));
    QString file = selectPicture( fd );
    if ( !file.isEmpty() )
    {
        filename = file;
        return true;
    }
    return false;
}

QString KWInsertPicDia::selectPicture( KFileDialog & fd )
{
    fd.setPreviewWidget( new KoPictureFilePreview( &fd ) );
    KURL url;
    if ( fd.exec() == QDialog::Accepted )
        url = fd.selectedURL();

    if( url.isEmpty() )
      return QString::null;

    QString chosen = QString::null;
    if (!KIO::NetAccess::download( url, chosen ))
        return QString::null;
    return chosen;
}

QSize KWInsertPicDia::pixmapSize() const
{
    return m_preview->pixmapSize();
}

#include "kwinsertpicdia.moc"
