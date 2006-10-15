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
//#include "testobject.h"

#include <QLabel>
#include <QMenu>
#include <QGroupBox>
#include <QComboBox>
#include <QDir>
#include <QVBoxLayout>

#include <ktextedit.h>
#include <kpushbutton.h>
#include <kmenubar.h>
#include <kstandarddirs.h>

TestWindow::TestWindow(const QString& interpretername, const QString& scriptcode)
    : KMainWindow()
    , m_interpretername(interpretername)
    , m_scriptcode(scriptcode)
{
    QMenu *menuFile = menuBar()->addMenu( "&File" );

    //Kross::Manager::self().addModule( Kross::Module::Ptr(new TestPluginModule("krosstestpluginmodule")) );
    Kross::Action::Ptr action( new Kross::Action( "test" ) );
    m_scriptextension = new Kross::GUIClient(this, this);

    /*
    QString file = KGlobal::dirs()->findResource("appdata", "testscripting.rc");
    if(file.isNull())
        file = QDir( QDir::currentDirPath() ).filePath("testscripting.rc");

    Kross::krossdebug(QString("XML-file: %1").arg(file));
    m_scriptextension->setXMLFile(file);
    */

    //menuFile->insertSeparator();

    menuFile->addAction( m_scriptextension->action("executescriptfile") );
    menuFile->addAction( m_scriptextension->action("configurescripts") );
    menuFile->addAction( m_scriptextension->action("scripts") );

    QWidget* mainbox = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(mainbox);

    QGroupBox* interpretergrpbox = new QGroupBox("Interpreter", mainbox);
    layout->addWidget(interpretergrpbox);
    new QVBoxLayout(interpretergrpbox);
    m_interpretercombo = new QComboBox(interpretergrpbox);
    interpretergrpbox->layout()->addWidget(m_interpretercombo);
    m_interpretercombo->addItems( Kross::Manager::self().interpreters() );
    int i = m_interpretercombo->findText(interpretername);
    if(i != -1)
        m_interpretercombo->setCurrentIndex(i);
    else
        m_interpretercombo->setItemText(m_interpretercombo->currentIndex(), interpretername);

    QGroupBox* scriptgrpbox = new QGroupBox("Scripting code", mainbox);
    layout->addWidget(scriptgrpbox);
    new QVBoxLayout(scriptgrpbox);
    m_codeedit = new KTextEdit(scriptgrpbox);
    scriptgrpbox->layout()->addWidget(m_codeedit);
    m_codeedit->setText(m_scriptcode);
    m_codeedit->setWordWrapMode(QTextOption::NoWrap);
    m_codeedit->setTextFormat(Qt::PlainText);

    KPushButton* execbtn = new KPushButton("Execute", mainbox);
    layout->addWidget(execbtn);
    connect(execbtn, SIGNAL(clicked()), this, SLOT(execute()));

    setCentralWidget(mainbox);
    setMinimumSize(600,420);
}

TestWindow::~TestWindow()
{
}

void TestWindow::execute()
{
    m_action->setInterpreter( m_interpretercombo->currentText() );
    m_action->setCode( m_codeedit->text() );

    m_action->trigger();

#if 0
    Kross::Object::Ptr result = m_action->execute();
    if(m_action->hadException()) {
        Kross::krossdebug( QString("EXCEPTION => %1").arg(m_action->getException()->toString()) );
    }
    else {
        QString s = result ? result->toString() : QString::null;
        Kross::krossdebug( QString("DONE => %1").arg(s) );
    }
#endif
}

#include "testwindow.moc"
