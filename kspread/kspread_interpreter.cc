#include "kspread_interpreter.h"
#include "kspread_util.h"
#include "kspread_doc.h"
#include "kspread_map.h"
#include "kspread_cell.h"
#include "kspread_table.h"

#include <kscript_parser.h>
#include <kscript_util.h>
#include <kscript_func.h>
#include <kscript_proxy.h>
#include <kscript_synext.h>

#include <math.h>
#include <klocale.h>
#include <qdatetime.h>

#include <dcopobject.h>
#include <dcopclient.h>
#include <kdebug.h>

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
void makeDepends( KSContext& context, KSParseNode* node, KSpreadMap* m, KSpreadTable* t, QList<KSpreadDepend>& depends )
{
  KSParseNodeExtra* extra = node->extra();
  if ( !extra )
  {
    if ( node->getType() == t_cell )
    {
      KSParseNodeExtraPoint* extra = new KSParseNodeExtraPoint( node->getStringLiteral(), m, t );
      kdDebug(36001) << "--------------------- Got dep " << extra->point()->pos.x() << "|"
		     << extra->point()->pos.y() << endl;
      KSpreadDepend* d = new KSpreadDepend;
      d->m_iColumn = extra->point()->pos.x();
      d->m_iRow = extra->point()->pos.y();
      d->m_iColumn2 = -1;
      d->m_iRow2 = -1;
      d->m_pTable = extra->point()->table;
	  if (!d->m_pTable)
	  {
        QString tmp( "The expression %1 is not valid" );
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
      KSpreadDepend* d = new KSpreadDepend;
      d->m_iColumn = extra->range()->range.left();
      d->m_iRow = extra->range()->range.top();
      d->m_iColumn2 = extra->range()->range.right();
      d->m_iRow2 = extra->range()->range.bottom();
      d->m_pTable = extra->range()->table;
	  if (!d->m_pTable)
	  {
        QString tmp( "The expression %1 is not valid" );
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
 *********************************************************************/

static bool kspreadfunc_sin( KSContext& context )
{
  QValueList<KSValue::Ptr>& args = context.value()->listValue();

  if ( !KSUtil::checkArgumentsCount( context, 1, "sin", true ) )
    return false;

  if ( !KSUtil::checkType( context, args[0], KSValue::DoubleType, true ) )
    return false;

  context.setValue( new KSValue( sin( args[0]->doubleValue() ) ) );

  return true;
}


static bool kspreadfunc_cos( KSContext& context )
{
  QValueList<KSValue::Ptr>& args = context.value()->listValue();

  if ( !KSUtil::checkArgumentsCount( context, 1, "cos", true ) )
    return false;

  if ( !KSUtil::checkType( context, args[0], KSValue::DoubleType, true ) )
    return false;

  context.setValue( new KSValue( cos( args[0]->doubleValue() ) ) );

  return true;
}

static bool kspreadfunc_sqrt( KSContext& context )
{
  QValueList<KSValue::Ptr>& args = context.value()->listValue();

  if ( !KSUtil::checkArgumentsCount( context, 1, "sqrt", true ) )
    return false;

  if ( !KSUtil::checkType( context, args[0], KSValue::DoubleType, true ) )
    return false;

  context.setValue( new KSValue( sqrt( args[0]->doubleValue() ) ) );

  return true;
}

static bool kspreadfunc_fabs( KSContext& context )
{
  QValueList<KSValue::Ptr>& args = context.value()->listValue();

  if ( !KSUtil::checkArgumentsCount( context, 1, "fabs", true ) )
    return false;

  if ( !KSUtil::checkType( context, args[0], KSValue::DoubleType, true ) )
    return false;

  context.setValue( new KSValue( fabs( args[0]->doubleValue() ) ) );

  return true;
}

static bool kspreadfunc_tan( KSContext& context )
{
  QValueList<KSValue::Ptr>& args = context.value()->listValue();

  if ( !KSUtil::checkArgumentsCount( context, 1, "tan", true ) )
    return false;

  if ( !KSUtil::checkType( context, args[0], KSValue::DoubleType, true ) )
    return false;

  context.setValue( new KSValue( tan( args[0]->doubleValue() ) ) );

  return true;
}

static bool kspreadfunc_exp( KSContext& context )
{
  QValueList<KSValue::Ptr>& args = context.value()->listValue();

  if ( !KSUtil::checkArgumentsCount( context, 1, "exp",true ) )
    return false;

  if ( !KSUtil::checkType( context, args[0], KSValue::DoubleType, true ) )
    return false;

  context.setValue( new KSValue( exp( args[0]->doubleValue() ) ) );

  return true;
}

static bool kspreadfunc_ceil( KSContext& context )
{
  QValueList<KSValue::Ptr>& args = context.value()->listValue();

  if ( !KSUtil::checkArgumentsCount( context, 1, "ceil", true ) )
    return false;

  if ( !KSUtil::checkType( context, args[0], KSValue::DoubleType, true ) )
    return false;

  context.setValue( new KSValue( ceil( args[0]->doubleValue() ) ) );

  return true;
}

static bool kspreadfunc_floor( KSContext& context )
{
  QValueList<KSValue::Ptr>& args = context.value()->listValue();

  if ( !KSUtil::checkArgumentsCount( context, 1, "floor", true ) )
    return false;

  if ( !KSUtil::checkType( context, args[0], KSValue::DoubleType, true ) )
    return false;

  context.setValue( new KSValue( floor( args[0]->doubleValue() ) ) );

  return true;
}

static bool kspreadfunc_atan( KSContext& context )
{
  QValueList<KSValue::Ptr>& args = context.value()->listValue();

  if ( !KSUtil::checkArgumentsCount( context, 1, "atan", true ) )
    return false;

  if ( !KSUtil::checkType( context, args[0], KSValue::DoubleType, true ) )
    return false;

  context.setValue( new KSValue( atan( args[0]->doubleValue() ) ) );

  return true;
}
static bool kspreadfunc_ln( KSContext& context )
{
  QValueList<KSValue::Ptr>& args = context.value()->listValue();

  if ( !KSUtil::checkArgumentsCount( context, 1, "ln", true ) )
    return false;

  if ( !KSUtil::checkType( context, args[0], KSValue::DoubleType, true ) )
    return false;

  context.setValue( new KSValue( log( args[0]->doubleValue() ) ) );

  return true;
}

static bool kspreadfunc_asin( KSContext& context )
{
  QValueList<KSValue::Ptr>& args = context.value()->listValue();

  if ( !KSUtil::checkArgumentsCount( context, 1, "asin", true ) )
    return false;

  if ( !KSUtil::checkType( context, args[0], KSValue::DoubleType, true ) )
    return false;

  context.setValue( new KSValue( asin( args[0]->doubleValue() ) ) );

  return true;
}

static bool kspreadfunc_acos( KSContext& context )
{
  QValueList<KSValue::Ptr>& args = context.value()->listValue();

  if ( !KSUtil::checkArgumentsCount( context, 1, "acos", true ) )
    return false;

  if ( !KSUtil::checkType( context, args[0], KSValue::DoubleType, true ) )
    return false;

  context.setValue( new KSValue( acos( args[0]->doubleValue() ) ) );

  return true;
}

static bool kspreadfunc_log( KSContext& context )
{
  QValueList<KSValue::Ptr>& args = context.value()->listValue();

  if ( !KSUtil::checkArgumentsCount( context, 1, "log", true ) )
    return false;

  if ( !KSUtil::checkType( context, args[0], KSValue::DoubleType, true ) )
    return false;

  context.setValue( new KSValue( log10( args[0]->doubleValue() ) ) );

  return true;
}

static bool kspreadfunc_asinh( KSContext& context )
{
  QValueList<KSValue::Ptr>& args = context.value()->listValue();

  if ( !KSUtil::checkArgumentsCount( context, 1, "asinh", true ) )
    return false;

  if ( !KSUtil::checkType( context, args[0], KSValue::DoubleType, true ) )
    return false;

  context.setValue( new KSValue( asinh( args[0]->doubleValue() ) ) );

  return true;
}

static bool kspreadfunc_acosh( KSContext& context )
{
  QValueList<KSValue::Ptr>& args = context.value()->listValue();

  if ( !KSUtil::checkArgumentsCount( context, 1, "acosh", true ) )
    return false;

  if ( !KSUtil::checkType( context, args[0], KSValue::DoubleType, true ) )
    return false;

  context.setValue( new KSValue( acosh( args[0]->doubleValue() ) ) );

  return true;
}

static bool kspreadfunc_atanh( KSContext& context )
{
  QValueList<KSValue::Ptr>& args = context.value()->listValue();

  if ( !KSUtil::checkArgumentsCount( context, 1, "atanh", true ) )
    return false;

  if ( !KSUtil::checkType( context, args[0], KSValue::DoubleType, true ) )
    return false;

  context.setValue( new KSValue( atanh( args[0]->doubleValue() ) ) );

  return true;
}

static bool kspreadfunc_tanh( KSContext& context )
{
  QValueList<KSValue::Ptr>& args = context.value()->listValue();

  if ( !KSUtil::checkArgumentsCount( context, 1, "tanh", true ) )
    return false;

  if ( !KSUtil::checkType( context, args[0], KSValue::DoubleType, true ) )
    return false;

  context.setValue( new KSValue( tanh( args[0]->doubleValue() ) ) );

  return true;
}

static bool kspreadfunc_sinh( KSContext& context )
{
  QValueList<KSValue::Ptr>& args = context.value()->listValue();

  if ( !KSUtil::checkArgumentsCount( context, 1, "sinh", true ) )
    return false;

  if ( !KSUtil::checkType( context, args[0], KSValue::DoubleType, true ) )
    return false;

  context.setValue( new KSValue( sinh( args[0]->doubleValue() ) ) );

  return true;
}

static bool kspreadfunc_cosh( KSContext& context )
{
  QValueList<KSValue::Ptr>& args = context.value()->listValue();

  if ( !KSUtil::checkArgumentsCount( context, 1, "cosh", true ) )
    return false;

  if ( !KSUtil::checkType( context, args[0], KSValue::DoubleType, true ) )
    return false;

  context.setValue( new KSValue( cosh( args[0]->doubleValue() ) ) );

  return true;
}

static bool kspreadfunc_degree( KSContext& context )
{
  QValueList<KSValue::Ptr>& args = context.value()->listValue();

  if ( !KSUtil::checkArgumentsCount( context, 1, "degree", true ) )
    return false;

  if ( !KSUtil::checkType( context, args[0], KSValue::DoubleType, true ) )
    return false;

  context.setValue( new KSValue( (args[0]->doubleValue()*180)/M_PI  ));

  return true;
}

static bool kspreadfunc_radian( KSContext& context )
{
  QValueList<KSValue::Ptr>& args = context.value()->listValue();

  if ( !KSUtil::checkArgumentsCount( context, 1, "radian", true ) )
    return false;

  if ( !KSUtil::checkType( context, args[0], KSValue::DoubleType, true ) )
    return false;

  context.setValue( new KSValue( (args[0]->doubleValue()*M_PI )/180  ));

  return true;
}


static bool kspreadfunc_sum_helper( KSContext& context, QValueList<KSValue::Ptr>& args, double& result )
{
  QValueList<KSValue::Ptr>::Iterator it = args.begin();
  QValueList<KSValue::Ptr>::Iterator end = args.end();

  for( ; it != end; ++it )
  {
    if ( KSUtil::checkType( context, *it, KSValue::ListType, false ) )
    {
      if ( !kspreadfunc_sum_helper( context, (*it)->listValue(), result ) )
	return false;
    }
    else if ( KSUtil::checkType( context, *it, KSValue::DoubleType, true ) )
      {
      result += (*it)->doubleValue();
      //kdDebug(36001) << "Value : " << (*it)->doubleValue() << endl;
      }
    else
      return false;
  }
  //kdDebug(36001) << "Quit :" << endl;
  return true;
}

static bool kspreadfunc_sum( KSContext& context )
{
  double result = 0.0;
  bool b = kspreadfunc_sum_helper( context, context.value()->listValue(), result );

  if ( b )
    context.setValue( new KSValue( result ) );

  return b;
}

static bool kspreadfunc_max_helper( KSContext& context, QValueList<KSValue::Ptr>& args, double& result,int& inter)
{
  QValueList<KSValue::Ptr>::Iterator it = args.begin();
  QValueList<KSValue::Ptr>::Iterator end = args.end();

  for( ; it != end; ++it )
  {
    if ( KSUtil::checkType( context, *it, KSValue::ListType, false ) )
    {

      if ( !kspreadfunc_max_helper( context, (*it)->listValue(), result,inter ) )
	return false;
    }
    else if ( KSUtil::checkType( context, *it, KSValue::DoubleType, true ) )
      {
      if(inter == 0)
      	{
      	result=(*it)->doubleValue();
      	inter=1;
      	}
      if(result <  (*it)->doubleValue())
      	result =(*it)->doubleValue();
    }
    else
      return false;
  }

  return true;
}

static bool kspreadfunc_max( KSContext& context )
{
  double result = 0.0;

  //init first element
  int inter=0;

  bool b = kspreadfunc_max_helper( context, context.value()->listValue(), result ,inter );

  if ( b )
    context.setValue( new KSValue( result ) );

  return b;
}

static bool kspreadfunc_min_helper( KSContext& context, QValueList<KSValue::Ptr>& args, double& result,int& inter)
{
  QValueList<KSValue::Ptr>::Iterator it = args.begin();
  QValueList<KSValue::Ptr>::Iterator end = args.end();

  for( ; it != end; ++it )
  {
    if ( KSUtil::checkType( context, *it, KSValue::ListType, false ) )
    {

      if ( !kspreadfunc_min_helper( context, (*it)->listValue(), result,inter ) )
	return false;
    }
    else if ( KSUtil::checkType( context, *it, KSValue::DoubleType, true ) )
      {
      if(inter == 0)
      	{
      	result=(*it)->doubleValue();
      	inter=1;
      	}
      if(result >  (*it)->doubleValue())
      	result =(*it)->doubleValue();
    }
    else
      return false;
  }

  return true;
}

static bool kspreadfunc_min( KSContext& context )
{
  double result = 0.0;

  //init first element
  int inter=0;

  bool b = kspreadfunc_min_helper( context, context.value()->listValue(), result ,inter );

  if ( b )
    context.setValue( new KSValue( result ) );

  return b;
}

static bool kspreadfunc_average_helper( KSContext& context, QValueList<KSValue::Ptr>& args, double& result,int& number)
{
  QValueList<KSValue::Ptr>::Iterator it = args.begin();
  QValueList<KSValue::Ptr>::Iterator end = args.end();

  for( ; it != end; ++it )
  {
    if ( KSUtil::checkType( context, *it, KSValue::ListType, false ) )
    {
      if ( !kspreadfunc_average_helper( context, (*it)->listValue(), result ,number) )
	return false;
	
    }
    else if ( KSUtil::checkType( context, *it, KSValue::DoubleType, true ) )
      {
      result += (*it)->doubleValue();
      number++;
      }
    else
    	return false;
  }

  return true;
}

static bool kspreadfunc_average( KSContext& context )
{
  double result = 0.0;

  int number=0;
  bool b = kspreadfunc_average_helper( context, context.value()->listValue(), result ,number);

  if ( b )
    context.setValue( new KSValue( result/number ) );


  return b;
}

static bool kspreadfunc_variance_helper( KSContext& context, QValueList<KSValue::Ptr>& args, double& result,double& avera)
{
  QValueList<KSValue::Ptr>::Iterator it = args.begin();
  QValueList<KSValue::Ptr>::Iterator end = args.end();

  for( ; it != end; ++it )
  {
    if ( KSUtil::checkType( context, *it, KSValue::ListType, false ) )
    {
      if ( !kspreadfunc_variance_helper( context, (*it)->listValue(), result ,avera) )
	return false;
	
    }
    else if ( KSUtil::checkType( context, *it, KSValue::DoubleType, true ) )
      {
      result += (((*it)->doubleValue()-avera)*((*it)->doubleValue()-avera));
      }
    else
    	return false;
  }

  return true;
}

static bool kspreadfunc_variance( KSContext& context )
{
  double result = 0.0;
  double avera = 0.0;
  int number=0;
  bool b = kspreadfunc_average_helper( context, context.value()->listValue(), result ,number);

  if ( b )
  	{
  	avera=result/number;
  	result=0.0;
  	bool b = kspreadfunc_variance_helper( context, context.value()->listValue(), result,avera );
    	if(b)
    		context.setValue( new KSValue(result/number ) );
      	}

  return b;
}

static bool kspreadfunc_stddev_helper( KSContext& context, QValueList<KSValue::Ptr>& args, double& result,double& avera)
{
  QValueList<KSValue::Ptr>::Iterator it = args.begin();
  QValueList<KSValue::Ptr>::Iterator end = args.end();

  for( ; it != end; ++it )
  {
    if ( KSUtil::checkType( context, *it, KSValue::ListType, false ) )
    {
      if ( !kspreadfunc_stddev_helper( context, (*it)->listValue(), result ,avera) )
	return false;

    }
    else if ( KSUtil::checkType( context, *it, KSValue::DoubleType, true ) )
      {
      result += (((*it)->doubleValue()-avera)*((*it)->doubleValue()-avera));
      }
    else
    	return false;
  }

  return true;
}

static bool kspreadfunc_stddev( KSContext& context )
{
  double result = 0.0;
  double avera = 0.0;
  int number=0;
  bool b = kspreadfunc_average_helper( context, context.value()->listValue(), result ,number);

  if ( b )
  	{
  	avera=result/number;
  	result=0.0;
  	bool b = kspreadfunc_stddev_helper( context, context.value()->listValue(), result,avera );
    	if(b)
    		context.setValue( new KSValue(sqrt(result/number )) );
      	}

  return b;
}

static bool kspreadfunc_mult_helper( KSContext& context, QValueList<KSValue::Ptr>& args, double& result )
{
  QValueList<KSValue::Ptr>::Iterator it = args.begin();
  QValueList<KSValue::Ptr>::Iterator end = args.end();

  for( ; it != end; ++it )
  {
    if ( KSUtil::checkType( context, *it, KSValue::ListType, false ) )
    {
      if ( !kspreadfunc_mult_helper( context, (*it)->listValue(), result ) )
	return false;
    }
    else if ( KSUtil::checkType( context, *it, KSValue::DoubleType, true ) )
      result *= (*it)->doubleValue();
    else
      return false;
  }

  return true;
}

static bool kspreadfunc_mult( KSContext& context )
{
  double result = 1.0;
  bool b = kspreadfunc_mult_helper( context, context.value()->listValue(), result );

  if ( b )
    context.setValue( new KSValue( result ) );

  return b;
}


static bool kspreadfunc_join_helper( KSContext& context, QValueList<KSValue::Ptr>& args, QString& tmp )
{
  QValueList<KSValue::Ptr>::Iterator it = args.begin();
  QValueList<KSValue::Ptr>::Iterator end = args.end();
  QString tmp2;
  for( ; it != end; ++it )
  {
    if ( KSUtil::checkType( context, *it, KSValue::ListType, false ) )
    {
      if ( !kspreadfunc_join_helper( context, (*it)->listValue(), tmp ) )
	return false;
    }
    else if ( KSUtil::checkType( context, *it, KSValue::StringType, true ) )
    	tmp+= (*it)->stringValue();
    else if( KSUtil::checkType( context, *it, KSValue::DoubleType, true ) )
      	tmp+= tmp2.setNum((*it)->doubleValue());
    else
      return false;
  }
  return true;
}

static bool kspreadfunc_join( KSContext& context )
{
  QString tmp;
  bool b = kspreadfunc_join_helper( context, context.value()->listValue(), tmp );

  if ( b )
    context.setValue( new KSValue( tmp ) );

  return b;
}

static bool kspreadfunc_not( KSContext& context )
{
  QValueList<KSValue::Ptr>& args = context.value()->listValue();

  if ( !KSUtil::checkArgumentsCount( context, 1, "not", true ) )
    return false;

  if ( !KSUtil::checkType( context, args[0], KSValue::BoolType, true ) )
    return false;

  bool toto = !args[0]->boolValue();
  context.setValue( new KSValue(toto));
  return true;
}


static bool kspreadfunc_or_helper( KSContext& context, QValueList<KSValue::Ptr>& args, bool& first )
{
  QValueList<KSValue::Ptr>::Iterator it = args.begin();
  QValueList<KSValue::Ptr>::Iterator end = args.end();

  for( ; it != end; ++it )
  {
    if ( KSUtil::checkType( context, *it, KSValue::ListType, false ) )
    {
      if ( !kspreadfunc_or_helper( context, (*it)->listValue(), first ) )
	return false;
    }
    else if ( KSUtil::checkType( context, *it, KSValue::BoolType, true ) )
      first = (first || (*it)->boolValue());
    else
      return false;
  }

  return true;
}

static bool kspreadfunc_or( KSContext& context )
{
  bool first = false;
  bool b = kspreadfunc_or_helper( context, context.value()->listValue(), first );

  if ( b )
    context.setValue( new KSValue( first ) );

  return b;
}

static bool kspreadfunc_nor( KSContext& context )
{
  bool first = false;
  bool b = kspreadfunc_or_helper( context, context.value()->listValue(), first );

  if ( b )
    context.setValue( new KSValue( !first ) );

  return b;
}


static bool kspreadfunc_and_helper( KSContext& context, QValueList<KSValue::Ptr>& args, bool& first )
{
  QValueList<KSValue::Ptr>::Iterator it = args.begin();
  QValueList<KSValue::Ptr>::Iterator end = args.end();

  for( ; it != end; ++it )
  {
    if ( KSUtil::checkType( context, *it, KSValue::ListType, false ) )
    {
      if ( !kspreadfunc_and_helper( context, (*it)->listValue(), first ) )
	return false;
    }
    else if ( KSUtil::checkType( context, *it, KSValue::BoolType, true ) )
      first = first && (*it)->boolValue();
    else
      return false;
  }

  return true;
}

static bool kspreadfunc_and( KSContext& context )
{
  bool first = true;
  bool b = kspreadfunc_and_helper( context, context.value()->listValue(), first );

  if ( b )
    context.setValue( new KSValue( first ) );

  return b;
}

static bool kspreadfunc_nand( KSContext& context )
{
  bool first = true;
  bool b = kspreadfunc_and_helper( context, context.value()->listValue(), first );

  if ( b )
    context.setValue( new KSValue( !first ) );

  return b;
}


static bool kspreadfunc_if( KSContext& context )
{
    QValueList<KSValue::Ptr>& args = context.value()->listValue();

    if ( !KSUtil::checkArgumentsCount( context, 3, "if", true ) )
	return false;

    if ( !KSUtil::checkType( context, args[0], KSValue::BoolType, true ) )
	return false;

    if (  args[0]->boolValue() == true )
	context.setValue( new KSValue( *(args[1]) ) );
    else
	context.setValue( new KSValue( *(args[2]) ) );

    return true;
}

static bool kspreadfunc_left( KSContext& context )
{
    QValueList<KSValue::Ptr>& args = context.value()->listValue();

    if ( !KSUtil::checkArgumentsCount( context, 2, "left", true ) )
	return false;

    if ( !KSUtil::checkType( context, args[0], KSValue::StringType, true ) )
	return false;
    int nb;
    if( KSUtil::checkType( context, args[1], KSValue::DoubleType, false ) )
	nb = (int) args[1]->doubleValue();
    else if( KSUtil::checkType( context, args[1], KSValue::IntType, true ) )
	nb = args[1]->intValue();
    else
	return false;

    QString tmp = args[0]->stringValue().left(nb);
    context.setValue( new KSValue( tmp ) ); 	
    return true;
}

static bool kspreadfunc_right( KSContext& context )
{
    QValueList<KSValue::Ptr>& args = context.value()->listValue();

    if ( !KSUtil::checkArgumentsCount( context, 2, "right", true ) )
	return false;

    if ( !KSUtil::checkType( context, args[0], KSValue::StringType, true ) )
	return false;
    int nb;
    if( KSUtil::checkType( context, args[1], KSValue::DoubleType, false ) )
	nb = (int) args[1]->doubleValue();
    else if( KSUtil::checkType( context, args[1], KSValue::IntType, true ) )
	nb = args[1]->intValue();
    else
	return false;

    QString tmp = args[0]->stringValue().right(nb);
    context.setValue( new KSValue(tmp)); 	
    return true;
}

static bool kspreadfunc_upper( KSContext& context )
{
    QValueList<KSValue::Ptr>& args = context.value()->listValue();

    if ( !KSUtil::checkArgumentsCount( context, 1, "upper", true ) )
	return false;

    if ( !KSUtil::checkType( context, args[0], KSValue::StringType, true ) )
	return false;

    QString tmp = args[0]->stringValue().upper();
    context.setValue( new KSValue( tmp ) ); 	
    return true;
}

static bool kspreadfunc_lower( KSContext& context )
{
    QValueList<KSValue::Ptr>& args = context.value()->listValue();

    if ( !KSUtil::checkArgumentsCount( context, 1, "lower", true ) )
	return false;

    if ( !KSUtil::checkType( context, args[0], KSValue::StringType, true ) )
	return false;

    QString tmp = args[0]->stringValue().lower();
    context.setValue( new KSValue( tmp ) ); 	
    return true;
}

static bool kspreadfunc_find( KSContext& context )
{
    QValueList<KSValue::Ptr>& args = context.value()->listValue();

    if ( !KSUtil::checkArgumentsCount( context, 2, "find", true ) )
	return false;

    if ( !KSUtil::checkType( context, args[0], KSValue::StringType, true ) )
	return false;

    if ( !KSUtil::checkType( context, args[1], KSValue::StringType, true ) )
	return false;

    QString string_find = args[0]->stringValue();
    QString string = args[1]->stringValue();
    context.setValue( new KSValue( string.find(string_find) ) ); 	
    return true;
}



static bool kspreadfunc_mid( KSContext& context )
{
    QValueList<KSValue::Ptr>& args = context.value()->listValue();

    uint len = 0xffffffff;
    if ( KSUtil::checkArgumentsCount( context, 3, "mid", false ) )
    {
	if( KSUtil::checkType( context, args[2], KSValue::DoubleType, false ) )
	    len = (uint) args[2]->doubleValue();
	else if( KSUtil::checkType( context, args[2], KSValue::IntType, true ) )
	    len = (uint) args[2]->intValue();
	else
	    return false;
    }
    else if ( !KSUtil::checkArgumentsCount( context, 2, "mid", true ) )
	return false;

    if ( !KSUtil::checkType( context, args[0], KSValue::StringType, true ) )
	return false;
    int pos;
    if( KSUtil::checkType( context, args[1], KSValue::DoubleType, false ) )
	pos = (int) args[1]->doubleValue();
    else if( KSUtil::checkType( context, args[1], KSValue::IntType, true ) )
	pos = args[1]->intValue();
    else
	return false;

    QString tmp = args[0]->stringValue().mid( pos, len );
    context.setValue( new KSValue(tmp)); 	
    return true;
}

static bool kspreadfunc_len( KSContext& context )
{
  QValueList<KSValue::Ptr>& args = context.value()->listValue();

  if ( !KSUtil::checkArgumentsCount( context, 1, "len", true ) )
    return false;

  if ( !KSUtil::checkType( context, args[0], KSValue::StringType, true ) )
    return false;

  int nb=args[0]->stringValue().length();
  context.setValue( new KSValue(nb));
  return true;
}

static bool kspreadfunc_EXACT( KSContext& context )
{
  QValueList<KSValue::Ptr>& args = context.value()->listValue();

  if ( !KSUtil::checkArgumentsCount( context, 2, "EXACT", true ) )
    return false;

  if ( !KSUtil::checkType( context, args[0], KSValue::StringType, true ) )
    return false;
  if ( !KSUtil::checkType( context, args[1], KSValue::StringType, true ) )
    return false;
  bool toto=true;
  if(args[1]->stringValue()==args[0]->stringValue())
  	toto=true;
  else
  	toto=false;
  context.setValue( new KSValue(toto)); 	
  return true;
}


static bool kspreadfunc_STXT( KSContext& context )
{
  QValueList<KSValue::Ptr>& args = context.value()->listValue();

  if ( !KSUtil::checkArgumentsCount( context, 3, "STXT", true ) )
    return false;

  if ( !KSUtil::checkType( context, args[0], KSValue::StringType, true ) )
    return false;
  if ( !KSUtil::checkType( context, args[1], KSValue::DoubleType, true ) )
    return false;
  if ( !KSUtil::checkType( context, args[2], KSValue::DoubleType, true ) )
    return false;
  QString tmp;
  tmp=args[0]->stringValue().right(args[0]->stringValue().length()-(int)args[1]->doubleValue());
  tmp=tmp.left(  (int)args[2]->doubleValue());
  context.setValue( new KSValue(tmp)); 	
  return true;
}

static bool kspreadfunc_ENT( KSContext& context )
{
  QValueList<KSValue::Ptr>& args = context.value()->listValue();

  if ( !KSUtil::checkArgumentsCount( context, 1, "ENT", true ) )
    return false;
  if ( !KSUtil::checkType( context, args[0], KSValue::DoubleType, true ) )
    return false;

  context.setValue( new KSValue((int)args[0]->doubleValue())); 	
  return true;
}

static bool kspreadfunc_PI( KSContext& context )
{
    // QValueList<KSValue::Ptr>& args = context.value()->listValue();

    if ( !KSUtil::checkArgumentsCount( context, 0, "PI", true ) )
	return false;

    context.setValue( new KSValue(M_PI));
    return true;
}

static bool kspreadfunc_REPT( KSContext& context )
{
  QValueList<KSValue::Ptr>& args = context.value()->listValue();

  if ( !KSUtil::checkArgumentsCount( context, 2, "REPT", true ) )
    return false;

  if ( !KSUtil::checkType( context, args[0], KSValue::StringType, true ) )
    return false;
  if( !KSUtil::checkType( context, args[1], KSValue::DoubleType, true ) )
    return false;

  int nb=(int) args[1]->doubleValue();
  QString tmp=args[0]->stringValue();
  QString tmp1;
  for (int i=0 ;i<nb;i++)
  	tmp1+=tmp;
  context.setValue( new KSValue(tmp1)); 	
  return true;
}

static bool kspreadfunc_islogic( KSContext& context )
{
  QValueList<KSValue::Ptr>& args = context.value()->listValue();

  if ( !KSUtil::checkArgumentsCount( context, 1, "ISLOGIC", true ) )
    return false;

  bool logic;
  if ( KSUtil::checkType( context, args[0], KSValue::BoolType, true ) )
  	logic=true;
  else
  	logic=false;
 	

  context.setValue( new KSValue(logic));
  return true;
}

static bool kspreadfunc_istext( KSContext& context )
{
  QValueList<KSValue::Ptr>& args = context.value()->listValue();

  if ( !KSUtil::checkArgumentsCount( context, 1, "ISTEXT", true ) )
    return false;

  bool logic;
  if ( KSUtil::checkType( context, args[0], KSValue::StringType, true ) )
  	logic=true;
  else
  	logic=false;


  context.setValue( new KSValue(logic));
  return true;
}

static bool kspreadfunc_isnum( KSContext& context )
{
  QValueList<KSValue::Ptr>& args = context.value()->listValue();

  if ( !KSUtil::checkArgumentsCount( context, 1, "ISNUM", true ) )
    return false;

  bool logic;
  if ( KSUtil::checkType( context, args[0], KSValue::DoubleType, true ) )
  	logic=true;
  else
  	logic=false;
 	

  context.setValue( new KSValue(logic));
  return true;
}

static bool kspreadfunc_pow( KSContext& context )
{
  QValueList<KSValue::Ptr>& args = context.value()->listValue();

  if ( !KSUtil::checkArgumentsCount( context, 2, "pow",true ) )
    return false;

  if ( !KSUtil::checkType( context, args[0], KSValue::DoubleType, true ) )
    return false;

  if ( !KSUtil::checkType( context, args[1], KSValue::DoubleType, true ) )
    return false;

  context.setValue( new KSValue( pow( args[0]->doubleValue(),args[1]->doubleValue() ) ) );

  return true;
}

static bool kspreadfunc_mod( KSContext& context )
{
  double result=0;
  QValueList<KSValue::Ptr>& args = context.value()->listValue();

  if ( !KSUtil::checkArgumentsCount( context, 2, "MOD",true ) )
    return false;

  if ( !KSUtil::checkType( context, args[0], KSValue::DoubleType, true ) )
    return false;

  if ( !KSUtil::checkType( context, args[1], KSValue::DoubleType, true ) )
    return false;

  result=(int)args[0]->doubleValue() % (int)args[1]->doubleValue();
  context.setValue( new KSValue(  result  ) );

  return true;
}



static bool kspreadfunc_date( KSContext& context )
{
  QValueList<KSValue::Ptr>& args = context.value()->listValue();

  if ( !KSUtil::checkArgumentsCount( context,3, "date",true ) )
    return false;

  if ( !KSUtil::checkType( context, args[0], KSValue::IntType, true ) )
    return false;

  if ( !KSUtil::checkType( context, args[1], KSValue::IntType, true ) )
    return false;

  if ( !KSUtil::checkType( context, args[2], KSValue::IntType, true ) )
    return false;

  context.setValue( new KSValue(KGlobal::locale()->formatDate(QDate(args[0]->intValue(),
  			args[1]->intValue(),args[2]->intValue()) )));

  return true;
}

static bool kspreadfunc_day( KSContext& context )
{
  QValueList<KSValue::Ptr>& args = context.value()->listValue();
  QString tmp;
  if ( !KSUtil::checkArgumentsCount( context,1, "day",true ) )
    return false;

  if ( !KSUtil::checkType( context, args[0], KSValue::IntType, true ) )
    return false;

  if(KGlobal::locale()->WeekDayName(args[0]->intValue()).isNull())
        tmp="Err";
  else
        tmp= KGlobal::locale()->WeekDayName(args[0]->intValue());

  //context.setValue( new KSValue(KGlobal::locale()->WeekDayName(args[0]->intValue())));
  context.setValue( new KSValue(tmp));
  return true;
}

static bool kspreadfunc_month( KSContext& context )
{
  QValueList<KSValue::Ptr>& args = context.value()->listValue();
  QString tmp;
  if ( !KSUtil::checkArgumentsCount( context,1, "month",true ) )
    return false;

  if ( !KSUtil::checkType( context, args[0], KSValue::IntType, true ) )
    return false;

  if(KGlobal::locale()->MonthName(args[0]->intValue()).isNull())
        tmp="Err";
  else
        tmp=KGlobal::locale()->MonthName(args[0]->intValue());

  context.setValue( new KSValue(tmp));
  //context.setValue( new KSValue(KGlobal::locale()->MonthName(args[0]->intValue())));

  return true;
}

static bool kspreadfunc_time( KSContext& context )
{
  QValueList<KSValue::Ptr>& args = context.value()->listValue();

  if ( !KSUtil::checkArgumentsCount( context,3, "time",true ) )
    return false;

  if ( !KSUtil::checkType( context, args[0], KSValue::IntType, true ) )
    return false;

  if ( !KSUtil::checkType( context, args[1], KSValue::IntType, true ) )
    return false;

  if ( !KSUtil::checkType( context, args[2], KSValue::IntType, true ) )
    return false;

  context.setValue( new KSValue(KGlobal::locale()->formatTime(QTime(args[0]->intValue(),
  			args[1]->intValue(),args[2]->intValue()),true )));

  return true;
}

static bool kspreadfunc_currentDate( KSContext& context )
{
    // QValueList<KSValue::Ptr>& args = context.value()->listValue();

    if ( !KSUtil::checkArgumentsCount( context,0, "currentDate",true ) )
	return false;

    context.setValue( new KSValue(KGlobal::locale()->formatDate(QDate::currentDate())));

    return true;
}

static bool kspreadfunc_currentTime( KSContext& context )
{
    // QValueList<KSValue::Ptr>& args = context.value()->listValue();

    if ( !KSUtil::checkArgumentsCount( context,0, "currentTime",true ) )
	return false;

    context.setValue( new KSValue(KGlobal::locale()->formatTime(QTime::currentTime())));

    return true;
}

static bool kspreadfunc_currentDateTime( KSContext& context )
{
    // QValueList<KSValue::Ptr>& args = context.value()->listValue();

    if ( !KSUtil::checkArgumentsCount( context,0, "currentDateTime",true ) )
	return false;

    context.setValue( new KSValue(KGlobal::locale()->formatDateTime(QDateTime::currentDateTime())));

    return true;
}

static bool kspreadfunc_dayOfYear( KSContext& context )
{
  QValueList<KSValue::Ptr>& args = context.value()->listValue();

  if ( !KSUtil::checkArgumentsCount( context,3, "dayOfYear",true ) )
    return false;

  if ( !KSUtil::checkType( context, args[0], KSValue::IntType, true ) )
    return false;

  if ( !KSUtil::checkType( context, args[1], KSValue::IntType, true ) )
    return false;

  if ( !KSUtil::checkType( context, args[2], KSValue::IntType, true ) )
    return false;

  context.setValue( new KSValue(QDate(args[0]->intValue(),
  			args[1]->intValue(),args[2]->intValue()).dayOfYear() ));

  return true;
}

double fact(double val,double end)
{
/* fact =i*(i-1)*(i-2)*...*1 */
if(val <0||end<0)
        return (-1);
if(val==0)
        return(1);
else if(val==end)
        return(1);
        /*val==end => you don't multiplie it */
else
        return(val*fact((double)(val-1),end));

}


static bool kspreadfunc_fact( KSContext& context )
{
  double result;
  QString tmp;
  QValueList<KSValue::Ptr>& args = context.value()->listValue();

  if ( !KSUtil::checkArgumentsCount( context,1, "fact",true ) )
    return false;

  if ( !KSUtil::checkType( context, args[0], KSValue::IntType, true ) )
    return false;

  result=fact((double)args[0]->intValue(),0);
  //In fact function val must be positive
  tmp="Err";
  if(result==-1)
        context.setValue( new KSValue(tmp));
  else
        context.setValue( new KSValue(result ));

  return true;
}

static bool kspreadfunc_arrang( KSContext& context )
{ /* arrang : fact(n)/(fact(n-m) */
  double result;
  QString tmp;
  QValueList<KSValue::Ptr>& args = context.value()->listValue();

  if ( !KSUtil::checkArgumentsCount( context,2, "PERMUT",true ) )
    return false;

  if ( !KSUtil::checkType( context, args[0], KSValue::IntType, true ) )
    return false;

  if ( !KSUtil::checkType( context, args[1], KSValue::IntType, true ) )
    return false;

  tmp="Err";
  if((double)args[0]->intValue()<(double)args[1]->intValue())
          context.setValue( new KSValue(tmp ));

  else if((double)args[1]->intValue()<0)
          context.setValue( new KSValue(tmp ));

  else
        {
        result=fact((double)args[0]->intValue(),
        ((double)args[0]->intValue()-(double)args[1]->intValue()));
        //In fact function val must be positive

        if(result==-1)
                context.setValue( new KSValue(tmp));
        else
                context.setValue( new KSValue(result ));
        }
  return true;
}

static bool kspreadfunc_combin( KSContext& context )
{ /*combin : fact(n)/(fact(n-m)*fact(m)) */
  double result;
  QString tmp;
  QValueList<KSValue::Ptr>& args = context.value()->listValue();

  if ( !KSUtil::checkArgumentsCount( context,2, "COMBIN",true ) )
    return false;

  if ( !KSUtil::checkType( context, args[0], KSValue::IntType, true ) )
    return false;

  if ( !KSUtil::checkType( context, args[1], KSValue::IntType, true ) )
    return false;

  tmp="Err";
  if((double)args[0]->intValue()<(double)args[1]->intValue())
          context.setValue( new KSValue(tmp ));

  else if((double)args[1]->intValue()<0)
          context.setValue( new KSValue(tmp ));

  else
        {
        result=(fact((double)args[0]->intValue(),
        ((double)args[0]->intValue()-(double)args[1]->intValue()))
        /fact((double)args[1]->intValue(),0));
        //In fact function val must be positive

        if(result==-1)
                context.setValue( new KSValue(tmp));
        else
                context.setValue( new KSValue(result ));
        }
  return true;
}

static bool kspreadfunc_bino( KSContext& context )
{
  double result=0;
  QString tmp;
  QValueList<KSValue::Ptr>& args = context.value()->listValue();

  if ( !KSUtil::checkArgumentsCount( context,3, "BINO",true ) )
    return false;

  if ( !KSUtil::checkType( context, args[0], KSValue::DoubleType, true ) )
    return false;

  if ( !KSUtil::checkType( context, args[1], KSValue::DoubleType, true ) )
    return false;

  if ( !KSUtil::checkType( context, args[2], KSValue::DoubleType, true ) )
    return false;

  tmp="Err";
  if(args[0]->doubleValue()<args[1]->doubleValue())
          context.setValue( new KSValue(tmp ));

  else if(args[1]->doubleValue()<0)
          context.setValue( new KSValue(tmp ));

  // 0<proba<1
  else if((args[2]->doubleValue()<0)||(args[2]->doubleValue()>1))
  	  context.setValue( new KSValue(tmp ));
  else
        {
        result=(fact(args[0]->doubleValue(),
        (args[0]->doubleValue()-args[1]->doubleValue()))
        /fact(args[1]->doubleValue(),0));
        //In fact function val must be positive

        if(result==-1)
                context.setValue( new KSValue(tmp));
        else
                {
              result=result*pow(args[2]->doubleValue(),(int)args[1]->doubleValue())*
               	pow((1-args[2]->doubleValue()),((int)args[0]->doubleValue()-
                ((int)args[1]->doubleValue())));
                context.setValue( new KSValue(result ));
                }
        }
  return true;


}


static bool kspreadfunc_bino_inv( KSContext& context )
{
  double result=0;
  QString tmp;
  QValueList<KSValue::Ptr>& args = context.value()->listValue();

  if ( !KSUtil::checkArgumentsCount( context,3, "INVBINO",true ) )
    return false;

  if ( !KSUtil::checkType( context, args[0], KSValue::DoubleType, true ) )
    return false;

  if ( !KSUtil::checkType( context, args[1], KSValue::DoubleType, true ) )
    return false;

  if ( !KSUtil::checkType( context, args[2], KSValue::DoubleType, true ) )
    return false;

  tmp="Err";
  if(args[0]->doubleValue()<args[1]->doubleValue())
          context.setValue( new KSValue(tmp ));

  else if(args[1]->doubleValue()<0)
          context.setValue( new KSValue(tmp ));

  // 0<proba<1
  else if((args[2]->doubleValue()<0)||(args[2]->doubleValue()>1))
  	  context.setValue( new KSValue(tmp ));
  else
        {
        result=(fact(args[0]->doubleValue(),
        (args[0]->doubleValue()-args[1]->doubleValue()))
        /fact(args[1]->doubleValue(),0));
        //In fact function val must be positive

        if(result==-1)
                context.setValue( new KSValue(tmp));
        else
                {
          result=result*pow((1-args[2]->doubleValue()),((int)args[0]->doubleValue()-
              (int)args[1]->doubleValue()))*pow(args[2]->doubleValue(),(
                (int)args[1]->doubleValue()));
                context.setValue( new KSValue(result ));
                }
        }
  return true;


}

static bool kspreadfunc_fv( KSContext& context )
{
/* Returns future value, given current value, interest rate and time */
  QValueList<KSValue::Ptr>& args = context.value()->listValue();

  if ( !KSUtil::checkArgumentsCount( context, 3, "FV", true ) )
    return false;

  if ( !KSUtil::checkType( context, args[0], KSValue::DoubleType, true ) )
    return false;
  if ( !KSUtil::checkType( context, args[1], KSValue::DoubleType, true ) )
    return false;
  if ( !KSUtil::checkType( context, args[2], KSValue::DoubleType, true ) )
    return false;
  double present = args[0]->doubleValue();
  double interest = args[1]->doubleValue();
  double periods = args[2]->doubleValue();


  context.setValue( new KSValue( present * pow(1+interest, periods)));
  return true;
}

static bool kspreadfunc_compound( KSContext& context )
{
/* Returns value after compounded interest, given principal, rate, periods
per year and year */
  QValueList<KSValue::Ptr>& args = context.value()->listValue();

  if ( !KSUtil::checkArgumentsCount( context, 4, "compound", true ) )
    return false;

  if ( !KSUtil::checkType( context, args[0], KSValue::DoubleType, true ) )
    return false;
  if ( !KSUtil::checkType( context, args[1], KSValue::DoubleType, true ) )
    return false;
  if ( !KSUtil::checkType( context, args[2], KSValue::DoubleType, true ) )
    return false;
  if ( !KSUtil::checkType( context, args[3], KSValue::DoubleType, true ) )
    return false;
  double principal = args[0]->doubleValue();
  double interest = args[1]->doubleValue();
  double periods = args[2]->doubleValue();
  double years = args[3]->doubleValue();

  context.setValue( new KSValue( principal * pow(1+(interest/periods),
periods*years)));

  return true;
}

static bool kspreadfunc_continuous( KSContext& context )
{
/* Returns value after continuous compounding of interest, given prinicpal,
rate and years */
  QValueList<KSValue::Ptr>& args = context.value()->listValue();

  if ( !KSUtil::checkArgumentsCount( context, 3, "continuous", true ) )
    return false;

  if ( !KSUtil::checkType( context, args[0], KSValue::DoubleType, true ) )
    return false;
  if ( !KSUtil::checkType( context, args[1], KSValue::DoubleType, true ) )
    return false;
  if ( !KSUtil::checkType( context, args[2], KSValue::DoubleType, true ) )
    return false;
  double principal = args[0]->doubleValue();
  double interest = args[1]->doubleValue();
  double years = args[2]->doubleValue();


  context.setValue( new KSValue( principal * exp(interest * years)));
  return true;
}

static bool kspreadfunc_pv( KSContext& context )
{
/* Returns presnt value, given future value, interest rate and years */
  QValueList<KSValue::Ptr>& args = context.value()->listValue();

  if ( !KSUtil::checkArgumentsCount( context, 3, "PV", true ) )
    return false;

  if ( !KSUtil::checkType( context, args[0], KSValue::DoubleType, true ) )
    return false;
  if ( !KSUtil::checkType( context, args[1], KSValue::DoubleType, true ) )
    return false;
  if ( !KSUtil::checkType( context, args[2], KSValue::DoubleType, true ) )
    return false;
  double future = args[0]->doubleValue();
  double interest = args[1]->doubleValue();
  double periods = args[2]->doubleValue();


  context.setValue( new KSValue( future / pow(1+interest, periods)));
  return true;
}

static bool kspreadfunc_pv_annuity( KSContext& context )
{
/* Returns present value of an annuity or cash flow, given payment,
interest rate,
periods, initial amount and whether payments are made at the start (TRUE)
or end of a period */

  QValueList<KSValue::Ptr>& args = context.value()->listValue();

  if ( !KSUtil::checkArgumentsCount( context, 5, "PV_annuity", true ) )
    return false;

  if ( !KSUtil::checkType( context, args[0], KSValue::DoubleType, true ) )
    return false;
  if ( !KSUtil::checkType( context, args[1], KSValue::DoubleType, true ) )
    return false;
  if ( !KSUtil::checkType( context, args[2], KSValue::DoubleType, true ) )
    return false;
  if ( !KSUtil::checkType( context, args[3], KSValue::DoubleType, true ) )
    return false;
  if ( !KSUtil::checkType( context, args[4], KSValue::BoolType, true ) )
    return false;
  double amount= args[0]->doubleValue();
  double interest = args[1]->doubleValue();
  double periods = args[2]->doubleValue();
  double initial = args[3]->doubleValue();
  bool start = args[4]->boolValue();

  if (start) periods +=1;
  double first_term = 1/interest;
  double second_term = 1/(interest*pow(1+interest, periods)) ;

  context.setValue( new KSValue( initial + amount * (first_term -
second_term)  ));
  return true;
}


static bool kspreadfunc_fv_annuity( KSContext& context )
{
/* Returns future value of an annuity or cash flow, given payment, interest
rate,
periods, initial amount and whether payments are made at the start (TRUE)
or end of a period */


  QValueList<KSValue::Ptr>& args = context.value()->listValue();

  if ( !KSUtil::checkArgumentsCount( context, 5, "FV_annuity", true ) )
    return false;

  if ( !KSUtil::checkType( context, args[0], KSValue::DoubleType, true ) )
    return false;
  if ( !KSUtil::checkType( context, args[1], KSValue::DoubleType, true ) )
    return false;
  if ( !KSUtil::checkType( context, args[2], KSValue::DoubleType, true ) )
    return false;
  if ( !KSUtil::checkType( context, args[3], KSValue::DoubleType, true ) )
    return false;
  if ( !KSUtil::checkType( context, args[4], KSValue::BoolType, true ) )
    return false;
  double amount= args[0]->doubleValue();
  double interest = args[1]->doubleValue();
  double periods = args[2]->doubleValue();
  double initial = args[3]->doubleValue();
  bool start = args[4]->boolValue();

  if (start) periods +=1;
  double first_term = pow(1+interest, periods)/interest ;
  double second_term = 1/interest;
  initial =  initial * pow(1+interest, periods);
  context.setValue( new KSValue( initial + amount * (first_term -
second_term)  ));
  return true;
}


static bool kspreadfunc_effective( KSContext& context )
{
/* Returns effective interest rate given nominal rate and periods per year */

  QValueList<KSValue::Ptr>& args = context.value()->listValue();

  if ( !KSUtil::checkArgumentsCount( context, 2, "effective", true ) )
    return false;

  if ( !KSUtil::checkType( context, args[0], KSValue::DoubleType, true ) )
    return false;
  if ( !KSUtil::checkType( context, args[1], KSValue::DoubleType, true ) )
    return false;
  double nominal = args[0]->doubleValue();
  double periods = args[1]->doubleValue();


  context.setValue( new KSValue(  pow(1+nominal/periods, periods)-1));
  return true;
}


static bool kspreadfunc_nominal( KSContext& context )
{
/* Returns nominal interest rate given effective rate and periods per year */

  QValueList<KSValue::Ptr>& args = context.value()->listValue();

  if ( !KSUtil::checkArgumentsCount( context, 2, "nominal", true ) )
    return false;

  if ( !KSUtil::checkType( context, args[0], KSValue::DoubleType, true ) )
    return false;
  if ( !KSUtil::checkType( context, args[1], KSValue::DoubleType, true ) )
    return false;
  double effective = args[0]->doubleValue();
  double periods = args[1]->doubleValue();


  context.setValue( new KSValue(  periods *( pow(1+effective, 1/periods)-1)
));
  return true;
}


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

    qDebug("x=%i",x);
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

static bool kspreadfunc_app( KSContext& context )
{
    KSpreadDoc* doc = ((KSpreadInterpreter*)context.interpreter())->document();

    // ############## Dont return the document here, but we have no app iface currently
    context.setValue( new KSValue( new KSProxy( kapp->dcopClient()->appId(), doc->dcopObject()->objId() ) ) );

    return TRUE;
}

static bool kspreadfunc_doc( KSContext& context )
{
    KSpreadDoc* doc = ((KSpreadInterpreter*)context.interpreter())->document();

    context.setValue( new KSValue( new KSProxy( kapp->dcopClient()->appId(), doc->dcopObject()->objId() ) ) );

    return TRUE;
}

static bool kspreadfunc_map( KSContext& context )
{
    KSpreadMap* map = ((KSpreadInterpreter*)context.interpreter())->document()->map();

    context.setValue( new KSValue( new KSProxy( kapp->dcopClient()->appId(), map->dcopObject()->objId() ) ) );

    return TRUE;
}

static bool kspreadfunc_table( KSContext& context )
{
    KSpreadTable* table = ((KSpreadInterpreter*)context.interpreter())->table();

    context.setValue( new KSValue( new KSProxy( kapp->dcopClient()->appId(), table->dcopObject()->objId() ) ) );

    return TRUE;
}

static KSModule::Ptr kspreadCreateModule_KSpread( KSInterpreter* interp )
{
  KSModule::Ptr module = new KSModule( interp, "kspread" );

  module->addObject( "cos", new KSValue( new KSBuiltinFunction( module, "cos", kspreadfunc_cos ) ) );
  module->addObject( "sin", new KSValue( new KSBuiltinFunction( module, "sin", kspreadfunc_sin ) ) );
  module->addObject( "sum", new KSValue( new KSBuiltinFunction( module, "sum", kspreadfunc_sum ) ) );
  module->addObject( "sqrt", new KSValue( new KSBuiltinFunction( module, "sqrt", kspreadfunc_sqrt ) ) );
  module->addObject( "fabs", new KSValue( new KSBuiltinFunction( module, "fabs", kspreadfunc_fabs ) ) );
  module->addObject( "floor", new KSValue( new KSBuiltinFunction( module, "floor", kspreadfunc_floor ) ) );
  module->addObject( "ceil", new KSValue( new KSBuiltinFunction( module, "ceil", kspreadfunc_ceil ) ) );
  module->addObject( "tan", new KSValue( new KSBuiltinFunction( module, "tan", kspreadfunc_tan ) ) );
  module->addObject( "exp", new KSValue( new KSBuiltinFunction( module, "exp", kspreadfunc_exp ) ) );
  module->addObject( "ln", new KSValue( new KSBuiltinFunction( module, "ln", kspreadfunc_ln ) ) );
  module->addObject( "atan", new KSValue( new KSBuiltinFunction( module, "atan", kspreadfunc_atan ) ) );
  module->addObject( "asin", new KSValue( new KSBuiltinFunction( module, "asin", kspreadfunc_asin ) ) );
  module->addObject( "acos", new KSValue( new KSBuiltinFunction( module, "acos", kspreadfunc_acos ) ) );
  module->addObject( "log", new KSValue( new KSBuiltinFunction( module, "log", kspreadfunc_log ) ) );
  module->addObject( "max", new KSValue( new KSBuiltinFunction( module, "max", kspreadfunc_max ) ) );
  module->addObject( "min", new KSValue( new KSBuiltinFunction( module, "min", kspreadfunc_min ) ) );
  module->addObject( "cosh", new KSValue( new KSBuiltinFunction( module, "cosh", kspreadfunc_cosh ) ) );
  module->addObject( "sinh", new KSValue( new KSBuiltinFunction( module, "sinh", kspreadfunc_sinh ) ) );
  module->addObject( "tanh", new KSValue( new KSBuiltinFunction( module, "tanh", kspreadfunc_tanh ) ) );
  module->addObject( "acosh", new KSValue( new KSBuiltinFunction( module, "acosh", kspreadfunc_acosh ) ) );
  module->addObject( "asinh", new KSValue( new KSBuiltinFunction( module, "asinh", kspreadfunc_asinh ) ) );
  module->addObject( "atanh", new KSValue( new KSBuiltinFunction( module, "atanh", kspreadfunc_atanh ) ) );
  module->addObject( "degree", new KSValue( new KSBuiltinFunction( module, "degree", kspreadfunc_degree ) ) );
  module->addObject( "radian", new KSValue( new KSBuiltinFunction( module, "radian", kspreadfunc_radian ) ) );
  module->addObject( "average", new KSValue( new KSBuiltinFunction( module, "average", kspreadfunc_average ) ) );
  module->addObject( "variance", new KSValue( new KSBuiltinFunction( module, "variance", kspreadfunc_variance) ) );
  module->addObject( "multiply", new KSValue( new KSBuiltinFunction( module, "multiply", kspreadfunc_mult) ) );
  module->addObject( "OR", new KSValue( new KSBuiltinFunction( module, "OR", kspreadfunc_or) ) );
    module->addObject( "AND", new KSValue( new KSBuiltinFunction( module, "AND", kspreadfunc_and) ) );
  module->addObject( "NOR", new KSValue( new KSBuiltinFunction( module, "NOR", kspreadfunc_nor) ) );
    module->addObject( "NAND", new KSValue( new KSBuiltinFunction( module, "NAND", kspreadfunc_nand) ) );

  module->addObject( "stddev", new KSValue( new KSBuiltinFunction( module, "stderr", kspreadfunc_stddev) ) );
  module->addObject( "join", new KSValue( new KSBuiltinFunction( module, "join", kspreadfunc_join) ) );
  module->addObject( "not", new KSValue( new KSBuiltinFunction( module, "not", kspreadfunc_not) ) );
  module->addObject( "if", new KSValue( new KSBuiltinFunction( module, "if", kspreadfunc_if) ) );
  module->addObject( "left", new KSValue( new KSBuiltinFunction( module, "left", kspreadfunc_left) ) );
  module->addObject( "right", new KSValue( new KSBuiltinFunction( module, "right", kspreadfunc_right) ) );
  module->addObject( "mid", new KSValue( new KSBuiltinFunction( module, "mid", kspreadfunc_mid) ) );
  module->addObject( "len", new KSValue( new KSBuiltinFunction( module, "len", kspreadfunc_len) ) );
  module->addObject( "EXACT", new KSValue( new KSBuiltinFunction( module, "EXACT", kspreadfunc_EXACT) ) );
  module->addObject( "STXT", new KSValue( new KSBuiltinFunction( module, "STXT", kspreadfunc_STXT) ) );
  module->addObject( "ENT", new KSValue( new KSBuiltinFunction( module, "ENT",kspreadfunc_ENT) ) );
  module->addObject( "PI", new KSValue( new KSBuiltinFunction( module, "PI",kspreadfunc_PI) ) );
  module->addObject( "REPT", new KSValue( new KSBuiltinFunction( module, "REPT",kspreadfunc_REPT) ) );
  module->addObject( "ISLOGIC", new KSValue( new KSBuiltinFunction( module,"ISLOGIC",kspreadfunc_islogic) ) );
  module->addObject( "ISTEXT", new KSValue( new KSBuiltinFunction( module,"ISTEXT",kspreadfunc_istext) ) );
  module->addObject( "ISNUM", new KSValue( new KSBuiltinFunction( module,"ISNUM",kspreadfunc_isnum) ) );
  module->addObject( "cell", new KSValue( new KSBuiltinFunction( module,"cell",kspreadfunc_cell) ) );
  module->addObject( "select", new KSValue( new KSBuiltinFunction( module,"select",kspreadfunc_select) ) );
  module->addObject( "application", new KSValue( new KSBuiltinFunction( module, "application", kspreadfunc_app ) ) );
  module->addObject( "document", new KSValue( new KSBuiltinFunction( module, "document", kspreadfunc_doc ) ) );
  module->addObject( "map", new KSValue( new KSBuiltinFunction( module, "map", kspreadfunc_map ) ) );
  module->addObject( "table", new KSValue( new KSBuiltinFunction( module, "table", kspreadfunc_table ) ) );
  module->addObject( "pow", new KSValue( new KSBuiltinFunction( module,"pow",kspreadfunc_pow) ) );
  module->addObject( "MOD", new KSValue( new KSBuiltinFunction( module,"MOD",kspreadfunc_mod) ) );
  module->addObject( "date", new KSValue( new KSBuiltinFunction( module,"date",kspreadfunc_date) ) );
  module->addObject( "day", new KSValue( new KSBuiltinFunction( module,"day",kspreadfunc_day) ) );
  module->addObject( "month", new KSValue( new KSBuiltinFunction( module,"month",kspreadfunc_month) ) );
  module->addObject( "time", new KSValue( new KSBuiltinFunction( module,"time",kspreadfunc_time) ) );
  module->addObject( "currentTime", new KSValue( new KSBuiltinFunction( module,"currentTime",kspreadfunc_currentTime) ) );
  module->addObject( "currentDate", new KSValue( new KSBuiltinFunction( module,"currentDate",kspreadfunc_currentDate) ) );
  module->addObject( "currentDateTime", new KSValue( new KSBuiltinFunction( module,"currentDateTime",kspreadfunc_currentDateTime) ) );
  module->addObject( "dayOfYear", new KSValue( new KSBuiltinFunction( module,"dayOfYear",kspreadfunc_dayOfYear) ) );
  module->addObject( "fact", new KSValue( new KSBuiltinFunction( module,"fact",kspreadfunc_fact) ) );
  module->addObject( "COMBIN", new KSValue( new KSBuiltinFunction( module,"COMBIN",kspreadfunc_combin) ) );
  module->addObject( "PERMUT", new KSValue( new KSBuiltinFunction( module,"PERMUT",kspreadfunc_arrang) ) );
  module->addObject( "BINO", new KSValue( new KSBuiltinFunction( module,"BINO",kspreadfunc_bino) ) );
  module->addObject( "INVBINO", new KSValue( new KSBuiltinFunction( module,"INVBINO",kspreadfunc_bino_inv) ) );
  module->addObject( "lower", new KSValue( new KSBuiltinFunction( module,"lower",kspreadfunc_lower) ) );
  module->addObject( "upper", new KSValue( new KSBuiltinFunction( module,"upper",kspreadfunc_upper) ) );
  module->addObject( "find", new KSValue( new KSBuiltinFunction( module,"find",kspreadfunc_find) ) );
  module->addObject( "compound", new KSValue( new KSBuiltinFunction(
module,"compound",kspreadfunc_compound) ) );
  module->addObject( "continuous", new KSValue( new KSBuiltinFunction(
module,"continuous",kspreadfunc_continuous) ) );
  module->addObject( "effective", new KSValue( new KSBuiltinFunction(
module,"effective",kspreadfunc_effective) ) );
  module->addObject( "nominal", new KSValue( new KSBuiltinFunction(
module,"nominal",kspreadfunc_nominal) ) );
  module->addObject( "FV", new KSValue( new KSBuiltinFunction(
module,"FV",kspreadfunc_fv) ) );
  module->addObject( "PV_annuity", new KSValue( new KSBuiltinFunction(
module,"PV_annuity",kspreadfunc_pv_annuity) ) );
  module->addObject( "PV", new KSValue( new KSBuiltinFunction(
module,"PV",kspreadfunc_pv) ) );
  module->addObject( "FV_annuity", new KSValue( new KSBuiltinFunction(
module,"FV_annuity",kspreadfunc_fv_annuity) ) );

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

    if ( cell->isDefault() )
      context.setValue( new KSValue( 0.0 ) );
    else if ( cell->isValue() )
      context.setValue( new KSValue( cell->valueDouble() ) );
    else if ( cell->isBool() )
      context.setValue( new KSValue( cell->valueBool() ) );
    else if ( cell->valueString().isEmpty() )
      context.setValue( new KSValue( 0.0 ) );
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
	  c = new KSValue( 0.0 );
	else if ( cell->isValue() )
	  c = new KSValue( cell->valueDouble() );
	else if ( cell->isBool() )
	  c = new KSValue( cell->valueBool() );
	else if ( cell->valueString().isEmpty() )
	  c = new KSValue( 0.0 );
	else
	  c = new KSValue( cell->valueString() );

	l->listValue().append( c );
      }
      v->listValue().append( l );
    }
    context.setValue( v );

    return true;
  }
  else
    ASSERT( 0 );

  // Never reached
  return false;
}

KSParseNode* KSpreadInterpreter::parse( KSContext& context, KSpreadTable* table, const QString& formula, QList<KSpreadDepend>& depends )
{
  // Create the parse tree.
  KSParser parser;
  if ( !parser.parse( formula, KSCRIPT_EXTENSION_KSPREAD ) )
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
