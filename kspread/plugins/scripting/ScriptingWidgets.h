/*
 * This file is part of KSpread
 *
 * Copyright (c) 2007 Sebastian Sauer <mail@dipe.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef SCRIPTINGWIDGETS_H
#define SCRIPTINGWIDGETS_H

#include <QObject>
#include <QVariant>
#include <QStringList>
#include <QWidget>

class QTreeView;
class ScriptingModule;

/**
 * The ScriptingSheetsListView provides a listview-widget that displays
 * all sheets and lets the user choose 0..n of them plus specify
 * cell-ranges for all of them.
 */
class ScriptingSheetsListView : public QWidget
{
        Q_OBJECT
    public:

        /**
        * Constructor.
        * \param module The \a ScriptingModule instance that should be used
        * as datasource for what should be displayed within the listview.
        * \param parent The parent QWidget this widget should be displayed in.
        */
        ScriptingSheetsListView(ScriptingModule* module, QWidget* parent);

        /**
        * Destructor.
        */
        virtual ~ScriptingSheetsListView();

    public slots:

        /**
        * Return the list of all selected sheets. Selected are those sheets the
        * user enabled the checkbutton for and the resulting list contains for
        * each such sheetitem a list of the name and the range rectangle.
        */
        QVariantList sheets();

    private:
        ScriptingModule* m_module;
        QTreeView* m_view;
        bool m_initialized;
        QVariantList m_prevlist;

        void initialize();
        void finalize();

        virtual void showEvent(QShowEvent* event);
};

#endif
