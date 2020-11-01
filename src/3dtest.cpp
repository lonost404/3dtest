#define GLM_ENABLE_EXPERIMENTAL
#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <GL/gl.h>
#include <vector>
#include <map>
#include <math.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtc/matrix_transform.hpp>

const char *vertexShaderSource = R"(#version 320 es
#ifdef GL_ES
precision mediump float;
#endif

layout (location = 0) in vec3 aPos;

uniform vec3 objectPos;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform float time;

out vec3 pos;

void main()
{
   pos = objectPos;
   gl_Position = view * projection * vec4(aPos + objectPos, 1.0f);
})";

const char *fragmentShaderSource = R"(#version 320 es
#ifdef GL_ES
precision mediump float;
#endif

out vec4 FragColor;
in vec3 pos;
uniform float time;

void main()
{
    FragColor = vec4((cos(time + pos.x)/2.0f) + 0.5f, (cos(time+40.0f + pos.x)/2.0f) + 0.5f, (cos(time+10.0f + pos.x)/2.0f) + 0.5f, 1.0f);
}

)";
glm::vec3 playerDirection = glm::vec3(0.0f);
bool pressedKeys[5];
//keys
#define KEY_W 0
#define KEY_A 1
#define KEY_S 2
#define KEY_D 3
#define KEY_SPACE 4
#define KEY_CONTROL 5

glm::vec3 playerPos   = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 playerVel   = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp    = glm::vec3(0.0f, 1.0f, 0.0f);

bool firstMouse = true;
float yaw = 270.f;
float pitch = 0;
int lastX;
int lastY;
bool isJumping = true;
float deltaTime = 0.0f;

class SceneObject {
public:
	SceneObject(int objId, glm::vec3 pos) :
		objId(objId),
		pos(pos) {}

	int objId;
	glm::vec3 pos;
};


GLuint compileshaders(const char * vShader, const char* fShader) {

	GLuint vertexShader;
	GLuint fragmentShader;
	GLuint shaderProgram;
	GLint status;
	char infoLog[512];

	/* Create and compile vertex shader */
	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vShader, NULL);
	glCompileShader(vertexShader);
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &status);

	if (!status) {
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
		std::cout<<"Failed compiling vertex shader"<<std::endl;
		std::cout<<infoLog<<std::endl;
		exit(1);
	}
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fShader, NULL);
	glCompileShader(fragmentShader);
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &status);

	if (!status) {
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
		std::cout<<"Failed compiling fragment shader"<<std::endl;
		std::cout<<infoLog<<std::endl;
		exit(1);
	}
	/* Shader Program */
	shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);

	glDetachShader(shaderProgram, vertexShader);
	glDetachShader(shaderProgram, fragmentShader);
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &status);
	if(!status) {
	    glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
	    std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
	    exit(1);
	}

	return shaderProgram;
}
GLuint createObject(std::vector<float> vertices, std::vector<int> elements) {

	/* Make gpu buffer */
	GLuint vbo;
	GLuint ebo;
	GLuint vao;

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	

	//glGenVertexArrays(1, &vao);

	/* Bind to buffer */

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER,
		vertices.size() * sizeof(float),
		vertices.data(),
		GL_STATIC_DRAW
	);

	glVertexAttribPointer(
		0,
		3,
		GL_FLOAT,
		GL_FALSE,
		sizeof(float) * 3,
		nullptr
	);

	glGenBuffers(1, &ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER,
		elements.size()*sizeof(int),
		elements.data(),
		GL_STATIC_DRAW);


	glEnableVertexAttribArray(0);


	return vao;
}

void setPlayerDirection(int key, bool release) {

}
void playerJump() {
	if (!isJumping) {
		if (pressedKeys[KEY_SPACE]) {
			isJumping = true;
			playerVel.y = 0.0004f;
		}
	}

}

void playerGravity() {
	playerVel.y -=  0.001f * deltaTime;
	playerPos.y += playerVel.y;
	if (playerPos.y <= 0.0f) {
		playerPos.y = 0.0f;
		playerVel.y = 0;
		isJumping = false;
		playerJump();

	}
}
void updatePlayerDirection() {

	playerDirection[0] = 0.0f;
	playerDirection[1] = 0.0f;
	playerDirection[2] = 0.0f;

	if (pressedKeys[KEY_W]) {
		playerDirection[2] = pressedKeys[KEY_S] ? 0.0f : 1.0f;
	} else if (pressedKeys[KEY_S]) {
		playerDirection[2] = pressedKeys[KEY_W] ? 0.0f : -1.0f;

	}
	if (pressedKeys[KEY_A]) {
		playerDirection[0] = pressedKeys[KEY_D] ? 0.0f : -1.0f;
	}  else if (pressedKeys[KEY_D]) {
		playerDirection[0] = pressedKeys[KEY_A] ? 0.0f : 1.0f;
	}

	if (glm::abs(playerDirection[0]) == 1.f && glm::abs(playerDirection[2]) == 1.f) {
		playerDirection[0] = 0.707 * playerDirection[0];
		playerDirection[2] = 0.707 * playerDirection[2];;
	}

	if (pressedKeys[KEY_SPACE] && !isJumping) {
		playerJump();
	}
	if (pressedKeys[KEY_CONTROL]) {
		playerDirection[0] *= 2.f;
		playerDirection[2] *= 2.f;
	}

	//




}
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {

	bool release = action != GLFW_RELEASE;
	switch(key) {
		case GLFW_KEY_W: pressedKeys[KEY_W] = true && release; break;
		case GLFW_KEY_A: pressedKeys[KEY_A] = true && release; break;
		case GLFW_KEY_S: pressedKeys[KEY_S] = true && release; break;
		case GLFW_KEY_D: pressedKeys[KEY_D] = true && release; break;
		case GLFW_KEY_LEFT_CONTROL: pressedKeys[KEY_CONTROL] = true && release; break;
		case GLFW_KEY_SPACE: pressedKeys[KEY_SPACE] = true && release; break;
		case GLFW_KEY_ESCAPE: if (action == GLFW_PRESS) { glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL); } break;
	}
	updatePlayerDirection();
}


void cursorPositionCallback(GLFWwindow* window, double xpos, double ypos) {
	if (firstMouse) {
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos;
	lastX = xpos;
	lastY = ypos;
	float sensitivity = 0.1f;
	xoffset *= sensitivity;
	yoffset *= sensitivity;

	yaw   += xoffset;
	pitch += yoffset;

	pitch = glm::clamp(pitch, -89.0f, 89.0f);

	glm::vec3 direction;
	direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	direction.y = sin(glm::radians(pitch));
	direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	cameraFront = direction;

}

int main(int argc, char **argv) {

	GLFWwindow* window;

	/* Initialize the library */
	if (!glfwInit())
		return -1;

	const GLFWvidmode * mode = glfwGetVideoMode(glfwGetPrimaryMonitor());

	int width = 640; //mode->width;
	int height = 430; //mode->height;
	/* Create a windowed mode window and its OpenGL context */
	window = glfwCreateWindow(width, height, "3dtest", NULL, NULL);

	if (!window) {
		glfwTerminate();
		return -1;
	}

	/* Make the window's context current */
	glfwMakeContextCurrent(window);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);


	//
	/* Use modern opengl for glew */
	glewExperimental = GL_TRUE;
	glewInit();
	
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS); 
	
	//createObject(triangle_vertices, sizeof(triangle_vertices));
	std::vector<SceneObject*> sceneObjs;

	GLuint cube = createObject({
	  // Cara delantera
	  -1.0, -1.0,  1.0,
	   1.0, -1.0,  1.0,
	   1.0,  1.0,  1.0,
	  -1.0,  1.0,  1.0,

	  // Cara trasera
	  -1.0, -1.0, -1.0,
	  -1.0,  1.0, -1.0,
	   1.0,  1.0, -1.0,
	   1.0, -1.0, -1.0,

	  // Top face
	  -1.0,  1.0, -1.0,
	  -1.0,  1.0,  1.0,
	   1.0,  1.0,  1.0,
	   1.0,  1.0, -1.0,

	  // Bottom face
	  -1.0, -1.0, -1.0,
	   1.0, -1.0, -1.0,
	   1.0, -1.0,  1.0,
	  -1.0, -1.0,  1.0,

	  // Right face
	   1.0, -1.0, -1.0,
	   1.0,  1.0, -1.0,
	   1.0,  1.0,  1.0,
	   1.0, -1.0,  1.0,

	  // Left face
	  -1.0, -1.0, -1.0,
	  -1.0, -1.0,  1.0,
	  -1.0,  1.0,  1.0,
	  -1.0,  1.0, -1.0
	}, {
	  0,  1,  2,      0,  2,  3,    // enfrente
	  4,  5,  6,      4,  6,  7,    // atrás
	  8,  9,  10,     8,  10, 11,   // arriba
	  12, 13, 14,     12, 14, 15,   // fondo
	  16, 17, 18,     16, 18, 19,   // derecha
	  20, 21, 22,     20, 22, 23    // izquierda

	});

	GLuint triangle = createObject({
		-0.5f,-0.5f, 0.0,
		 0.5f,-0.5f, 0.0,
		 0.0f, 0.5f, 0.0
	}, {
		 0,1,2
	});
	for (int x=0; x<10; x+=2) {
		for (int y=0; y<10; y+=2) {
			sceneObjs.push_back(new SceneObject(cube, {x, y, 0}));
			//sceneObjs.push_back(new SceneObject(triangle, {x-15.f, y, 0}));
		}

	}

	GLuint shaderProgram = compileshaders(vertexShaderSource, fragmentShaderSource);
	GLint viewUniform = glGetUniformLocation(shaderProgram, "view");
	GLint projectionUniform = glGetUniformLocation(shaderProgram, "projection");
	GLint modelUniform = glGetUniformLocation(shaderProgram, "model");
	GLint timeUniform = glGetUniformLocation(shaderProgram, "time");
	GLint objectPosUniform = glGetUniformLocation(shaderProgram, "objectPos");

	glm::mat4 viewMat = glm::mat4(1.0f);
	glm::mat4 projectionMat = glm::mat4(1.0f);

	playerPos = glm::vec3(0.0f, 1.0f, 3.0f);


	float velocity = 100.f;
	//viewMat = glm::perspective(glm::radians(45.0f), ((GLfloat)width / (GLfloat)height), 150.0f);

	viewMat = glm::perspective(glm::radians(90.0f), (float)width / (float)height, 0.001f, 100.0f);

	glUniformMatrix4fv(viewUniform, 1, GL_FALSE, &viewMat[0][0]);
	glUniformMatrix4fv(projectionUniform, 1, GL_FALSE, &projectionMat[0][0]);
	//glUniformMatrix4fv(modelUniform, 1, GL_FALSE, &modelMat[0][0]);
	glUseProgram(shaderProgram);


    glfwSetKeyCallback(window, keyCallback);
    glfwSetCursorPosCallback(window, cursorPositionCallback);

    glfwSwapInterval(1);
	/* Loop until the user closes the window */
    int frames = 0;
    float playerSpeed = 40.f;
	float lastTime = 0.1f;
    double nextTime = glfwGetTime() + 1.f;

	while (!glfwWindowShouldClose(window)) {
		/* Render here */
		frames++;

		deltaTime = glfwGetTime() - lastTime;
		lastTime = glfwGetTime();

		if (lastTime >= nextTime) {
			nextTime = lastTime + 1.f;
			std::cout<<frames<<std::endl;
			frames = 0;
		}

		glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glm::vec3 cameraDir = glm::vec3(cameraFront.x, 0.0f, cameraFront.z);

		playerVel += cameraDir * (playerDirection.z * playerSpeed);
		playerVel += glm::cross(cameraDir, cameraUp) * (playerDirection.x * playerSpeed);
		playerVel.x *= 0.1f;
		playerVel.z *= 0.1f;

		if (isJumping) {
			playerGravity();
		}
	

		glm::vec3 playerNewPos = playerPos + playerVel * deltaTime;

		for (SceneObject* obj : sceneObjs) {
			glUniform3fv(objectPosUniform, 1, (const float*)&obj->pos[0]);

			if ((playerNewPos.x >= obj->pos.x - 1 && playerNewPos.x <= obj->pos.x + 1) &&
				playerNewPos.z >= obj->pos.z - 1 && playerNewPos.z <= obj->pos.z + 1) {
				playerVel.x = (playerPos.x - obj->pos.x) * 0.01f;
				playerVel.z = (playerPos.z - obj->pos.z) * 0.01f;

			} 
			std::cout<<glm::to_string(playerVel)<<std::endl;

			if (obj->objId == cube) {
				glBindVertexArray(cube);
				glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
			} else if (obj->objId == triangle) {
				glBindVertexArray(triangle);
				glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0);
			}
		}
		
		playerPos += playerVel * deltaTime; 

		projectionMat = glm::lookAt(
			playerPos, // La posición de tu cámara en el espacio mundo
			playerPos + cameraFront, // Hacia donde quieres mirar, en el espacio mundo
			cameraUp   // Probablemente glm::vec3(0,1,0), porque (0,-1,0) te haría mirar cabeza abajo, aunque puede ser divertido.
		);

		glUniformMatrix4fv(viewUniform, 1, GL_FALSE, &viewMat[0][0]);
		glUniformMatrix4fv(projectionUniform, 1, GL_FALSE, &projectionMat[0][0]);
		glUniform1f(timeUniform, glfwGetTime());
		/* Swap front and back buffers */

		glfwSwapBuffers(window);

		/* Poll for and process events */
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}
