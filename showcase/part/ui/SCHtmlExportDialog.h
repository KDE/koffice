/* This file is part of the KDE project
 * Copyright (C) 2009 Yannick Motta <yannick.motta@gmail.com>
 * Copyright (C) 2009-2010 Benjamin Port <port.benjamin@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef SCHTMLEXPORTDIALOG_H
#define SCHTMLEXPORTDIALOG_H

#include <ui_SCHtmlExport.h>

#include <QWebPage>

#include <KoPAPage.h>

class SCHtmlExportDialog  : public KDialog
{
    Q_OBJECT
public:
    SCHtmlExportDialog(const QList<KoPAPage*> &slides, const QString &title, const QString &author, QWidget *parent=0);

    QList<KoPAPage*> checkedSlides();
    QStringList slidesNames();
    KUrl templateUrl();
    QString title();
    QString author();
    bool openBrowser();

private slots:
    void checkAllItems();
    void uncheckAllItems();
    void renderPreview();
    void favoriteAction();
    void updateFavoriteButton();
    void generateNext();
    void generatePrevious();
    void generatePreview(int item=-1);
    void browserAction();


private:
    void generateSlidesNames(const QList<KoPAPage*> &slides);
    void loadTemplatesList();
    bool selectedTemplateIsFavorite();
    bool selectedTemplateIsSystemFavorite();
    bool verifyZipFile(const QString &zipLocalPath);
    void addSelectedTemplateToFavorite();
    void delSelectedTemplateFromFavorite();

    QList<KoPAPage*> m_allSlides;
    QString m_title;
    Ui::SCHtmlExport ui;
    QWebPage preview;
    int frameToRender;


};



#endif // SCHTMLEXPORTDIALOG_H
