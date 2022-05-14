#include <GL/glut.h>
#include <cmath>
#include <iostream>
#include <vector>
#include <vecmath.h>
#include "mesh.h" 
#include "simplify.h" 

using namespace std;

// Globals
# define M_PI           3.14159265358979323846  /* pi */

// You will need more global variables to implement color and position changes
const int SPIN_TIME = 1; 
int colorId = 0, xCount = 0, yCount = 0; 
bool rotateCam = false, simplify = false;
float theta = 0;

Mesh mesh; 
GarlandHeckbert *algo; 

// This function is called whenever a "Normal" key press is received.
void keyboardFunc( unsigned char key, int x, int y )
{
  switch ( key )
  {
    case 27: // Escape key
      exit(0);
      break;
    case 'r': 
      rotateCam = !rotateCam;
      break;
    case 's': 
      simplify = !simplify; 
      break; 
    case 'c':
      // add code to change color here
      colorId = (colorId + 1) % 4;
      break;
    default:
      cout << "Unhandled key press " << key << "." << endl;        
  }

  // this will refresh the screen so that the user sees the color change
  glutPostRedisplay();
}

void timerFunc (int value) {
  if (rotateCam) 
    theta += M_PI / 20;
  if (simplify) 
    algo->simplifyStep(); 
  glutPostRedisplay();
  glutTimerFunc(SPIN_TIME, timerFunc, 0); 
}

// This function is called whenever a "Special" key press is received.
// Right now, it's handling the arrow keys.
void specialFunc( int key, int x, int y )
{
  switch ( key )
  {
    case GLUT_KEY_UP:
      // add code to change light position
      yCount++; 
      break;
    case GLUT_KEY_DOWN:
      // add code to change light position
      yCount--; 
      break;
    case GLUT_KEY_LEFT:
      // add code to change light position
      xCount--;
      break;
    case GLUT_KEY_RIGHT:
      // add code to change light position
      xCount++; 
      break;
  }

  // this will refresh the screen so that the user sees the light position
  glutPostRedisplay();
}

// This function is responsible for displaying the object.
void drawScene(void)
{
  int i;

  // Clear the rendering window
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Rotate the image
  glMatrixMode( GL_MODELVIEW );  // Current matrix affects objects positions
  glLoadIdentity();              // Initialize to the identity

  // Position the camera at [0,0,5], looking at [0,0,0],
  // with [0,1,0] as the up direction.
  gluLookAt(5.0 * sin(theta), 0.0, 5.0 * cos(theta), 
      0.0, 0.0, 0.0,
      0.0, 1.0, 0.0);

  // Set material properties of object

  // Here are some colors you might use - feel free to add more
  GLfloat diffColors[4][4] = { {0.5, 0.5, 0.9, 1.0},
    {0.9, 0.5, 0.5, 1.0},
    {0.5, 0.9, 0.3, 1.0},
    {0.3, 0.8, 0.9, 1.0} };

  // Select first color value according to current value of colorId.
  glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, diffColors[colorId]);

  // Define specular color and shininess
  GLfloat specColor[] = {1.0, 1.0, 1.0, 1.0};
  GLfloat shininess[] = {100.0};

  // Note that the specular color and shininess can stay constant
  glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specColor);
  glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, shininess);

  // Set light properties

  // Light color (RGBA)
  GLfloat Lt0diff[] = {1.0,1.0,1.0,1.0};
  // Light position
  GLfloat Lt0pos[] = {1.0f + 0.5f * xCount, 1.0f + 0.5f * yCount, 5.0f, 1.0f};

  glLightfv(GL_LIGHT0, GL_DIFFUSE, Lt0diff);
  glLightfv(GL_LIGHT0, GL_POSITION, Lt0pos);

  // This GLUT method draws a teapot.  You should replace
  // it with code which draws the object you loaded.
  mesh.draw();

  // Dump the image to the screen.
  glutSwapBuffers();


}

// Initialize OpenGL's rendering modes
void initRendering()
{
  glEnable(GL_DEPTH_TEST);   // Depth testing must be turned on
  glEnable(GL_LIGHTING);     // Enable lighting calculations
  glEnable(GL_LIGHT0);       // Turn on light #0.
}

// Called when the window is resized
// w, h - width and height of the window in pixels.
void reshapeFunc(int w, int h)
{
  // Always use the largest square viewport possible
  if (w > h) {
    glViewport((w - h) / 2, 0, h, h);
  } else {
    glViewport(0, (h - w) / 2, w, w);
  }

  // Set up a perspective view, with square aspect ratio
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  // 50 degree fov, uniform aspect ratio, near = 1, far = 100
  gluPerspective(50.0, 1.0, 1.0, 100.0);
}

// Main routine.
// Set up OpenGL, define the callbacks and start the main loop
int main( int argc, char** argv )
{
  mesh.read(); 
  algo = new GarlandHeckbert(mesh); 
  glutInit(&argc,argv);

  // We're going to animate it, so double buffer 
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH );

  // Initial parameters for window position and size
  glutInitWindowPosition( 60, 60 );
  glutInitWindowSize( 360, 360 );
  glutCreateWindow("Simplify");

  // Initialize OpenGL parameters.
  initRendering();

  // Set up callback functions for key presses
  glutKeyboardFunc(keyboardFunc); // Handles "normal" ascii symbols
  glutSpecialFunc(specialFunc);   // Handles "special" keyboard keys
  glutTimerFunc(SPIN_TIME, timerFunc, 0); // Called every 20 ms. 

  // Set up the callback function for resizing windows
  glutReshapeFunc( reshapeFunc );

  // Call this whenever window needs redrawing
  glutDisplayFunc( drawScene );

  // Start the main loop.  glutMainLoop never returns.
  glutMainLoop( );
  return 0;	// This line is never reached.
}
