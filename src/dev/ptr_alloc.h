#ifndef __PTR_ALLOC__
#define __PTR_ALLOC__ 1

template<class T>
class PtrAlloc {
public:
  PtrAlloc() : ptr_(0L) {}

  PtrAlloc(T* t) {
    ptr_ = t;
    if (ptr_) ptr_->ref();
  }

  PtrAlloc(const PtrAlloc& other) {
    ptr_ = other.ptr_;
    if (ptr_) ptr_->ref();
  }

  ~PtrAlloc() {
    if (ptr_) ptr_->unref();
    ptr_ = nullptr;
  }


  T* alloc() {
    if (ptr_) return ptr_;
    (*this) = new T();
    return ptr_;
  }

  void swap(PtrAlloc& other) {
    T* aux = ptr_;
    ptr_ = other.ptr_;
    other.ptr_ = aux;
  }

  void clear() {
    PtrAlloc empty;
    swap(empty);
  }

  T* get() { return ptr_; }

  PtrAlloc& operator=(PtrAlloc& other) {
    if (ptr_ == other.ptr_) return *this;

    T* aux = ptr_;
    ptr_ = other.ptr_;
    if (ptr_) ptr_->ref();
    if (aux)  aux->unref();

    return *this;
  }

  PtrAlloc& operator=(T& other) {
    if (ptr_) return *this;
    alloc();
    *ptr_ = other;

    return *this;
  }

  PtrAlloc& operator=(T* other) {
    if (ptr_ == other) return *this;

    T* aux = ptr_;
    ptr_ = other;
    if (ptr_) ptr_->ref();
    if (aux)  aux->unref();

    return *this;
  }

  T* operator->() { return ptr_; }
  T& operator*() { return *ptr_; }

private:
  T* ptr_;

};

#endif
