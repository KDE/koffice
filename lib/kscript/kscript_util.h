#ifndef __KSCRIPT_UTIL_H__
#define __KSCRIPT_UTIL_H__

#include "kscript_value.h"

class KSContext;

class KSUtil
{
public:
  static bool checkArgumentsCount( KSContext& context, uint count, const QString& funcname, bool fatal = true );
  static bool checkType( KSContext&, KSValue* v, KSValue::Type t, bool fatal = true );
  static void castingError( KSContext& context, KSValue* v, KSValue::Type t );
  static void castingError( KSContext& context, const QString& from, const QString& to );
  static void argumentsMismatchError( KSContext& context, const QString& methodname );
  static void tooFewArgumentsError( KSContext& context, const QString& methodname );
  static void tooManyArgumentsError( KSContext& context, const QString& methodname );
};

#endif
