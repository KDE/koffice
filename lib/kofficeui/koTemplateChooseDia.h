/*
   This file is part of the KDE project
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

#ifndef koTemplateChooseDia_h
#define koTemplateChooseDia_h

#include <kdialogbase.h>
#include <kicondialog.h>
#include <kiconview.h>

// KoTCD : KoTemplateChooseDia

class KoTCDIconViewItem;
class KoTemplateTree;
class KoTemplateGroup;
class QGridLayout;

/**
 * Our reimplementation of KIconCanvas used within the template-chooser dialog.
 * @internal
 */
class KoTCDIconCanvas : public KIconCanvas
{
    Q_OBJECT
    public:
	KoTCDIconCanvas( QWidget *parent = 0, const char *name = 0L )
	    : KIconCanvas( parent, name ) {}

	bool isCurrentValid() { return currentItem(); }
	QIconViewItem * load(KoTemplateGroup *group, const QString& name);

    protected:
	virtual void keyPressEvent( QKeyEvent *e ) {
	    if ( e->key() == Key_Return || e->key() == Key_Enter )
		e->ignore();
	    else
		KIconCanvas::keyPressEvent( e );
	}
};

/// @internal
class KoTCDIconViewItem : public KIconViewItem
{
    public:
	KoTCDIconViewItem(QIconView *parent=0)
	    : KIconViewItem ( parent )
	    {}

	KoTCDIconViewItem(QIconView *parent=0, const QString &text=0, const QPixmap &icon=0, const QString &descr=0, const QString &fullname=0)
	    : KIconViewItem(parent, text, icon)
	    {
		m_descr = descr;
		m_full = fullname;
	    }

	QString getDescr() { return m_descr; }
	QString getFName() { return m_full; }

    private :
	QString m_descr;
	QString m_full;

};


class KInstance;
class KoTemplateChooseDiaPrivate;

/**
 *  class KoTemplateChooseDia - This class is used to show the template dialog
 *  on startup. Unless you need something special, you should use the static
 *  method choose().
 *
 *  @short The template choose dialog
 *  @author Reginald Stadlbauer <reggie@kde.org>
 *  @author Werner Trobin <trobin@kde.org>
 */
class KoTemplateChooseDia : public KDialogBase
{
    Q_OBJECT

    public:
	/**
	 * The Dialog returns one of these values depending
	 * on the input of the user.
	 * Cancel = The user pressed 'Cancel'
	 * Template = The user selected a template
	 * File = The user has chosen a file
	 * Empty = The user selected "Empty document"
	 */
	enum ReturnType { Cancel, Template, File, Empty };
	/**
	 * To configure the dialog you have to use this enum.
	 * Everything = Show templates and the rest of the dialog
	 * OnlyTemplates = Show only the templates
	 * NoTemplates = Just guess :)
	 */
	enum DialogType { Everything, OnlyTemplates, NoTemplates };

	/**
	 * This is the CTOR to create a dialog
	 * @param parent parent the parent of the dialog
	 * @param name the Qt internal name
	 * @param global the KInstance of your app
	 * @param format is the mimetype of the app (e.g. application/x-kspread)
	 * @param nativeName is the name of your app (e.g KSpread)
	 * @param nativePattern is the native pattern (e.g. *.ksp)
	 * @param dialogType the type of the dialog
	 * @param templateType the template type of your application (see kword or
	 *        kpresenter for details)
	 * @param hasCancel if there should be a "Cancel" button
	 * @return The return type (see above)
	 */
	KoTemplateChooseDia(QWidget *parent, const char *name, KInstance* global,
		const QCString &format="",
		const QString &nativePattern=QString::null,
		const QString &nativeName=QString::null,
		const DialogType &dialogType=Everything,
		const QCString& templateType="");
	~KoTemplateChooseDia();

	/**
	 * This is the static method you'll normally use to show the
	 * dialog.
	 * @param global the KInstance of your app
	 * @param file this is the filename which is returned to your app
	 * @param format is the mimetype of the app (e.g. application/x-kspread)
	 * @param nativeName is the name of your app (e.g KSpread)
	 * @param nativePattern is the native pattern (e.g. *.ksp)
	 * @param dialogType the type of the dialog
	 * @param templateType the template type of your application (see kword or
	 *        kpresenter for details)
	 * @param hasCancel if there should be a "Cancel" button
	 * @return The return type (see above)
	 */
	static ReturnType choose(KInstance* global, QString &file,
		const QCString &format="",
		const QString &nativePattern=QString::null,
		const QString &nativeName=QString::null,
		const DialogType &dialogType=Everything,
		const QCString& templateType="");

	/**
	 * Method to get the current template
	 */
	QString getTemplate();
	/**
	 * Method to get the "full" template (path+template)
	 */
	QString getFullTemplate();
	/**
	 * The ReturnType (call this one after exec())
	 */
	ReturnType getReturnType();
	/**
	 * The dialogType - normally you won't need this one
	 */
	DialogType getDialogType();

    protected slots:
	    /**
	     * Activated when the Ok button has been clicked.
	     */
	    virtual void slotOk();

    private:

	KoTemplateChooseDiaPrivate *d;

	QString descriptionText(const QString &name, const QString &description);
	void setupDialog();
	void setupTemplateDialog(QWidget * widgetbase, QGridLayout * layout);
	void setupFileDialog(QWidget * widgetbase, QGridLayout * layout);
	void setupRecentDialog(QWidget * widgetbase, QGridLayout * layout);
	bool collectInfo();
	bool noStartupDlg();

    private slots:

        void chosen(QIconViewItem *);
	void currentChanged( QIconViewItem * );
	void recentSelected( QIconViewItem * );
};

#endif

