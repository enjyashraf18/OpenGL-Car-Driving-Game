#define _USE_MATH_DEFINES
#include <cstdlib>
#include <cmath>
#include <iostream>

using namespace std;

#include <Gl\glew.h>
#include <Gl\freeglut.h>

static int width, height;

// car stuff (coordinates and angle)
static float angle = 0.0; 
static float xVal = 0, zVal = 0; 

static unsigned int car_display_list; // Display list for the car 


static int frameCount = 0; // counter of frames darwn

void keyInput(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 27:
		exit(0);
		break;
	default:
		break;
	}
}


void frameCounter(int value)
{
	if (value != 0) // No output the first time frameCounter() is called (from main()).
		cout << "FPS = " << frameCount << endl;
	frameCount = 0;
	glutTimerFunc(1000, frameCounter, 1);
}

void setup(void)
{

	car_display_list = glGenLists(1);
	glNewList(car_display_list, GL_COMPILE);

	// car body 
	glPushMatrix();
	glRotatef(180.0, 0.0, 1.0, 0.0); // car points down the z-axis initially
	glColor3f(0.0, 0.7, 0.7);
	glTranslatef(0.0, -12.5, 0.0);
	glScalef(2.5, 0.5, 1.0);
	glutSolidCube(6.0);
	glPopMatrix();

	// top of the car 
	glPushMatrix();
	glRotatef(180.0, 0.0, 1.0, 0.0);
	glColor3f(0.0, 0.8, 0.8); 
	glTranslatef(0.0, -10, 0.0);
	glScalef(2, 0.27, 1.0);
	glutSolidCube(5.0); 
	glPopMatrix();

	// Wheels
	glColor3f(0.2, 0.2, 0.2); 

	glPushMatrix();
	glRotatef(180.0, 0.0, 1.0, 0.0);
	glTranslatef(-4.0, -16, 2.5); // Front-left wheel
	glutSolidTorus(0.5, 1.0, 10, 10);
	glPopMatrix();

	glPushMatrix();
	glRotatef(180.0, 0.0, 1.0, 0.0);
	glTranslatef(4.0, -16, 2.5); // Front-right wheel
	glutSolidTorus(0.5, 1.0, 10, 10);
	glPopMatrix();

	glPushMatrix();
	glRotatef(180.0, 0.0, 1.0, 0.0);
	glTranslatef(-4.0, -16, -2.5); // Back-left wheel
	glutSolidTorus(0.5, 1.0, 10, 10);
	glPopMatrix();

	glPushMatrix();
	glRotatef(180.0, 0.0, 1.0, 0.0);
	glTranslatef(4.0, -16, -2.5); // Back-right wheel
	glutSolidTorus(0.5, 1.0, 10, 10);
	glPopMatrix();


	glEndList();

	glEnable(GL_DEPTH_TEST);
	glClearColor(0.0, 0.0, 0.0, 0.0);

	glutTimerFunc(0, frameCounter, 0); // Initial call of frameCounter().
}

void drawScene(void)
{
	frameCount++; // Increment number of frames every redraw.

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Begin left viewport.
	glViewport(0, 0, width / 2.0, height);
	glLoadIdentity();

	//// Write text in isolated (i.e., before gluLookAt) translate block.
	//glPushMatrix();
	//glColor3f(1.0, 0.0, 0.0);
	//glRasterPos3f(-28.0, 25.0, -30.0);
	//glPopMatrix();

	// Fixed camera.
	gluLookAt(0.0, 10.0, 20.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);

	// Draw car
	glPushMatrix();
	glTranslatef(xVal, 0.0, zVal);
	glRotatef(angle, 0.0, 1.0, 0.0);
	glCallList(car_display_list);
	glPopMatrix();
	// End left viewport.



	// Begin right viewport.
	glViewport(width / 2.0, 0, width / 2.0, height);
	glLoadIdentity();

	//// Write text in isolated (i.e., before gluLookAt) translate block.
	//glPushMatrix();
	//glColor3f(1.0, 0.0, 0.0);
	//glRasterPos3f(-28.0, 25.0, -30.0);
	//glPopMatrix();

	// Draw a vertical line in the middle to separate the two viewports
	glColor3f(1.0, 1.0, 1.0);
	glLineWidth(2.0);
	glBegin(GL_LINES);
	glVertex3f(-5.0, -5.0, -5.0);
	glVertex3f(-5.0, 5.0, -5.0);
	glEnd();
	glLineWidth(1.0);

	// Locate the camera at the tip of the car and pointing in the direction of the car.
	gluLookAt(xVal - 10 * sin((M_PI / 180.0) * angle),
		0.0,
		zVal - 10 * cos((M_PI / 180.0) * angle),
		xVal - 11 * sin((M_PI / 180.0) * angle),
		0.0,
		zVal - 11 * cos((M_PI / 180.0) * angle),
		0.0,
		1.0,
		0.0);
	// End right viewport.

	glutSwapBuffers();
}

// OpenGL window reshape 
void resize(int w, int h)
{
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glFrustum(-5.0, 5.0, -5.0, 5.0, 5.0, 250.0);
	glMatrixMode(GL_MODELVIEW);

	// Pass the size 
	width = w;
	height = h;
}
int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitContextVersion(4, 3);
	glutInitContextProfile(GLUT_COMPATIBILITY_PROFILE);

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowSize(800, 400);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("Car Driving Game");
	glutDisplayFunc(drawScene);
	glutReshapeFunc(resize);
	glutKeyboardFunc(keyInput);

	glewExperimental = GL_TRUE;
	glewInit();

	setup();

	glutMainLoop();
}