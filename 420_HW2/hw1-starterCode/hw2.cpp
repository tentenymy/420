#include "hw2.h"
using namespace std;

void normalizeSpline() {
  double maxX = -1000.0, minX = 1000.0;
  double maxY = -1000.0, minY = 1000.0;
  double maxZ = -1000.0, minZ = 1000.0;
  double bound = 0.8;
  for (int i = 0; i < numSplines; ++i) {
    for (int j = 0; j < splines[i].numControlPoints; j++) {
      if (splines[i].points[j].x > maxX)
          maxX = splines[i].points[j].x;
      if (splines[i].points[j].x < minX)
          minX = splines[i].points[j].x;

      if (splines[i].points[j].y > maxY)
          maxY = splines[i].points[j].y;
      if (splines[i].points[j].y < minY)
          minY = splines[i].points[j].y;

      if (splines[i].points[j].z > maxZ)
          maxZ = splines[i].points[j].z;
      if (splines[i].points[j].z < minZ)
          minZ = splines[i].points[j].z;
    }
  }
  double scaleXYZ = maxX - minX > maxY - minY ? maxX - minX : maxY - minY;
  scaleXYZ = scaleXYZ < maxZ - minZ ? maxZ - minZ : scaleXYZ;
  scaleXYZ = 2.0 * bound / scaleXYZ;
  double centerX = (maxX - minX) * 0.5;
  double centerY = (maxY - minY) * 0.5;
  double centerZ = (maxZ - minZ) * 0.5;

  for (int i = 0; i < numSplines; ++i) {
    for (int j = 0; j < splines[i].numControlPoints; j++) {
      splines[i].points[j].x = (splines[i].points[j].x - centerX) * scaleXYZ;
      splines[i].points[j].y = (splines[i].points[j].y - centerY) * scaleXYZ;
      splines[i].points[j].z = (splines[i].points[j].z - centerZ) * scaleXYZ;
    }
  }
}

void initialSpline() {
  // 1. Normalization the spline
  normalizeSpline();

  // 2. Initial data for splines
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
        float matrixTanU[4] = {3 * u * u, 2 * u, 1, 0};
        float matrixTanUB[4] = {}, matrixTanUBC[3] = {};
        for (int m = 0; m < 4; ++m) {
          for (int n = 0; n < 4; ++n)
            matrixUB[m] += matrixU[n] * matrixBasic[n * 4 + m];
        }
        for (int m = 0; m < 3; ++m) {
          for (int n = 0; n < 4; ++n)
            matrixUBC[m] += matrixUB[n] * matrixControl[n * 3 + m];
        }
         for (int m = 0; m < 4; ++m) {
          for (int n = 0; n < 4; ++n)
            matrixTanUB[m] += matrixTanU[n] * matrixBasic[n * 4 + m];
        }
        for (int m = 0; m < 3; ++m) {
          for (int n = 0; n < 4; ++n)
            matrixTanUBC[m] += matrixTanUB[n] * matrixControl[n * 3 + m];
        }
        float temp = 1 / sqrt(matrixTanUBC[0] * matrixTanUBC[0] + matrixTanUBC[1] * matrixTanUBC[1] + matrixTanUBC[2] * matrixTanUBC[2]);
        matrixTanUBC[0] *= temp;
        matrixTanUBC[1] *= temp;
        matrixTanUBC[2] *= temp;
        posSpline.push_back(matrixUBC[0]);
        posSpline.push_back(matrixUBC[1]);
        posSpline.push_back(matrixUBC[2]);
        tanSpline.push_back(matrixTanUBC[0]);
        tanSpline.push_back(matrixTanUBC[1]);
        tanSpline.push_back(matrixTanUBC[2]);
      }
    }
    posSpline.push_back(splines[i].points[splines[i].numControlPoints - 2].x);
    posSpline.push_back(splines[i].points[splines[i].numControlPoints - 2].y);
    posSpline.push_back(splines[i].points[splines[i].numControlPoints - 2].z);
    tanSpline.push_back(-1.0f);
    tanSpline.push_back(0.0f);
    tanSpline.push_back(0.0f);
  }
  GLfloat tempUV[] = {
    0.0f, 0.0f,
    0.1f, 0.0f,
    0.1f, 0.1f
  };
  for (int i = 0; i < sizeof(tempUV) / sizeof(GLfloat); i++) {
    uvSpline.push_back(tempUV[i]);
  }

  // 2. Load texture
  glGenTextures(1, &textureSplineID);
  int code = initTexture(textureSplineFilename, textureSplineID);
  if (code != 0) {
    printf("Error loading the texture image. \n");
    exit(EXIT_FAILURE);
  }

  // 3. Link vbo
  glGenBuffers(1, &posSplineBuffer); 
  glBindBuffer(GL_ARRAY_BUFFER, posSplineBuffer);  
  glBufferData(GL_ARRAY_BUFFER, posSpline.size() * sizeof(GLfloat), &posSpline[0], GL_STATIC_DRAW); 
  glGenBuffers(1, &uvSplineBuffer);
  glBindBuffer(GL_ARRAY_BUFFER, uvSplineBuffer);
  glBufferData(GL_ARRAY_BUFFER, uvSpline.size() * sizeof(GLfloat), &uvSpline[0], GL_STATIC_DRAW);
}

void initialRail() {
  float T0[3] = {tanSpline[0], tanSpline[1], tanSpline[2]};
  float V[3] = {0, 1, 0};
  N0[0] = T0[1] * V[2] - T0[2] * V[1];
  N0[1] = T0[2] * V[0] - T0[0] * V[2];
  N0[2] = T0[0] * V[1] - T0[1] * V[0];
  B0[0] = T0[1] * N0[2] - T0[2] * N0[1];
  B0[1] = T0[2] * N0[0] - T0[0] * N0[2];
  B0[2] = T0[0] * N0[1] - T0[1] * N0[0];
  float tempB0[3] = {B0[0], B0[1], B0[2]};

  float vertex[8][3] = {
    posSpline[0] + scaleRail * (-N0[0] + B0[0]),
    posSpline[1] + scaleRail * (-N0[1] + B0[1]),
    posSpline[2] + scaleRail * (-N0[2] + B0[2]),
    posSpline[0] + scaleRail * (N0[0] + B0[0]),
    posSpline[1] + scaleRail * (N0[1] + B0[1]),
    posSpline[2] + scaleRail * (N0[2] + B0[2]),
    posSpline[0] + scaleRail * (N0[0] - B0[0]),
    posSpline[1] + scaleRail * (N0[1] - B0[1]),
    posSpline[2] + scaleRail * (N0[2] - B0[2]),
    posSpline[0] + scaleRail * (-N0[0] - B0[0]),
    posSpline[1] + scaleRail * (-N0[1] - B0[1]),
    posSpline[2] + scaleRail * (-N0[2] - B0[2])
  };

  float vertex2[8][3] = {
    posSpline[0] - scaleNCross * N0[0] + scaleBCross * B0[0],
    posSpline[1] - scaleNCross * N0[1] + scaleBCross * B0[1],
    posSpline[2] - scaleNCross * N0[2] + scaleBCross * B0[2],
    posSpline[0] + scaleNCross * N0[0] + scaleBCross * B0[0],
    posSpline[1] + scaleNCross * N0[1] + scaleBCross * B0[1],
    posSpline[2] + scaleNCross * N0[2] + scaleBCross * B0[2],
    posSpline[0] + scaleNCross * N0[0] - scaleBCross * B0[0],
    posSpline[1] + scaleNCross * N0[1] - scaleBCross * B0[1],
    posSpline[2] + scaleNCross * N0[2] - scaleBCross * B0[2],
    posSpline[0] - scaleNCross * N0[0] - scaleBCross * B0[0],
    posSpline[1] - scaleNCross * N0[1] - scaleBCross * B0[1],
    posSpline[2] - scaleNCross * N0[2] - scaleBCross * B0[2]
  };


  int indexlist[4] = {1, 2, 0, 3};
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 3; j++) {
      posRailLeft.push_back(vertex[indexlist[i]][j] + centerRail * B0[j]);
      posRailRight.push_back(vertex[indexlist[i]][j] - centerRail * B0[j]);
    }
  }
  
  // 2. Initial data for splines
  for (int i = 1; i < posSpline.size() / 3; ++i) {
    float T1[3] = {tanSpline[i * 3], tanSpline[i * 3 + 1], tanSpline[i * 3 + 2]};
    float N1[3] = { // BO * T1
      B0[1] * T1[2] - B0[2] * T1[1], 
      B0[2] * T1[0] - B0[0] * T1[2],
      B0[0] * T1[1] - B0[1] * T1[0]
    };
    float sumN1 = 1.0f / sqrt(N1[0] * N1[0] + N1[1] * N1[1] + N1[2] * N1[2]);
    N1[0] *= sumN1;
    N1[1] *= sumN1;
    N1[2] *= sumN1;
    
    float B1[3] = { // T1 * N1
      T1[1] * N1[2] - T1[2] * N1[1], 
      T1[2] * N1[0] - T1[0] * N1[2],
      T1[0] * N1[1] - T1[1] * N1[0]
    };
    float sumB1 = 1.0f / sqrt(B1[0] * B1[0] + B1[1] * B1[1] + B1[2] * B1[2]);
    B1[0] *= sumB1;
    B1[1] *= sumB1;
    B1[2] *= sumB1;
    
    for (int j = 0; j < 3; j++) {
      vertex[4][j] = posSpline[i * 3 + j] + scaleRail * (-N1[j] + B1[j]);
      vertex[5][j] = posSpline[i * 3 + j] + scaleRail * (N1[j] + B1[j]);
      vertex[6][j] = posSpline[i * 3 + j] + scaleRail * (N1[j] - B1[j]);
      vertex[7][j] = posSpline[i * 3 + j] + scaleRail * (-N1[j] - B1[j]);
    }

    for (int j = 0; j < 3; j++) {
      vertex2[4][j] = posSpline[i * 3 + j] - scaleNCross * N1[j] + scaleBCross * B1[j];
      vertex2[5][j] = posSpline[i * 3 + j] + scaleNCross * N1[j] + scaleBCross * B1[j];
      vertex2[6][j] = posSpline[i * 3 + j] + scaleNCross * N1[j] - scaleBCross * B1[j];
      vertex2[7][j] = posSpline[i * 3 + j] - scaleNCross * N1[j] - scaleBCross * B1[j];
    }
      
    int indexList[] = {
      0, 1, 3, 1, 3, 2, 3, 2, 7, 2, 7, 6,
      7, 6, 4, 6, 4, 5, 4, 5, 0, 5, 0, 1, 
      1, 2, 5, 2, 5, 6, 0, 3, 4, 3, 4, 7
    };
    for (int k = 0; k < sizeof(indexList) / sizeof(int); k++) {
      for (int j = 0; j < 3; j++) {
        float temp = indexList[k] < 4 ? centerRail * B0[j] : centerRail * B1[j];
        posRailLeft.push_back(vertex[indexList[k]][j] + temp);
        posRailRight.push_back(vertex[indexList[k]][j] - temp);
      }
    }

    if (i % 1000 == 0) {
      for (int k = 0; k < sizeof(indexList) / sizeof(int); k++) {
        for (int j = 0; j < 3; j++) {
          //float temp = indexList[k] <= 1 || indexList[k] == 4 || indexList[k] == 5  ? scaleBCross * B1[j] : -scaleBCross * B1[j];
          posRailCross.push_back(vertex2[indexList[k]][j]);
        }
      }
    }
    
    B0[0] = B1[0];
    B0[1] = B1[1];
    B0[2] = B1[2];
    for (int k = 0; k < 4; k++) {
      for (int j = 0; j < 3; j++)
        vertex[k][j] = vertex[k + 4][j];
    }
      
   for (int k = 0; k < 4; k++) {
    for (int j = 0; j < 3; j++)
      vertex2[k][j] = vertex2[k + 4][j];
   }
  }
  
  B0[0] = tempB0[0];
  B0[1] = tempB0[1];
  B0[2] = tempB0[2];

  // UV
  GLfloat tempUV[] = {
    0.0f, 0.0f,
    0.0f, 1.0f,
    1.0f, 0.0f,
    1.0f, 1.0f
  };
  for (int i = 0; i < sizeof(tempUV) / sizeof(GLfloat); i++) {
    uvRail.push_back(tempUV[i]);
  }

  for (int i = 0; i < sizeof(tempUV) / sizeof(GLfloat); i++) {
    uvRailCross.push_back(tempUV[i]);
  }

   // 2. Load texture
  glGenTextures(1, &textureRailCrossID);
  int code = initTexture(textureRailCrossFilename, textureRailCrossID);
  if (code != 0) {
    printf("Error loading the texture image. \n");
    exit(EXIT_FAILURE);
  }
  cout << textureRailCrossFilename << endl;

  // 3. Link vbo
  glGenBuffers(1, &posRailLeftBuffer); 
  glBindBuffer(GL_ARRAY_BUFFER, posRailLeftBuffer);  
  glBufferData(GL_ARRAY_BUFFER, posRailLeft.size() * sizeof(GLfloat) , &posRailLeft[0], GL_STATIC_DRAW); 

  glGenBuffers(1, &posRailRightBuffer); 
  glBindBuffer(GL_ARRAY_BUFFER, posRailRightBuffer);  
  glBufferData(GL_ARRAY_BUFFER, posRailRight.size() * sizeof(GLfloat) , &posRailRight[0], GL_STATIC_DRAW); 

  glGenBuffers(1, &posRailCrossBuffer); 
  glBindBuffer(GL_ARRAY_BUFFER, posRailCrossBuffer);  
  glBufferData(GL_ARRAY_BUFFER, posRailCross.size() * sizeof(GLfloat) , &posRailCross[0], GL_STATIC_DRAW); 

  glGenBuffers(1, &uvRailBuffer);
  glBindBuffer(GL_ARRAY_BUFFER, uvRailBuffer);
  glBufferData(GL_ARRAY_BUFFER, uvRail.size() * sizeof(GLfloat), &uvRail[0], GL_STATIC_DRAW);

  glGenBuffers(1, &uvRailCrossBuffer);
  glBindBuffer(GL_ARRAY_BUFFER, uvRailCrossBuffer);
  glBufferData(GL_ARRAY_BUFFER, uvRailCross.size() * sizeof(GLfloat), &uvRailCross[0], GL_STATIC_DRAW);
}

void initialEnvironment() {
  // 1. Initial data for sky and ground
  int numVertex = 6;
  for (int i = 0; i < numVertex * 3; ++i) 
    posGround.push_back(cubemapPos[i]);
  for (int i = 0; i < numVertex * 2; ++i) 
    uvGround.push_back(cubemapUV[i]);
  for (int i = numVertex * 3; i < sizeof(cubemapPos) / sizeof(GLfloat); ++i) 
    posSky.push_back(cubemapPos[i]);
  for (int i = numVertex * 2; i < sizeof(cubemapUV) / sizeof(GLfloat); ++i) 
    uvSky.push_back(cubemapUV[i]);

  // 2. Load cube map
  glGenTextures(1, &textureGroundID);
  int code = initTexture(textureGroundFilename, textureGroundID);
  if (code != 0) {
    printf("Error loading the texture image. \n");
    exit(EXIT_FAILURE);
  }

  // 3. Link vbo for ground
  glGenBuffers(1, &posGroundBuffer); 
  glBindBuffer(GL_ARRAY_BUFFER, posGroundBuffer);  
  glBufferData(GL_ARRAY_BUFFER, posGround.size() * sizeof(GLfloat), &posGround[0], GL_STATIC_DRAW); 
  glGenBuffers(1, &uvGroundBuffer);
  glBindBuffer(GL_ARRAY_BUFFER, uvGroundBuffer);
  glBufferData(GL_ARRAY_BUFFER, uvGround.size() * sizeof(GLfloat), &uvGround[0], GL_STATIC_DRAW);

  // 4. Link vbo for sky
  glGenBuffers(1, &posSkyBuffer); 
  glBindBuffer(GL_ARRAY_BUFFER, posSkyBuffer);  
  glBufferData(GL_ARRAY_BUFFER, posSky.size() * sizeof(GLfloat), &posSky[0], GL_STATIC_DRAW); 
  glGenBuffers(1, &uvSkyBuffer);
  glBindBuffer(GL_ARRAY_BUFFER, uvSkyBuffer);
  glBufferData(GL_ARRAY_BUFFER, uvSky.size() * sizeof(GLfloat), &uvSky[0], GL_STATIC_DRAW);
  
}

void initScene(int argc, char *argv[]) {
  // 1. Clear and set scene
  glClearColor(colorClear[0], colorClear[1], colorClear[2], colorClear[3]);
  glEnable(GL_DEPTH_TEST);

  // 2. Initialize matrix and pipeline
  glMatrix = new OpenGLMatrix();
  pipelineProgram = new BasicPipelineProgram();
  pipelineProgram->Init("../openGLHelper-starterCode");
  pipelineProgram->Bind();
  programID = pipelineProgram->GetProgramHandle();

  // 3. Initialize VAO 
  glGenVertexArrays(1, &vertexArrayObjects);
  glBindVertexArray(vertexArrayObjects); 

  // 4. Initial Uniform for shader
  h_textureSampler = glGetUniformLocation(programID, "myTextureSampler");

  // 5. Initialize data
  initialSpline();
  initialRail();
  initialEnvironment();
}

void bindTexture(GLint num, GLuint textureID) {
  glActiveTexture(num);
  glUniform1i(h_textureSampler, num - GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, textureID);
}

GLuint bindBufferPos (GLuint posBufferID) {
  glBindBuffer(GL_ARRAY_BUFFER, posBufferID);
  GLuint loc = glGetAttribLocation(programID, "position"); // Position
  glEnableVertexAttribArray(loc);
  glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 0, (const void*) 0);
  return loc;
}

GLuint bindBufferUV (GLuint uvBufferID) {
  glBindBuffer(GL_ARRAY_BUFFER, uvBufferID);
  GLuint loc = glGetAttribLocation(programID, "uv");
  glEnableVertexAttribArray(loc);
  glVertexAttribPointer(loc, 2, GL_FLOAT, GL_FALSE, 0, (const void*) 0);
  return loc;
}

void drawGround() {
  bindTexture(GL_TEXTURE0, textureGroundID);
  GLuint locPos = bindBufferPos(posGroundBuffer);
  GLuint locUV = bindBufferUV(uvGroundBuffer);
  glDrawArrays(GL_TRIANGLES, 0, posGround.size() / 3);
  glDisableVertexAttribArray(locPos);
  glDisableVertexAttribArray(locUV);
}

void drawSky() {
  bindTexture(GL_TEXTURE0, textureGroundID);
  GLuint locPos = bindBufferPos(posSkyBuffer);
  GLuint locUV = bindBufferUV(uvSkyBuffer);
  glDrawArrays(GL_TRIANGLES, 0, posSky.size() / 3);
  glDisableVertexAttribArray(locPos);
  glDisableVertexAttribArray(locUV);
}

void drawSpline() {
  bindTexture(GL_TEXTURE1, textureSplineID);
  GLuint locPos = bindBufferPos(posSplineBuffer);
  GLuint locUV = bindBufferUV(uvSplineBuffer);
  glDrawArrays(GL_LINE_STRIP, 0, posSpline.size() / 3);
  glDisableVertexAttribArray(locPos);
  glDisableVertexAttribArray(locUV);
}

void drawRail() {
  bindTexture(GL_TEXTURE1, textureSplineID);
  GLuint locPos = bindBufferPos(posRailLeftBuffer);
  GLuint locUV = bindBufferUV(uvRailBuffer);
  glDrawArrays(GL_TRIANGLES, 0, posRailLeft.size() / 3);
  glDisableVertexAttribArray(locPos);
  glDisableVertexAttribArray(locUV);

  bindTexture(GL_TEXTURE1, textureSplineID);
  locPos = bindBufferPos(posRailRightBuffer);
  locUV = bindBufferUV(uvRailBuffer);
  glDrawArrays(GL_TRIANGLES, 0, posRailRight.size() / 3);
  glDisableVertexAttribArray(locPos);
  glDisableVertexAttribArray(locUV);

  bindTexture(GL_TEXTURE1, textureRailCrossID);
  locPos = bindBufferPos(posRailCrossBuffer);
  locUV = bindBufferUV(uvRailCrossBuffer);
  glDrawArrays(GL_TRIANGLES, 0, posRailCross.size() / 3);
  glDisableVertexAttribArray(locPos);
  glDisableVertexAttribArray(locUV);
}

void updateCamera() {
  int index = countPoint;
  float T1[3] = {tanSpline[index], tanSpline[index + 1], tanSpline[index + 2]};
  float N1[3] = { // BO * T1
    B0[1] * T1[2] - B0[2] * T1[1], 
    B0[2] * T1[0] - B0[0] * T1[2],
    B0[0] * T1[1] - B0[1] * T1[0]
  };
  float sumN1 = sqrt(N1[0] * N1[0] + N1[1] * N1[1] + N1[2] * N1[2]);
  N1[0] /= sumN1;
  N1[1] /= sumN1;
  N1[2] /= sumN1;
  float B1[3] = { // T1 * N1
    T1[1] * N1[2] - T1[2] * N1[1], 
    T1[2] * N1[0] - T1[0] * N1[2],
    T1[0] * N1[1] - T1[1] * N1[0]
  };
  float sumB1 = sqrt(B1[0] * B1[0] + B1[1] * B1[1] + B1[2] * B1[2]);
  B1[0] /= sumB1;
  B1[1] /= sumB1;
  B1[2] /= sumB1;

  float scaleN = 0.06f;
  matLookat[0] = posSpline[index] + scaleN * N1[0];
  matLookat[1] = posSpline[index + 1] + scaleN * N1[1];
  matLookat[2] = posSpline[index + 2] + scaleN * N1[2];
  matLookat[3] = posSpline[index] + tanSpline[index] * scaleCamera + scaleN * N1[0];
  matLookat[4] = posSpline[index + 1] + tanSpline[index + 1] * scaleCamera + scaleN * N1[1];
  matLookat[5] = posSpline[index + 2] + tanSpline[index + 2] * scaleCamera + scaleN * N1[2];
  matLookat[6] = N1[0];
  matLookat[7] = N1[1];
  matLookat[8] = N1[2];
  countPoint += speedCamera;
  if (countPoint >= posSpline.size() - 100)
      exit(1);
  N0[0] = N1[0];
  N0[1] = N1[1];
  N0[2] = N1[2];
  B0[0] = B1[0];
  B0[1] = B1[1];
  B0[2] = B1[2];
}


void displayFunc() {
  // 1. Clear the display and bind VAO and VBO, set data to VBO
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Camera
  updateCamera();

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

  // 4. Draw
  drawGround();
  drawSky();
  drawSpline();
  drawRail();

  bindTexture(GL_TEXTURE1, textureGroundID);
  GLuint locPos = bindBufferPos(posSkyBuffer);
  GLuint locUV = bindBufferUV(uvSkyBuffer);
  glDrawArrays(GL_TRIANGLES, 0, posSky.size() / 3);
  glDisableVertexAttribArray(locPos);
  glDisableVertexAttribArray(locUV);

  // 5. Swap buffers
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


