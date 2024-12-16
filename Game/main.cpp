#define _USE_MATH_DEFINES

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <glm.hpp>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include "getBMP.h"
#define ROWS 2				// rows of cones.
#define COLUMNS 20			// columns of cones.
#define FILL_PROBABILITY 10 // Percentage probability that a particular row-column slot will be
// filled with cones. It should be an integer between 0 and 100.

// Globals.
static unsigned int texture[3]; // Array of texture indices.
static unsigned int current_ground_texture;
// static long font = (long)GLUT_BITMAP_8_BY_13;
static int width, height;
static float skyAngle = 0.0;	  // Angle of rotation of sky coordinates.
static int animationPeriod = 100; // Time interval between frames.

// car stuff (coordinates and angle)
// static int width, height;
static float angle = 0.0;
static float xVal = 0, zVal = 0;
static unsigned int car_display_list; // Display list for the car

static int displayMessage = 0;		  // 0: No message, 1: "You lose!", 2: "You won!"
static float messageStartTime = 0.0f; // Time when the message starts being displayed.

static int isCollision = 0;		// Is there collision between the spacecraft and an cones?
static unsigned int spacecraft; // Display lists base index.
static int frameCount = 0;		// Number of frames
// Gate Position
static float gateX = 0.0f, gateY = 0.0f, gateZ = -250.0f; // Fixed position of the gate
static float gateLength = 25.0f;
static float gateHeight = 30.0f;
static float gateDepth = 10.0f; // Depth of the gate

// Flags for key states
bool moveForwardFlag = false;
bool moveBackwardFlag = false;
bool moveRightFlag = false;
bool moveLeftFlag = false;

float speed = 1.;

// color  variables declaration
float lightPos1[] = {2.5, 3.0, 15.0, 1.0}; // Spotlight position in 3D space
float lightPos2[] = {-2, 3.0, 15.0, 1.0};  // Spotlight position in 3D space

static float spotAngle = 10.0;											  // Spotlight cone half-angle.
float spotDirection[] = {0.0, 0.0, -1.0};								  // Spotlight direction.
static float spotExponent = 2.0;										  // Spotlight attenuation exponent.
static float xMove = 0.0, zMove = 0.0;									  // Movement components.
static char theStringBuffer[10];										  // String buffer (A buffer for converting float values to strings)
static uintptr_t font = reinterpret_cast<uintptr_t>(GLUT_BITMAP_8_BY_13); // Holds the font type used for bitmap text rendering.

// Timer function. sky animation
void animate(int value)
{
	skyAngle += 0.01;
	if (skyAngle > 360.0)
		skyAngle -= 360.0;
	glutPostRedisplay();
	glutTimerFunc(animationPeriod, animate, 1);
}

// Load external textures.
void loadTextures()
{
	// Local storage for bmp image data.
	imageFile *image[3];

	// Load the images.
	image[0] = getBMP("grass.bmp");
	image[1] = getBMP("sky.bmp");
	image[2] = getBMP("desert_texture.bmp");

	// Bind grass image to texture object texture[0].
	glBindTexture(GL_TEXTURE_2D, texture[0]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image[0]->width, image[0]->height, 0,
				 GL_RGBA, GL_UNSIGNED_BYTE, image[0]->data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // Use mipmap linear filtering
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);				// Use linear filtering for magnification

	// Generate mipmaps for the grass texture.
	glGenerateMipmap(GL_TEXTURE_2D);

	// Bind sky image to texture object texture[1].
	glBindTexture(GL_TEXTURE_2D, texture[1]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image[1]->width, image[1]->height, 0,
				 GL_RGBA, GL_UNSIGNED_BYTE, image[1]->data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // Use mipmap linear filtering
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);				// Use linear filtering for magnification

	// Generate mipmaps for the ground texture.
	glGenerateMipmap(GL_TEXTURE_2D);

	// Bind ground image to texture object texture[2].
	glBindTexture(GL_TEXTURE_2D, texture[2]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image[2]->width, image[2]->height, 0,
				 GL_RGBA, GL_UNSIGNED_BYTE, image[2]->data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // Use mipmap linear filtering
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);				// Use linear filtering for magnification

	// Generate mipmaps for the ground texture.
	glGenerateMipmap(GL_TEXTURE_2D);
}

// A function that writes a string to the screen using bitmap fonts.
void writeBitmapString(void *font, char *string)
{
	char *c;
	for (c = string; *c != '\0'; c++)
		glutBitmapCharacter(font, *c);
}

// Converts a floating-point number to a string with a specified precision (4 decimal places) and stores it in destStr
void floatToString(char *destStr, int precision, float val)
{
	sprintf_s(destStr, 10, "%.4f", val);
}

// Cones class and drawing function
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
        float coneBase = radius;
        float coneHeight = radius * 2.0;
        glutWireCone(coneBase, coneHeight, 100, 100);
        glPopMatrix();
    }
}

Cone arrayCones[ROWS][COLUMNS]; // Global array of cones

// Function to check if the car is close enough to the gate
int checkWinCondition(float carX, float carY, float carZ)
{
	// Calculate the distance from the car to the gate
	float distance = sqrt((carX - gateX) * (carX - gateX) +
						  (carY - gateY) * (carY - gateY) +
						  (carZ - gateZ) * (carZ - gateZ));

	// If the xar is within the gate's radius, it's a win
	if (distance < gateDepth)
	{
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

// The top menu callback function.
void top_menu(int id)
{
	if (id == 1)
		current_ground_texture = texture[0];
	if (id == 2)
		current_ground_texture = texture[2];
	if (id == 3)
	{
		exit(0);
	}
	glutPostRedisplay();
}

// Routine to make the menu.
void makeMenu(void)
{
	// The top menu is created: its callback function is registered and menu entries,
	// including a submenu, added.
	glutCreateMenu(top_menu);
	glutAddMenuEntry("Grass", 1);
	glutAddMenuEntry("Desert", 2);
	glutAddMenuEntry("Quit", 3);

	// The menu is attached to a mouse button.
	glutAttachMenu(GLUT_RIGHT_BUTTON);
}
void resetGame()
{
	// Reset car position and angle.
	xVal = 0.0f;
	zVal = 0.0f;
	angle = 0.0f;

	// Reset collision state and messages.
	isCollision = 0;
	displayMessage = 0;
	messageStartTime = 0.0f; // Set start time

	// Reset cones.
	for (int j = 0; j < COLUMNS; j++)
	{
		for (int i = 0; i < ROWS; i++)
		{
			if (rand() % 100 < FILL_PROBABILITY)
			{
				float x = -30.0f + static_cast<float>(rand() % 61); // Range: [-30, 30]
				float y = -4.4;
				float z = -190.0f + static_cast<float>(rand() % 191);

				arrayCones[i][j] = Cone(x, y, z, 2.5f, rand() % 256, rand() % 256, rand() % 256);
			}
			else
			{
				arrayCones[i][j] = Cone(); // Empty cone slot.
			}
		}
	}
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
	for (j = 0; j < COLUMNS; j++)
	{
		for (i = 0; i < ROWS; i++)
		{
			if (rand() % 100 < FILL_PROBABILITY)
			{
				// Generate random positions
				float x = -30.0f + static_cast<float>(rand() % 61); // Range: [-30, 30]
				float y = -4.4;										// Constant height for all obstacles
				float z = -190.0f + static_cast<float>(rand() % 191);

				// Create a cone (obstacle)
				arrayCones[i][j] = Cone(
					x, y, z, // Position
					2.5f,	 // Radius
					rand() % 256,
					rand() % 256,
					rand() % 256);
			}
		}
	}

	glClearColor(0.0, 0.0, 0.0, 0.0); // Clear background color
	glEnable(GL_DEPTH_TEST);		  // Enable depth testing.
	glEnable(GL_LIGHTING);			  // Turn on OpenGL lighting.

	glEnable(GL_LIGHT0);

	GLfloat light_ambient[] = {0.4f, 0.4f, 0.4f, 1.0f};	 // Dim ambient light
	GLfloat light_diffuse[] = {1.0f, 0.8f, 0.5f, 1.0f};	 // Orange-reddish diffuse light
	GLfloat light_specular[] = {1.0f, 0.9f, 0.7f, 1.0f}; // Subtle specular highlight
	// Position/direction vector (w=0 means directional light)
	GLfloat light_position[] = {-1.0f, 0.5f, -1.0f, 0.0f};

	glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);

	glEnable(GL_NORMALIZE);

	// Light property vectors.
	float lightAmb[] = {0.0, 0.0, 0.0, 1.0};		// Ambient Lighting "black"
	float lightDifAndSpec[] = {1.0, 1.0, 1.0, 1.0}; // Diffuse and Specular Lighting "white"
	float globAmb[] = {0.05, 0.05, 0.05, 1.0};		// Global Ambient Lighting "grey shade"

	// Light properties.
	glLightfv(GL_LIGHT1, GL_AMBIENT, lightAmb);			// set ambient light
	glLightfv(GL_LIGHT1, GL_DIFFUSE, lightDifAndSpec);	// set diffuse light
	glLightfv(GL_LIGHT1, GL_SPECULAR, lightDifAndSpec); // set specular light

	// Light properties.
	glLightfv(GL_LIGHT2, GL_AMBIENT, lightAmb);			// set ambient light
	glLightfv(GL_LIGHT2, GL_DIFFUSE, lightDifAndSpec);	// set diffuse light
	glLightfv(GL_LIGHT2, GL_SPECULAR, lightDifAndSpec); // set specular light

	glEnable(GL_LIGHT1); // Enable particular light source.
	glEnable(GL_LIGHT2); // Enable particular light source.

	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, globAmb);	 // set global ambient light.
	glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE); // Enable local viewpoint.

	// Material property vectors.
	float matSpec[] = {1.0, 1.0, 1.0, 1.0}; // Material Specular Reflection (white)
	float matShine[] = {50.0};				// Material shineness

	// Material properties shared by all the spheres.
	glMaterialfv(GL_FRONT, GL_SPECULAR, matSpec);	// set material specular reflection
	glMaterialfv(GL_FRONT, GL_SHININESS, matShine); // set material shineness

	glEnable(GL_COLOR_MATERIAL);					   // Enable color material mode
	glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE); // ambient & diffuse color of front faces will track color set by glColor().
	// Create texture ids.
	glGenTextures(3, texture);

	// Load external textures.
	loadTextures();
	current_ground_texture = texture[0];

	// Specify how texture values combine with current surface color values.
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

	// Enable textures
	glEnable(GL_TEXTURE_2D);

	// Set texture environment mode to GL_MODULATE (combine texture with color)
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glutTimerFunc(0, frameCounter, 0); // Initial call of frameCounter().
	animate(1);
	// Make menu.
	makeMenu();
}

void setCarMaterial()
{
	GLfloat mat_ambient[] = {0.2f, 0.3f, 0.3f, 1.0f};
	GLfloat mat_diffuse[] = {0.0f, 0.8f, 0.8f, 1.0f};
	GLfloat mat_specular[] = {0.8f, 0.8f, 0.8f, 1.0f};
	GLfloat mat_shininess[] = {50.0f};

	glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
	glMaterialf(GL_FRONT, GL_SHININESS, mat_shininess[0]);
}

void setRoadMaterial()
{
	GLfloat mat_ambient[] = {0.2f, 0.2f, 0.2f, 1.0f};
	GLfloat mat_diffuse[] = {0.3f, 0.3f, 0.3f, 1.0f};
	GLfloat mat_specular[] = {0.2f, 0.2f, 0.2f, 1.0f};
	GLfloat mat_shininess[] = {10.0f};

	glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
	glMaterialf(GL_FRONT, GL_SHININESS, mat_shininess[0]);
}

void setConeMaterial()
{
	GLfloat mat_ambient[] = {0.4f, 0.2f, 0.0f, 1.0f};
	GLfloat mat_diffuse[] = {1.0f, 0.6f, 0.0f, 1.0f};
	GLfloat mat_specular[] = {0.5f, 0.5f, 0.5f, 1.0f};
	GLfloat mat_shininess[] = {20.0f};

	glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
	glMaterialf(GL_FRONT, GL_SHININESS, mat_shininess[0]);
}

// Gate material (white metallic)
void setGateMaterial()
{
	GLfloat mat_ambient[] = {0.4f, 0.4f, 0.4f, 1.0f};
	GLfloat mat_diffuse[] = {1.0f, 1.0f, 1.0f, 1.0f};
	GLfloat mat_specular[] = {1.0f, 1.0f, 1.0f, 1.0f};
	GLfloat mat_shininess[] = {60.0f};

	glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
	glMaterialf(GL_FRONT, GL_SHININESS, mat_shininess[0]);
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

// Camera movement functions (based on the flags)
void updateMovement()
{
    float tempxVal = xVal, tempzVal = zVal, tempAngle = angle;

	if (moveForwardFlag)
	{
		// std::cout << "f " << std::endl;
		tempxVal = tempxVal - sin(angle * M_PI / 180.0);
		tempzVal = tempzVal - cos(angle * M_PI / 180.0);
	}

	if (moveBackwardFlag)
	{
		// std::cout << "b " << std::endl;
		tempxVal = tempxVal + sin(angle * M_PI / 180.0);
		tempzVal = tempzVal + cos(angle * M_PI / 180.0);
	}

	if (moveRightFlag)
	{
		// std::cout << "r " << std::endl;
		tempAngle = tempAngle - 5.0;
	}

	if (moveLeftFlag)
	{
		// std::cout << "l " << std::endl;
		tempAngle = tempAngle + 5.0;
	}
	// Angle correction.
	if (tempAngle > 360.0)
		tempAngle -= 360.0;
	if (tempAngle < 0.0)
		tempAngle += 360.0;

	// Move spacecraft to next position only if there will not be collision with an asteroid.
	if (!ConeCarCollision(tempxVal, tempzVal, tempAngle))
	{
		isCollision = 0;
		xVal = tempxVal;
		zVal = tempzVal;
		angle = tempAngle;
	}
	else
		isCollision = 1;

	// glutPostRedisplay();
}

void gameTimer(int value)
{
    float currentTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;

	// Automatically reset the game after the message is displayed for 3 seconds
	if (displayMessage > 0)
	{
		float elapsedTime = currentTime - messageStartTime;
		if (elapsedTime >= 2.0f)
		{
			resetGame();		// Reset the game state
			displayMessage = 0; // Clear the message
			glutPostRedisplay();
		}
	}

    // Continue to set the timer callback
    glutTimerFunc(0, gameTimer, 0);
}

void drawScene()
{
    frameCount++; // Increment number of frames every redraw.
    updateMovement();
    int i, j;
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Begin left viewport.
    glViewport(0, 0, width / 2.0, height);
    glLoadIdentity();

    // Get the current time in seconds since the program started
    float currentTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;

	// Write text in isolated (i.e., before gluLookAt) translate block.
	glPushMatrix();
	glDisable(GL_LIGHTING);
	glColor3f(1.0, 1.0, 1.0);
	glRasterPos3f(-28.0, 25.0, -30.0);

	// Display messages
	if (displayMessage > 0)
	{
		if (displayMessage == 1)
		{
			writeBitmapString((void *)GLUT_BITMAP_TIMES_ROMAN_24, (char *)"You lose!");
		}
		else if (displayMessage == 2)
		{
			writeBitmapString((void *)GLUT_BITMAP_TIMES_ROMAN_24, (char *)"You won!");
		}
	}

	// Add spotlight exponent display
	floatToString(theStringBuffer, 4, spotExponent);
	glRasterPos3f(-28.0, 22.0, -30.0);
	writeBitmapString((void *)font, (char *)"Attenuation exponent: ");
	writeBitmapString((void *)font, theStringBuffer);

	if (isCollision)
	{
		displayMessage = 1;										 // "You lose!" message
		messageStartTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0f; // Set start time
	}

	if (checkWinCondition(xVal, 0.0f, zVal))
	{
		displayMessage = 2;										 // "You won!" message
		messageStartTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0f; // Set start time
	}

	glEnable(GL_LIGHTING);
	glPopMatrix();

	// Fixed camera.
	gluLookAt(xVal - 10 * sin((M_PI / 180.0) * angle), 10, zVal - 10 * cos((M_PI / 180.0) * angle) + 30,
			  xVal - 11 * sin((M_PI / 180.0) * angle), 0.0, zVal - 11 * cos((M_PI / 180.0) * angle),
			  0.0, 1.0, 0.0);

	glColor3f(1.0, 1.0, 1.0);
	// Grass texture mapping (x-z plane)
	glBindTexture(GL_TEXTURE_2D, current_ground_texture);
	glBegin(GL_POLYGON);
	glTexCoord2f(0.0, 0.0);
	glVertex3f(-200, -4.5, 200);
	glTexCoord2f(8.0, 0.0);
	glVertex3f(200, -4.5, 200);
	glTexCoord2f(8.0, 8.0);
	glVertex3f(250, -4.5, zVal - 250);
	glTexCoord2f(0.0, 8.0);
	glVertex3f(-250, -4.5, zVal - 250);
	glEnd();

	glMatrixMode(GL_TEXTURE);
	glPushMatrix();
	glTranslatef(0.1 * cos(skyAngle), 0.1 * sin(skyAngle), 0.0);
	// Sky texture mapping (xy plane)
	glBindTexture(GL_TEXTURE_2D, texture[1]);
	glBegin(GL_POLYGON);
	glTexCoord2f(0.0, 0.0);
	glVertex3f(-250, -5, zVal - 150);
	glTexCoord2f(1.0, 0.0);
	glVertex3f(250, -5, zVal - 150);
	glTexCoord2f(1.0, 1.0);
	glVertex3f(250, 120.0, zVal - 200);
	glTexCoord2f(0.0, 1.0);
	glVertex3f(-250, 120.0, zVal - 200);
	glEnd();
	glPopMatrix();

	glMatrixMode(GL_MODELVIEW);
	// Road
	glPushMatrix();
	setRoadMaterial();
	glColor3f(0.4f, 0.4f, 0.4f); // Dark gray color for the street.
	glBegin(GL_QUADS);
	glVertex3f(-30, -4.4f, 50.0f); // Bottom-left corner.
	glVertex3f(30, -4.4f, 50.0f);  // Bottom-right corner.
	glVertex3f(30, -4.4, gateZ);   // Top-right corner.
	glVertex3f(-30, -4.4, gateZ);  // Top-left corner.
	glEnd();

	// Add road lines in the middle
	glLineWidth(5.0f);
	glColor3f(1.0f, 1.0f, 1.0f); // White color for the road lines
	// Right line
	glBegin(GL_LINES);
	glVertex3f(5.0f, -4.4, 50.0f); // Starting point of the second line.
	glVertex3f(5.0f, -4.4, gateZ); // Ending point of the second line.
	glEnd();

	// Left line
	glBegin(GL_LINES);
	glVertex3f(-5.0f, -4.4f, 50.0f); // Starting point of the first line.
	glVertex3f(-5.0f, -4.4, gateZ);	 // Ending point of the first line.
	glEnd();

	glPopMatrix();

	setConeMaterial();
	// Draw all the cones.
	for (j = 0; j < COLUMNS; j++)
		for (i = 0; i < ROWS; i++)
			arrayCones[i][j].draw();

	// Draw car with spotlights
	glPushMatrix();
	setCarMaterial();
	glTranslatef(xVal, 15.0, zVal);
	glRotatef(angle, 0.0, 1.0, 0.0);

	// First spotlight setup
	glPushMatrix();
	glTranslatef(-2.5, -10, -2.0); // Offset from car center, raised slightly, and moved forward
	glTranslatef(xMove, 0.0, zMove);

	// Green cube for first spotlight
	glPushMatrix();
	glColor3f(0.0, 1.0, 0.0);
	glDisable(GL_LIGHTING);
	glutSolidCube(1.0);
	glEnable(GL_LIGHTING);
	glPopMatrix();

	// Wireframe cone for first spotlight
	glPushMatrix();
	glDisable(GL_LIGHTING);
	glColor3f(1.0, 1.0, 1.0);
	glutWireCone(3.0 * tan(spotAngle / 180.0 * M_PI), 3.0, 20, 20);
	glEnable(GL_LIGHTING);
	glPopMatrix();

	// Set first spotlight properties
	glLightfv(GL_LIGHT1, GL_POSITION, lightPos1);
	glLightf(GL_LIGHT1, GL_SPOT_CUTOFF, spotAngle);
	glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, spotDirection);
	glLightf(GL_LIGHT1, GL_SPOT_EXPONENT, spotExponent);
	glPopMatrix();

	// Second spotlight setup
	glPushMatrix();
	glTranslatef(2.5, -10.0, -2.0); // Offset from car center, raised slightly, and moved forward
	glTranslatef(xMove, 0.0, zMove);

	// Green cube for second spotlight
	glPushMatrix();
	glColor3f(0.0, 1.0, 0.0);
	glDisable(GL_LIGHTING);
	glutSolidCube(1.0);
	glEnable(GL_LIGHTING);
	glPopMatrix();

	// Wireframe cone for second spotlight
	glPushMatrix();
	glDisable(GL_LIGHTING);
	glColor3f(1.0, 1.0, 1.0);
	glutWireCone(3.0 * tan(spotAngle / 180.0 * M_PI), 3.0, 20, 20);
	glEnable(GL_LIGHTING);
	glPopMatrix();

	// Set second spotlight properties
	glLightfv(GL_LIGHT2, GL_POSITION, lightPos2);
	glLightf(GL_LIGHT2, GL_SPOT_CUTOFF, spotAngle);
	glLightfv(GL_LIGHT2, GL_SPOT_DIRECTION, spotDirection);
	glLightf(GL_LIGHT2, GL_SPOT_EXPONENT, spotExponent);
	glPopMatrix();

	// Draw car
	glCallList(car_display_list);
	glPopMatrix();

	// gate
	glPushMatrix();
	setGateMaterial();
	if (zVal > -100)
	{
		glTranslatef(gateX, 0.0, zVal - 150);
	}
	else
		glTranslatef(gateX, 0.0, gateZ);
	glColor3f(1.0f, 1.0f, 1.0f);
	glScalef(gateLength, gateHeight, gateDepth);
	glutSolidCube(1.0f);
	glPopMatrix();
	// End left viewport.

    // Begin right viewport.
    glViewport(width / 2.0, 0, width / 2.0, height);
    glLoadIdentity();

    // Draw a vertical line on the left of the viewport to separate the two viewports
    glColor3f(1.0, 1.0, 1.0);
    glLineWidth(2.0);
    glBegin(GL_LINES);
    glVertex3f(-5.0, -5.0, -5.0);
    glVertex3f(-5.0, 5.0, -5.0);
    glEnd();
    glLineWidth(1.0);

	// Locate the camera at the tip of the car and pointing in the direction of the car.
	gluLookAt(xVal - 10 * sin((M_PI / 180.0) * angle), 0.0, zVal - 10 * cos((M_PI / 180.0) * angle),
			  xVal - 11 * sin((M_PI / 180.0) * angle), 0.0, zVal - 11 * cos((M_PI / 180.0) * angle),
			  0.0, 1.0, 0.0);

	glPushMatrix();
	glColor3f(0.2f, 0.2f, 0.2f); // Dark gray color for the street.
	glBegin(GL_QUADS);
	// road
	glVertex3f(-40.0f, -4.4, 50.0f);   // Bottom-left corner.
	glVertex3f(40.0f, -4.4, 50.0f);	   // Bottom-right corner.
	glVertex3f(40.0f, -4.4, -300.0f);  // Top-right corner.
	glVertex3f(-40.0f, -4.4, -300.0f); // Top-left corner.
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
	case 't':
		if (spotExponent > 0.0)
			spotExponent -= 0.1;
		glutPostRedisplay();
		break;
	case 'T':
		spotExponent += 0.1;
		glutPostRedisplay();
		break;
	default:
		break;
	}
}

// Callback routine for non-ASCII key entry.
void specialKeyInput(int key, int x, int y)
{
	if (key == GLUT_KEY_PAGE_DOWN)
	{
		if (spotAngle > 0.0)
			spotAngle -= 1.0;
	}
	if (key == GLUT_KEY_PAGE_UP)
	{
		if (spotAngle < 90.0)
			spotAngle += 1.0;
	}
	if (key == GLUT_KEY_UP)
	{
		moveForwardFlag = true;
		// if (zMove > -4.0) zMove -= 0.1;
	}
	if (key == GLUT_KEY_DOWN)
	{
		moveBackwardFlag = true;
		// if (zMove < 4.0) zMove += 0.1;
	}
	if (key == GLUT_KEY_LEFT)
	{
		moveLeftFlag = true;
		// if (xMove > -4.0) xMove -= 0.1;
	}
	if (key == GLUT_KEY_RIGHT)
	{
		moveRightFlag = true;
		// if (xMove < 4.0) xMove += 0.1;
	}
	glutPostRedisplay();
}

void specialKeyUp(int key, int x, int y)
{
	switch (key)
	{
	case GLUT_KEY_LEFT:
		moveLeftFlag = false;
		// std::cout << "released left " << moveLeftFlag << std::endl;
		break;
	case GLUT_KEY_RIGHT:
		moveRightFlag = false;
		// std::cout << "released right " << moveRightFlag << std::endl;
		break;
	case GLUT_KEY_UP:
		moveForwardFlag = false;
		// std::cout << "released up " << moveForwardFlag<< std::endl;
		break;
	case GLUT_KEY_DOWN:
		moveBackwardFlag = false;
		// std::cout << "released down " << moveBackwardFlag<< std::endl;
		break;
	default:
		break;
	}
	glutPostRedisplay();
}

// Routine to output interaction instructions to the C++ window.
void printInteraction(void)
{
	std::cout << "Interaction:" << std::endl;
	std::cout << "Press the page up/down arrow keys to increase/decrease the spotlight cone angle." << std::endl
			  << "Press the arrow keys to move the spotlight." << std::endl
			  << "Press 't/T' to decrease/increase the spotlight's attenuation exponent." << std::endl
			  << "Press the left/right arrow keys to turn the craft." << std::endl
			  << "Press the up/down arrow keys to move the craft." << std::endl
			  << "Right click the mouse to change ground texture." << std::endl;
}

// Main routine.
int main(int argc, char **argv)
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
    glutSpecialUpFunc(specialKeyUp);

    glewExperimental = GL_TRUE;
    glewInit();

	setup();
	glutTimerFunc(0, gameTimer, 0); // Start the game timer.

    glutMainLoop();
}
