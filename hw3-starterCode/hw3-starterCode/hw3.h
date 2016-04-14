/* **************************
 * CSCI 420
 * Assignment 3 Raytracer
 * Name: <Your name here>
 * *************************
*/

#ifdef WIN32
  #include <windows.h>
#endif

#if defined(WIN32) || defined(linux)
  #include <GL/gl.h>
  #include <GL/glut.h>
#elif defined(__APPLE__)
  #include <OpenGL/gl.h>
  #include <GLUT/glut.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef WIN32
  #define strcasecmp _stricmp
#endif

#include <imageIO.h>

#include <math.h>
#include <float.h>
#include <iostream>
using namespace std;

/****************************************************/
/**************** PARAMETER SETTING *****************/
/****************************************************/
#define MAX_TRIANGLES 20000
#define MAX_SPHERES 100
#define MAX_LIGHTS 100

char * filename = NULL;

//different display modes
#define MODE_DISPLAY 1
#define MODE_JPEG 2

int mode = MODE_DISPLAY;

//you may want to make these smaller for debugging purposes
#define WIDTH 80 
#define HEIGHT 60

//the field of view of the camera
#define fov 60.0
unsigned char buffer[HEIGHT][WIDTH][3];

const char BACKGROUND_R = (char)255;
const char BACKGROUND_G = (char)255;
const char BACKGROUND_B = (char)255;
const double PI = 3.14159265;
const double MIN_DEPTH = -10000000.0;
const int SPHERE = 0;
const int TRIANGLE = 1;

/********************************************************/
/**************** STRUCTURE DECLARATION *****************/
/********************************************************/
struct Vertex
{
  double position[3];
  double color_diffuse[3];
  double color_specular[3];
  double normal[3];
  double shininess;
};

struct Triangle
{
  Vertex v[3];
};

struct Sphere
{
  double position[3];
  double color_diffuse[3];
  double color_specular[3];
  double shininess;
  double radius;
};

struct Light
{
  double position[3];
  double color[3];
};

Triangle triangles[MAX_TRIANGLES];
Sphere spheres[MAX_SPHERES];
Light lights[MAX_LIGHTS];
double ambient_light[3];

int num_triangles = 0;
int num_spheres = 0;
int num_lights = 0;


/***************************************************/
/**************** CLASS DECLARATION ****************/
/***************************************************/
class Vec3 {
public:
  double x;
  double y;
  double z;
  Vec3(double newX, double newY, double newZ) {
    x = newX;
    y = newY;
    z = newZ;
  }
  static Vec3 Normalize(Vec3 v) {
    double temp = sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
    if (temp != 0)
        temp = 1.0 / temp;
    return Vec3(v.x * temp, v.y * temp, v.z * temp);
  }
  static Vec3 Multiply(Vec3 v1, double mul) {
    return Vec3(v1.x * mul, v1.y * mul, v1.z * mul);
  }
  static Vec3 Add(Vec3 v1, Vec3 v2) {
    return Vec3(v1.x + v2.x, v1.y + v2.y, v1.z + v2.z);
  }
  static Vec3 Minus(Vec3 v1, Vec3 v2) {
    return Vec3(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z);
  }
  static Vec3 CrossProduct(Vec3 v1, Vec3 v2) {
    return Vec3(v1.y * v2.z - v1.z * v2.y, v1.z * v2.x - v1.x * v2.z, v1.x * v2.y - v1.y * v2.x);
  }
  static double DotProduct(Vec3 v1, Vec3 v2) {
    return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
  }
  static double Magnitude(Vec3 v) {
    double temp = v.x * v.x + v.y * v.y + v.z * v.z;
    if (temp == 0)
      return 0;
    return sqrt(temp);
  }
  static Vec3 AddThreeScale(Vec3 v1, Vec3 v2, Vec3 v3, double scale1, double scale2, double scale3) {
    v1 = Multiply(v1, scale1);
    v2 = Multiply(v2, scale2);
    v3 = Multiply(v3, scale3);
    return Vec3(v1.x + v2.x + v3.x, v1.y + v2.y + v3.y, v1.z + v2.z + v3.z);
  }
};

/******************************************************/
/**************** FUNCTION DECLARATION ****************/
/******************************************************/
Vec3 IntersectSphere(Vec3 origin, Vec3 direction, Sphere sphere);
Vec3 IntersectTriangle(Vec3 origin, Vec3 direction, Triangle triangle);
bool IsNotShadow(Light light, Vec3 origin, int mode, int num);
void ColorSphere(Sphere sphere, Light light, Vec3 position, Vec3& color);
void ColorTriangle(Triangle triangle, Light light, Vec3 position, Vec3& color);
double BoundColor(double color);
void ColorAmbient(Vec3& color);
void SetColor(unsigned int x, unsigned int y, Vec3 position, int mode, int num);

void plot_pixel_display(int x,int y,unsigned char r,unsigned char g,unsigned char b);
void plot_pixel_jpeg(int x,int y,unsigned char r,unsigned char g,unsigned char b);
void plot_pixel(int x,int y,unsigned char r,unsigned char g,unsigned char b);

