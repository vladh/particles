#include <GLFW/glfw3.h>
#include <stdio.h>

bool _keyStates[1024] = {false};
bool _prevKeyStates[1024] = {true};

// The last time the key states were updated
float _tLastUpdate = 0;

// The last time the key states were requested. This only
// considers change events such as `didKeyComeUp`.
float _tLastStateChange = 0;

const float MOUSE_SENS = 0.05f;
float _mouseX, _mouseY;
float _yaw = -135.0f, _pitch = -60.0f;
bool _mouseSeen = false;

void controlsUpdateMouselook(double x, double y, bool programmatic, float* pitch, float* yaw) {
  if (!_mouseSeen && !programmatic) {
    _mouseX = x;
    _mouseY = y;
    _mouseSeen = true;
  }

  float xOffset = x - _mouseX;
  float yOffset = y - _mouseY;
  _mouseX = x;
  _mouseY = y;

  xOffset *= MOUSE_SENS;
  yOffset *= MOUSE_SENS;

  _yaw += xOffset;
  _pitch -= yOffset;

  if (_pitch > 89.0f) { _pitch =  89.0f; }
  if (_pitch < -89.0f) { _pitch = -89.0f; }

  *pitch = _pitch;
  *yaw = _yaw;
}

void controlsUpdateKeys(GLFWwindow* window, int key, int scancode, int action, int mods) {
  _tLastUpdate = glfwGetTime();
  _prevKeyStates[key] = _keyStates[key];
  if(action == GLFW_PRESS) {
    _keyStates[key] = true;
  } else if(action == GLFW_RELEASE) {
    _keyStates[key] = false;
  }
}

bool controlsIsKeyDown(int key) {
  return _keyStates[key];
}

bool controlsIsKeyUp(int key) {
  return !_keyStates[key];
}

bool controlsDidKeyComeDown(int key) {
  bool changed = _keyStates[key] && !_prevKeyStates[key];
  if (_tLastStateChange > _tLastUpdate) { return false; }
  if (changed) { _tLastStateChange = glfwGetTime(); }
  return changed;
}

bool controlsDidKeyComeUp(int key) {
  bool changed = !_keyStates[key] && _prevKeyStates[key];
  if (_tLastStateChange > _tLastUpdate) { return false; }
  if (changed) { _tLastStateChange = glfwGetTime(); }
  return changed;
}
