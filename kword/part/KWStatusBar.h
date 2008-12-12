/* This file is part of the KDE project
 * Copyright (C) 2007 Sebastian Sauer <mail@dipe.org>
 * Copyright (C) 2008 Thomas Zander <zander@kde.org>
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
 * Boston, MA 02110-1301, USA
 */

#ifndef KWSTATUSBAR_H
#define KWSTATUSBAR_H

#include <QPointer>

class QPoint;
class KStatusBar;
class KWView;
class QLabel;
class KSqueezedTextLabel;
class KoCanvasController;

/**
* The KWStatusBar class implements an extended statusbar for KWord.
*/
class KWStatusBar : public QObject
{
    Q_OBJECT
public:

    /**
    * Constructor.
    *
    * \param statusBar The parent statusbar this statusbar is child
    * of. We will embed our own widgets into this statusbar.
    * \param view The KWord view instance the statusbar belongs to. Each
    * KWStatusBar instance belongs to exactly one view.
    */
    KWStatusBar(KStatusBar* statusBar, KWView* view);

    /**
    * Destructor.
    */
    virtual ~KWStatusBar();

public slots:
    void setText(const QString& text);

private slots:
    void setModified(bool modified);
    void updatePageCount();
    void updateMousePosition(const QPoint&);
    void resourceChanged(int, const QVariant&);
    void updateCurrentTool();

private:
    KStatusBar * m_statusbar;
    KWView* m_view;
    QPointer<KoCanvasController> m_controller;
    int m_currentPageNumber;

    QLabel* m_modifiedLabel;
    QLabel* m_pageLabel;
    QLabel* m_mousePosLabel;
    KSqueezedTextLabel* m_statusLabel;
    QWidget* m_zoomWidget;
};

#endif
