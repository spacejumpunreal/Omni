#pragma once
#include "Runtime/Prelude/Omni.h"

namespace Omni
{
	template<typename TReturn>
	struct Action0
	{
	public:
		using TFunc = TReturn(*)();
	public:
		Action0(TFunc func)
			: mFunc(func)
		{}
		TReturn operator()() const
		{
			return mFunc();
		}
	private:
		TFunc mFunc;
	};

	template<typename TReturn, typename TArg0>
	struct Action1
	{
	public:
		using TFunc = TReturn(*)(TArg0 arg0);

		Action1(TFunc func, TArg0 arg0)
			: mFunc(func)
			, mArg0(arg0)
		{}
		TReturn operator()() const
		{
			return mPtr(mArg0);
		}
	private:
		TFunc mFunc;
		TArg0 mArg0;
	};

	class ICallback
	{
	public:
		virtual void operator()() = 0;
		virtual ~ICallback() {};
		static void Run(ICallback* callback) { (*callback)(); }
	};

}