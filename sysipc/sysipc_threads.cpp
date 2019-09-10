/*
  Copyright (c) 2014-2015 by NASa Qian <nasacj@nasacj.net>
  This file is part of the woodycxx library.

  This software is distributed under BSD 3-Clause License.
  The full license agreement can be found in the LICENSE file.

  This software is distributed without any warranty.
*/

#ifdef WIN32
#define _CRT_SECURE_NO_DEPRECATE 1
#else
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <limits.h>
#endif
#include <string.h>
#include "sysipc_threads.h"
#include "sysipc_event_log.h"

namespace woodycxx {
namespace sysipc {

#ifdef WIN32
int fork(){ return -1; }
int pipe( int des[ 2 ] ){ return -1; }
int getpid(){ return -1; }
inline int get_errno(){ return -1; }
int close( int ){ return -1; }
int waitpid( int, int*, int ){ return -1;}
int dup2( int, int ){ return -1; }
#define WIFEXITED( _child_status )  0
#define WIFSIGNALED( _child_status )  0
#define WNOHANG 0
#else
inline int get_errno() { return errno; }
#endif

//-------------------------------------------------------------------------------------------------------------
process::process(handler h, void *context) :
    parent_pid(getpid()), child_pid(-1), error(0), _handler(h), _context(context),
    _child_state(init), _child_error(false) {
}

//-------------------------------------------------------------------------------------------------------------
bool process::start() {
  DEBUG_INFO("TH", "process::start() _child_state=" << dec << _child_state);
  // only fork if new process brand new, or previous fork is complete
  if ((_child_state == init) || (_child_state == complete)) {
    _child_state = init;

    // assume the worst
    _child_error = true;

    DEBUG_INFO("TH", "ready to set up pipes");
    // set up pipe
    // 0- read, 1 - write
    // to pipe used by parent to send data from the child
    // from pipe is used to read data from the child
    int rtn = pipe(to_pipe);
    if (rtn != -1)
      rtn = pipe(from_pipe);

    error = get_errno();
    if (rtn != -1) {
      DEBUG_INFO("TH", "Setting up pipes");
      error = 0;
      child_pid = fork();
      error = get_errno();
      if (child_pid == -1) {
        DEBUG_INFO("TH", "fork failed");
        close(to_pipe[0]);
        close(to_pipe[1]);
        close(from_pipe[0]);
        close(from_pipe[1]);
      } else {
        if (child_pid == 0) {
          // child process, close read side of the from pipe, write side of the to pipe
          close(to_pipe[1]);
          close(from_pipe[0]);
          child_pid = getpid();

          // connect stdout of process to write side of the from pipe
          dup2(from_pipe[1], 1);

          // connect stdin of process to read side of the to pipe
          dup2(to_pipe[0], 0);

          int exit_code = -1;
          // child will call handler routine
          if (_handler)
            exit_code = (*_handler)(_context);

          // since that routine may or may not come back here, need to clean up here just in case.
          close(to_pipe[0]);
          close(from_pipe[1]);
          exit(exit_code);
        } else {
          parent_pid = getpid();
          DEBUG_INFO("TH", "fork successful, in parent process");
          // parent process, close read side of the to pipe, write side of the from pipe
          close(to_pipe[0]);
          close(from_pipe[1]);

          _child_state = running;
          _child_error = false;
          // parent just returns to main routine
        }
      }
    }
  }
  return is_running();
}

//-------------------------------------------------------------------------------------------------------------
void process::get_status(bool wait) {
  // must be in parent
  if (is_running() && (getpid() == parent_pid)) {
    int status = -1;
    int pid;
    if (wait) {
      pid = waitpid(child_pid, &status, 0);
      _child_state = complete;
      DEBUG_INFO("TH", "child process complete");
      child_pid = -1;
    } else {
      pid = waitpid(child_pid, &status, WNOHANG);
      if (pid == child_pid) {
        DEBUG_INFO("TH", "child process complete");
        // we got status on something, so the process is done
        _child_state = complete;
        // close pipes
        child_pid = -1;
        close(to_pipe[1]);
        close(from_pipe[0]);
      }
    }

    if (is_complete()) {
      if (WIFEXITED(status)) {
        DEBUG_INFO("TH",
                   "child process exited exit code=: " << WEXITSTATUS(status) << ", error= "
                                                       << strerror(WEXITSTATUS(status)));
        _child_error = false;
      } else if (WIFSIGNALED(status)) {
        DEBUG_INFO("TH", "child process killed by signal, signal=" << WTERMSIG(status));
        _child_error = true;
      }
    }
  }
}

//-------------------------------------------------------------------------------------------------------------
//    void process::make_nice(int priority)
//    {
//#ifndef WIN32
//        if ( is_running() )
//            setpriority( PRIO_PROCESS, child_pid, priority); 
//#endif
//    }

thread::~thread() {
}

#ifdef WIN32
//-------------------------------------------------------------------------------------------------------------
class windows_thread : public thread
{
    static const uint32 flags = THREAD_PRIORITY_IDLE | CREATE_SUSPENDED;

    HANDLE  handle;

    static DWORD WINAPI thread_proc( void* me );

public:
    windows_thread( const char* nm, handler hdlr, void* contxt, uint stack_size );
    ~windows_thread();

    virtual void start();
    virtual void exit(){}
};

//-------------------------------------------------------------------------------------------------------------
windows_thread::windows_thread( const char* nm, handler hdlr, void* contxt, uint stack_size ) :
thread( nm, hdlr, contxt, stack_size ), handle( 0 )
{
    DEBUG_INFO("TH", "creating thread " << nm );
    handle = CreateThread( 0, stack_size, thread_proc, this, flags, &id );
    if ( handle == 0 )
        _error_code = GetLastError();
}

//-------------------------------------------------------------------------------------------------------------
windows_thread::~windows_thread()
{
    TerminateThread( handle, 2 );
    WaitForSingleObject( handle, 100 );
}

//-------------------------------------------------------------------------------------------------------------
void windows_thread::start()
{
    DEBUG_INFO("TH", "Starting thread, thread= " << name );
    ResumeThread( handle );
}

//-------------------------------------------------------------------------------------------------------------
DWORD windows_thread::thread_proc( void* me )
{
    windows_thread* thrd = reinterpret_cast<windows_thread*>( me );
    thrd->_running = true;
    if ( thrd->_handler )
        (*thrd->_handler)( thrd->context );
    thrd->_running = false;
    return 0;
}


//-------------------------------------------------------------------------------------------------------------
thread* thread::create( const char* nm, handler hdlr, void* contxt, uint stack_size )
{
    return new windows_thread( nm, hdlr, contxt, stack_size );
}

//-------------------------------------------------------------------------------------------------------------
void thread::sleep( uint millisecs )
{
    Sleep( millisecs );
}

//-------------------------------------------------------------------------------------------------------------
process::~process()
{
}

//-------------------------------------------------------------------------------------------------------------
int process::get_pid()
{
    int pid = 0;
    pid = ( 0x7FFF & rand() ) << 16;
    pid |= ( time( NULL ) & 0xFF ) << 8;
    return pid;
}

//-------------------------------------------------------------------------------------------------------------
void process::kill_child()
{
}


//-------------------------------------------------------------------------------------------------------------
critical_section::critical_section()
{
    InitializeCriticalSection( &cs );
}


//-------------------------------------------------------------------------------------------------------------
void critical_section::enter()
{
    EnterCriticalSection( &cs );
}


//-------------------------------------------------------------------------------------------------------------
void critical_section::leave()
{
    LeaveCriticalSection( &cs );
}

//-------------------------------------------------------------------------------------------------------------
critical_section::~critical_section()
{
    DeleteCriticalSection( &cs );
}

#else
//-------------------------------------------------------------------------------------------------------------
class linux_pthread : public thread {
  static void *thread_proc(void *context);
  pthread_mutex_t ready_mutex;
  pthread_mutex_t start_mutex;
  pthread_cond_t start_cond;
  bool ready;

 public:
  linux_pthread(const char *nm, handler hdlr, void *contxt, uint stack_size);
  virtual ~linux_pthread();
  virtual bool wait_ready() {
    bool rtn = is_ok();
    if (rtn) {
      if (!ready) {
        _error_code = pthread_mutex_lock(&ready_mutex);
        ready = true;
        rtn = ready;
        _error_code = pthread_mutex_unlock(&ready_mutex);
      }
    }
    return rtn;
  }

  virtual void start();
  virtual void exit();
};

//-------------------------------------------------------------------------------------------------------------
linux_pthread::~linux_pthread() {
  // eventually need something here to do cleanup??????
  DEBUG_INFO("TH", "destroying thread " << name);
  // Request to cancel the thread, but make sure we do not destroy this object
  // until the thread is returned, else we will crash the entire app
  pthread_cancel(id);
  pthread_join(id, NULL);
  pthread_cond_destroy(&start_cond);
  pthread_mutex_destroy(&start_mutex);
  pthread_mutex_destroy(&ready_mutex);
  DEBUG_INFO("TH", "destroyed thread " << name);

}

//-------------------------------------------------------------------------------------------------------------
void thread::sleep(uint millisecs) {
  usleep(millisecs * 1000);
}

//-------------------------------------------------------------------------------------------------------------
linux_pthread::linux_pthread(const char *nm, handler hdlr, void *contxt, uint stack_size) :
    thread(nm, hdlr, contxt, stack_size), ready(false) {
  if (_error_code == 0) {
    DEBUG_INFO("TH", "initing ready mutex on thread " << nm);
    // Initialize the start mutex with default attributes (NULL)
    _error_code = pthread_mutex_init(&ready_mutex, NULL);
    if (_error_code == 0) {
      DEBUG_INFO("TH", "acquiring lock on ready mutex " << nm);
      _error_code = pthread_mutex_lock(&ready_mutex);
    }
    if (_error_code == 0) {
      DEBUG_INFO("TH", "initing mutex on thread " << nm);
      // Initialize the start mutex with default attributes (NULL)
      _error_code = pthread_mutex_init(&start_mutex, NULL);

      if (_error_code == 0) {
        DEBUG_INFO("TH", "initing condition variable thread " << nm);
        // Initialize start condition variable
        _error_code = pthread_cond_init(&start_cond, NULL);

        if (_error_code == 0) {
          DEBUG_INFO("TH", "acquiring lock on thread " << nm);
          // need to lock the mutex to ensure thread starts and gets to its condition wait on run_flag
          _error_code = pthread_mutex_lock(&start_mutex);

          if (_error_code == 0) {
            // check stack size
            if (_stack_size < PTHREAD_STACK_MIN)
              _stack_size = PTHREAD_STACK_MIN;
            else {
              // round stack size up to nearest multiple of PTHREAD_STACK_MIN
              if ((_stack_size % PTHREAD_STACK_MIN) > 0)
                _stack_size = ((_stack_size / PTHREAD_STACK_MIN) + 1) * PTHREAD_STACK_MIN;
              else
                _stack_size = (_stack_size / PTHREAD_STACK_MIN) * PTHREAD_STACK_MIN;

            }
            // change stacksize
            pthread_attr_t attr;
            _error_code = pthread_attr_init(&attr);
            if (_error_code == 0) {
              size_t default_stacksize = 0;
              pthread_attr_getstacksize(&attr, &default_stacksize);
              DEBUG_INFO("TH", "default stacksize, size: " << default_stacksize);
              DEBUG_INFO("TH", "setting stacksize, size: " << _stack_size);
              _error_code = pthread_attr_setstacksize(&attr, _stack_size);
              if (_error_code == 0) {
                DEBUG_INFO("TH", "creating thread " << nm);
                // using default attributes ( second parameter = NULL )
                _error_code = pthread_create(&id, &attr, thread_proc, this);
                pthread_attr_destroy(&attr);
              }
            }
          }
        }
      }
    }
  }
}

//-------------------------------------------------------------------------------------------------------------
void *linux_pthread::thread_proc(void *me) {
  linux_pthread *thrd = reinterpret_cast<linux_pthread *>( me );

  // because there is a mutex lock in constructor, and another lock in run, we are guaranteed to ge there
  // with run_flag - false and so we are gauranteed to hit the condition wait.
  // wait for run to wake us up. be careful of spurious wakeups, so check predicate run_flag
  DEBUG_INFO("TH", "thread waiting to run, thread= " << thrd->name);
  while (!thrd->run_flag) {
    pthread_mutex_unlock(&thrd->ready_mutex);
    // unlocks the mutex and waits for run
    pthread_cond_wait(&thrd->start_cond, &thrd->start_mutex);
  }
  DEBUG_INFO("TH", "thread wakeup, thread= " << thrd->name);

  if (thrd->_handler)
    (*thrd->_handler)(thrd->context);

  return 0;
}

//-------------------------------------------------------------------------------------------------------------
thread *thread::create(const char *nm, handler hdlr, void *contxt, uint stack_size) {
  return new linux_pthread(nm, hdlr, contxt, stack_size);
}

//-------------------------------------------------------------------------------------------------------------
void linux_pthread::start() {
  if ((_error_code == 0) && (!run_flag)) {
    DEBUG_INFO("TH", "Starting thread, thread= " << name);
    // Putting a lock here ensures we won't signal until thread was created an blocked waiting for this signal
    pthread_mutex_lock(&start_mutex);
    run_flag = true;
    pthread_mutex_unlock(&start_mutex);
    pthread_cond_signal(&start_cond);
  }
}

//-------------------------------------------------------------------------------------------------------------
void linux_pthread::exit() {
}

//-------------------------------------------------------------------------------------------------------------
int process::get_pid() {
  return getpid();
}

//-------------------------------------------------------------------------------------------------------------
process::~process() {
  if (is_running()) {
    kill(child_pid, SIGTERM);
    // Blocking get_status
    get_status();
  }
}

//-------------------------------------------------------------------------------------------------------------
void process::kill_child() {
  if (is_running()) {
    kill(child_pid, SIGTERM);
  }
}

//-------------------------------------------------------------------------------------------------------------
critical_section::critical_section() {
  pthread_mutex_init(&cs, NULL);
}

//-------------------------------------------------------------------------------------------------------------
void critical_section::enter() {
  pthread_mutex_lock(&cs);
}

//-------------------------------------------------------------------------------------------------------------
void critical_section::leave() {
  pthread_mutex_unlock(&cs);
}

//-------------------------------------------------------------------------------------------------------------
critical_section::~critical_section() {
  pthread_mutex_destroy(&cs);
}

#endif

} // end namespace
} // end woodycxx namespace

