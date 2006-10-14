/***************************************************************************
 * action.cpp
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

#include "action.h"
#include "interpreter.h"
#include "script.h"
#include "manager.h"

#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QDomElement>

#include <klocale.h>
#include <kicon.h>
#include <kmimetype.h>
#include <kstandarddirs.h>

using namespace Kross;

namespace Kross {

    /// @internal
    class ActionPrivate
    {
        public:

            /**
            * The \a Script instance the \a Action uses if initialized. It will
            * be NULL as long as we didn't initialized it what will be done on
            * demand.
            */
            Script* script;

            /**
            * The optional description to provide some more details about the
            * Action to the user.
            */
            QString description;

            /**
            * The scripting code.
            */
            QString code;

            /**
            * The name of the interpreter. This could be something
            * like for example "python" for the python binding.
            */
            QString interpretername;

            /**
            * The name of the scriptfile that should be executed. Those
            * scriptfile will be readed and the content will be used to
            * set the scripting code and, if not defined, the used
            * interpreter.
            */
            KUrl scriptfile;

            /**
            * The current path the \a Script is running in or
            * an empty string if there is no path current defined.
            */
            QString currentpath;

            /**
            * Map of options that overwritte the \a InterpreterInfo::Option::Map
            * standard options.
            */
            QMap<QString, QVariant> options;

            ActionPrivate() : script(0) {}
    };

}

Action::Action(const QString& name)
    : KAction( 0 /* no parent KActionCollection */, name )
    , KShared()
    , ChildrenInterface()
    , ErrorInterface()
    , d( new ActionPrivate() )
{
    //krossdebug( QString("Action::Action() Ctor name='%1'").arg(objectName()) );
    //connect(this, SIGNAL(triggered(bool)), this, SLOT(slotTriggered()));
    //connect(this, SIGNAL(triggered(Qt::MouseButtons,Qt::KeyboardModifiers)), this, SLOT(slotTriggered()));
}

Action::Action(const KUrl& file)
    : KAction( 0 /* no parent KActionCollection */, file.path() )
    , KShared()
    , ChildrenInterface()
    , ErrorInterface()
    , d( new ActionPrivate() )
{
    d->scriptfile = file;
    d->currentpath = file.directory();
    setText(file.fileName());
    setIcon( KIcon( KMimeType::iconNameForUrl(file) ) );
}

Action::Action(KActionCollection* collection, const QString& name, const KUrl& file)
    : KAction( collection, name )
    , KShared()
    , ChildrenInterface()
    , ErrorInterface()
    , d( new ActionPrivate() )
{
    d->scriptfile = file;
    d->currentpath = file.directory();
}

Action::Action(KActionCollection* collection, const QDomElement& element, const QDir& packagepath)
    : KAction( collection, element.attribute("name") )
    , KShared()
    , ChildrenInterface()
    , ErrorInterface()
    , d( new ActionPrivate() )
{
    setText( element.attribute("text") );
    setDescription( element.attribute("description") );
    setInterpreter( element.attribute("interpreter") );

    QString file = element.attribute("file");
    if(! file.isEmpty()) {
        if(! QFileInfo(file).exists()) {
            QFileInfo fi(packagepath, file);
            if(fi.exists())
                file = fi.absoluteFilePath();
            else
                setEnabled(false);
        }
        d->scriptfile = KUrl(file);
        d->currentpath = d->scriptfile.directory();
    }
    else {
        d->currentpath = packagepath.absolutePath();
    }

    QString icon = element.attribute("icon");
    if(icon.isEmpty() && d->scriptfile.isValid())
        icon = KMimeType::iconNameForUrl(d->scriptfile);
    setIcon( KIcon(icon) );
}

Action::~Action()
{
    //krossdebug( QString("Action::~Action() Dtor name='%1'").arg(objectName()) );
    finalize();
    delete d;
}

QString Action::description() const
{
    return d->description;
}

void Action::setDescription(const QString& description)
{
    d->description = description;
}

QString Action::code() const
{
    return d->code;
}

void Action::setCode(const QString& code)
{
    finalize();
    d->code = code;
}

QString Action::interpreter() const
{
    return d->interpretername;
}

void Action::setInterpreter(const QString& interpretername)
{
    finalize();
    d->interpretername = interpretername;
}

KUrl Action::getFile() const
{
    return d->scriptfile;
}

QString Action::currentPath() const
{
    return d->currentpath;
}

QMap<QString, QVariant>& Action::getOptions()
{
    return d->options;
}

QVariant Action::getOption(const QString name, QVariant defaultvalue, bool /*recursive*/)
{
    if(d->options.contains(name))
        return d->options[name];
    InterpreterInfo* info = Manager::self().interpreterInfo( d->interpretername );
    return info ? info->getOptionValue(name, defaultvalue) : defaultvalue;
}

bool Action::setOption(const QString name, const QVariant& value)
{
    InterpreterInfo* info = Manager::self().interpreterInfo( d->interpretername );
    if(info) {
        if(info->hasOption(name)) {
            d->options.insert(name, value);
            return true;
        } else krosswarning( QString("Kross::Action::setOption(%1, %2): No such option").arg(name).arg(value.toString()) );
    } else krosswarning( QString("Kross::Action::setOption(%1, %2): No such interpreterinfo").arg(name).arg(value.toString()) );
    return false;
}

QStringList Action::functionNames()
{
    if(! d->script) {
        if(! initialize())
            return QStringList();
    }
    return d->script->functionNames();
}

QVariant Action::callFunction(const QString& name, const QVariantList& args)
{
    if(! d->script) {
        if(! initialize())
            return QVariant();
    }
    return d->script->callFunction(name, args);
}

#if 0
const QStringList getFunctionNames()
{
    return d->script ? d->script->getFunctionNames() : QStringList(); //FIXME init before if needed?
}
Object::Ptr Action::callFunction(const QString& functionname, List::Ptr arguments)
{
    if(! d->script)
        if(! initialize())
            return Object::Ptr();
    if(hadException())
        return Object::Ptr();
    if(functionname.isEmpty()) {
        setException( new Exception(QString(i18n("No functionname defined for Action::callFunction()."))) );
        finalize();
        return Object::Ptr();
    }
    Object::Ptr r = d->script->callFunction(functionname, arguments);
    if(d->script->hadException()) {
        setException( d->script->getException() );
        finalize();
        return Object::Ptr();
    }
    return r;
}
QStringList Action::getClassNames()
{
    return d->script ? d->script->getClassNames() : QStringList(); //FIXME init before if needed?
}
Object::Ptr Action::classInstance(const QString& classname)
{
    if(! d->script)
        if(! initialize())
            return Object::Ptr();
    if(hadException())
        return Object::Ptr();
    Object::Ptr r = d->script->classInstance(classname);
    if(d->script->hadException()) {
        setException( d->script->getException() );
        finalize();
        return Object::Ptr();
    }
    return r;
}
#endif

bool Action::initialize()
{
    finalize();

    if(d->scriptfile.isValid()) {
        const QString file = d->scriptfile.toLocalFile();
        QFile f(file);
        if(! f.exists()) {
            setError(i18n("There exists no such scriptfile '%1'",file));
            return false;
        }

        krossdebug( QString("Kross::Action::initialize() file=%1").arg(file) );
        if(d->interpretername.isNull()) {
            d->interpretername = Manager::self().interpreternameForFile(file);
            if(d->interpretername.isNull()) {
                setError(i18n("Failed to determinate interpreter for scriptfile '%1'",file));
                return false;
            }
        }

        if(! f.open(QIODevice::ReadOnly)) {
            setError(i18n("Failed to open scriptfile '%1'",file));
            return false;
        }
        d->code = QString( f.readAll() );
        f.close();
    }

    Interpreter* interpreter = Manager::self().interpreter(d->interpretername);
    if(! interpreter) {
        setError(i18n("Unknown interpreter '%1'",d->interpretername));
        return false;
    }

    d->script = interpreter->createScript(this);
    if(! d->script) {
        setError(i18n("Failed to create script for interpreter '%1'",d->interpretername));
        return false;
    }

    if( d->script->hadError() ) {
        setError(d->script);
        finalize();
        return false;
    }

    clearError(); // clear old exception
    return true;
}

void Action::finalize()
{
    delete d->script;
    d->script = 0;
}

void Action::slotTriggered()
{
    //krossdebug( QString("Action::slotTriggered()") );
    emit activated(this);

    if(! d->script) {
        if(! initialize())
            Q_ASSERT( hadError() );
    }

    if(hadError()) {
        krossdebug( QString("Action::slotTriggered() name=%1 errorMessage=%2").arg(objectName()).arg(errorMessage()) );
        emit failed(errorMessage(), errorTrace());
        return;
    }

    //what to do with data() ?

    d->script->execute();
    if(d->script->hadError()) {
        setError(d->script);
        finalize();
        emit failed(errorMessage(), errorTrace());
    }
    else {
        clearError();
        emit success();
    }
}

#include "action.moc"
