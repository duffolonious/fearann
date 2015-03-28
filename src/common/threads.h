/*
 * threads.h
 * Copyright (C) 2008 by Bryan Duff <duff0097@umn.edu>
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

#ifndef __FEARANN_COMMON_THREADS_H__
#define __FEARANN_COMMON_THREADS_H__

#include <pthread.h>
#include <memory>

class Mutex {
public:
	Mutex() {
		pthread_mutex_init(&m, 0);
	}
	void lock() {
		pthread_mutex_lock(&m);
	}

	void unlock() {
		pthread_mutex_unlock(&m);
	}

private:
	pthread_mutex_t m;
};

class MutexLocker {
public:
	MutexLocker(Mutex & pm):m(pm) {
		m.lock();
	} ~MutexLocker() {
		m.unlock();
	}
private:
	Mutex & m;
};

class Thread {
public:
	Thread(void *function) {
		rc = pthread_create(&thread, NULL, (void* (*)(void*))function, NULL);
		if(rc) {
			throw "failed to create thread";
		}
	}
  
	Thread(void* function, void *arg) {
		//rc = pthread_create(&thread, NULL, (void* (*)(void *))function, (void*)arg);
		rc = pthread_create(&thread, NULL, (void* (*)(void*))function, arg);
	}

	void Join() {
		pthread_join(thread, NULL);
	}

	void Detach() {
		pthread_detach(thread);
	}

private:
	int rc;
	pthread_t thread;
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
