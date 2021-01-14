#ifndef __REFERENCED__
#define __REFERENCED__ 1

#include <cstdlib>
#include "common_def.h"

class Referenced {
public:
  void ref() { ++ref_; };
  void unref() { 
    if (ref_ <= 0) abort();
    --ref_;
    if (ref_ <= 0) 
      delete(this);
  }

protected:
  Referenced() { ref_ = 0; }
  virtual ~Referenced(){}

private:
  Referenced(const Referenced& other) { ref_ = other.ref_; }
  int32 ref_;
};

#endif // !__REFERENCED__
