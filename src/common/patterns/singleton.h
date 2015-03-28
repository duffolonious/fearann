/*
 * singleton.h
 * Copyright (C) 2008 by Manuel A. Fernandez Montecelo <mafm@users.sourceforge.net>
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __FEARANN_COMMON_PATTERNS_SINGLETON_H__
#define __FEARANN_COMMON_PATTERNS_SINGLETON_H__

/** \file singleton
 *
 * This file provides traditional Singleton pattern via template.  It allows to
 * create a single-instance by inheriting off of the template class.
 *
 * To use it:
 *
 *   class Foo : public Singleton<Foo>
 *
 * The class will need to provide either a public or a protected friend
 * constructor:
 *
 *   friend class Singleton<Foo>;
 *
 * and also needs to initialize it's own instance in an implementation file:
 *
 *   // statically initialize the instance
 *   template <> Foo* Singleton<Foo>::INSTANCE = 0;
 */

#include <cstdlib>


/** Singleton template class
 */
template <typename T> class Singleton {
public:
	/**
	 * Singleton, access to the instance
	 */
	inline static T& instance() {
		if (!INSTANCE) {
			INSTANCE = new T;
			// destroy the instance when the application terminates
			std::atexit(Singleton::destroy);
		}
		return *INSTANCE;
	}

protected:
	/**
	 * Protected constructor to avoid instantiation from non-derived classes
	 */
	Singleton() { }
	/**
	 * Destructor doesn't delete
	 */
	~Singleton() { INSTANCE = 0; }

	/**
	 * Destroy instance at exit (nice for valgrind and similar tools)
	 */
	static void destroy() {
		if (INSTANCE) {
			delete INSTANCE;
			INSTANCE = 0;
		}
	}

private:
	/** Singleton instance */
	static T* INSTANCE;
};


#endif


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 8 ***
// indent-tabs-mode: t ***
// fill-column: 80 ***
// End: ***
// ex: shiftwidth=2 tabstop=8
