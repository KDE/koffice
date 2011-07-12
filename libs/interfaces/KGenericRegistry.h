/* This file is part of the KDE project
 *  Copyright (c) 2004 Cyrille Berger <cberger@cberger.net>
 *  Copyright (c) 2006 Boudewijn Rempt <boud@valdyas.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef _KO_GENERIC_REGISTRY_H_
#define _KO_GENERIC_REGISTRY_H_

#include <kdemacros.h>
#include <QtCore/QList>
#include <QtCore/QString>
#include <QtCore/QHash>

/**
 * Base class for registry objects.
 *
 * Items are mapped by QString as a unique Id.
 *
 * Exemple of use:
 * @code
 * class KoMyClassRegistry : public KGenericRegistry<MyClass*> {
 * public:
 *   static KoMyClassRegistry * instance();
 * private:
 *  static KoMyClassRegistry* s_instance;
 * };
 *
 * KoMyClassRegistry *KoMyClassRegistry::s_instance = 0;
 * KoMyClassRegistry * KoMyClassRegistry::instance()
 * {
 *    if(s_instance == 0)
 *    {
 *      s_instance = new KoMyClassRegistry;
 *    }
 *    return s_instance;
 * }
 *
 * @endcode
 */
template<typename T>
class KGenericRegistry
{
public:

    /**
     * This internal class acts as a wrapper to the QHash<QString, T>::const_iterator class. 
     * This allows us to access the contents of the registry efficiently.
     * It is not a full implementation of a const_iterator class, but provides the bare minimum number of operations
     * required to do its job.
     */
    class const_iterator {
	public:
		/**
		 * Conversion ctor which promotes a QHash const_iterator to a KGenericRegistr::const_iterator
		 * @param iter The iterator to promote
		 */
		const_iterator(typename QHash<QString, T>::const_iterator iter){
		    m_iter = iter;
		}

		/**
		 * Access the value of the current item pointed to by the iterator.
		 * @return A const reference to the current value.
		 */
		const T& value() const { return m_iter.value(); }

		/**
		 * A thin wrapper to the key() function.
		 * @return A const reference to the key of the current item.
		*/
		const QString& key() const { return m_iter.key(); }

		/**
		 * Acts as a thin wrapper to the QHash comparison.
		 * @return Returns true if @param other points to the same item as this iterator.  Otherwise, returns false.
		 */
		bool operator==(const const_iterator& other) const {
			return m_iter == other.m_iter;
		}

		/**
		 * Acts as a thin wrapper to the QHash comparison.
		 * @return Returns false if @param other points to a different item than this iterator.  Otherwise, returns false.
		 */
		bool operator!=(const const_iterator& other) const {
			return !const_iterator::operator==(other);
		}

		/**
		 * Advance the iterator one position.
		 * Acts as a wrapper to the QHash iterator increment.
		 */
		const_iterator& operator++(){
			++m_iter;
			return *this;
		}
		
		/**
		 * Rewind the iterator one position.
		 * Acts as a wrapper to the QHash iterator decrement.
		 */
		const_iterator& operator--(){
			--m_iter;
			return *this;
		}

	private:
		typename QHash<QString, T>::const_iterator m_iter;
    };

    KGenericRegistry() { }
    virtual ~KGenericRegistry() { }

public:
    /**
     * add an object to the registry
     * @param item the item to add (NOTE: T must have an QString id() const   function)
     */
    void add(T item) {
        Q_ASSERT(item);
        m_hash.insert(item->id(), item);
    }

    /**
     * add an object to the registry
     * @param id the id of the object
     * @param item the item to add
     */
    void add(const QString &id, T item) {
        Q_ASSERT(item);
        m_hash.insert(id, item);
    }

    /**
     * This function removes an item from the registry
     */
    void remove(const QString &id) {
        m_hash.remove(id);
    }

    /**
     * Retrieve the object from the registry based on the unique
     * identifier string.
     *
     * @param id the id
     */
    T get(const QString& id) const {
        return value(id);
    }

    /**
     * @return if there is an object stored in the registry identified
     * by the id.
     * @param id the unique identifier string
     */
    bool contains(const QString &id) const {
        return m_hash.contains(id);
    }

    /**
     * Retrieve the object from the registry based on the unique identifier string
     * @param id the id
     */
    const T value(const QString &id) const {
        return m_hash.value(id);
    }

    /**
     * @return a list of all keys
     */
    QList<QString> keys() const {
        return m_hash.keys();
    }

    int count() const {
        return m_hash.count();
    }

    QList<T> values() const {
        return m_hash.values();
    }

    KGenericRegistry<T>::const_iterator constBegin() const {
	    return const_iterator(m_hash.constBegin());
    }
    KGenericRegistry<T>::const_iterator constEnd() const {
	    return const_iterator(m_hash.constEnd());
    }

private:
    QHash<QString, T> m_hash;
};

#endif
