/* This file is part of the KDE project
   Copyright (C) 2006-2008 Thorsten Zachmann <zachmann@kde.org>

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
 * Boston, MA 02110-1301, USA.
*/

#ifndef SCDOCUMENT_H
#define SCDOCUMENT_H

#include <KoPADocument.h>

class SCDeclarations;
class SCShapeAnimation;
class SCShapeAnimations;
class SCCustomSlideShows;

class SCDocument : public KoPADocument
{
    Q_OBJECT
public:
    explicit SCDocument(QWidget* parentWidget, QObject* parent, bool singleViewMode = false);
    ~SCDocument();

    /// reimplemented
    virtual KoPAPage *newPage(KoPAMasterPage *masterPage);
    /// reimplemented
    virtual KoPAMasterPage * newMasterPage();

    /// reimplemented
    virtual KOdf::DocumentType documentType() const;

    /**
     * @brief Add animation to shape
     *
     * @param animation animation to add to shape
     */
    void addAnimation(SCShapeAnimation * animation);

    /**
     * @brief Remove animation from shape
     *
     * @param animation animation to remove from shape
     * @param removeFromApplicationData if true the animation will also be removed from the
     *        application data
     */
    void removeAnimation(SCShapeAnimation * animation, bool removeFromApplicationData = true);

    /**
     * @brief get the slideShows defined for this document
     */
    SCCustomSlideShows* customSlideShows();
    void setCustomSlideShows(SCCustomSlideShows* replacement);

    /**
     * Get the presentation monitor (screen) used for presentation
     *
     * @return the screen used for presentation, starting from screen 0
     */
    int presentationMonitor();

    /**
     * Set the presentation monitor (screen) used for presentation
     *
     * @param monitor the new screen number used for presentation
     */
    void setPresentationMonitor(int monitor);

    /**
     * Check whether the presenter view feature is enabled for presentation
     *
     * @return true if presenter view is enabled, false otherwise
     */
    bool isPresenterViewEnabled();

    /**
     * Enable / disable the presenter view features
     *
     * @param enabled whether the presenter view should be enabled or disabled
     */
    void setPresenterViewEnabled(bool enabled);

    /**
     * Get the list of pages for slide show. It is possible that the pages for
     * slideshow are different from KoPADocument::pages() due to custom slide show
     *
     * @return the list of pages for slide show
     */
    QList<KoPAPage*> slideShow() const;

    /**
     * Get the name of currently active custom slide show, or an empty string
     * if "all slides" is used for the slide show and no active custom slide show
     *
     * @return the name of currently active custom slide show, or empty string if none
     */
    QString activeCustomSlideShow() const;

    /**
     * Set the currently active custom slide show. The custom slide show name should
     * be valid, i.e. SCCustomSlideShow::names() contains the name
     *
     * @param customSlideShow the new active custom slide show
     */
    void setActiveCustomSlideShow(const QString &customSlideShow);

    /// reimplemented
    virtual void saveOdfDocumentStyles(KoPASavingContext &context);

    /// reimplemented
    virtual bool loadOdfDocumentStyles(KoPALoadingContext &context);

    /// reimplemented
    virtual bool loadOdfProlog(const KXmlElement &body, KoPALoadingContext &context);

    /**
     * Get the page type used in the document
     *
     * The default page type KoPageApp::Page is returned
     */
    virtual KoPageApp::PageType pageType() const;

    /**
     * Get the SCDeclarations pointer
     */
    SCDeclarations * declarations() const;

    /**
     * Creates and shows the start up widget. Reimplemented from KoDocument.
     *
     * @param parent the KoMainWindow used as parent for the widget.
     * @param alwaysShow always show the widget even if the user has configured it to not show.
     */
    void showStartUpWidget(KoMainWindow * parent, bool alwaysShow);

public slots:
    virtual void initEmpty();

signals:
    /**
     * Emitted when the active custom slide show changes.
     * This is to allow for signalling dbus interfaces.
     *
     * @param customSlideShow the new active custom slide show
     */
    void activeCustomSlideShowChanged(const QString &customSlideShow);

    /**
     * Emitted when the custom slide shows have been modified.
     * This is to allow for signalling dbus interfaces.
     */
    void customSlideShowsModified();

protected:
    /// reimplemented
    virtual KoView * createViewInstance(QWidget *parent);
    /// reimplemented
    virtual const char *odfTagName(bool withNamespace);

    /// reimplemented
    virtual bool loadOdfEpilogue(const KXmlElement &body, KoPALoadingContext &context);

    /// reimplemented
    virtual bool saveOdfProlog(KoPASavingContext &paContext);

    /// reimplemented
    virtual bool saveOdfEpilogue(KoPASavingContext &context);

    /// reimplemented
    virtual void postAddShape(KoPAPage * page, KShape * shape);
    /// reimplemented
    virtual void postRemoveShape(KoPAPage * page, KShape * shape);

    /// reimplemented
    virtual void pageRemoved(KoPAPage * page, QUndoCommand * parent);

    /// load configuration specific to Showcase
    void loadKPrConfig();

    /// save configuration specific to Showcase
    void saveKPrConfig();

    /**
     * @brief get the animations of the page
     */
    SCShapeAnimations &animationsByPage(KoPAPage * page);

    SCCustomSlideShows *m_customSlideShows;

protected slots:
    /// Quits Showcase with error message from m_errorMessage.
    void showErrorAndDie();

private:
    int m_presentationMonitor;
    bool m_presenterViewEnabled;
    QString m_activeCustomSlideShow;
    /// Message shown before Showcase quits with an error if something is wrong
    QString m_errorMessage;
    SCDeclarations *m_declarations;
};

#endif /* SCDOCUMENT_H */
