#ifndef KSPREAD_TABLE_IFACE_H
#define KSPREAD_TABLE_IFACE_H

#include <dcopobject.h>
#include <dcopref.h>

#include <qstring.h>
#include <qrect.h>

class KSpreadTable;
class KSpreadCellProxy;

class KSpreadTableIface : virtual public DCOPObject
{
    K_DCOP
public:
    KSpreadTableIface( KSpreadTable* );
    ~KSpreadTableIface();

    bool processDynamic( const QCString& fun, const QByteArray& data,
			 QCString& replyType, QByteArray &replyData );

k_dcop:
    virtual DCOPRef cell( int x, int y );
    virtual DCOPRef cell( const QString& name );
    virtual QRect selection() const;
    virtual void setSelection( const QRect& selection );
    virtual QString name() const;

private:
    KSpreadTable* m_table;
    KSpreadCellProxy* m_proxy;
};

#endif
