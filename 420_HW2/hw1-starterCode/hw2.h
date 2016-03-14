/*
Subject   : CSCI420 - Computer Graphics 
Assignment 2: Simulating a Roller Coaster
Author    : Meiyi Yang
USC ID    : 6761040585
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
GLenum drawArrayMode = GL_LINE_STRIP;

// OUTSIDE the box
//float matLookat[9] = {0.0f, -3.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f}; 
//float matPerspective[4] = {80.0f, windowWidth / windowHeight, 0.1f, 10.0f}; 

// INSIDE the box
float matLookat[9] = {0.285714f, -0.685714f, 0.0f, -0.4f, 0.8f, 0.0f, 0.0f, 0.0f, 1.0f}; 
float matPerspective[4] = {80.0f, windowWidth / windowHeight, 0.00001f, 10.0f}; 


////////////////////////////////////////////////////
///////////////// Parameters /////////////////
////////////////////////////////////////////////////
// Save Screen Shot
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
GLuint textureSkyID; //also textureGroundFilename

// Spline
Spline *splines; // the spline array 
int numSplines; // total number of splines 
vector<GLfloat> posSpline;
vector<GLfloat> uvSpline;
vector<GLfloat> TSpline;
vector<GLfloat> BSpline;
vector<GLfloat> NSpline;
GLuint posSplineBuffer; 
GLuint uvSplineBuffer;
GLuint textureSplineID;
const char textureSplineFilename[] = "heightmap/iron4.jpg";
float matrixBasic[] = {-0.5, 1.5, -1.5, 0.5, 1.0, -2.5, 2.0, -0.5, -0.5, 0.0, 0.5, 0.0, 0.0, 1.0, 0.0, 0.0 };
float matrixControl[12] = {};
float maxSplineZ = -2.0;
float intervalSpline = 0.0005f; // 0.0001f / 0.02f (test)
double boundSpline = 0.5; // bounded spline coord to [-bound, bound]
float tempV[3] = {0, 1, 0}; // N0 = T0 * V

// Rail
vector<GLfloat> posRailLeftDown;
vector<GLfloat> posRailRightDown;
vector<GLfloat> uvRail;
GLuint posRailLeftDownBuffer;
GLuint posRailRightDownBuffer;
GLuint uvRailBuffer;
float scaleRailT = 0.0f; 
float scaleRailN = 0.005f;
float scaleRailB = 0.010f;
float centerRailB = 0.03f;

// Rail T-shaped
vector<GLfloat> posRailLeftUp;
vector<GLfloat> posRailRightUp;
GLuint posRailLeftUpBuffer;
GLuint posRailRightUpBuffer;
float centerRailUpN = 0.0125f;
float scaleRailUpB = 0.0025f;
float scaleRailUpN = 0.0075f;

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
int distanceCross = 12;
int lengthCross = 4;
float centerCrossN = 0.01f;

// Camera
bool UpdateCameraMode = 1; // 1: update camera 0: no update
float speedCamera = 2;
float scaleCameraT = 0.1f;
float scaleCameraN = 0.06f;
float minSpeed = 0.1; // make sure maxHeight position also can move
float countPoint = 0; // integer 
int waitingCamera = 0;
int waitingCameraMax = 60; // waiting frames

// Cube
GLuint posCubeBuffer; 
GLuint uvCubeBuffer;
vector<GLfloat> posCube;
vector<GLfloat> uvCube;
GLuint textureCubeID;
const char textureCubeFilename[] = "heightmap/cubemap_fighton.jpg";
float scaleCube = 0.05f;
float centerCubeX = 0.1f;
float centerCubeY = -0.35f;
float centerCubeZ = 0.71f;

///////////////////////////////////////////////
///////////////// DECLARATION /////////////////
///////////////////////////////////////////////
/* my helper functions */
// Initialization 
void normalizeSpline();
vector<float> getMatrixUBC(float u);
vector<float> getMatrixTanUBC(float u);
void subdivideMatrixU(float u0, float u1);
void initialSpline();
float getRailPos(float center, float scaleT, float scaleN, float scaleB, float T, float N, float B);
void initialRail();
void initialRailUp(); 
void initialCross();
void initialEnvironment();
void initialCube(); 
void initialCamera();
void printDetail();

// Display
void bindTexture(GLint, GLuint);
GLuint bindBufferPos(GLuint);
GLuint bindBufferUV(GLuint);
void drawGround();
void drawSky();
void drawCube();
void drawSpline();
void drawRail();
void UpdateCamera();

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


/////////////////////////////////////////////
///////////////// CUBE DATA /////////////////
/////////////////////////////////////////////
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


