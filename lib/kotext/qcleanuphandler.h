#ifndef QCLEANUPHANDLER_H
#define QCLEANUPHANDLER_H

#ifndef QT_H
#include <qptrlist.h>
#include <qguardedptr.h>
#endif // QT_H

template<class Type>
class Q_EXPORT QGuardedCleanupHandler
{
public:
    ~QGuardedCleanupHandler() { clear(); }

    void add( Type* object )
    {
	cleanupObjects.insert( 0, new QGuardedPtr<Type>(object) );
    }

    void remove( Type *object )
    {
	QPtrListIterator<QGuardedPtr<Type> > it( cleanupObjects );
	while ( it.current() ) {
	    QGuardedPtr<Type>* guard = it.current();
	    ++it;
	    if ( (Type *)guard == object ) {
		cleanupObjects.removeRef( guard );
		delete guard;
		break;
	    }
	}
    }

    bool isEmpty() const
    {
	QPtrListIterator<QGuardedPtr<Type> > it( cleanupObjects );
	while ( it.current() ) {
	    QGuardedPtr<Type>* guard = it.current();
	    ++it;
	    if ( (Type*)*guard )
		return FALSE;
	}
	return TRUE;
    }

    void clear() {
	QPtrListIterator<QGuardedPtr<Type> > it( cleanupObjects );
	it.toLast();
	while ( it.current() ) {
	    QGuardedPtr<Type>* guard = it.current();
	    --it;
	    cleanupObjects.removeRef( guard );
	    delete (Type*)*guard;
	    delete guard;
	}
    }

private:
    QPtrList<QGuardedPtr<Type> > cleanupObjects;
};

template<class Type>
class Q_EXPORT QCleanupHandler
{
public:
    QCleanupHandler() : cleanupObjects( 0 )
    {}
    ~QCleanupHandler() { clear(); }

    void add( Type* object )
    {
	if ( !cleanupObjects ) {
	    cleanupObjects = new QPtrList<Type>;
	}
	cleanupObjects->insert( 0, object );
    }

    void remove( Type *object )
    {
	if ( !cleanupObjects )
	    return;
	if ( object )
	    cleanupObjects->removeRef( object );
    }

    bool isEmpty() const
    {
	return cleanupObjects ? cleanupObjects->isEmpty() : TRUE;
    }

    void clear()
    {
	if ( !cleanupObjects )
	    return;

	QPtrListIterator<Type> it( *cleanupObjects );
	it.toLast();
	while ( it.current() ) {
	    Type* object = it.current();
	    --it;
	    cleanupObjects->removeRef( object );
	    delete object;
	}

	delete cleanupObjects;
	cleanupObjects = 0;
    }

private:
    QPtrList<Type> *cleanupObjects;
};

#endif //QCLEANUPHANDLER_H
