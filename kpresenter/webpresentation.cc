/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Reginald Stadlbauer <reggie@kde.org>

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

#include <webpresentation.h>

#include <kpresenter_view.h>
#include "kprcanvas.h"

#include <kstandarddirs.h>
#include <kprocess.h>
#include <unistd.h>
#include <pwd.h>
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

#include <kdebug.h>
#include <klocale.h>
#include <kcolorbutton.h>
#include <kfiledialog.h>
#include <kmessagebox.h>
#include <kbuttonbox.h>
#include <ksimpleconfig.h>
#include <kimageio.h>
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

#include "koDocumentInfo.h"

/******************************************************************/
/* Class: KPWebPresentation                                       */
/******************************************************************/

/*================================================================*/
KPWebPresentation::KPWebPresentation( KPresenterDoc *_doc, KPresenterView *_view )
    : config( QString::null )
{
    doc = _doc;
    view = _view;
    init();
}

/*================================================================*/
KPWebPresentation::KPWebPresentation( const QString &_config, KPresenterDoc *_doc, KPresenterView *_view )
    : config( _config )
{
    doc = _doc;
    view = _view;
    init();
    loadConfig();
}

/*================================================================*/
KPWebPresentation::KPWebPresentation( const KPWebPresentation &webPres )
    : config( webPres.config ), author( webPres.author ), title( webPres.title ), email( webPres.email ),
      slideInfos( webPres.slideInfos ), backColor( webPres.backColor ), titleColor( webPres.titleColor ),
      textColor( webPres.textColor ), path( webPres.path ), imgFormat( webPres.imgFormat ), zoom( webPres.zoom ),
      m_encoding( webPres.m_encoding )
{
    doc = webPres.doc;
    view = webPres.view;
}

/*================================================================*/
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
    path = cfg.readEntry( "Path", path );
    imgFormat = static_cast<ImageFormat>( cfg.readNumEntry( "ImageFormat", static_cast<int>( imgFormat ) ) );
    zoom = cfg.readNumEntry( "Zoom", zoom );
    m_encoding = cfg.readEntry( "Encoding", m_encoding );
}

/*================================================================*/
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
    cfg.writeEntry( "Path", path );
    cfg.writeEntry( "ImageFormat", static_cast<int>( imgFormat ) );
    cfg.writeEntry( "Zoom", zoom );
    cfg.writeEntry( "Encoding", m_encoding );
}

/*================================================================*/
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

    QString format = "." + imageFormat( imgFormat );

    const char *pics[] = { "home", "first", "next", "prev", "last", 0 };
    uint index = 0;

    QString filename;
    QString fullpath;

    while ( pics[ index ] ) {
        filename = pics[index] + format;
        fullpath = path + "/pics/" + filename;
        QString cmd("cp ");
        cmd += KShellProcess::quote(locate( "slideshow", filename, KPresenterFactory::global() ));
        cmd += " ";
        cmd += KShellProcess::quote(fullpath);
        system( QFile::encodeName( cmd ) );
        p = progressBar->progress();
        progressBar->setProgress( ++p );
        kapp->processEvents();
        index++;
    }
}

/*================================================================*/
void KPWebPresentation::createSlidesPictures( KProgress *progressBar )
{
    if ( slideInfos.isEmpty() )
        return;
    QPixmap pix( 10, 10 );
    QString filename;
    QString format = imageFormat( imgFormat );
    int p;
    for ( unsigned int i = 0; i < slideInfos.count(); i++ ) {
        int pgNum = slideInfos[i].pageNumber;
        view->getCanvas()->drawPageInPix( pix, pgNum, zoom, true /*force real variable value*/ );
        filename = QString( "%1/pics/slide_%2.%3" ).arg( path ).arg( i + 1 ).arg( format );

        pix.save( filename, format.upper().latin1() );   //lukas: provide the option to choose image quality

        p = progressBar->progress();
        progressBar->setProgress( ++p );
        kapp->processEvents();
    }
}

/*================================================================*/
void KPWebPresentation::createSlidesHTML( KProgress *progressBar )
{
    unsigned int pgNum;
    int p;
    QString format = imageFormat( imgFormat );

    QTextCodec *codec = KGlobal::charsets()->codecForName( m_encoding );
    QString chsetName = codec->mimeName();

    QString html;
    for ( unsigned int i = 0; i < slideInfos.count(); i++ ) {
        pgNum = i + 1;
        html = QString( "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0 Transitional//EN\">" );
        html += QString( "<HTML><HEAD><TITLE>%1 - %2</TITLE>\n" ).arg( title ).arg( slideInfos[ i ].slideTitle );

        html += QString( "<META HTTP-Equiv=\"Content-Type\" CONTENT=\"text/html; charset=%1\">\n" )
            .arg( chsetName );

        html += QString( "</HEAD>\n" );
        html += QString( "<BODY bgcolor=\"%1\" text=\"%2\">\n" ).arg( backColor.name() ).arg( textColor.name() );

        html += QString( "  <CENTER>\n" );
        if ( i > 0 )
            html += QString( "    <A HREF=\"slide_1.html\">" );
        html += QString( "<IMG src=\"../pics/first.%1\" border=\"0\">" ).arg( format );
        if ( i > 0 )
            html += "</A>\n";
        else
            html += "\n";

        if ( i > 0 )
            html += QString( "    <A HREF=\"slide_%1.html\">" ).arg( pgNum - 1 );
        html += QString( "<IMG src=\"../pics/prev.%1\" border=\"0\">" ).arg( format );
        if ( i > 0 )
            html += "</A>\n";
        else
            html += "\n";

        if ( i < slideInfos.count() - 1 )
            html += QString( "    <A HREF=\"slide_%1.html\">" ).arg( pgNum + 1 );
        html += QString( "<IMG src=\"../pics/next.%1\" border=\"0\">" ).arg( format );
        if ( i < slideInfos.count() - 1 )
            html += "</A>\n";
        else
            html += "\n";

        if ( i < slideInfos.count() - 1 )
            html += QString( "    <A HREF=\"slide_%1.html\">" ).arg( slideInfos.count() );
        html += QString( "<IMG src=\"../pics/last.%1\" border=\"0\">" ).arg( format );
        if ( i < slideInfos.count() - 1 )
            html += "</A>\n";
        else
            html += "\n";

        html += "    &nbsp; &nbsp; &nbsp; &nbsp; \n";

        html += "    <A HREF=\"../index.html\">";
        html += QString( "<IMG src=\"../pics/home.%1\" border=\"0\">" ).arg( format );
        html += "</A>\n";

        html += "  </CENTER><BR><HR noshade>\n";

        html += QString( "  <FONT color=\"%1\">\n" ).arg( titleColor.name() );
        html += QString( "  <CENTER><B>%1</B> - <I>%2</I></CENTER>\n" ).arg( title ).arg( slideInfos[ i ].slideTitle );

        html += "  </FONT><HR noshade><BR>\n";

        html += "  <CENTER>\n";

        html += "    ";
        if ( i < slideInfos.count() - 1 )
            html += QString( "<A HREF=\"slide_%1.html\">" ).arg( pgNum + 1 );
        html += QString( "<IMG src=\"../pics/slide_%1.%2\" BORDER=0>" ).arg( pgNum ).arg( format );
        if ( i < slideInfos.count() - 1 )
            html += "</A>\n";
        else
            html += "\n";

        html += "  </CENTER><BR><HR noshade>\n";

        QPtrList<KPrPage> _tmpList( doc->getPageList() );
        QString note = _tmpList.at(i)->noteText();
        if ( !note.isEmpty() ) {
            html += QString( "  <B>%1</B>\n" ).arg( i18n( "Note" ) );
            html += "  <BLOCKQUOTE>\n";
            html += "  <P>\n";

            note.replace( QRegExp( "\n" ), QString::fromLatin1( "<BR>\n" ) );
            html += note;

            html += "  </P>\n";
            html += "  </BLOCKQUOTE><HR NOSHADE>\n";
        }

        html += "  <CENTER>\n";
        html += "    <B>"+ i18n("Author:") + " </B>";
        if ( !email.isEmpty() )
            html += QString( "<A HREF=\"mailto:%1\">" ).arg( email );
        html += QString( "<I>%1</I>" ).arg( author );
        if ( !email.isEmpty() )
            html += "</A>";

        html += i18n(" - created with %1").arg("<A HREF=\"http://www.koffice.org/kpresenter/\">KPresenter</A>");
        html += "  </CENTER><HR noshade>\n";
        html += "</BODY></HTML>\n";

        QFile file( QString( "%1/html/slide_%2.html" ).arg( path ).arg( pgNum ) );
        file.open( IO_WriteOnly );
        QTextStream t( &file );
        t.setCodec( codec );
        t << html;
        file.close();

        p = progressBar->progress();
        progressBar->setProgress( ++p );
        kapp->processEvents();
    }
}

/*================================================================*/
void KPWebPresentation::createMainPage( KProgress *progressBar )
{
    QString html;

    QTextCodec *codec = KGlobal::charsets()->codecForName( m_encoding );
    QString chsetName = codec->mimeName();

    html = QString( "<HTML><HEAD><TITLE>%1 - ").arg( title );
    html += i18n("Table of Contents");
    html += "</TITLE>\n";
    html += QString( "<META HTTP-Equiv=\"Content-Type\" CONTENT=\"text/html; charset=%1\">\n" )
            .arg( chsetName );
    html += "</HEAD>\n";

    html += QString( "<BODY bgcolor=\"%1\" text=\"%2\">\n" ).arg( backColor.name() ).arg( textColor.name() );

    html += QString( "<FONT color=\"%1\">\n" ).arg( titleColor.name() );
    html += QString( "<BR><CENTER><H1>%1</H1></CENTER>\n" ).arg( title );
    html += "</FONT>\n";

    html += "<BR><BR><CENTER><H3><A HREF=\"html/slide_1.html\">";
    html += i18n("Click here to start the Slideshow");
    html += "</A></H3></CENTER><BR>\n";

    html += "<HR noshade><BR><BR>\n";

    if ( email.isEmpty() )
        html += i18n( "Created on %1 by <I>%2</I>" ).
	  arg( KGlobal::locale()->formatDate ( QDate::currentDate() ) ).
	  arg( author );
    else
        html += i18n( "Created on %1 by <I><A HREF=\"mailto:%2\">%3</A></I>" ).
	  arg( KGlobal::locale()->formatDate ( QDate::currentDate() ) ).
	  arg( email ).
	  arg( author );

    html += "<BR><BR>\n<B>" + i18n("Table of Contents") + "</B><BR>\n";
    html += "<OL>\n";

    for ( unsigned int i = 0; i < slideInfos.count(); i++ )
        html += QString( "  <LI><A HREF=\"html/slide_%1.html\">%2</A><BR>\n" ).arg( i + 1 ).arg( slideInfos[ i ].slideTitle );

    html += "</OL></BODY></HTML>\n";

    QFile file( QString( "%1/index.html" ).arg( path ) );
    file.open( IO_WriteOnly );
    QTextStream t( &file );
    t.setCodec( codec );
    t << html;
    file.close();

    progressBar->setProgress( progressBar->totalSteps() );
    kapp->processEvents();
}

/*================================================================*/
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
    if (KImageIO::canWrite("JPEG"))
        imgFormat = JPEG;
    else
        imgFormat = PNG;

    path = KGlobalSettings::documentPath() + "www";

    zoom = 100;
    m_encoding = QTextCodec::codecForLocale()->name();
}

/******************************************************************/
/* Class: KPWebPresentationWizard                                 */
/******************************************************************/

/*================================================================*/
KPWebPresentationWizard::KPWebPresentationWizard( const QString &_config, KPresenterDoc *_doc,
                                                  KPresenterView *_view )
    : QWizard( 0, "", false ), config( _config ), webPres( config, _doc, _view )
{
    doc = _doc;
    view = _view;

    setupPage1();
    setupPage2();
    setupPage3();

    connect( nextButton(), SIGNAL( clicked() ), this, SLOT( pageChanged() ) );
    connect( backButton(), SIGNAL( clicked() ), this, SLOT( pageChanged() ) );
    connect( finishButton(), SIGNAL( clicked() ), this, SLOT( finish() ) );
}

/*================================================================*/
KPWebPresentationWizard::~KPWebPresentationWizard()
{
    view->enableWebPres();
}

/*================================================================*/
void KPWebPresentationWizard::createWebPresentation( const QString &_config, KPresenterDoc *_doc,
                                                     KPresenterView *_view )
{
    KPWebPresentationWizard *dlg = new KPWebPresentationWizard( _config, _doc, _view );

    dlg->setCaption( i18n( "Create HTML Slideshow" ) );
    dlg->resize( 640, 350 );
    dlg->show();
}

/*================================================================*/
void KPWebPresentationWizard::setupPage1()
{
    page1 = new QHBox( this );
    page1->setSpacing( 5 );
    page1->setMargin( 5 );

    QLabel *helptext = new QLabel( page1 );
    helptext->setMargin( 5 );
    helptext->setBackgroundMode( PaletteLight );
    helptext->setText( i18n( "Enter your name, email address and\n"
                             "the title of the web presentation.\n"
                             "Also enter the path into which the\n"
                             "web presentation should be created\n"
                             "(This must be a directory).\n" ) );
    helptext->setMaximumWidth( helptext->sizeHint().width() );

    QVBox *canvas = new QVBox( page1 );

    QHBox *row1 = new QHBox( canvas );
    QHBox *row2 = new QHBox( canvas );
    QHBox *row3 = new QHBox( canvas );
    QHBox *row4 = new QHBox( canvas );

    QLabel *label1 = new QLabel( i18n("Author:"), row1 );
    label1->setAlignment( Qt::AlignVCenter );
    QLabel *label2 = new QLabel( i18n("Title:"), row2 );
    label2->setAlignment( Qt::AlignVCenter );
    QLabel *label3 = new QLabel( i18n("Email address:"), row3 );
    label3->setAlignment( Qt::AlignVCenter );
    QLabel *label4 = new QLabel( i18n("Path:"), row4 );
    label4->setAlignment( Qt::AlignVCenter );

    author = new KLineEdit( webPres.getAuthor(), row1 );
    title = new KLineEdit( webPres.getTitle(), row2 );
    email = new KLineEdit( webPres.getEmail(), row3 );

    path=new KURLRequester( row4 );
    path->setMode( KFile::Directory);
    path->lineEdit()->setText(webPres.getPath());
    connect(path, SIGNAL(textChanged(const QString&)),
           this,SLOT(slotChoosePath(const QString&)));
    connect(path, SIGNAL(urlSelected( const QString& )),
           this,SLOT(slotChoosePath(const QString&)));

    addPage( page1, i18n( "General Information" ) );

    setHelpEnabled(page1, false);  //doesn't do anything currently

}

/*================================================================*/
void KPWebPresentationWizard::setupPage2()
{
    page2 = new QHBox( this );
    page2->setSpacing( 6 );
    page2->setMargin( 6 );

    QLabel *helptext = new QLabel( page2 );
    helptext->setMargin( 6 );
    helptext->setBackgroundMode( PaletteLight );
    QString help = i18n("Here you can configure the style\n"
                        "of the web pages (colors). You also\n"
                        "need to specify the picture format\n"
                        "which should be used for the slides.\n"
                        "PNG is a very optimized and well\n"
                        "compressed format, but may not be\n"
                        "supported by some old web browsers.\n"
                        "BMP is a picture format with a bad\n"
                        "compression, but is supported by\n"
                        "old web browsers.\n");

    if ( KImageIO::canWrite( "JPEG" ) )
        help += i18n("JPEG is a picture format with quite a good\n"
                     "compression and which is supported by\n"
                     "all web browsers.\n");

    help += i18n( "\n"
                  "Finally you can also specify the zoom\n"
                  "for the slides." );
    helptext->setText(help);
    helptext->setMaximumWidth( helptext->sizeHint().width() );

    QVBox *canvas = new QVBox( page2 );

    QHBox *row1 = new QHBox( canvas );
    QHBox *row2 = new QHBox( canvas );
    QHBox *row3 = new QHBox( canvas );
    QHBox *row4 = new QHBox( canvas );
    QHBox *row5 = new QHBox( canvas );
    QHBox *row6 = new QHBox( canvas );

    QLabel *label1 = new QLabel( i18n("Text color:"), row1 );
    label1->setAlignment( Qt::AlignVCenter );
    QLabel *label2 = new QLabel( i18n("Title color:"), row2 );
    label2->setAlignment( Qt::AlignVCenter );
    QLabel *label3 = new QLabel( i18n("Background color:"), row3 );
    label3->setAlignment( Qt::AlignVCenter );
    QLabel *label4 = new QLabel( i18n("Picture format:"), row4 );
    label4->setAlignment( Qt::AlignVCenter );
    QLabel *label5 = new QLabel( i18n("Zoom:"), row5 );
    label5->setAlignment( Qt::AlignVCenter );
    QLabel *label6 = new QLabel( i18n( "Default encoding:" ), row6 );
    label6->setAlignment( Qt::AlignVCenter );

    textColor = new KColorButton( webPres.getTextColor(), row1 );
    titleColor = new KColorButton( webPres.getTitleColor(), row2 );
    backColor = new KColorButton( webPres.getBackColor(), row3 );
    format = new KComboBox( false, row4 );
    format->insertItem( "BMP", -1 );
    format->insertItem( "PNG", -1 );
    if ( KImageIO::canWrite( "JPEG" ) )
        format->insertItem( "JPEG", -1 );
    format->setCurrentItem( static_cast<int>( webPres.getImageFormat() ) );

    zoom = new KIntNumInput( webPres.getZoom(), row5 );
    zoom->setSuffix( " %" );
    zoom->setRange( 1, 1000, 1 );

    encoding = new KComboBox( false, row6 );
    QStringList _strList = KGlobal::charsets()->availableEncodingNames();
    encoding->insertStringList( _strList );
    QString _name = webPres.getEncoding();
    encoding->setCurrentItem( _strList.findIndex( _name.lower() ) );


    addPage( page2, i18n( "Style" ) );

    setHelpEnabled(page2, false);  //doesn't do anything currently
}

/*================================================================*/
void KPWebPresentationWizard::setupPage3()
{
    page3 = new QHBox( this );
    page3->setSpacing( 5 );
    page3->setMargin( 5 );

    QLabel *helptext = new QLabel( page3 );
    helptext->setMargin( 5 );
    helptext->setBackgroundMode( PaletteLight );
    helptext->setText( i18n( "Here you can specify titles for\n"
                             "each slide. Click on a slide in\n"
                             "the list and then enter the title\n"
                             "in the textbox below. If you\n"
                             "click on a title, the KPresenter\n"
                             "mainview will scroll to this\n"
                             "slide, so it can be seen." ) );
    helptext->setMaximumWidth( helptext->sizeHint().width() );

    QVBox *canvas = new QVBox( page3 );

    QHBox *row = new QHBox( canvas );
    QLabel *label = new QLabel( i18n( "Slide title:" ), row );
    label->setAlignment( Qt::AlignVCenter );
    label->setMinimumWidth( label->sizeHint().width() );
    label->setMaximumWidth( label->sizeHint().width() );

    slideTitle = new KLineEdit( row );
    connect( slideTitle, SIGNAL( textChanged( const QString & ) ), this,
             SLOT( slideTitleChanged( const QString & ) ) );

    slideTitles = new KListView( canvas );
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

    addPage( page3, i18n( "Slide Titles" ) );

    setHelpEnabled(page3, false);  //doesn't do anything currently

    setFinish( page3, true );
}


/*================================================================*/
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
    webPres.setImageFormat( static_cast<KPWebPresentation::ImageFormat>( format->currentItem() ) );
    webPres.setPath( path->lineEdit()->text() );
    webPres.setZoom( zoom->value() );
    webPres.setEncoding( encoding->currentText() );

    close();
    KPWebPresentationCreateDialog::createWebPresentation( doc, view, webPres );
}


/*================================================================*/
bool KPWebPresentationWizard::isPathValid() const
{
    QFileInfo fi( path->lineEdit()->text() );

    if ( fi.exists() && fi.isDir() )
        return true;

    return false;
}

/*================================================================*/
void KPWebPresentationWizard::pageChanged()
{
    if ( currentPage() != page3 ) {
        if ( !isPathValid() ) {
            KMessageBox::error( this,
                                i18n( "The path you entered is not a valid directory!\n"
                                      "Please correct this." ),
                                i18n( "Invalid Path" ) );
            showPage( page1 );
            path->setFocus();
        }
    } else
        finishButton()->setEnabled( true );
}

/*================================================================*/
void KPWebPresentationWizard::slideTitleChanged( const QString &s )
{
    if ( slideTitles->currentItem() )
        slideTitles->currentItem()->setText( 1, s );
}

/*================================================================*/
void KPWebPresentationWizard::slideTitleChanged( QListViewItem *i )
{
    if ( !i ) return;

    slideTitle->setText( i->text( 1 ) );
    view->skipToPage( i->text( 0 ).toInt() - 1 );
}

/*================================================================*/
void KPWebPresentationWizard::closeEvent( QCloseEvent *e )
{
    view->enableWebPres();
    QWizard::closeEvent( e );
}

void KPWebPresentationWizard::slotChoosePath(const QString &text)
{
    webPres.setPath(text);
}

/******************************************************************/
/* Class: KPWebPresentationCreateDialog                           */
/******************************************************************/

/*================================================================*/
KPWebPresentationCreateDialog::KPWebPresentationCreateDialog( KPresenterDoc *_doc, KPresenterView *_view,
                                                              const KPWebPresentation &_webPres )
    : QDialog( 0, "", false ), webPres( _webPres )
{
    doc = _doc;
    view = _view;

    setupGUI();
}

/*================================================================*/
KPWebPresentationCreateDialog::~KPWebPresentationCreateDialog()
{
    view->enableWebPres();
}

/*================================================================*/
void KPWebPresentationCreateDialog::createWebPresentation( KPresenterDoc *_doc, KPresenterView *_view,
                                                           const KPWebPresentation &_webPres )
{
    KPWebPresentationCreateDialog *dlg = new KPWebPresentationCreateDialog( _doc, _view, _webPres );

    dlg->setCaption( i18n( "Create HTML Slideshow" ) );
    dlg->resize( 400, 300 );
    dlg->show();
    dlg->start();
}

/*================================================================*/
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

/*================================================================*/
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

/*================================================================*/
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

/*================================================================*/
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

/*================================================================*/
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

/*================================================================*/
void KPWebPresentationCreateDialog::setupGUI()
{
    back = new QVBox( this );
    back->setMargin( 10 );

    QFrame *line;

    line = new QFrame( back );
    line->setFrameStyle( QFrame::HLine | QFrame::Sunken );
    line->setMaximumHeight( 20 );

    step1 = new QLabel( i18n( "Initialize (create file structure, etc.)" ), back );
    step2 = new QLabel( i18n( "Create Pictures of the Slides" ), back );
    step3 = new QLabel( i18n( "Create HTML pages for the slides" ), back );
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

/*================================================================*/
void KPWebPresentationCreateDialog::resizeEvent( QResizeEvent *e )
{
    QDialog::resizeEvent( e );
    back->resize( size() );
}

/*================================================================*/
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

    if( !url.isLocalFile() )
    {
      KMessageBox::sorry( 0L, i18n( "Only local files are currently supported." ) );
      return;
    }

    filename = url.path();

    if ( !filename.isEmpty() ) {
        webPres.setConfig( filename );
        webPres.saveConfig();
    }
}

#include <webpresentation.moc>
