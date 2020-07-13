/*
SoLoud audio engine
Copyright (c) 2013-2014 Jari Komppa

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

   1. The origin of this software must not be misrepresented; you must not
   claim that you wrote the original software. If you use this software
   in a product, an acknowledgment in the product documentation would be
   appreciated but is not required.

   2. Altered source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.

   3. This notice may not be removed or altered from any source
   distribution.
*/

#ifndef SOLOUD_THREAD_H
#define SOLOUD_THREAD_H

#include "soloud.h"

namespace SoLoud
{
	namespace Thread
	{
		typedef void (*threadFunction)(void *aParam);

        struct ThreadHandleData;
        typedef ThreadHandleData* ThreadHandle;

		void * createMutex();
		void destroyMutex(void *aHandle);
		void lockMutex(void *aHandle);
		void unlockMutex(void *aHandle);

		ThreadHandle createThread(threadFunction aThreadFunction, void *aParameter);

		void sleep(int aMSec);
        void wait(ThreadHandle aThreadHandle);
        void release(ThreadHandle aThreadHandle);
		int getTimeMillis();

#define MAX_THREADPOOL_TASKS 1024

		class PoolTask
		{
		public:
			virtual void work() = 0;
		};

		class Pool
		{
		public:
			// Initialize and run thread pool. For thread count 0, work is done at addWork call.
			void init(int aThreadCount);
			// Ctor, sets known state
			Pool();
			// Dtor. Waits for the threads to finish. Work may be unfinished.
			~Pool();
			// Add work to work list. Object is not automatically deleted when work is done.
			void addWork(PoolTask *aTask);
			// Called from worker thread to get a new task. Returns null if no work available.
			PoolTask *getWork();
		public:
			int mThreadCount; // number of threads
			ThreadHandle *mThread; // array of thread handles
			void *mWorkMutex; // mutex to protect task array/maxtask
			PoolTask *mTaskArray[MAX_THREADPOOL_TASKS]; // pointers to tasks
			int mMaxTask; // how many tasks are pending
			int mRobin; // cyclic counter, used to pick jobs for threads
			volatile int mRunning; // running flag, used to flag threads to stop
		};
	}
}

#endif