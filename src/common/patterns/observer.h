/*
 * observer.h
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

#ifndef __FEARANN_COMMON_PATTERNS_OBSERVER_H__
#define __FEARANN_COMMON_PATTERNS_OBSERVER_H__


/** @file observer.h
 *
 * @author mafm
 *
 * @brief
 *	Observer pattern interfaces.
 */


#include <string>
#include <list>


class ObserverEvent;
class Observer;
class Observable;


/**
 * @brief Event for Observer pattern.
 *
 * @author mafm
 *
 * Interface of an Event for the Observer pattern.  The intent of this class is
 * to be subclassed and then identified by RTTI, see "Contract Programming and
 * RTTI" at http://www.ddj.com/cpp/184401608
 */
class ObserverEvent
{
public:
	/** Name of the class */
	const std::string _className;

protected:
	/** Default constructor */
	ObserverEvent(const std::string& className) : _className(className) { }

	/** Virtual destructor
	 *
	 * \note Virtual methods cause RTTI/dynamic_cast to be enabled/work for
	 * this class
	 */
	virtual ~ObserverEvent() { }
};


/**
 * @brief Observer pattern.
 *
 * @author mafm
 *
 * Interface of a Observer pattern, to get notifications when a Observable being
 * observed has any event of interest to communicate.
 */
class Observer
{
public:
	/** Name
	 *
	 * \note To be used to print logging messages or the like, not for more
	 * serious purposes
	 */
	const std::string _name;


	/** Constructor */
	Observer(const std::string& name);
	/** Virtual destructor */
	virtual ~Observer();

	/** Called when an event occurs */
	virtual void updateFromObservable(const ObserverEvent& event) = 0;

	/** Attached to an Observable */
	void attachedToObservable(Observable* observable);
	/** Detached from an Observable */
	void detachedFromObservable(Observable* observable);

protected:
	/** Set of Observables */
	std::list<Observable*> _observables;
};


/**
 * @brief Observable abstract class, for observers to attach to
 *
 * @author mafm
 *
 * Interface of Observable for the Observer pattern, to allow observers to
 * attach and detach and so receive notifications of interesting events.
 */
class Observable
{
public:
	/** Virtual destructor */
	virtual ~Observable();

	/** Attach an observer */
	void attachObserver(Observer* observer);
	/** Detach an observer
	 *
	 * @param callback Set to false when you don't want the observer to get
	 * called back, in example because the observer is calling this method
	 * from the destructor and doesn't make sense to keep him processing our
	 * data (it seems to causes some "pure virtual method called", also)
	 */
	void detachObserver(Observer* observer, bool callback = true);
	/** Detach all observers (same effect than detaching individually, but
	 * it might be more efficient) */
	void detachAllObservers();

	/** Implementation of the notification */
	virtual void notifyObservers(const ObserverEvent& event);

protected:
	/** Set of observers */
	std::list<Observer*> _observers;


	/** For especial requirements of the derived classes */
	virtual void onAttachObserver(Observer* observer) { }
	/** For especial requirements of the derived classes */
	virtual void onDetachObserver(Observer* observer) { }
};


#endif


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 8 ***
// indent-tabs-mode: t ***
// fill-column: 80 ***
// fill-column: 80 ***
// End: ***
// ex: shiftwidth=2 tabstop=8
