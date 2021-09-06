#ifndef __INPUT_MANAGER_H__
#define __INPUT_MANAGER_H__ 1

#include "common_def.h"
#include "glm/glm.hpp"

enum KeyCode {
  kKeyCode_W = 0,
  kKeyCode_A = 1,
  kKeyCode_S = 2,
  kKeyCode_D = 3,
  kKeyCode_ESC = 4,
  kKeyCode_MAX
};

class InputManager {
public:
  InputManager();
  InputManager(const InputManager&){}
  ~InputManager(){}

  static void keyCallback(Window* window, int32 key, int32 scancode, int32 action, int32 mods);
  static void mouseCallback(Window* window, double xpos, double ypos);
  static bool getInputState(const KeyCode code);
  static float* getCursorPosition();


private:
  static uint8 inputState[kKeyCode_MAX];
  static glm::vec2 cursorPosition;
};

#endif // __INPUT_MANAGER_H__ 1
