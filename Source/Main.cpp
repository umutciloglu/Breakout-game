#include <iostream>
#include <glad/glad.h>
#include <GL/freeglut.h>

// Game window dimensions
const int windowWidth = 800;
const int windowHeight = 600;

// Paddle properties
const float paddleWidth = 10.0f;
const float paddleHeight = 100.0f;
float paddleLeftY = windowHeight / 2.0f;
float paddleRightY = windowHeight / 2.0f;
const float paddleSpeed = 15.0f;

// Ball properties
float ballX = windowWidth / 2.0f;
float ballY = windowHeight / 2.0f;
float ballSize = 10.0f;
float ballSpeedX = 4.0f;
float ballSpeedY = 4.0f;

// Scores
int scoreLeft = 0;
int scoreRight = 0;

void framebuffer_size_callback(int width, int height)
{
	//Window resized callback.
	glViewport(0, 0, width, height);

}

void processInput(unsigned char key, int x, int y)
{
	//Key and mouse callback.
	switch (key) {
	case 'w':
		paddleLeftY += paddleSpeed;
		if (paddleLeftY + paddleHeight / 2 > windowHeight) paddleLeftY = windowHeight - paddleHeight / 2;
		break;
	case 's':
		paddleLeftY -= paddleSpeed;
		if (paddleLeftY - paddleHeight / 2 < 0) paddleLeftY = paddleHeight / 2;
		break;
	// Add controls for the right paddle if desired (e.g., using arrow keys, which requires glutSpecialFunc)
	// For simplicity, we'll control the right paddle with 'o' and 'l' for now
	case 'o':
		paddleRightY += paddleSpeed;
		if (paddleRightY + paddleHeight / 2 > windowHeight) paddleRightY = windowHeight - paddleHeight / 2;
		break;
	case 'l':
		paddleRightY -= paddleSpeed;
		if (paddleRightY - paddleHeight / 2 < 0) paddleRightY = paddleHeight / 2;
		break;
	case 27: // ESC key
		glutLeaveMainLoop();
		break;
	}
}

void drawRect(float x, float y, float width, float height)
{
	glBegin(GL_QUADS);
	glVertex2f(x - width / 2, y - height / 2);
	glVertex2f(x + width / 2, y - height / 2);
	glVertex2f(x + width / 2, y + height / 2);
	glVertex2f(x - width / 2, y + height / 2);
	glEnd();
}

void drawText(float x, float y, const char* string)
{
	glColor3f(1.0f, 1.0f, 1.0f); // White text
	glRasterPos2f(x, y);
	for (const char* c = string; *c != '\0'; c++) {
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
	}
}

void display()
{
	//Rendering method.

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Black background
	glClear(GL_COLOR_BUFFER_BIT);

	// Set up orthographic projection
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0, windowWidth, 0, windowHeight);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// Draw paddles
	glColor3f(1.0f, 1.0f, 1.0f); // White color
	drawRect(paddleWidth, paddleLeftY, paddleWidth, paddleHeight); // Left paddle
	drawRect(windowWidth - paddleWidth, paddleRightY, paddleWidth, paddleHeight); // Right paddle

	// Draw ball
	drawRect(ballX, ballY, ballSize, ballSize);

	// TODO: Draw scores
	char scoreStr[50];
	sprintf_s(scoreStr, "Left: %d  Right: %d", scoreLeft, scoreRight);
	drawText(windowWidth / 2.0f - 70.0f, windowHeight - 30.0f, scoreStr);

	glutSwapBuffers();
}

void update(int value)
{
	// Ball movement
	ballX += ballSpeedX;
	ballY += ballSpeedY;

	// Ball collision with top/bottom walls
	if (ballY + ballSize / 2 > windowHeight || ballY - ballSize / 2 < 0) {
		ballSpeedY = -ballSpeedY;
	}

	// Ball collision with paddles
	// Left paddle
	if (ballX - ballSize / 2 < paddleWidth + paddleWidth / 2 && // Ball's left edge vs paddle's right edge
		ballX + ballSize / 2 > paddleWidth - paddleWidth / 2 && // Ball's right edge vs paddle's left edge
		ballY + ballSize / 2 > paddleLeftY - paddleHeight / 2 &&
		ballY - ballSize / 2 < paddleLeftY + paddleHeight / 2) {
		ballSpeedX = -ballSpeedX;
		// Optional: adjust ballSpeedY based on where it hits the paddle
	}
	// Right paddle
	if (ballX + ballSize / 2 > windowWidth - paddleWidth - paddleWidth / 2 &&
		ballX - ballSize / 2 < windowWidth - paddleWidth + paddleWidth / 2 &&
		ballY + ballSize / 2 > paddleRightY - paddleHeight / 2 &&
		ballY - ballSize / 2 < paddleRightY + paddleHeight / 2) {
		ballSpeedX = -ballSpeedX;
		// Optional: adjust ballSpeedY based on where it hits the paddle
	}


	// Ball out of bounds (scoring)
	if (ballX < 0) {
		scoreRight++;
		std::cout << "Score: Left - " << scoreLeft << " Right - " << scoreRight << std::endl;
		// Reset ball
		ballX = windowWidth / 2.0f;
		ballY = windowHeight / 2.0f;
		ballSpeedX = 4.0f; // Or randomize
	}
	else if (ballX > windowWidth) {
		scoreLeft++;
		std::cout << "Score: Left - " << scoreLeft << " Right - " << scoreRight << std::endl;
		// Reset ball
		ballX = windowWidth / 2.0f;
		ballY = windowHeight / 2.0f;
		ballSpeedX = -4.0f; // Or randomize
	}

	glutPostRedisplay(); // Request a redraw
	glutTimerFunc(16, update, 0); // ~60 FPS
}

int main(int argc, char** argv)
{
	glutInit(&argc, argv);

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(windowWidth, windowHeight);
	glutCreateWindow("Pong Game - FreeGLUT");

	if (!gladLoadGLLoader((GLADloadproc)glutGetProcAddress)) {
		std::cerr << "[GLAD]: Failed to load OpenGL methods from driver." << std::endl;
		return -1;
	}

	glutKeyboardFunc(processInput);
	glutReshapeFunc(framebuffer_size_callback);
	glutDisplayFunc(display);
	glutTimerFunc(0, update, 0); // Start the update loop
	glutMainLoop();

	return 0;
}