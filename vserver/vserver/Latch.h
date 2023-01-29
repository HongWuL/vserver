#ifndef __VS_LATCH_H
#define __VS_LATCH_H

#include "Noncopyable.h"
#include "Lock.h"

namespace vs {

class CountDownLatch : Noncopyable {

public:
    explicit CountDownLatch(int count);

    void wait();

    void countDown();

    int getCount() const;

private:
    mutable Mutex m_mutex;
    Condition m_cond;
    int m_count;
};

}

#endif