#include "KSpreadTableIface.h"
#include "KSpreadCellIface.h"

#include "kspread_table.h"
#include "kspread_util.h"

#include <kapp.h>
#include <dcopclient.h>

/*********************************************
 *
 * KSpreadCellProxy
 *
 *********************************************/

class KSpreadCellProxy : public DCOPObjectProxy
{
public:
    KSpreadCellProxy( KSpreadTable* table, const QCString& prefix );
    ~KSpreadCellProxy();

    virtual bool process( const QCString& obj, const QCString& fun, const QByteArray& data,
			  QCString& replyType, QByteArray &replyData );

private:
    QCString m_prefix;
    KSpreadCellIface* m_cell;
    KSpreadTable* m_table;
};

KSpreadCellProxy::KSpreadCellProxy( KSpreadTable* table, const QCString& prefix )
    : DCOPObjectProxy( kapp->dcopClient() ), m_prefix( prefix )
{
    m_cell = new KSpreadCellIface;
    m_table = table;
}

KSpreadCellProxy::~KSpreadCellProxy()
{
    delete m_cell;
}

bool KSpreadCellProxy::process( const QCString& obj, const QCString& fun, const QByteArray& data,
					QCString& replyType, QByteArray &replyData )
{
    if ( strncmp( m_prefix.data(), obj.data(), m_prefix.length() ) != 0 )
	return FALSE;

    KSpreadPoint p( obj.data() + m_prefix.length() );
    if ( !p.isValid() )
	return FALSE;

    m_cell->setCell( m_table, p.pos );
    return m_cell->process( fun, data, replyType, replyData );
}

/************************************************
 *
 * KSpreadTableIface
 *
 ************************************************/

KSpreadTableIface::KSpreadTableIface( KSpreadTable* t )
    : DCOPObject( t )
{
    m_table = t;

    QCString str = objId();
    str += "/";
    m_proxy = new KSpreadCellProxy( t, str );
}

KSpreadTableIface::~KSpreadTableIface()
{
    delete m_proxy;
}

DCOPRef KSpreadTableIface::cell( int x, int y )
{
    QCString str = objId();
    str += "/";
    str += util_cellName( x, y );

    return DCOPRef( kapp->dcopClient()->appId(), str );
}

DCOPRef KSpreadTableIface::cell( const QString& name )
{
    QCString str = objId();
    str += "/";
    str += name.latin1();

    return DCOPRef( kapp->dcopClient()->appId(), str );
}

QRect KSpreadTableIface::selection() const
{
    return m_table->selectionRect();
}

void KSpreadTableIface::setSelection( const QRect& selection )
{
    m_table->setSelection( selection );
}

QString KSpreadTableIface::name() const
{
    return m_table->tableName();
}

bool KSpreadTableIface::processDynamic( const QCString& fun, const QByteArray& data,
					QCString& replyType, QByteArray &replyData )
{
    qDebug("Calling '%s'", fun.data());
    // Does the name follow the pattern "foobar()" ?
    uint len = fun.length();
    if ( len < 3 )
	return FALSE;

    if ( fun[ len - 1 ] != ')' || fun[ len - 2 ] != '(' )
	return FALSE;

    // Is the function name a valid cell like "B5" ?
    KSpreadPoint p( fun.left( len - 2 ).data() );
    if ( !p.isValid() )
	return FALSE;

    QCString str = objId() + "/" + fun.left( len - 2 );

    replyType = "DCOPRef";
    QDataStream out( replyData, IO_WriteOnly );
    out << DCOPRef( kapp->dcopClient()->appId(), str );
    return TRUE;
}
