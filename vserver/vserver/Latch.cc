#include "Latch.h"

namespace vs {

CountDownLatch::CountDownLatch(int count) :
    m_count(count) {
}

void CountDownLatch::wait() {
  MutexLockGuard lock(m_mutex);
  while (m_count > 0) {
    m_cond.wait(m_mutex);
  }
}

void CountDownLatch::countDown() {
  MutexLockGuard lock(m_mutex);
  --m_count;
  if (m_count == 0) {
    m_cond.boradcast();
  }
}

int CountDownLatch::getCount() const {
  MutexLockGuard lock(m_mutex);
  return m_count;
}

}
