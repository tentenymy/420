
  CSCI 420 Computer Graphics, USC
  Assignment 1: Height Fields
  C++ starter code
  
  Student username: meiyiyan@usc.edu
  Name: Meiyi Yang


*****************************
****** COMPILE AND RUN ******
*****************************

> unzip assign1_coreOpenGL_starterCode.zip
> cd hw1-starterCode
> make
> ./hw1 heightmap/OhioPyle-512.jpg


***********************
****** ANIMATION ******
***********************

JPEG frames are in the folder:
> cd assign1_coreOpenGL_starterCode/hw1-starterCode/save/

This animation is based on a input color image ‘sea512.jpg’


*********************
****** DEFALUT ******
*********************
Case 1(Default):

Image size: maximum is 768 * 768
Image type: color or grayscale
Render type: Points or Lines or Triangles(default)
	(Set renderMode to modify:
	Points: renderMode = 0; Lines: renderMode = 1; Triangles: renderMode = 2;)
Scene: rotating
Result color: gradient from black to yellow depends on its depth.
Setting:
	Set hw1.cpp (default):
		int renderMode = 2;
		int colorMode = 0;	
		int saveMode = 0; 
	Set basic.vertexShader.glsl (default): 
		int mode = 0; 


**************************
****** EXTRA CREDIT ******
**************************
1. Support color (ImageIO::getBytesPerPixel == 3) in input images
Case 1:Default case can support color image
Case 2: Render color image and colored it by itself color
Image type: color
Image size: maximum 768 * 768
Render type: Point / Line / Triangle
Result color: color of the image
Setting:
	Set hw1.cpp:
		int renderMode = 2; 
		int colorMode = 1;
		int saveMode = 0;
	Set basic.vertexShader.glsl: 
		int mode = 1; 


2. Color the vertices based on color values taken from another image of equal size. 
Case 3: 
Image type: grayscale or color
Image size: 128 * 128, 256 * 256, 512 * 512, 768 * 768
Render type: Point / Line / Triangle
Result color: color of another image (e.g.:USC512.jpg)
Setting:
	Set hw1.cpp:
		int renderMode = 2; 
		int colorMode = 2;
		int saveMode = 0;
	Set basic.vertexShader.glsl: 
		int mode = 1; 

