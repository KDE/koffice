#include "kstartparams.h"

KStartParams::KStartParams( int& argc, char** argv )
{
  int i;

  for( i = 1; i < argc; i++ )
  {
    m_paramList.append( QString( argv[ i ] ) );
  }
}

KStartParams::~KStartParams()
{
  // Liste leeren
  // => wird automatisch gemacht
}

uint KStartParams::countParams() const
{
  return m_paramList.count();
}

QString KStartParams::getParam( uint _index ) const
{
  QString result = "";
  if( _index < countParams() )
  {
    result = *m_paramList.at( _index );
  }
  return result;
}

void KStartParams::deleteParam( uint _index )
{
  if( _index < m_paramList.count() )
  {
    m_paramList.remove( m_paramList.at( _index ) );
  }
}

bool KStartParams::paramIsPresent( const QString& _param )
{
  QStringList::Iterator it;

  for( it = m_paramList.begin(); it != m_paramList.end(); ++it )
  {
    if( compareParam( *it, _param ) )
    {
      return true;
    }
  }
  return false;
}

bool KStartParams::paramIsPresent( const QString& _longparam, const QString& _shortparam )
{
  QStringList::Iterator it;

  for( it = m_paramList.begin(); it != m_paramList.end(); ++it )
  {
    if( compareParam( *it, _longparam ) || compareParam( *it, _shortparam ) )
    {
      return true;
    }
  }
  return false;
}

int KStartParams::getIndex( const QString& _param )
{
  uint result = 0;
  QStringList::Iterator it;

  for( it = m_paramList.begin(); it != m_paramList.end(); ++it )
  {
    if( compareParam( *it, _param ) )
    {
      return result;
    }
    result++;
  }
  return -1;
}

int KStartParams::getIndex( const QString& _longparam, const QString& _shortparam )
{
  uint result = 0;
  QStringList::Iterator it;

  for( it = m_paramList.begin(); it != m_paramList.end(); ++it )
  {
    if( compareParam( *it, _longparam ) || compareParam( *it, _shortparam ) )
    {
      return result;
    }
    result++;
  }
  return -1;
}

bool KStartParams::compareParam( const QString& _arg, const QString& _param ) const
{
  if( _arg.length() < _param.length() )
    return false;
  if( _arg.left( _param.length() ) != _param ) 
    return false;  
  return true;
};

QStringList m_paramList;
