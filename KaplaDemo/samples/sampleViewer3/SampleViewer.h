#ifndef SAMPLE_VIEWER_H
#define SAMPLE_VIEWER_H

// glut callbacks
void RenderCallback();
void ReshapeCallback(int width, int height);
void IdleCallback();
void KeyboardCallback(unsigned char key, int x, int y);
void KeyboardUpCallback(unsigned char key, int x, int y);
void SpecialCallback(int key, int x, int y);
void MouseCallback(int button, int state, int x, int y);
void MotionCallback(int x, int y);
void InitGlut(int argc, char **argv);

float UpdateTime();
void WaitForPhysics();
void RunPhysics();

int main(int argc, char** argv);

#endif  // SAMPLE_VIEWER_H
