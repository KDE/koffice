#ifndef __KSCRIPT_VALUE_H
#define __KSCRIPT_VALUE_H

#include <qstring.h>
#include <qvaluelist.h>
#include <qmap.h>
#include <qshared.h>

#include "kscript_ptr.h"
#include "kscript_types.h"

class KSFunction;
class KSClass;
class KSObject;
class KSMethod;
class KSContext;
class KSProperty;
class KSModule;
class KSStruct;
class KSStructClass;
class KSProxy;
class KSInterface;
class KSAttribute;
class KSTypeCode;

typedef bool (KSObject::*KSBuiltinMethod)( KSContext& );
typedef bool (KSStruct::*KSStructBuiltinMethod)( KSContext&, const QString& );
typedef bool (KSProxy::*KSProxyBuiltinMethod)( KSContext&, const QString& );

/**
 * This class acts like a union. It can hold one value at the
 * time and it can hold the most common types.
 * For CORBA people: It is a poor mans CORBA::Any.
 */
class KSValue : public QShared
{
public:
    typedef KSSharedPtr<KSValue> Ptr;

    enum Type {
      Empty,
      StringType,
      IntType,
      BoolType,
      DoubleType,
      ListType,
      MapType,
      CharType,
      CharRefType,
      FunctionType,
      ClassType,
      ObjectType,
      MethodType,
      BuiltinMethodType,
      PropertyType,
      ModuleType,
      StructType,
      StructClassType,
      StructBuiltinMethodType,
      ProxyType,
      InterfaceType,
      AttributeType,
      TypeCodeType,
      ProxyBuiltinMethodType,
      NTypes
    };

    enum Mode {
      LeftExpr,
      Constant,
      Temp
    };

    KSValue();
    KSValue( Type );
    KSValue( const KSValue& );
    virtual ~KSValue();

    KSValue( const QString& _v ) { m_mode = Temp; typ = Empty; setValue( _v ); }
    KSValue( const QValueList<Ptr>& _v ) { m_mode = Temp; typ = Empty; setValue( _v ); }
    KSValue( const QMap<QString,Ptr>& _v ) { m_mode = Temp; typ = Empty; setValue( _v ); }
    KSValue( KScript::Long _v ) { m_mode = Temp; typ = Empty; setValue( _v ); }
    KSValue( int _v ) { m_mode = Temp; typ = Empty; setValue( (KScript::Long)_v ); }
    KSValue( KScript::Boolean _v ) { m_mode = Temp; typ = Empty; setValue( _v ); }
    KSValue( KScript::Double _v ) { m_mode = Temp; typ = Empty; setValue( _v ); }
    KSValue( const KScript::Char& _v ) { m_mode = Temp; typ = Empty; setValue( _v ); }
    KSValue( const KScript::CharRef& _v ) { m_mode = Temp; typ = Empty; setValue( _v ); }
    KSValue( KSFunction* _v ) { m_mode = Temp; typ = Empty; setValue( _v ); }
    KSValue( KSClass* _v ) { m_mode = Temp; typ = Empty; setValue( _v ); }
    KSValue( KSObject* _v ) { m_mode = Temp; typ = Empty; setValue( _v ); }
    KSValue( KSMethod* _v ) { m_mode = Temp; typ = Empty; setValue( _v ); }
    KSValue( KSBuiltinMethod _v ) { m_mode = Temp; typ = Empty; setValue( _v ); }
    KSValue( KSProperty* _v ) { m_mode = Temp; typ = Empty; setValue( _v ); }
    KSValue( KSModule* _v ) { m_mode = Temp; typ = Empty; setValue( _v ); }
    KSValue( KSStruct* _v ) { m_mode = Temp; typ = Empty; setValue( _v ); }
    KSValue( KSStructClass* _v ) { m_mode = Temp; typ = Empty; setValue( _v ); }
    KSValue( KSStructBuiltinMethod _v ) { m_mode = Temp; typ = Empty; setValue( _v ); }
    KSValue( KSProxy* _v ) { m_mode = Temp; typ = Empty; setValue( _v ); }
    KSValue( KSInterface* _v ) { m_mode = Temp; typ = Empty; setValue( _v ); }
    KSValue( KSProxyBuiltinMethod _v ) { m_mode = Temp; typ = Empty; setValue( _v ); }
    KSValue( KSTypeCode* _v ) { m_mode = Temp; typ = Empty; setValue( _v ); }
    KSValue( KSAttribute* _v ) { m_mode = Temp; typ = Empty; setValue( _v ); }

    KSValue& operator= ( const KSValue& );

    void setValue( const QString& );
    void setValue( const QValueList<Ptr>& );
    void setValue( const QMap<QString,Ptr>& );
    void setValue( int _v ) { setValue( (KScript::Long)_v ); }
    void setValue( KScript::Long );
    void setValue( KScript::Boolean );
    void setValue( KScript::Double );
    void setValue( const KScript::Char& );
    void setValue( const KScript::CharRef& );
    void setValue( KSFunction* );
    void setValue( KSClass* );
    void setValue( KSObject* );
    void setValue( KSMethod* );
    void setValue( KSBuiltinMethod );
    void setValue( KSProperty* );
    void setValue( KSModule* );
    void setValue( KSStruct* );
    void setValue( KSStructClass* );
    void setValue( KSStructBuiltinMethod );
    void setValue( KSProxy* );
    void setValue( KSInterface* );
    void setValue( KSProxyBuiltinMethod );
    void setValue( KSTypeCode* );
    void setValue( KSAttribute* );

    void suck( KSValue* );

    Mode mode() const { return m_mode; }
    void setMode( Mode m ) { m_mode = m; }

    Type type() const { return typ; }
    virtual QString typeName() const;

    bool isEmpty() const { return ( typ == Empty ); }

    const QString& stringValue() const { ASSERT( typ == StringType ); return *((QString*)val.ptr); }
    QString& stringValue() { ASSERT( typ == StringType ); return *((QString*)val.ptr); }
    const QValueList<Ptr>& listValue() const { ASSERT( typ == ListType );  return *((QValueList<Ptr>*)val.ptr); }
    QValueList<Ptr>& listValue() { ASSERT( typ == ListType );  return *((QValueList<Ptr>*)val.ptr); }
    const QMap<QString,Ptr>& mapValue() const { ASSERT( typ == MapType ); return *((QMap<QString,Ptr>*)val.ptr); }
    QMap<QString,Ptr>& mapValue() { ASSERT( typ == MapType ); return *((QMap<QString,Ptr>*)val.ptr); }
    KScript::Long intValue() const { ASSERT( typ == IntType ); return val.i; }
    KScript::Boolean boolValue() const { ASSERT( typ == BoolType ); return val.b; }
    KScript::Double doubleValue() const { ASSERT( typ == DoubleType ); return val.d; }
    const KScript::Char charValue() const { if ( typ == CharRefType ) return *((KScript::CharRef*)val.ptr);
                                            ASSERT( typ == CharType ); return QChar( val.c ); }
    KScript::CharRef& charRefValue() { ASSERT( typ == CharRefType ); return *((KScript::CharRef*)val.ptr); }
    const KScript::CharRef& charRefValue() const { ASSERT( typ == CharRefType ); return *((KScript::CharRef*)val.ptr); }
    KSFunction* functionValue() { ASSERT( typ == FunctionType ); return ((KSFunction*)val.ptr); }
    const KSFunction* functionValue() const { ASSERT( typ == FunctionType ); return ((KSFunction*)val.ptr); }
    KSClass* classValue() { ASSERT( typ == ClassType ); return ((KSClass*)val.ptr); }
    const KSClass* classValue() const { ASSERT( typ == ClassType ); return ((KSClass*)val.ptr); }
    KSObject* objectValue() { ASSERT( typ == ObjectType ); return ((KSObject*)val.ptr); }
    const KSObject* objectValue() const { ASSERT( typ == ObjectType ); return ((KSObject*)val.ptr); }
    KSMethod* methodValue() { ASSERT( typ == MethodType ); return ((KSMethod*)val.ptr); }
    const KSMethod* methodValue() const { ASSERT( typ == MethodType ); return ((KSMethod*)val.ptr); }
    KSBuiltinMethod builtinMethodValue() { ASSERT( typ == BuiltinMethodType ); return val.m; }
    KSProperty* propertyValue() { ASSERT( typ == PropertyType ); return ((KSProperty*)val.ptr); }
    const KSProperty* propertyValue() const { ASSERT( typ == PropertyType ); return ((KSProperty*)val.ptr); }
    KSModule* moduleValue() { ASSERT( typ == ModuleType ); return ((KSModule*)val.ptr); }
    const KSModule* moduleValue() const { ASSERT( typ == ModuleType ); return ((KSModule*)val.ptr); }
    KSStructClass* structClassValue() { ASSERT( typ == StructClassType ); return ((KSStructClass*)val.ptr); }
    const KSStructClass* structClassValue() const { ASSERT( typ == StructClassType ); return ((KSStructClass*)val.ptr); }
    KSStruct* structValue() { ASSERT( typ == StructType ); return ((KSStruct*)val.ptr); }
    const KSStruct* structValue() const { ASSERT( typ == StructType ); return ((KSStruct*)val.ptr); }
    KSStructBuiltinMethod structBuiltinMethodValue() { ASSERT( typ == StructBuiltinMethodType ); return val.sm; }
    KSInterface* interfaceValue() { ASSERT( typ == InterfaceType ); return ((KSInterface*)val.ptr); }
    const KSInterface* interfaceValue() const { ASSERT( typ == InterfaceType ); return ((KSInterface*)val.ptr); }
    KSProxy* proxyValue() { ASSERT( typ == ProxyType ); return ((KSProxy*)val.ptr); }
    const KSProxy* proxyValue() const { ASSERT( typ == ProxyType ); return ((KSProxy*)val.ptr); }
    KSProxyBuiltinMethod proxyBuiltinMethodValue() { ASSERT( typ == ProxyBuiltinMethodType ); return val.pm; }
    KSTypeCode* typeCodeValue() { ASSERT( typ == TypeCodeType ); return ((KSTypeCode*)val.ptr); }
    const KSTypeCode* typeCodeValue() const { ASSERT( typ == TypeCodeType ); return ((KSTypeCode*)val.ptr); }
    KSAttribute* attributeValue() { ASSERT( typ == AttributeType ); return ((KSAttribute*)val.ptr); }
    const KSAttribute* attributeValue() const { ASSERT( typ == AttributeType ); return ((KSAttribute*)val.ptr); }

    bool cast( Type );

    QString toString() const;

    bool operator==( const KSValue& v ) const;

    bool cmp( const KSValue& v ) const;

    /**
     * Frees all data allocated by this KSValue.
     */
    void clear();

    static QString typeToName( Type _typ );
    /**
     * @return KSValue::Empty if the given name is empty or unknown.
     */
    static Type nameToType( const QString& _name );

    /**
     * @return an empty value. Its reference count is increased so that you can assign
     *         it directly to some @ref KSContext.
     */
    static KSValue* null() { if ( !s_null ) s_null = new KSValue; s_null->ref(); return s_null; }

protected:

    Mode m_mode;
    Type typ;
    union
    {
      KScript::Long i;
      KScript::Boolean b;
      KScript::Double d;
      ushort c;
      void *ptr;
      KSBuiltinMethod m;
      KSStructBuiltinMethod sm;
      KSProxyBuiltinMethod pm;
    } val;

private:
    static void initTypeNameMap();
    static KSValue* s_null;
};

#endif

