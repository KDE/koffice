#include "kscript_ext_qt.h"
#include "kscript_ext_qwidget.h"
#include "kscript_ext_qdialog.h"
#include "kscript_ext_qapplication.h"
#include "kscript_ext_qlineedit.h"
#include "kscript_ext_qbutton.h"
#include "kscript_ext_qcheckbox.h"
#include "kscript_ext_qradiobutton.h"
#include "kscript_ext_qpushbutton.h"
#include "kscript_ext_qcombobox.h"
#include "kscript_ext_qlabel.h"
#include "kscript_ext_qboxlayout.h"
#include "kscript_ext_qvboxlayout.h"
#include "kscript_ext_qhboxlayout.h"
#include "kscript_ext_qbuttongroup.h"
#include "kscript_ext_qstructs.h"
#include "kscript_value.h"
#include "kscript_util.h"
#include "kscript.h"

#undef CHECKTYPE
#define CHECKTYPE( context, v, type ) if ( !checkType( context, v, type ) ) return FALSE;

KSModule::Ptr ksCreateModule_Qt( KSInterpreter* interp )
{
  KSModule::Ptr module = new KSModule( interp, "qt" );

  // Add all Qt classes to the module
  module->addObject( "QWidget", new KSValue( new KSClass_QWidget( module ) ) );
  module->addObject( "QDialog", new KSValue( new KSClass_QDialog( module ) ) );
  module->addObject( "QApplication", new KSValue( new KSClass_QApplication( module ) ) );
  module->addObject( "QLineEdit", new KSValue( new KSClass_QLineEdit( module ) ) );
  module->addObject( "QButton", new KSValue( new KSClass_QButton( module ) ) );
  module->addObject( "QRadioButton", new KSValue( new KSClass_QRadioButton( module ) ) );
  module->addObject( "QPushButton", new KSValue( new KSClass_QPushButton( module ) ) );
  module->addObject( "QCheckBox", new KSValue( new KSClass_QCheckBox( module ) ) );
  module->addObject( "QComboBox", new KSValue( new KSClass_QComboBox( module ) ) );
  module->addObject( "QLabel", new KSValue( new KSClass_QLabel( module ) ) );
  module->addObject( "QBoxLayout", new KSValue( new KSClass_QBoxLayout( module ) ) );
  module->addObject( "QVBoxLayout", new KSValue( new KSClass_QVBoxLayout( module ) ) );
  module->addObject( "QHBoxLayout", new KSValue( new KSClass_QHBoxLayout( module ) ) );
  module->addObject( "QButtonGroup", new KSValue( new KSClass_QButtonGroup( module ) ) );
  module->addObject( "QRect", new KSValue( new KSQt::Rect( module, "QRect" ) ) );
  module->addObject( "QPoint", new KSValue( new KSQt::Point( module, "QPoint" ) ) );
  module->addObject( "QSize", new KSValue( new KSQt::Size( module, "QSize" ) ) );
    
  return module;
}

/**********************************************
 *
 * KS_Qt_Callback
 *
 **********************************************/

KS_Qt_Callback* KS_Qt_Callback::s_pSelf = 0;

KS_Qt_Callback* KS_Qt_Callback::self()
{
  if ( !s_pSelf )
    s_pSelf = new KS_Qt_Callback;
  return s_pSelf;
}

void KS_Qt_Callback::connect( QObject* s, KS_Qt_Object* r )
{
  DestroyCallback c;
  c.m_sender = s;
  c.m_receiver = r;
  m_callbacks.append( c );
  QObject::connect( s, SIGNAL( destroyed() ), this, SLOT( slotDestroyed() ) );
}

void KS_Qt_Callback::connect( QObject* s, const char* qt_sig, const char* qt_slot,
			      KSObject* r, const char* ks_sig )
{
  Connection c;
  c.m_sender = s;
  c.m_receiver = r;
  c.m_kscriptSignal = ks_sig;
  m_connections.append( c );

  QObject::connect( s, qt_sig, this, qt_slot );
}

void KS_Qt_Callback::disconnect( KSObject* r )
{
  QValueList<Connection>::Iterator it =  m_connections.begin();
  while( it != m_connections.end() )
  {
    if ( r == (*it).m_receiver )
      it = m_connections.remove( it );
    else
      ++it;
  }

  QValueList<DestroyCallback>::Iterator it2 = m_callbacks.begin();
  while( it2 != m_callbacks.end() )
  {
    if ( r == (*it2).m_receiver )
    {
	(*it2).m_sender->disconnect( SIGNAL( destroyed() ), this, SLOT( slotDestroyed() ) );
	it2 = m_callbacks.remove( it2 );
    }
    else
      ++it2;
  }
}

void KS_Qt_Callback::slotDestroyed()
{
  QObject *s = (QObject*)sender();

  QValueList<DestroyCallback>::Iterator it = m_callbacks.begin();
  while( it != m_callbacks.end() )
  {
      if ( s == (*it).m_sender && (*it).m_receiver->status() == KSObject::Alive )
      {
	  qDebug("QObject %x sent kill signal", (int)s);
	  (*it).m_receiver->setObject( 0 );
	  (*it).m_receiver->kill();
	  // Try again. The problem is that ->kill() causes our list
	  // to be modified somewhere. So we trust no one (tm) and start again
	  // from the very beginning.
	  // PS: If you dont know what (tm) means, you have to watch TV :-)
	  it = m_callbacks.begin();
      }
      else
	  ++it;
  }
}

void KS_Qt_Callback::emitSignal( const QValueList<KSValue::Ptr>& params, const char* name )
{
  QValueList<Connection>::Iterator it =  m_connections.begin();
  while( it != m_connections.end() )
  {
    if ( sender() == (*it).m_sender && (*it).m_kscriptSignal == name )
    {
	// Get context
	KSContext& context = (*it).m_receiver->getClass()->module()->interpreter()->context();
	KSModule* module = context.scope()->popModule();

	// Switch to our modules namespace
	context.scope()->pushModule( (*it).m_receiver->getClass()->module() );

	context.setValue( new KSValue( params ) );

	(*it).m_receiver->emitSignal( (*it).m_kscriptSignal, context );

	context.scope()->popModule();
	context.scope()->pushModule( module );
    }
    ++it;
  }
}

void KS_Qt_Callback::textChanged( const QString& param1 )
{
    QValueList<KSValue::Ptr> params;
    params.append( new KSValue( param1 ) );

    emitSignal( params, "textChanged" );
}

void KS_Qt_Callback::clicked()
{
    QValueList<KSValue::Ptr> params;

    emitSignal( params, "clicked" );
}

void KS_Qt_Callback::activated( int param1 )
{
    QValueList<KSValue::Ptr> params;
    params.append( new KSValue( param1 ) );
    emitSignal( params, "activated");
}

void KS_Qt_Callback::activated( const QString& param1 )
{
    QValueList<KSValue::Ptr> params;
    params.append( new KSValue( param1 ) );
    emitSignal( params, "activated");
}


/**********************************************
 *
 * KS_Qt_Object
 *
 **********************************************/

KS_Qt_Object::KS_Qt_Object( KSClass* c )
    : KSScriptObject( c )
{
    m_object = 0;
    // QObject objects hold a reference on
    ref();
}

KS_Qt_Object::~KS_Qt_Object()
{
    qDebug("KS_Qt_Object::~KS_Qt_Object");
  if ( status() == Alive )
    destructor();
}

void KS_Qt_Object::setObject( QObject* o, bool ownership )
{
  if ( m_object )
    KS_Qt_Callback::self()->disconnect( this );

  m_object = o;
  m_ownership = ownership;

  if ( m_object )
    KS_Qt_Callback::self()->connect( m_object, this );
}

bool KS_Qt_Object::destructor()
{
    if ( status() == Dead )
	return TRUE;

    qDebug("KS_Qt_Object::destructor");
    bool b = KSScriptObject::destructor();

    KS_Qt_Callback::self()->disconnect( this );

    if ( m_object )
    {
	if ( m_ownership )
        {
	    qDebug("Deleting %x and widget %x %s",(int)this,(int)m_object,m_object->className());
	    delete m_object;
	}
	m_object = 0;
    }

    // In the constructor there is an extra reference count.
    // That is dropped upon calling "destroy".
    if ( deref() )
	delete this;

    return b;
}

KSValue::Ptr KS_Qt_Object::member( KSContext& context, const QString& name )
{
  KSValue::Ptr ptr;
  if ( name == "name" )
    ptr = new KSValue( QString( m_object->name() ) );

  if ( ptr )
  {
    ptr->setMode( KSValue::LeftExpr );
    return ptr;
  }

  return KSObject::member( context, name );
}

bool KS_Qt_Object::setMember( KSContext& context, const QString& name, const KSValue::Ptr& v )
{
  if ( name == "name" )
  {
    CHECKTYPE( context, v, StringType );
    m_object->setName( v->stringValue() );
    return TRUE;
  }

  return KSObject::setMember( context, name, v );
}

bool KS_Qt_Object::KSQObject_destroy( KSContext& context )
{
  qDebug("QObject::delete\n");

  if ( !KSUtil::checkArgumentsCount( context, 0, "QObject::delete" ) )
  {
      if ( deref() ) delete this;
      return false;
  }

  KS_Qt_Callback::self()->disconnect( this );

  if ( object() )
  {
      delete object();
      setObject( 0 );
  }

  // In the constructor there is an extra reference count.
  // That is dropped upon calling "destroy".
  if ( deref() ) delete this;
	
  return true;
}

/***********************************************
 *
 * Helper
 *
 ***********************************************/

bool KS_Qt_Object::checkArguments( KSContext& context, KSValue* v, const QString& name, KS_Qt_Object::Type t1,
				   KS_Qt_Object::Type t2, KS_Qt_Object::Type t3,
				   KS_Qt_Object::Type t4, KS_Qt_Object::Type t5, KS_Qt_Object::Type t6 )
{
  QValueList<KSValue::Ptr>::Iterator it = v->listValue().begin();
  QValueList<KSValue::Ptr>::Iterator end = v->listValue().end();

  if ( t1 != NoType )
  {
    if ( it == end )
    {
      KSUtil::tooFewArgumentsError( context, name );
      return false;
    }
    if ( !checkType( context, *it, t1, true ) )
      return false;
    ++it;
  }
  if ( t2 != NoType )
  {
    if ( it == end )
    {
      KSUtil::tooFewArgumentsError( context, name );
      return false;
    }
    if ( !checkType( context, *it, t2, true ) )
      return false;
    ++it;
  }
  if ( t3 != NoType )
  {
    if ( it == end )
    {
      KSUtil::tooFewArgumentsError( context, name );
      return false;
    }
    if ( !checkType( context, *it, t3, true ) )
      return false;
    ++it;
  }
  if ( t4 != NoType )
  {
    if ( it == end )
    {
      KSUtil::tooFewArgumentsError( context, name );
      return false;
    }
    if ( !checkType( context, *it, t4, true ) )
      return false;
    ++it;
  }
  if ( t5 != NoType )
  {
    if ( it == end )
    {
      KSUtil::tooFewArgumentsError( context, name );
      return false;
    }
    if ( !checkType( context, *it, t5, true ) )
      return false;
    ++it;
  }
  if ( t6 != NoType )
  {
    if ( it == end )
    {
      KSUtil::tooFewArgumentsError( context, name );
      return false;
    }
    if ( !checkType( context, *it, t6, true ) )
      return false;
    ++it;
  }

  // Too many parameters ?
  if ( it != end )
  {
    KSUtil::tooManyArgumentsError( context, name );
    return false;
  }

  return true;
}

bool KS_Qt_Object::tryArguments( KSContext& context, KSValue* v, KS_Qt_Object::Type t1,
				 KS_Qt_Object::Type t2, KS_Qt_Object::Type t3,
				 KS_Qt_Object::Type t4, KS_Qt_Object::Type t5, KS_Qt_Object::Type t6 )
{
  QValueList<KSValue::Ptr>::Iterator it = v->listValue().begin();
  QValueList<KSValue::Ptr>::Iterator end = v->listValue().end();

  if ( t1 != NoType )
  {
    if ( it == end )
      return false;
    if ( !checkType( context, *it, t1, false ) );
      return false;
    ++it;
  }
  if ( t2 != NoType )
  {
    if ( it == end )
      return false;
    if ( !checkType( context, *it, t2, false ) );
      return false;
    ++it;
  }
  if ( t3 != NoType )
  {
    if ( it == end )
      return false;
    if ( !checkType( context, *it, t3, false ) );
      return false;
    ++it;
  }
  if ( t4 != NoType )
  {
    if ( it == end )
      return false;
    if ( !checkType( context, *it, t4, false ) );
      return false;
    ++it;
  }
  if ( t5 != NoType )
  {
    if ( it == end )
      return false;
    if ( !checkType( context, *it, t5, false ) );
      return false;
    ++it;
  }
  if ( t6 != NoType )
  {
    if ( it == end )
      return false;
    if ( !checkType( context, *it, t6, false ) );
      return false;
    ++it;
  }

  // Too many parameters ?
  if ( it != end )
    return false;

  return true;
}

bool KS_Qt_Object::checkType( KSContext& context, KSValue* v, KS_Qt_Object::Type type, bool _fatal )
{
  switch( type )
  {
  case StringType:
    return KSUtil::checkType( context, v, KSValue::StringType, _fatal );
  case IntType:
    return KSUtil::checkType( context, v, KSValue::IntType, _fatal );
  case BoolType:
    return KSUtil::checkType( context, v, KSValue::BoolType, _fatal );
  case DoubleType:
    return KSUtil::checkType( context, v, KSValue::DoubleType, _fatal );
  case RectType:
    if ( !KSUtil::checkType( context, v, KSValue::StructType, _fatal ) )
      return false;
    if ( v->structValue()->getClass()->name() == "QRect" )
      return true;
    KSUtil::castingError( context, "Object", "QRect" );
    return false;
  case ObjectType:
    if ( !KSUtil::checkType( context, v, KSValue::ObjectType, _fatal ) )
      return false;
    if ( v->objectValue()->inherits( "KS_Qt_Object" ) )
      return true;
    KSUtil::castingError( context, "Object", "QObject" );
    return false;
  case WidgetType:
    if ( !KSUtil::checkType( context, v, KSValue::ObjectType, _fatal ) )
      return false;
    if ( v->objectValue()->inherits( "KSObject_QWidget" ) )
      return true;
    KSUtil::castingError( context, "Object", "QWidget" );
    return false;
  case NoType:
    ASSERT( 0 );
  }

  // never reached
  return false;
}

bool KS_Qt_Object::checkDoubleConstructor( KSContext& context, const QString& name )
{
  if ( m_object )
  {
    QString tmp( "The constructor of the class %1 was called twice." );
    context.setException( new KSException( "ConstructorCalledTwice", tmp.arg( name ), -1 ) );
    return false;
  }

  return true;
}

bool KS_Qt_Object::checkLive( KSContext& context, const QString& name )
{
  if ( !m_object )
  {
    QString tmp( "The method %1 was called before the constructor or after the destructor" );
    context.setException( new KSException( "MethodCalledBeforeConstructor", tmp.arg( name ), -1 ) );
    return false;
  }

  return true;
}

/**********************************************
 *
 * KSClass_QObject
 *
 **********************************************/

KSClass_QObject::KSClass_QObject( KSModule* m, const char* name ) : KSScriptClass( m, name, 0 )
{
    nameSpace()->insert( "destroy", new KSValue( (KSBuiltinMethod)&KS_Qt_Object::KSQObject_destroy ) );
}

bool KSClass_QObject::hasSignal( const QString& name )
{
    if ( m_signals.contains( name ) )
	return TRUE;

    return KSScriptClass::hasSignal( name );
}

void KSClass_QObject::addQtSignal( const QString& str )
{
    m_signals.append( str );
}

#include "kscript_ext_qt.moc"

#undef CHECKTYPE
