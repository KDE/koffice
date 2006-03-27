/***************************************************************************
 * testwindow.cpp
 * This file is part of the KDE project
 * copyright (C)2004-2005 by Sebastian Sauer (mail@dipe.org)
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

#include "testwindow.h"
#include "testplugin.h"

#include <qlabel.h>
#include <q3vbox.h>
#include <qvgroupbox.h>
//#include <qhgroupbox.h>
#include <qcombobox.h>
#include <qdir.h>
#include <q3popupmenu.h>

#include <ktextedit.h>
#include <kpushbutton.h>
#include <kmenu.h>
#include <kmenubar.h>
#include <kstandarddirs.h>
#include <kdebug.h>

TestWindow::TestWindow(const QString& interpretername, const QString& scriptcode)
    : KMainWindow()
    , m_interpretername(interpretername)
    , m_scriptcode(scriptcode)
{
    Kross::Api::Manager* manager = Kross::Api::Manager::scriptManager();
    manager->addModule( new TestPluginModule("krosstestpluginmodule") );
    m_scriptcontainer = manager->getScriptContainer("test");

    KMenu *menuFile = new KMenu( this );
    menuBar()->insertItem( "&File", menuFile );

    m_scriptextension = new Kross::Api::ScriptGUIClient(this, this);

    QString file = KGlobal::dirs()->findResource("appdata", "testscripting.rc");
    if(file.isNull())
        file = QDir(QDir::currentPath()).filePath("testscripting.rc");
    else kDebug()<<"-------------------------222222"<<endl;

    kDebug()<<"XML-file: "<<file<<endl;
    m_scriptextension->setXMLFile(file);

    //menuFile->insertSeparator();

    KAction* execaction = m_scriptextension->action("executescriptfile");
    if(execaction) execaction->plug(menuFile);

    KAction* configaction = m_scriptextension->action("configurescripts");
    if(configaction) configaction->plug(menuFile);

    KAction* scriptsaction = m_scriptextension->action("installedscripts");
    if(scriptsaction) scriptsaction->plug(menuFile);
    //menuFile->insertItem( ( (KActionMenu*)m_scriptextension->action("scripts") )->popupMenu() );

    Q3VBox* mainbox = new Q3VBox(this);

    QVGroupBox* interpretergrpbox = new QVGroupBox("Interpreter", mainbox);
    QStringList interpreters = Kross::Api::Manager::scriptManager()->getInterpreters();
    m_interpretercombo = new QComboBox(interpretergrpbox);
    m_interpretercombo->insertStringList(interpreters);
    m_interpretercombo->setCurrentText(interpretername);

    QVGroupBox* scriptgrpbox = new QVGroupBox("Scripting code", mainbox);
    m_codeedit = new KTextEdit(m_scriptcode, QString::null, scriptgrpbox);
    m_codeedit->setWordWrap(Q3TextEdit::NoWrap);
    m_codeedit->setTextFormat(Qt::PlainText);

    Q3HBox* btnbox = new Q3HBox(mainbox);
    KPushButton* execbtn = new KPushButton("Execute", btnbox);
    connect(execbtn, SIGNAL(clicked()), this, SLOT(execute()));

    setCentralWidget(mainbox);
    setMinimumSize(600,420);
}

TestWindow::~TestWindow()
{
}

void TestWindow::execute()
{
    m_scriptcontainer->setInterpreterName( m_interpretercombo->currentText() );
    m_scriptcontainer->setCode(m_codeedit->text());
    Kross::Api::Object::Ptr result = m_scriptcontainer->execute();
    if(m_scriptcontainer->hadException()) {
        kDebug() << "EXCEPTION => " << m_scriptcontainer->getException()->toString() << endl;
    }
    else {
        QString s = result ? result->toString() : QString::null;
        kDebug() << "DONE => " << s << endl;
    }
}

#include "testwindow.moc"
