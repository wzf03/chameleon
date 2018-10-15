/**
 *
 * @file chameleonwinthread.c
 *
 * @copyright 2009-2014 The University of Tennessee and The University of
 *                      Tennessee Research Foundation. All rights reserved.
 * @copyright 2012-2014 Bordeaux INP, CNRS (LaBRI UMR 5800), Inria,
 *                      Univ. Bordeaux. All rights reserved.
 *
 ***
 *
 * @brief Chameleon windows threads interface
 *
 * @version 1.0.0
 * @author Piotr Luszczek
 * @author Cedric Castagnede
 * @date 2012-09-15
 *
 */
#include "control/chameleonwinthread.h"

#include <limits.h>

/** this is needed to get a declaration for _beginthreadex() */
#include <process.h>

#include <stdio.h>
#include <chameleon.h>

CRITICAL_SECTION chameleonwinthread_static_initializer_check_lock;
static int chameleonwinthread_initialized = 0;

CHAMELEON_DLLPORT unsigned int CHAMELEON_CDECL pthread_self_id(void) {
  return GetCurrentThreadId();
}

CHAMELEON_DLLPORT pthread_t CHAMELEON_CDECL pthread_self(void) {
  pthread_t pt;

  pt.hThread = GetCurrentThread();
  pt.uThId = GetCurrentThreadId();
  return pt;
}

CHAMELEON_DLLPORT int CHAMELEON_CDECL pthread_equal(pthread_t thread1, pthread_t thread2) {
  if (thread1.uThId == thread2.uThId) // && thread1.hThread == thread2.hThread)
    return 1;
  return 0;
}

CHAMELEON_DLLPORT int CHAMELEON_CDECL pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t * attr) {
  *mutex =
  CreateMutex( NULL,  /** no security atributes */
               FALSE, /** not owned (initialy) by the creating thread */
               NULL   /** no name provided: cannot be shared between processes */
  );

  return 0;
}

static int pthread_mutex_check_for_static_initialization( pthread_mutex_t *mutex ) {
    int retval = 0;
    /* This should be called once to initialize some structures */
    if ( chameleonwinthread_initialized == 0 ) {
        InitializeCriticalSection( &chameleonwinthread_static_initializer_check_lock );
        chameleonwinthread_initialized = 1;
    }
    EnterCriticalSection( &chameleonwinthread_static_initializer_check_lock );
    if ( *mutex == PTHREAD_MUTEX_INITIALIZER )
        retval = pthread_mutex_init( mutex, NULL );
    LeaveCriticalSection( &chameleonwinthread_static_initializer_check_lock );
    return retval;
}

CHAMELEON_DLLPORT int CHAMELEON_CDECL pthread_mutex_lock(pthread_mutex_t *mutex) {
  DWORD rv;

  if ( *mutex == PTHREAD_MUTEX_INITIALIZER ) pthread_mutex_check_for_static_initialization( mutex );
  rv = WaitForSingleObject( *mutex, INFINITE );
  switch (rv) {
    case WAIT_OBJECT_0: /** the wait was succesful */
      return 0;
    case WAIT_FAILED: /** the wait failed */
      return -1;
    case WAIT_ABANDONED: /** thread killed during the wait */
      return -1;
    case WAIT_TIMEOUT: /** impossible because of INFINITE */
      return -1;
    default:
      return -1;
  }
}

CHAMELEON_DLLPORT int CHAMELEON_CDECL pthread_mutex_trylock(pthread_mutex_t *mutex) {
  DWORD rv;

  if ( *mutex == PTHREAD_MUTEX_INITIALIZER ) pthread_mutex_check_for_static_initialization( mutex );
  rv = WaitForSingleObject( *mutex, 0 );
  switch (rv) {
    case WAIT_OBJECT_0: /** the wait was succesful */
      return 0;
    case WAIT_FAILED: /** the wait failed */
      return -1;
    case WAIT_ABANDONED: /** thread killed during the wait */
      return -1;
    case WAIT_TIMEOUT: /** impossible because of INFINITE */
      return -1;
    default:
      return -1;
  }
}

CHAMELEON_DLLPORT int CHAMELEON_CDECL pthread_mutex_unlock(pthread_mutex_t *mutex) {
  if (! ReleaseMutex( *mutex ))
    return -1;

  return 0;
}

CHAMELEON_DLLPORT int CHAMELEON_CDECL pthread_mutex_destroy(pthread_mutex_t *mutex) {
  CloseHandle( *mutex );
  return 0;
}

CHAMELEON_DLLPORT int CHAMELEON_CDECL pthread_attr_init(pthread_attr_t *attr) {
  *attr = 1;
  return 0;
}

CHAMELEON_DLLPORT int CHAMELEON_CDECL pthread_attr_destroy(pthread_attr_t *attr) {
  *attr = 0;
  return 0;
}

CHAMELEON_DLLPORT int CHAMELEON_CDECL pthread_attr_setscope(pthread_attr_t *attr, int scope) {
  if (*attr != 1)
    return -1;

  if (scope != PTHREAD_SCOPE_SYSTEM)
    return -1;

  return 0;
}

void *(*CHAMELEON_realThStart)(void *);

/*
  This function is only called to have a proxy that is compatible with WINAPI.
 */
unsigned WINAPI CHAMELEON_winThStart(void *arg) {
  CHAMELEON_realThStart( arg );
  return 0;
}

CHAMELEON_DLLPORT int CHAMELEON_CDECL pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start) (void *), void *arg) {

  /* this assumes that the threads call the same function, always; it also assumes there
     is no race condition while assigning a pointer and using it from within threads
     (this assumption is fulfilled by creating the new thread in suspended state) */
  CHAMELEON_realThStart = start;

  thread->hThread = (HANDLE)_beginthreadex(
    NULL, /* default security */
    0, /* stack size: use the size of calling thread */
    CHAMELEON_winThStart,
    arg,
    CREATE_SUSPENDED,
    /*0,*/   /* the thread will run immedietally (rather than get suspended) */
    &thread->uThId );

  /* We need to make sure that _beginthreadex() returns to the parent thread first
     so we can safely fill up the members of the pthread_t structure without possible
     race conditions. If the new thread is created in supsended state we eliminate
     the race condition but now we have to resume the new thread. */
  ResumeThread( thread->hThread );

  return 0;
}

CHAMELEON_DLLPORT int CHAMELEON_CDECL pthread_join(pthread_t thread, void **value_ptr) {
  WaitForSingleObject( thread.hThread, INFINITE );
  CloseHandle( thread.hThread );
  return 0;
}

CHAMELEON_DLLPORT int CHAMELEON_CDECL pthread_cond_init(pthread_cond_t *cond, const pthread_condattr_t *attr) {
  InitializeCriticalSection( &cond->cs );
  cond->hSem = CreateSemaphore( NULL, /* no security attributes */
    0, /* initial count */
    LONG_MAX, /* maximum count*/
    NULL ); /* unnamed semaphore */
  cond->hEvt = CreateEvent( NULL, /* no security attributes */
    FALSE, /* reset to not-singaled automatically */
    FALSE, /* set initial status to not-signaled */
    NULL ); /* unnamed event */
  cond->waitCount = 0;
  return 0;
}

CHAMELEON_DLLPORT int CHAMELEON_CDECL pthread_cond_destroy(pthread_cond_t *cond) {
  DeleteCriticalSection( &cond->cs );
  CloseHandle( cond->hSem );
  CloseHandle( cond->hEvt );
  return 0;
}

CHAMELEON_DLLPORT int CHAMELEON_CDECL pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex) {
  int last;

  if ( *mutex == PTHREAD_MUTEX_INITIALIZER ) pthread_mutex_check_for_static_initialization( mutex );

  /* Avoid race condition on waiting thread counter. */
  EnterCriticalSection(&cond->cs);
  cond->waitCount++;
  LeaveCriticalSection(&cond->cs);

  /* Releases _atomically_ the mutex and wait on the semaphore until
     pthread_cond_signal() or pthread_cond_broadcast() are called (by another thread). */
  SignalObjectAndWait(*mutex, cond->hSem, INFINITE, FALSE);

  /* Avoid race condition on waiting thread counter. */
  EnterCriticalSection(&cond->cs);
  cond->waitCount--; /* this thread doesn't wait any more */

  /* if this is the last thread to have waited */
  last = cond->waitCount == 0;

  LeaveCriticalSection(&cond->cs);

  /* If this thread is the last waiter thread during this particular broadcast
     then let all the other threads proceed. */
  if (last)
    /* This call ensures that two things happen atomically: signaling the hEvt event and
       waiting until "mutex" can be acquired. */
    SignalObjectAndWait(cond->hEvt, *mutex, INFINITE, FALSE);
  else
    WaitForSingleObject(*mutex, INFINITE); /* Upon return, this thread has to own "mutex". */

  return 0;
}

CHAMELEON_DLLPORT int CHAMELEON_CDECL pthread_cond_broadcast(pthread_cond_t *cond) {
  int more_waiters = 0;

  /* This is needed to ensure exclusive access to "waitCount" */
  EnterCriticalSection (&cond->cs);

  if (cond->waitCount > 0) {
    /* always are broadcasting - no need for pthread_cond_singal() case */
    more_waiters = 1;
  }

  if (more_waiters) {
    /* this will wake up all the waiters atomically at once. */
    ReleaseSemaphore(cond->hSem, cond->waitCount, 0);

    LeaveCriticalSection(&cond->cs);

    /* Wait for all the awakened threads to acquire the counting semaphore. */
    WaitForSingleObject(cond->hEvt, INFINITE);
  } else
    LeaveCriticalSection(&cond->cs);

  return 0;
}

int pthread_conclevel;

CHAMELEON_DLLPORT int CHAMELEON_CDECL pthread_setconcurrency (int level) {
  pthread_conclevel = level;
  return 0;
}