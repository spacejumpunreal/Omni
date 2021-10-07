#pragma once
#include "Runtime/Prelude/Omni.h"
#include "Runtime/Base/BaseAPI.h"
#include <type_traits>

namespace Omni
{
    template<typename T>
    class HasIsCleanMethod
    {//standard use of SFINAE: https://en.wikipedia.org/wiki/Substitution_failure_is_not_an_error
        template<typename C> static std::true_type Test(decltype(&C::IsClean));
        template<typename C> static std::false_type Test(...);
    public:
        using Type = decltype(Test<T>(nullptr));
        static constexpr bool Value = Type::value;
    };


    class BASE_API IThreadLocal
    {
    public:
        IThreadLocal();
        ~IThreadLocal() = default; //virtual dtor is not needed since thread_locals are not deleted through pointer
        virtual bool IsClean() = 0;
        static void CheckAllThreadLocalClean();
        static IThreadLocal* GetAllThreadLocals();
    protected:
        void CheckIsClean();
    private:
        IThreadLocal*       mNext;
    };


    template<typename T>
    class ThreadLocalWrapper : public IThreadLocal
    {
    public:
        static constexpr bool IsPointer = std::is_pointer_v<T>;
    public:
        ThreadLocalWrapper() : mValue{} {}
        ~ThreadLocalWrapper() { CheckIsClean(); } //can't called this in base dtor, vtable is destroyed then}
        std::conditional_t<IsPointer, T, T*> operator->()
        { 
            if constexpr (IsPointer)
                return mValue;
            else
                return &mValue; 
        }
        T& GetRaw() { return mValue; }
        bool IsClean() override
        {
            if constexpr (HasIsCleanMethod<T>::Value)
                return mValue.IsClean();
            else
                return mValue == T{};
        }
        
    private:
        T       mValue;
    };


#define OMNI_DECLARE_THREAD_LOCAL(Type, Name) static thread_local ThreadLocalWrapper<Type> Name
}


