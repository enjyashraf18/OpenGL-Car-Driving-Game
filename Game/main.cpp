#define _USE_MATH_DEFINES

#include <cstdlib>
#include <cmath>
#include <iostream>

#include <GL/glew.h>
#include <GL/freeglut.h> 

#define ROWS 2  // rows of cones.
#define COLUMNS 20 //columns of cones.
#define FILL_PROBABILITY 50 // Percentage probability that a particular row-column slot will be 
							 // filled with an cones. It should be an integer between 0 and 100.

// Globals.
static long font = (long)GLUT_BITMAP_8_BY_13; 
static int width, height;

// car stuff (coordinates and angle)
static float angle = 0.0;
static float xVal = 0, zVal = 0;
static unsigned int car_display_list; // Display list for the car 


static int isCollision = 0; // Is there collision between the spacecraft and an cones?
static unsigned int spacecraft; // Display lists base index.
static int frameCount = 0; // Number of frames
// Gate Position
static float gateX = 0.0f, gateY = 0.0f, gateZ = -250.0f; // Fixed position of the gate
static float gateLength = 25.0f;
static float gateHeight = 30.0f;
static float gateDepth = 10.0f; // Depth of the gate

// Routine to draw a bitmap character string.
void writeBitmapString(void* font, char* string)
{
	char* c;

	for (c = string; *c != '\0'; c++) glutBitmapCharacter(font, *c);
}

class Cone
{
public:
	Cone();
	Cone(float x, float y, float z, float r, unsigned char colorR,
		unsigned char colorG, unsigned char colorB);
	float getCenterX() { return centerX; }
	float getCenterY() { return centerY; }
	float getCenterZ() { return centerZ; }
	float getRadius() { return radius; }
	void draw();

private:
	float centerX, centerY, centerZ, radius;
	unsigned char color[3];
};

Cone::Cone()
{
	centerX = 0.0;
	centerY = 0.0;
	centerZ = 0.0;
	radius = 0.0; 
	color[0] = 0;
	color[1] = 0;
	color[2] = 0;
}

// cone constructor.
Cone::Cone(float x, float y, float z, float r, unsigned char colorR,
	unsigned char colorG, unsigned char colorB)
{
	centerX = x;
	centerY = y;
	centerZ = z;
	radius = r;
	color[0] = colorR;
	color[1] = colorG;
	color[2] = colorB;
}

// Function to draw cone.
void Cone::draw()
{
	if (radius > 0.0) // If asteroid exists.
	{
		glPushMatrix();
		glTranslatef(centerX, centerY, centerZ);
		glColor3ub(255, 165, 0);
		glRotatef(-90.0, 1.0, 0.0, 0.0);
		float coneBase = radius ; 
		float coneHeight = radius * 2.0; 
		glutWireCone(coneBase, coneHeight, 100, 100);
		glPopMatrix();
	}
}

Cone arrayCones[ROWS][COLUMNS]; // Global array of cones.


// Function to check if the car is close enough to the gate
int checkWinCondition(float carX, float carY, float carZ) {
	// Calculate the distance from the car to the gate
	float distance = sqrt((carX - gateX) * (carX - gateX) +
		(carY - gateY) * (carY - gateY) +
		(carZ - gateZ) * (carZ - gateZ));

	// If the xar is within the gate's radius, it's a win
	if (distance < gateDepth) {
		return 1; // Win condition met
	}
	return 0; // No win
}


// Routine to count the number of frames drawn every second.
void frameCounter(int value)
{
	if (value != 0) // No output the first time frameCounter() is called (from main()).
		std::cout << "FPS = " << frameCount << std::endl;
	frameCount = 0;
	glutTimerFunc(1000, frameCounter, 1);
}

// Initialization routine.
void setup(void)
{
	int i, j;

	car_display_list = glGenLists(1);
	glNewList(car_display_list, GL_COMPILE);

	glTranslatef(0.0, 0.0, 5.0);
	// car body 
	glPushMatrix();
	glRotatef(90.0, 0.0, 1.0, 0.0); // car points down the z-axis initially
	glColor3f(0.0, 0.7, 0.7);
	glTranslatef(0.0, -12.5, 0.0);
	glScalef(2.5, 0.5, 1.0);
	glutSolidCube(6.0);
	glPopMatrix();

	// top of the car 
	glPushMatrix();
	glRotatef(90.0, 0.0, 1.0, 0.0);
	glColor3f(0.0, 0.8, 0.8);
	glTranslatef(0.0, -10, 0.0);
	glScalef(2, 0.27, 1.0);
	glutSolidCube(5.0);
	glPopMatrix();

	// Wheels
	glColor3f(0.2, 0.2, 0.2);

	glPushMatrix();
	glRotatef(90.0, 0.0, 1.0, 0.0);
	glTranslatef(-4.0, -16, 2.5); // Front-left wheel
	glutSolidTorus(0.5, 1.0, 10, 10);
	glPopMatrix();

	glPushMatrix();
	glRotatef(90.0, 0.0, 1.0, 0.0);
	glTranslatef(4.0, -16, 2.5); // Front-right wheel
	glutSolidTorus(0.5, 1.0, 10, 10);
	glPopMatrix();

	glPushMatrix();
	glRotatef(90.0, 0.0, 1.0, 0.0);
	glTranslatef(-4.0, -16, -2.5); // Back-left wheel
	glutSolidTorus(0.5, 1.0, 10, 10);
	glPopMatrix();

	glPushMatrix();
	glRotatef(90.0, 0.0, 1.0, 0.0);
	glTranslatef(4.0, -16, -2.5); // Back-right wheel
	glutSolidTorus(0.5, 1.0, 10, 10);
	glPopMatrix();


	glEndList();


	// Initialize global CONE ARRAY .
	for (j = 0; j < COLUMNS; j++) {
		for (i = 0; i < ROWS; i++) {
			if (rand() % 100 < FILL_PROBABILITY) {
				// Generate random positions
				float x = -30.0f + static_cast<float>(rand() % 61);  // Range: [-30, 30]
				float y = 0.0f; // Constant height for all obstacles
				float z = -190.0f + static_cast<float>(rand() % 191); 

				// Create a cone (obstacle)
				arrayCones[i][j] = Cone(
					x, y, z,                        // Position
					2.5f,                           // Radius
					rand() % 256,                   
					rand() % 256,                   
					rand() % 256                    
				);
			}
		}
	}

	glEnable(GL_DEPTH_TEST);
	glClearColor(0.0, 0.0, 0.0, 0.0);

	glutTimerFunc(0, frameCounter, 0); // Initial call of frameCounter().
}

// Function to check if two spheres centered at (x1,y1,z1) and (x2,y2,z2) with
// radius r1 and r2 intersect.
int checkSpheresIntersection(float x1, float y1, float z1, float r1,
	float x2, float y2, float z2, float r2)
{
	return ((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2) + (z1 - z2) * (z1 - z2) <= (r1 + r2) * (r1 + r2));
}

// Function to check if the spacecraft collides with an asteroid when the center of the base
// of the craft is at (x, 0, z) and it is aligned at an angle a to to the -z direction.
// Collision detection is approximate as instead of the spacecraft we use a bounding sphere.
int ConeCarCollision(float x, float z, float a)
{
	int i, j;

	// Check for collision with each cone.
	for (j = 0; j < COLUMNS; j++)
		for (i = 0; i < ROWS; i++)
			if (arrayCones[i][j].getRadius() > 0) // If  exists.
				if (checkSpheresIntersection(x - 5 * sin((M_PI / 180.0) * a), 0.0,
					z - 5 * cos((M_PI / 180.0) * a), 7.072,
					arrayCones[i][j].getCenterX(), arrayCones[i][j].getCenterY(),
					arrayCones[i][j].getCenterZ(), arrayCones[i][j].getRadius()))
					return 1;
	return 0;
}

// Drawing routine.
void drawScene(void)
{
	frameCount++; // Increment number of frames every redraw.

	int i, j;
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Begin left viewport.
	glViewport(0, 0, width / 2.0, height);
	glLoadIdentity();	

	// Write text in isolated (i.e., before gluLookAt) translate block.
	glPushMatrix();
	glColor3f(1.0, 0.0, 0.0);
	glRasterPos3f(-28.0, 25.0, -30.0);
	if (isCollision) writeBitmapString((void*)GLUT_BITMAP_TIMES_ROMAN_24, (char*)"collision");
	if (checkWinCondition(xVal, 0.0f, zVal)) {
		printf("hi");
		writeBitmapString((void*)GLUT_BITMAP_TIMES_ROMAN_24, (char*)"You won!");


	}

	glPopMatrix();

	// Fixed camera.
	gluLookAt(0.0, 10.0, 20.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);


	// Road
	glPushMatrix();
	glColor3f(0.1f, 0.1f, 0.1f); // Dark gray color for the street.
	glBegin(GL_QUADS);
	glVertex3f(-40.0f, -5.f, 50.0f);  // Bottom-left corner.
	glVertex3f(40.0f, -5.0f, 50.0f);   // Bottom-right corner.
	glVertex3f(40.0f, -5.0f, gateZ); // Top-right corner.
	glVertex3f(-40.0f, -5.0f, gateZ); // Top-left corner.
	glEnd();

	// Add road lines in the middle
	glLineWidth(5.0f);
	glColor3f(1.0f, 1.0f, 1.0f); // White color for the road lines
	// Right line
	glBegin(GL_LINES);
	glVertex3f(5.0f, -5.01f, 50.0f); // Starting point of the second line.
	glVertex3f(5.0f, -5.01f, gateZ); // Ending point of the second line.
	glEnd();

	// Left line
	glBegin(GL_LINES);
	glVertex3f(-5.0f, -5.01f, 50.0f);  // Starting point of the first line.
	glVertex3f(-5.0f, -5.01f, gateZ);  // Ending point of the first line.
	glEnd();

	glPopMatrix();



	// Draw all the cones.
	for (j = 0; j < COLUMNS; j++)
		for (i = 0; i < ROWS; i++)
			arrayCones[i][j].draw();

	// Draw car
	glPushMatrix();
	glTranslatef(xVal, 15.0, zVal);
	glRotatef(angle, 0.0, 1.0, 0.0);
	glCallList(car_display_list);
	glPopMatrix();
	//gate
	glPushMatrix();
	glTranslatef(gateX, 0.0, gateZ);
	glColor3f(1.0f, 1.0f, 1.0f);
	glScalef(gateLength, gateHeight, gateDepth);
	glutSolidCube(1.0f);
	glPopMatrix();
	// End left viewport.

	// Begin right viewport.
	glViewport(width / 2.0, 0, width / 2.0, height);
	glLoadIdentity();

	glPushMatrix();
	glColor3f(1.0, 0.0, 0.0);
	glRasterPos3f(-28.0, 25.0, -30.0);
	if (isCollision) writeBitmapString((void*)GLUT_BITMAP_TIMES_ROMAN_24, (char*)"collision");
	if (checkWinCondition(xVal, 0.0f, zVal)) {
		printf("hi");
		writeBitmapString((void*)GLUT_BITMAP_TIMES_ROMAN_24, (char*)"You won!");


	}

	glPopMatrix();

	// Draw a vertical line on the left of the viewport to separate the two viewports
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

	glPushMatrix();
	glColor3f(0.2f, 0.2f, 0.2f); // Dark gray color for the street.
	glBegin(GL_QUADS);
	//road
	glVertex3f(-40.0f, -0.1f, 50.0f);  // Bottom-left corner.
	glVertex3f(40.0f, -0.1f, 50.0f);   // Bottom-right corner.
	glVertex3f(40.0f, -0.1f, -300.0f); // Top-right corner.
	glVertex3f(-40.0f, -0.1f, -300.0f); // Top-left corner.
	glEnd();
	glPopMatrix();

	// Draw all the cones.
	for (j = 0; j < COLUMNS; j++)
		for (i = 0; i < ROWS; i++)
			arrayCones[i][j].draw();
	// gate
	glPushMatrix();
	glTranslatef(gateX, 15.0, gateZ);
	glColor3f(1.0f, 1.0f, 1.0f);
	glScalef(gateLength, gateHeight, gateDepth);
	glutSolidCube(1.0f);
	glPopMatrix();
	// End right viewport.

	glutSwapBuffers();
}

// OpenGL window reshape routine.
void resize(int w, int h)
{
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glFrustum(-5.0, 5.0, -5.0, 5.0, 5.0, 250.0);
	glMatrixMode(GL_MODELVIEW);

	// Pass the size of the OpenGL window.
	width = w;
	height = h;
}

// Keyboard input processing routine.
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

// Callback routine for non-ASCII key entry.
void specialKeyInput(int key, int x, int y)
{
	float tempxVal = xVal, tempzVal = zVal, tempAngle = angle;

	// Compute next position.
	if (key == GLUT_KEY_LEFT) tempAngle = angle + 5.0;
	if (key == GLUT_KEY_RIGHT) tempAngle = angle - 5.0;
	if (key == GLUT_KEY_UP)
	{
		tempxVal = xVal - sin(angle * M_PI / 180.0);
		tempzVal = zVal - cos(angle * M_PI / 180.0);
	}
	if (key == GLUT_KEY_DOWN)
	{
		tempxVal = xVal + sin(angle * M_PI / 180.0);
		tempzVal = zVal + cos(angle * M_PI / 180.0);
	}

	// Angle correction.
	if (tempAngle > 360.0) tempAngle -= 360.0;
	if (tempAngle < 0.0) tempAngle += 360.0;

	// Move spacecraft to next position only if there will not be collision with an asteroid.
	if (!ConeCarCollision(tempxVal, tempzVal, tempAngle))
	{
		isCollision = 0;
		xVal = tempxVal;
		zVal = tempzVal;
		angle = tempAngle;
	}
	else isCollision = 1;

	


	glutPostRedisplay();
}

// Routine to output interaction instructions to the C++ window.
void printInteraction(void)
{
	std::cout << "Interaction:" << std::endl;
	std::cout << "Press the left/right arrow keys to turn the craft." << std::endl
		<< "Press the up/down arrow keys to move the craft." << std::endl;
}

// Main routine.
int main(int argc, char** argv)
{
	printInteraction();
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
	glutSpecialFunc(specialKeyInput);

	glewExperimental = GL_TRUE;
	glewInit();

	setup();

	glutMainLoop();
}
