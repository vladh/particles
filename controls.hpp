#ifndef CONTROLS_H
#define CONTROLS_H

/*
Update looking.
@param programmatic {bool} If true, this is not actually a mouse movement.
@param pitch {float*} The camera's new pitch
@param yaw {float*} The camera's new yaw
*/
void controlsUpdateMouselook(double x, double y, bool programmatic, float* pitch, float* yaw);

void controlsUpdateKeys(GLFWwindow* window, int key, int scancode, int action, int mods);

/*
Whether or not the given key is currently down.
*/
bool controlsIsKeyDown(int key);

/*
Whether or not the given key is currently up.
*/
bool controlsIsKeyUp(int key);

/*
Whether or not the given key came down on this frame.
*/
bool controlsDidKeyComeDown(int key);

/*
Whether or not the given key came up on this frame.
*/
bool controlsDidKeyComeUp(int key);

#endif
