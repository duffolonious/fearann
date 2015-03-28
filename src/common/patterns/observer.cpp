/*
 * observer.cpp
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

/** @file observer.cpp
 *
 * @author mafm
 *
 * @brief
 *	Observer pattern implementation.
 */

#include "config.h"

#include "observer.h"


/*******************************************************************************
 * Observer
 ******************************************************************************/
Observer::Observer(const std::string& name) :
	_name(name)
{
}

Observer::~Observer()
{
	LogDBG("Observer::~Observer()");

	// Observable::detachObserver() calls back to ::detachedFromObservable()
	// and a race condition is raised accessing the list, so we have to ask
	// for detaching with no call back
	for (std::list<Observable*>::iterator it = _observables.begin(); it != _observables.end(); ++it) {
		(*it)->detachObserver(this, false); // don't call back
		it = _observables.erase(it);
	}
}

void Observer::attachedToObservable(Observable* observable)
{
	_observables.push_back(observable);
}

void Observer::detachedFromObservable(Observable* observable)
{
	_observables.remove(observable);
}


/*******************************************************************************
 * Observable
 ******************************************************************************/
Observable::~Observable()
{
	LogDBG("Observable::~Observable()");
	detachAllObservers();
}

void Observable::attachObserver(Observer* observer)
{
	observer->attachedToObservable(this);
	_observers.push_back(observer);
	onAttachObserver(observer);
}

void Observable::detachObserver(Observer* observer, bool callback)
{
	if (callback) {
		onDetachObserver(observer);
		observer->detachedFromObservable(this);
	}
	_observers.remove(observer);
}

void Observable::detachAllObservers()
{
	for (std::list<Observer*>::iterator it = _observers.begin(); it != _observers.end(); ++it) {
		onDetachObserver(*it);
		(*it)->detachedFromObservable(this);
		it = _observers.erase(it);
	}
}

void Observable::notifyObservers(const ObserverEvent& event)
{
	for (std::list<Observer*>::iterator it = _observers.begin(); it != _observers.end(); ++it) {
		(*it)->updateFromObservable(event);
	}
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 8 ***
// indent-tabs-mode: t ***
// fill-column: 80 ***
// fill-column: 80 ***
// End: ***
// ex: shiftwidth=2 tabstop=8
