#include "kscript_struct.h"
#include "kscript_util.h"
#include "kscript_object.h"
#include "kscript.h"

/***************************************************
 *
 * KSStructClass
 *
 ***************************************************/

KSStructClass::KSStructClass( KSModule* m, const QString& name )
  : m_name( name ), m_module( m )
{
  m_space.insert( "isA", new KSValue( &KSStruct::isA ) );
}

bool KSStructClass::constructor( KSContext& context )
{
  context.setValue( new KSValue( constructor() ) );

  return true;
}

KSStruct* KSStructClass::constructor()
{
  return new KSStruct( this );
}

KSValue::Ptr KSStructClass::member( KSContext& context, const QString& name )
{
  KSNamespace::Iterator it = m_space.find( name );
  if ( it == m_space.end() )
  {
    QString tmp( "Unknown symbol '%1' in struct of type %2 of module '%3'" );
    context.setException( new KSException( "UnknownName", tmp.arg( name ).arg( m_name ).arg( module()->name() ) ) );
    return 0;
  }

  return it.data();
}

/***************************************************
 *
 * KSStruct
 *
 ***************************************************/

bool KSStruct::isA( KSContext& context )
{
  if ( !KSUtil::checkArgumentsCount( context, 0, "Struct::isA" ) )
    return false;

  context.setValue( new KSValue( m_class->name() ) );

  return true;
}

KSValue::Ptr KSStruct::member( KSContext& context, const QString& name )
{
  if ( context.leftExpr() )
  {
    this->ref();
    KSValue::Ptr ptr( new KSValue( new KSProperty( this, name ) ) );
    ptr->setMode( KSValue::LeftExpr );
    return ptr;
  }

  KSNamespace::Iterator it = m_space.find( name );
  if ( it != m_space.end() )
    return it.data();

  it = m_class->nameSpace()->find( name );
  if ( it != m_class->nameSpace()->end() )
    return it.data();

  QString tmp( "Unknown symbol '%1' in object of struct '%2'" );
  context.setException( new KSException( "UnknownName", tmp.arg( name ).arg( getClass()->name() ) ) );
  return 0;
}

bool KSStruct::setMember( KSContext& context, const QString& name, const KSValue::Ptr& v )
{
  if ( !m_class->vars().contains( name ) )
  {
    QString tmp( "Unknown symbol '%1' in object of struct '%2'" );
    context.setException( new KSException( "UnknownName", tmp.arg( name ).arg( getClass()->name() ) ) );
    return false;
  }

  m_space.insert( name, v );

  return true;
}

bool KSBuiltinStructClass::hasMethod( const QString& name ) const
{
    return m_methods.contains( name );
}

/***************************************************
 *
 * KSBuiltinStructClass
 *
 ***************************************************/

KSBuiltinStructClass::KSBuiltinStructClass( KSModule* module, const QString& name )
    : KSStructClass( module, name )
{
}

void KSBuiltinStructClass::addMethod( const QString& name, KSBuiltinStructClass::MethodPtr m, const QCString& signature )
{
    Method s;
    s.m_method = m;
    s.m_signature = signature;
    m_methods[ name ] = s;
}

bool KSBuiltinStructClass::call( void* object, KSContext& context, const QString& name )
{
}

/***************************************************
 *
 * KSBuiltinStruct
 *
 ***************************************************/

KSBuiltinStruct::KSBuiltinStruct( KSStructClass* c, void* object )
    : KSStruct( c )
{
    m_object = object;
}

KSBuiltinStruct::~KSBuiltinStruct()
{
    ((KSBuiltinStructClass*)getClass())->destructor( module()->interpreter()->context() );
}

KSValue::Ptr KSBuiltinStruct::member( KSContext& context, const QString& name )
{
    if ( context.leftExpr() )
    {
	this->ref();
	KSValue::Ptr ptr( new KSValue( new KSProperty( this, name ) ) );
	ptr->setMode( KSValue::LeftExpr );
	return ptr;
    }

    // Is it a method ?
    if ( ((KSBuiltinStructClass*)getClass())->hasMethod( name ) )
	return ( new KSValue( (KSStructBuiltinMethod)&KSBuiltinStruct::call ) );

    // Look in the KSStructClass namespace
    KSNamespace::Iterator it = getClass()->nameSpace()->find( name );
    if ( it != getClass()->nameSpace()->end() )
	return it.data();

    // Is it a variable ?
    if ( getClass()->hasVariable( name ) )
    {
	// Call the get method
	KSContext l( context );
	l.setValue( new KSValue( KSValue::ListType ) );
	    
	call( l, name );

	if ( l.exception() )
        {
	    context.setException( l.shareException() );
	    return 0;
	}
	
	return KSValue::Ptr( l.shareValue() );
    }
    
    QString tmp( "Unknown symbol '%1' in object of struct '%2'" );
    context.setException( new KSException( "UnknownName", tmp.arg( name ).arg( getClass()->name() ) ) );
    return 0;
}

bool KSBuiltinStruct::setMember( KSContext& context, const QString& name, const KSValue::Ptr& v )
{
    if ( !getClass()->vars().contains( name ) )
    {
	QString tmp( "Unknown symbol '%1' in object of struct '%2'" );
	context.setException( new KSException( "UnknownName", tmp.arg( name ).arg( getClass()->name() ) ) );
	return false;
    }
    
    // Call the set method
    KSContext l( context );
    l.setValue( new KSValue( KSValue::ListType ) );
    l.value()->listValue().append( v );
    
    bool b = call( l, name );

    if ( l.exception() )
    {
	context.setException( l.shareException() );
	return FALSE;
    }

    return b;
}

bool KSBuiltinStruct::call( KSContext& context, const QString& name )
{
    return ((KSBuiltinStructClass*)getClass())->call( m_object, context, name );
}

KSStruct* KSBuiltinStruct::clone()
{
    return ((KSBuiltinStructClass*)getClass())->clone();
}
