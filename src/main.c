#include <stdio.h>
#include <GLFW/glfw3.h>
#include <unistd.h>
#include <stdint.h>
#include <math.h>
#include <stdbool.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#define WINDOW_WIDTH 512
#define WINDOW_HEIGHT 512

typedef struct {
	int32_t x, y, z;
} v3i;

typedef struct {
	float x, y, z;
} v3;

void v3_normalize(v3* vec)
{
	float length = sqrtf(vec->x*vec->x + vec->y*vec->y + vec->z*vec->z);
	vec->x /= length;
	vec->y /= length;
	vec->z /= length;
}

void v3_axis_rotate(v3* axis, float theta, v3* v)
{
	v->x = 
		v->x*(cos(theta) + (axis->x*axis->x*(1 - cos(theta)))) +
		v->y*((axis->x*axis->y*(1 - cos(theta))) - axis->z*sin(theta)) +
		v->z*(axis->x*axis->y*(1 - cos(theta)) + axis->y*sin(theta));
	v->y =
		v->x*(axis->y*axis->x*(1 - cos(theta)) + axis->z*sin(theta)) +
		v->y*(cos(theta) + axis->y*axis->y*(1 - cos(theta))) +
		v->z*(axis->y*axis->z*(1 - cos(theta)) - axis->x*sin(theta));
	v->z =
		v->x*(axis->z*axis->x*(1 - cos(theta)) + axis->y*sin(theta)) +
		v->y*(axis->z*axis->y*(1 - cos(theta)) + axis->x*sin(theta)) +
		v->z*(cos(theta) + axis->z*axis->z*(1 - cos(theta)));
}

#define WORLD_WIDTH 32
#define WORLD_HEIGHT 32
#define WORLD_LENGTH 32

uint32_t World [WORLD_WIDTH][WORLD_HEIGHT][WORLD_LENGTH] = {
	[16][16][16] = 1,
	[17][17][16] = 2,
	[17][16][16] = 1,
	[16][17][16] = 1,
	[14][14][14] = 1,
	[17][14][14] = 1,
	[14][17][14] = 1,
	[18][16][0] = 1,
	[14][16][0] = 1,
	[0][16][18] = 1,
	[0][16][14] = 1,
};

v3 player_pos = {0,0,0};
v3 player_dir = {1,0,0};
float pitch = 0;
float yaw = 0;
v3 player_up = {0,1,0};

const GLfloat cube_vertices[] = {
	-0.5f, -0.5f, 0.5f, 	 0.5f, -0.5f, 0.5f, 	 0.5f, 0.5f, 0.5f,  	-0.5f, 0.5f, 0.5f,
	-0.5f, 0.5f, -0.5f, 	 0.5f, 0.5f, -0.5f, 	 0.5f, -0.5f, -0.5f,	-0.5f, -0.5f, -0.5f,
	 0.5f, -0.5f, 0.5f, 	 0.5f, -0.5f, -0.5f,	 0.5f, 0.5f, -0.5f, 	 0.5f, 0.5f, 0.5f,
	-0.5f, -0.5f, -0.5f,	-0.5f, -0.5f, 0.5f, 	-0.5f, 0.5f, 0.5f,  	-0.5f, 0.5f, -0.5f
};

GLfloat vertices[] = {
	-0.5f, -0.5f, 0.5f, 	 0.5f, -0.5f, 0.5f, 	 0.5f, 0.5f, 0.5f,  	-0.5f, 0.5f, 0.5f,
	-0.5f, 0.5f, -0.5f, 	 0.5f, 0.5f, -0.5f, 	 0.5f, -0.5f, -0.5f,	-0.5f, -0.5f, -0.5f,
	 0.5f, -0.5f, 0.5f, 	 0.5f, -0.5f, -0.5f,	 0.5f, 0.5f, -0.5f, 	 0.5f, 0.5f, 0.5f,
	-0.5f, -0.5f, -0.5f,	-0.5f, -0.5f, 0.5f, 	-0.5f, 0.5f, 0.5f,  	-0.5f, 0.5f, -0.5f
};

const GLfloat texcoords_cube_side[] = {
	0.0,0.0, 0.0625,0.0, 0.0625,0.0625, 0.0,0.0625,
    0.0,0.0, 0.0625,0.0, 0.0625,0.0625, 0.0,0.0625,
    0.0,0.0, 0.0625,0.0, 0.0625,0.0625, 0.0,0.0625,
    0.0,0.0, 0.0625,0.0, 0.0625,0.0625, 0.0,0.0625
};

GLfloat texcoords[] = {
	0.0,0.0, 0.0625,0.0, 0.0625,0.0625, 0.0,0.0625,
    0.0,0.0, 0.0625,0.0, 0.0625,0.0625, 0.0,0.0625,
    0.0,0.0, 0.0625,0.0, 0.0625,0.0625, 0.0,0.0625,
    0.0,0.0, 0.0625,0.0, 0.0625,0.0625, 0.0,0.0625
};

GLubyte cubeIndices[24] = {
	0,1,2,3,	4,5,6,7,  	3,2,5,4,
	7,6,1,0,	8,9,10,11,	12,13,14,15
};

GLfloat colors[] = {
	1, 0, 0,	1, 0, 0,	1, 0, 0,	1, 0, 0,
	0, 1, 0,	0, 1, 0,	0, 1, 0,	0, 1, 0,
	1, 1, 0,	1, 1, 0,	1, 1, 0,	1, 1, 0,
	0, 0, 1,	0, 0, 1,	0, 0, 1,	0, 0, 1,
	1, 0, 1,	1, 0, 1,	1, 0, 1,	1, 0, 1,
	0, 1, 1,	0, 1, 1,	0, 1, 1,	0, 1, 1,
};

void controls(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if(action == GLFW_REPEAT) {
        switch (key) {
			case GLFW_KEY_ESCAPE:
            	glfwSetWindowShouldClose(window, GL_TRUE);
			case GLFW_KEY_W:
				player_pos.x -= player_dir.x;
				player_pos.z -= player_dir.z;
				break;
			case GLFW_KEY_S:
				player_pos.x += player_dir.x;
				player_pos.z += player_dir.z;
				break;
			case GLFW_KEY_D:
				player_pos.x += cos(M_PI/2)*player_dir.x + sin(M_PI/2)*player_dir.z;
				player_pos.z += cos(M_PI/2)*player_dir.z - sin(M_PI/2)*player_dir.x;
				break;
			case GLFW_KEY_A:
				player_pos.x -= cos(M_PI/2)*player_dir.x + sin(M_PI/2)*player_dir.z;
				player_pos.z -= cos(M_PI/2)*player_dir.z - sin(M_PI/2)*player_dir.x;
				break;
			case GLFW_KEY_UP:;
				v3_axis_rotate(&(v3){
					cos(M_PI/2)*player_dir.x + sin(M_PI/2)*player_dir.z,
					0,
					cos(M_PI/2)*player_dir.z - sin(M_PI/2)*player_dir.x
				},-0.1,&player_dir);
				v3_normalize(&player_dir);
				break;
			case GLFW_KEY_DOWN:;
				v3_axis_rotate(&(v3){
					cos(M_PI/2)*player_dir.x + sin(M_PI/2)*player_dir.z,
					0,
					cos(M_PI/2)*player_dir.z - sin(M_PI/2)*player_dir.x
				},0.1,&player_dir);
				v3_normalize(&player_dir);
				break;
			case GLFW_KEY_LEFT:
				player_dir.x = cos(0.1)*player_dir.x + sin(0.1)*player_dir.z;
				player_dir.z = cos(0.1)*player_dir.z - sin(0.1)*player_dir.x;
				break;
			case GLFW_KEY_RIGHT:
				player_dir.x = cos(-0.1)*player_dir.x + sin(-0.1)*player_dir.z;
				player_dir.z = cos(-0.1)*player_dir.z - sin(-0.1)*player_dir.x;
				break;
		}
	}
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	static float lastX  = WINDOW_WIDTH  / 2.0f;
	static float lastY  = WINDOW_HEIGHT / 2.0f;
	static bool firstrun = true;

	if (firstrun) {
		lastX = xpos;
		lastY = ypos;
		firstrun = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos;
	lastX = xpos;
	lastY = ypos;

	#define SENSITIVITY 0.001f
	xoffset *= SENSITIVITY;
	yoffset *= SENSITIVITY;

	yaw   += xoffset;
	pitch += yoffset;

	if (pitch > (M_PI / 2.01f))
    	pitch = M_PI / 2.01f;
	else if (pitch < -(M_PI / 2.01f))
    	pitch = -(M_PI / 2.01f);

	player_dir.x = cos(pitch) * cos(yaw);
	player_dir.y = cos(pitch) * sin(yaw);
	player_dir.z = sin(pitch);
	v3_normalize(&player_dir);
}

GLFWwindow* initWindow(const int resX, const int resY)
{
    if(!glfwInit())
    {
        fprintf(stderr, "Failed to initialize GLFW\n");
        return NULL;
    }
    glfwWindowHint(GLFW_SAMPLES, 4); // 4x antialiasing

    // Open a window and create its OpenGL context
    GLFWwindow* window = glfwCreateWindow(resX, resY, "TEST", NULL, NULL);

    if(window == NULL)
    {
        fprintf(stderr, "Failed to open GLFW window.\n");
        glfwTerminate();
        return NULL;
    }

    glfwMakeContextCurrent(window);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetKeyCallback(window, controls);
	//glfwSetCursorPosCallback(window, mouse_callback);

    // Get info of GPU and supported OpenGL version
    printf("Renderer: %s\n", glGetString(GL_RENDERER));
    printf("OpenGL version supported %s\n", glGetString(GL_VERSION));

    glEnable(GL_DEPTH_TEST); // Depth Testing
    glDepthFunc(GL_LEQUAL);
    glDisable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    return window;
}

GLuint texture;

void drawCubes()
{	
	glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glColor3f(1.0f, 1.0f, 1.0f);
    /* We have a color array and a vertex array */
    glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, vertices);
	glTexCoordPointer(2, GL_FLOAT, 0, texcoords);

    /* Send data : 24 vertices */
	int32_t world_iterator [3];
	for (world_iterator[0] = 0; world_iterator[0] < WORLD_WIDTH; world_iterator[0]++) {
		for (world_iterator[1] = 0; world_iterator[1] < WORLD_HEIGHT; world_iterator[1]++) {
			for (world_iterator[2] = 0; world_iterator[2] < WORLD_LENGTH; world_iterator[2]++) {
				if (World[world_iterator[0]][world_iterator[1]][world_iterator[2]]) {
					for (uint32_t i = 0; i < 48;i++)
						vertices[i] = cube_vertices[i] + ((float)world_iterator[i % 3]-(16.0f)) + ((float*)&player_pos)[i % 3];
					for (uint32_t i = 0;i < 32;i++)
						texcoords[i] = texcoords_cube_side[i] + 0.0625*(World[world_iterator[0]][world_iterator[1]][world_iterator[2]]-1);
					glDrawElements(GL_QUADS, 24, GL_UNSIGNED_BYTE, cubeIndices);
				}
			}
		}
	}

    /* Cleanup states */
    glDisableClientState(GL_VERTEX_ARRAY);
	glDisable(GL_TEXTURE_2D);
}

#define FPS 1/60
double current_time;
double last_time;

void display( GLFWwindow* window )
{
	int textureWidth, textureHeight = 0;
	uint8_t* image = stbi_load("blocks.png", &textureWidth, &textureHeight, &texture, 3);
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, textureWidth, textureHeight, 0,
				GL_RGB, GL_UNSIGNED_BYTE, image);
    while(!glfwWindowShouldClose(window))
    {
        // Scale to window size
        GLint windowWidth, windowHeight;
        glfwGetWindowSize(window, &windowWidth, &windowHeight);
        glViewport(0, 0, windowWidth, windowHeight);

        // Draw stuff
        glClearColor(0, 0, 0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		current_time = glfwGetTime();
		double delta = current_time - last_time;
		if (delta < FPS)
			
		last_time = current_time;
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		gluLookAt(
		0.0f,0.0f,0.0f,
		player_dir.x,player_dir.y,player_dir.z,
		player_up.x,player_up.y,player_up.z);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluPerspective(50.0f,1.0f,0.1f,50.0f);
        drawCubes();

        // Update Screen
        glfwSwapBuffers(window);

        // Check for any input, or window movement
        glfwPollEvents();
    }
}

int main(int argc, char** argv)
{
    GLFWwindow* window = initWindow(WINDOW_WIDTH, WINDOW_HEIGHT);
    if( NULL != window )
    {
        display( window );
    }
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}