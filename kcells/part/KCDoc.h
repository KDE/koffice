/* This file is part of the KDE project
   Copyright 2007 Stefan Nikolaus <stefan.nikolaus@kdemail.net>
   Copyright 2007 Thorsten Zachmann <zachmann@kde.org>
   Copyright 2004 Ariya Hidayat <ariya@kde.org>
   Copyright 2002-2003 Norbert Andres <nandres@web.de>
   Copyright 2000-2005 Laurent Montel <montel@kde.org>
   Copyright 2002 John Dailey <dailey@vt.edu>
   Copyright 2002 Phillip Mueller <philipp.mueller@gmx.de>
   Copyright 2000 Werner Trobin <trobin@kde.org>
   Copyright 1999-2000 Simon Hausmann <hausmann@kde.org>
   Copyright 1999 David Faure <faure@kde.org>
   Copyright 1998-2000 Torben Weis <weis@kde.org>
   
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
   Boston, MA 02110-1301, USA.
*/

#ifndef KSPREAD_DOC
#define KSPREAD_DOC

#include <QList>
#include <QMap>
#include <QRect>
#include <QString>

#include <kglobalsettings.h>

#include <KoDocument.h>
#include <KXmlReader.h>
#include <KOdfGenericStyle.h>

#include "../Global.h"
#include "../KCDocBase.h"

#include "../kcells_export.h"

class QDomDocument;
class QPainter;

class KOdfGenericStyles;
class KoOdfSettings;
class KoResourceManager;
class KOdfStore;
class KXmlWriter;

#define MIME_TYPE "application/x-kcells"

class KCSheet;
class KCDoc;
class KCView;
class KCMap;
class KCRegion;
class UndoAction;
class KCSheetAccessModel;

/**
 * This class holds the data that makes up a spreadsheet.
 */
class KCELLS_EXPORT KCDoc : public KCDocBase
{
    Q_OBJECT
    Q_PROPERTY(int syntaxVersion READ syntaxVersion)

public:
    /**
     * Creates a new document.
     * @param parentWidget the parent widget
     * @param parent the parent object
     * @param singleViewMode enables single view mode, if @c true
     */
    explicit KCDoc(QWidget* parentWidget = 0, QObject* parent = 0, bool singleViewMode = false);

    /**
     * Destroys the document.
     */
    ~KCDoc();


    /**
     * @return the MIME type of KCells document
     */
    virtual QByteArray mimeType() const {
        return MIME_TYPE;
    }

    virtual bool completeSaving(KOdfStore* _store);


    /**
     * \ingroup NativeFormat
     * Main saving method.
     */
    virtual QDomDocument saveXML();

    /**
     * \ingroup NativeFormat
     * Main loading method.
     */
    virtual bool loadXML(const KoXmlDocument& doc, KOdfStore *store);

    /**
     * \ingroup OpenDocument
     */
    void loadOdfCalculationSettings(const KoXmlElement& body);


    virtual int supportedSpecialFormats() const;

    virtual bool loadChildren(KOdfStore* _store);

    virtual void addView(KoView *_view);

    bool docData(QString const & xmlTag, QDomElement & data);

    // reimplemented; paints the thumbnail
    virtual void paintContent(QPainter & painter, const QRect & rect);
    virtual void paintContent(QPainter & painter, const QRect & rect, KCSheet* sheet);

    void initConfig();
    void saveConfig();

    void updateBorderButton();

    void addIgnoreWordAll(const QString & word);
    void clearIgnoreWordAll();
    void addIgnoreWordAllList(const QStringList & _lst);
    QStringList spellListIgnoreAll() const ;

    /* KCFunction specific when we load config from file */
    void loadConfigFromFile();
    bool configLoadFromFile() const;


    virtual bool saveOdfHelper(SavingContext &documentContext, SaveFlag saveFlag,
                       QString* plainText = 0);

    /**
     * Requests an update of all attached user interfaces (views).
     */
    void updateAllViews();


public Q_SLOTS:
    virtual void initEmpty();

Q_SIGNALS:
    /**
     * Emitted, if all user interfaces (views) have to be updated.
     */
    void updateView();

protected Q_SLOTS:
    virtual void openTemplate(const KUrl& url);

    void sheetAdded(KCSheet* sheet);
protected:

    KoView* createViewInstance(QWidget* parent);

    /**
     * @reimp Overloaded function of KoDocument.
     */
    virtual bool completeLoading(KOdfStore*);

    /**
     * @reimp Overloaded function of KoDocument.
     */
    virtual bool saveChildren(KOdfStore* _store);

    virtual void saveOdfViewSettings(KXmlWriter& settingsWriter);
    virtual void saveOdfViewSheetSettings(KCSheet *sheet, KXmlWriter &settingsWriter);
private:
    Q_DISABLE_COPY(KCDoc)

    class Private;
    Private * const dd;

    /**
     * \ingroup NativeFormat
     */
    void loadPaper(KoXmlElement const & paper);
};

#endif /* KSPREAD_DOC */
