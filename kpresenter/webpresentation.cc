// -*- Mode: c++; c-basic-offset: 4; indent-tabs-mode: nil; tab-width: 4; -*-
/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Reginald Stadlbauer <reggie@kde.org>
   Copyright 2001, 2002 Nicolas GOUTTE <goutte@kde.org>
   Copyright 2002 Ariya Hidayat <ariya@kde.org>

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

#include "webpresentation.h"

#include "kpresenter_view.h"
#include "kprcanvas.h"

#include <kstandarddirs.h>
#include <unistd.h>
#include <sys/types.h>

#include <qfile.h>
#include <qtextstream.h>
#include <qvbox.h>
#include <qhbox.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qfileinfo.h>
#include <qframe.h>
#include <qfont.h>
#include <qpixmap.h>
#include <qdatetime.h>
#include <qdir.h>
#include <qheader.h>
#include <qwmatrix.h>
#include <qtextcodec.h>
#include <qregexp.h>
#include <qimage.h>
#include <qlayout.h>

#include <kdebug.h>
#include <klocale.h>
#include <kcolorbutton.h>
#include <kfiledialog.h>
#include <kmessagebox.h>
#include <kbuttonbox.h>
#include <ksimpleconfig.h>
#include <kapplication.h>
#include <kprogress.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <kcharsets.h>
#include <kurlrequester.h>
#include <klineedit.h>
#include <klistview.h>
#include <knuminput.h>
#include <kcombobox.h>
#include <kurl.h>
#include <kio/netaccess.h>
#include <kdialog.h>

#include "koDocumentInfo.h"


// Comes from koffice/filters/libexport/KWEFUtils.cc
static QString EscapeSgmlText(const QTextCodec* codec, const QString& strIn,
                              const bool quot = false , const bool apos = false )
{
    QString strReturn;
    QChar ch;

    for (uint i=0; i<strIn.length(); i++)
    {
        ch=strIn[i];
        switch (ch.unicode())
        {
        case 38: // &
        {
            strReturn+="&amp;";
            break;
        }
        case 60: // <
        {
            strReturn+="&lt;";
            break;
        }
        case 62: // >
        {
            strReturn+="&gt;";
            break;
        }
        case 34: // "
        {
            if (quot)
                strReturn+="&quot;";
            else
                strReturn+=ch;
            break;
        }
        case 39: // '
        {
            // NOTE:  HTML does not define &apos; by default (only XML/XHTML does)
            if (apos)
                strReturn+="&apos;";
            else
                strReturn+=ch;
            break;
        }
        default:
        {
            // verify that the character ch can be expressed in the
            //   encoding in which we will write the HTML file.
            if (codec)
            {
                if (!codec->canEncode(ch))
                {
                    strReturn+=QString("&#%1;").arg(ch.unicode());
                    break;
                }
            }
            strReturn+=ch;
            break;
        }
        }
    }

    return strReturn;
}

// Escape only if the encoding do not support the character
// Special SGML characters like < > & are supposed to be already escaped.
static QString EscapeEncodingOnly(const QTextCodec* codec, const QString& strIn)
{
    QString strReturn;
    QChar ch;

    for (uint i=0; i<strIn.length(); i++)
    {
        ch=strIn[i];
        if (codec)
        {
            if (!codec->canEncode(ch))
            {
                strReturn+=QString("&#%1;").arg(ch.unicode());
                continue;
            }
        }
        strReturn+=ch;
    }
    return strReturn;
}

KPWebPresentation::KPWebPresentation( KPresenterDoc *_doc, KPresenterView *_view )
    : config( QString::null ), xml( false )
{
    doc = _doc;
    view = _view;
    init();
}

KPWebPresentation::KPWebPresentation( const QString &_config, KPresenterDoc *_doc, KPresenterView *_view )
    : config( _config ), xml( false )
{
    doc = _doc;
    view = _view;
    init();
    loadConfig();
}

KPWebPresentation::KPWebPresentation( const KPWebPresentation &webPres )
    : config( webPres.config ), author( webPres.author ), title( webPres.title ), email( webPres.email ),
      slideInfos( webPres.slideInfos ), backColor( webPres.backColor ), titleColor( webPres.titleColor ),
      textColor( webPres.textColor ), path( webPres.path ),
      xml( webPres.xml), zoom( webPres.zoom ), m_encoding( webPres.m_encoding )
{
    doc = webPres.doc;
    view = webPres.view;
}

void KPWebPresentation::loadConfig()
{
    if ( config.isEmpty() )
        return;

    KSimpleConfig cfg( config );
    cfg.setGroup( "General" );

    author = cfg.readEntry( "Author", author );
    title = cfg.readEntry( "Title", title );
    email = cfg.readEntry( "EMail", email );
    unsigned int num = cfg.readNumEntry( "Slides", slideInfos.count() );
    //kdDebug(33001) << "KPWebPresentation::loadConfig num=" << num << endl;

    if ( num <= slideInfos.count() ) {
        for ( unsigned int i = 0; i < num; i++ )
        {
            QString key = QString::fromLatin1( "SlideTitle%1" ).arg( i );
            if ( cfg.hasKey( key ) )
            {
                // We'll assume that the selected pages haven't changed... Hmm.
                slideInfos[ i ].slideTitle = cfg.readEntry( key );
                kdDebug(33001) << "KPWebPresentation::loadConfig key=" << key << " data=" << slideInfos[i].slideTitle << endl;
            } else kdDebug(33001) << " key not found " << key << endl;
        }
    }

    backColor = cfg.readColorEntry( "BackColor", &backColor );
    titleColor = cfg.readColorEntry( "TitleColor", &titleColor );
    textColor = cfg.readColorEntry( "TextColor", &textColor );
    path = cfg.readPathEntry( "Path", path );
    xml = cfg.readBoolEntry( "XML", xml );
    zoom = cfg.readNumEntry( "Zoom", zoom );
    m_encoding = cfg.readEntry( "Encoding", m_encoding );
}

void KPWebPresentation::saveConfig()
{
    KSimpleConfig cfg( config );
    cfg.setGroup( "General" );

    cfg.writeEntry( "Author", author );
    cfg.writeEntry( "Title", title );
    cfg.writeEntry( "EMail", email );
    cfg.writeEntry( "Slides", slideInfos.count() );

    for ( unsigned int i = 0; i < slideInfos.count(); i++ )
        cfg.writeEntry( QString::fromLatin1( "SlideTitle%1" ).arg( i ), slideInfos[ i ].slideTitle );

    cfg.writeEntry( "BackColor", backColor );
    cfg.writeEntry( "TitleColor", titleColor );
    cfg.writeEntry( "TextColor", textColor );
#if KDE_IS_VERSION(3,1,3)
    cfg.writePathEntry( "Path", path );
#else
    cfg.writeEntry( "Path", path );
#endif
    cfg.writeEntry( "XML", xml );
    cfg.writeEntry( "Zoom", zoom );
    cfg.writeEntry( "Encoding", m_encoding );
}

void KPWebPresentation::initCreation( KProgress *progressBar )
{
    QString cmd;
    int p;

    QDir( path ).mkdir( path + "/html" );

    p = progressBar->progress();
    progressBar->setProgress( ++p );
    kapp->processEvents();

    QDir( path ).mkdir( path + "/pics" );

    p = progressBar->progress();
    progressBar->setProgress( ++p );
    kapp->processEvents();

    const char *pics[] = { "home", "first", "next", "prev", "last", 0 };

    KURL srcurl, desturl;

    for ( uint index = 0; pics[ index ]; index ++ )
    {
        QString filename = pics[ index ];
        filename += ".png";
        srcurl.setPath( locate( "slideshow", filename, KPresenterFactory::global() ) );
        desturl.setPath ( path + "/pics/" + filename );
        KIO::NetAccess::del( desturl ); // Copy does not remove existing destination file
        KIO::NetAccess::copy( srcurl, desturl );
        p = progressBar->progress();
        progressBar->setProgress( ++p );
        kapp->processEvents();
    }
}

void KPWebPresentation::createSlidesPictures( KProgress *progressBar )
{
    if ( slideInfos.isEmpty() )
        return;
    QPixmap pix( 10, 10 );
    QString filename;
    int p;
    for ( unsigned int i = 0; i < slideInfos.count(); i++ ) {
        int pgNum = slideInfos[i].pageNumber;
        view->getCanvas()->drawPageInPix( pix, pgNum, zoom, true /*force real variable value*/ );
        filename = QString( "%1/pics/slide_%2.png" ).arg( path ).arg( i + 1 );

        pix.save( filename, "PNG" );

        p = progressBar->progress();
        progressBar->setProgress( ++p );
        kapp->processEvents();
    }
}

QString KPWebPresentation::escapeHtmlText( QTextCodec *codec, const QString& strText ) const
{
    // Escape quotes (needed in attributes)
    // Do not escape apostrophs (only allowed in XHTML!)
    return EscapeSgmlText( codec, strText, true, false );
}

void KPWebPresentation::writeStartOfHeader(QTextStream& streamOut, QTextCodec *codec, const QString& subtitle)
{
    QString mimeName ( codec->mimeName() );
    if ( isXML() )
    {   //Write out the XML declaration
        streamOut << "<?xml version=\"1.0\" encoding=\""
                  << mimeName << "\"?>\n";
    }
    // write <!DOCTYPE
    streamOut << "<!DOCTYPE ";
    if ( isXML() )
    {
        streamOut << "html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\"";
        streamOut << " \"DTD/xhtml1-transitional.dtd\">\n";
    }
    else
    {
        streamOut << "HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\"";
        streamOut << " \"http://www.w3.org/TR/html4/loose.dtd\">\n";
    }
    streamOut << "<html";
    if ( isXML() )
    {
        // XHTML has an extra attribute defining its namespace (in the <html> opening tag)
        streamOut << " xmlns=\"http://www.w3.org/1999/xhtml\"";
    }
    streamOut << ">\n" << "<head>\n";

    // Declare what charset we are using
    streamOut << "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=";
    streamOut << mimeName << '"' << ( isXML() ?" /":"") << ">\n" ;

    // Tell who we are (with the CVS revision number) in case we have a bug in our output!
    QString strVersion("$Revision$");
    // Eliminate the dollar signs
    //  (We don't want that the version number changes if the HTML file is itself put in a CVS storage.)
    streamOut << "<meta name=\"Generator\" content=\"KPresenter's Web Presentation "
              << strVersion.mid(10).replace("$","")
              << "\""<< ( isXML() ?" /":"") // X(HT)ML closes empty elements, HTML not!
              << ">\n";

    streamOut << "<title>"<< escapeHtmlText( codec, title ) << " - " << escapeHtmlText( codec, subtitle ) << "</title>\n";

    // ### TODO: transform documentinfo.xml into many <META> elements (at least the author!)
}

void KPWebPresentation::createSlidesHTML( KProgress *progressBar )
{
    QTextCodec *codec = KGlobal::charsets()->codecForName( m_encoding );

    const QString brtag ( "<br" + QString(isXML()?" /":"") + ">" );

    for ( unsigned int i = 0; i < slideInfos.count(); i++ ) {

        unsigned int pgNum = i + 1;

        QFile file( QString( "%1/html/slide_%2.html" ).arg( path ).arg( pgNum ) );
        file.open( IO_WriteOnly );
        QTextStream streamOut( &file );
        streamOut.setCodec( codec );

        writeStartOfHeader( streamOut, codec, slideInfos[ i ].slideTitle );

        // ### TODO: transform documentinfo.xml into many <META> elements (at least the author!)

        if ( i > 0 ) {
            streamOut <<  "<link rel=\"first\" href=\"slide_1.html\"" << ( isXML() ?" /":"") << ">\n";
            streamOut <<  "<link rel=\"prev\" href=\"slide_" << pgNum - 1 << ".html\"" << ( isXML() ?" /":"") << ">\n";
        }
        if ( i < slideInfos.count() - 1 ) {
            streamOut <<  "<link rel=\"next\" href=\"slide_" << pgNum + 1 << ".html\"" << ( isXML() ?" /":"") << ">\n";
            streamOut <<  "<link rel=\"last\" href=\"slide_" << slideInfos.count() << ".html\"" << ( isXML() ?" /":"") << ">\n";
        }
        streamOut <<  "<link rel=\"contents\" href=\"../index.html\"" << ( isXML() ?" /":"") << ">\n";

        streamOut << "</head>\n";
        streamOut << "<body bgcolor=\"" << backColor.name() << "\" text=\"" << textColor.name() << "\">\n";

        streamOut << "  <center>\n";

        if ( i > 0 )
            streamOut << "    <a href=\"slide_1.html\">";
        streamOut << "<img src=\"../pics/first.png\" border=\"0\" alt=\"" << i18n( "First" )
                  << "\" title=\"" << i18n( "First" ) << "\"" << ( isXML() ?" /":"") << ">";
        if ( i > 0 )
            streamOut << "</a>";

        streamOut << "\n";

        if ( i > 0 )
            streamOut << "    <a href=\"slide_" << pgNum - 1 << ".html\">";
        streamOut << "<img src=\"../pics/prev.png\" border=\"0\" alt=\"" << i18n( "Previous" )
                  << "\" title=\"" << i18n( "Previous" ) << "\"" << ( isXML() ?" /":"") << ">";
        if ( i > 0 )
            streamOut << "</a>";

        streamOut << "\n";

        if ( i < slideInfos.count() - 1 )
            streamOut << "    <a href=\"slide_" << pgNum + 1 << ".html\">";
        streamOut << "<img src=\"../pics/next.png\" border=\"0\" alt=\"" << i18n( "Next" )
                  << "\" title=\"" << i18n( "Next" ) << "\"" << ( isXML() ?" /":"") << ">";
        if ( i < slideInfos.count() - 1 )
            streamOut << "</a>";

        streamOut << "\n";

        if ( i < slideInfos.count() - 1 )
            streamOut << "    <a href=\"slide_" << slideInfos.count() << ".html\">";
        streamOut << "<img src=\"../pics/last.png\" border=\"0\" alt=\"" << i18n( "Last" )
                  << "\" title=\"" << i18n( "Last" ) << "\"" << ( isXML() ?" /":"") << ">";
        if ( i < slideInfos.count() - 1 )
            streamOut << "</a>";

        streamOut << "\n" << "    &nbsp; &nbsp; &nbsp; &nbsp;\n";

        streamOut << "    <a href=\"../index.html\">";
        streamOut << "<img src=\"../pics/home.png\" border=\"0\" alt=\"" << i18n( "Home" )
                  << "\" title=\"" << i18n( "Home" ) << "\"" << ( isXML() ?" /":"") << ">";
        streamOut << "</a>\n";

        streamOut << " </center>" << brtag << "<hr noshade=\"noshade\"" << ( isXML() ?" /":"") << ">\n"; // ### TODO: is noshade W3C?

        streamOut << "  <center>\n    <font color=\"" << escapeHtmlText( codec, titleColor.name() ) << "\">\n";
        streamOut << "    <b>" << escapeHtmlText( codec, title ) << "</b> - <i>" << escapeHtmlText( codec, slideInfos[ i ].slideTitle ) << "</i>\n";

        streamOut << "    </font>\n  </center><hr noshade=\"noshade\"" << ( isXML() ?" /":"") << ">" << brtag << "\n";

        streamOut << "  <center>\n    ";

        if ( i < slideInfos.count() - 1 )
            streamOut << "<a href=\"slide_" << pgNum + 1 << ".html\">";
        streamOut << "<img src=\"../pics/slide_" << pgNum << ".png\" border=\"0\" alt=\""
                  << i18n( "Slide %1" ).arg( pgNum ) << "\"" << ( isXML() ?" /":"") << ">";
        if ( i < slideInfos.count() - 1 )
            streamOut << "</a>";

        streamOut << "\n";

        streamOut << "    </center>" << brtag << "<hr noshade=\"noshade\"" << ( isXML() ?" /":"") << ">\n";

        QPtrList<KPrPage> _tmpList( doc->getPageList() );
        QString note ( escapeHtmlText( codec, _tmpList.at(i)->noteText() ) );
        if ( !note.isEmpty() ) {
            streamOut << "  <b>" << escapeHtmlText( codec, i18n( "Note" ) ) << "</b>\n";
            streamOut << " <blockquote>\n";

            streamOut << note.replace( "\n", brtag );

            streamOut << "  </blockquote><hr noshade=\"noshade\"" << ( isXML() ?" /":"") << ">\n";
        }

        streamOut << "  <center>\n";

        QString htmlAuthor;
        if (email.isEmpty())
            htmlAuthor=escapeHtmlText( codec, author );
        else
            htmlAuthor=QString("<a href=\"mailto:%1\">%2</a>").arg( escapeHtmlText( codec, email )).arg( escapeHtmlText( codec, author ));
        streamOut << EscapeEncodingOnly ( codec, i18n( "Created on %1 by <i>%2</i> with <a href=\"http://www.koffice.org/kpresenter\">KPresenter</a>" )
                                          .arg( KGlobal::locale()->formatDate ( QDate::currentDate() ) ).arg( htmlAuthor ) );

        streamOut << "    </center><hr noshade=\"noshade\"" << ( isXML() ?" /":"") << ">\n";
        streamOut << "</body>\n</html>\n";

        file.close();

        int p = progressBar->progress();
        progressBar->setProgress( ++p );
        kapp->processEvents();
    }
}

void KPWebPresentation::createMainPage( KProgress *progressBar )
{
    QTextCodec *codec = KGlobal::charsets()->codecForName( m_encoding );

    QFile file( QString( "%1/index.html" ).arg( path ) );
    file.open( IO_WriteOnly );
    QTextStream streamOut( &file );
    streamOut.setCodec( codec );

    writeStartOfHeader( streamOut, codec, i18n("Table of Contents") );
    streamOut << "</head>\n";

    streamOut << "<body bgcolor=\"" << backColor.name() << "\" text=\"" << textColor.name() << "\">\n";

    streamOut << "<h1 align=\"center\"><font color=\"" << titleColor.name()
              << "\">" << title << "</font></h1>";

    streamOut << "<p align=\"center\"><a href=\"html/slide_1.html\">";
    streamOut << i18n("Click here to start the Slideshow");
    streamOut << "</a></p>\n";

    streamOut << "<p><b>" << i18n("Table of Contents") << "</b></p>\n";

    // create list of slides (with proper link)
    streamOut << "<ol>\n";
    for ( unsigned int i = 0; i < slideInfos.count(); i++ )
        streamOut << "  <li><a href=\"html/slide_" << i+1 << ".html\">" << slideInfos[ i ].slideTitle << "</a></li>\n";
    streamOut << "</ol>\n";

    // footer: author name, e-mail
    QString htmlAuthor = email.isEmpty() ? escapeHtmlText( codec, author ) :
                         QString("<a href=\"mailto:%1\">%2</a>").arg( escapeHtmlText( codec, email )).arg( escapeHtmlText( codec, author ));
    streamOut << EscapeEncodingOnly ( codec, i18n( "Created on %1 by <i>%2</i> with <a href=\"http://www.koffice.org/kpresenter\">KPresenter</a>" )
                                      .arg( KGlobal::locale()->formatDate ( QDate::currentDate() ) ).arg( htmlAuthor ) );

    streamOut << "</body>\n</html>\n";
    file.close();

    progressBar->setProgress( progressBar->totalSteps() );
    kapp->processEvents();
}

void KPWebPresentation::init()
{

    KoDocumentInfo * info = doc->documentInfo();
    KoDocumentInfoAuthor * authorPage = static_cast<KoDocumentInfoAuthor *>(info->page( "author" ));
    if ( !authorPage )
        kdWarning() << "Author information not found in documentInfo !" << endl;
    else
    {
        author = authorPage->fullName();
        email = authorPage->email();
    }

    title = i18n("Slideshow");
    kdDebug(33001) << "KPWebPresentation::init : " << doc->getPageNums() << " pages." << endl;
    for ( unsigned int i = 0; i < doc->getPageNums(); i++ )
    {
        if ( doc->isSlideSelected( i ) )
        {
            SlideInfo info;
            info.pageNumber = i;
            info.slideTitle = doc->pageList().at(i)->pageTitle( i18n( "Slide %1" ).arg( i+1 ) );
            slideInfos.append( info );
        }
    }
    if ( slideInfos.isEmpty() )
        kdWarning() << "No slides selected!" << endl;
    backColor = Qt::white;
    textColor = Qt::black;
    titleColor = Qt::red;

    path = KGlobalSettings::documentPath() + "www";

    zoom = 100;
    m_encoding = QTextCodec::codecForLocale()->name();
}

KPWebPresentationWizard::KPWebPresentationWizard( const QString &_config, KPresenterDoc *_doc,
                                                  KPresenterView *_view )
    : KWizard( 0, "", false ), config( _config ), webPres( config, _doc, _view )
{
    doc = _doc;
    view = _view;

    setupPage1();
    setupPage2();
    setupPage3();
    setupPage4();

    connect( nextButton(), SIGNAL( clicked() ), this, SLOT( pageChanged() ) );
    connect( backButton(), SIGNAL( clicked() ), this, SLOT( pageChanged() ) );
    connect( finishButton(), SIGNAL( clicked() ), this, SLOT( finish() ) );
}

KPWebPresentationWizard::~KPWebPresentationWizard()
{
    view->enableWebPres();
}

void KPWebPresentationWizard::createWebPresentation( const QString &_config, KPresenterDoc *_doc,
                                                     KPresenterView *_view )
{
    KPWebPresentationWizard *dlg = new KPWebPresentationWizard( _config, _doc, _view );

    dlg->setCaption( i18n( "Create HTML Slideshow Wizard" ) );
    dlg->show();
}

void KPWebPresentationWizard::setupPage1()
{
    page1 = new QHBox( this );
    page1->setSpacing( KDialog::spacingHint() );
    page1->setMargin( KDialog::marginHint() );

    QLabel* sidebar = new QLabel( page1 );
    sidebar->setMinimumSize( 106, 318 );
    sidebar->setMaximumSize( 106, 318 );
    sidebar->setFrameShape( QFrame::Panel );
    sidebar->setFrameShadow( QFrame::Sunken );
    sidebar->setPixmap(locate("data", "kpresenter/pics/webslideshow-sidebar.png"));

    QWidget* canvas = new QWidget( page1 );
    QGridLayout *layout = new QGridLayout( canvas, 7, 2,
                                           KDialog::marginHint(), KDialog::spacingHint() );

    QLabel *helptext = new QLabel( canvas );
    helptext->setAlignment( Qt::WordBreak | Qt::AlignTop| Qt::AlignLeft );
    helptext->setText( i18n( "Enter your name, email address and "
                             "the title of the web presentation. "
                             "Also enter the output directory where the "
                             "web presentation should be saved. " ) );
    layout->addMultiCellWidget( helptext, 0, 0, 0, 1 );

    layout->addMultiCell( new QSpacerItem( 1, 50 ), 1, 1, 0, 1 );

    QLabel *label1 = new QLabel( i18n("Author:"), canvas );
    label1->setAlignment( Qt::AlignVCenter | Qt::AlignRight );
    layout->addWidget( label1, 2, 0 );

    QLabel *label2 = new QLabel( i18n("Title:"), canvas );
    label2->setAlignment( Qt::AlignVCenter | Qt::AlignRight );
    layout->addWidget( label2, 3, 0 );

    QLabel *label3 = new QLabel( i18n("E-mail address:"), canvas );
    label3->setAlignment( Qt::AlignVCenter | Qt::AlignRight );
    layout->addWidget( label3, 4, 0 );

    QLabel *label4 = new QLabel( i18n("Path:"), canvas );
    label4->setAlignment( Qt::AlignVCenter | Qt::AlignRight );
    layout->addWidget( label4, 5, 0 );

    author = new KLineEdit( webPres.getAuthor(), canvas );
    layout->addWidget( author, 2, 1 );

    title = new KLineEdit( webPres.getTitle(), canvas );
    layout->addWidget( title, 3, 1 );

    email = new KLineEdit( webPres.getEmail(), canvas );
    layout->addWidget( email, 4, 1 );

    path=new KURLRequester( canvas );
    path->setMode( KFile::Directory);
    path->lineEdit()->setText(webPres.getPath());
    layout->addWidget( path, 5, 1 );

    QSpacerItem* spacer = new QSpacerItem( 1, 10,
                                           QSizePolicy::Minimum, QSizePolicy::Expanding );
    layout->addMultiCell( spacer, 6, 6, 0, 1 );

    connect(path, SIGNAL(textChanged(const QString&)),
            this,SLOT(slotChoosePath(const QString&)));
    connect(path, SIGNAL(urlSelected( const QString& )),
            this,SLOT(slotChoosePath(const QString&)));

    addPage( page1, i18n( "Step 1: General Information" ) );

    setHelpEnabled(page1, false);  //doesn't do anything currently
}

void KPWebPresentationWizard::setupPage2()
{
    page2 = new QHBox( this );
    page2->setSpacing( KDialog::spacingHint() );
    page2->setMargin( KDialog::marginHint() );

    QLabel* sidebar = new QLabel( page2 );
    sidebar->setMinimumSize( 106, 318 );
    sidebar->setMaximumSize( 106, 318 );
    sidebar->setFrameShape( QFrame::Panel );
    sidebar->setFrameShadow( QFrame::Sunken );
    sidebar->setPixmap(locate("data", "kpresenter/pics/webslideshow-sidebar.png"));

    QWidget* canvas = new QWidget( page2 );
    QGridLayout *layout = new QGridLayout( canvas, 6, 2,
                                           KDialog::marginHint(), KDialog::spacingHint() );

    QLabel *helptext = new QLabel( canvas );
    helptext->setAlignment( Qt::WordBreak | Qt::AlignVCenter| Qt::AlignLeft );
    QString help = i18n("Here you can configure the style of the web pages.");
    help += i18n( "You can also specify the zoom for the slides." );
    helptext->setText(help);

    layout->addMultiCellWidget( helptext, 0, 0, 0, 1 );

    layout->addMultiCell( new QSpacerItem( 1, 50 ), 1, 1, 0, 1 );

    QLabel *label1 = new QLabel( i18n("Zoom:"), canvas );
    label1->setAlignment( Qt::AlignVCenter | Qt::AlignRight );
    layout->addWidget( label1, 2, 0 );

    QLabel *label2 = new QLabel( i18n( "Default encoding:" ), canvas );
    label2->setAlignment( Qt::AlignVCenter | Qt::AlignRight );
    layout->addWidget( label2, 3, 0 );

    QLabel *label3 = new QLabel( i18n( "Document type:" ), canvas );
    label3->setAlignment( Qt::AlignVCenter | Qt::AlignRight );
    layout->addWidget( label3, 4, 0 );

    zoom = new KIntNumInput( webPres.getZoom(), canvas );
    layout->addWidget( zoom, 2, 1 );
    zoom->setSuffix( " %" );
    zoom->setRange( 25, 1000, 5 );

    encoding = new KComboBox( false, canvas );
    layout->addWidget( encoding, 3, 1 );
    QStringList _strList = KGlobal::charsets()->availableEncodingNames();
    encoding->insertStringList( _strList );
    QString _name = webPres.getEncoding();
    encoding->setCurrentItem( _strList.findIndex( _name.lower() ) );

    doctype = new KComboBox( false, canvas );
    layout->addWidget( doctype, 4, 1 );
    doctype->insertItem( "HTML 4.01", -1 );
    doctype->insertItem( "XHTML 1.0", -1 );

    QSpacerItem* spacer = new QSpacerItem( 1, 10,
                                           QSizePolicy::Minimum, QSizePolicy::Expanding );
    layout->addMultiCell( spacer, 5, 5, 0, 1 );

    addPage( page2, i18n( "Step 2: Configure HTML" ) );

    setHelpEnabled(page2, false);  //doesn't do anything currently
}

void KPWebPresentationWizard::setupPage3()
{
    page3 = new QHBox( this );
    page3->setSpacing( KDialog::spacingHint() );
    page3->setMargin( KDialog::marginHint() );

    QLabel* sidebar = new QLabel( page3 );
    sidebar->setMinimumSize( 106, 318 );
    sidebar->setMaximumSize( 106, 318 );
    sidebar->setFrameShape( QFrame::Panel );
    sidebar->setFrameShadow( QFrame::Sunken );
    sidebar->setPixmap(locate("data", "kpresenter/pics/webslideshow-sidebar.png"));

    QWidget* canvas = new QWidget( page3 );
    QGridLayout *layout = new QGridLayout( canvas, 6, 2,
                                           KDialog::marginHint(), KDialog::spacingHint() );

    QLabel *helptext = new QLabel( canvas );
    helptext->setAlignment( Qt::WordBreak | Qt::AlignVCenter| Qt::AlignLeft );
    helptext->setText( i18n( "Now you can customize the colors of the web pages." ) );
    layout->addMultiCellWidget( helptext, 0, 0, 0, 1 );

    layout->addMultiCell( new QSpacerItem( 1, 50 ), 1, 1, 0, 1 );

    QLabel *label1 = new QLabel( i18n("Text color:"), canvas );
    label1->setAlignment( Qt::AlignVCenter | Qt::AlignRight );
    layout->addWidget( label1, 2, 0 );

    QLabel *label2 = new QLabel( i18n("Title color:"), canvas );
    label2->setAlignment( Qt::AlignVCenter | Qt::AlignRight );
    layout->addWidget( label2, 3, 0 );

    QLabel *label3 = new QLabel( i18n("Background color:"), canvas );
    label3->setAlignment( Qt::AlignVCenter | Qt::AlignRight );
    layout->addWidget( label3, 4, 0 );

    textColor = new KColorButton( webPres.getTextColor(), canvas );
    layout->addWidget( textColor, 2, 1 );

    titleColor = new KColorButton( webPres.getTitleColor(), canvas );
    layout->addWidget( titleColor, 3, 1 );

    backColor = new KColorButton( webPres.getBackColor(), canvas );
    layout->addWidget( backColor, 4, 1 );

    QSpacerItem* spacer = new QSpacerItem( 1, 10,
                                           QSizePolicy::Minimum, QSizePolicy::Expanding );
    layout->addMultiCell( spacer, 5, 5, 0, 1 );

    addPage( page3, i18n( "Step 3: Customize Colors" ) );

    setHelpEnabled(page3, false);  //doesn't do anything currently
}

void KPWebPresentationWizard::setupPage4()
{
    page4 = new QHBox( this );
    page4->setSpacing( KDialog::spacingHint() );
    page4->setMargin( KDialog::marginHint() );

    QLabel* sidebar = new QLabel( page4 );
    sidebar->setMinimumSize( 106, 318 );
    sidebar->setMaximumSize( 106, 318 );
    sidebar->setFrameShape( QFrame::Panel );
    sidebar->setFrameShadow( QFrame::Sunken );
    sidebar->setPixmap(locate("data", "kpresenter/pics/webslideshow-sidebar.png"));

    QWidget* canvas = new QWidget( page4 );
    QGridLayout *layout = new QGridLayout( canvas, 3, 2,
                                           KDialog::marginHint(), KDialog::spacingHint() );

    QLabel *helptext = new QLabel( canvas );
    helptext->setAlignment( Qt::WordBreak | Qt::AlignVCenter| Qt::AlignLeft );
    helptext->setText( i18n( "Here you can specify titles for "
                             "each slide. Click on a slide in "
                             "the list and then enter the title "
                             "in the textbox below. If you "
                             "click on a title, KPresenter "
                             "mainview will display the slide.") );

    layout->addMultiCellWidget( helptext, 0, 0, 0, 1 );

    QLabel *label = new QLabel( i18n( "Slide title:" ), canvas );
    label->setAlignment( Qt::AlignVCenter | Qt::AlignRight );
    layout->addWidget( label, 1, 0 );

    slideTitle = new KLineEdit( canvas );
    layout->addWidget( slideTitle, 1, 1 );
    connect( slideTitle, SIGNAL( textChanged( const QString & ) ), this,
             SLOT( slideTitleChanged( const QString & ) ) );

    slideTitles = new KListView( canvas );
    layout->addMultiCellWidget( slideTitles, 2, 2, 0, 1 );
    slideTitles->addColumn( i18n( "Slide No." ) );
    slideTitles->addColumn( i18n( "Slide Title" ) );
    connect( slideTitles, SIGNAL( selectionChanged( QListViewItem * ) ), this,
             SLOT( slideTitleChanged( QListViewItem * ) ) );
    slideTitles->setSorting( -1 );
    slideTitles->setAllColumnsShowFocus( true );
    slideTitles->setResizeMode( QListView::LastColumn );
    slideTitles->header()->setMovingEnabled( false );

    QValueList<KPWebPresentation::SlideInfo> infos = webPres.getSlideInfos();
    for ( int i = infos.count() - 1; i >= 0; --i ) {
        KListViewItem *item = new KListViewItem( slideTitles );
        item->setText( 0, QString::number( i + 1 ) );
        //kdDebug(33001) << "KPWebPresentationWizard::setupPage3 " << infos[ i ].slideTitle << endl;
        item->setText( 1, infos[ i ].slideTitle );
    }

    slideTitles->setSelected( slideTitles->firstChild(), true );

    addPage( page4, i18n( "Step 4: Customize Slide Titles" ) );

    setHelpEnabled(page4, false);  //doesn't do anything currently

    setFinish( page4, true );
}


void KPWebPresentationWizard::finish()
{
    webPres.setAuthor( author->text() );
    webPres.setEMail( email->text() );
    webPres.setTitle( title->text() );

    QListViewItemIterator it( slideTitles );
    for ( ; it.current(); ++it )
        webPres.setSlideTitle( it.current()->text( 0 ).toInt() - 1, it.current()->text( 1 ) );

    webPres.setBackColor( backColor->color() );
    webPres.setTitleColor( titleColor->color() );
    webPres.setTextColor( textColor->color() );
    webPres.setPath( path->lineEdit()->text() );
    webPres.setZoom( zoom->value() );
    webPres.setXML( doctype->currentItem() != 0 );
    webPres.setEncoding( encoding->currentText() );

    close();
    KPWebPresentationCreateDialog::createWebPresentation( doc, view, webPres );
}

void KPWebPresentationWizard::pageChanged()
{
    if ( currentPage() != page4 )
    {
        QString pathname = path->lineEdit()->text();
        QFileInfo fi( pathname );

        // path doesn't exist. ask user if it should be created.
        if ( !fi.exists() )
        {
            QString msg = i18n( "<qt>The directory <b>%1</b> does not exist.<br>"
                                "Do you want create it?</qt>" );
            if( KMessageBox::questionYesNo( this, msg.arg( pathname ),
                                            i18n( "Directory Not Found" ) )
                == KMessageBox::Yes)
            {
                QDir dir;
                bool ok = dir.mkdir( pathname );
                if( !ok )
                {
                    KMessageBox::sorry( this,
                                        i18n( "Can't create directory!" ) );
                    // go back to first step
                    showPage( page1 );
                    path->setFocus();
                }

            }
            else
            {
                // go back to first step
                showPage( page1 );
                path->setFocus();
            }
        }

        // path exists but it's not a valid directory. warn the user.
        else if ( !fi.isDir() )
        {
            KMessageBox::error( this,
                                i18n( "The path you entered is not a valid directory!\n"
                                      "Please correct this." ),
                                i18n( "Invalid Path" ) );

            // go back to first step
            showPage( page1 );
            path->setFocus();
        }
    } else
        finishButton()->setEnabled( true );
}

void KPWebPresentationWizard::slideTitleChanged( const QString &s )
{
    if ( slideTitles->currentItem() )
        slideTitles->currentItem()->setText( 1, s );
}

void KPWebPresentationWizard::slideTitleChanged( QListViewItem *i )
{
    if ( !i ) return;

    slideTitle->setText( i->text( 1 ) );
    view->skipToPage( i->text( 0 ).toInt() - 1 );
}

void KPWebPresentationWizard::closeEvent( QCloseEvent *e )
{
    view->enableWebPres();
    KWizard::closeEvent( e );
}

void KPWebPresentationWizard::slotChoosePath(const QString &text)
{
    webPres.setPath(text);
}

KPWebPresentationCreateDialog::KPWebPresentationCreateDialog( KPresenterDoc *_doc, KPresenterView *_view,
                                                              const KPWebPresentation &_webPres )
    : QDialog( 0, "", false ), webPres( _webPres )
{
    doc = _doc;
    view = _view;

    setupGUI();
}

KPWebPresentationCreateDialog::~KPWebPresentationCreateDialog()
{
    view->enableWebPres();
}

void KPWebPresentationCreateDialog::createWebPresentation( KPresenterDoc *_doc, KPresenterView *_view,
                                                           const KPWebPresentation &_webPres )
{
    KPWebPresentationCreateDialog *dlg = new KPWebPresentationCreateDialog( _doc, _view, _webPres );

    dlg->setCaption( i18n( "Create HTML Slideshow" ) );
    dlg->resize( 400, 300 );
    dlg->show();
    dlg->start();
}

void KPWebPresentationCreateDialog::start()
{
    setCursor( waitCursor );
    initCreation();
    createSlidesPictures();
    createSlidesHTML();
    createMainPage();
    setCursor( arrowCursor );

    bDone->setEnabled( true );
    bSave->setEnabled( true );
}

void KPWebPresentationCreateDialog::initCreation()
{
    QFont f = step1->font(), f2 = step1->font();
    f.setBold( true );
    step1->setFont( f );

    progressBar->setProgress( 0 );
    progressBar->setTotalSteps( webPres.initSteps() );

    webPres.initCreation( progressBar );

    step1->setFont( f2 );
    progressBar->setProgress( progressBar->totalSteps() );
}

void KPWebPresentationCreateDialog::createSlidesPictures()
{
    QFont f = step2->font(), f2 = f;
    f.setBold( true );
    step2->setFont( f );

    progressBar->setProgress( 0 );
    if ( webPres.slides1Steps() > 0 )
    {
        progressBar->setTotalSteps( webPres.slides1Steps() );
        webPres.createSlidesPictures( progressBar );
    }

    step2->setFont( f2 );
    progressBar->setProgress( progressBar->totalSteps() );
}

void KPWebPresentationCreateDialog::createSlidesHTML()
{
    QFont f = step3->font(), f2 = step3->font();
    f.setBold( true );
    step3->setFont( f );

    progressBar->setProgress( 0 );
    if ( webPres.slides1Steps() > 0 )
    {
        progressBar->setTotalSteps( webPres.slides1Steps() );
        webPres.createSlidesHTML( progressBar );
    }

    step3->setFont( f2 );
    progressBar->setProgress( progressBar->totalSteps() );
}

void KPWebPresentationCreateDialog::createMainPage()
{
    QFont f = step4->font(), f2 = step4->font();
    f.setBold( true );
    step4->setFont( f );

    progressBar->setProgress( 0 );
    progressBar->setTotalSteps( webPres.slides1Steps() );

    webPres.createMainPage( progressBar );

    step4->setFont( f2 );
    progressBar->setProgress( progressBar->totalSteps() );
}

void KPWebPresentationCreateDialog::setupGUI()
{
    back = new QVBox( this );
    back->setMargin( KDialog::marginHint() );

    QFrame *line;

    line = new QFrame( back );
    line->setFrameStyle( QFrame::HLine | QFrame::Sunken );
    line->setMaximumHeight( 20 );

    step1 = new QLabel( i18n( "Initialize (create file structure, etc.)" ), back );
    step2 = new QLabel( i18n( "Create Pictures of the Slides" ), back );
    step3 = new QLabel( i18n( "Create HTML Pages for the Slides" ), back );
    step4 = new QLabel( i18n( "Create Main Page (Table of Contents)" ), back );

    line = new QFrame( back );
    line->setFrameStyle( QFrame::HLine | QFrame::Sunken );
    line->setMaximumHeight( 20 );

    progressBar = new KProgress( back );

    line = new QFrame( back );
    line->setFrameStyle( QFrame::HLine | QFrame::Sunken );
    line->setMaximumHeight( 20 );

    KButtonBox *bb = new KButtonBox( back );
    bSave = bb->addButton( i18n( "Save Configuration..." ) );
    bb->addStretch();
    bDone = bb->addButton( i18n( "Done" ) );

    bSave->setEnabled( false );
    bDone->setEnabled( false );

    connect( bDone, SIGNAL( clicked() ), this, SLOT( accept() ) );
    connect( bSave, SIGNAL( clicked() ), this, SLOT( saveConfig() ) );
}

void KPWebPresentationCreateDialog::resizeEvent( QResizeEvent *e )
{
    QDialog::resizeEvent( e );
    back->resize( size() );
}

void KPWebPresentationCreateDialog::saveConfig()
{
    QString filename = webPres.getConfig();
    if ( QFileInfo( filename ).exists() )
        filename = QFileInfo( filename ).absFilePath();
    else
        filename = QString::null;

    KURL url = KFileDialog::getOpenURL( filename, i18n("*.kpweb|KPresenter Web-Presentation (*.kpweb)") );

    if( url.isEmpty() )
        return;

    // ### TODO: use KIO::NetAccess for remote files (floppy: is remote!)
    if( !url.isLocalFile() )
    {
        KMessageBox::sorry( 0L, i18n( "Only local files are currently supported." ) );
        return;
    }

    filename = url.path();

    if ( !filename.isEmpty() ) {
        if (filename.endsWith(".kpweb"))
            webPres.setConfig( filename );
        else
            webPres.setConfig( filename + ".kpweb" );
        webPres.saveConfig();
    }
}

#include "webpresentation.moc"
