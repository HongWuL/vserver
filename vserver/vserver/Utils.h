#ifndef __VS_UTILS_H
#define __VS_UTILS_H

#include <unistd.h>
#include <pthread.h>
#include <sys/syscall.h>
#include <string>
#include <algorithm>
#include <cstring>
#include <stdarg.h>
#include <sys/time.h>
#include <unistd.h>
#include "Noncopyable.h"
#include <memory>
#include <functional>

namespace vs
{

typedef uint64_t time_type;

pid_t GetThreadId();    //返回线程在内核中的ID
uint32_t GetFiberId();


time_type GetCurrentTimeMS();

inline time_type TimeAdd(time_type t1, time_type t2) {
    return t1 + t2;
}

inline int64_t TimeSub(time_type t1, time_type t2) {
    return t1 - t2;
}

template<typename To, typename From>
inline To implicit_cast(From const &f) {
    return f;      
}

template<typename T>
class AtomicIntegerT : Noncopyable {
public:
    AtomicIntegerT():m_value(0) { }

    T get() {
        return  __atomic_load_n(&m_value, __ATOMIC_SEQ_CST);
    }

    T getAndAdd(T x) {
        return __atomic_fetch_add(&m_value, x, __ATOMIC_SEQ_CST);
    }

    T addAndGet(T x) {
        return getAndAdd(x) + x;
    }

    T incrementAndGet() {
        return addAndGet(1);
    }

    T decrementAndGet() {
        return addAndGet(-1);
    }

    void add(T x) {
        getAndAdd(x);
    }

    void increment() {
        incrementAndGet();
    }

    void decrement() {
        decrementAndGet();
    }

    T getAndSet(T newValue) {
        return __atomic_exchange_n(&m_value, newValue, __ATOMIC_SEQ_CST);
    }

private:
    volatile T m_value;
};

template<typename CLASS, typename... ARGS>
class WeakCallback {

public:

    WeakCallback(const std::weak_ptr<CLASS>& object,
                const std::function<void (CLASS*, ARGS...)>& function)
    : object_(object), function_(function) {
    }

    void operator()(ARGS&&... args) const {
        std::shared_ptr<CLASS> ptr(object_.lock());
        if (ptr) {
            function_(ptr.get(), std::forward<ARGS>(args)...);
        }
    }

private:

    std::weak_ptr<CLASS> object_;
    std::function<void (CLASS*, ARGS...)> function_;
};

template<typename CLASS, typename... ARGS>
WeakCallback<CLASS, ARGS...> makeWeakCallback(const std::shared_ptr<CLASS>& object,
                                              void (CLASS::*function)(ARGS...)) {
  return WeakCallback<CLASS, ARGS...>(object, function);
}

template<typename CLASS, typename... ARGS>
WeakCallback<CLASS, ARGS...> makeWeakCallback(const std::shared_ptr<CLASS>& object,
                                              void (CLASS::*function)(ARGS...) const) {
  return WeakCallback<CLASS, ARGS...>(object, function);
}

std::string ToUpper(const std::string& name);

std::string ToLower(const std::string& name);

std::string Time2Str(time_t ts = time(0), const std::string& format = "%Y-%m-%d %H:%M:%S");
time_t Str2Time(const char* str, const char* format = "%Y-%m-%d %H:%M:%S");

// class StringUtil {
// public:
//     static std::string Format(const char* fmt, ...);
//     static std::string Formatv(const char* fmt, va_list ap);

//     static std::string UrlEncode(const std::string& str, bool space_as_plus = true);
//     static std::string UrlDecode(const std::string& str, bool space_as_plus = true);

//     static std::string Trim(const std::string& str, const std::string& delimit = " \t\r\n");
//     static std::string TrimLeft(const std::string& str, const std::string& delimit = " \t\r\n");
//     static std::string TrimRight(const std::string& str, const std::string& delimit = " \t\r\n");


//     static std::string WStringToString(const std::wstring& ws);
//     static std::wstring StringToWString(const std::string& s);

// };

} // namespace vs


#endif
