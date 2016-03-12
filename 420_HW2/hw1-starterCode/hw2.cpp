#include "hw2.h"
using namespace std;

////////////////////////////////////////////
////////////// Initialization //////////////
////////////////////////////////////////////
/* normalizae spline coord to the [-boundSpline, boundSpline] box */
void normalizeSpline() {
  double maxX = -1000.0, minX = 1000.0;
  double maxY = -1000.0, minY = 1000.0;
  double maxZ = -1000.0, minZ = 1000.0;
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
  scaleXYZ = 2.0 * boundSpline / scaleXYZ;
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


vector<float> getMatrixUBC(float u) {
  float matrixU[4] = {u * u * u, u * u, u, 1.0f}, matrixUB[4] = {};
  vector<float> matrixUBC(3, 0.0f);
  for (int m = 0; m < 4; ++m) 
    for (int n = 0; n < 4; ++n)
      matrixUB[m] += matrixU[n] * matrixBasic[n * 4 + m];
  for (int m = 0; m < 3; ++m) 
    for (int n = 0; n < 4; ++n)
      matrixUBC[m] += matrixUB[n] * matrixControl[n * 3 + m];
  return matrixUBC;
}

vector<float> getMatrixTanUBC(float u) {
  float matrixU[4] = {3.0f * u * u, 2.0f * u, 1.0f, 0.0f}, matrixUB[4] = {};
  vector<float> matrixUBC(3, 0.0f);
  for (int m = 0; m < 4; ++m) 
    for (int n = 0; n < 4; ++n)
      matrixUB[m] += matrixU[n] * matrixBasic[n * 4 + m];
  for (int m = 0; m < 3; ++m) 
    for (int n = 0; n < 4; ++n)
      matrixUBC[m] += matrixUB[n] * matrixControl[n * 3 + m];
  float temp = sqrt(matrixUBC[0] * matrixUBC[0] + matrixUBC[1] * matrixUBC[1] + matrixUBC[2] * matrixUBC[2]);
  temp = temp != 0 ? 1.0f / temp : 0;
  matrixUBC[0] *= temp;
  matrixUBC[1] *= temp;
  matrixUBC[2] *= temp;
  return matrixUBC;
}

void subdivideMatrixU(float u0, float u1) {
  if (u1 - u0 <= 0.000001f)
    return;
  float uMid = (u0 + u1) / 2.0f; 
  double lengthSquare = 0.0;
  vector<float> matrix0 = getMatrixUBC(u0);
  vector<float> matrix1 = getMatrixUBC(u1);
  for (int i = 0; i < 3; ++i)
   lengthSquare += (double)(matrix0[i] - matrix1[i]) * (double)(matrix0[i] - matrix1[i]);
  if (lengthSquare > intervalSpline) {
    subdivideMatrixU(u0, uMid);
    subdivideMatrixU(uMid, u1);
  } else {
    vector<float> matrixTan0 = getMatrixTanUBC(u0);
    vector<float> matrixTan1 = getMatrixTanUBC(u1);
    for (int i = 0; i < 3; ++i)
      posSpline.push_back(matrix0[i]);
    for (int i = 0; i < 3; ++i)
      posSpline.push_back(matrix1[i]);  
    for (int i = 0; i < 3; ++i)
      TSpline.push_back(matrixTan0[i]);
    for (int i = 0; i < 3; ++i)
      TSpline.push_back(matrixTan1[i]);
    maxSplineZ = maxSplineZ > matrix0[2] ? maxSplineZ : matrix0[2];
    maxSplineZ = maxSplineZ > matrix1[2] ? maxSplineZ : matrix1[2];
    return;
  }
}
/* Initial Spline line: intervalSpline */
void initialSpline() {
  // 1. Normalization the spline
  normalizeSpline();

  // 2. Initial data for splines posSpline = U * Basic * Control and TSpline = U' * Basic * Control
  for(int i = 0; i < numSplines; ++i) {
    for (int j = 0; j < splines[i].numControlPoints - 3; j++) {
      for (int k = 0; k < 4; ++k) {
        matrixControl[3 * k] = splines[i].points[j + k].x;
        matrixControl[3 * k + 1] = splines[i].points[j + k].y;
        matrixControl[3 * k + 2] = splines[i].points[j + k].z;
      }
      subdivideMatrixU(0.0f, 1.0f);  
    }
    posSpline.push_back(splines[i].points[splines[i].numControlPoints - 2].x);
    posSpline.push_back(splines[i].points[splines[i].numControlPoints - 2].y);
    posSpline.push_back(splines[i].points[splines[i].numControlPoints - 2].z);
    maxSplineZ = maxSplineZ > splines[i].points[splines[i].numControlPoints - 2].z ? maxSplineZ : splines[i].points[splines[i].numControlPoints - 2].z;
    TSpline.push_back(-1.0f);
    TSpline.push_back(0.0f);
    TSpline.push_back(0.0f);
  }
  // 1. Initial T0, N0 = T0 * V, B0 = T0 * N0;
  double N0[3] = {TSpline[1] * tempV[2] - TSpline[2] * tempV[1],
    TSpline[2] * tempV[0] - TSpline[0] * tempV[2],
    TSpline[0] * tempV[1] - TSpline[1] * tempV[0]};
  double tempSumN = N0[0] * N0[0] + N0[1] * N0[1] + N0[2] * N0[2];
  tempSumN = tempSumN == 0 ? 0.0 : 1.0 / sqrt(tempSumN);
  N0[0] *= tempSumN;
  N0[1] *= tempSumN;
  N0[2] *= tempSumN;
  NSpline.push_back((float)N0[0]);
  NSpline.push_back((float)N0[1]);
  NSpline.push_back((float)N0[2]);
  BSpline.push_back(TSpline[1] * NSpline[2] - TSpline[2] * NSpline[1]);
  BSpline.push_back(TSpline[2] * NSpline[0] - TSpline[0] * NSpline[2]);
  BSpline.push_back(TSpline[0] * NSpline[1] - TSpline[1] * NSpline[0]);

  // 3. Initial UV data for splines: uvSpline
  GLfloat tempUV[] = {
    0.0f, 0.0f,
    0.0f, 1.0f,
    1.0f, 0.0f,
    1.0f, 1.0f
  };
  for (int i = 0; i < sizeof(tempUV) / sizeof(GLfloat); ++i)
    uvSpline.push_back(tempUV[i]);

  // 4. Load texture
  glGenTextures(1, &textureSplineID);
  int code = initTexture(textureSplineFilename, textureSplineID);
  if (code != 0) {
    printf("Error loading the texture image. \n");
    exit(EXIT_FAILURE);
  }

  // 5. Link posSpline and uvSpline to the buffer
  glGenBuffers(1, &posSplineBuffer); 
  glBindBuffer(GL_ARRAY_BUFFER, posSplineBuffer);  
  glBufferData(GL_ARRAY_BUFFER, posSpline.size() * sizeof(GLfloat), &posSpline[0], GL_STATIC_DRAW); 
  glGenBuffers(1, &uvSplineBuffer);
  glBindBuffer(GL_ARRAY_BUFFER, uvSplineBuffer);
  glBufferData(GL_ARRAY_BUFFER, uvSpline.size() * sizeof(GLfloat), &uvSpline[0], GL_STATIC_DRAW);
}

/* Initial Rail (left, right, cross) */
float getRailPos(float center, float scaleT, float scaleN, float scaleB, float T, float N, float B) {
  return center + scaleT * T + scaleB * B + scaleN * N;
}

void initialRail() {
  // 2. Initial four of eight vertexs for rail Left / Right / Cross and push to the vector
  float vertexLeft[8][3] = {}, vertexRight[8][3] = {};
  for (int j = 0; j < 3; ++j) {
    vertexLeft[0][j] = getRailPos(posSpline[j] + centerRail * BSpline[j], scaleRailT, -scaleRailN, scaleRailB, TSpline[j], NSpline[j], BSpline[j]);
    vertexRight[0][j] = getRailPos(posSpline[j] - centerRail * BSpline[j], scaleRailT, -scaleRailN, scaleRailB, TSpline[j], NSpline[j], BSpline[j]);
  }
  for (int j = 0; j < 3; ++j) {
    vertexLeft[1][j] = getRailPos(posSpline[j] + centerRail * BSpline[j], scaleRailT, scaleRailN, scaleRailB, TSpline[j], NSpline[j], BSpline[j]);
    vertexRight[1][j] = getRailPos(posSpline[j] - centerRail * BSpline[j], scaleRailT, scaleRailN, scaleRailB, TSpline[j], NSpline[j], BSpline[j]);
  }
  for (int j = 0; j < 3; ++j) {
    vertexLeft[2][j] = getRailPos(posSpline[j] + centerRail * BSpline[j], scaleRailT, scaleRailN, -scaleRailB, TSpline[j], NSpline[j], BSpline[j]);
    vertexRight[2][j] = getRailPos(posSpline[j] - centerRail * BSpline[j], scaleRailT, scaleRailN, -scaleRailB, TSpline[j], NSpline[j], BSpline[j]);
  }
  for (int j = 0; j < 3; ++j) {
    vertexLeft[3][j] = getRailPos(posSpline[j] + centerRail * BSpline[j], scaleRailT, -scaleRailN, -scaleRailB, TSpline[j], NSpline[j], BSpline[j]);
    vertexRight[3][j] = getRailPos(posSpline[j] - centerRail * BSpline[j], scaleRailT, -scaleRailN, -scaleRailB, TSpline[j], NSpline[j], BSpline[j]);
  }
  int indexList1[4] = {1, 2, 0, 3};
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 3; j++) {
      posRailLeft.push_back(vertexLeft[indexList1[i]][j]);
      posRailRight.push_back(vertexRight[indexList1[i]][j]);
    }
  }
  
  // 3. Initial pos vector for Left / Right / Cross
  for (int i = 1; i < posSpline.size() / 3; ++i) {
    // 3.1 N1 = B0 * T1, B1 = T1 * N1;
    double B0[3] = {BSpline[i * 3 - 3], BSpline[i * 3 - 2], BSpline[i * 3 - 1]};
    double T1[3] = {TSpline[i * 3], TSpline[i * 3 + 1], TSpline[i * 3 + 2]};
    double N1[3] = {B0[1] * T1[2] - B0[2] * T1[1], B0[2] * T1[0] - B0[0] * T1[2], B0[0] * T1[1] - B0[1] * T1[0]};
    double tempSumN = N1[0] * N1[0] + N1[1] * N1[1] + N1[2] * N1[2];
    tempSumN = tempSumN == 0 ? 0.0 : 1.0 / sqrt(tempSumN);
    N1[0] *= tempSumN;
    N1[1] *= tempSumN;
    N1[2] *= tempSumN;
    double B1[3] = {T1[1] * N1[2] - T1[2] * N1[1], T1[2] * N1[0] - T1[0] * N1[2], T1[0] * N1[1] - T1[1] * N1[0]};
    double tempSumB = B1[0] * B1[0] + B1[1] * B1[1] + B1[2] * B1[2];
    tempSumB = tempSumB == 0 ? 0.0 : 1.0 / sqrt(tempSumB);
    B1[0] *= tempSumB;
    B1[1] *= tempSumB;
    B1[2] *= tempSumB;
    for (int j = 0; j < 3; ++j) {
      NSpline.push_back((float)N1[j]);
      BSpline.push_back((float)B1[j]);
    }
    
    // 3.2 Vertex4-7 Left / Right / Cross
    for (int j = 0; j < 3; ++j) {
      vertexLeft[4][j] = getRailPos(posSpline[i * 3 + j] + centerRail * B1[j], scaleRailT, -scaleRailN, scaleRailB, T1[j], N1[j], B1[j]);
      vertexLeft[5][j] = getRailPos(posSpline[i * 3 + j] + centerRail * B1[j], scaleRailT, scaleRailN, scaleRailB, T1[j], N1[j], B1[j]);
      vertexLeft[6][j] = getRailPos(posSpline[i * 3 + j] + centerRail * B1[j], scaleRailT, scaleRailN, -scaleRailB, T1[j], N1[j], B1[j]);
      vertexLeft[7][j] = getRailPos(posSpline[i * 3 + j] + centerRail * B1[j], scaleRailT, -scaleRailN, -scaleRailB, T1[j], N1[j], B1[j]);
      vertexRight[4][j] = getRailPos(posSpline[i * 3 + j] - centerRail * B1[j], scaleRailT, -scaleRailN, scaleRailB, T1[j], N1[j], B1[j]);
      vertexRight[5][j] = getRailPos(posSpline[i * 3 + j] - centerRail * B1[j], scaleRailT, scaleRailN, scaleRailB, T1[j], N1[j], B1[j]);
      vertexRight[6][j] = getRailPos(posSpline[i * 3 + j] - centerRail * B1[j], scaleRailT, scaleRailN, -scaleRailB, T1[j], N1[j], B1[j]);
      vertexRight[7][j] = getRailPos(posSpline[i * 3 + j] - centerRail * B1[j], scaleRailT, -scaleRailN, -scaleRailB, T1[j], N1[j], B1[j]);
    }

    // 3.3 push to vector pos Left / Right /Cross
    int indexList2[] = {
      0, 1, 3, 1, 3, 2, 3, 2, 7, 2, 7, 6,
      7, 6, 4, 6, 4, 5, 4, 5, 0, 5, 0, 1, 
      1, 2, 5, 2, 5, 6, 3, 0, 7, 0, 7, 4
    };
    for (int k = 0; k < sizeof(indexList2) / sizeof(int); ++k) {
      for (int j = 0; j < 3; ++j) {
        posRailLeft.push_back(vertexLeft[indexList2[k]][j]);
        posRailRight.push_back(vertexRight[indexList2[k]][j]);
      }
    }

    // 3.4 store back to 0
    for (int k = 0; k < 4; k++) {
      for (int j = 0; j < 3; j++) {
        vertexLeft[k][j] = vertexLeft[k + 4][j];
        vertexRight[k][j] = vertexRight[k + 4][j];
      }
    }
  }

  // 4. Initial uvRail and uvCross
  GLfloat tempUV[] = {0.8, 0.2, 0.2, 0.2, 0.2, 0.8, 0.2, 0.2, 0.2, 0.8, 0.8, 0.8};
  for (int i = 0; i < posRailLeft.size(); i++) {
    uvRail.push_back(tempUV[i % 6]);
  }

  // 6. Link vbo with pos RailLeft / Right
  glGenBuffers(1, &posRailLeftBuffer);  // posRailLeft
  glBindBuffer(GL_ARRAY_BUFFER, posRailLeftBuffer);  
  glBufferData(GL_ARRAY_BUFFER, posRailLeft.size() * sizeof(GLfloat) , &posRailLeft[0], GL_STATIC_DRAW); 
  glGenBuffers(1, &posRailRightBuffer);  // posRailRight
  glBindBuffer(GL_ARRAY_BUFFER, posRailRightBuffer);  
  glBufferData(GL_ARRAY_BUFFER, posRailRight.size() * sizeof(GLfloat) , &posRailRight[0], GL_STATIC_DRAW); 
  glGenBuffers(1, &uvRailBuffer); // uvRail
  glBindBuffer(GL_ARRAY_BUFFER, uvRailBuffer);
  glBufferData(GL_ARRAY_BUFFER, uvRail.size() * sizeof(GLfloat), &uvRail[0], GL_STATIC_DRAW);
}

void initialCross() {
  // 2. Initial eight vertexs for rail Left / Right / Cross and push to the vector
  float vertexCross[8][3] = {};
  for (int i = 0; i < posSpline.size() / 3 - lengthCross; i += distanceCross) {
    for (int j = 0; j < 3; ++j) {
      vertexCross[0][j] = getRailPos(posSpline[i * 3 + j] - centerCrossN * NSpline[i * 3 + j], scaleCrossT, -scaleCrossN, scaleCrossB, TSpline[i * 3 + j], NSpline[i * 3 + j], BSpline[i * 3 + j]);
      vertexCross[1][j] = getRailPos(posSpline[i * 3 + j] - centerCrossN * NSpline[i * 3 + j], scaleCrossT, scaleCrossN, scaleCrossB, TSpline[i * 3 + j], NSpline[i * 3 + j], BSpline[i * 3 + j]);
      vertexCross[2][j] = getRailPos(posSpline[i * 3 + j] - centerCrossN * NSpline[i * 3 + j], scaleCrossT, scaleCrossN, -scaleCrossB, TSpline[i * 3 + j], NSpline[i * 3 + j], BSpline[i * 3 + j]);
      vertexCross[3][j] = getRailPos(posSpline[i * 3 + j] - centerCrossN * NSpline[i * 3 + j], scaleCrossT, -scaleCrossN, -scaleCrossB, TSpline[i * 3 + j], NSpline[i * 3 + j], BSpline[i * 3 + j]);
      vertexCross[4][j] = getRailPos(posSpline[i * 3 + j + lengthCross * 3] - centerCrossN * NSpline[i * 3 + j + lengthCross * 3], scaleCrossT, -scaleCrossN, scaleCrossB, TSpline[i * 3 + j + lengthCross * 3], NSpline[i * 3 + j + lengthCross * 3], BSpline[i * 3 + j + lengthCross * 3]);
      vertexCross[5][j] = getRailPos(posSpline[i * 3 + j + lengthCross * 3] - centerCrossN * NSpline[i * 3 + j + lengthCross * 3], scaleCrossT, scaleCrossN, scaleCrossB, TSpline[i * 3 + j + lengthCross * 3], NSpline[i * 3 + j + lengthCross * 3], BSpline[i * 3 + j + lengthCross * 3]);
      vertexCross[6][j] = getRailPos(posSpline[i * 3 + j + lengthCross * 3] - centerCrossN * NSpline[i * 3 + j + lengthCross * 3], scaleCrossT, scaleCrossN, -scaleCrossB, TSpline[i * 3 + j + lengthCross * 3], NSpline[i * 3 + j + lengthCross * 3], BSpline[i * 3 + j + lengthCross * 3]);
      vertexCross[7][j] = getRailPos(posSpline[i * 3 + j + lengthCross * 3] - centerCrossN * NSpline[i * 3 + j + lengthCross * 3], scaleCrossT, -scaleCrossN, -scaleCrossB, TSpline[i * 3 + j + lengthCross * 3], NSpline[i * 3 + j + lengthCross * 3], BSpline[i * 3 + j + lengthCross * 3]);
    }
    int indexList2[] = {
      0, 1, 3, 1, 3, 2, 3, 2, 7, 2, 7, 6,
      7, 6, 4, 6, 4, 5, 4, 5, 0, 5, 0, 1, 
      1, 2, 5, 2, 5, 6, 3, 0, 7, 0, 7, 4
    };
    for (int k = 0; k < sizeof(indexList2) / sizeof(int); ++k) 
      for (int j = 0; j < 3; ++j) 
        posCross.push_back(vertexCross[indexList2[k]][j]);
  }

  // 4. Initial uvRail and uvCross
  GLfloat tempUV[] = {0.2, 0.2, 0.2, 0.8, 0.8, 0.2, 0.2, 0.8, 0.8, 0.2, 0.8, 0.8};
  for (int i = 0; i < posCross.size(); i++) 
    uvCross.push_back(tempUV[i % 12]);

   // 5. Load texture for Cross
  glGenTextures(1, &textureCrossID);
  int code = initTexture(textureCrossFilename, textureCrossID);
  if (code != 0) {
    printf("Error loading the texture image. \n");
    exit(EXIT_FAILURE);
  }

  // 6. Link vbo with pos Cross
  glGenBuffers(1, &posCrossBuffer);  // posRailCross
  glBindBuffer(GL_ARRAY_BUFFER, posCrossBuffer);  
  glBufferData(GL_ARRAY_BUFFER, posCross.size() * sizeof(GLfloat) , &posCross[0], GL_STATIC_DRAW); 
  glGenBuffers(1, &uvCrossBuffer); // uvCross
  glBindBuffer(GL_ARRAY_BUFFER, uvCrossBuffer);
  glBufferData(GL_ARRAY_BUFFER, uvCross.size() * sizeof(GLfloat), &uvCross[0], GL_STATIC_DRAW);
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

  // 3. Link vbo for ground and sky
  glGenBuffers(1, &posGroundBuffer); // posGround
  glBindBuffer(GL_ARRAY_BUFFER, posGroundBuffer);  
  glBufferData(GL_ARRAY_BUFFER, posGround.size() * sizeof(GLfloat), &posGround[0], GL_STATIC_DRAW); 
  glGenBuffers(1, &uvGroundBuffer); // uvGround
  glBindBuffer(GL_ARRAY_BUFFER, uvGroundBuffer);
  glBufferData(GL_ARRAY_BUFFER, uvGround.size() * sizeof(GLfloat), &uvGround[0], GL_STATIC_DRAW);
  glGenBuffers(1, &posSkyBuffer); // posSky
  glBindBuffer(GL_ARRAY_BUFFER, posSkyBuffer);  
  glBufferData(GL_ARRAY_BUFFER, posSky.size() * sizeof(GLfloat), &posSky[0], GL_STATIC_DRAW); 
  glGenBuffers(1, &uvSkyBuffer); // uvSky
  glBindBuffer(GL_ARRAY_BUFFER, uvSkyBuffer);
  glBufferData(GL_ARRAY_BUFFER, uvSky.size() * sizeof(GLfloat), &uvSky[0], GL_STATIC_DRAW);
}

void printDetail() {
  cout << endl << "********* Detail *********" << endl;
  cout << "Ground: pos/uv: " << posGround.size() << "/" << uvGround.size() << endl;
  cout << "Sky: pos/uv: " << posSky.size() << "/" << uvSky.size() << endl;
  cout << "Spline: num/pos/uv/tan: " << splines[0].numControlPoints << "/" << posSpline.size() << "/" << uvSpline.size() << "/" << TSpline.size() << endl;
  cout << "Spline: T/N/B: " << TSpline.size() << "/" << NSpline.size() << "/" << BSpline.size() << endl;
  cout << "T: " << TSpline[0] << "/" << TSpline[1] << "/" << TSpline[2] << endl;
  cout << "N: " << NSpline[0] << "/" << NSpline[1] << "/" << NSpline[2] << endl;
  cout << "B: " << BSpline[0] << "/" << BSpline[1] << "/" << BSpline[2] << endl;
  cout << "V: " << tempV[0] << "/" << tempV[1] << "/" << tempV[2] << endl;
  int count = 0;
  cout << endl << "N list: " << endl;
  for (int i = 0; i < NSpline.size() / 3; ++i) {
    if (i < 80){
       cout << NSpline[i * 3] << " \t " << NSpline[i * 3 + 1] << " \t " << NSpline[i * 3 + 2] << "\t";
       cout << NSpline[i * 3] * NSpline[i * 3] + NSpline[i * 3 + 1] * NSpline[i * 3 + 1] + NSpline[i * 3 + 2] * NSpline[i * 3 + 2] << endl;
    }
     
    if (NSpline[i * 3 + 2] <= 0)
      count++;
  }
  cout << endl << "B list: " << endl;
  for (int i = 0; i < BSpline.size() / 3; ++i) {
    if (i < 80){
       cout << BSpline[i * 3] << " \t " << BSpline[i * 3 + 1] << " \t " << BSpline[i * 3 + 2] << "\t";
       cout << BSpline[i * 3] * BSpline[i * 3] + BSpline[i * 3 + 1] * BSpline[i * 3 + 1] + BSpline[i * 3 + 2] * BSpline[i * 3 + 2] << endl;
    }
  }
  cout << endl << "Spline list: " << endl;
  for (int i = 0; i < posSpline.size() / 3; ++i) {
    //if (i < 80){
       cout << posSpline[i * 3] << " \t " << posSpline[i * 3 + 1] << " \t " << posSpline[i * 3 + 2] << endl;
    //}
  }
  cout << "Count negative: " << count << " / " << NSpline.size() << endl << endl;
  cout << "Rail: posLeft/posRight/uv: " << posRailLeft.size() << "/" << posRailRight.size() << "/" << uvRail.size() << endl;
  cout << "Cross: pos/uv: " << posCross.size() << "/" << uvCross.size() << endl;
  cout << endl;
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
  initialCross();

  // 6. Print Detail
  printDetail();
}

/////////////////////////////////////
////////////// Display //////////////
/////////////////////////////////////
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
  glDrawArrays(GL_TRIANGLE_STRIP, 0, posRailLeft.size() / 3);
  glDisableVertexAttribArray(locPos);
  glDisableVertexAttribArray(locUV);

  bindTexture(GL_TEXTURE1, textureSplineID);
  locPos = bindBufferPos(posRailRightBuffer);
  locUV = bindBufferUV(uvRailBuffer);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, posRailRight.size() / 3);
  glDisableVertexAttribArray(locPos);
  glDisableVertexAttribArray(locUV);

  bindTexture(GL_TEXTURE1, textureCrossID);
  locPos = bindBufferPos(posCrossBuffer);
  locUV = bindBufferUV(uvCrossBuffer);
  glDrawArrays(GL_TRIANGLES, 0, posCross.size() / 3);
  glDisableVertexAttribArray(locPos);
  glDisableVertexAttribArray(locUV);
}

void updateCamera() {
  if (countPoint == 0 && waitingCamera <= waitingCameraMax) {
    waitingCamera++;
    matLookat[0] = posSpline[0] + scaleCameraN * NSpline[0];
    matLookat[1] = posSpline[1] + scaleCameraN * NSpline[1];
    matLookat[2] = posSpline[2] + scaleCameraN * NSpline[2];
    matLookat[3] = posSpline[0] + TSpline[0] * scaleCameraT + scaleCameraN * NSpline[0];
    matLookat[4] = posSpline[1] + TSpline[1] * scaleCameraT + scaleCameraN * NSpline[1];
    matLookat[5] = posSpline[2] + TSpline[2] * scaleCameraT + scaleCameraN * NSpline[2];
    matLookat[6] = NSpline[0];
    matLookat[7] = NSpline[1];
    matLookat[8] = NSpline[2];
  } else {
     int index = (int)countPoint * 3;
    float T1[3] = {TSpline[index], TSpline[index + 1], TSpline[index + 2]};
    float N1[3] = {NSpline[index], NSpline[index + 1], NSpline[index + 2]};
    float B1[3] = {BSpline[index], BSpline[index + 1], BSpline[index + 2]};
    matLookat[0] = posSpline[index] + scaleCameraN * N1[0];
    matLookat[1] = posSpline[index + 1] + scaleCameraN * N1[1];
    matLookat[2] = posSpline[index + 2] + scaleCameraN * N1[2];
    matLookat[3] = posSpline[index] + TSpline[index] * scaleCameraT + scaleCameraN * N1[0];
    matLookat[4] = posSpline[index + 1] + TSpline[index + 1] * scaleCameraT + scaleCameraN * N1[1];
    matLookat[5] = posSpline[index + 2] + TSpline[index + 2] * scaleCameraT + scaleCameraN * N1[2];
    matLookat[6] = N1[0];
    matLookat[7] = N1[1];
    matLookat[8] = N1[2];
    
    float tempSpeed = sqrt(maxSplineZ - posSpline[index + 2]);
    countPoint += speedCamera * tempSpeed + minSpeed;
    //cout << countPoint << endl;
    if (countPoint >= posSpline.size() / 3) {
      countPoint = 0;
      waitingCamera = 0;
    }
  }
}


void displayFunc() {
  // 1. Clear the display and bind VAO and VBO, set data to VBO
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Camera
  if (updateCameraMode)
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
  //drawSpline();
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

/////////////////////////////////////
////////////// Reshape //////////////
/////////////////////////////////////
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


