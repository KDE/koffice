/***************************************************************************
 * KoScriptManager.h
 * This file is part of the KDE project
 * copyright (c) 2005-2006 Cyrille Berger <cberger@cberger.net>
 * copyright (C) 2006-2007 Sebastian Sauer <mail@dipe.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 ***************************************************************************/

#ifndef KOKROSS_KOSCRIPTMANAGER_H
#define KOKROSS_KOSCRIPTMANAGER_H

#include <QtGui/QWidget>

#include <kdialog.h>

class KoScriptManagerView;

/**
* The KoScriptManagerCollection class shows a QListView where the content of a
* \a ActionCollection is displayed and some buttons to run, stop, add, edit
* and remove scripts.
*/
class KoScriptManagerCollection : public QWidget
{
    Q_OBJECT
public:

    /**
    * Constructor.
    * \param parent The parent widget this widget is child of.
    */
    explicit KoScriptManagerCollection(QWidget *parent);

    /**
    * Destructor.
    */
    virtual ~KoScriptManagerCollection();

    /**
    * \return true if the collection was modified.
    */
    //bool isModified() const;

#if 0
public slots:

    /**
    * Run the selected script.
    */
    void slotRun();

    /**
    * Stop the selected script if running.
    */
    void slotStop();

    /**
    * Edit the select item.
    */
    void slotEdit();

    /**
    * Add a new item.
    */
    void slotAdd();

    /**
    * Remove the selected item.
    */
    void slotRemove();

private slots:
    /// The selected item changed.
    void slotSelectionChanged();
    /// The data changed.
    void slotDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight);
#endif

private:
    bool m_modified;
    KoScriptManagerView *m_view;
};

class KoScriptManagerDialog : public KDialog
{
    Q_OBJECT
public:
    explicit KoScriptManagerDialog();
    virtual ~KoScriptManagerDialog();

private slots:
    void slotAccepted();

private:
    KoScriptManagerCollection *m_collection;
};

#endif
