#include "kspread_util.h"
#include "kspread_doc.h"
#include "kspread_table.h"

#include <koscript_parser.h>
#include <koscript_util.h>
#include <koscript_func.h>
#include <koscript_synext.h>

#include <stdlib.h>
#include <math.h>
#include <float.h>

#include <kdebug.h>

// defined in kspread_functions_trig.cc
bool kspreadfunc_sin( KSContext& context );
bool kspreadfunc_cos( KSContext& context );
bool kspreadfunc_tan( KSContext& context );
bool kspreadfunc_atan( KSContext& context );
bool kspreadfunc_asin( KSContext& context );
bool kspreadfunc_acos( KSContext& context );
bool kspreadfunc_asinh( KSContext& context );
bool kspreadfunc_acosh( KSContext& context );
bool kspreadfunc_atanh( KSContext& context );
bool kspreadfunc_tanh( KSContext& context );
bool kspreadfunc_sinh( KSContext& context );
bool kspreadfunc_cosh( KSContext& context );
bool kspreadfunc_degree( KSContext& context );
bool kspreadfunc_radian( KSContext& context );
bool kspreadfunc_PI( KSContext& context );
bool kspreadfunc_atan2( KSContext& context );

// defined in kspread_functions_math.cc
bool kspreadfunc_sqrt( KSContext& context );
bool kspreadfunc_sqrtn( KSContext& context );
bool kspreadfunc_cur( KSContext& context );
bool kspreadfunc_fabs( KSContext& context );
bool kspreadfunc_exp( KSContext& context );
bool kspreadfunc_ceil( KSContext& context );
bool kspreadfunc_floor( KSContext& context );
bool kspreadfunc_ln( KSContext& context );
bool kspreadfunc_logn( KSContext& context );
bool kspreadfunc_log( KSContext& context );
bool kspreadfunc_sum( KSContext& context );
bool kspreadfunc_product( KSContext& context );
bool kspreadfunc_div( KSContext& context );
bool kspreadfunc_sumsq( KSContext& context );
bool kspreadfunc_max( KSContext& context );
bool kspreadfunc_lcd( KSContext & context );
bool kspreadfunc_lcm( KSContext & context );
bool kspreadfunc_min( KSContext& context );
bool kspreadfunc_mult( KSContext& context );
bool kspreadfunc_INT( KSContext& context );
bool kspreadfunc_eps( KSContext& context );
bool kspreadfunc_rand( KSContext& context );
bool kspreadfunc_randbetween( KSContext& context );
bool kspreadfunc_pow( KSContext& context );
bool kspreadfunc_mod( KSContext& context );
bool kspreadfunc_fact( KSContext& context );
bool kspreadfunc_sign( KSContext& context );
bool kspreadfunc_inv( KSContext& context );
bool kspreadfunc_rounddown( KSContext& context );
bool kspreadfunc_roundup( KSContext& context );
bool kspreadfunc_round( KSContext& context );
bool kspreadfunc_complex( KSContext& context );
bool kspreadfunc_complex_imag( KSContext& context );
bool kspreadfunc_complex_real( KSContext& context );
bool kspreadfunc_imsum( KSContext& context );
bool kspreadfunc_imsub( KSContext& context );
bool kspreadfunc_improduct( KSContext& context );
bool kspreadfunc_imconjugate( KSContext& context );
bool kspreadfunc_imargument( KSContext& context );
bool kspreadfunc_imabs( KSContext& context );
bool kspreadfunc_imcos( KSContext& context );
bool kspreadfunc_imsin( KSContext& context );
bool kspreadfunc_imln( KSContext& context );
bool kspreadfunc_imexp( KSContext& context );
bool kspreadfunc_imsqrt( KSContext& context );
bool kspreadfunc_impower( KSContext& context );
bool kspreadfunc_imdiv( KSContext& context );
bool kspreadfunc_delta( KSContext& context );
bool kspreadfunc_even( KSContext& context );
bool kspreadfunc_odd( KSContext& context );
bool kspreadfunc_count( KSContext& context );

// defined in kspread_functions_datetime.cc
bool kspreadfunc_years( KSContext& context );
bool kspreadfunc_months( KSContext& context );
bool kspreadfunc_weeks( KSContext& context );
bool kspreadfunc_days( KSContext& context );
bool kspreadfunc_hours( KSContext& context );
bool kspreadfunc_minutes( KSContext& context );
bool kspreadfunc_seconds( KSContext& context );
bool kspreadfunc_date( KSContext& context );
bool kspreadfunc_day( KSContext& context );
bool kspreadfunc_month( KSContext& context );
bool kspreadfunc_time( KSContext& context );
bool kspreadfunc_currentDate( KSContext& context );
bool kspreadfunc_shortcurrentDate( KSContext& context );
bool kspreadfunc_currentTime( KSContext& context );
bool kspreadfunc_currentDateTime( KSContext& context );
bool kspreadfunc_dayOfYear( KSContext& context );
bool kspreadfunc_daysInMonth( KSContext& context );
bool kspreadfunc_isLeapYear ( KSContext& context );
bool kspreadfunc_daysInYear ( KSContext& context );
bool kspreadfunc_weeksInYear( KSContext& context );

// defined in kspread_functions_logic.cc
bool kspreadfunc_not( KSContext& context );
bool kspreadfunc_or( KSContext& context );
bool kspreadfunc_nor( KSContext& context );
bool kspreadfunc_and( KSContext& context );
bool kspreadfunc_nand( KSContext& context );
bool kspreadfunc_xor( KSContext& context );
bool kspreadfunc_if( KSContext& context );
bool kspreadfunc_islogic( KSContext& context );
bool kspreadfunc_istext( KSContext& context );
bool kspreadfunc_isnottext( KSContext& context );
bool kspreadfunc_isnum( KSContext& context );
bool kspreadfunc_istime( KSContext& context );
bool kspreadfunc_isdate( KSContext& context );
bool kspreadfunc_isodd( KSContext& context );
bool kspreadfunc_iseven( KSContext& context );

// defined in kspread_functions_text.cc
bool kspreadfunc_join( KSContext& context );
bool kspreadfunc_left( KSContext& context );
bool kspreadfunc_right( KSContext& context );
bool kspreadfunc_upper( KSContext& context );
bool kspreadfunc_toggle( KSContext& context );
bool kspreadfunc_clean( KSContext& context );
bool kspreadfunc_sleek( KSContext& context );
bool kspreadfunc_proper(KSContext & context);
bool kspreadfunc_lower( KSContext& context );
bool kspreadfunc_find( KSContext& context );
bool kspreadfunc_mid( KSContext& context );
bool kspreadfunc_trim(KSContext& context );
bool kspreadfunc_len( KSContext& context );
bool kspreadfunc_EXACT( KSContext& context );
bool kspreadfunc_compare( KSContext& context );
bool kspreadfunc_replace( KSContext& context );
bool kspreadfunc_REPT( KSContext& context );

// defined in kspread_functions_conversion.cc
bool kspreadfunc_dec2hex( KSContext& context );
bool kspreadfunc_dec2oct( KSContext& context );
bool kspreadfunc_dec2bin( KSContext& context );
bool kspreadfunc_bin2dec( KSContext& context );
bool kspreadfunc_bin2oct( KSContext& context );
bool kspreadfunc_bin2hex( KSContext& context );
bool kspreadfunc_oct2dec( KSContext& context );
bool kspreadfunc_oct2bin( KSContext& context );
bool kspreadfunc_oct2hex( KSContext& context );
bool kspreadfunc_hex2dec( KSContext& context );
bool kspreadfunc_hex2bin( KSContext& context );
bool kspreadfunc_hex2oct( KSContext& context );
bool kspreadfunc_polr( KSContext& context );
bool kspreadfunc_pola( KSContext& context );
bool kspreadfunc_carx( KSContext& context );
bool kspreadfunc_cary( KSContext& context );
bool kspreadfunc_decsex( KSContext& context );
bool kspreadfunc_sexdec( KSContext& context );
bool kspreadfunc_roman( KSContext& context );
bool kspreadfunc_AsciiToChar( KSContext& context );
bool kspreadfunc_CharToAscii( KSContext& context );
bool kspreadfunc_inttobool( KSContext & context );
bool kspreadfunc_booltoint( KSContext & context );
bool kspreadfunc_BoolToString( KSContext& context );
bool kspreadfunc_NumberToString( KSContext& context );

// defined in kspread_functions_financial.cc
bool kspreadfunc_fv( KSContext& context );
bool kspreadfunc_compound( KSContext& context );
bool kspreadfunc_continuous( KSContext& context );
bool kspreadfunc_pv( KSContext& context );
bool kspreadfunc_pv_annuity( KSContext& context );
bool kspreadfunc_fv_annuity( KSContext& context );
bool kspreadfunc_effective( KSContext& context );
bool kspreadfunc_zero_coupon( KSContext& context );
bool kspreadfunc_level_coupon( KSContext& context );
bool kspreadfunc_nominal( KSContext& context );
bool kspreadfunc_sln( KSContext& context );
bool kspreadfunc_syd( KSContext& context );
bool kspreadfunc_db( KSContext& context );
bool kspreadfunc_euro( KSContext& context );

// defined in kspread_functions_statistical.cc
bool kspreadfunc_arrang( KSContext& context );
bool kspreadfunc_average( KSContext& context );
bool kspreadfunc_median( KSContext& context );
bool kspreadfunc_variance( KSContext& context );
bool kspreadfunc_stddev( KSContext& context );
bool kspreadfunc_combin( KSContext& context );
bool kspreadfunc_bino( KSContext& context );
bool kspreadfunc_bino_inv( KSContext& context );
bool kspreadfunc_phi(KSContext& context);
bool kspreadfunc_gauss(KSContext& context);
bool kspreadfunc_gammadist( KSContext& context );
bool kspreadfunc_betadist( KSContext& context );
bool kspreadfunc_fisher( KSContext& context );
bool kspreadfunc_fisherinv( KSContext& context );
bool kspreadfunc_normdist(KSContext& context );
bool kspreadfunc_lognormdist(KSContext& context );
bool kspreadfunc_stdnormdist(KSContext& context );
bool kspreadfunc_expondist(KSContext& context );
bool kspreadfunc_weibull( KSContext& context );
bool kspreadfunc_normsinv( KSContext& context );
bool kspreadfunc_norminv( KSContext& context );
bool kspreadfunc_gammaln( KSContext& context );
bool kspreadfunc_poisson( KSContext& context );
bool kspreadfunc_confidence( KSContext& context );
bool kspreadfunc_tdist( KSContext& context );
bool kspreadfunc_fdist( KSContext& context );
bool kspreadfunc_chidist( KSContext& context );
bool kspreadfunc_sumproduct( KSContext& context );
bool kspreadfunc_sumx2py2( KSContext& context );
bool kspreadfunc_sumx2my2( KSContext& context );
bool kspreadfunc_sumxmy2( KSContext& context );

/***************************************************************
 *
 * Classes which store extra informations in some KSParseNode.
 *
 ***************************************************************/

/**
 * For a node of type t_cell.
 */
class KSParseNodeExtraPoint : public KSParseNodeExtra
{
public:
  KSParseNodeExtraPoint( const QString& s, KSpreadMap* m, KSpreadTable* t ) : m_point( s, m, t ) { }

  KSpreadPoint* point() { return &m_point; }

private:
  KSpreadPoint m_point;
};

/**
 * For a node of type t_range.
 */
class KSParseNodeExtraRange : public KSParseNodeExtra
{
public:
  KSParseNodeExtraRange( const QString& s, KSpreadMap* m, KSpreadTable* t ) : m_range( s, m, t ) { }

  KSpreadRange* range() { return &m_range; }

private:
  KSpreadRange m_range;
};

/****************************************************
 *
 * Helper functions
 *
 ****************************************************/

/**
 * Creates dependencies from the parse tree of a formula.
 */
void makeDepends( KSContext& context, KSParseNode* node, KSpreadMap* m, KSpreadTable* t, QPtrList<KSpreadDependency>& depends )
{
  KSParseNodeExtra* extra = node->extra();
  if ( !extra )
  {
    if ( node->getType() == t_cell )
    {
      KSParseNodeExtraPoint* extra = new KSParseNodeExtraPoint( node->getStringLiteral(), m, t );
      kdDebug(36002) << "-------- Got dep " << util_cellName( extra->point()->pos.x(), extra->point()->pos.y() ) << endl;
      KSpreadDependency* d = new KSpreadDependency(extra->point()->pos.x(), extra->point()->pos.y(),
					       extra->point()->table);
      if (!d->Table())
      {
        QString tmp( i18n("The expression %1 is not valid") );
        tmp = tmp.arg( node->getStringLiteral() );
        context.setException( new KSException( "InvalidTableExpression", tmp ) );
        return;
      }
      depends.append( d );
      node->setExtra( extra );
    }
    else if ( node->getType() == t_range )
    {
      KSParseNodeExtraRange* extra = new KSParseNodeExtraRange( node->getStringLiteral(), m, t );
      KSpreadDependency* d = new KSpreadDependency(extra->range()->range.left(),
						   extra->range()->range.top(),
						   extra->range()->range.right(),
						   extra->range()->range.bottom(),
						   extra->range()->table);
      if (!d->Table())
      {
        QString tmp( i18n("The expression %1 is not valid") );
        tmp = tmp.arg( node->getStringLiteral() );
        context.setException( new KSException( "InvalidTableExpression", tmp ) );
        return;
      }
      depends.append( d );
      node->setExtra( extra );
    }
  }

  if ( node->branch1() )
    makeDepends( context, node->branch1(), m, t, depends );
  if ( node->branch2() )
    makeDepends( context, node->branch2(), m, t, depends );
  if ( node->branch3() )
    makeDepends( context, node->branch3(), m, t, depends );
  if ( node->branch4() )
    makeDepends( context, node->branch4(), m, t, depends );
  if ( node->branch5() )
    makeDepends( context, node->branch5(), m, t, depends );
}

/*********************************************************************
 *
 * Module with global functions like "sin", "cos", "sum" etc.
 *
 * Note: These modules must be registered in kspread_interpreter::kspreadCreateModule_KSpread
 *       They should also be documented in KSPREAD/extensions/builtin.xml
 *
 *********************************************************************/


static bool kspreadfunc_cell( KSContext& context )
{
    QValueList<KSValue::Ptr>& args = context.value()->listValue();

    if ( !KSUtil::checkArgumentsCount( context, 3, "cell", true ) )
      return false;

    if ( !KSUtil::checkType( context, args[0], KSValue::ListType, true ) )
      return false;
    if ( !KSUtil::checkType( context, args[1], KSValue::StringType, true ) )
      return false;
    if ( !KSUtil::checkType( context, args[2], KSValue::StringType, true ) )
      return false;

    const QValueList<KSValue::Ptr>& lines = args[0]->listValue();
    if ( lines.count() < 2 )
      return FALSE;

    QValueList<KSValue::Ptr>::ConstIterator it = lines.begin();
    if ( !KSUtil::checkType( context, (*it), KSValue::ListType, true ) )
      return false;
    const QValueList<KSValue::Ptr>& line = (*it)->listValue();
    QValueList<KSValue::Ptr>::ConstIterator it2 = line.begin();
    int x = 1;
    ++it;
    ++it2;
    for( ; it2 != line.end(); ++it2 )
    {
      if ( !KSUtil::checkType( context, (*it2), KSValue::StringType, true ) )
        return false;
      if ( (*it2)->stringValue() == args[1]->stringValue() )
        break;
      ++x;
    }
    if ( it2 == line.end() )
      return FALSE;

    kdDebug(36002) <<"x= "<<x<<endl;
    for( ; it != lines.end(); ++it )
    {
      const QValueList<KSValue::Ptr>& l = (*it)->listValue();
      if ( x >= (int)l.count() )
        return FALSE;
      if ( l[0]->stringValue() == args[2]->stringValue() )
      {
        context.setValue( new KSValue( *(l[x]) ) );
        return TRUE;
      }
    }

    context.setValue( new KSValue( 0.0 ) );
    return true;
}

static bool kspreadfunc_select_helper( KSContext& context, QValueList<KSValue::Ptr>& args, QString& result )
{
    QValueList<KSValue::Ptr>::Iterator it = args.begin();
    QValueList<KSValue::Ptr>::Iterator end = args.end();

    for( ; it != end; ++it )
    {
      if ( KSUtil::checkType( context, *it, KSValue::ListType, false ) )
      {
        if ( !kspreadfunc_select_helper( context, (*it)->listValue(), result ) )
          return false;
      }
      else if ( !(*it)->toString( context ).isEmpty() )
      {
        if ( !result.isEmpty() )
          result += "\\";
        result += (*it)->toString( context );
      }
    }

    return true;
}

static bool kspreadfunc_select( KSContext& context )
{
  QString result( "" );
  bool b = kspreadfunc_select_helper( context, context.value()->listValue(), result );

  if ( b )
    context.setValue( new KSValue( result ) );

  return b;
}

typedef struct
{
  const char *name;
  bool (*function)( KSContext& );
} functionEntry;

static const functionEntry funcTab[] = {

  // trigonometric
  { "cos", kspreadfunc_cos },
  { "sin", kspreadfunc_sin },
  { "tan", kspreadfunc_tan },
  { "atan", kspreadfunc_atan },
  { "asin", kspreadfunc_asin },
  { "acos", kspreadfunc_acos },
  { "cosh", kspreadfunc_cosh },
  { "sinh", kspreadfunc_sinh },
  { "tanh", kspreadfunc_tanh },
  { "acosh", kspreadfunc_acosh },
  { "asinh", kspreadfunc_asinh },
  { "atanh", kspreadfunc_atanh },

  // math
  { "DIV", kspreadfunc_div },
  { "PRODUCT", kspreadfunc_product },
  { "sqrt", kspreadfunc_sqrt },
  { "fabs", kspreadfunc_fabs },
  { "abs", kspreadfunc_fabs },
  { "floor", kspreadfunc_floor },
  { "ceil", kspreadfunc_ceil },
  { "exp", kspreadfunc_exp },
  { "ln", kspreadfunc_ln },
  { "log", kspreadfunc_log },
  { "LCM", kspreadfunc_lcm },
  { "LCD", kspreadfunc_lcd },
  { "multiply", kspreadfunc_mult },
  { "PI", kspreadfunc_PI },
  { "eps", kspreadfunc_eps },
  { "rand", kspreadfunc_rand },
  { "pow", kspreadfunc_pow },
  { "MOD", kspreadfunc_mod },
  { "fact", kspreadfunc_fact },

  // date & time
  { "date", kspreadfunc_date },
  { "day", kspreadfunc_day },
  { "month", kspreadfunc_month },
  { "time", kspreadfunc_time },
  { "currentTime", kspreadfunc_currentTime },
  { "currentDate", kspreadfunc_currentDate },
  { "currentDateTime", kspreadfunc_currentDateTime },
  { "dayOfYear", kspreadfunc_dayOfYear },

  // conversion
  { "degree", kspreadfunc_degree },
  { "radian", kspreadfunc_radian },
  { "DEC2HEX", kspreadfunc_dec2hex },
  { "DEC2BIN", kspreadfunc_dec2bin },
  { "DEC2OCT", kspreadfunc_dec2oct },
  { "CharToAscii", kspreadfunc_CharToAscii },
  { "AsciiToChar", kspreadfunc_AsciiToChar },
  { "BOOL2STRING", kspreadfunc_BoolToString },
  { "NUM2STRING", kspreadfunc_NumberToString },
  { "BOOL2INT", kspreadfunc_booltoint },
  { "INT2BOOL", kspreadfunc_inttobool },

  // logical
  { "OR", kspreadfunc_or },
  { "AND", kspreadfunc_and },
  { "NOR", kspreadfunc_nor },
  { "NAND", kspreadfunc_nand },
  { "XOR", kspreadfunc_xor },
  { "NOT", kspreadfunc_not },
  { "ISLOGIC", kspreadfunc_islogic },
  { "ISTEXT", kspreadfunc_istext },
  { "ISNUM", kspreadfunc_isnum },
  { "ISNOTTEXT", kspreadfunc_isnottext },
  { "ISODD", kspreadfunc_isodd },
  { "ISEVEN", kspreadfunc_iseven },

  // statistical
  { "average", kspreadfunc_average },
  { "median", kspreadfunc_median },
  { "variance", kspreadfunc_variance },
  { "stddev", kspreadfunc_stddev },
  { "COMBIN", kspreadfunc_combin },
  { "PERMUT", kspreadfunc_arrang },
  { "BINO", kspreadfunc_bino },
  { "INVBINO", kspreadfunc_bino_inv },
  { "GAUSS", kspreadfunc_gauss },
  { "PHI", kspreadfunc_phi },
  { "GAMMADIST", kspreadfunc_gammadist },
  { "BETADIST", kspreadfunc_betadist },
  { "FISHER", kspreadfunc_fisher },
  { "FISHERINV", kspreadfunc_fisherinv },
  { "NORMDIST", kspreadfunc_normdist },
  { "LOGNORMDIST", kspreadfunc_lognormdist },
  { "EXPONDIST", kspreadfunc_expondist },
  { "WEIBULL", kspreadfunc_weibull },
  { "NORMSINV", kspreadfunc_normsinv },
  { "NORMINV", kspreadfunc_norminv },
  { "GAMMALN", kspreadfunc_gammaln },
  { "POISSON", kspreadfunc_poisson },
  { "CONFIDENCE", kspreadfunc_confidence },
  { "TDIST", kspreadfunc_tdist },
  { "FDIST", kspreadfunc_fdist },
  { "CHIDIST", kspreadfunc_chidist },

  // financial
  { "effective", kspreadfunc_effective },
  { "nominal", kspreadfunc_nominal },
  { "FV", kspreadfunc_fv },
  { "FV_annuity", kspreadfunc_fv_annuity },
  { "PV", kspreadfunc_pv },
  { "PV_annuity", kspreadfunc_pv_annuity },
  { "zero_coupon", kspreadfunc_zero_coupon },
  { "level_coupon", kspreadfunc_level_coupon },
  { "SLN", kspreadfunc_sln },
  { "SYD", kspreadfunc_syd },
  { "DB", kspreadfunc_db },
  { "EURO", kspreadfunc_euro },

  // text
  { "left", kspreadfunc_left },
  { "right", kspreadfunc_right },
  { "mid", kspreadfunc_mid },
  { "len", kspreadfunc_len },
  { "EXACT", kspreadfunc_EXACT },
  { "COMPARE", kspreadfunc_compare },
  { "PROPER", kspreadfunc_proper },
  { "REPLACE", kspreadfunc_replace },
  { "REPT", kspreadfunc_REPT },
  { "lower", kspreadfunc_lower },
  { "upper", kspreadfunc_upper },
  { "TOGGLE", kspreadfunc_toggle },
  { "find", kspreadfunc_find },
  { "CLEAN", kspreadfunc_clean },
  { "SLEEK", kspreadfunc_sleek },

  // FIXME uncategorized yet
  { "sumsq", kspreadfunc_sumsq },
  { "max", kspreadfunc_max },
  { "min", kspreadfunc_min },
  { "join", kspreadfunc_join },
  { "IF", kspreadfunc_if },
  { "INT", kspreadfunc_INT },
  { "cell", kspreadfunc_cell },
  { "select", kspreadfunc_select },
  { "compound", kspreadfunc_compound },
  { "continuous", kspreadfunc_continuous },

  // compatibility with KSpread 1.0
  { "not", kspreadfunc_not },
  { "if", kspreadfunc_if },
  { "ENT", kspreadfunc_INT },
  { "DECHEX", kspreadfunc_dec2hex },
  { "DECBIN", kspreadfunc_dec2bin },
  { "DECOCT", kspreadfunc_dec2oct },

  // end  marker
  { NULL, NULL }
};

static KSModule::Ptr kspreadCreateModule_KSpread( KSInterpreter* interp )
{
  KSModule::Ptr module = new KSModule( interp, "kspread" );

  unsigned count = sizeof(funcTab)/sizeof(funcTab[0]);
  for( unsigned i=0; i<count; i++ )
  {
    QString name = funcTab[i].name;
    bool (*function)(KSContext&) = funcTab[i].function;
    if( function ) module->addObject( name, new KSValue(
      new KSBuiltinFunction( module, name, function ) ) );
  }

  module->addObject( "sign", new KSValue( new KSBuiltinFunction( module,"sign",kspreadfunc_sign) ) );
  module->addObject( "atan2", new KSValue( new KSBuiltinFunction( module,"atan2",kspreadfunc_atan2) ) );
  module->addObject( "INV", new KSValue( new KSBuiltinFunction( module,"INV",kspreadfunc_inv) ) );

  module->addObject( "ROUNDDOWN", new KSValue( new KSBuiltinFunction( module,"ROUNDDOWN",kspreadfunc_rounddown) ) );
  module->addObject( "ROUNDUP", new KSValue( new KSBuiltinFunction( module,"ROUNDUP",kspreadfunc_roundup) ) );
  module->addObject( "ROUND", new KSValue( new KSBuiltinFunction( module,"ROUND",kspreadfunc_round) ) );
  module->addObject( "BIN2DEC", new KSValue( new KSBuiltinFunction( module,"BIN2DEC",kspreadfunc_bin2dec) ) );
  module->addObject( "BIN2OCT", new KSValue( new KSBuiltinFunction( module,"BIN2OCT",kspreadfunc_bin2oct) ) );
  module->addObject( "BIN2HEX", new KSValue( new KSBuiltinFunction( module,"BIN2HEX",kspreadfunc_bin2hex) ) );
  module->addObject( "OCT2BIN", new KSValue( new KSBuiltinFunction( module,"OCT2BIN",kspreadfunc_oct2bin) ) );
  module->addObject( "OCT2DEC", new KSValue( new KSBuiltinFunction( module,"OCT2DEC",kspreadfunc_oct2dec) ) );
  module->addObject( "OCT2HEX", new KSValue( new KSBuiltinFunction( module,"OCT2HEX",kspreadfunc_oct2hex) ) );
  module->addObject( "HEX2BIN", new KSValue( new KSBuiltinFunction( module,"HEX2BIN",kspreadfunc_hex2bin) ) );
  module->addObject( "HEX2DEC", new KSValue( new KSBuiltinFunction( module,"HEX2DEC",kspreadfunc_hex2dec) ) );
  module->addObject( "HEX2OCT", new KSValue( new KSBuiltinFunction( module,"HEX2OCT",kspreadfunc_hex2oct) ) );
  module->addObject( "POLR", new KSValue( new KSBuiltinFunction( module,"POLR",kspreadfunc_polr) ) );
  module->addObject( "POLA", new KSValue( new KSBuiltinFunction( module,"POLA",kspreadfunc_pola) ) );
  module->addObject( "CARX", new KSValue( new KSBuiltinFunction( module,"CARX",kspreadfunc_carx) ) );
  module->addObject( "CARY", new KSValue( new KSBuiltinFunction( module,"CARY",kspreadfunc_cary) ) );
  //complex
  module->addObject( "COMPLEX", new KSValue( new KSBuiltinFunction( module,"COMPLEX",kspreadfunc_complex) ) );
  module->addObject( "IMAGINARY", new KSValue( new KSBuiltinFunction( module,"IMAGINARY",kspreadfunc_complex_imag) ) );
  module->addObject( "IMREAL", new KSValue( new KSBuiltinFunction( module,"IMREAL",kspreadfunc_complex_real) ) );
  module->addObject( "IMSUM", new KSValue( new KSBuiltinFunction( module, "IMSUM", kspreadfunc_imsum ) ) );
  module->addObject( "IMSUB", new KSValue( new KSBuiltinFunction( module, "IMSUB", kspreadfunc_imsub ) ) );
  module->addObject( "IMPRODUCT", new KSValue( new KSBuiltinFunction( module, "IMPRODUCT", kspreadfunc_improduct ) ) );
  module->addObject( "IMCONJUGATE", new KSValue( new KSBuiltinFunction( module, "IMCONJUGATE", kspreadfunc_imconjugate ) ) );
  module->addObject( "IMARGUMENT", new KSValue( new KSBuiltinFunction( module, "IMARGUMENT", kspreadfunc_imargument ) ) );
  module->addObject( "IMABS", new KSValue( new KSBuiltinFunction( module, "IMABS", kspreadfunc_imabs ) ) );
  module->addObject( "IMDIV", new KSValue( new KSBuiltinFunction( module, "IMDIV", kspreadfunc_imdiv ) ) );
  module->addObject( "IMCOS", new KSValue( new KSBuiltinFunction( module, "IMCOS", kspreadfunc_imcos ) ) );
  module->addObject( "IMSIN", new KSValue( new KSBuiltinFunction( module, "IMSIN", kspreadfunc_imsin ) ) );
  module->addObject( "IMEXP", new KSValue( new KSBuiltinFunction( module, "IMEXP", kspreadfunc_imexp ) ) );
  module->addObject( "IMLN", new KSValue( new KSBuiltinFunction( module, "IMLN", kspreadfunc_imln ) ) );
  module->addObject( "IMSQRT", new KSValue( new KSBuiltinFunction( module, "IMSQRT", kspreadfunc_imsqrt ) ) );
  module->addObject( "IMPOWER", new KSValue( new KSBuiltinFunction( module, "IMPOWER", kspreadfunc_impower ) ) );

  module->addObject( "SUMPRODUCT", new KSValue( new KSBuiltinFunction( module, "SUMPRODUCT", kspreadfunc_sumproduct ) ) );
  module->addObject( "SUMX2PY2", new KSValue( new KSBuiltinFunction( module, "SUMX2PY2", kspreadfunc_sumx2py2 ) ) );
  module->addObject( "SUMX2MY2", new KSValue( new KSBuiltinFunction( module, "SUMX2MY2", kspreadfunc_sumx2my2 ) ) );
  module->addObject( "SUM2XMY", new KSValue( new KSBuiltinFunction( module, "SUM2XMY", kspreadfunc_sumxmy2 ) ) );
  module->addObject( "DELTA", new KSValue( new KSBuiltinFunction( module, "DELTA", kspreadfunc_delta ) ) );
  module->addObject( "EVEN", new KSValue( new KSBuiltinFunction( module, "EVEN", kspreadfunc_even ) ) );
  module->addObject( "ODD", new KSValue( new KSBuiltinFunction( module, "ODD", kspreadfunc_odd ) ) );
  module->addObject( "RANDBETWEEN", new KSValue( new KSBuiltinFunction( module, "RANDBETWEEN", kspreadfunc_randbetween ) ) );
  module->addObject( "LOGn", new KSValue( new KSBuiltinFunction( module, "LOGn", kspreadfunc_logn ) ) );
  module->addObject( "SQRTn", new KSValue( new KSBuiltinFunction( module, "SQRTn", kspreadfunc_sqrtn ) ) );
  module->addObject( "count", new KSValue( new KSBuiltinFunction( module, "count", kspreadfunc_count ) ) );
  module->addObject( "CUR", new KSValue( new KSBuiltinFunction( module, "CUR", kspreadfunc_cur ) ) );
  module->addObject( "DECSEX", new KSValue( new KSBuiltinFunction( module, "DECSEX", kspreadfunc_decsex) ) );
  module->addObject( "SEXDEC", new KSValue( new KSBuiltinFunction( module, "SEXDEC", kspreadfunc_sexdec) ) );
  module->addObject( "ISTIME", new KSValue( new KSBuiltinFunction( module, "ISTIME", kspreadfunc_istime) ) );
  module->addObject( "ISDATE", new KSValue( new KSBuiltinFunction( module, "ISDATE", kspreadfunc_isdate) ) );
  module->addObject( "hours", new KSValue( new KSBuiltinFunction( module, "hours", kspreadfunc_hours) ) );
  module->addObject( "minutes", new KSValue( new KSBuiltinFunction( module, "minutes", kspreadfunc_minutes) ) );
  module->addObject( "DAYS", new KSValue( new KSBuiltinFunction( module, "DAYS", kspreadfunc_days) ) );
  module->addObject( "WEEKS", new KSValue( new KSBuiltinFunction( module, "WEEKS", kspreadfunc_weeks) ) );
  module->addObject( "MONTHS", new KSValue( new KSBuiltinFunction( module, "MONTHS", kspreadfunc_months) ) );
  module->addObject( "YEARS", new KSValue( new KSBuiltinFunction( module, "YEARS", kspreadfunc_years) ) );
  module->addObject( "isLeapYear", new KSValue( new KSBuiltinFunction( module, "isLeapYear", kspreadfunc_isLeapYear) ) );
  module->addObject( "daysInMonth", new KSValue( new KSBuiltinFunction( module, "daysInMonth", kspreadfunc_daysInMonth) ) );
  module->addObject( "daysInYear", new KSValue( new KSBuiltinFunction( module, "daysInYear", kspreadfunc_daysInYear) ) );
  module->addObject( "weeksInYear", new KSValue( new KSBuiltinFunction( module, "weeksInYear", kspreadfunc_weeksInYear) ) );
  module->addObject( "seconds", new KSValue( new KSBuiltinFunction( module, "seconds", kspreadfunc_seconds) ) );
  module->addObject( "ROMAN", new KSValue( new KSBuiltinFunction( module, "ROMAN", kspreadfunc_roman) ) );
  module->addObject( "shortcurrentDate", new KSValue( new KSBuiltinFunction( module, "shortcurrentDate", kspreadfunc_shortcurrentDate) ) );
  module->addObject( "trim", new KSValue( new KSBuiltinFunction( module, "trim", kspreadfunc_trim) ) );



  return module;
}

/*********************************************************************
 *
 * KSpreadInterpreter
 *
 *********************************************************************/

KSpreadInterpreter::KSpreadInterpreter( KSpreadDoc* doc ) : KSInterpreter()
{
  m_doc = doc;

  KSModule::Ptr m = kspreadCreateModule_KSpread( this );
  m_modules.insert( m->name(), m );

  // Integrate the KSpread module in the global namespace for convenience
  KSNamespace::Iterator it = m->nameSpace()->begin();
  KSNamespace::Iterator end = m->nameSpace()->end();
  for(; it != end; ++it )
    m_global->insert( it.key(), it.data() );
}

bool KSpreadInterpreter::processExtension( KSContext& context, KSParseNode* node )
{
  KSParseNodeExtra* extra = node->extra();
  if ( !extra )
  {
    if ( node->getType() == t_cell )
      extra = new KSParseNodeExtraPoint( node->getStringLiteral(), m_doc->map(), m_table );
    else if ( node->getType() == t_range )
      extra = new KSParseNodeExtraRange( node->getStringLiteral(), m_doc->map(), m_table );
    else
      return KSInterpreter::processExtension( context, node );
    node->setExtra( extra );
  }

  if ( node->getType() == t_cell )
  {
    KSParseNodeExtraPoint* p = (KSParseNodeExtraPoint*)extra;
    KSpreadPoint* point = p->point();

    if ( !point->isValid() )
    {
      QString tmp( i18n("The expression %1 is not valid") );
      tmp = tmp.arg( node->getStringLiteral() );
      context.setException( new KSException( "InvalidCellExpression", tmp ) );
      return false;
    }

    KSpreadCell* cell = point->cell();

    if ( cell->hasError() )
    {
      QString tmp( i18n("The cell %1 has an error:\n\n%2") );
      tmp = tmp.arg( util_cellName( cell->table(), cell->column(), cell->row() ) );
      tmp = tmp.arg( node->getStringLiteral() );
      context.setException( new KSException( "ErrorInCell", tmp ) );
      return false;
    }

    if ( cell->isDefault())
      context.setValue( new KSValue(  /*KSValue::Empty*/ 0.0 ) );
    else if(cell->isObscured() && cell->isObscuringForced())
      context.setValue( new KSValue( 0.0 ) );
    else if ( cell->isNumeric() )
      context.setValue( new KSValue( cell->valueDouble() ) );
    else if ( cell->isBool() )
      context.setValue( new KSValue( cell->valueBool() ) );
    else if ( cell->isTime() )
      context.setValue( new KSValue( cell->valueTime() ) );
    else if ( cell->isDate() )
      context.setValue( new KSValue( cell->valueDate() ) );
    else if ( cell->valueString().isEmpty() )
      context.setValue( new KSValue( 0.0  /*KSValue::Empty*/ ) );
    else
      context.setValue( new KSValue( cell->valueString() ) );
    return true;
  }
  // Parse a range like "A1:B3"
  else if ( node->getType() == t_range )
  {
    KSParseNodeExtraRange* p = (KSParseNodeExtraRange*)extra;
    KSpreadRange* r = p->range();

    // Is it a valid range ?
    if ( !r->isValid() )
    {
      QString tmp( i18n("The expression %1 is not valid") );
      tmp = tmp.arg( node->getStringLiteral() );
      context.setException( new KSException( "InvalidRangeExpression", tmp ) );
      return false;
    }

    // The range is translated in a list or lists of integers
    KSValue* v = new KSValue( KSValue::ListType );
    for( int y = 0; y < r->range.height(); ++y )
    {
      KSValue* l = new KSValue( KSValue::ListType );

      for( int x = 0; x < r->range.width(); ++x )
      {
        KSValue* c;
        KSpreadCell* cell = r->table->cellAt( r->range.x() + x, r->range.y() + y );

        if ( cell->hasError() )
        {
          QString tmp( i18n("The cell %1 has an error:\n\n%2") );
          tmp = tmp.arg( util_cellName( cell->table(), cell->column(), cell->row() ) );
          tmp = tmp.arg( node->getStringLiteral() );
          context.setException( new KSException( "ErrorInCell", tmp ) );
          return false;
        }

        if ( cell->isDefault() )
          c = new KSValue( 0.0 /*KSValue::Empty*/);
        else if ( cell->isNumeric() )
          c = new KSValue( cell->valueDouble() );
        else if ( cell->isBool() )
          c = new KSValue( cell->valueBool() );
        else if ( cell->isDate() )
          c = new KSValue( cell->valueDate() );
        else if ( cell->isTime() )
          c = new KSValue( cell->valueTime() );
        else if ( cell->valueString().isEmpty() )
          c = new KSValue( 0.0 /*KSValue::Empty*/ );
        else
          c = new KSValue( cell->valueString() );
        if ( !(cell->isObscured() && cell->isObscuringForced()) )
                l->listValue().append( c );
      }
      v->listValue().append( l );
    }
    context.setValue( v );

    return true;
  }
  else
    Q_ASSERT( 0 );

  // Never reached
  return false;
}

KSParseNode* KSpreadInterpreter::parse( KSContext& context, KSpreadTable* table, const QString& formula, QPtrList<KSpreadDependency>& depends )
{
    // Create the parse tree.
    KSParser parser;
    // Tell the parser the locale so that it can parse localized numbers.
    if ( !parser.parse( formula.utf8(), KSCRIPT_EXTENSION_KSPREAD, table->doc()->locale() ) )
    {
	context.setException( new KSException( "SyntaxError", parser.errorMessage() ) );
	return 0;
    }

    KSParseNode* n = parser.donateParseTree();
    makeDepends( context, n, table->map(), table, depends );

    return n;
}

bool KSpreadInterpreter::evaluate( KSContext& context, KSParseNode* node, KSpreadTable* table )
{
    // Save the current table to make this function reentrant.
    KSpreadTable* t = m_table;
    m_table = table;

    bool b = node->eval( context );

    m_table = t;

    return b;
}
