// Host-side unit test (no ESP32 needed):
//   g++ -std=c++17 -Wall -Wextra -I src test/native/ring_buffer_test.cpp -o /tmp/rb && /tmp/rb
#include <cassert>
#include <cstdio>

#include "ring_buffer.h"

int main() {
  RingBuffer<int, 3> rb;
  assert(rb.empty() && rb.size() == 0 && rb.capacity() == 3);

  rb.push(1);
  rb.push(2);
  assert(rb.size() == 2 && rb.front() == 1);

  rb.pop();
  assert(rb.size() == 1 && rb.front() == 2);

  // Fill and overflow: the oldest element is dropped, order is preserved.
  rb.push(3);
  rb.push(4);
  assert(rb.size() == 3 && rb.dropped() == 0);
  rb.push(5);  // drops 2
  assert(rb.size() == 3 && rb.dropped() == 1);
  assert(rb.front() == 3);
  rb.pop();
  assert(rb.front() == 4);
  rb.pop();
  assert(rb.front() == 5);
  rb.pop();
  assert(rb.empty());

  rb.pop();  // popping an empty buffer is a no-op
  assert(rb.empty());

  rb.push(9);
  rb.clear();
  assert(rb.empty());

  std::puts("ring_buffer_test: OK");
  return 0;
}
