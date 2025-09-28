#pragma once
#include <boost/lockfree/spsc_queue.hpp>

template <class T, size_t N> struct Channel {
  boost::lockfree::spsc_queue<T, boost::lockfree::capacity<N>> q;
  bool push(const T &v) { return q.push(v); }
  bool pop(T &out) { return q.pop(out); }
};
