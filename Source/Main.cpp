#include <iostream>
#include <fstream>
#include <glad/glad.h>
#include <GL/freeglut.h>
#include <vector>
#include <cmath>

// Game constants
const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;
const float PADDLE_WIDTH = 100.0f;
const float PADDLE_HEIGHT = 20.0f;
const float BALL_SIZE = 10.0f;
const float BRICK_WIDTH = 75.0f;
const float BRICK_HEIGHT = 25.0f;
const int BRICK_ROWS = 8;
const int BRICK_COLS = 10;
const float PADDLE_SPEED = 300.0f;
const float BALL_SPEED = 200.0f;

std::ofstream log_file;

void checkOpenGLError(const char* operation) {
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        log_file << "[OpenGL ERROR] " << operation << ": ";
        const char* errorStr = "";
        switch(error) {
            case GL_INVALID_ENUM: errorStr = "GL_INVALID_ENUM"; break;
            case GL_INVALID_VALUE: errorStr = "GL_INVALID_VALUE"; break;
            case GL_INVALID_OPERATION: errorStr = "GL_INVALID_OPERATION"; break;
            case GL_OUT_OF_MEMORY: errorStr = "GL_OUT_OF_MEMORY"; break;
            default: errorStr = "Unknown error"; break;
        }
        log_file << errorStr << " (" << error << ")" << std::endl;
    }
}


// Game state
struct Vector2 {
	float x, y;
	Vector2(float x = 0, float y = 0) : x(x), y(y) {}
	Vector2 operator+(const Vector2& other) const { return Vector2(x + other.x, y + other.y); }
	Vector2 operator*(float scalar) const { return Vector2(x * scalar, y * scalar); }
};

struct Brick {
	Vector2 position;
	bool active;
	int color; // 0=red, 1=orange, 2=yellow, 3=green, 4=blue, 5=purple, 6=pink, 7=cyan
	
	Brick(float x, float y, int c) : position(x, y), active(true), color(c) {}
};

struct Ball {
	Vector2 position;
	Vector2 velocity;
	
	Ball(float x, float y, float vx, float vy) : position(x, y), velocity(vx, vy) {}
};

struct Paddle {
	Vector2 position;
	
	Paddle(float x, float y) : position(x, y) {}
};

// Game objects
std::vector<Brick> bricks;
Ball ball(WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2, -BALL_SPEED * 0.7f, -BALL_SPEED * 0.7f);
Paddle paddle(WINDOW_WIDTH / 2 - PADDLE_WIDTH / 2, 50);

// Game state
bool gameRunning = true;
bool gameWon = false;
bool gameLost = false;
int score = 0;
int lives = 3;

// Input state
bool keys[256] = {false};

// Time tracking
int lastTime = 0;

void setColor(int colorIndex) {
	switch(colorIndex) {
		case 0: glColor3f(1.0f, 0.0f, 0.0f); break; // Red
		case 1: glColor3f(1.0f, 0.5f, 0.0f); break; // Orange
		case 2: glColor3f(1.0f, 1.0f, 0.0f); break; // Yellow
		case 3: glColor3f(0.0f, 1.0f, 0.0f); break; // Green
		case 4: glColor3f(0.0f, 0.0f, 1.0f); break; // Blue
		case 5: glColor3f(0.5f, 0.0f, 1.0f); break; // Purple
		case 6: glColor3f(1.0f, 0.0f, 1.0f); break; // Pink
		case 7: glColor3f(0.0f, 1.0f, 1.0f); break; // Cyan
		default: glColor3f(1.0f, 1.0f, 1.0f); break; // White
	}
}

void drawRect(float x, float y, float width, float height) {
	glBegin(GL_QUADS);
	glVertex2f(x, y);
	glVertex2f(x + width, y);
	glVertex2f(x + width, y + height);
	glVertex2f(x, y + height);
	glEnd();
}

void drawCircle(float x, float y, float radius) {
	glBegin(GL_TRIANGLE_FAN);
	glVertex2f(x, y); // Center
	for (int i = 0; i <= 20; i++) {
		float angle = 2.0f * 3.14159f * i / 20;
		glVertex2f(x + cos(angle) * radius, y + sin(angle) * radius);
	}
	glEnd();
}

void drawText(float x, float y, const char* text) {
	glRasterPos2f(x, y);
	for (const char* c = text; *c != '\0'; c++) {
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
	}
}

void initBricks() {
	bricks.clear();
	float startX = (WINDOW_WIDTH - (BRICK_COLS * BRICK_WIDTH)) / 2;
	float startY = WINDOW_HEIGHT - 100;
	
	for (int row = 0; row < BRICK_ROWS; row++) {
		for (int col = 0; col < BRICK_COLS; col++) {
			float x = startX + col * BRICK_WIDTH;
			float y = startY - row * BRICK_HEIGHT;
			bricks.push_back(Brick(x, y, row)); // Different color per row
		}
	}
}

void resetBall() {
	ball.position = Vector2(WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2);
	ball.velocity = Vector2(-BALL_SPEED * 0.7f, -BALL_SPEED * 0.7f);
}

void resetGame() {
	initBricks();
	resetBall();
	paddle.position = Vector2(WINDOW_WIDTH / 2 - PADDLE_WIDTH / 2, 50);
	score = 0;
	lives = 3;
	gameRunning = true;
	gameWon = false;
	gameLost = false;
}

bool checkCollision(const Vector2& pos1, float w1, float h1, const Vector2& pos2, float w2, float h2) {
	return pos1.x < pos2.x + w2 && pos1.x + w1 > pos2.x && pos1.y < pos2.y + h2 && pos1.y + h1 > pos2.y;
}

void updateGame(float deltaTime) {
	if (!gameRunning) return;
	
	// Handle input
	if (keys['a'] || keys['A']) {
		paddle.position.x -= PADDLE_SPEED * deltaTime;
		if (paddle.position.x < 0) paddle.position.x = 0;
	}
	if (keys['d'] || keys['D']) {
		paddle.position.x += PADDLE_SPEED * deltaTime;
		if (paddle.position.x + PADDLE_WIDTH > WINDOW_WIDTH) 
			paddle.position.x = WINDOW_WIDTH - PADDLE_WIDTH;
	}
	
	// Update ball position
	ball.position = ball.position + ball.velocity * deltaTime;
	
	// Ball collision with walls
	if (ball.position.x <= 0 || ball.position.x + BALL_SIZE >= WINDOW_WIDTH) {
		ball.velocity.x = -ball.velocity.x;
		ball.position.x = ball.position.x <= 0 ? 0 : WINDOW_WIDTH - BALL_SIZE;
	}
	if (ball.position.y + BALL_SIZE >= WINDOW_HEIGHT) {
		ball.velocity.y = -ball.velocity.y;
		ball.position.y = WINDOW_HEIGHT - BALL_SIZE;
	}
	
	// Ball collision with paddle
	if (checkCollision(ball.position, BALL_SIZE, BALL_SIZE, paddle.position, PADDLE_WIDTH, PADDLE_HEIGHT)) {
		// Calculate bounce angle based on where ball hits paddle
		float paddleCenter = paddle.position.x + PADDLE_WIDTH / 2;
		float ballCenter = ball.position.x + BALL_SIZE / 2;
		float hitPos = (ballCenter - paddleCenter) / (PADDLE_WIDTH / 2); // -1 to 1
		
		ball.velocity.x = hitPos * BALL_SPEED;
		ball.velocity.y = abs(ball.velocity.y); // Always bounce up
		
		// Normalize velocity to maintain speed
		float speed = sqrt(ball.velocity.x * ball.velocity.x + ball.velocity.y * ball.velocity.y);
		ball.velocity.x = (ball.velocity.x / speed) * BALL_SPEED;
		ball.velocity.y = (ball.velocity.y / speed) * BALL_SPEED;
		
		ball.position.y = paddle.position.y + PADDLE_HEIGHT;
	}
	
	// Ball collision with bricks
	for (auto& brick : bricks) {
		if (brick.active && checkCollision(ball.position, BALL_SIZE, BALL_SIZE, brick.position, BRICK_WIDTH, BRICK_HEIGHT)) {
			brick.active = false;
			ball.velocity.y = -ball.velocity.y;
			score += 10;
			break;
		}
	}
	
	// Check for ball falling below paddle
	if (ball.position.y < 0) {
		lives--;
		if (lives <= 0) {
			gameLost = true;
			gameRunning = false;
		} else {
			resetBall();
		}
	}
	
	// Check for win condition
	bool allBricksDestroyed = true;
	for (const auto& brick : bricks) {
		if (brick.active) {
			allBricksDestroyed = false;
			break;
		}
	}
	if (allBricksDestroyed) {
		gameWon = true;
		gameRunning = false;
	}
}

void display() {
	// Calculate delta time
	int currentTime = glutGet(GLUT_ELAPSED_TIME);
	float deltaTime = (currentTime - lastTime) / 1000.0f;
	lastTime = currentTime;
	
	updateGame(deltaTime);
	
	glClearColor(0.0f, 0.0f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	
	// Set up 2D rendering
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, WINDOW_WIDTH, 0, WINDOW_HEIGHT, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	if (gameRunning || gameWon || gameLost) {
		// Draw bricks
		for (const auto& brick : bricks) {
			if (brick.active) {
				setColor(brick.color);
				drawRect(brick.position.x, brick.position.y, BRICK_WIDTH - 2, BRICK_HEIGHT - 2);
			}
		}
		
		// Draw paddle
		glColor3f(0.8f, 0.8f, 0.8f);
		drawRect(paddle.position.x, paddle.position.y, PADDLE_WIDTH, PADDLE_HEIGHT);
		
		// Draw ball
		glColor3f(1.0f, 1.0f, 1.0f);
		drawCircle(ball.position.x + BALL_SIZE/2, ball.position.y + BALL_SIZE/2, BALL_SIZE/2);
		
		// Draw UI
		glColor3f(1.0f, 1.0f, 1.0f);
		char scoreText[50];
		sprintf(scoreText, "Score: %d", score);
		drawText(10, WINDOW_HEIGHT - 30, scoreText);
		
		char livesText[50];
		sprintf(livesText, "Lives: %d", lives);
		drawText(10, WINDOW_HEIGHT - 55, livesText);
		
		if (gameWon) {
			drawText(WINDOW_WIDTH/2 - 100, WINDOW_HEIGHT/2, "YOU WIN! Press R to restart");
		} else if (gameLost) {
			drawText(WINDOW_WIDTH/2 - 120, WINDOW_HEIGHT/2, "GAME OVER! Press R to restart");
		}
	}
	
	// Draw instructions
	if (!gameRunning && !gameWon && !gameLost) {
		glColor3f(1.0f, 1.0f, 1.0f);
		drawText(WINDOW_WIDTH/2 - 150, WINDOW_HEIGHT/2 + 50, "BREAKOUT");
		drawText(WINDOW_WIDTH/2 - 180, WINDOW_HEIGHT/2, "Use A and D keys to move paddle");
		drawText(WINDOW_WIDTH/2 - 100, WINDOW_HEIGHT/2 - 30, "Press SPACE to start");
		drawText(WINDOW_WIDTH/2 - 80, WINDOW_HEIGHT/2 - 60, "Press R to restart");
	}
	
	glutSwapBuffers();
	glutPostRedisplay(); // Continuous rendering
}

void processInput(unsigned char key, int x, int y) {
	keys[key] = true;
	
	if (key == ' ' && (!gameRunning && !gameWon && !gameLost)) {
		resetGame();
	}
	if (key == 'r' || key == 'R') {
		resetGame();
	}
	if (key == 27) { // ESC key
		exit(0);
	}
}

void processInputUp(unsigned char key, int x, int y) {
	keys[key] = false;
}

void framebuffer_size_callback(int width, int height) {
	glViewport(0, 0, width, height);
}

int main(int argc, char** argv) {
	log_file = std::ofstream("log.txt");
	glutInit(&argc, argv);
	
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
	glutCreateWindow("Breakout Game - FreeGLUT");
	
	if (!gladLoadGLLoader((GLADloadproc)glutGetProcAddress)) {
		log_file << "[GLAD]: Failed to load OpenGL methods from driver." << std::endl;
		log_file.close();
		return -1;
	}
	
	log_file << "OpenGL Successfully Initialized" << std::endl;
	log_file << "OpenGL Vendor: " << glGetString(GL_VENDOR) << std::endl;
	log_file << "OpenGL Renderer: " << glGetString(GL_RENDERER) << std::endl;
	log_file << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
	
	// Initialize game
	initBricks();
	lastTime = glutGet(GLUT_ELAPSED_TIME);
	gameRunning = false; // Start in menu state
	
	glutKeyboardFunc(processInput);
	glutKeyboardUpFunc(processInputUp);
	glutReshapeFunc(framebuffer_size_callback);
	glutDisplayFunc(display);
	
	checkOpenGLError("Before main loop");
	glutMainLoop();
	
	
	log_file.close();
	return 0;
}