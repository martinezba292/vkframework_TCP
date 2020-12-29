#include "input_manager.h"
#include "GLFW/glfw3.h"
#include "internal.h"


uint8 InputManager::inputState[kKeyCode_MAX];
float InputManager::cursorPosition[2];

InputManager::InputManager()
{
  for (size_t i = 0; i < kKeyCode_MAX; i++) {
    inputState[i] = 0;
  }
}

void InputManager::keyCallback(Window* window, int32 key, int32 scancode, int32 action, int32 mods)
{
  uint8 state = 0;
  if (action != GLFW_RELEASE)
    state = 1;

  switch (key) {
  case GLFW_KEY_W: {
    inputState[kKeyCode_W] = state;
    break;
  }
  case GLFW_KEY_A: {
    inputState[kKeyCode_A] = state;
    break;
  }
  case GLFW_KEY_S: {
    inputState[kKeyCode_S] = state;
    break;
  }
  case GLFW_KEY_D: {
    inputState[kKeyCode_D] = state;
    break;
  }

  default: {
    break;
  }
  }
}

void InputManager::mouseCallback(Window* window, double xpos, double ypos)
{
  cursorPosition[0] = xpos;
  cursorPosition[1] = ypos;
}

bool InputManager::getInputState(const KeyCode code)
{
  return inputState[code];
}

float* InputManager::getCursorPosition()
{
  return cursorPosition;
}

