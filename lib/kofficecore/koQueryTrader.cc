#include "koQueryTrader.h"
#include "koIMR.h"

#include <qstring.h>
#include <qstrlist.h>

#include <kapp.h>
#include <opApplication.h>

/*******************************************************************
 *
 * Trader
 *
 *******************************************************************/

void koInitTrader()
{
  QString exec = kapp->kde_bindir();
  exec += "/kotrader";
  
  QStrList repos;
  repos.append( "IDL:omg.org/CosTrading/Lookup:1.0" );
  
  CORBA::ImplRepository_var imr;
  CORBA::Object_var obj = opapp_orb->resolve_initial_references( "ImplementationRepository" );
  imr = CORBA::ImplRepository::_narrow( obj );
  assert( !CORBA::is_nil( imr ) );
  
  imr_create( "Trader", "shared", exec, repos, imr );
}

void koQueryTrader( const char *_service_type, const char *_constr, unsigned int _count,
		    CosTrading::OfferSeq*& offers, CosTrading::OfferIterator_ptr& offer_itr,
		    CosTrading::PolicyNameSeq*& limits_applied )
{
  CORBA::Object_var obj = imr_activate( "Trader", "IDL:omg.org/CosTrading/Lookup:1.0" );
  assert( !CORBA::is_nil( obj ) );
  CosTrading::Lookup_var lookup = CosTrading::Lookup::_narrow( obj );
  assert( !CORBA::is_nil( lookup ) );

  CosTrading::ServiceTypeName_var type;
  CosTrading::Constraint_var constr;
  CosTrading::Lookup::Preference_var prefs;
  CosTrading::Lookup::SpecifiedProps desired;
  desired._d( CosTrading::Lookup::all );
  CosTrading::PolicySeq policyseq;
  policyseq.length( 0 );
  prefs = CORBA::string_dup( "" );
  constr = CORBA::string_dup( _constr );
  type = CORBA::string_dup( _service_type );
  lookup->query( type, constr, prefs, policyseq, desired, (CORBA::ULong)_count, offers, offer_itr, limits_applied );

  /***
   * DEBUG
   ****/
  if ( offers != 0L )
    cout << "Got " <<  offers->length() << " results" << endl;
  else
    cout << "Got no results" << endl;

  int max = offers->length();
  int i;
  for( i = 0; i < max; i++ )
  {
    CosTrading::PropertySeq &p = (*offers)[i].properties;
    int max2 = p.length();
    cout << max2 << " Properties" << endl;
    int j;
    for( j = 0; j < max2; j++ )
    {
      cout << "Property " << (const char*)p[j].name << " = ";
      char *s;
      if ( p[j].value >>= s )
      {  
	cout << (const char*)s;
	cout << endl;
	CORBA::string_free( s );
      }
      else
	cout << endl;
    }
  }
  /***
   * END DEBUG
   ****/
}

/*******************************************************************
 *
 * Component
 *
 *******************************************************************/

KoComponentEntry koParseComponentProperties( CosTrading::PropertySeq& p  )
{
  KoComponentEntry e;

  unsigned int max = p.length();
  for( unsigned int j = 0; j < max; j++ )
  {
    if ( strcmp( p[j].name, "Name" ) == 0 )
    {  
      char *s;
      if ( p[j].value >>= s )
      {  
	e.name = s;
	CORBA::string_free( s );
      }
    }
    else if ( strcmp( p[j].name, "Comment" ) == 0 )
    {  
      char *s;
      if ( p[j].value >>= s )
      {  
	e.comment = s;
	CORBA::string_free( s );
      }
    }
    else if ( strcmp( p[j].name, "Exec" ) == 0 )
    {  
      char *s;
      if ( p[j].value >>= s )
      {  
	e.exec = s;
	CORBA::string_free( s );
      }
    }
    else if ( strcmp( p[j].name, "ActivationMode" ) == 0 )
    {  
      char *s;
      if ( p[j].value >>= s )
      {  
	e.activationMode = s;
	CORBA::string_free( s );
      }
    }
    else if ( strcmp( p[j].name, "MiniIcon" ) == 0 )
    {
      char *s;
      if ( p[j].value >>= s )
      {  
	QString tmp = s;
	QPixmap pix;
	pix.loadFromData( (const uchar*)tmp.data(), tmp.length() );
	e.miniIcon = pix;
	CORBA::string_free( s );
      }	
    }
    else if ( strcmp( p[j].name, "Icon" ) == 0 )
    {
      char *s;
      if ( p[j].value >>= s )
      {  
	QString tmp = s;
	QPixmap pix;
	pix.loadFromData( (const uchar*)tmp.data(), tmp.length() );
	e.icon = pix;
	CORBA::string_free( s );
      }	
    }
    else if ( strcmp( p[j].name, "RepoID" ) == 0 )
    {  
      CosTrading::StringList lst;
      if ( p[j].value >>= lst )
      {  
	for( unsigned int l = 0; l < lst.length(); ++l )
	  e.repoID.append( lst[l].in() );
      }
    }
  }

  return e;
}

/*******************************************************************
 *
 * Document
 *
 *******************************************************************/

KoDocumentEntry koParseDocumentProperties( CosTrading::PropertySeq& p  )
{
  KoDocumentEntry e( koParseComponentProperties( p ) );

  unsigned int max = p.length();
  for( unsigned int j = 0; j < max; j++ )
  {
    if ( strcmp( p[j].name, "MimeTypes" ) == 0 )
    {  
      CosTrading::StringList lst;
      if ( p[j].value >>= lst )
      {  
	for( unsigned int l = 0; l < lst.length(); ++l )
	  e.mimeTypes.append( lst[l].in() );
      }
    }
  }
  
  return e;
}
  
vector<KoDocumentEntry> koQueryDocuments( const char *_constr, int _count )
{
  vector<KoDocumentEntry> lst;
  
  CosTrading::OfferSeq_var offers;
  CosTrading::OfferIterator_var offer_iter;
  CosTrading::PolicyNameSeq_var limits;
  koQueryTrader( "KOfficeDocument", _constr, _count, offers, offer_iter, limits );

  // DEBUG
  if ( offers->length() != 0 )
    cout << "Got " <<  offers->length() << " results" << endl;
  else
    cout << "Got no results" << endl;
  // END DEBUG

  lst.reserve( offers->length() );
  
  unsigned int max = offers->length();
  for( unsigned int i = 0; i < max; i++ )
  {
    lst.push_back( koParseDocumentProperties( (*offers)[i].properties ) );
    lst.back().reference = CORBA::Object::_duplicate( (*offers)[i].reference );
  }
  
  return lst;
}

KoDocumentEntry::KoDocumentEntry( const KoComponentEntry _e )
{
  comment = _e.comment;
  name = _e.name;
  exec = _e.exec;
  activationMode = _e.activationMode;
  repoID = _e.repoID;
  miniIcon = _e.miniIcon;
  icon = _e.icon;
}


/*******************************************************************
 *
 * Filters
 *
 *******************************************************************/

KoFilterEntry koParseFilterProperties( CosTrading::PropertySeq& p  )
{
  KoFilterEntry e( koParseComponentProperties( p ) );

  unsigned int max = p.length();
  for( unsigned int j = 0; j < max; j++ )
  {
    if ( strcmp( p[j].name, "Import" ) == 0 )
    {  
      char *s;
      if ( p[j].value >>= s )
      {  
	e.import = s;
	CORBA::string_free( s );
      }
    }
    else if ( strcmp( p[j].name, "ImportDescription" ) == 0 )
    {  
      char *s;
      if ( p[j].value >>= s )
      {  
	e.importDescription = s;
	CORBA::string_free( s );
      }
    }
    else if ( strcmp( p[j].name, "Export" ) == 0 )
    {  
      char *s;
      if ( p[j].value >>= s )
      {  
	e.export = s;
	CORBA::string_free( s );
      }
    }
    else if ( strcmp( p[j].name, "ExportDescription" ) == 0 )
    {  
      char *s;
      if ( p[j].value >>= s )
      {  
	e.exportDescription = s;
	CORBA::string_free( s );
      }
    }
  }
  
  return e;
}

vector<KoFilterEntry> koQueryFilters( const char *_constr, int _count )
{
  vector<KoFilterEntry> lst;
  
  CosTrading::OfferSeq_var offers;
  CosTrading::OfferIterator_var offer_iter;
  CosTrading::PolicyNameSeq_var limits;
  koQueryTrader( "KOfficeFilter", _constr, _count, offers, offer_iter, limits );

  // DEBUG
  if ( offers->length() != 0 )
    cout << "Got " <<  offers->length() << " results" << endl;
  else
    cout << "Got no results" << endl;
  // END DEBUG

  lst.reserve( offers->length() );
  
  unsigned int max = offers->length();
  for( unsigned int i = 0; i < max; i++ )
  {
    lst.push_back( koParseFilterProperties( (*offers)[i].properties ) );
    lst.back().reference = CORBA::Object::_duplicate( (*offers)[i].reference );
  }
  
  return lst;
}

KoFilterEntry::KoFilterEntry( const KoComponentEntry& _e )
{
  comment = _e.comment;
  name = _e.name;
  exec = _e.exec;
  activationMode = _e.activationMode;
  repoID = _e.repoID;
  miniIcon = _e.miniIcon;
  icon = _e.icon;
}

