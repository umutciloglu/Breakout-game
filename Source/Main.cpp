#include <iostream>
#include <glad/glad.h>
#include <GL/freeglut.h>
#include <cmath> // Added for sin, cos
#include <vector> // Added for std::vector

// Camera variables - Adjusted for starting in a corridor
float cameraX = 0.0f;
float cameraY = 1.5f; // Assuming corridor height of ~3.0f, camera in middle
float cameraZ = 1.0f; // Start slightly inside the first corridor segment
float cameraYaw = 90.0f;   // Corrected: Look along positive Z-axis (90 degrees from +X)
float cameraPitch = 0.0f;
float moveSpeed = 3.0f; // Adjusted: Now represents units per second
const float sprintMultiplier = 2.0f; // Multiplier for sprint speed
float mouseSensitivity = 0.1f;
const float  M_PI = 3.1415f;
// Mouse state
int lastX = 400, lastY = 300; // Initialize to center of a 800x600 window
bool firstMouse = true;

// Key states for simultaneous movement
bool key_w_down = false;
bool key_s_down = false;
bool key_a_down = false; // For strafing right (inverted)
bool key_d_down = false; // For strafing left (inverted)
bool key_space_down = false;
bool key_c_down = false;
bool key_shift_down = false; // Added for Shift key state

// Projectile constants
const float PROJECTILE_SPEED = 40.0f; // Increased speed
const float PROJECTILE_MAX_LIFETIME = 1.5f; // Shorter lifetime
const float PROJECTILE_RADIUS = 0.08f;

// Projectile structure
struct Projectile {
    float x, y, z;          // Current position
    float dirX, dirY, dirZ; // Direction vector (normalized)
    bool active;
    float lifeRemaining;    // in seconds
};
std::vector<Projectile> projectiles;
bool mouse_left_click_event = false; // For single fire event

// Time tracking for frame-rate independent movement
float lastFrameTime = 0.0f;

// Helper: Vector Normalization
void normalizeVector(float& x, float& y, float& z) {
    float length = sqrt(x * x + y * y + z * z);
    if (length > 0.00001f) { // Avoid division by zero and near-zero lengths
        float invLength = 1.0f / length;
        x *= invLength;
        y *= invLength;
        z *= invLength;
    }
}

// Helper function to draw a simple 2D gun model
void drawGun() {
    // All coordinates are relative to the gun's anchor point (0,0 for this model)
    // which will be positioned on screen by glTranslatef in display()
    // Gun model: (0,0) is considered bottom-left of the main gun body/grip area.

    // Main body (dark grey)
    glColor3f(0.25f, 0.25f, 0.25f);
    glBegin(GL_QUADS);
        glVertex2f(0.0f,   10.0f);  // Back-bottom of body
        glVertex2f(60.0f,  10.0f);  // Front-bottom of body
        glVertex2f(60.0f,  30.0f);  // Front-top of body
        glVertex2f(0.0f,   30.0f);  // Back-top of body
    glEnd();

    // Barrel (slightly lighter grey, extends from body)
    glColor3f(0.35f, 0.35f, 0.35f);
    glBegin(GL_QUADS);
        glVertex2f(60.0f, 15.0f);  // Connects to body front-bottom of barrel
        glVertex2f(100.0f, 15.0f); // Tip of barrel, bottom
        glVertex2f(100.0f, 25.0f); // Tip of barrel, top
        glVertex2f(60.0f, 25.0f);  // Connects to body front-top of barrel
    glEnd();

    // Grip (darker, extends downwards from the back of the body)
    glColor3f(0.2f, 0.2f, 0.2f);
    glBegin(GL_QUADS);
        glVertex2f(5.0f,  -20.0f); // Grip bottom-left
        glVertex2f(20.0f, -20.0f); // Grip bottom-right
        glVertex2f(20.0f,  10.0f); // Connects to body bottom-right of grip area
        glVertex2f(5.0f,   10.0f); // Connects to body bottom-left of grip area
    glEnd();
}

// Helper function to draw a simple 3D gun model
void draw3DGunModel() {
    // Colors for the 3D gun
    float body_r = 0.15f, body_g = 0.15f, body_b = 0.15f; // Dark grey body
    float barrel_r = 0.2f, barrel_g = 0.2f, barrel_b = 0.2f; // Slightly lighter barrel

    glPushMatrix(); // Save current transformation state for the gun model parts

    // Main Body of the 3D gun
    glColor3f(body_r, body_g, body_b);
    glPushMatrix();
    glScalef(0.3f, 0.15f, 0.1f); // Width, Height, Depth of body
    glutSolidCube(1.0f);
    glPopMatrix();

    // Barrel (in front of the body)
    glColor3f(barrel_r, barrel_g, barrel_b);
    glPushMatrix();
    // Position barrel relative to body: body extends to 0.15 in X, barrel starts there
    // Barrel center at x=0.15 + 0.2/2 = 0.25. Body height is 0.15, barrel height 0.08, centered.
    glTranslatef(0.15f + 0.1f, 0.0f, 0.0f); // Move to the front-center of the body
    glScalef(0.4f, 0.08f, 0.08f); // Length, Height, Depth of barrel
    glutSolidCube(1.0f);
    glPopMatrix();

    // Optional: Grip (below and slightly back from the body's center)
    // glColor3f(body_r * 0.8f, body_g * 0.8f, body_b * 0.8f);
    // glPushMatrix();
    // glTranslatef(0.0f, -0.075f - 0.05f, 0.0f); // Position grip below body
    // glScalef(0.08f, 0.2f, 0.08f); // Width, Height, Depth of grip
    // glutSolidCube(1.0f);
    // glPopMatrix();

    glPopMatrix(); // Restore transformation state before gun model parts
}

// Helper function to draw a corridor segment
// Draws an open-ended segment from z=0 to z=length in its local coordinates
void drawCorridorSegment(float width, float height, float length, 
                         float wall_r, float wall_g, float wall_b,
                         float floor_r, float floor_g, float floor_b,
                         float ceil_r, float ceil_g, float ceil_b)
{
    float w_half = width / 2.0f;

    glBegin(GL_QUADS);

    // Floor
    glColor3f(floor_r, floor_g, floor_b);
    glVertex3f(-w_half, 0.0f, length); // Back-left
    glVertex3f( w_half, 0.0f, length); // Back-right
    glVertex3f( w_half, 0.0f, 0.0f);   // Front-right
    glVertex3f(-w_half, 0.0f, 0.0f);   // Front-left

    // Ceiling
    glColor3f(ceil_r, ceil_g, ceil_b);
    glVertex3f(-w_half, height, 0.0f);   // Front-left
    glVertex3f( w_half, height, 0.0f);   // Front-right
    glVertex3f( w_half, height, length); // Back-right
    glVertex3f(-w_half, height, length); // Back-left

    // Left Wall
    glColor3f(wall_r, wall_g, wall_b);
    glVertex3f(-w_half, 0.0f, 0.0f);    // Bottom-front
    glVertex3f(-w_half, 0.0f, length);  // Bottom-back
    glVertex3f(-w_half, height, length); // Top-back
    glVertex3f(-w_half, height, 0.0f);   // Top-front
    
    // Right Wall
    // Using a slightly darker shade of the wall color for the other wall for subtle difference
    glColor3f(wall_r * 0.9f, wall_g * 0.9f, wall_b * 0.9f);
    glVertex3f( w_half, 0.0f, length);  // Bottom-back
    glVertex3f( w_half, 0.0f, 0.0f);    // Bottom-front
    glVertex3f( w_half, height, 0.0f);   // Top-front
    glVertex3f( w_half, height, length); // Top-back

    glEnd();
}

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

void processKeyDown(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 'w': key_w_down = true; break;
	case 's': key_s_down = true; break;
	case 'a': key_a_down = true; break; // 'a' will now map to moving right
	case 'd': key_d_down = true; break; // 'd' will now map to moving left
	case ' ': key_space_down = true; break;
	case 'c': key_c_down = true; break;
	case 27: // Escape key
		glutLeaveMainLoop();
		break;
	}
}

void processKeyUp(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 'w': key_w_down = false; break;
	case 's': key_s_down = false; break;
	case 'a': key_a_down = false; break;
	case 'd': key_d_down = false; break;
	case ' ': key_space_down = false; break;
	case 'c': key_c_down = false; break;
	}
}

void processSpecialKeyDown(int key, int x, int y) {
    if (key == GLUT_KEY_SHIFT_L || key == GLUT_KEY_SHIFT_R) {
        key_shift_down = true;
    }
    // Can add other special keys here if needed (e.g., F1, Arrows)
}

void processSpecialKeyUp(int key, int x, int y) {
    if (key == GLUT_KEY_SHIFT_L || key == GLUT_KEY_SHIFT_R) {
        key_shift_down = false;
    }
}

void processMouseButton(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        mouse_left_click_event = true;
    }
}

void gameLoopUpdate() // Renamed from updateMovement
{
    float currentTime = glutGet(GLUT_ELAPSED_TIME);
    float deltaTime = (currentTime - lastFrameTime) / 1000.0f; 
    lastFrameTime = currentTime;

    // 1. Handle Player Movement
    float currentBaseSpeed = moveSpeed;
    if (key_shift_down) { 
        currentBaseSpeed *= sprintMultiplier;
    }
    float currentFrameMoveSpeed = currentBaseSpeed * deltaTime; 
    float yawRad_player = cameraYaw * (M_PI / 180.0f);
    float pitchRad_player = cameraPitch * (M_PI / 180.0f); 
    float playerForwardX = cos(yawRad_player) * cos(pitchRad_player);
    float playerForwardZ = sin(yawRad_player) * cos(pitchRad_player);
    // Note: For pure XZ plane movement, Y component of forward could be ignored here for W/S.
    // If W/S should also move up/down based on pitch, add playerForwardY * currentFrameMoveSpeed to cameraY.
    // float playerForwardY = sin(pitchRad_player); 

    float playerRightX = sin(yawRad_player); 
    float playerRightZ = -cos(yawRad_player); 

    bool player_moved = false;
    if (key_w_down) { cameraX += playerForwardX * currentFrameMoveSpeed; cameraZ += playerForwardZ * currentFrameMoveSpeed; player_moved = true;}
    if (key_s_down) { cameraX -= playerForwardX * currentFrameMoveSpeed; cameraZ -= playerForwardZ * currentFrameMoveSpeed; player_moved = true;}
    if (key_a_down) { cameraX += playerRightX * currentFrameMoveSpeed; cameraZ += playerRightZ * currentFrameMoveSpeed; player_moved = true;}
    if (key_d_down) { cameraX -= playerRightX * currentFrameMoveSpeed; cameraZ -= playerRightZ * currentFrameMoveSpeed; player_moved = true;}
    if (key_space_down) { cameraY += currentFrameMoveSpeed; player_moved = true;}
    if (key_c_down) { cameraY -= currentFrameMoveSpeed; player_moved = true;}

    // 2. Handle Projectile Spawning
    if (mouse_left_click_event) {
        mouse_left_click_event = false;

        float fireYawRad = cameraYaw * (M_PI / 180.0f);
        float firePitchRad = cameraPitch * (M_PI / 180.0f);

        Projectile p;
        p.dirX = cos(fireYawRad) * cos(firePitchRad);
        p.dirY = sin(firePitchRad);
        p.dirZ = sin(fireYawRad) * cos(firePitchRad);
        normalizeVector(p.dirX, p.dirY, p.dirZ);

        // Calculate camera's local axes for offsetting the projectile start point
        float camRightX, camRightY, camRightZ;
        float camUpX, camUpY, camUpZ;
        float worldUpX = 0.0f, worldUpY = 1.0f, worldUpZ = 0.0f;

        // Right vector: cross(forward, worldUp)
        camRightX = p.dirY * worldUpZ - p.dirZ * worldUpY;
        camRightY = p.dirZ * worldUpX - p.dirX * worldUpZ;
        camRightZ = p.dirX * worldUpY - p.dirY * worldUpX;
        normalizeVector(camRightX, camRightY, camRightZ);

        // Up vector: cross(right, forward)
        camUpX = camRightY * p.dirZ - camRightZ * p.dirY;
        camUpY = camRightZ * p.dirX - camRightX * p.dirZ;
        camUpZ = camRightX * p.dirY - camRightY * p.dirX;
        normalizeVector(camUpX, camUpY, camUpZ);
        
        // Offsets in camera's local space to approximate gun barrel tip
        float gunTipOffsetX_camSpace = 0.35f; // Tuned: slightly less right
        float gunTipOffsetY_camSpace = -0.25f; // Tuned: slightly lower
        float gunTipOffsetZ_camSpace = -0.5f; // Tuned: Start from gun model's Z pos, not beyond
                                             // This means relative to camera's position and orientation

        // Projectile starts from a point offset from camera, along its local axes
        p.x = cameraX + camRightX * gunTipOffsetX_camSpace + camUpX * gunTipOffsetY_camSpace + p.dirX * gunTipOffsetZ_camSpace;
        p.y = cameraY + camRightY * gunTipOffsetX_camSpace + camUpY * gunTipOffsetY_camSpace + p.dirY * gunTipOffsetZ_camSpace;
        p.z = cameraZ + camRightZ * gunTipOffsetX_camSpace + camUpZ * gunTipOffsetY_camSpace + p.dirZ * gunTipOffsetZ_camSpace;
        
        p.active = true;
        p.lifeRemaining = PROJECTILE_MAX_LIFETIME;
        projectiles.push_back(p);
    }

    // 3. Update Active Projectiles
    for (size_t i = 0; i < projectiles.size(); ++i) {
        if (projectiles[i].active) {
            projectiles[i].x += projectiles[i].dirX * PROJECTILE_SPEED * deltaTime;
            projectiles[i].y += projectiles[i].dirY * PROJECTILE_SPEED * deltaTime;
            projectiles[i].z += projectiles[i].dirZ * PROJECTILE_SPEED * deltaTime;
            projectiles[i].lifeRemaining -= deltaTime;
            if (projectiles[i].lifeRemaining <= 0.0f) {
                projectiles[i].active = false;
            }
        }
    }

    // Remove inactive projectiles (simple approach)
    projectiles.erase(std::remove_if(projectiles.begin(), projectiles.end(),
                                     [](const Projectile& p) { return !p.active; }),
                      projectiles.end());

    glutPostRedisplay(); // Always redisplay if game logic is running
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
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f); 
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 

	// --- Setup Main Camera for 3D Scene ---
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	float yawRad = cameraYaw * (M_PI / 180.0f);
	float pitchRad = cameraPitch * (M_PI / 180.0f);
	float lookX = cos(yawRad) * cos(pitchRad);
	float lookY = sin(pitchRad);
	float lookZ = sin(yawRad) * cos(pitchRad);
	gluLookAt(cameraX, cameraY, cameraZ,
		cameraX + lookX, cameraY + lookY, cameraZ + lookZ,
		0.0f, 1.0f, 0.0f); 

	// --- Draw Main 3D Scene (Corridors and Axes) ---
	glLineWidth(2.0f); 
	glBegin(GL_LINES);
	glColor3f(1.0f, 0.0f, 0.0f); glVertex3f(0.0f, 0.0f, 0.0f); glVertex3f(2.0f, 0.0f, 0.0f); 
	glColor3f(0.0f, 1.0f, 0.0f); glVertex3f(0.0f, 0.0f, 0.0f); glVertex3f(0.0f, 2.0f, 0.0f); 
	glColor3f(0.0f, 0.0f, 1.0f); glVertex3f(0.0f, 0.0f, 0.0f); glVertex3f(0.0f, 0.0f, 2.0f); 
	glEnd();
	glLineWidth(1.0f); 
    float corridorWidth = 4.0f; float corridorHeight = 3.0f;
    float floor_r = 0.4f, floor_g = 0.4f, floor_b = 0.4f;
    float ceil_r = 0.6f, ceil_g = 0.6f, ceil_b = 0.6f;
    float c1_length = 20.0f;
    drawCorridorSegment(corridorWidth, corridorHeight, c1_length, 0.8f, 0.7f, 0.5f, floor_r, floor_g, floor_b, ceil_r, ceil_g, ceil_b);
    glPushMatrix();
    glTranslatef(0.0f, 0.0f, c1_length); 
    glRotatef(90.0f, 0.0f, 1.0f, 0.0f); 
    float c2_length = 15.0f;
    drawCorridorSegment(corridorWidth, corridorHeight, c2_length, 0.5f, 0.8f, 0.5f, floor_r, floor_g, floor_b, ceil_r, ceil_g, ceil_b);
    glPushMatrix();
    glTranslatef(0.0f, 0.0f, c2_length); 
    float c3_length = 18.0f;
    drawCorridorSegment(corridorWidth, corridorHeight, c3_length, 0.5f, 0.6f, 0.8f, floor_r, floor_g, floor_b, ceil_r, ceil_g, ceil_b);
    glPushMatrix();
    glTranslatef(0.0f, 0.0f, c3_length); 
    glRotatef(-90.0f, 0.0f, 1.0f, 0.0f); 
    float c4_length = 12.0f;
    drawCorridorSegment(corridorWidth, corridorHeight, c4_length,0.8f, 0.5f, 0.5f, floor_r, floor_g, floor_b, ceil_r, ceil_g, ceil_b);
    glPopMatrix(); 
    glPopMatrix(); 
    glPopMatrix(); 

    // --- Draw 3D Held Gun Model ---
    glClear(GL_DEPTH_BUFFER_BIT); 
    glPushMatrix(); 
    glLoadIdentity(); 
    glTranslatef(0.25f, -0.20f, -0.7f); 
    draw3DGunModel();
    glPopMatrix(); 

    // --- Draw Active Projectiles ---
    for (const auto& p : projectiles) {
        if (p.active) {
            glPushMatrix();
            glTranslatef(p.x, p.y, p.z);
            glColor3f(1.0f, 1.0f, 0.0f); // Yellow
            glutSolidSphere(PROJECTILE_RADIUS, 8, 8); 
            glPopMatrix();
        }
    }

    // --- Draw 2D Gun Overlay (existing logic) --- 
    glClear(GL_DEPTH_BUFFER_BIT); 
    int winWidth = glutGet(GLUT_WINDOW_WIDTH);
    int winHeight = glutGet(GLUT_WINDOW_HEIGHT);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix(); 
    glLoadIdentity();
    gluOrtho2D(0.0, (GLdouble)winWidth, 0.0, (GLdouble)winHeight); 
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix(); 
    glLoadIdentity(); 
    glTranslatef((float)winWidth - 120.0f, 30.0f, 0.0f);
    drawGun(); 
    glPopMatrix(); 
    glMatrixMode(GL_PROJECTION);
    glPopMatrix(); 
    glMatrixMode(GL_MODELVIEW); 

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

    lastFrameTime = glutGet(GLUT_ELAPSED_TIME); // Initialize lastFrameTime

	glutKeyboardFunc(processKeyDown);
	glutKeyboardUpFunc(processKeyUp);
	glutSpecialFunc(processSpecialKeyDown); // Added for Shift (and other special keys) down
	glutSpecialUpFunc(processSpecialKeyUp); // Added for Shift (and other special keys) up
	glutMouseFunc(processMouseButton); // Added mouse button callback
	glutReshapeFunc(framebuffer_size_callback);
	glutDisplayFunc(display);
	glutPassiveMotionFunc(mouseMotion); // For continuous mouse tracking
	glutIdleFunc(gameLoopUpdate); // Renamed from updateMovement

	glutSetCursor(GLUT_CURSOR_NONE); // Hide the mouse cursor

	glutMainLoop();

	return 0;
}