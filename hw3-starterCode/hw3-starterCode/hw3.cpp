#include "hw3.h"
/***********************************************/
/**************** MODIFY ****************/
/***********************************************/
Vec3 IntersectSphere(Vec3 origin, Vec3 direction, Sphere sphere) {
  // 1. check intersect: t01 = -b +- sqrt(b^2 - 4c). If bb - 4c < 0, no intersect
  Vec3 position(0, 0, MIN_DEPTH);
  double b = 2.0 * (direction.x * (origin.x - sphere.position[0]) + direction.y * (origin.y - sphere.position[1]) 
    + direction.z * (origin.z - sphere.position[2]));
  double c = (origin.x - sphere.position[0]) * (origin.x - sphere.position[0]) 
    + (origin.y - sphere.position[1]) * (origin.y - sphere.position[1]) 
    + (origin.z - sphere.position[2]) * (origin.z - sphere.position[2]) 
    - sphere.radius * sphere.radius;
  if ((b * b - 4.0 * c) < 0)
    return position;
  double t0 = (-b + sqrt(b * b - 4.0 * c)) * 0.5;
  double t1 = (-b - sqrt(b * b - 4.0 * c)) * 0.5;
  if (t0 <= 0 && t1 <= 0) // out of range
    return position; 
  else if (t0 > 0 && t1 > 0)
    t0 = t0 < t1 ? t0 : t1; 
  else if (t1 > 0)
    t0 = t1;

  // 2. Set position
  position = Vec3(origin.x + t0 * direction.x, origin.y + t0 * direction.y, origin.z + t0 * direction.z);
  return position;
}

Vec3 IntersectTriangle(Vec3 origin, Vec3 direction, Triangle triangle) { 
  // 1. check intersect: t = -(n.p0 + D) / (n.d); If n.d = 0, no intersection; if t <= 0, intersection is behind ray origin
  Vec3 position(0, 0, MIN_DEPTH);
  Vec3 verA(triangle.v[0].position[0], triangle.v[0].position[1], triangle.v[0].position[2]);
  Vec3 verB(triangle.v[1].position[0], triangle.v[1].position[1], triangle.v[1].position[2]);
  Vec3 verC(triangle.v[2].position[0], triangle.v[2].position[1], triangle.v[2].position[2]);
  Vec3 normal = Vec3::Normalize(Vec3::CrossProduct(Vec3::Minus(verB, verA), Vec3::Minus(verC, verA)));
  double tempUp = Vec3::DotProduct(normal, Vec3::Minus(verA, origin));
  double tempDown = Vec3::DotProduct(normal, direction);
  if (tempDown == 0) // no intersection
    return position;
  double t = tempUp / tempDown;
  if (t <= 0) // behind ray origin
    return position;
  position = Vec3::Add(origin, Vec3::Multiply(direction, t)); 

  // 2. check whether position is inside the triangle
  double alpha = Vec3::DotProduct(normal, Vec3::CrossProduct(Vec3::Minus(verB, verA), Vec3::Minus(position, verA)));
  double beta = Vec3::DotProduct(normal, Vec3::CrossProduct(Vec3::Minus(verC, verB), Vec3::Minus(position, verB)));
  double omega = Vec3::DotProduct(normal, Vec3::CrossProduct(Vec3::Minus(verA, verC), Vec3::Minus(position, verC)));
  if (alpha < 0 || beta < 0 || omega < 0)
    return Vec3(0, 0, MIN_DEPTH);
  else
    return position;
}

bool IsNotShadow(Light light, Vec3 origin, int mode, int num){
  Vec3 direction = Vec3::Normalize(Vec3(light.position[0] - origin.x, light.position[1] - origin.y, light.position[2] - origin.z));
  for (int i = 0; i < num_spheres; i++) {
    if (mode == SPHERE && num == i)
      continue;
    Vec3 position = IntersectSphere(origin, direction, spheres[i]);
    if (position.z != MIN_DEPTH) 
      return true;
  }
  for (int i = 0; i < num_triangles; i++) {
    if (mode == TRIANGLE && num == i)
      continue;
    Vec3 position = IntersectTriangle(origin, direction, triangles[i]);
    if (position.z != MIN_DEPTH) 
      return true;
  }
  return false;
}

void ColorSphere(Sphere sphere, Light light, Vec3 position, Vec3& color) {
  // 1. calculate color: I = lightColor * (kd * L . N) + ks * (R . V) ^ sh);
  Vec3 paraN = Vec3::Normalize(Vec3(position.x - sphere.position[0], position.y - sphere.position[1],
    position.z - sphere.position[2]));
  Vec3 paraL = Vec3::Normalize(Vec3(light.position[0] - position.x, light.position[1] - position.y, light.position[2] - position.z));
  double mulLN = Vec3::DotProduct(paraL, paraN);
  Vec3 paraR = Vec3::Minus(Vec3::Multiply(paraN, mulLN * 2), paraL);
  Vec3 paraV = Vec3::Normalize(Vec3::Multiply(position, -1.0));
  double mulRV = Vec3::DotProduct(paraR, paraV);
  mulLN = mulLN < 0 ? 0 : mulLN;
  mulRV = mulRV < 0 ? 0 : mulRV;
  
  // 2. set color
  color.x += light.color[0] * ((sphere.color_diffuse[0] * mulLN + sphere.color_specular[0] * pow(mulRV, sphere.shininess)));
  color.y += light.color[1] * ((sphere.color_diffuse[1] * mulLN + sphere.color_specular[1] * pow(mulRV, sphere.shininess)));
  color.z += light.color[2] * ((sphere.color_diffuse[2] * mulLN + sphere.color_specular[2] * pow(mulRV, sphere.shininess)));
}

void ColorTriangle(Triangle triangle, Light light, Vec3 position, Vec3& color) {
  // 1. get scale for each vertex: alpha = Area(PBC) / Area(ABC), beta = Area(APC) / Area(ABC), omega = Area(ABP) / Area(ABC)
  Vertex v[3] = {triangle.v[0], triangle.v[1], triangle.v[2]};
  Vec3 verA = Vec3(v[0].position[0], v[0].position[1], v[0].position[2]);
  Vec3 verB = Vec3(v[1].position[0], v[1].position[1], v[1].position[2]);
  Vec3 verC = Vec3(v[2].position[0], v[2].position[1], v[2].position[2]);
  double areaA = Vec3::Magnitude(Vec3::CrossProduct(Vec3::Minus(verC, verB), Vec3::Minus(position, verB)));
  double areaB = Vec3::Magnitude(Vec3::CrossProduct(Vec3::Minus(verA, verC), Vec3::Minus(position, verC)));
  double areaC = Vec3::Magnitude(Vec3::CrossProduct(Vec3::Minus(verB, verA), Vec3::Minus(position, verA)));
  double totalArea = areaA + areaB + areaC;
  double scaleA = areaA / totalArea;
  double scaleB = areaB / totalArea;
  double scaleC = areaC / totalArea;

  // 2. calculate parameter and interpolate N and clamp: I = lightColor * (kd * L . N) + ks * (R . V) ^ sh);
  Vec3 paraL = Vec3::Normalize(Vec3(light.position[0] - position.x, light.position[1] - position.y, light.position[2] - position.z));
  Vec3 paraN = Vec3::Normalize(Vec3::AddThreeScale(Vec3(v[0].normal[0], v[0].normal[1], v[0].normal[2]), Vec3(v[1].normal[0], v[1].normal[1], v[1].normal[2]), 
    Vec3(v[2].normal[0], v[2].normal[1], v[2].normal[2]), scaleA, scaleB, scaleC));
  double mulLN = Vec3::DotProduct(paraL, paraN);
  Vec3 paraR = Vec3::Minus(Vec3::Multiply(paraN, mulLN * 2), paraL);
  Vec3 paraV = Vec3::Normalize(Vec3::Multiply(position, -1.0));
  double mulRV = Vec3::DotProduct(paraR, paraV);
  mulLN = mulLN < 0 ? 0 : mulLN;
  mulRV = mulRV < 0 ? 0 : mulRV;

  // 3. interploate Kd, ks, sh
  double paraKD[3], paraKS[3];
  for (int i = 0; i < 3; i++) {
    paraKD[i] = scaleA * v[0].color_diffuse[i] + scaleB * v[1].color_diffuse[i] + scaleC * v[2].color_diffuse[i];
    paraKS[i] = scaleA * v[0].color_specular[i] + scaleB * v[1].color_specular[i] + scaleC * v[2].color_specular[i];
  }
  double paraSH = scaleA * v[0].shininess + scaleB * v[1].shininess + scaleC * v[2].shininess;

  // 4. set color
  color.x += light.color[0] * ((paraKD[0] * mulLN + paraKS[0] * pow(mulRV, paraSH)));
  color.y += light.color[1] * ((paraKD[1] * mulLN + paraKS[1] * pow(mulRV, paraSH)));
  color.z += light.color[2] * ((paraKD[2] * mulLN + paraKS[2] * pow(mulRV, paraSH)));
}

double BoundColor(double color) {
  if (color > 1)
    color = 1.0;
  if (color < 0)
    color = 0.0;
  return color;
}

void ColorAmbient(Vec3& color) {
  color.x = BoundColor(color.x + ambient_light[0]);
  color.y = BoundColor(color.y + ambient_light[1]);
  color.z = BoundColor(color.z + ambient_light[2]);
}

void SetColor(unsigned int x, unsigned int y, Vec3 position, int mode, int num) {
  Vec3 color(0, 0, 0);
  for (int i = 0; i < num_lights; ++i) {
    if (!IsNotShadow(lights[i], position, mode, num)) {
      if (mode == SPHERE)
        ColorSphere(spheres[num], lights[i], position, color);
      else
        ColorTriangle(triangles[num], lights[i], position, color);
    } 
  }
  ColorAmbient(color);
  buffer[y][x][0] = (unsigned char)(color.x * 255);
  buffer[y][x][1] = (unsigned char)(color.y * 255);
  buffer[y][x][2] = (unsigned char)(color.z * 255);
  //plot_pixel(x, y, buffer[y][x][0], buffer[y][x][1], buffer[y][x][2]);
}

void Ray_Tracing() {
  // 1. Get window size
  double maxY = tan((fov * PI / 180) / 2);
  double maxX = (double)WIDTH / (double)HEIGHT * maxY;
  double posX = -maxX;

  // 2. Generate Ray for each pixel
  for(unsigned int x = 0; x < WIDTH; x++){
    double posY = -maxY;
    for(unsigned int y = 0; y < HEIGHT; y++){
      Vec3 origin(0, 0, 0); // ray
      Vec3 direction = Vec3::Normalize(Vec3(posX, posY, -1)); //ray
      Vec3 closest(0, 0, MIN_DEPTH);
      int mode, num;

      // 3. Find cloest intersection point
      for (int i = 0; i < num_spheres; i++) {
        Vec3 position = IntersectSphere(origin, direction, spheres[i]);
        if (position.z != MIN_DEPTH && position.z > closest.z) {
          closest = position;
          mode = SPHERE;
          num = i;
        }
      }
      for (int i = 0; i < num_triangles; i++) {
        Vec3 position = IntersectTriangle(origin, direction, triangles[i]);
        if (position.z != MIN_DEPTH && position.z > closest.z) {
          closest = position;
          mode = TRIANGLE;
          num = i; 
        }
      }

      // 4. Plot the image
      if (closest.z == MIN_DEPTH) { // background
        buffer[y][x][0] = BACKGROUND_R;
        buffer[y][x][1] = BACKGROUND_G;
        buffer[y][x][2] = BACKGROUND_B;
      }
      else  // cloest intersection point
        SetColor(x, y, closest, mode, num);
      posY += 2.0 * maxY / (double)(HEIGHT - 1);
    }
    posX += 2.0 * maxX / (double)(WIDTH - 1);
  }
}

void Print_Buffer() {
  cout << "BUFFER: (" << WIDTH << ", " << HEIGHT << ")" << endl;
  for(unsigned int x = 0; x < WIDTH; x++){
    for(unsigned int y = 0; y < HEIGHT; y++){
      cout << (int)buffer[y][x][0] << " " << (int)buffer[y][x][1] << " " << (int)buffer[y][x][2] << " ";
    }
    cout << endl;
  }
}

void draw_scene() {
  Ray_Tracing();
  //Print_Buffer();
  glBegin(GL_POINTS);
    glPointSize(2.0);
  for(unsigned int x = 0; x < WIDTH; x++){
    for(unsigned int y = 0; y < HEIGHT; y++){
      glColor3f(((float)buffer[y][x][0]) / 255.0f, ((float)buffer[y][x][1]) / 255.0f, ((float)buffer[y][x][2]) / 255.0f);
      glVertex2i(x,y);
    }
  }
  glEnd();
  glFlush();
}


/***********************************************/
/**************** DO NOT MODIFY ****************/
/***********************************************/
void plot_pixel_display(int x, int y, unsigned char r, unsigned char g, unsigned char b)
{
  glColor3f(((float)r) / 255.0f, ((float)g) / 255.0f, ((float)b) / 255.0f);
  glVertex2i(x,y);
}

void plot_pixel_jpeg(int x, int y, unsigned char r, unsigned char g, unsigned char b)
{
  buffer[y][x][0] = r;
  buffer[y][x][1] = g;
  buffer[y][x][2] = b;
}

void plot_pixel(int x, int y, unsigned char r, unsigned char g, unsigned char b)
{
  plot_pixel_display(x, y, r, g, b);
  if(mode == MODE_JPEG)
    plot_pixel_jpeg(x,y,r,g,b);
}

void save_jpg()
{
  printf("Saving JPEG file: %s\n", filename);

  ImageIO img(WIDTH, HEIGHT, 3, &buffer[0][0][0]);
  if (img.save(filename, ImageIO::FORMAT_JPEG) != ImageIO::OK)
    printf("Error in Saving\n");
  else 
    printf("File saved Successfully\n");
}

void parse_check(const char *expected, char *found)
{
  if(strcasecmp(expected,found))
  {
    printf("Expected '%s ' found '%s '\n", expected, found);
    printf("Parse error, abnormal abortion\n");
    exit(0);
  }
}

void parse_doubles(FILE* file, const char *check, double p[3])
{
  char str[100];
  fscanf(file,"%s",str);
  parse_check(check,str);
  fscanf(file,"%lf %lf %lf",&p[0],&p[1],&p[2]);
  printf("%s %lf %lf %lf\n",check,p[0],p[1],p[2]);
}

void parse_rad(FILE *file, double *r)
{
  char str[100];
  fscanf(file,"%s",str);
  parse_check("rad:",str);
  fscanf(file,"%lf",r);
  printf("rad: %f\n",*r);
}

void parse_shi(FILE *file, double *shi)
{
  char s[100];
  fscanf(file,"%s",s);
  parse_check("shi:",s);
  fscanf(file,"%lf",shi);
  printf("shi: %f\n",*shi);
}

int loadScene(char *argv)
{
  FILE * file = fopen(argv,"r");
  int number_of_objects;
  char type[50];
  Triangle t;
  Sphere s;
  Light l;
  fscanf(file,"%i", &number_of_objects);

  printf("number of objects: %i\n",number_of_objects);

  parse_doubles(file,"amb:",ambient_light);

  for(int i=0; i<number_of_objects; i++)
  {
    fscanf(file,"%s\n",type);
    printf("%s\n",type);
    if(strcasecmp(type,"triangle")==0)
    {
      printf("found triangle\n");
      for(int j=0;j < 3;j++)
      {
        parse_doubles(file,"pos:",t.v[j].position);
        parse_doubles(file,"nor:",t.v[j].normal);
        parse_doubles(file,"dif:",t.v[j].color_diffuse);
        parse_doubles(file,"spe:",t.v[j].color_specular);
        parse_shi(file,&t.v[j].shininess);
      }

      if(num_triangles == MAX_TRIANGLES)
      {
        printf("too many triangles, you should increase MAX_TRIANGLES!\n");
        exit(0);
      }
      triangles[num_triangles++] = t;
    }
    else if(strcasecmp(type,"sphere")==0)
    {
      printf("found sphere\n");

      parse_doubles(file,"pos:",s.position);
      parse_rad(file,&s.radius);
      parse_doubles(file,"dif:",s.color_diffuse);
      parse_doubles(file,"spe:",s.color_specular);
      parse_shi(file,&s.shininess);

      if(num_spheres == MAX_SPHERES)
      {
        printf("too many spheres, you should increase MAX_SPHERES!\n");
        exit(0);
      }
      spheres[num_spheres++] = s;
    }
    else if(strcasecmp(type,"light")==0)
    {
      printf("found light\n");
      parse_doubles(file,"pos:",l.position);
      parse_doubles(file,"col:",l.color);

      if(num_lights == MAX_LIGHTS)
      {
        printf("too many lights, you should increase MAX_LIGHTS!\n");
        exit(0);
      }
      lights[num_lights++] = l;
    }
    else
    {
      printf("unknown type in scene description:\n%s\n",type);
      exit(0);
    }
  }
  return 0;
}

void display()
{
}

void init()
{
  glMatrixMode(GL_PROJECTION);
  glOrtho(0,WIDTH,0,HEIGHT,1,-1);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glClearColor(0,0,0,0);
  glClear(GL_COLOR_BUFFER_BIT);
}

void idle()
{
  //hack to make it only draw once
  static int once=0;
  if(!once)
  {
    draw_scene();
    if(mode == MODE_JPEG)
      save_jpg();
  }
  once=1;
}

int main(int argc, char ** argv)
{
  if ((argc < 2) || (argc > 3))
  {  
    printf ("Usage: %s <input scenefile> [output jpegname]\n", argv[0]);
    exit(0);
  }
  if(argc == 3)
  {
    mode = MODE_JPEG;
    filename = argv[2];
  }
  else if(argc == 2)
    mode = MODE_DISPLAY;

  glutInit(&argc,argv);
  loadScene(argv[1]);

  glutInitDisplayMode(GLUT_RGBA | GLUT_SINGLE);
  glutInitWindowPosition(0,0);
  glutInitWindowSize(WIDTH,HEIGHT);
  int window = glutCreateWindow("Ray Tracer");
  glutDisplayFunc(display);
  glutIdleFunc(idle);
  init();
  glutMainLoop();
}

