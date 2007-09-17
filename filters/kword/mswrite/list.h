
/* This file is part of the LibMSWrite project
   Copyright (c) 2001-2003, 2007 Clarence Dang <clarencedang@users.sf.net>
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:

   1. Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
   2. Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.

   THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
   IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
   OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
   IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
   INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
   NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
   THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

   LibMSWrite Project Website:
   http://sourceforge.net/projects/libmswrite/
*/

/* 
 * list.h - linked list implementation
 * This file ensures that we do not have to depend on STL but don't waste
 * time and memory trying to reallocate arrays.
 *
 * An implementation without C++-style iterators would have taken half
 * as many lines and would probably be twice as efficient...
 *
 */

#ifndef __MSWRITE_LIST_H__
#define __MSWRITE_LIST_H__

#ifndef NULL
	#define NULL 0
#endif

#include <assert.h>
// TODO: implement ConstIterator so that all the List<dtype>::end (void) const's can have the const etc.
namespace MSWrite
{
	template <class dtype>	class List;
	template <class dtype>	class ListIterator;
	template <class dtype>
	class ListElement
	{
	private:
		dtype m_data;
		ListElement *m_prev, *m_next;

	public:
		ListElement ()
		{
			m_prev = m_next = NULL;
		}

		ListElement (dtype &data)
		{
			m_prev = m_next = NULL;
			m_data = data;
		}

		~ListElement ()
		{
		}

		dtype &getData (void) const
		{
			return m_data;
		}
		dtype &operator* (void) const
		{
			return m_data;
		}
		bool operator== (const dtype &rhs) const
		{
			return this->data == rhs.data;
		}

		void setData (const dtype &data)
		{
			m_data = data;
		}
		dtype &operator= (const dtype &rhs)
		{
			if (this == &rhs)
				return *this;

			this->data = rhs.data;
			return this->data;
		}

		friend class List <dtype>;
		friend class ListIterator <dtype>;
	};

	template <class dtype>	class List;
	template <class dtype>
	class ListIterator
	{
	private:
		bool m_forward;
		ListElement <dtype> *m_upto;

		void setPtr (ListElement <dtype> *ptr)
		{
			m_upto = ptr;
		}

	public:
		ListIterator (const bool forward = true)
		{
			m_forward = forward;
		}

		ListIterator <dtype> &prev (void)
		{
			if (m_forward)
				m_upto = m_upto->m_prev;
			else
				m_upto = m_upto->m_next;

			return *this;
		}

		~ListIterator ()
		{
		}
		
		ListIterator <dtype> &operator-- (void)
		{
			return prev ();
		}

		ListIterator <dtype> &operator-- (int)
		{
			return prev ();
		}

		ListIterator <dtype> &next (void)
		{
			if (m_forward)
				m_upto = m_upto->m_next;
			else
				m_upto = m_upto->m_prev;

			return *this;
		}

		ListIterator <dtype> &operator++ (void)
		{
			return next ();
		}

		ListIterator <dtype> &operator++ (int)
		{
			return next ();
		}

		bool operator== (const ListIterator <dtype> &rhs) const
		{
			return this->m_upto == rhs.m_upto;
		}

		bool operator!= (const ListIterator <dtype> &rhs) const
		{
			return this->m_upto != rhs.m_upto;
		}

		dtype &operator* (void) const
		{
			//return *(*m_upto);	// why doesn't this work?
			return m_upto->m_data;
		}

		friend class List <dtype>;
	};

	template <class dtype>
	class List
	{
	private:
		ListElement <dtype> *m_head, *m_tail;
		int m_num;
		bool m_good;

	public:
		List ()
		{
			m_head = m_tail = (ListElement <dtype> *) NULL;
			m_num = 0;
			m_good = true;
		}

		virtual ~List ()
		{
			killself ();
		}

		void killself (void)
		{
			ListElement <dtype> *e = m_head;
			ListElement <dtype> *nexte;
			while (e)
			{
				nexte = e->m_next;
				delete e;
				e = nexte;
			}
			m_head = m_tail = NULL;
			m_num = 0;
			m_good = true;
		}

		bool empty (void) const
		{
			return m_head;
		}

		bool good (void) const
		{
			return m_good;
		}

		bool bad (void) const
		{
			return !good ();
		}

		bool addToFront (dtype &data)
		{
			ListElement <dtype> *e = new ListElement <dtype> (data);
			if (!e)
			{
				m_good = false;
				return false;
			}

			if (m_head)
			{
				e->next = m_head;
				m_head->prev = e;
				m_head = e;
			}
			// empty
			else
			{
				m_head = m_tail = e;
			}

			m_num++;
			return true;
		}

		bool addToBack (void)
		{
			ListElement <dtype> *e = new ListElement <dtype> ();
			if (!e)
			{
				m_good = false;
				return false;
			}

			if (m_tail)
			{
				e->m_prev = m_tail;
				m_tail->m_next = e;
				m_tail = e;
			}
			// empty
			else
			{
				m_head = m_tail = e;
			}

			m_num++;
			return true;
		}

		// for efficiency, you can call the above void argument list version
		// and initialise the data within the list yourself (avoiding a copy)
		bool addToBack (const dtype &data)
		{
			if (!addToBack ())
				return false;
			m_tail->setData (data);
			return true;
		}

		int getNumElements (void) const
		{
			return m_num;
		}

		List <dtype> &operator= (const List <dtype> &rhs)
		{
			if (this == &rhs)
				return *this;

			killself ();

			m_num = rhs.m_num;
			m_good = rhs.m_good;

			ListElement <dtype> *e = rhs.m_head;
			while (e)
			{
				if (!addToBack (e->m_data)) break;
				e = e->m_next;
			}

			return *this;
		}

		typedef ListIterator <dtype> Iterator;
		friend class ListIterator <dtype>;

		ListIterator <dtype> begin (const bool forward = true) const
		{
			ListIterator <dtype> ret (forward);
			if (forward)
				ret.setPtr (m_head);
			else
				ret.setPtr (m_tail);
			return ret;	// not a reference
		}

		ListIterator <dtype> end (void) const
		{
			ListIterator <dtype> ret;
			ret.setPtr (NULL);	// true regardless of which direction
			return ret;	// not a reference
		}

		ListIterator <dtype> erase (ListIterator <dtype> &it)
		{
			// regardless of iterator direction
			ListElement <dtype> *eat = it.m_upto;
			ListElement <dtype> *prevElement = it.m_upto->m_prev;
			ListElement <dtype> *nextElement = it.m_upto->m_next;
			it++;
			delete (eat);

			if (prevElement)
				prevElement->m_next = nextElement;
			else
				m_head = nextElement;

			if (nextElement)
				nextElement->m_prev = prevElement;
			else
				m_tail = prevElement;

			m_num--;
			return it;
		}
		
		ListIterator <dtype> search (const dtype &value) const
		{
			ListIterator <dtype> it;
			for (it = begin (); it != end (); it++)
			{
				if ((*it) == value)
					break;
			}

			// not reference
			return it;
		}
	};
}	// namespace MSWrite	{

#endif	// #ifndef __MSWRITE_LIST_H__

// end of list.h
