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

////////////////////////////////////////////////////
///////////////// Parameters /////////////////
////////////////////////////////////////////////////
int saveMode = 0; // 0: no save; 1: save screen shot
char saveScreenShotName1 = '0', saveScreenShotName2 = '0', saveScreenShotName3 = '0';

// Global
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
const char textureGroundFilename[] = "heightmap/cubemap3.jpg";

// Sky
GLuint posSkyBuffer; 
GLuint uvSkyBuffer;
vector<GLfloat> posSky;
vector<GLfloat> uvSky;
GLuint textureSkyID;


// Spline
Spline *splines; // the spline array 
int numSplines; // total number of splines 
float intervalSpline = 0.0005f; // updateCamera = true , this 0.0001f; 0.02f (67)
double boundSpline = 0.8; // bound spline coord to [-bound, bound]
float matrixBasic[] = {-0.5, 1.5, -1.5, 0.5, 1.0, -2.5, 2.0, -0.5, -0.5, 0.0, 0.5, 0.0, 0.0, 1.0, 0.0, 0.0 };
float matrixControl[12] = {};

vector<GLfloat> posSpline;
vector<GLfloat> uvSpline;
vector<GLfloat> TSpline;
vector<GLfloat> BSpline;
vector<GLfloat> NSpline;
GLuint posSplineBuffer; 
GLuint uvSplineBuffer;
GLuint textureSplineID;
const char textureSplineFilename[] = "heightmap/iron3.jpg";

float tempV[3] = {0, 1, 0};
float maxSplineZ = -2.0;

// Rail
vector<GLfloat> posRailLeft;
vector<GLfloat> posRailRight;
vector<GLfloat> uvRail;
GLuint posRailLeftBuffer;
GLuint posRailRightBuffer;
GLuint uvRailBuffer;

float scaleRailT = 0.0f;
float scaleRailN = 0.005f;
float scaleRailB = 0.005f;
float centerRail = 0.03f;

// Rail cross
vector<GLfloat> posCross;
vector<GLfloat> uvCross;
GLuint posCrossBuffer;
GLuint uvCrossBuffer;
GLuint textureCrossID;
const char textureCrossFilename[] = "heightmap/wood3.jpg";

float scaleCrossT = 0.005f;
float scaleCrossN = 0.005f;
float scaleCrossB = 0.05f;
int distanceCross = 30;
int lengthCross = 6;
float centerCrossN = 0.01f;

// Camera
bool updateCameraMode = 1; // 1 slow
float speedCamera = 1;
float scaleCameraT = 0.1f;
float scaleCameraN = 0.06f;
float minSpeed = 0.1;

float countPoint = 0; // 3 * interger
int waitingCamera = 0;
int waitingCameraMax = 100;

///////////////////////////////////////////////
///////////////// DECLARATION /////////////////
///////////////////////////////////////////////
/* my helper functions */
/* normalizae spline coord to the [-boundSpline, boundSpline] box */
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


