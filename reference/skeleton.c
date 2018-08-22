/*
COMP37111 Lab 1
Vlad-Stefan Harbuz

Makes use of:
  * Skeleton code for COMP37111 coursework, 2013-14 (Arturs Bekasovs and Toby Howard, CC-BY 3.0)
*/

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
#include <stdbool.h>

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

GLuint axisList;
const int AXIS_SIZE = 200;
bool axisEnabled = true;

/*
Return random double within range [0,1]
*/
double myRandom() {
  return (rand() / (double)RAND_MAX);
}

void display() {
  glLoadIdentity();
  gluLookAt(0.0, 100.0, 1000.0,
            0.0, 0.0, 0.0,
            0.0, 1.0, 0.0);
  glClear(GL_COLOR_BUFFER_BIT);
  if (axisEnabled) glCallList(axisList);
  glutSwapBuffers();
}

void keyboard(unsigned char key, int x, int y) {
  if (key == 27) { // ESC
    exit(0);
  }
  glutPostRedisplay();
}

void reshape(int width, int height) {
  glClearColor(0.9, 0.9, 0.9, 1.0);
  glViewport(0, 0, (GLsizei)width, (GLsizei)height);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(60, (GLfloat)width / (GLfloat)height, 1.0, 10000.0);
  glMatrixMode(GL_MODELVIEW);
}

/*
Create a display list for drawing coord axis
*/
void makeAxes() {
  axisList = glGenLists(1);
  glNewList(axisList, GL_COMPILE);
  glLineWidth(2.0);
  glBegin(GL_LINES);

  // x axis
  glColor3f(1.0, 0.0, 0.0);
  glVertex3f(0.0, 0.0, 0.0);
  glVertex3f(AXIS_SIZE, 0.0, 0.0);

  // y axis
  glColor3f(0.0, 1.0, 0.0);
  glVertex3f(0.0, 0.0, 0.0);
  glVertex3f(0.0, AXIS_SIZE, 0.0);

  // z axis
  glColor3f(0.0, 0.0, 1.0);
  glVertex3f(0.0, 0.0, 0.0);
  glVertex3f(0.0, 0.0, AXIS_SIZE);

  glEnd();
  glEndList();
}

void initGraphics(int argc, char *argv[]) {
  glutInit(&argc, argv);
  glutInitWindowSize(800, 600);
  glutInitWindowPosition(100, 100);
  glutInitDisplayMode(GLUT_DOUBLE);
  glutCreateWindow("COMP37111 Particles");
  glutDisplayFunc(display);
  glutKeyboardFunc(keyboard);
  glutReshapeFunc(reshape);
  makeAxes();
}

int main(int argc, char *argv[]) {
  double f;
  srand(time(NULL));
  initGraphics(argc, argv);
  glEnable(GL_POINT_SMOOTH);
  glutMainLoop();
}
