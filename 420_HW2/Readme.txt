Subject 	: CSCI420 - Computer Graphics 
Assignment 2: Simulating a Roller Coaster
Author		: Meiyi Yang

Description: In this assignment, we use Catmull-Rom splines along with OpenGL texture mapping to create a roller coaster simulation.

Core Credit Features: (Answer these Questions with Y/N; you can also insert comments as appropriate)
======================

1. Uses OpenGL core profile, version 3.2 or higher - [Y]

2. Completed all Levels:
  Level 1 : -[Y]
  level 2 : -[Y]
  Level 3 : -[Y]
  Level 4 : -[Y]
  Level 5 : -[Y]

3. Used Catmull-Rom Splines to render the Track - [Y]

4. Rendered a Rail Cross Section -[Y]

5. Rendered the camera at a reasonable speed in a continuous path/orientation -[Y]

6. Run at interactive frame rate (>15fps at 1280 x 720) -[Y]

7. Understandably written, well commented code -[Y]

8. Attached an Animation folder containing not more than 1000 screenshots -[Y] “/hw1-starterCode/save/

9. Attached this ReadMe File -[Y] “/Readme.txt”

Extra Credit Features: (Answer these Questions with Y/N; you can also insert comments as appropriate)
======================

1. Render a T-shaped rail cross section -[Y]

2. Render a Double Rail -[Y]

3. Made the track circular and closed it with C1 continuity -[Y]

4. Added OpenGl lighting - [N]

5. Any Additional Scene Elements? (list them here)
	Add a “Fight On TROJAN” cube at the beginning of the rail-coaster.

6. Generate track from several sequences of splines - [N]

7. Draw splines using recursive subdivision -[Y]

8. Modify velocity with which the camera moves -[Y]

9. Create tracks that mimic a real world coaster - [N]

10. Render environment in a better manner - [N]

Additional Features: (Please document any additional features you may have implemented other than the ones described above)
1. the Camera will waiting 4 seconds before it moves, and it will back to the beginning point after the rail is end.
2. the animation submitted used a new “.sp” file “rollerCoaster2.sp” writing by myself

Open-Ended Problems: (Please document approaches to any open-ended problems that you have tackled)
1.
2.

Keyboard/Mouse controls: (Please document Keyboard/Mouse controls if any)
1.
2.

Names of the .cpp files you made changes to:
1. hw2.cpp
2.

Comments : (If any)
// Compile and Run:
make clean
make
./hw2 track.txt

// Files added
hw2.h 
hw2.cpp
textures in heightmap/


// Parameter modification
all the parameters are setting in hw2.h
UpdateCameraMode = 0 : stop the camera update
speedCamera: set the speedCamera







