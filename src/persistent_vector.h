#ifndef PERSISTENT_VECTOR
#define PERSISTENT_VECTOR

#include <EEPROM.h>

/** This class implements a fixed-size vector.
 *
 * The vector is stored on the EEPROM of an ESP8266 micro-controller.
 */
template<class T>
class persistent_vector {
public:
  typedef T value_type;
  typedef std::size_t size_type;
  typedef value_type& reference;
  typedef const value_type& const_reference;
  
  /** Constructor.
   *
   * @param offset Offset from the EEPROM base address.
   * @param capacity Vector's capacity (in elements).
   */ 
  persistent_vector(int offset, size_type capacity)
    : capacity_{capacity}
  {
    uint8_t* data = EEPROM.getDataPtr() + offset;
    storage_ = reinterpret_cast<storage_area*>(data);
            
    if (storage_->signature != SIGNATURE) {
      storage_->signature = SIGNATURE;
      storage_->size = 0;
    }
  }

  /**
   * Computes the necessary storage size to hold a vector of the given capacity.
   *
   * @param capacity Vector's capacity (in elements).
   */
  static constexpr size_type storage_size(size_type capacity) {
    return sizeof(storage_area::signature)
        + sizeof(storage_area::size)
        + capacity * sizeof(value_type);
  }

  /** Checks whether the vector is empty.
   *
   * @return True if the vector is empty; false otherwise.
   */
  bool empty() const {
    return size() == 0;
  }

  /** Checks whether the vector is full.
   *
   * @return True if the vector is full; false otherwise.
   */
  bool full() const {
    return size() == capacity();
  }

  /** Returns the vector's size.
   *
   * The size correspond to the number of elements currently in the vector.
   *
   * @return The vector's size.
   */
  size_type size() const {
    return storage_->size;
  }

  /** Returns the vector's capacity.
   *
   * The capacity correspond to the maximum number of elements that the vector
   * can store.
   *
   * @return The vector's capacity.
   */
  size_type capacity() const {
    return capacity_;
  }

  /** Returns an element given its position in the vector.
   *
   * @param pos The position or index of the element to retrieve.
   * @return A reference to the element.
   */
  reference operator[](size_type pos) {
    return storage_->data()[pos];
  }

  /** Returns an element given its position in the vector.
   *
   * @param pos The position or index of the element to retrieve.
   * @return A constant reference to the element.
   */
  const_reference operator[](size_type pos) const {
    return storage_->data()[pos];
  }

  /** Pushes an element into the vector.
   *
   * The element is pushed at the end of the vector.
   *
   * @param value The element to be pushed.
   * @return True if the element was insterted; false otherwise. 
   */
  bool push_back(const value_type& value) {
    if (full())
      return false;
    
    storage_->data()[storage_->size] = value;
    ++(storage_->size);
    return true;
  }

  /** Pushes an element into the vector.
   *
   * The element is pushed at the end of the vector.
   *
   * @param value The element to be pushed.
   * @return True if the element was insterted; false otherwise. 
   */
  bool push_back(value_type&& value) {
    if (full())
      return false;

    storage_->data()[storage_->size] = std::move(value);
    ++(storage_->size);
    return true;
  }

  /** Pops an element from the vector.
   *
   * The element at the end is popped (removed).
   *
   * @return True if there was an element to pop; false otherwise.
   */
  bool pop_back() {
    if (empty())
      return false;

    --(storage_->size);
    return true;
  }

private:
  struct storage_area {
    unsigned signature;
    size_type size;
    
    value_type* data() {
      uint8_t* ptr_to_data =
          reinterpret_cast<uint8_t*>(&signature)
          + sizeof(signature)
          + sizeof(size);
      return reinterpret_cast<value_type*>(ptr_to_data);
    }
  };

  static constexpr unsigned SIGNATURE { 0xa2bedef9 };
  
  const size_type capacity_;
  storage_area* storage_;

  persistent_vector() = delete;
  
  persistent_vector(const persistent_vector& other) = delete;
  persistent_vector& operator=(const persistent_vector& other) = delete;
  
  persistent_vector(persistent_vector&& other) = delete;
  persistent_vector& operator=(persistent_vector&& other) = delete;
};

#endif // PERSISTENT_VECTOR

