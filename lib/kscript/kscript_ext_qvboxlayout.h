#ifndef __KSCRIPT_EXT_QVBOXLAYOUT_H__
#define __KSCRIPT_EXT_QVBOXLAYOUT_H__

#include "kscript_class.h"
#include "kscript_ext_qwidget.h"

class KSContext;

class QVBoxLayout;

class KSClass_QVBoxLayout : public KSClass_QObject
{
public:
  KSClass_QVBoxLayout( KSModule*, const char* name = "QVBoxLayout" );

protected:
  virtual KSScriptObject* createObject( KSClass* c );
};

class KSObject_QVBoxLayout : public KS_Qt_Object
{
public:
  KSObject_QVBoxLayout( KSClass* );

  bool ksQVBoxLayout( KSContext& );
  bool ksQVBoxLayout_addWidget( KSContext& );

  KSValue::Ptr member( KSContext& context, const QString& name );
  bool setMember( KSContext& context, const QString& name, const KSValue::Ptr& v );

  bool inherits( const char* name ) { return ( strcmp( name, "KSObject_QVBoxLayout" ) == 0 || KS_Qt_Object::inherits( name ) ); }

  static QVBoxLayout* convert( KSValue* v ) { return (QVBoxLayout*) ((KS_Qt_Object*)v->objectValue())->object(); }
};

#endif
