/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Reginald Stadlbauer <reggie@kde.org>
   2000, 2001 Werner Trobin <trobin@kde.org>
   2002, 2003 Thomas Nagy <tnagy@eleve.emn.fr>

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

// Description: Template Choose Dialog

/******************************************************************/

#include <qlayout.h>
#include <qtabwidget.h>
#include <qcombobox.h>
#include <qcheckbox.h>
#include <qpoint.h>
#include <qobjectlist.h>
#include <qvgroupbox.h>

#include <klocale.h>
#include <kdeversion.h>
#include <kfiledialog.h>
#include <kinstance.h>
#include <koFilterManager.h>
#include <koTemplates.h>
#include <koDocument.h>
#include <kmainwindow.h>

#include "koTemplateChooseDia.h"
#include <kdebug.h>

#include <kpushbutton.h>
#include <kjanuswidget.h>
#include <kglobalsettings.h>
#include <ktextedit.h>
#include <kfileiconview.h>
#include <kfileitem.h>
#include <kmessagebox.h>
#include <qapplication.h>
#include <qtooltip.h>
#include <kapplication.h>

class MyFileDialog : public KFileDialog
{
    public :
        MyFileDialog(
                const QString& startDir=0,
                const QString& filter =0,
                QWidget *parent=0,
                const char *name=0,
                bool modal=0)
            :  KFileDialog (startDir, filter, parent, name, modal) {}

        KURL currentURL()
        {
            setResult( QDialog::Accepted ); // selectedURL tests for it
            return KFileDialog::selectedURL();
        }

        // Return true if the current URL exists, show msg box if not
        bool checkURL()
        {
            bool ok = true;
            KURL url = currentURL();
            if ( url.isLocalFile() )
            {
                ok = QFile::exists( url.path() );
                if ( !ok ) {
                    // Maybe offer to create a new document with that name? (see alos KoDocument::openFile)
                    KMessageBox::error( this, i18n( "The file %1 doesn't exist." ).arg( url.path() ) );
                }
            }
            return ok;
        }
    protected:
    // Typing a file that doesn't exist closes the file dialog, we have to
    // handle this case better here.
        virtual void accept() {
            if ( checkURL() )
                KFileDialog::accept();
        }

        virtual void reject() {
		KFileDialog::reject();
		emit cancelClicked();
        }

};

/*================================================================*/

/*================================================================*/

class KoTemplateChooseDiaPrivate {
    public:
	KoTemplateChooseDiaPrivate(const QCString& templateType, KInstance* global,
		const QCString &format, const QString &nativePattern,
		const QString &nativeName,
		const KoTemplateChooseDia::DialogType &dialogType) :
	    m_templateType(templateType), m_global(global), m_format(format),
	m_nativePattern(nativePattern), m_nativeName(nativeName),
	m_dialogType(dialogType), tree(0L), m_mainwidget(0L)
	{
	    m_returnType = KoTemplateChooseDia::Empty;
	    m_nostartupdlg = false;
	    tree = 0;
	    m_nodiag = 0;
	}

	~KoTemplateChooseDiaPrivate() {}

	QCString m_templateType;
	KInstance* m_global;
	QCString m_format;
	QString m_nativePattern;
	QString m_nativeName;
	KoTemplateChooseDia::DialogType m_dialogType;
	KoTemplateTree *tree;

	QString m_templateName;
	QString m_fullTemplateName;
	KoTemplateChooseDia::ReturnType m_returnType;

	bool m_nostartupdlg;

	// the main widget
	QWidget *m_mainwidget;

	// do not show this dialog at startup
	QCheckBox *m_nodiag;

	// choose a template
	KJanusWidget * m_jwidget;
	KFileIconView *m_recent;
	QVGroupBox * boxdescription;
	KTextEdit * textedit;

	// choose a file
	MyFileDialog *m_filedialog;

	// for the layout
	QTabWidget* tabWidget;
	QWidget* newTab;
	QWidget* existingTab;
	QWidget* recentTab;

};

/******************************************************************/
/* Class: KoTemplateChooseDia                                     */
/******************************************************************/

/*================================================================*/
KoTemplateChooseDia::KoTemplateChooseDia(QWidget *parent, const char *name, KInstance* global,
	const QCString &format, const QString &nativePattern,
	const QString &nativeName, const DialogType &dialogType,
	const QCString& templateType) :
KDialogBase(parent, name, true, i18n("Open Document"), KDialogBase::Ok | KDialogBase::Cancel,
	KDialogBase::Ok) {

    d = new KoTemplateChooseDiaPrivate(
	    templateType,
	    global,
	    format,
	    nativePattern,
	    nativeName,
	    dialogType);

    QPushButton* ok = actionButton( KDialogBase::Ok );
    QPushButton* cancel = actionButton( KDialogBase::Cancel );
    cancel->setAutoDefault(false);
    ok->setDefault(true);
    //enableButtonOK(false);

    if (!templateType.isNull() && !templateType.isEmpty() && dialogType!=NoTemplates)
	d->tree=new KoTemplateTree(templateType, global, true);

    d->m_mainwidget=makeMainWidget();

    d->m_templateName = "";
    d->m_fullTemplateName = "";
    d->m_returnType = Cancel;

    setupDialog();

}

KoTemplateChooseDia::~KoTemplateChooseDia()
{
    delete d->tree;
    delete d;
    d=0L;
}

static bool cancelQuits() {
    bool onlyDoc = !KoDocument::documentList() || KoDocument::documentList()->count() <= 1;
    bool onlyMainWindow = !KMainWindow::memberList || KMainWindow::memberList->count() <= 1;
    return onlyDoc && onlyMainWindow && kapp->instanceName() != "koshell"; // hack for koshell
}

/*================================================================*/
// static
KoTemplateChooseDia::ReturnType KoTemplateChooseDia::choose(KInstance* global, QString &file,
	const QCString &format, const QString &nativePattern,
	const QString &nativeName,
	const KoTemplateChooseDia::DialogType &dialogType,
	const QCString& templateType) {

    KoTemplateChooseDia *dlg = new KoTemplateChooseDia( 0, "Choose", global, format, nativePattern,
	    nativeName, dialogType, templateType);

    KoTemplateChooseDia::ReturnType rt = Cancel;

    if (dlg->noStartupDlg())
    {
	// start with the default template
	file = dlg->getFullTemplate();
	rt = dlg->getReturnType();
    }
    else
    {
	dlg->resize( 700, 480 );
	if ( dlg->exec() == QDialog::Accepted )
	{
	    file = dlg->getFullTemplate();
	    rt = dlg->getReturnType();
	}
    }

    delete dlg;
    if ( rt == Cancel && dialogType == Everything && cancelQuits() )
        // The button says quit, so let's quit
        kapp->quit();

    return rt;
}

bool KoTemplateChooseDia::noStartupDlg() const {
    return d->m_nostartupdlg;
}


QString KoTemplateChooseDia::getTemplate() const{
    return d->m_templateName;
}

QString KoTemplateChooseDia::getFullTemplate() const{
    return d->m_fullTemplateName;
}

KoTemplateChooseDia::ReturnType KoTemplateChooseDia::getReturnType() const {
    return d->m_returnType;
}

KoTemplateChooseDia::DialogType KoTemplateChooseDia::getDialogType() const {
    return d->m_dialogType;
}

/*================================================================*/
// private
void KoTemplateChooseDia::setupRecentDialog(QWidget * widgetbase, QGridLayout * layout)
{

        d->m_recent = new KoTCDRecentFilesIconView(widgetbase, "recent files");
        // I prefer the icons to be in "most recent first" order (DF)
        d->m_recent->setSorting( static_cast<QDir::SortSpec>( QDir::Time | QDir::Reversed ) );
        layout->addWidget(d->m_recent,0,0);

        QString oldGroup = d->m_global->config()->group();
        d->m_global->config()->setGroup( "RecentFiles" );

        int i = 0;
        QString value;
        do {
                QString key=QString( "File%1" ).arg( i );
                value=d->m_global->config()->readPathEntry( key );
                if ( !value.isEmpty() ) {
                        KURL url(value);
                        KFileItem *item = new KFileItem( KFileItem::Unknown, KFileItem::Unknown, url );
                        d->m_recent->insertItem(item);
                }
                i++;
        } while ( !value.isEmpty() || i<=10 );

        d->m_global->config()->setGroup( oldGroup );
        d->m_recent->showPreviews();

	connect(d->m_recent, SIGNAL( doubleClicked ( QIconViewItem * ) ),
			this, SLOT( recentSelected( QIconViewItem * ) ) );

}

/*================================================================*/
// private
void KoTemplateChooseDia::setupFileDialog(QWidget * widgetbase, QGridLayout * layout)
{
    QString dir = QString::null;
    QPoint point( 0, 0 );

    d->m_filedialog=new MyFileDialog(dir,
	    QString::null,
	    widgetbase,
	    "file dialog",
	    false);

    layout->addWidget(d->m_filedialog,0,0);
    d->m_filedialog->reparent( widgetbase , point );
    //d->m_filedialog->setOperationMode( KFileDialog::Opening);

    QObjectList *l = d->m_filedialog->queryList( "QPushButton" );
    QObjectListIt it( *l );
    QObject *obj;
    while ( (obj = it.current()) != 0 ) {
	++it;
	((QPushButton*)obj)->hide();
    }
    delete l;

    d->m_filedialog->setSizeGripEnabled ( FALSE );
    d->m_filedialog->setMimeFilter(
	    KoFilterManager::mimeFilter( d->m_format, KoFilterManager::Import ));

    connect(d->m_filedialog, SIGNAL(  okClicked() ),
	    this, SLOT (  slotOk() ));

    connect(d->m_filedialog, SIGNAL( cancelClicked() ),
	    this, SLOT (  slotCancel() ));

}

/*================================================================*/
// private
void KoTemplateChooseDia::setupTemplateDialog(QWidget * widgetbase, QGridLayout * layout)
{

    d->m_jwidget = new KJanusWidget(
	    widgetbase,
	    "kjanuswidget",
	    KJanusWidget::IconList);
    layout->addWidget(d->m_jwidget,0,0);

    d->boxdescription = new QVGroupBox(
	    i18n("Selected Template"),
	    widgetbase,
	    "boxdescription");
    layout->addWidget(d->boxdescription, 1, 0 );

    // config
    KConfigGroup grp( d->m_global->config(), "TemplateChooserDialog" );
    int templateNum = grp.readEntry( "TemplateTab" ).toInt();
    QString templateName = grp.readPathEntry( "TemplateName" );

    // item which will be selected initially
    QIconViewItem * itemtoselect = 0;

    // count the templates inserted
    int entriesnumber = 0;

    for ( KoTemplateGroup *group = d->tree->first(); group!=0L; group=d->tree->next() )
    {
	if (group->isHidden())
	    continue;

	QFrame * frame = d->m_jwidget->addPage (
		group->name(),
		group->name(),
		group->first()->loadPicture());

	QGridLayout * layout = new QGridLayout(frame);
	KoTCDIconCanvas *canvas = new KoTCDIconCanvas( frame );
	layout->addWidget(canvas,0,0);

	canvas->setBackgroundColor( colorGroup().base() );
	canvas->setResizeMode(QIconView::Adjust);
	canvas->setWordWrapIconText( true );
	canvas->show();

	QIconViewItem * tempitem = canvas->load(group, templateName);
	if (tempitem)
	    itemtoselect = tempitem;

	canvas->sort();
	canvas->setSelectionMode(QIconView::Single);

	connect( canvas, SIGNAL( clicked ( QIconViewItem * ) ),
		this, SLOT( currentChanged( QIconViewItem * ) ) );

	connect( canvas, SIGNAL( doubleClicked( QIconViewItem * ) ),
		this, SLOT( chosen(QIconViewItem *) ) );

	entriesnumber++;
    }

    d->boxdescription->setInsideMargin ( 3 );
    d->boxdescription->setInsideSpacing ( 3 );

    d->textedit = new KTextEdit( d->boxdescription );
    d->textedit->setReadOnly(1);
    d->textedit->setText(descriptionText(i18n("Empty Document"), i18n("Creates an empty document")));
    d->textedit->setLineWidth(0);
    d->textedit->setMaximumHeight(50);

    // Hide the widget if there is no template available. This should never happen ;-)
    if (!entriesnumber)
	d->m_jwidget->hide();

    // Set the initially shown page, possibly from the last usage of the dialog
    if (entriesnumber >= templateNum)
	d->m_jwidget->showPage(templateNum);

    // Set the initially selected template, possibly from the last usage of the dialog
    currentChanged(itemtoselect);

    // setup the checkbox
    QString translatedstring = i18n("Always start %1 with the selected template").arg(d->m_nativeName);

    d->m_nodiag = new QCheckBox ( translatedstring , widgetbase);
    layout->addWidget(d->m_nodiag, 2, 0);
    QString  startwithoutdialog = grp.readEntry( "NoStartDlg" );
    d->m_nodiag->setChecked( startwithoutdialog == QString("yes") );
}

/*================================================================*/
// private
void KoTemplateChooseDia::setupDialog()
{

    QGridLayout *maingrid=new QGridLayout( d->m_mainwidget, 1, 1, 2, 6);
    KConfigGroup grp( d->m_global->config(), "TemplateChooserDialog" );

    if (d->m_dialogType == Everything)
    {

	// the user may want to start with his favorite template
	if (grp.readEntry( "NoStartDlg" ) == QString("yes") )
	{
	    d->m_nostartupdlg = true;
	    d->m_returnType = Empty;

	    // no default template, just start with an empty document
	    if (grp.readEntry("LastReturnType") == QString("Empty") )
		return;

	    // start with the default template
	    d->m_templateName = grp.readPathEntry( "TemplateName" );
	    d->m_fullTemplateName = grp.readPathEntry( "FullTemplateName" );

	    // be paranoid : invalid template means empty template
	    if (!QFile::exists(d->m_fullTemplateName))
		return;

	    if (d->m_fullTemplateName.length() < 2)
		return;

	    d->m_returnType = Template;
	    return;
	}

	if ( cancelQuits() )
	    setButtonCancelText(i18n("&Quit"));

	d->tabWidget = new QTabWidget( d->m_mainwidget, "tabWidget" );
	maingrid->addWidget( d->tabWidget, 0, 0 );

	// new document
	d->newTab = new QWidget( d->tabWidget, "newTab" );
	d->tabWidget->insertTab( d->newTab, i18n( "&Create Document" ) );
	QGridLayout * newTabLayout = new QGridLayout( d->newTab, 1, 1, KDialogBase::marginHint(), KDialogBase::spacingHint());

	// existing document
	d->existingTab = new QWidget( d->tabWidget, "existingTab" );
	d->tabWidget->insertTab( d->existingTab, i18n( "Open &Existing Document" ) );
	QGridLayout * existingTabLayout = new QGridLayout( d->existingTab, 1, 1, 0, KDialog::spacingHint());

        // recent document
        d->recentTab = new QWidget( d->tabWidget, "recentTab" );
        d->tabWidget->insertTab( d->recentTab, i18n( "Open &Recent Document" ) );
        QGridLayout * recentTabLayout = new QGridLayout( d->recentTab, 1, 1, KDialogBase::marginHint(), KDialog::spacingHint());

	setupTemplateDialog(d->newTab, newTabLayout);
	setupFileDialog(d->existingTab, existingTabLayout);
	setupRecentDialog(d->recentTab, recentTabLayout);

	QString tabhighlighted = grp.readEntry("LastReturnType");
	if ( tabhighlighted == QString("Template"))
	    d->tabWidget->setCurrentPage(0);
	else
	    d->tabWidget->setCurrentPage(1);
    }
    else
    {
	// open a file
	if (d->m_dialogType == NoTemplates)
	{
	    setupFileDialog(d->m_mainwidget, maingrid);
	}
	// create a new document from a template
	if (d->m_dialogType == OnlyTemplates)
	{
	    setCaption(i18n( "Create Document" ));
	    setupTemplateDialog(d->m_mainwidget, maingrid);
	}
    }
}

/*================================================================*/
// private SLOT
void KoTemplateChooseDia::currentChanged( QIconViewItem * item)
{
    if (item)
    {
	QIconView* canvas =  item->iconView();

	// set text in the textarea
	d->textedit->setText( descriptionText(
				item->text(),
				((KoTCDIconViewItem *) item)->getDescr()
				));

	// set the icon in the canvas selected
	if (canvas)
	    canvas->setSelected(item,1,0);

	// register the current template
	d->m_templateName = item->text();
	d->m_fullTemplateName = ((KoTCDIconViewItem *) item)->getFName();
    }
}

/*================================================================*/
// private SLOT
void KoTemplateChooseDia::chosen(QIconViewItem * item)
{
    // the user double clicked on a template
    if (item)
    {
	currentChanged(item);
	slotOk();
    }
}

/* */
// private SLOT
void KoTemplateChooseDia::recentSelected( QIconViewItem * item)
{
	if (item)
	{
		slotOk();
	}
}

/*================================================================*/
// protected SLOT
void KoTemplateChooseDia::slotOk()
{
    // Collect info from the dialog into d->m_returnType and d->m_templateName etc.
    if (collectInfo())
    {
	// Save it for the next time
	KConfigGroup grp( d->m_global->config(), "TemplateChooserDialog" );
	static const char* const s_returnTypes[] = { 0 /*Cancel ;)*/, "Template", "File", "Empty" };
	if ( d->m_returnType <= Empty )
	{
	    grp.writeEntry( "LastReturnType", QString::fromLatin1(s_returnTypes[d->m_returnType]) );
	    if (d->m_returnType == Template)
	    {
		grp.writeEntry( "TemplateTab", d->m_jwidget->activePageIndex() );
#if KDE_IS_VERSION(3,1,3)
		grp.writePathEntry( "TemplateName", d->m_templateName );
		grp.writePathEntry( "FullTemplateName", d->m_fullTemplateName);
#else
		grp.writeEntry( "TemplateName", d->m_templateName );
		grp.writeEntry( "FullTemplateName", d->m_fullTemplateName);
#endif
	    }

	    if (d->m_nodiag)
	    {
		if (d->m_nodiag->isChecked())
		    grp.writeEntry( "NoStartDlg", "yes");
		else
		    grp.writeEntry( "NoStartDlg", "no");
	    }
	}
	else
	{
	    kdWarning(30003) << "Unsupported template chooser result: " << d->m_returnType << endl;
	    grp.writeEntry( "LastReturnType", QString::null );
	}
	KDialogBase::slotOk();
    }
}

/*================================================================*/
// private
bool KoTemplateChooseDia::collectInfo()
{


    // to determine what tab is selected in "Everything" mode
    bool newTabSelected = false;
    if ( d->m_dialogType == Everything)
	if ( d->tabWidget->currentPage() == d->newTab )
	    newTabSelected = true;

    // is it a template or a file ?
    if ( d->m_dialogType==OnlyTemplates || newTabSelected )
    {
	// a template is chosen
	if (d->m_templateName.length() > 0)
	    d->m_returnType = Template;
	else
	    d->m_returnType=Empty;

	return true;
    }
    else if ( d->m_dialogType != OnlyTemplates )
    {
	// a file is chosen
	if (d->m_dialogType == Everything && d->tabWidget->currentPage() == d->recentTab)
	{
		// Recent file
		KFileItem * item = d->m_recent->currentFileItem();
		if (! item)
			return false;
		KURL url = item->url();
		if(url.isLocalFile() && !QFile::exists(url.path()))
		{
			KMessageBox::error( this, i18n( "The file %1 doesn't exist." ).arg( url.path() ) );
			return false;
		}
		d->m_fullTemplateName = url.url();
		d->m_returnType = File;
	}
	else
	{
		// Existing file from file dialog
		KURL url = d->m_filedialog->currentURL();
		d->m_fullTemplateName = url.url();
		d->m_returnType = File;
                return d->m_filedialog->checkURL();
	}
	return true;
    }

    d->m_returnType=Empty;
    return false;
}

/*================================================================*/
//private
QString KoTemplateChooseDia::descriptionText(const QString &name, const QString &description)
{
	QString descrText(i18n("Name:"));
	descrText += " " + name;
	descrText += "\n";
	descrText += i18n("Description:");
	if (description.isEmpty())
	      descrText += " " + i18n("No description available");
	else
              descrText += " " + description;
	return descrText;
}

/*================================================================*/

QIconViewItem * KoTCDIconCanvas::load( KoTemplateGroup *group, const QString& name)
{
    QIconViewItem * itemtoreturn = 0;

    for (KoTemplate *t=group->first(); t!=0L; t=group->next()) {
	if (t->isHidden())
	    continue;
	QIconViewItem *item = new KoTCDIconViewItem(
		this,
		t->name(),
		t->loadPicture(),
		t->description(),
		t->file());

	if (name == t->name())
	{
	    itemtoreturn = item;
	}

	item->setKey(t->name());
	item->setDragEnabled(false);
	item->setDropEnabled(false);
    }

    return itemtoreturn;
}

/*================================================================*/

KoTCDRecentFilesIconView::~KoTCDRecentFilesIconView()
{
    removeToolTip();
}

void KoTCDRecentFilesIconView::showToolTip( QIconViewItem* item )
{
    removeToolTip();
    if ( !item )
        return;

    // Mostly duplicated from KFileIconView, because it only shows tooltips
    // for truncated icon texts, and we want tooltips on all icons,
    // with the full path...
    // KFileIconView would need a virtual method for deciding if a tooltip should be shown,
    // and another one for deciding what's the text of the tooltip...
    const KFileItem *fi = ( (KFileIconViewItem*)item )->fileInfo();
    QString toolTipText = fi->url().prettyURL( 0, KURL::StripFileProtocol );
    toolTip = new QLabel( QString::fromLatin1(" %1 ").arg(toolTipText), 0,
                          "myToolTip",
                          WStyle_StaysOnTop | WStyle_Customize | WStyle_NoBorder | WStyle_Tool | WX11BypassWM );
    toolTip->setFrameStyle( QFrame::Plain | QFrame::Box );
    toolTip->setLineWidth( 1 );
    toolTip->setAlignment( AlignLeft | AlignTop );
    toolTip->move( QCursor::pos() + QPoint( 14, 14 ) );
    toolTip->adjustSize();
    QRect screen = QApplication::desktop()->screenGeometry(
        QApplication::desktop()->screenNumber(QCursor::pos()));
    if (toolTip->x()+toolTip->width() > screen.right()) {
        toolTip->move(toolTip->x()+screen.right()-toolTip->x()-toolTip->width(), toolTip->y());
    }
    if (toolTip->y()+toolTip->height() > screen.bottom()) {
        toolTip->move(toolTip->x(), screen.bottom()-toolTip->y()-toolTip->height()+toolTip->y());
    }
    toolTip->setFont( QToolTip::font() );
    toolTip->setPalette( QToolTip::palette(), TRUE );
    toolTip->show();
}

void KoTCDRecentFilesIconView::removeToolTip()
{
    delete toolTip;
    toolTip = 0;
}

void KoTCDRecentFilesIconView::hideEvent( QHideEvent *ev )
{
    removeToolTip();
    KFileIconView::hideEvent( ev );
}

#include "koTemplateChooseDia.moc"
