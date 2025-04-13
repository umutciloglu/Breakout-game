#include <iostream>
#include <glad/glad.h>
#include <GL/freeglut.h>

void framebuffer_size_callback(int width, int height)
{
	//Window resized callback.
	glViewport(0, 0, width, height);

}

void processInput(unsigned char key, int x, int y)
{
	//Key and mouse callback.
}

void display()
{
	//Rendering method.

	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	glutSwapBuffers();
}

int main(int argc, char** argv)
{
	glutInit(&argc, argv);

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(800, 600);
	glutCreateWindow("OpenGL Template");

	if (!gladLoadGLLoader((GLADloadproc)glutGetProcAddress)) {
		std::cerr << "[GLAD]: Failed to load OpenGL methods from driver." << std::endl;
		return -1;
	}

	glutKeyboardFunc(processInput);
	glutReshapeFunc(framebuffer_size_callback);
	glutDisplayFunc(display);
	glutMainLoop();

	return 0;
}