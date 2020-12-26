#ifndef __USER_MAIN__
#define __USER_MAIN__ 1

class UserMain {
public:
  UserMain() {};
  ~UserMain(){};
  void init();
  void run(float delta_time);
  void clear();
};


#endif // __USER_MAIN__
