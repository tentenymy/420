#include "hw2.h"
using namespace std;

/* HW2 tips */
/*void addTriangle(float posA[3], float posB[3], float posC[3], float uvA[2], float uvB[2], float uvC[2]) {
  for (int i = 0; i < 3; ++i)
    pos.push_back(posA[i]); 
  for (int i = 0; i < 3; ++i)
    pos.push_back(posB[i]); 
  for (int i = 0; i < 3; ++i)
    pos.push_back(posC[i]); 
  for (int i = 0; i < 2; ++i)
    uvs.push_back(uvA[i]); 
  for (int i = 0; i < 2; ++i)
    uvs.push_back(uvB[i]); 
  for (int i = 0; i < 2; ++i)
    uvs.push_back(uvC[i]);
}*/

/* For debug */
void printDetail() {
  cout << "Vertices of Positions: " << pos.size() / 3 << endl;
  cout << "Vertices of Colors: " << col.size() / 3 << endl;
  cout << "Positions: " << endl;
  for (int i = 0; i < pos.size() / 3 && i < 10; ++i)
    cout << pos[3 * i] << ", " << pos[3 * i + 1] << ", " << pos[3 * i + 2] << endl;
  cout << endl;
}

/* HW2: initial postion of splines
 * Catmull-Rom Spline Matrix
 * P = U * Basic * Control 
 */
void myRenderPosition() {
  for(int i = 0; i < numSplines; ++i) {
    for (int j = 0; j < splines[i].numControlPoints - 3; j++) {
      float matrixControl[12] = {};
      for (int k = 0; k < 4; ++k) {
        matrixControl[3 * k] = splines[i].points[j + k].x;
        matrixControl[3 * k + 1] = splines[i].points[j + k].y;
        matrixControl[3 * k + 2] = splines[i].points[j + k].z;
      }
      for (float u = 0.0f; u < 1.0f; u += interval) {
        float matrixU[4] = {u * u * u, u * u, u, 1};
        float matrixUB[4] = {}, matrixUBC[3] = {};
        for (int m = 0; m < 4; ++m) {
          for (int n = 0; n < 4; ++n)
            matrixUB[m] += matrixU[n] * matrixBasic[n * 4 + m];
        }
        for (int m = 0; m < 3; ++m) {
          for (int n = 0; n < 4; ++n)
            matrixUBC[m] += matrixUB[n] * matrixControl[n * 3 + m];
        }
        pos.push_back(matrixUBC[0]);
        pos.push_back(matrixUBC[1]);
        pos.push_back(matrixUBC[2]);
      }
    }
    pos.push_back(splines[i].points[splines[i].numControlPoints - 2].x);
    pos.push_back(splines[i].points[splines[i].numControlPoints - 2].y);
    pos.push_back(splines[i].points[splines[i].numControlPoints - 2].z);
  }
}

/* HW2: initial color of splines
 * no color
 */
void myRenderColor() {}

void initScene(int argc, char *argv[]) {
  // 1. Initialize position and color
  myRenderPosition();
  myRenderColor();

  // 2. Clear and set scene
  glClearColor(colorClear[0], colorClear[1], colorClear[2], colorClear[3]);
  glEnable(GL_DEPTH_TEST);

  // 3. Initialize VAO and VBO
  GLsizei sizePos = pos.size() * sizeof(GLfloat);
  GLsizei sizeUV = uvs.size() * sizeof (GLfloat);
  GLsizei sizeCol = col.size() * sizeof(GLfloat);
  glGenVertexArrays(1, &vertexArrayObjects);
  glBindVertexArray(vertexArrayObjects); 
  glGenBuffers(1, &vertexBufferObject); 
  glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObject);  
  glBufferData(GL_ARRAY_BUFFER, sizePos + sizeCol + sizeUV, NULL, GL_STATIC_DRAW); 
  glBufferSubData(GL_ARRAY_BUFFER, 0, sizePos, &pos[0]); // Position
  glBufferSubData(GL_ARRAY_BUFFER, sizePos, sizeUV, &uvs[0]); // UV
  glBufferSubData(GL_ARRAY_BUFFER, sizePos + sizeUV, sizeCol, &col[0]); // Color

  // 4. Initialize matrix and pipeline
  glMatrix = new OpenGLMatrix();
  pipelineProgram = new BasicPipelineProgram();
  pipelineProgram->Init("../openGLHelper-starterCode");
  pipelineProgram->Bind();
  programID = pipelineProgram->GetProgramHandle();
}

/* save the screen shot if saveMode == 1 */
void idleFunc() {
  if (saveMode == 1) {
    char fName[] = 
    {'s', 'a', 'v', 'e', '/', saveScreenShotName1, saveScreenShotName2, saveScreenShotName3, '.', 'j', 'p', 'g', '\0'};
    ++saveScreenShotName3;
    if (saveScreenShotName3 > '9') {++saveScreenShotName2; saveScreenShotName3 = '0';}
    if (saveScreenShotName2 > '9') {++saveScreenShotName1; saveScreenShotName2 = '0'; cout << fName << endl;}
    if (saveScreenShotName1 > '9') saveScreenShotName1 = '0'; 
    saveScreenshot(fName);
  }
  glutPostRedisplay(); 
}

/* Modify the parameter in head file
 * matLookat: lookat parameter
 * drawArrayMode: GL_LINES or GL_TRIANGLES ..
 */
void displayFunc() {
  // 1. Clear the display and bind VAO and VBO, set data to VBO
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObject);
  GLuint attributePos = glGetAttribLocation(programID, "position"); // Position
  glEnableVertexAttribArray(attributePos);
  glVertexAttribPointer(attributePos, 3, GL_FLOAT, GL_FALSE, 0, (const void*) 0);
  GLuint attributeCol = glGetAttribLocation(programID, "color"); // Color
  glEnableVertexAttribArray(attributeCol);
  glVertexAttribPointer(attributeCol, 4, GL_FLOAT, GL_FALSE, 0, (const void*)(pos.size() * sizeof(GLfloat)));

  // 2. Set matrix transformation
  glMatrix->SetMatrixMode(OpenGLMatrix::ModelView); 
  glMatrix->LoadIdentity();
  glMatrix->LookAt(matLookat[0], matLookat[1], matLookat[2], matLookat[3], matLookat[4], matLookat[5], matLookat[6], matLookat[7], matLookat[8]); // default camera 
  glMatrix->Rotate(landRotate[0], 1.0, 0.0, 0.0); 
  glMatrix->Rotate(landRotate[1], 0.0, 1.0, 0.0); 
  glMatrix->Rotate(landRotate[2], 0.0, 0.0, 1.0);
  glMatrix->Translate(landTranslate[0], landTranslate[1], landTranslate[2]); 
  glMatrix->Scale(landScale[0], landScale[1], landScale[2]);

  // 3. Pass matrix to the pipeline
  float m[16] = {}, p[16] = {};
  glMatrix->GetMatrix(m);
  pipelineProgram->SetModelViewMatrix(m);
  glMatrix->SetMatrixMode(OpenGLMatrix::Projection); 
  glMatrix->GetMatrix(p);
  pipelineProgram->SetProjectionMatrix(p);
  glMatrix->SetMatrixMode(OpenGLMatrix::ModelView);

  // 4. Draw the triangle or lines
  glDrawArrays(drawArrayMode, 0, pos.size() / 3);
  glDisableVertexAttribArray(attributePos);
  glDisableVertexAttribArray(attributeCol);
  glutSwapBuffers();
}

/* Modify the parameter in head file
 * matPerspective: Perspective parameter
 */
void reshapeFunc(int w, int h) {
  glViewport(0, 0, w, h); 
  glMatrix->SetMatrixMode(OpenGLMatrix::Projection); 
  glMatrix->LoadIdentity();
  glMatrix->Perspective(matPerspective[0], matPerspective[1], matPerspective[2], matPerspective[3]);
  glMatrix->SetMatrixMode(OpenGLMatrix::ModelView); 
}

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

int loadSplines(char * argv) 
{
  char * cName = (char *) malloc(128 * sizeof(char));
  FILE * fileList;
  FILE * fileSpline;
  int iType, i = 0, j, iLength;

  // load the track file 
  fileList = fopen(argv, "r");
  if (fileList == NULL) {
    printf ("can't open file\n");
    exit(1);
  }
  
  // stores the number of splines in a global variable 
  fscanf(fileList, "%d", &numSplines);

  splines = (Spline*) malloc(numSplines * sizeof(Spline));

  // reads through the spline files 
  for (j = 0; j < numSplines; j++) {
    i = 0;
    fscanf(fileList, "%s", cName);
    fileSpline = fopen(cName, "r");

    if (fileSpline == NULL) {
      printf ("can't open file\n");
      exit(1);
    }

    // gets length for spline file
    fscanf(fileSpline, "%d %d", &iLength, &iType);

    // allocate memory for all the points
    splines[j].points = (Point *)malloc(iLength * sizeof(Point));
    splines[j].numControlPoints = iLength;

    // saves the data to the struct
    while (fscanf(fileSpline, "%lf %lf %lf", 
     & splines[j].points[i].x, 
     & splines[j].points[i].y, 
     & splines[j].points[i].z) != EOF) {
      i++;
    }
  }
  free(cName);
  return 0;
}

int initTexture(const char * imageFilename, GLuint textureHandle) 
{
  // read the texture image
  ImageIO img;
  ImageIO::fileFormatType imgFormat;
  ImageIO::errorType err = img.load(imageFilename, &imgFormat);

  if (err != ImageIO::OK) {
    printf("Loading texture from %s failed.\n", imageFilename);
    return -1;
  }

  // check that the number of bytes is a multiple of 4
  if (img.getWidth() * img.getBytesPerPixel() % 4) {
    printf("Error (%s): The width*numChannels in the loaded image must be a multiple of 4.\n", imageFilename);
    return -1;
  }

  // allocate space for an array of pixels
  int width = img.getWidth();
  int height = img.getHeight();
  unsigned char * pixelsRGBA = new unsigned char[4 * width * height]; // we will use 4 bytes per pixel, i.e., RGBA

  // fill the pixelsRGBA array with the image pixels
  memset(pixelsRGBA, 0, 4 * width * height); // set all bytes to 0
  for (int h = 0; h < height; h++) {
    for (int w = 0; w < width; w++) {
      // assign some default byte values (for the case where img.getBytesPerPixel() < 4)
      pixelsRGBA[4 * (h * width + w) + 0] = 0; // red
      pixelsRGBA[4 * (h * width + w) + 1] = 0; // green
      pixelsRGBA[4 * (h * width + w) + 2] = 0; // blue
      pixelsRGBA[4 * (h * width + w) + 3] = 255; // alpha channel; fully opaque

      // set the RGBA channels, based on the loaded image
      int numChannels = img.getBytesPerPixel();
      for (int c = 0; c < numChannels; c++) // only set as many channels as are available in the loaded image; the rest get the default value
        pixelsRGBA[4 * (h * width + w) + c] = img.getPixel(w, h, c);
    }
  }

  // bind the texture
  glBindTexture(GL_TEXTURE_2D, textureHandle);

  // initialize the texture
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixelsRGBA);

  // generate the mipmaps for this texture
  glGenerateMipmap(GL_TEXTURE_2D);

  // set the texture parameters
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  // query support for anisotropic texture filtering
  GLfloat fLargest;
  glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &fLargest);
  printf("Max available anisotropic samples: %f\n", fLargest);
  // set anisotropic texture filtering
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 0.5f * fLargest);

  // query for any errors
  GLenum errCode = glGetError();
  if (errCode != 0) {
    printf("Texture initialization error. Error code: %d.\n", errCode);
    return -1;
  }
  
  // de-allocate the pixel array -- it is no longer needed
  delete [] pixelsRGBA;

  return 0;
}

int main(int argc, char *argv[])
{
  if (argc < 2) {  
    printf ("usage: %s <trackfile>\n", argv[0]);
    exit(0);
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

  // load the splines from the provided filename (Homework2)
  loadSplines(argv[1]);
  printf("Loaded %d spline(s).\n", numSplines);
  for(int i = 0; i < numSplines; i++)
    printf("Num control points in spline %d: %d.\n", i, splines[i].numControlPoints);



  // do initialization
  initScene(argc, argv);

  // sink forever into the glut loop
  glutMainLoop();
}


