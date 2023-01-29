#ifndef __VS_SINGLETON_H__
#define __VS_SINGLETON_H__

namespace vs {

template<class T>
class Singleton {
public:
    static T* getInstance() {
        static T v;
        return &v;
    }
};

template<class T>
class SingletonPtr {
public:
    static std::shared_ptr<T> getInstance() {
        static std::shared_ptr<T> v(new T);
        return v;
    }
    // template<typename ...Args>
    // static std::shared_ptr<T> getInstance(Args... args) {
    //     static std::shared_ptr<T> v(new T(args));
    //     return v;
    // }
};


}

#endif