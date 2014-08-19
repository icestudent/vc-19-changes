/***
* ==++==
*
* Copyright (c) Microsoft Corporation. All rights reserved.
*
* ==--==
* =+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
*
* pplawait.h
*
* Await Compiler Support for Parallel Patterns Library - PPL Tasks
*
* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
****/

#pragma once

#ifndef _PPLAWAIT_H
#define _PPLAWAIT_H

#include <ppltasks.h>
#include <windows.h>
#include <map>
#include <set>

namespace Concurrency
{
	namespace details
	{
		// Represent a pool of fibers. 
		// The DeleteFiber operation is quite expensive. So if there are a rapidly number of growing fibers, re-use older fibers that 
		// were recently deleted rather than creating / deleting a new fiber.
		// This will delete fibers as the count grow smaller again, and will keep 2 or 3 around for immediate use.
		// This also provides an official 'SetFiberData' functionality, which we need and is not available via WinRT.
		class FiberPool
		{
			struct FiberInfo
			{
				LPFIBER_START_ROUTINE lpStartAddress;
				LPVOID lpParameter;
			};

			std::map<LPVOID, FiberInfo> fibers;
			std::set<LPVOID> availableFibers;
			SRWLOCK srwlock;

			// Fiber pool function. This just continues to 'run' (only when explicitly 
			// scheduled of course), until we delete the fiber.
			static VOID CALLBACK FiberProc(PVOID lpParameter)
			{
				FiberPool* pool = (FiberPool*)lpParameter;
				while (true)
				{
					AcquireSRWLockShared(&pool->srwlock);
						auto fiberEntry = pool->fibers.find(GetCurrentFiber());
						_ASSERT( fiberEntry != pool->fibers.end() );

						auto fiberInfo = fiberEntry->second;

					ReleaseSRWLockShared (&pool->srwlock);

					_ASSERT(fiberInfo.lpStartAddress);

					LPFIBER_START_ROUTINE lpStartAddress = fiberInfo.lpStartAddress;

					// Set the start address to null, so that we can detect if we're spinning around
					// in a loop for no reason. This should be reset to something else when the fiber 
					// gets spun again
					fiberInfo.lpStartAddress = nullptr;

					// Calls the real fiber workitem. This will suspend the fiber when done. On wakeup
					// a new lpStartAddress is expected to be set.
					lpStartAddress( fiberInfo.lpParameter );
				}
			}

		public:
			int allocSize;  // Default reserve size.
			int commitSize; // Default comit size.


			// Create a new fiber pool
			FiberPool()
			{
				allocSize = 1048576; // 1 mb (equivalent to a standard Win32 stack)
				commitSize = 8192;   // 1 page data, 1 page guard
				InitializeSRWLock(&srwlock);
			}

			// Deletes the fiber pool, which deletes all of the existing unused fibers.
			// This will not delete or block on any running fibers. 
			// This is generally called from DLL detach - it is considered a program
			// bug to unload the DLL while the DLL has open awaits.
			~FiberPool()
			{
				AcquireSRWLockExclusive(&srwlock);

				for (auto fiber : availableFibers)
				{
					::DeleteFiber( fiber );
				}

				availableFibers.clear();

				ReleaseSRWLockExclusive(&srwlock);
			}

			// Returns the count of actively Running Fibers
			size_t RunningFibers()
			{
				AcquireSRWLockShared (&srwlock);

				size_t sz =  fibers.size();

				ReleaseSRWLockShared (&srwlock);

				return sz;
			}

			// Returns the count of all Fibers in the pool, running or available
			size_t UsedFibers()
			{
				AcquireSRWLockShared (&srwlock);

				size_t sz =  availableFibers.size() + fibers.size();

				ReleaseSRWLockShared (&srwlock);

				return sz;
			}

			// Fiber pool equivalent of the Win32 API GetFiberData
			LPVOID GetFiberData(void)
			{
				AcquireSRWLockShared (&srwlock);

				auto fiberEntry = fibers.find(GetCurrentFiber());
				_ASSERT( fiberEntry != fibers.end() );

				auto fiberInfo = fiberEntry->second;

				ReleaseSRWLockShared (&srwlock);

				return fiberInfo.lpParameter;
			}

			// Sets the data on a fiber. (No WinRT API equivalent)
			void SetFiberData(LPVOID data)
			{
				AcquireSRWLockShared (&srwlock);

				auto fiberEntry = fibers.find(GetCurrentFiber());
				_ASSERT( fiberEntry != fibers.end() );

				auto fiberInfo = fiberEntry->second;
				fiberInfo.lpParameter = data;

				fiberEntry->second = fiberInfo;

				ReleaseSRWLockShared (&srwlock);
			}

			// Fiber pool equivalent of the Win32 API CreateFiberEx
			LPVOID CreateFiberEx(
				_In_ SIZE_T dwStackCommitSize, 
				_In_ SIZE_T dwStackReserveSize, 
				_In_ DWORD dwFlags, 
				_In_ LPFIBER_START_ROUTINE lpStartAddress, 
				_In_opt_ LPVOID lpParameter)
			{
				AcquireSRWLockExclusive(&srwlock);

				LPVOID lpFiber;
				if (availableFibers.size() > 0)
				{
					lpFiber = *availableFibers.begin();
					availableFibers.erase( lpFiber );
				}
				else
				{
					lpFiber = ::CreateFiberEx( dwStackCommitSize, dwStackReserveSize, dwFlags, &FiberProc, this);
				}

				FiberInfo fiberInfo = { lpStartAddress, lpParameter };
				fibers[lpFiber] = fiberInfo;

				ReleaseSRWLockExclusive(&srwlock);

				return lpFiber;
			}

			// Fiber pool equivalent of the Win32 API DeleteFiber
			VOID DeleteFiber( _In_ LPVOID lpFiber	)
			{
				AcquireSRWLockExclusive(&srwlock);

				fibers.erase(lpFiber);
				availableFibers.insert( lpFiber );

				size_t totalFiberCount = availableFibers.size() + fibers.size();
				size_t availableFiberCount = availableFibers.size();
				float percentage = (float)availableFiberCount / (float)totalFiberCount;

				// If we've dropped by 75%, let half of them go
				int deleteCount = 0;
				if (percentage > 0.75)
				{
					availableFiberCount /= 2;
					while (availableFiberCount-- > 0)
					{
						deleteCount++;
						LPVOID lpFiber = *availableFibers.begin();
						availableFibers.erase( lpFiber );

						::DeleteFiber(lpFiber);

					}
				}

				ReleaseSRWLockExclusive(&srwlock);
			}
		};

		__declspec(selectany) FiberPool fiberPool;

		// Use POOL #define in order to test between using the fiber pool or making the
		// Win32 API calls directly.
		#define POOL fiberPool.
		// #define POOL

		// The __resumable_func_fiber_data structure that contains the information about the fiber running the resumable
		// function. This structure is placement allocated at the bottom of the stack of the fiber that runs the 
		// resumable function.
		struct __resumable_func_fiber_data
		{
			int magic1;                 // Check for structure corruption
			LPVOID threadFiber;         // The thread fiber from ConvertThreadToFiber (to which we return when done or awaiting).
			LPVOID resumableFuncFiber;  // The fiber on which this function will run
			bool   returned;            // The function returned
			int    awaitingThreadId;    // The thread that last called the await function.
			volatile LONG refcount;     // refcount of the fiber
			volatile bool suspending;   // The fiber is busy suspending (i.e. user called await, which scheduled work, but hasn't returned yet).
			int magic2;					// Check for structure corruption

			__resumable_func_fiber_data() : magic1(0x12344321), magic2(0x12344321), threadFiber(nullptr), resumableFuncFiber(nullptr), returned(false), awaitingThreadId(0), refcount(1), suspending(false) 
			{
			}

			void verify()
			{
				_ASSERT(magic1 == 0x12344321);
				_ASSERT(magic2 == 0x12344321);
			}

			// Adds a reference to the current fiber
			void AddRef()
			{
				InterlockedIncrement(&refcount);
			}

			// Releases a reference to the fiber. If the reference count drops to 0 the fiber is deleted.
			void Release()
			{
				LONG ref = InterlockedDecrement(&refcount);
				if (ref == 0)
				{
					_ASSERT( GetCurrentFiber() != resumableFuncFiber);
					POOL DeleteFiber(resumableFuncFiber);
				}
			}

			// Get the __resumable_func_fiber_data for the current fiber.
			static __resumable_func_fiber_data* GetCurrentResumableFuncData()
			{
				_ASSERT( IsThreadAFiber() );
				__resumable_func_fiber_data* pAwaitInfo = (__resumable_func_fiber_data*)POOL GetFiberData();

				_ASSERT(pAwaitInfo);

				return pAwaitInfo;
			}

			// Structure used by ConvertCurrentThreadToFiber in order to pass back to ConvertFiberBackToThread
			struct PreviousState
			{
				bool   createdFiberFromThread;
				LPVOID fiberData;
			};

			// Convert the thread to a fiber. This returns a structure which can be passed back to ConvertFiberBackToThread.
			static PreviousState ConvertCurrentThreadToFiber()
			{
				PreviousState state = { false, nullptr };
				if (!IsThreadAFiber())
				{
					// Convert the thread to a fiber. Use FIBER_FLAG_FLOAT_SWITCH on x86.
					LPVOID threadFiber = ConvertThreadToFiberEx( nullptr, FIBER_FLAG_FLOAT_SWITCH);
					if (threadFiber == NULL)
					{
						throw std::bad_alloc();
					}

					state.createdFiberFromThread = true;
				}
				else
				{
					state.fiberData = POOL GetFiberData();
				}
				return state;
			}

			// Convert the fiber back to a thread, using the previously returned data from ConvertCurrentThreadToFiber.
			static void ConvertFiberBackToThread(PreviousState previousState)
			{
				if (previousState.createdFiberFromThread)
				{
					if ( ConvertFiberToThread() == FALSE ) 
					{
						throw std::bad_alloc();
					}
				}
				else
				{
					POOL SetFiberData( previousState.fiberData );
				}
			}
		};

		// The data that we use to set up the resumable function fiber. This is placed on the trampoline
		// function's stack, passed by pointer to the suspendable function, which then copies some data over
		// before we allow the trampoline function to disappear.
		template <typename TTask, typename TFunc, typename TTaskType>
		class __resumable_func_setup_data
		{
			__resumable_func_fiber_data*	_func_data;
			LPVOID							_pSystemFiber;
			LPVOID							_pvLambda;
			task_completion_event<TTaskType> _tce;

		public:
			// Execute the lambda that calls into the suspendable function, and set the returned
			// result on the tce.
			template <typename TFunc, typename TaskType>
			static void ExecuteAndSetTCE(task_completion_event<TaskType>& tce, TFunc& lambda)
			{
				try
				{
					tce.set( lambda() );
				}
				catch(...)
				{
					tce.set_exception(std::current_exception());
				}
			}

			// Specialization for task<void>
			template <>
			static void ExecuteAndSetTCE(task_completion_event<void>& tce, TFunc& lambda)
			{
				try
				{
					lambda();
					tce.set();
				}
				catch(...)
				{
					tce.set_exception(std::current_exception());
				}
			}

			// Entry proc for the Resumable Function Fiber.
			static VOID CALLBACK ResumableFuncFiberProc(PVOID lpParameter)
			{
				LPVOID threadFiber;

				// This function does not formally return, due to the SwitchToFiber call at the bottom.
				// This scope block is needed for the destructors of the locals in this block to fire
				// before we do the SwitchToFiber.
				{
					__resumable_func_fiber_data func_data_on_fiber_stack;
					__resumable_func_setup_data* pSetupData = (__resumable_func_setup_data*)lpParameter;

					// The callee needs to setup some more stuff after we return (which would be either on
					// await or an ordinary return). Hence the callee needs the pointer to the func_data 
					// on our stack. This is not unsafe since the callee has a refcount on this structure
					// which means the fiber will continue to live.
					pSetupData->_func_data = &func_data_on_fiber_stack;

					POOL SetFiberData(&func_data_on_fiber_stack);

					func_data_on_fiber_stack.threadFiber = pSetupData->_pSystemFiber;
					func_data_on_fiber_stack.resumableFuncFiber = GetCurrentFiber();

					{
						task_completion_event<TTaskType> tce = pSetupData->_tce;

						TFunc* pFunctor = (TFunc*)(pSetupData->_pvLambda);
						TFunc lambda = std::move(*pFunctor);
			
						ExecuteAndSetTCE<TFunc, TTaskType>(tce, lambda);
					}

					// After this point assume pSetupData is pointing to invalid memory. This can happen because 'ExecuteAndSetTce' may
					// have caused us to switch threads, and 'pSetupData' points to the stack on the originating thread, which
					// may no longer be present.

					func_data_on_fiber_stack.returned = true; // We set return to true meaning this is the final 'real' return and not one of the 'await' returns.
					threadFiber = func_data_on_fiber_stack.threadFiber;
				}

				SwitchToFiber( threadFiber );

				// On a normal fiber this function won't exit after this point. However, if the fiber is in a fiber-pool 
				// and re-used we can get control back. So just exit this function, which will cause the fiber pool to spin around and re-enter.
			}

			// Run a specific functor inside a resumable context. The functor is of the format: void ();
			static TTask RunActionInResumableContext(TFunc& functor)
			{
				// Convert the current thread to a fiber. This is needed because the thread needs to "be" a fiber in order
				// to be able to switch to another fiber.
				auto previousState = __resumable_func_fiber_data::ConvertCurrentThreadToFiber();

				// Set up the initialization data on the stack, and pass it to the other fiber (which will copy it to that stack)
				__resumable_func_setup_data __init_on_stack; 
				__init_on_stack._func_data = nullptr;
				__init_on_stack._pSystemFiber = GetCurrentFiber();

				TFunc* pFunctor = &functor;
				__init_on_stack._pvLambda = pFunctor;

				// Create a new fiber (or re-use an existing one from the pool)
				LPVOID resumableFuncFiber = POOL CreateFiberEx( fiberPool.commitSize, fiberPool.allocSize, FIBER_FLAG_FLOAT_SWITCH, &ResumableFuncFiberProc, &__init_on_stack);
				if (!resumableFuncFiber)
				{
					throw std::bad_alloc();
				}

				// Switch to the newly created fiber. When this "returns" the functor has either returned, or issued an 'await' statement.
				SwitchToFiber( resumableFuncFiber );

				// Create a task to return to the caller.
				// NOTE: When moving over to PPLx, this should see if the function did a direct exit, and if so
				// just immediately return without going via the tce.
				TTask tsk( __init_on_stack._tce );

				__init_on_stack._func_data->suspending = false;
				__init_on_stack._func_data->Release();

				__resumable_func_fiber_data::ConvertFiberBackToThread( previousState );

				return tsk;
			}
		};

		// Await a task. This can only be called from a context that has been called from RunActionInResumableContext.
		// It will suspend the state of the current fiber and switch to the original calling thread
		// to return the function.
		// This is called from the compiler '__await' operator. (As-is).
		template <typename T>
		__declspec(noinline) T await_task(concurrency::task<T> func)
		{
			// This can run concurrently with the previous .then() call.
			_ASSERT( IsThreadAFiber() );
			__resumable_func_fiber_data* func_data_on_fiber_stack = __resumable_func_fiber_data::GetCurrentResumableFuncData();

			// Add-ref's the fiber. Even though there can only be one thread active in the fiber context, there
			// can be multiple threads accessing the fiber data.
			func_data_on_fiber_stack->AddRef();

			_ASSERT(func_data_on_fiber_stack);
			func_data_on_fiber_stack->verify();

			// Mark as busy suspending. We cannot run the code in the 'then' statement 
			// concurrently with the await doing the setting up of the fiber.
			_ASSERT( !func_data_on_fiber_stack->suspending );
			func_data_on_fiber_stack->suspending = true;

			// Make note of the thread that we're being called from to see if .then
			// resumes on the same thread.
			func_data_on_fiber_stack->awaitingThreadId = GetCurrentThreadId();
			bool directReturn = false;

			_ASSERT( func_data_on_fiber_stack->resumableFuncFiber == GetCurrentFiber() );
			LPVOID threadFiber = func_data_on_fiber_stack->threadFiber;

			func.then( [func_data_on_fiber_stack, &directReturn] (concurrency::task<T>) 
			{
				// NOTE: This routing can be running concurrently with a then spawned by the next await call.

				func_data_on_fiber_stack->verify();

				// See if we have a PPL callback on the same thread that we issued 'await'
				// on. Either this will be because of a message loop, or a task that has
				// results immediately ready.
				if (IsThreadAFiber() && (func_data_on_fiber_stack->awaitingThreadId == GetCurrentThreadId()))
				{
					LPVOID currentFiber = GetCurrentFiber();

					if ( func_data_on_fiber_stack->resumableFuncFiber == currentFiber )
					{
						// Yeah, we're in a direct callback situation. That means the await_task
						// function is blocking on the .then() call. Just set 'directReturn' and 
						// return.
						directReturn = true;
						_ASSERT( func_data_on_fiber_stack->resumableFuncFiber == currentFiber );

						func_data_on_fiber_stack->suspending = false;

						// Release the refcount that was added by 'await_task'.
						func_data_on_fiber_stack->Release();

						return;
					}
				}

				// Covert the thread to a fiber so we can do a fiber switch
				auto previousState = __resumable_func_fiber_data::ConvertCurrentThreadToFiber();

				LPVOID resumableFuncFiber = func_data_on_fiber_stack->resumableFuncFiber;

				_ASSERT(resumableFuncFiber);
				func_data_on_fiber_stack->threadFiber = GetCurrentFiber();

				while (func_data_on_fiber_stack->suspending)
				{
					// We could have a situation where we have an await, which triggers the .then()
					// which then calls into the function calling another await, triggering another .then().
					// However, the first .then() can be busy exiting while the next one wants to enter...
					//
					// So spin until the first .then() has time to get out of the fiber, since only one
					// thread can be in a fiber at a time.
				}

				// Execute the resuming function. When it 'await's again it will call back into the 'await' and we'll call
				// SwitchToFiber to switch back to our currently running fiber.
				SwitchToFiber(  resumableFuncFiber );

				// Someone must have explicitly called 'SwitchToFiber' (by returning or doing 'await') for us to be back here.

				// See if this was an actual return or an 'await'.
				if (func_data_on_fiber_stack->returned)
				{
					_ASSERT( resumableFuncFiber != GetCurrentFiber() );
				}

				_ASSERT( IsThreadAFiber() );
				__resumable_func_fiber_data::ConvertFiberBackToThread( previousState );

				// Set suspending to 'false' - we can let the next .then() run
				func_data_on_fiber_stack->suspending = false;
				func_data_on_fiber_stack->Release();
			}
		#ifdef __cplusplus_winrt
			, task_continuation_context::use_current()
		#endif
			);

			if (!directReturn)
			{
				// Return the calling thread.
				SwitchToFiber( threadFiber );
			}

			// If we're at this line either:
			//    * We never switched back to the system fiber (the .then returned immediately)
			//    * The .then continuation has a result and switched us back to the resumable fiber.
			// either way, we can now call .get() which should never block.
			// NOTE: Once we have PPLx, do an assert for is_ready.
			return func.get();
		}

		#ifdef __cplusplus_winrt
		// Allow a await_task on a WinRT IAsyncOperation
		template <typename T>
		__declspec(noinline) T await_task(Windows::Foundation::IAsyncOperation<T>^ op)
		{
			return await_task( concurrency::task<T> (op) );
		}

		// Allow a await_task on a WinRT IAsyncAction
		__declspec(noinline) void await_task(Windows::Foundation::IAsyncAction^ op)
		{
			return await_task(concurrency::task<void>(op));
		}


		// Allow a await_task on a WinRT IAsyncActionWithProgress
		template <typename _ProgressType>
		__declspec(noinline) void await_task(Windows::Foundation::IAsyncActionWithProgress<_ProgressType>^ op)
		{
			return await_task(concurrency::task<void>(op));
		}

		// Allow a await_task on a WinRT IAsyncOperationWithProgress
		template <typename _ReturnType, typename _ProgressType>
		__declspec(noinline) _ReturnType await_task(Windows::Foundation::IAsyncOperationWithProgress<_ReturnType, _ProgressType>^ op)
		{
			return await_task(concurrency::task<_ReturnType>(op));
		}

		#endif

		// Create a resumable context to run the function in. This is called by the compiler when we have
		// __resumable on a function.
		template <typename TRet, typename TFunc>
		TRet create_resumable_context(TFunc func)
		{
			typedef typename TRet::result_type TTaskType;

			return __resumable_func_setup_data<TRet, TFunc, TTaskType>::RunActionInResumableContext(func);
		}

		// Special-case for create_resumable_context to handle 'void' returns (not task<void> - which is handled above)
		template <typename TFunc>
		void create_resumable_context_void(TFunc func)
		{
			__resumable_func_setup_data<task<void>, TFunc, void>::RunActionInResumableContext(func);
		}


		template<typename _TFunc>
		LPVOID __stdcall make_fiber_indirect_call(LPVOID pv)
		{
			_TFunc& func = *reinterpret_cast<_TFunc*>(pv);

			try
			{
				func();
			}
			catch (...)
			{
				std::exception_ptr* eptr = new std::exception_ptr(std::current_exception());
				return eptr;
			}

			return nullptr;
		}

#pragma warning(push)

#pragma warning(disable: 4483)
#pragma warning(disable: 4715)
#pragma warning(disable: 4716)

		template <typename _Functor, typename ..._TArgs>
		auto setup_fiber_indirect_call_udt(_Functor _Func, _TArgs&&... _Args) ->  decltype(_Func(std::forward<_TArgs>(_Args)...))
		{
			__resumable_func_fiber_data* data = __resumable_func_fiber_data::GetCurrentResumableFuncData();

			auto lambda = [_Args..., _Func, __identifier("__$ReturnUdt")]() { *__identifier("__$ReturnUdt") = _Func(_Args...); };

			std::exception_ptr* eptr = reinterpret_cast<std::exception_ptr*>(CalloutOnFiberStack(data->threadFiber, make_fiber_indirect_call<decltype(lambda)>, &lambda));
			if (eptr)
			{
				std::exception_ptr e = *eptr;
				delete eptr;
				std::rethrow_exception(e);
			}
		}

		template <typename _Functor, typename ..._TArgs>
		auto setup_fiber_indirect_call(_Functor _Func, _TArgs&&... _Args) ->  decltype(_Func(std::forward<_TArgs>(_Args)...))
		{
			__resumable_func_fiber_data* data = __resumable_func_fiber_data::GetCurrentResumableFuncData();

			decltype(_Func(std::forward<_TArgs>(_Args)...)) tmp;
			auto lambda = [_Args..., _Func, &tmp]() { tmp = _Func(_Args...); };

			std::exception_ptr* eptr = reinterpret_cast<std::exception_ptr*>(CalloutOnFiberStack(data->threadFiber, make_fiber_indirect_call<decltype(lambda)>, &lambda));
			if (eptr)
			{
				std::exception_ptr e = *eptr;
				delete eptr;
				std::rethrow_exception(e);
			}
			return tmp;
		}

		template <typename _Functor, typename ..._TArgs>
		void setup_fiber_indirect_call_void(_Functor _Func, _TArgs&&... _Args)
		{
			__resumable_func_fiber_data* data = __resumable_func_fiber_data::GetCurrentResumableFuncData();

			auto lambda = [_Args..., _Func]() { _Func(_Args...); };

			std::exception_ptr* eptr = reinterpret_cast<std::exception_ptr*>(CalloutOnFiberStack(data->threadFiber, make_fiber_indirect_call<decltype(lambda)>, &lambda));
			if (eptr)
			{
				std::exception_ptr e = *eptr;
				delete eptr;
				std::rethrow_exception(e);
			}
		}

		template <typename _TObject, typename _TFunc, typename ..._TArgs>
		auto setup_fiber_indirect_call_ptr_mbr(_TObject* _Obj, _TFunc _Func, _TArgs&&... _Args) ->  decltype((_Obj->*_Func)(std::forward<_TArgs>(_Args)...))
		{
			__resumable_func_fiber_data* data = __resumable_func_fiber_data::GetCurrentResumableFuncData();

			decltype((_Obj->*_Func)(std::forward<_TArgs>(_Args)...)) tmp;
			auto lambda = [_Args..., _Func, &tmp, &_Obj]() { tmp = (_Obj->*_Func)(_Args...); };

			std::exception_ptr* eptr = reinterpret_cast<std::exception_ptr*>(CalloutOnFiberStack(data->threadFiber, make_fiber_indirect_call<decltype(lambda)>, &lambda));
			if (eptr)
			{
				std::exception_ptr e = *eptr;
				delete eptr;
				std::rethrow_exception(e);
			}
			return tmp;
		}

		template <typename _TObject, typename _TFunc, typename ..._TArgs>
		auto setup_fiber_indirect_call_ptr_mbr_udt(_TObject* _Obj, _TFunc _Func, _TArgs&&... _Args) ->  decltype((_Obj->*_Func)(std::forward<_TArgs>(_Args)...))
		{
			__resumable_func_fiber_data* data = __resumable_func_fiber_data::GetCurrentResumableFuncData();

			auto lambda = [_Args..., _Func, __identifier("__$ReturnUdt"), &_Obj]() { *__identifier("__$ReturnUdt") = (_Obj->*_Func)(_Args...); };

			std::exception_ptr* eptr = reinterpret_cast<std::exception_ptr*>(CalloutOnFiberStack(data->threadFiber, make_fiber_indirect_call<decltype(lambda)>, &lambda));
			if (eptr)
			{
				std::exception_ptr e = *eptr;
				delete eptr;
				std::rethrow_exception(e);
			}
		}

		template <typename _TObject, typename _TFunc, typename ..._TArgs>
		void setup_fiber_indirect_call_ptr_mbr_void(_TObject* _Obj, _TFunc _Func, _TArgs&&... _Args)
		{
			__resumable_func_fiber_data* data = __resumable_func_fiber_data::GetCurrentResumableFuncData();

			auto lambda = [_Args..., _Func, &_Obj]() { (_Obj->*_Func)(_Args...); };

			std::exception_ptr* eptr = reinterpret_cast<std::exception_ptr*>(CalloutOnFiberStack(data->threadFiber, make_fiber_indirect_call<decltype(lambda)>, &lambda));
			if (eptr)
			{
				std::exception_ptr e = *eptr;
				delete eptr;
				std::rethrow_exception(e);
			}
		}

		template <typename _TObject>
		void setup_fiber_indirect_call_dtor(_TObject* _Obj)
		{
			__resumable_func_fiber_data* data = __resumable_func_fiber_data::GetCurrentResumableFuncData();

			auto lambda = [&_Obj]() { _Obj->~_TObject(); };

			std::exception_ptr* eptr = reinterpret_cast<std::exception_ptr*>(CalloutOnFiberStack(data->threadFiber, make_fiber_indirect_call<decltype(lambda)>, &lambda));
			if (eptr)
			{
				std::exception_ptr e = *eptr;
				delete eptr;
				std::rethrow_exception(e);
			}
		}
#pragma warning (pop)

	} // namespace Details
} // namespace Concurrency

namespace concurrency = Concurrency;

#endif  /* _PPLTASKS_H */
