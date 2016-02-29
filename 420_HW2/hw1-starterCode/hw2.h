/*
  CSCI 420 Computer Graphics, USC
  Assignment 1: Height Fields
  C++ starter code
  
  Student username: meiyiyan@usc.edu
  Name: Meiyi Yang
*/

#include <iostream>
#include <vector>
#include <cstring>
#include "openGLHeader.h"
#include "glutHeader.h"

#include "imageIO.h"
#include "openGLMatrix.h"
#include "basicPipelineProgram.h"

#ifdef WIN32
  #ifdef _DEBUG
    #pragma comment(lib, "glew32d.lib")
  #else
    #pragma comment(lib, "glew32.lib")
  #endif
#endif

#ifdef WIN32
  char shaderBasePath[1024] = SHADER_BASE_PATH;
#else
  char shaderBasePath[1024] = "../openGLHelper-starterCode";
#endif

using namespace std;


//////////////////////////////////////////////////
///////////////// STATE OF WORLD /////////////////
//////////////////////////////////////////////////
int mousePos[2]; // x,y coordinate of the mouse position
int leftMouseButton = 0; // 1 if pressed, 0 if not 
int middleMouseButton = 0; // 1 if pressed, 0 if not
int rightMouseButton = 0; // 1 if pressed, 0 if not

typedef enum { ROTATE, TRANSLATE, SCALE } CONTROL_STATE;
CONTROL_STATE controlState = ROTATE;

float landRotate[3] = { 0.0f, 0.0f, 0.0f };
float landTranslate[3] = { 0.0f, 0.0f, 0.0f };
float landScale[3] = { 1.0f, 1.0f, 1.0f };

int windowWidth = 1280;
int windowHeight = 720;
char windowTitle[512] = "CSCI 420 homework I";

float colorClear[4] = {0.0f, 0.0f, 0.4f, 0.0f}; // glClearColor();
float matLookat[9] = {0.0f, 20.0f, -3.0f, 0.0f, 20.0f, 0.0f, 1.0f, 1.0f, 0.0f}; // glMatrix->LookAt();
float matPerspective[4] = {170.0f, windowWidth / windowHeight, 0.1f, 200.0f}; 
GLenum drawArrayMode = GL_LINE_STRIP; // glDrawArrays();

//////////////////////////////////////////////
///////////////// HOMEWORK 1 /////////////////
//////////////////////////////////////////////
BasicPipelineProgram *pipelineProgram;
GLuint programID;
OpenGLMatrix *glMatrix;
GLuint vertexArrayObjects; /* Vertex Array Objects */
GLuint vertexBufferObject; /* Vertex Buffer Objects */
vector<GLfloat> pos;
vector<GLfloat> col;
int saveMode = 0; // 0: no save; 1: save screen shot
char saveScreenShotName1 = '0', saveScreenShotName2 = '0', saveScreenShotName3 = '0';

//////////////////////////////////////////////
///////////////// HOMEWORK 2 /////////////////
//////////////////////////////////////////////
struct Point {
	double x;
	double y;
	double z;
};

struct Spline {
	int numControlPoints;
	struct Point *points;
};

Spline *splines; // the spline array 
int numSplines; // total number of splines 

float interval = 0.001f;
float matrixBasic[] = {-0.5, 1.5, -1.5, 0.5, 1.0, -2.5, 2.0, -0.5, -0.5, 0.0, 0.5, 0.0, 0.0, 1.0, 0.0, 0.0 };

ImageIO 

///////////////////////////////////////////////
///////////////// DECLARATION /////////////////
///////////////////////////////////////////////
/* my helper functions */
void printDetail();
void myRenderPosition();
void myRenderColor();

/* HW1 & HW2 need to implement */
void initScene(int argc, char *argv[]);
void idleFunc();
void displayFunc();
void reshapeFunc(int w, int h);

/* HW1 starter code */
void saveScreenshot(const char * filename);
void mouseMotionDragFunc(int x, int y);
void mouseMotionFunc(int x, int y);
void mouseButtonFunc(int button, int state, int x, int y);
void keyboardFunc(unsigned char key, int x, int y);

/* HW2 starter code */
int loadSplines(char * argv);
int initTexture(const char * imageFilename, GLuint textureHandle);






