#include <iostream>
#include <glad/glad.h>
#include <GL/freeglut.h>
#include <cmath> // Added for sin, cos

// Camera variables
float cameraX = 0.0f;
float cameraY = 0.0f;
float cameraZ = 3.0f; // Initial Z position to see the origin
float cameraYaw = -90.0f; // Initialize yaw to look along negative Z-axis
float cameraPitch = 0.0f;
float moveSpeed = 0.1f;
float mouseSensitivity = 0.1f;
const float  M_PI = 3.1415f;
// Mouse state
int lastX = 400, lastY = 300; // Initialize to center of a 800x600 window
bool firstMouse = true;

void framebuffer_size_callback(int width, int height)
{
	//Window resized callback.
	glViewport(0, 0, width, height);

	// Set up the projection matrix
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	// Adjust zNear (e.g., 0.05f) and zFar (e.g., 500.0f) for extended view
	// Field of view (FOV) is 45 degrees, aspect ratio based on window dimensions
	if (height == 0) height = 1; // Prevent division by zero
	gluPerspective(45.0f, (GLfloat)width / (GLfloat)height, 0.05f, 500.0f);

	// Switch back to the modelview matrix
	glMatrixMode(GL_MODELVIEW);
	// glLoadIdentity(); // Not needed here as display() handles modelview setup
	glutPostRedisplay(); // Request redraw after projection change
}

void processInput(unsigned char key, int x, int y)
{
	//Key and mouse callback.
	float yawRad = cameraYaw * (M_PI / 180.0f);
	float pitchRad = cameraPitch * (M_PI / 180.0f);

	float forwardX = cos(yawRad) * cos(pitchRad);
	float forwardY = sin(pitchRad);
	float forwardZ = sin(yawRad) * cos(pitchRad);

	// Normalize the forward vector (optional but good practice)
	// float len = sqrt(forwardX * forwardX + forwardY * forwardY + forwardZ * forwardZ);
	// if (len > 0) {
	// 	forwardX /= len;
	// 	forwardY /= len;
	// 	forwardZ /= len;
	// }


	float rightX = cos(yawRad - M_PI / 2.0f);
	float rightZ = sin(yawRad - M_PI / 2.0f);


	switch (key)
	{
	case 'w':
		cameraX += forwardX * moveSpeed;
		cameraY += forwardY * moveSpeed;
		cameraZ += forwardZ * moveSpeed;
		break;
	case 's':
		cameraX -= forwardX * moveSpeed;
		cameraY -= forwardY * moveSpeed;
		cameraZ -= forwardZ * moveSpeed;
		break;
	case 'a':
		cameraX -= rightX * moveSpeed;
		cameraZ -= rightZ * moveSpeed;
		break;
	case 'd':
		cameraX += rightX * moveSpeed;
		cameraZ += rightZ * moveSpeed;
		break;
	case ' ': // Space for up
		cameraY += moveSpeed;
		break;
	case GLUT_KEY_CTRL_L: // Using a special key for down, GLUT_KEY_SHIFT_L for shift, etc.
	case 'c': // Or a regular key like 'c' for crouch/down
		cameraY -= moveSpeed;
		break;
	case 27: // Escape key
		glutLeaveMainLoop();
		break;
	}
	glutPostRedisplay(); // Request a redraw
}

void mouseMotion(int x, int y) {
    if (firstMouse) {
        lastX = x;
        lastY = y;
        firstMouse = false;
    }

	float xoffset = x - lastX;
	float yoffset = lastY - y; // Reversed since y-coordinates go from bottom to top

	xoffset *= mouseSensitivity;
	yoffset *= mouseSensitivity;

	cameraYaw += xoffset;
	cameraPitch += yoffset;

	// Clamp pitch to avoid flipping
	if (cameraPitch > 89.0f)
		cameraPitch = 89.0f;
	if (cameraPitch < -89.0f)
		cameraPitch = -89.0f;

    // Re-center the cursor and update lastX, lastY for the next motion event
    int winWidth = glutGet(GLUT_WINDOW_WIDTH);
    int winHeight = glutGet(GLUT_WINDOW_HEIGHT);
    int centerX = winWidth / 2;
    int centerY = winHeight / 2;

    if (x != centerX || y != centerY) {
        glutWarpPointer(centerX, centerY);
        lastX = centerX;
        lastY = centerY;
    } else {
        lastX = centerX;
        lastY = centerY;
    }

	glutPostRedisplay(); // Request a redraw
}

void display()
{
	//Rendering method.
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Also clear depth buffer

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	float yawRad = cameraYaw * (M_PI / 180.0f);
	float pitchRad = cameraPitch * (M_PI / 180.0f);

	float lookX = cos(yawRad) * cos(pitchRad);
	float lookY = sin(pitchRad);
	float lookZ = sin(yawRad) * cos(pitchRad);

	gluLookAt(cameraX, cameraY, cameraZ,
		cameraX + lookX, cameraY + lookY, cameraZ + lookZ,
		0.0f, 1.0f, 0.0f); // Up vector

	// Draw XYZ coordinate axes
	glLineWidth(2.0f); // Make axes lines thicker
	glBegin(GL_LINES);
	// X-axis (Red)
	glColor3f(1.0f, 0.0f, 0.0f);
	glVertex3f(0.0f, 0.0f, 0.0f);
	glVertex3f(2.0f, 0.0f, 0.0f); // Length of 2 units
	// Y-axis (Green)
	glColor3f(0.0f, 1.0f, 0.0f);
	glVertex3f(0.0f, 0.0f, 0.0f);
	glVertex3f(0.0f, 2.0f, 0.0f); // Length of 2 units
	// Z-axis (Blue)
	glColor3f(0.0f, 0.0f, 1.0f);
	glVertex3f(0.0f, 0.0f, 0.0f);
	glVertex3f(0.0f, 0.0f, 2.0f); // Length of 2 units
	glEnd();
	glLineWidth(1.0f); // Reset line width

	// Draw the original red cube at the origin
	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(1.0f, 1.0f);
	glColor3f(1.0f, 0.0f, 0.0f); // Red color
	glutSolidCube(1.0);      // Draw solid cube
	glDisable(GL_POLYGON_OFFSET_FILL);
	glColor3f(0.0f, 0.0f, 0.0f); // Black color for wireframe
	glutWireCube(1.0); // Draw wireframe cube at the origin

	// Add more cubes for better 3D visualization
	// Green cube
	glPushMatrix(); // Save the current modelview matrix
	glTranslatef(2.0f, 0.5f, -2.0f); // Move to new position
	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(1.0f, 1.0f);
	glColor3f(0.0f, 1.0f, 0.0f);    // Green color
	glutSolidCube(0.5);              // Smaller solid cube
	glDisable(GL_POLYGON_OFFSET_FILL);
	glColor3f(0.0f, 0.0f, 0.0f);
	glutWireCube(0.5);              // Smaller wireframe cube
	glPopMatrix();  // Restore the saved modelview matrix

	// Blue cube
	glPushMatrix();
	glTranslatef(-1.5f, 1.0f, 0.0f);
	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(1.0f, 1.0f);
	glColor3f(0.0f, 0.0f, 1.0f);    // Blue color
	glutSolidCube(0.75);
	glDisable(GL_POLYGON_OFFSET_FILL);
	glColor3f(0.0f, 0.0f, 0.0f);
	glutWireCube(0.75);
	glPopMatrix();

	// Yellow cube (larger and further away)
	glPushMatrix();
	glTranslatef(0.0f, -0.5f, 5.0f);
	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(1.0f, 1.0f);
	glColor3f(1.0f, 1.0f, 0.0f);    // Yellow color
	glutSolidCube(1.5);
	glDisable(GL_POLYGON_OFFSET_FILL);
	glColor3f(0.0f, 0.0f, 0.0f);
	glutWireCube(1.5);
	glPopMatrix();

	// --- New objects for extended near/far planes ---

	// Very distant, large, cyan cube
	glPushMatrix();
	glTranslatef(0.0f, 0.0f, -100.0f); // Far along negative Z
	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(1.0f, 1.0f);
	glColor3f(0.0f, 1.0f, 1.0f);      // Cyan color
	glutSolidCube(10.0f);                // Large solid cube
	glDisable(GL_POLYGON_OFFSET_FILL);
	glColor3f(0.0f, 0.0f, 0.0f);
	glutWireCube(10.0f);                // Large wireframe cube
	glPopMatrix();

	// Distant, medium, magenta cube
	glPushMatrix();
	glTranslatef(50.0f, 10.0f, -75.0f); // Far on positive X, moderate Z
	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(1.0f, 1.0f);
	glColor3f(1.0f, 0.0f, 1.0f);       // Magenta color
	glutSolidCube(5.0f);
	glDisable(GL_POLYGON_OFFSET_FILL);
	glColor3f(0.0f, 0.0f, 0.0f);
	glutWireCube(5.0f);
	glPopMatrix();

	// Very close, small, white cube
	glPushMatrix();
	glTranslatef(0.2f, -0.2f, 0.5f); // Close to origin
	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(1.0f, 1.0f);
	glColor3f(1.0f, 1.0f, 1.0f);     // White color
	glutSolidCube(0.1f);                // Very small solid cube
	glDisable(GL_POLYGON_OFFSET_FILL);
	glColor3f(0.0f, 0.0f, 0.0f);
	glutWireCube(0.1f);                // Very small wireframe cube
	glPopMatrix();


	glutSwapBuffers();
}

int main(int argc, char** argv)
{
	glutInit(&argc, argv);

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH); // Added GLUT_DEPTH for depth testing
	glutInitWindowSize(800, 600);
	glutCreateWindow("OpenGL Template");

	if (!gladLoadGLLoader((GLADloadproc)glutGetProcAddress)) {
		std::cerr << "[GLAD]: Failed to load OpenGL methods from driver." << std::endl;
		return -1;
	}

	glEnable(GL_DEPTH_TEST); // Enable depth testing

	// Initialize mouse position (center of the window)
	lastX = glutGet(GLUT_WINDOW_WIDTH) / 2;
	lastY = glutGet(GLUT_WINDOW_HEIGHT) / 2;
	// Optional: warp pointer to center initially, but passive motion handles updates
	// glutWarpPointer(lastX, lastY);


	glutKeyboardFunc(processInput);
	glutReshapeFunc(framebuffer_size_callback);
	glutDisplayFunc(display);
	glutPassiveMotionFunc(mouseMotion); // For continuous mouse tracking

	glutSetCursor(GLUT_CURSOR_NONE); // Hide the mouse cursor

	glutMainLoop();

	return 0;
}