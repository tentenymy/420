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
///////////////// Set Cases /////////////////
/////////////////////////////////////////////
/* Case1: Render grayscale image by trianlge, colored in gradient color */
int renderMode = 2; // 0: Points; 1: Lines; 2: Triangle
int colorMode = 0; // 0: graident color; 1: own image color; 2: texture color
int saveMode = 0; // 0: no save; 1: save screen shot
// Set basic.vertexShader.glsl: mode = 0


/* Case2: Render colorful image by trianlge */
/*
int renderMode = 2; // 0: Points; 1: Lines; 2: Triangle
int colorMode = 1; // 0: graident color; 1: own image color; 2: texture color
int saveMode = 0; // 0: no save; 1: save screen shot
// Set basic.vertexShader.glsl: mode = 1
*/

/* Case3: Render grayscale image by trianlge, colored by the same size colorful image (Save screen shot)*/
/*
int renderMode = 2; // 0: Points; 1: Lines; 2: Triangle
int colorMode = 2; // 0: graident color; 1: own image color; 2: texture color
int saveMode = 0; // 0: no save; 1: save screen shot
// Set basic.vertexShader.glsl: mode = 1
*/



int mousePos[2]; // x,y coordinate of the mouse position
int leftMouseButton = 0; // 1 if pressed, 0 if not 
int middleMouseButton = 0; // 1 if pressed, 0 if not
int rightMouseButton = 0; // 1 if pressed, 0 if not

typedef enum { ROTATE, TRANSLATE, SCALE } CONTROL_STATE;
CONTROL_STATE controlState = ROTATE;

// state of the world
float landRotate[3] = { 0.0f, 0.0f, 0.0f };
float landTranslate[3] = { 0.0f, 0.0f, 0.0f };
float landScale[3] = { 1.0f, 1.0f, 1.0f };

int windowWidth = 1280;
int windowHeight = 720;
char windowTitle[512] = "CSCI 420 homework I";


BasicPipelineProgram *pipelineProgram;
OpenGLMatrix *matrix;
GLuint VertexArrayID;
GLuint programID;
GLuint vertexbuffer;
GLuint colorbuffer;

ImageIO * heightmapImage;
ImageIO * colorImage;

GLfloat positions[768*768*6*3];
GLfloat colors[768*768*6*3];
GLsizei numVertices = 0;
GLsizei sizePosition = 0;
GLsizei sizeColor = 0;

char a1 = '0', a2 = '0', a3 = '0';

const int POINTS = 0;
const int LINES = 1;
const int TRIANGLES = 2;

float m[16] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
float p[16] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};

// write a screenshot to the specified filename
void saveScreenshot(const char * filename)
{
  unsigned char * screenshotData = new unsigned char[windowWidth * windowHeight * 3];
  glReadPixels(0, 0, windowWidth, windowHeight, GL_RGB, GL_UNSIGNED_BYTE, screenshotData);

  ImageIO screenshotImg(windowWidth, windowHeight, 3, screenshotData);

  if (screenshotImg.save(filename, ImageIO::FORMAT_JPEG) == ImageIO::OK)
    cout << "File " << filename << " saved successfully." << endl;
  else cout << "Failed to save file " << filename << '.' << endl;

  delete [] screenshotData;
}

void displayFunc()
{
  // 1. Clear the display
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // 2. Set matrix
  matrix->SetMatrixMode(OpenGLMatrix::ModelView); 
  matrix->LoadIdentity();
  matrix->LookAt(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f, 0.0f); // default camera 
  matrix->Rotate(landRotate[0], 1.0, 0.0, 0.0); 
  matrix->Rotate(landRotate[1], 0.0, 1.0, 0.0); 
  matrix->Rotate(landRotate[2], 0.0, 0.0, 1.0);
  matrix->Translate(landTranslate[0], landTranslate[1], landTranslate[2]); 
  matrix->Scale(landScale[0], landScale[1], landScale[2]);

  // 3. Bind VAO and VBO, set data to VBO
  glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
  GLuint loc = glGetAttribLocation(programID, "position"); 
  glEnableVertexAttribArray(loc);
  const void * offset = (const void*) 0;
  glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 0, offset);
  GLuint loc2 = glGetAttribLocation(programID, "color"); 
  glEnableVertexAttribArray(loc2);
  offset = (const void*) sizePosition; 
  glVertexAttribPointer(loc2, 4, GL_FLOAT, GL_FALSE, 0, offset);


  // 4. Set transformation to matrix
  GLint h_modelViewMatrix = glGetUniformLocation(programID, "modelViewMatrix");
  matrix->GetMatrix(m);
  glUniformMatrix4fv(h_modelViewMatrix, 1, GL_FALSE, m);
  GLint h_projectionMatrix = glGetUniformLocation(programID, "projectionMatrix");
  matrix->GetMatrix(p);
  glUniformMatrix4fv(h_projectionMatrix, 1, GL_FALSE, p);

  // 5. Draw the triangle
  GLint first = 0;
  GLsizei count = numVertices; 
  switch (renderMode) {
    case POINTS:
      glDrawArrays(GL_POINTS, first, count);
      break;
    case LINES:
      glDrawArrays(GL_LINES_ADJACENCY, first, count);
      break;
    default:
      glDrawArrays(GL_TRIANGLES, first, count);
  }
  glDisableVertexAttribArray(loc);
  glDisableVertexAttribArray(loc2);
  glutSwapBuffers();
}

void idleFunc()
{
  // save the screen shot
  if (saveMode == 1) {
    char fName[] = {'s', 'a', 'v', 'e', '/', a1, a2, a3, '.', 'j', 'p', 'g', '\0'};
    ++a3;
    if (a3 > '9') {
      ++a2; 
      a3 = '0';
    }
    if (a2 > '9') {
      ++a1; 
      a2 = '0';
      cout << fName << endl;
    }
    if (a1 > '9') 
      a1 = '0'; 
    saveScreenshot(fName);
  }
  
  // make the screen update 
  GLfloat delta = 1.0; 
  GLint axis = 1; 
  landRotate[axis] += delta;
  if (landRotate[axis] > 360.0)
    landRotate[axis] -= 360.0;
  glutPostRedisplay(); 
}

void reshapeFunc(int w, int h)
{
  GLfloat aspect = (GLfloat) w / (GLfloat) h; 
  glViewport(0, -280, 1280, 1280); 
  matrix->SetMatrixMode(OpenGLMatrix::Projection); 
  matrix->LoadIdentity();
  matrix->Perspective(60.0f, w / h, 0.1f, 100.0f);
  matrix->SetMatrixMode(OpenGLMatrix::ModelView); 
  matrix->LoadIdentity();
}

void mouseMotionDragFunc(int x, int y)
{
  // mouse has moved and one of the mouse buttons is pressed (dragging)

  // the change in mouse position since the last invocation of this function
  int mousePosDelta[2] = { x - mousePos[0], y - mousePos[1] };

  switch (controlState)
  {
    // translate the landscape
    case TRANSLATE:
      if (leftMouseButton)
      {
        // control x,y translation via the left mouse button
        landTranslate[0] += mousePosDelta[0] * 0.01f;
        landTranslate[1] -= mousePosDelta[1] * 0.01f;
      }
      if (middleMouseButton)
      {
        // control z translation via the middle mouse button
        landTranslate[2] += mousePosDelta[1] * 0.01f;
      }
      break;

    // rotate the landscape
    case ROTATE:
      if (leftMouseButton)
      {
        // control x,y rotation via the left mouse button
        landRotate[0] += mousePosDelta[1];
        landRotate[1] += mousePosDelta[0];
      }
      if (middleMouseButton)
      {
        // control z rotation via the middle mouse button
        landRotate[2] += mousePosDelta[1];
      }
      break;

    // scale the landscape
    case SCALE:
      if (leftMouseButton)
      {
        // control x,y scaling via the left mouse button
        landScale[0] *= 1.0f + mousePosDelta[0] * 0.01f;
        landScale[1] *= 1.0f - mousePosDelta[1] * 0.01f;
      }
      if (middleMouseButton)
      {
        // control z scaling via the middle mouse button
        landScale[2] *= 1.0f - mousePosDelta[1] * 0.01f;
      }
      break;
  }

  // store the new mouse position
  mousePos[0] = x;
  mousePos[1] = y;
}

void mouseMotionFunc(int x, int y)
{
  // mouse has moved
  // store the new mouse position
  mousePos[0] = x;
  mousePos[1] = y;
}

void mouseButtonFunc(int button, int state, int x, int y)
{
  // a mouse button has has been pressed or depressed

  // keep track of the mouse button state, in leftMouseButton, middleMouseButton, rightMouseButton variables
  switch (button)
  {
    case GLUT_LEFT_BUTTON:
      leftMouseButton = (state == GLUT_DOWN);
    break;

    case GLUT_MIDDLE_BUTTON:
      middleMouseButton = (state == GLUT_DOWN);
    break;

    case GLUT_RIGHT_BUTTON:
      rightMouseButton = (state == GLUT_DOWN);
    break;
  }

  // keep track of whether CTRL and SHIFT keys are pressed
  switch (glutGetModifiers())
  {
    case GLUT_ACTIVE_CTRL:
      controlState = TRANSLATE;
    break;

    case GLUT_ACTIVE_SHIFT:
      controlState = SCALE;
    break;

    // if CTRL and SHIFT are not pressed, we are in rotate mode
    default:
      controlState = ROTATE;
    break;
  }

  // store the new mouse position
  mousePos[0] = x;
  mousePos[1] = y;
}

void keyboardFunc(unsigned char key, int x, int y)
{
  switch (key)
  {
    case 27: // ESC key
      exit(0); // exit the program
    break;

    case ' ':
      cout << "You pressed the spacebar." << endl;
    break;

    case 'x':
      // take a screenshot
      saveScreenshot("screenshot.jpg");
    break;
  }
}

void myRenderPosition() {
  int imageHeight = heightmapImage->getHeight();
  int imageWidth = heightmapImage->getWidth();
  int imageByte = heightmapImage->getBytesPerPixel();

  if (renderMode == TRIANGLES) {
    numVertices = (imageHeight - 1) * (imageWidth - 1) * 6;
    sizePosition = numVertices * 3 * 4;
    int count = 0;
    float scale_XY = 1.0f / imageHeight;
    float scale_Z = 0.2f / 255.0f;
    for (int i = 0; i < imageHeight - 1; ++i) {
      for (int j = 0; j < imageWidth - 1; ++j) {
        positions[count++] = i * scale_XY - 0.5;
        positions[count++] = j * scale_XY - 0.5;
        positions[count++] = heightmapImage->getPixel(i, j, 0) * scale_Z;

        positions[count++] = (i + 1) * scale_XY - 0.5;
        positions[count++] = j * scale_XY - 0.5;
        positions[count++] = heightmapImage->getPixel(i + 1, j, 0) * scale_Z;

        positions[count++] = i * scale_XY - 0.5;
        positions[count++] = (j + 1) * scale_XY - 0.5;
        positions[count++] = heightmapImage->getPixel(i, j + 1, 0) * scale_Z;

        positions[count++] = (i + 1) * scale_XY - 0.5;
        positions[count++] = j * scale_XY - 0.5;
        positions[count++] = heightmapImage->getPixel(i + 1, j, 0) * scale_Z;

        positions[count++] = i * scale_XY - 0.5;
        positions[count++] = (j + 1) * scale_XY - 0.5;
        positions[count++] = heightmapImage->getPixel(i, j + 1, 0) * scale_Z;

        positions[count++] = (i + 1) * scale_XY - 0.5;
        positions[count++] = (j + 1) * scale_XY - 0.5;
        positions[count++] = heightmapImage->getPixel(i + 1, j + 1, 0) * scale_Z;
      }
    }
  } else {
    numVertices = imageHeight * imageWidth;
    sizePosition = numVertices * 3 * 4;
    int count = 0;
    float scale_XY = 1.0 / 255.0f;
    float scale_Z = scale_XY * 0.2;
    for (int i = 0; i < imageHeight; ++i) {
      for (int j = 0; j < imageWidth; ++j) {
        positions[count++] = i * scale_XY - 0.5;
        positions[count++] = j * scale_XY - 0.5;
        positions[count++] = heightmapImage->getPixel(i, j, 0) * scale_Z;
    
      }
    }
  }
}

void myRenderColor() {
  int imageHeight = heightmapImage->getHeight();
  int imageWidth = heightmapImage->getWidth();
  int imageByte = heightmapImage->getBytesPerPixel();

  char *filename;
  colorImage = new ImageIO();
  if (imageByte == 1 && colorMode == 2) {
    if (imageHeight == 128) {
      char filenameColor[] = "heightmap/USC128.jpg";
      filename = filenameColor;
    } else if (imageHeight == 256) {
      char filenameColor[] = "heightmap/USC256.jpg";
      filename = filenameColor;
    } else if (imageHeight == 512) {
      char filenameColor[] = "heightmap/sea512.jpg";
      filename = filenameColor;
    } else if (imageHeight == 768) {
      char filenameColor[] = "heightmap/USC768.jpg";
      filename = filenameColor;
    }
    if (colorImage->loadJPEG(filename) != ImageIO::OK)
    {
      cout << "Error reading image " << filename << "." << endl;
      exit(EXIT_FAILURE);
    } 
  } else {
    colorImage = heightmapImage;
  }
  
  if (colorMode == 0 || (colorMode == 1 && imageByte == 1)) { // gradient or grayscale own image

  } else if (renderMode != 2) { // color image
    sizeColor = numVertices * 4 * 4;
    int count = 0;
    float scale = 1.0f / 255.0f;
    for (int i = 0; i < imageHeight; ++i) {
      for (int j = 0; j < imageWidth; ++j) {
        colors[count++] = colorImage->getPixel(i, j, 0) * scale;
        colors[count++] = colorImage->getPixel(i, j, 1) * scale;
        colors[count++] = colorImage->getPixel(i, j, 2) * scale;
        colors[count++] = 1.0f;
      }
    }
  } else if (renderMode == 2){  // texture color image
    sizeColor = numVertices * 4 * 4;
    int count = 0;
    float scale = 1.0f / 255.0f;
    for (int i = 0; i < imageHeight - 1; ++i) {
      for (int j = 0; j < imageWidth - 1; ++j) {
        colors[count++] = colorImage->getPixel(i, j, 0) * scale;
        colors[count++] = colorImage->getPixel(i, j, 1) * scale;
        colors[count++] = colorImage->getPixel(i, j, 2) * scale;
        colors[count++] = 1.0f;
        colors[count++] = colorImage->getPixel(i + 1, j, 0) * scale;
        colors[count++] = colorImage->getPixel(i + 1, j, 1) * scale;
        colors[count++] = colorImage->getPixel(i + 1, j, 2) * scale;
        colors[count++] = 1.0f;
        colors[count++] = colorImage->getPixel(i, j + 1, 0) * scale;
        colors[count++] = colorImage->getPixel(i, j + 1, 1) * scale;
        colors[count++] = colorImage->getPixel(i, j + 1, 2) * scale;
        colors[count++] = 1.0f;
        colors[count++] = colorImage->getPixel(i + 1, j, 0) * scale;
        colors[count++] = colorImage->getPixel(i + 1, j, 1) * scale;
        colors[count++] = colorImage->getPixel(i + 1, j, 2) * scale;
        colors[count++] = 1.0f;
        colors[count++] = colorImage->getPixel(i, j + 1, 0) * scale;
        colors[count++] = colorImage->getPixel(i, j + 1, 1) * scale;
        colors[count++] = colorImage->getPixel(i, j + 1, 2) * scale;
        colors[count++] = 1.0f;
        colors[count++] = colorImage->getPixel(i + 1, j + 1, 0) * scale;
        colors[count++] = colorImage->getPixel(i + 1, j + 1, 1) * scale;
        colors[count++] = colorImage->getPixel(i + 1, j + 1, 2) * scale;
        colors[count++] = 1.0f;
      }
    }
  }
}


void initScene(int argc, char *argv[])
{
  // 1. Load the image from a jpeg disk file to main memory
  heightmapImage = new ImageIO();
  if (heightmapImage->loadJPEG(argv[1]) != ImageIO::OK)
  {
    cout << "Error reading image " << argv[1] << "." << endl;
    exit(EXIT_FAILURE);
  }
  myRenderPosition();
  myRenderColor();
  
  // 2. Clear and set scene
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  glEnable(GL_DEPTH_TEST);

  // 3. Initialize VAO and VBO
  glGenVertexArrays(1, &VertexArrayID);
  glBindVertexArray(VertexArrayID);
  glGenBuffers(1, &vertexbuffer);
  glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer); 
  glBufferData(GL_ARRAY_BUFFER, sizePosition, NULL, GL_STATIC_DRAW); 
  glBufferData(GL_ARRAY_BUFFER, sizePosition + sizeColor, NULL, GL_STATIC_DRAW); 
  glBufferSubData(GL_ARRAY_BUFFER, 0, sizePosition, &positions[0]);
  glBufferSubData(GL_ARRAY_BUFFER, sizePosition, sizeColor, &colors[0]); 

  // 4. Initialize matrix and pipeline
  matrix = new OpenGLMatrix();
  pipelineProgram = new BasicPipelineProgram();
  pipelineProgram->Init("../openGLHelper-starterCode");
  pipelineProgram->Bind();
  programID = pipelineProgram->GetProgramHandle();
}

int main(int argc, char *argv[])
{
  if (argc != 2)
  {
    cout << "The arguments are incorrect." << endl;
    cout << "usage: ./hw1 <heightmap file>" << endl;
    exit(EXIT_FAILURE);
  }

  cout << "Initializing GLUT..." << endl;
  glutInit(&argc,argv);

  cout << "Initializing OpenGL..." << endl;

  #ifdef __APPLE__
    glutInitDisplayMode(GLUT_3_2_CORE_PROFILE | GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_STENCIL);
  #else
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_STENCIL);
  #endif

  glutInitWindowSize(windowWidth, windowHeight);
  glutInitWindowPosition(0, 0);  
  glutCreateWindow(windowTitle);

  cout << "OpenGL Version: " << glGetString(GL_VERSION) << endl;
  cout << "OpenGL Renderer: " << glGetString(GL_RENDERER) << endl;
  cout << "Shading Language Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;

  // tells glut to use a particular display function to redraw 
  glutDisplayFunc(displayFunc);
  // perform animation inside idleFunc
  glutIdleFunc(idleFunc);
  // callback for mouse drags
  glutMotionFunc(mouseMotionDragFunc);
  // callback for idle mouse movement
  glutPassiveMotionFunc(mouseMotionFunc);
  // callback for mouse button changes
  glutMouseFunc(mouseButtonFunc);
  // callback for resizing the window
  glutReshapeFunc(reshapeFunc);
  // callback for pressing the keys on the keyboard
  glutKeyboardFunc(keyboardFunc);

  // init glew
  #ifdef __APPLE__
    // nothing is needed on Apple
  #else
    // Windows, Linux
    GLint result = glewInit();
    if (result != GLEW_OK)
    {
      cout << "error: " << glewGetErrorString(result) << endl;
      exit(EXIT_FAILURE);
    }
  #endif

  // do initialization
  initScene(argc, argv);

  // sink forever into the glut loop
  glutMainLoop();
}


