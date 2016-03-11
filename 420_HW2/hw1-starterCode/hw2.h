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

/////////////////////////////////////////////
///////////////// STRUCTURE /////////////////
/////////////////////////////////////////////
struct Point {
  double x;
  double y;
  double z;
};

struct Spline {
  int numControlPoints;
  struct Point *points;
};


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
//float matLookat[9] = {0.0f, 20.0f, -3.0f, 0.0f, 20.0f, 0.0f, 1.0f, 1.0f, 0.0f}; // glMatrix->LookAt();
//float matPerspective[4] = {170.0f, windowWidth / windowHeight, 0.1f, 200.0f}; 
GLenum drawArrayMode = GL_LINE_STRIP;

//float matLookat[9] = {0.0, 0.0, 3.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0}; // glMatrix->LookAt();
//float matPerspective[4] = {90.0f, 1.0f, 0.1f, 100.0f}; 
//GLenum drawArrayMode = GL_TRIANGLES; // glDrawArrays();

// OUTSIDE the box
//float matLookat[9] = {0.0f, -3.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f}; 
//float matPerspective[4] = {80.0f, windowWidth / windowHeight, 0.1f, 10.0f}; 

// INSIDE the box
float matLookat[9] = {0.285714f, -0.685714f, 0.0f, -0.4f, 0.8f, 0.0f, 0.0f, 0.0f, 1.0f}; 
float matPerspective[4] = {80.0f, windowWidth / windowHeight, 0.00001f, 10.0f}; 

/////////////////////////////////////////////
///////////////// Parameter /////////////////
/////////////////////////////////////////////
int saveMode = 0; // 0: no save; 1: save screen shot
char saveScreenShotName1 = '0', saveScreenShotName2 = '0', saveScreenShotName3 = '0';
int countPoint = 3;

BasicPipelineProgram *pipelineProgram;
GLuint programID;
OpenGLMatrix *glMatrix;
GLuint vertexArrayObjects; 
GLuint h_textureSampler;

// Ground
GLuint posGroundBuffer; 
GLuint uvGroundBuffer;
vector<GLfloat> posGround;
vector<GLfloat> uvGround;
GLuint textureGroundID;
const char textureGroundFilename[] = "heightmap/Cubemap.jpg";

// Sky
GLuint posSkyBuffer; 
GLuint uvSkyBuffer;
vector<GLfloat> posSky;
vector<GLfloat> uvSky;
GLuint textureSkyID;
//const char textureSkyFilename[] = "heightmap/forest512.jpg";

// Spline
Spline *splines; // the spline array 
int numSplines; // total number of splines 
float interval = 0.001f;
float matrixBasic[] = {-0.5, 1.5, -1.5, 0.5, 1.0, -2.5, 2.0, -0.5, -0.5, 0.0, 0.5, 0.0, 0.0, 1.0, 0.0, 0.0 };

vector<GLfloat> posSpline;
vector<GLfloat> uvSpline;
GLuint posSplineBuffer; 
GLuint uvSplineBuffer;
GLuint textureSplineID;
const char textureSplineFilename[] = "heightmap/USC256.jpg";
vector<GLfloat> tanSpline;


float B0[3] = {};
float N0[3] = {};

vector<GLfloat> posRailLeft;
vector<GLfloat> posRailRight;
vector<GLfloat> uvRail;
float scaleRail = 0.005f;
float centerRail = 0.05f;
GLuint posRailLeftBuffer;
GLuint posRailRightBuffer;
GLuint uvRailBuffer;

int speedCamera = 60;
float scaleCamera = 0.05;

vector<GLfloat> posRailCross;
vector<GLfloat> uvRailCross;
GLuint posRailCrossBuffer;
GLuint uvRailCrossBuffer;
float scaleNCross = 0.005f;
float scaleBCross = 0.05f;
float scaleTCross = 0.005f;

GLuint textureRailCrossID;
const char textureRailCrossFilename[] = "heightmap/coral.jpg";

///////////////////////////////////////////////
///////////////// DECLARATION /////////////////
///////////////////////////////////////////////
/* my helper functions */
void normalizeSpline();
void initialSpline();
void initialEnvironment();
void bindTexture(GLint, GLuint);
GLuint bindBufferPos(GLuint);
GLuint bindBufferUV(GLuint);
void drawGround();
void drawSky();
void drawSpline();

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


////////////////////////////////////////
///////////////// DATA /////////////////
////////////////////////////////////////
GLfloat cubemapPos[] = {
  -1.0f, 1.0f, -1.0f, // Ground
  1.0f, 1.0f, -1.0f,
  -1.0f, -1.0f, -1.0f,
  1.0f, 1.0f, -1.0f,
  -1.0f, -1.0f, -1.0f,
  1.0f, -1.0f, -1.0f,

  -1.0f, 1.0f, 1.0f, // front
  1.0f, 1.0f, 1.0f,
  -1.0f, 1.0f, -1.0f,
  1.0f, 1.0f, 1.0f,
  -1.0f, 1.0f, -1.0f,
  1.0f, 1.0f, -1.0f,

  1.0f, 1.0f, 1.0f, // right
  1.0f, -1.0f, 1.0f,
  1.0f, 1.0f, -1.0f,
  1.0f, -1.0f, 1.0f,
  1.0f, 1.0f, -1.0f,
  1.0f, -1.0f, -1.0f,

  1.0f, -1.0f, 1.0f, // back
  -1.0f, -1.0f, 1.0f,
  1.0f, -1.0f, -1.0f,
  -1.0f, -1.0f, 1.0f,
  1.0f, -1.0f, -1.0f,
  -1.0f, -1.0f, -1.0f,

  -1.0f, -1.0f, 1.0f, // left
  -1.0f, 1.0f, 1.0f,
  -1.0f, -1.0f, -1.0f,
  -1.0f, 1.0f, 1.0f,
  -1.0f, -1.0f, -1.0f,
  -1.0f, 1.0f, -1.0f,

  -1.0f, -1.0f, 1.0f,  // top
  1.0f, -1.0f, 1.0f,
  -1.0f, 1.0f, 1.0f,
  1.0f, -1.0f, 1.0f,
  -1.0f, 1.0f, 1.0f,
  1.0f, 1.0f, 1.0f
};
GLfloat cubemapUV[] = { 
  0.25f, 0.333333f,
  0.50f, 0.333333f,
  0.25f, 0.0f,
  0.50f, 0.333333f,
  0.25f, 0.0f,
  0.50f, 0.0f,

  0.25f, 0.666667f, 
  0.50f, 0.666667f,
  0.25f, 0.333333f,
  0.50f, 0.666667f,
  0.25f, 0.333333f,
  0.50f, 0.333333f,

  0.50f, 0.666667f, 
  0.75f, 0.666667f,
  0.50f, 0.333333f,
  0.75f, 0.666667f,
  0.50f, 0.333333f,
  0.75f, 0.333333f,

  0.75f, 0.666667f, 
  1.00f, 0.666667f,
  0.75f, 0.333333f, 
  1.00f, 0.666667f,
  0.75f, 0.333333f, 
  1.00f, 0.333333f,

  0.00f, 0.666667f,
  0.25f, 0.666667f,
  0.00f, 0.333333f,
  0.25f, 0.666667f,
  0.00f, 0.333333f,
  0.25f, 0.333333f,

  0.25f, 1.00f,
  0.50f, 1.00f, 
  0.25f, 0.666667f, 
  0.50f, 1.00f, 
  0.25f, 0.666667f, 
  0.50f, 0.666667f
};


