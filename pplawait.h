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
#include <experimental/resumable>
#include <type_traits>

#define __resumable

namespace std {
	namespace experimental {

		template <class _Ty, class... _Whatever>
		struct resumable_traits<Concurrency::task<_Ty>, _Whatever...>
		{
			struct promise_type
			{
				Concurrency::task_completion_event<_Ty> _MyPromise;

				Concurrency::task<_Ty> get_return_object()
				{
					return concurrency::create_task(_MyPromise);
				}

				suspend_never initial_suspend()
				{
					return{};
				}
				suspend_never final_suspend()
				{
					return{};
				}

				template <class _Ut = _Ty>
				enable_if_t<is_same<_Ut, void>::value>
				set_result()
				{
					_MyPromise.set();
				}

				template <class _Ut>
				enable_if_t<!is_same<_Ty, void>::value && !is_same<_Ut, void>::value>
				set_result(_Ut&& _Value)
				{
					_MyPromise.set(_STD forward<_Ut>(_Value));
				}

				void set_exception(std::exception_ptr _Exc)
				{
					_MyPromise.set_exception(std::move(_Exc));
				}

				bool cancellation_requested() const
				{
					return false;
				}
			}; // promise_type
		}; // // resumable_traits<Concurrency::task<_Ts>,...>
	}
}

namespace Concurrency {

	template <class _Ty>
	bool await_ready(task<_Ty> & _Task)
	{
		return _Task.is_done();
	}

	template <class _Ty>
	void await_suspend(task<_Ty> & _Task, std::experimental::resumable_handle<> _ResumeCb)
	{
		_Task.then([_ResumeCb](task<_Ty>&)
		{
			_ResumeCb();
		});
	}

	template <class _Ty>
	auto await_resume(task<_Ty> & _Task)
	{
		return _Task.get();
	}
}

#ifdef __cplusplus_winrt
// Defined awaitable for WinRT IAsyncOperation
// TODO: proper headers

namespace Windows {
	namespace Foundation {

		bool await_ready(IAsyncAction^ _Task)
		{
			return _Task->Status >= AsyncStatus::Completed;
		}

		void await_suspend(Windows::Foundation::IAsyncAction^ _Task, std::experimental::resumable_handle<> _ResumeCb)
		{
			_Task->Completed = ref new AsyncActionCompletedHandler(
				[_ResumeCb](IAsyncAction^, AsyncStatus)
				{
					_ResumeCb();
				}
			);
		}

		void await_resume(Windows::Foundation::IAsyncAction^ _Task)
		{
			_Task->GetResults();
		}
	}

	// IAsyncOperation
	namespace Foundation
	{
		template <class _Ty>
		bool await_ready(IAsyncOperation<_Ty>^ _Task)
		{
			return _Task->Status >= AsyncStatus::Completed;
		}

		template <class _Ty>
		void await_suspend(IAsyncOperation<_Ty>^ _Task, std::experimental::resumable_handle<> _ResumeCb)
		{
			op->Completed = ref new AsyncOperationCompletedHandler<_Ty>(
				[_ResumeCb](IAsyncOperation<T>^, AsyncStatus)
				{
					cb();
			    }
			);
		}

		template <class _Ty>
		auto await_resume(IAsyncOperation<_Ty>^ _Task)
		{
			return _Task->GetResults();
		}
	}

	// TODO: with Progress variants
}

#endif // __cplusplus_winrt
#endif // _PPLAWAIT_H
