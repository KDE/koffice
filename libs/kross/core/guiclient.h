/***************************************************************************
 * guiclient.h
 * This file is part of the KDE project
 * copyright (C) 2005-2006 by Sebastian Sauer (mail@dipe.org)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 * You should have received a copy of the GNU Library General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 ***************************************************************************/

#ifndef KROSS_GUICLIENT_H
#define KROSS_GUICLIENT_H

#include "../core/krossconfig.h"
#include "../core/action.h"

#include <QObject>

#include <kurl.h>
#include <kxmlguiclient.h>

namespace Kross {

    /**
     * The GUIClient class implements a KXMLGUIClient to provide
     * abstract access to the Kross Scripting Framework to an
     * application.
     */
    class KDE_EXPORT GUIClient
        : public QObject
        , public KXMLGUIClient
    {
            Q_OBJECT

        public:

            /**
             * Constructor.
             *
             * \param guiclient The KXMLGUIClient this \a GUIClient
             *        is a child of.
             * \param parent The parent QWidget. If defined Qt will handle
             *        freeing this \a GUIClient instance else the
             *        caller has to take care of freeing this instance
             *        if not needed any longer.
             */
            explicit GUIClient(KXMLGUIClient* guiclient, QWidget* parent = 0);

            /**
             * Destructor.
             */
            virtual ~GUIClient();

            /**
             * KXMLGUIClient overloaded method to set the XML file.
             */
            virtual void setXMLFile(const QString& file, bool merge = false, bool setXMLDoc = true);

            /**
             * KXMLGUIClient overloaded method to set the XML DOM-document.
             */
            virtual void setDOMDocument(const QDomDocument &document, bool merge = false);

            /**
             * \return the KActionCollection which holds the list of \a Action instances.
             */
            KActionCollection* scriptsActionCollection() const;

            /**
             * This method tries to determinate all available packages and fills
             * the configuration with actions defined there.
             */
            bool writeConfigFromPackages();

        public slots:

            /**
            * A KFileDialog will be displayed to let the user choose the scriptfile
            * that should be executed.
            */
            bool executeFile();

            /**
            * Execute the scriptfile \p file . Internaly we try to use
            * the defined filename to auto-detect the \a Interpreter which
            * should be used for the execution.
            */
            bool executeFile(const KUrl& file);

            /**
            * The \a ScriptManagerGUI dialog will be displayed to
            * let the user manage the scriptfiles.
            */
            void showManager();

        private slots:

            /**
             * Called before the "Scripts" menu is shown to update the list of displayed scripts.
             */
            void slotMenuAboutToShow();

            /**
             * Called if execution started.
             */
            void started(Kross::Action*);

            /**
             * Called if execution finished.
             */
            void finished(Kross::Action*);

        private:
            /// \internal d-pointer class.
            class Private;
            /// \internal d-pointer instance.
            Private* const d;
    };

}

#endif

