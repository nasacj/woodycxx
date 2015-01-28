/*
  Copyright (c) 2014-2015 by NASa Qian <nasacj@nasacj.net>
  This file is part of the woodycxx library.

  This software is distributed under BSD 3-Clause License.
  The full license agreement can be found in the LICENSE file.

  This software is distributed without any warranty.
*/

#ifndef __SYSIPC_THREADS_H
#define __SYSIPC_THREADS_H
#include "sysipc.h"
#include <signal.h>
#ifdef WIN32
#include <windows.h>
#define WEXITSTATUS( x ) x
#define WTERMSIG( x ) x
#define THREAD_ID   uint32
#else
#include <pthread.h>
#define THREAD_ID   pthread_t
#define CRITICAL_SECTION pthread_mutex_t
#endif

/// \ingroup sysipc
/// \file sysipc_threads.h
/// \brief Thread support

namespace woodycxx { namespace sysipc
{

    /// \ingroup sysipc
    /// \brief Thread handler
    class thread_handler
    {
    public:
        virtual void thread_proc( uint param ) = 0;
    };

    /// \ingroup threads
    /// \brief Thread class
    class thread
    {
        // hide constructors
        thread();
        thread( const thread& );

    public:
        static const uint DEFAULT_STACKSIZE = 64 * 1024;
        enum priority
        {
            time_citical,
            normal,
            idle
        };
        thread& operator=( const thread& );

        static void sleep( uint ms );

        typedef void (*handler)( void* me );

    protected:
        //typedef void (*handler)( void* me );

        std::string     name;
        handler         _handler;
        void*           context;
        THREAD_ID       id;
        os_error        _error_code;
        bool            run_flag;
        bool            terminate_flag;
        uint            _stack_size;
        bool            _running;

        thread( const char* thread_name, handler hdlr, void* contxt, uint stack_size ) :
        name( thread_name ), _handler( hdlr ), context( contxt), id( 0 ), _error_code( 0 ), run_flag( false ),
            terminate_flag( false ), _stack_size( stack_size ), _running( false ){}

        static thread* create( const char* nm, handler hdlr, void* contxt, uint stack_size );

    public:
        virtual ~thread() = 0;
        /// \brief Use this method to create a thread. Threads can only be created on the heap.
        virtual bool wait_ready(){ return is_ok(); }
        virtual void start() = 0;
        virtual void exit() = 0;
        os_error get_error() const { return _error_code; }
        bool is_ok() const { return _error_code == 0; }
        void terminate(){ terminate_flag = true; }
        bool should_terminate() const { return terminate_flag; }
        bool is_running(){ return _running; }
    };

    template<class T>
    class Tthread : public thread
    {
    public:
        typedef void (*thandler)( T* context );
        static Tthread<T>* create( const char* nm, thandler hdlr, T* context, uint stack_size = DEFAULT_STACKSIZE )
        {
            return static_cast<Tthread<T>*> ( thread::create( nm, reinterpret_cast<handler>(hdlr), (void*) context, stack_size) );
        }
    };

    /*
    class Runnable
    {
    public:
        virtual void run() = 0;
    };

    
    template<class R> class Thread
    {
    private:
        thread* my_thread;

    public:
        Thread()
        {
            my_thread = thread::create("", woodycxx::sysipc::thread::handler(&(R::run)), NULL, thread::DEFAULT_STACKSIZE);
        }

        ~Thread()
        {
            if (my_thread)
            {
                delete my_thread;
            }
        }
        void start(){ my_thread->start(); }

    };*/

    /// \ingroup threads
    /// \brief retry_timer
    class retry_timer
    {
        uint    max_delay;
        uint    current_delay;

    public:
        retry_timer( uint log2_t ) : max_delay( 1 << ( log2_t - 1 ) ){ reset(); }
        void reset(){ current_delay = 1; }

        bool wait()
        {
            if ( current_delay > max_delay )
            {
                reset();
                return false;
            }

            thread::sleep( current_delay );
            current_delay <<= 1;
            return true;
        };
    };




    /// \ingroup threads
    /// \brief Process class
    /// Allows forking and controlling a child process as well as piping to the child process's STDIN, STDOUT.
    class process
    {
    public:
        enum states
        {
            init,
            running,
            complete
        };

    protected:
        typedef int    (*handler)( void* context );

    private:
        int     to_pipe[ 2 ];
        int     from_pipe[ 2 ];
        int     parent_pid;
        int     child_pid;
        int     error;
        handler _handler;
        void*   _context;
        states  _child_state;
        bool    _child_error;

    protected:
        process( handler h, void* context );
        ~process();

    public:
        int get_read_fd() { return from_pipe[ 0 ]; }
        int get_write_fd() { return to_pipe[ 1 ]; }

        static int get_pid();
        bool is_parent() const;

        /// \brief This call forks the child process. Once the child process completes, it can be forked again by calling start
        /// \return true if sucessful, false if not
        bool start();

        bool is_running(){ return _child_state == running; }
        bool is_complete(){ return _child_state == complete; }
        bool is_error(){ return _child_error; }

        /// brief Waits for the process to complete.
        void wait_complete(){ get_status( true ); }
        void kill_child();
        int  get_child_pid(){ return child_pid; }

        /// \brief Checks on the process status
        /// \param wait true indicates blocking call, will wait until process completes, false: non-blocking
        void get_status( bool wait = false );

        /// \brief Makes the process more friendly, allowing other processes more CPU time.
        void make_nice(int priority = 10);
    };

    /// \brief Type specific process class
    template<class T>
    class Tprocess : public process
    {
    public:
        typedef int (*thandler)( T* context );

        /// \brief Create a type specific process object. When forked (start), the child proces will call the handler.
        /// \param h        Static handler function called by the child process when start is invoked.
        /// \param context  pointer to instance of context class to be used by the handler.
        Tprocess( thandler h, T* context ) :
        process( reinterpret_cast<handler>( h ), context ){}
    };

    /// \brief critical section
    class critical_section
    {
        CRITICAL_SECTION cs;
    public:
        critical_section();
        ~critical_section();
        void enter();
        void leave();
    };

    /// \brief critical section lock
    /// Enter and leave critical sections using RAII pattern
    class crit_section_lock
    {
        critical_section& _cs;
    public:

        crit_section_lock( critical_section& cs) : _cs(cs) 
        {
            _cs.enter();
        };

        ~crit_section_lock()
        {
            _cs.leave();
        };
    };

    /// \brief signal handler
    class signal_handler
    {
    public:
        typedef void (*handler)( int signum );
    public:
        static void reg( int signum, handler h )
        {
            signal( signum, h );
        }
    };  
} // end namespace
} // end woodycxx namespace

#endif
