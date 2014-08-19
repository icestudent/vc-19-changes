/***
* ==++==
*
* Copyright (c) Microsoft Corporation. All rights reserved.
*
* ==--==
* =+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
*
* pplconcrt.h
*
* Parallel Patterns Library - PPL ConcRT helpers
*
* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
****/

#pragma once

#ifndef _PPLCONCRT_H
#define _PPLCONCRT_H

#include <pplinterface.h>
#include <condition_variable>
#include <mutex>

#if !defined(BUILD_WINDOWS) && !defined(_CRT_WINDOWS)
#include <concrt.h>
#else !defined(__cplusplus_winrt)
#include <system_error>
#include <windows.h>
#endif

namespace Concurrency
{

// The extensibility namespace contains the type definitions that are used by ppltasks implementation
namespace extensibility
{
    typedef ::std::condition_variable condition_variable_t;
    typedef ::std::mutex critical_section_t;
    typedef std::unique_lock< ::std::mutex>  scoped_critical_section_t;

#if !defined(BUILD_WINDOWS) && !defined(_CRT_WINDOWS)
    typedef ::Concurrency::event event_t;
    typedef ::Concurrency::reader_writer_lock reader_writer_lock_t;
    typedef ::Concurrency::reader_writer_lock::scoped_lock scoped_rw_lock_t;
    typedef ::Concurrency::reader_writer_lock::scoped_lock_read scoped_read_lock_t;

    typedef ::Concurrency::details::_ReentrantBlockingLock recursive_lock_t;
    typedef recursive_lock_t::_Scoped_lock scoped_recursive_lock_t;
#endif
}  // namespace Concurrency::extensibility

_CRTIMP2 bool __cdecl is_current_task_group_canceling();

#if defined(BUILD_WINDOWS) || defined(_CRT_WINDOWS)
namespace details
{

#if defined(__cplusplus_winrt)
class default_scheduler : public ::Concurrency::scheduler_interface
{
public:
    virtual void schedule( TaskProc_t proc, _In_ void* param)
    {
        auto workItemHandler = ref new Windows::System::Threading::WorkItemHandler([proc, param](Windows::Foundation::IAsyncAction ^)
        {
            proc(param);
        });

        Windows::System::Threading::ThreadPool::RunAsync(workItemHandler); 
    }
};
#else

class default_scheduler : public ::Concurrency::scheduler_interface
{
public:
    struct _Scheduler_param
    {
        TaskProc_t m_proc;
        void * m_param;

        _Scheduler_param(TaskProc_t proc, _In_ void * param)
            : m_proc(proc), m_param(param)
        {}

        static void CALLBACK DefaultWorkCallback(PTP_CALLBACK_INSTANCE, PVOID pContext, PTP_WORK)
        {
            auto schedulerParam = reinterpret_cast<_Scheduler_param*>(pContext);
            schedulerParam->m_proc(schedulerParam->m_param);
            delete schedulerParam;
        }
    };

    virtual void schedule( TaskProc_t proc, _In_ void* param)
    {
        auto schedulerParam = new _Scheduler_param(proc, param);
        auto work = ::CreateThreadpoolWork(_Scheduler_param::DefaultWorkCallback, schedulerParam, nullptr);

        if (work == nullptr)
        {
            delete schedulerParam;
            throw std::system_error(static_cast<int>( ::GetLastError()), std::system_category());
        }

        ::SubmitThreadpoolWork(work);
        ::CloseThreadpoolWork(work);
    }
};
#endif // defined(__cplusplus_winrt)

// Spin lock to allow for locks to be used in global scope
class _Spin_lock
{
public:
    _Spin_lock() : _M_lock(0) {}

    void lock()
    {
        while ( details::atomic_compare_exchange(_M_lock, 1l, 0l) != 0l );
        {
            YieldProcessor();
        } 
    }

    void unlock()
    {
        // fence for release semantics
        details::atomic_exchange(_M_lock, 0l);
    }

private:
    atomic_long _M_lock;
};

__declspec(selectany) _Spin_lock _g_SpinLock;
__declspec(selectany) std::shared_ptr< ::Concurrency::scheduler_interface> _g_Scheduler;

} // namespace details
#endif


inline std::shared_ptr< ::Concurrency::scheduler_interface> get_ambient_scheduler()
{
#if !defined(BUILD_WINDOWS) && !defined(_CRT_WINDOWS)
	return nullptr;
#else
    if ( !details::_g_Scheduler)
    {
        ::std::lock_guard< details::_Spin_lock> _Lock(details::_g_SpinLock);
        if (!details::_g_Scheduler)
        {
            details::_g_Scheduler = std::make_shared< ::Concurrency::details::default_scheduler>();
        }
    }

    return details::_g_Scheduler;
#endif
}

inline void set_ambient_scheduler(const std::shared_ptr< ::Concurrency::scheduler_interface>& _Scheduler)
{
#if !defined(BUILD_WINDOWS) && !defined(_CRT_WINDOWS)
    (_Scheduler);
    throw invalid_operation("Scheduler is already initialized");
#else
    ::std::lock_guard< details::_Spin_lock> _Lock(details::_g_SpinLock);

    if (details::_g_Scheduler != nullptr)
    {
        throw invalid_operation("Scheduler is already initialized");
    }

    details::_g_Scheduler = _Scheduler;
#endif
}

namespace details
{
    typedef void (__cdecl * _UnobservedExceptionHandler)(void);
    _CRTIMP2 void __cdecl _SetUnobservedExceptionHandler(_UnobservedExceptionHandler);

    // Used to report unobserved task exceptions in ppltasks.h
    _CRTIMP2 void __cdecl _ReportUnobservedException();

    namespace platform 
    {
        _CRTIMP2 unsigned int __cdecl GetNextAsyncId();
        _CRTIMP2 size_t __cdecl CaptureCallstack(void **stackData, size_t skipFrames, size_t captureFrames);
        _CRTIMP2 long __cdecl GetCurrentThreadId();
    }
}

} // Concurrency

#include <pplcancellation_token.h>
#if !defined(BUILD_WINDOWS) && !defined(_CRT_WINDOWS)
#include <ppl.h>
#endif

namespace Concurrency
{

namespace details
{

// It has to be a macro because the debugger needs __debugbreak
// breaks on the frame with exception pointer.
// It can be only used within _ExceptionHolder
#ifndef _REPORT_PPLTASK_UNOBSERVED_EXCEPTION
#define _REPORT_PPLTASK_UNOBSERVED_EXCEPTION() do { \
        ReportUnhandledError(); \
        __debugbreak(); \
        Concurrency::details::_ReportUnobservedException(); \
} while(false)

#endif

    template<typename _T>
    struct _AutoDeleter
    {
        _AutoDeleter(_T *_PPtr) : _Ptr(_PPtr) {}
        ~_AutoDeleter () { delete _Ptr; } 
        _T *_Ptr;
    };

    struct _TaskProcHandle 
#if !defined(BUILD_WINDOWS) && !defined(_CRT_WINDOWS)
        : Concurrency::details::_UnrealizedChore
#endif
    {
        _TaskProcHandle()
        {
#if !defined(BUILD_WINDOWS) && !defined(_CRT_WINDOWS)
            this->m_pFunction = &Concurrency::details::_UnrealizedChore::_InvokeBridge<_TaskProcHandle>;
            this->_SetRuntimeOwnsLifetime(true);
#endif
        }

        virtual ~_TaskProcHandle() {}
        virtual void invoke() const = 0;

        void operator()() const
        {
            this->invoke();
        }

        static void __cdecl _RunChoreBridge(void * _Parameter)
        {
            auto _PTaskHandle = static_cast<_TaskProcHandle *>(_Parameter);
            _AutoDeleter<_TaskProcHandle> _AutoDeleter(_PTaskHandle);
            _PTaskHandle->invoke();
        }
    };

    class _TaskCollectionBaseImpl
    {
    protected:
        enum _TaskCollectionState {
            _New,
            _Scheduled,
            _Compleated
        };

        void SetCollectionState(_TaskCollectionState _NewState)
        {
            _ASSERTE(_NewState != _New);
            extensibility::scoped_critical_section_t _lock(_M_Cs);
            if (_M_State < _NewState) {
                    _M_State = _NewState;
            }

            _M_StateChanged.notify_all();
        }

        void WaitUntilStateChangedTo(_TaskCollectionState _State)
        {
            extensibility::scoped_critical_section_t _lock(_M_Cs);

            while(_M_State < _State) {
                _M_StateChanged.wait(_lock);
            }
        }
    public:

        typedef Concurrency::details::_TaskProcHandle _TaskProcHandle_t;

        _TaskCollectionBaseImpl(::Concurrency::scheduler_ptr _PScheduler) 
            : _M_pScheduler(_PScheduler), _M_State(_New)
        {
        }

        void _ScheduleTask(_TaskProcHandle_t* _Parameter, _TaskInliningMode _InliningMode)
        {
            if (_InliningMode == _ForceInline)
            {
                _TaskProcHandle_t::_RunChoreBridge(_Parameter);
            }
            else
            {
                _M_pScheduler->schedule(_TaskProcHandle_t::_RunChoreBridge, _Parameter);
            }
        }

        void _Cancel()
        {
            // No cancellation support
        }

        void _RunAndWait()
        {
            _Wait();
        }

        void _Wait()
        {
            WaitUntilStateChangedTo(_Compleated);
        }

        void _Complete()
        {
            // Ensure that RunAndWait makes progress.
            SetCollectionState(_Compleated);
        }

        ::Concurrency::scheduler_ptr _GetScheduler() const
        {
            return _M_pScheduler;
        }

        // Fire and forget
        static void _RunTask(TaskProc_t _Proc, void * _Parameter, _TaskInliningMode _InliningMode)
        {
            if (_InliningMode == _ForceInline)
            {
                _Proc(_Parameter);
            }
            else
            {
                // Schedule the work on the ambient scheduler
                get_ambient_scheduler()->schedule(_Proc, _Parameter);
            }
        }

        static bool _Is_cancellation_requested()
        {
            return false;
        }

    protected:
        ::Concurrency::extensibility::condition_variable_t _M_StateChanged;
        ::Concurrency::extensibility::critical_section_t _M_Cs;
        ::Concurrency::scheduler_ptr _M_pScheduler;
        _TaskCollectionState _M_State;
    };

#if !defined(BUILD_WINDOWS) && !defined(_CRT_WINDOWS)
    // This is an abstraction that is built on top of the scheduler to provide these additional functionalities
    // - Ability to wait on a work item
    // - Ability to cancel a work item
    // - Ability to inline work on invocation of RunAndWait
    // The concrt specific implementation provided the following additional features
    // - Interoperate with concrt task groups and ppl parallel_for algorithms for cancellation
    // - Stack guard
    // - Determine if the current task is cancelled
    class _TaskCollectionConcRtImpl : public _TaskCollectionBaseImpl
    {
    public:
        _TaskCollectionConcRtImpl(::Concurrency::scheduler_ptr _PScheduler) 
            : _TaskCollectionBaseImpl(_PScheduler), _M_pTaskCollection(nullptr)
        {
        }

        ~_TaskCollectionConcRtImpl()
        {
            if (_M_pTaskCollection != nullptr)
            {
                _M_pTaskCollection->_Release();
                _M_pTaskCollection = nullptr;
            }
        }

        void _ScheduleTask(_TaskProcHandle_t* _Parameter, _TaskInliningMode _InliningMode)
        {
            if (!_M_pScheduler)
            {
                // Construct the task collection; We use none token to prevent it becoming interruption point.
                _M_pTaskCollection = _AsyncTaskCollection::_NewCollection(::Concurrency::details::_CancellationTokenState::_None());
            }

            try 
            {
                if (_M_pTaskCollection != nullptr)
                {
                    // Do not need to check its returning state, more details please refer to _Wait method.
                    auto _PChore = static_cast< ::Concurrency::details::_UnrealizedChore*>(_Parameter);
                    _M_pTaskCollection->_ScheduleWithAutoInline(_PChore, _InliningMode);
                }
                else
                {
                    // Schedule the work on the user provided scheduler
                    if (_InliningMode == _ForceInline)
                    {
                        _TaskProcHandle_t::_RunChoreBridge(_Parameter);
                    }
                    else
                    {
                        _M_pScheduler->schedule(_TaskProcHandle_t::_RunChoreBridge, _Parameter);
                    }
                }
            }
            catch(...)
            {
                SetCollectionState(_Scheduled);
                throw;
            }

            // Set the event in case anyone is waiting to notify that this task has been scheduled. In the case where we
            // execute the chore inline, the event should be set after the chore has executed, to prevent a different thread 
            // performing a wait on the task from waiting on the task collection before the chore is actually added to it, 
            // and thereby returning from the wait() before the chore has executed.
            SetCollectionState(_Scheduled);
        }

        void _Cancel()
        {
            // Ensure that RunAndWait makes progress.
            SetCollectionState(_Scheduled);

            if (_M_pTaskCollection != nullptr)
            {
                _M_pTaskCollection->_Cancel();
            }
        }

        void _RunAndWait()
        {
            if (_M_pTaskCollection != nullptr)
            {
                WaitUntilStateChangedTo(_Scheduled);

                // When it returns cancelled, either work chore or the cancel thread should already have set task's state
                // properly -- cancelled state or completed state (because there was no interruption point). 
                // For tasks with unwrapped tasks, we should not change the state of current task, since the unwrapped task are still running.
                _M_pTaskCollection->_RunAndWait();
            }
            else
            {
                WaitUntilStateChangedTo(_Compleated);
            }
        }

        // Fire and forget
        static void _RunTask(TaskProc _Proc, void * _Parameter, _TaskInliningMode _InliningMode)
        {
            Concurrency::details::_StackGuard _Guard;

            if (_Guard._ShouldInline(_InliningMode))
            {
                _Proc(_Parameter);
            }
            else
            {
                // Schedule the work on the current scheduler
                _CurrentScheduler::_ScheduleTask(_Proc, _Parameter);
            }
        }

        static bool _Is_cancellation_requested()
        {
            // ConcRT scheduler under the hood is using TaskCollection, which is same as task_group
            return ::Concurrency::is_current_task_group_canceling();
        }

    private:
        _AsyncTaskCollection* _M_pTaskCollection;
    };

    typedef ::Concurrency::details::_TaskCollectionConcRtImpl _TaskCollection_t;
#else
    typedef ::Concurrency::details::_TaskCollectionBaseImpl _TaskCollection_t;
#endif

    // For create_async lambdas that return a (non-task) result, we oversubscriber the current task for the duration of the
    // lambda.
    struct _Task_generator_oversubscriber
    {
        _Task_generator_oversubscriber()
        {
#if !defined(BUILD_WINDOWS) && !defined(_CRT_WINDOWS)
            _Context::_Oversubscribe(true);
#endif
        }

        ~_Task_generator_oversubscriber()
        {
#if !defined(BUILD_WINDOWS) && !defined(_CRT_WINDOWS)
            _Context::_Oversubscribe(false);
#endif
        }
    };

    typedef ::Concurrency::details::_TaskInliningMode _TaskInliningMode_t;
    typedef ::Concurrency::details::_Task_generator_oversubscriber _Task_generator_oversubscriber_t;
} // details

} // Concurrency

namespace concurrency = Concurrency;

#endif // _PPLCONCRT_H
