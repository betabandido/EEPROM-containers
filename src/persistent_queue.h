#ifndef PERSISTENT_QUEUE
#define PERSISTENT_QUEUE

#include <EEPROM.h>

/** This class implements a fixed-size circular queue.
 *
 * The queue is stored on the EEPROM of an ESP8266 micro-controller.
 */
template<class T>
class persistent_queue {
public:
  typedef T value_type;
  typedef std::size_t size_type;
  typedef value_type& reference;
  typedef const value_type& const_reference;
 
  /** Constructor.
   *
   * @param offset Offset from the EEPROM base address.
   * @param capacity Queue's capacity (in elements).
   */ 
  persistent_queue(int offset, size_type capacity)
    : capacity_{capacity}
  {
    uint8_t* data = EEPROM.getDataPtr() + offset;
    storage_ = reinterpret_cast<storage_area*>(data);
            
    if (storage_->signature != SIGNATURE) {
      storage_->signature = SIGNATURE;
      storage_->begin = 0;
      storage_->end = 0;
      storage_->size = 0;
    }
  }

  /**
   * Computes the necessary storage size to hold a queue of the given capacity.
   *
   * @param capacity Queue's capacity (in elements).
   */
  static constexpr size_type storage_size(size_type capacity) {
    return sizeof(storage_area::signature)
        + sizeof(storage_area::begin)
        + sizeof(storage_area::end)
        + sizeof(storage_area::size)
        + capacity * sizeof(value_type);
  }

  /** Checks whether the queue is empty.
   *
   * @return True if the queue is empty; false otherwise.
   */
  bool empty() const {
    return size() == 0;
  }

  /** Checks whether the queue is full.
   *
   * @return True if the queue is full; false otherwise.
   */
  bool full() const {
    return size() == capacity();
  }

  /** Returns the queue's size.
   *
   * The size correspond to the number of elements currently in the queue.
   *
   * @return The queue's size.
   */
  size_type size() const {
    return storage_->size;
  }

  /** Returns the queue's capacity.
   *
   * The capacity correspond to the maximum number of elements that the queue
   * can store.
   *
   * @return The queue's capacity.
   */
  size_type capacity() const {
    return capacity_;
  }

  /** Returns the element at the queue's front.
   *
   * @return The element at the front.
   */
  reference front() {
    return storage_->data()[storage_->begin];
  }

  /** Returns the element at the queue's front.
   *
   * @return The element at the front.
   */
  const_reference front() const {
    return storage_->data()[storage_->begin];
  }

  /** Pushes an element into the queue.
   *
   * The element is pushed at the end of the queue.
   *
   * @param value The element to be pushed.
   * @return True if the element was insterted; false otherwise. 
   */
  bool push(const value_type& value) {
    if (full())
      return false;
    
    storage_->data()[storage_->end] = value;
    increment(storage_->end);
    ++(storage_->size);
    return true;
  }

  /** Pushes an element into the queue.
   *
   * The element is pushed at the end of the queue.
   *
   * @param value The element to be pushed.
   * @return True if the element was insterted; false otherwise. 
   */
  bool push(value_type&& value) {
    if (full())
      return false;

    storage_->data()[storage_->end] = std::move(value);
    increment(storage_->end);
    ++(storage_->size);
    return true;
  }

  /** Pops an element from the queue.
   *
   * The element at the front is popped (removed).
   *
   * @return True if there was an element to pop; false otherwise.
   */
  bool pop() {
    if (empty())
      return false;

    increment(storage_->begin);
    --(storage_->size);
    return true;
  }

private:
  struct storage_area {
    unsigned signature;
    unsigned begin;
    unsigned end;
    size_type size;
    
    value_type* data() {
      uint8_t* ptr_to_data =
          reinterpret_cast<uint8_t*>(&signature)
          + sizeof(signature)
          + sizeof(begin)
          + sizeof(end)
          + sizeof(size);
      return reinterpret_cast<value_type*>(ptr_to_data);
    }
  };

  static constexpr unsigned SIGNATURE { 0xa2bedef9 };
  
  const size_type capacity_;
  storage_area* storage_;

  void increment(unsigned& idx) const {
    if (++idx == capacity_)
      idx = 0;
  }

  persistent_queue() = delete;
  
  persistent_queue(const persistent_queue& other) = delete;
  persistent_queue& operator=(const persistent_queue& other) = delete;
  
  persistent_queue(persistent_queue&& other) = delete;
  persistent_queue& operator=(persistent_queue&& other) = delete;
};

#endif // PERSISTENT_QUEUE

