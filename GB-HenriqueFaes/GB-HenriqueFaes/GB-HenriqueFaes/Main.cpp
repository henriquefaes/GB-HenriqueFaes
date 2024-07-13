//Henrique Fernandes Faés
#include <iostream>
#include <string>
#include <assert.h>

using namespace std;

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <SOIL.h>
#include "Shader.h"
#include "Camera.h"
#include "Scene.h"

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void do_movement();
GLuint LoadTexture(const char* file);


const GLuint WIDTH = 800, HEIGHT = 600;

Camera camera(glm::vec3(0.0f, 0.0f, 0.0f));
float lastX = WIDTH / 2.0f;
float lastY = HEIGHT / 2.0f;
bool    keys[1024];

float deltaTime = 0.0f;
float lastFrame = 0.0f;

float posX = 0.0f, posY = 0.0f, posZ = 0.0f;
float scl = 1.0f;
float rot = 0.0f;
int x = 0, y = 0, z = 0;

int main()
{
	glfwInit();

	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "GB - Henrique Faes", nullptr, nullptr);
	glfwMakeContextCurrent(window);

	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glewExperimental = GL_TRUE;

	glewInit();

	const GLubyte* renderer = glGetString(GL_RENDERER);
	const GLubyte* version = glGetString(GL_VERSION);
	cout << "Renderer: " << renderer << endl;
	cout << "OpenGL version supported " << version << endl;

	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	glViewport(0, 0, width, height);

	Scene scene("../../cena.txt");

	camera.Position = scene.getCamPos();

	float lightX = scene.getLightX();
	float lightY = scene.getLightY();
	float lightZ = scene.getLightZ();

	float lightAmbX = scene.getLightAmbX();
	float lightAmbY = scene.getLightAmbY();
	float lightAmbZ = scene.getLightAmbZ();

	float lightDifX = scene.getLightDifX();
	float lightDifY = scene.getLightDifY();
	float lightDifZ = scene.getLightDifZ();

	float lightSpecX = scene.getLightSpecX();
	float lightSpecY = scene.getLightSpecY();
	float lightSpecZ = scene.getLightSpecZ();

	float bgColorR = scene.getBgColorR();
	float bgColorG = scene.getBgColorG();
	float bgColorB = scene.getBgColorB();

	int numVAOs = 0;

	for (int i = 0; i < scene.getObjects().size(); i++)
	{
		numVAOs += scene.getObjects()[i].getObj().getGroups()->size();
	}

	const char* shaderVS = "../shaders/lighting.vs";
	const char* shaderFrag = "../shaders/lighting.frag";


	Shader ourShader(shaderVS, shaderFrag);

	GLuint vVBO, nVBO, tVBO;
	GLuint* VAO = new GLuint[numVAOs];
	GLuint* textures = new GLuint[numVAOs];
	int v = 0;
	for (int i = 0; i < scene.getObjects().size(); i++)
	{
		Object object = scene.getObjects()[i];
		Obj obj = object.getObj();

		for (int i = 0; i < obj.getGroups()->size(); i++)
		{		
			glGenBuffers(1, &vVBO);
			glBindBuffer(GL_ARRAY_BUFFER, vVBO);
			glBufferData(GL_ARRAY_BUFFER, sizeof(float) * obj.getGroups()->at(i).vertices.size(), &obj.getGroups()->at(i).vertices[0], GL_STATIC_DRAW);
	
			if (obj.getGroups()->at(i).normals.size() > 0) {
				glGenBuffers(1, &nVBO);
				glBindBuffer(GL_ARRAY_BUFFER, nVBO);
				glBufferData(GL_ARRAY_BUFFER, sizeof(float) * obj.getGroups()->at(i).normals.size(), &obj.getGroups()->at(i).normals[0], GL_STATIC_DRAW);
			}

			if (obj.getGroups()->at(i).textures.size() > 0) {
				glGenBuffers(1, &tVBO);
				glBindBuffer(GL_ARRAY_BUFFER, tVBO);
				glBufferData(GL_ARRAY_BUFFER, sizeof(float) * obj.getGroups()->at(i).textures.size(), &obj.getGroups()->at(i).textures[0], GL_STATIC_DRAW);
			}

			glGenVertexArrays(1, &VAO[v]);
			glBindVertexArray(VAO[v]);

			glBindBuffer(GL_ARRAY_BUFFER, vVBO);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), NULL);
			glEnableVertexAttribArray(0);

			if (obj.getGroups()->at(i).normals.size() > 0) {
				glBindBuffer(GL_ARRAY_BUFFER, nVBO);
				glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), NULL);
				glEnableVertexAttribArray(1);
			}

			if (obj.getGroups()->at(i).textures.size() > 0) {
				glBindBuffer(GL_ARRAY_BUFFER, tVBO);
				glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), NULL);
				glEnableVertexAttribArray(2);
				string texturePath = "../objs/" + obj.getGroups()->at(i).material.getMapKd();
				int texture = LoadTexture(texturePath.c_str());
				textures[v] = texture;
			} else {
				textures[v] = 0;
			}

			v++;
		}	
	}

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	for (int i = 0; i < numVAOs; i++)
		glBindVertexArray(i);

	glEnable(GL_DEPTH_TEST);

	GLuint texture;
	string texturePath;

	float t = 0.0f;
	int lastAnimation = 0;

	while (!glfwWindowShouldClose(window))
	{
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		glfwPollEvents();
		do_movement();

		glClearColor(bgColorR, bgColorG, bgColorB, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		ourShader.Use();

		GLint viewPosLoc = glGetUniformLocation(ourShader.Program, "viewPos");

		GLint lightPosLoc = glGetUniformLocation(ourShader.Program, "lightPos");
		GLint lightAmbientLoc = glGetUniformLocation(ourShader.Program, "lightAmbient");
		GLint lightDiffuseLoc = glGetUniformLocation(ourShader.Program, "lightDiffuse");
		GLint lightSpecularLoc = glGetUniformLocation(ourShader.Program, "lightSpecular");

		GLint coefSpecLoc = glGetUniformLocation(ourShader.Program, "coefSpec");

		GLint ambient = glGetUniformLocation(ourShader.Program, "matAmbient");
		GLint diffuse = glGetUniformLocation(ourShader.Program, "matDiffuse");
		GLint specular = glGetUniformLocation(ourShader.Program, "matSpecular");

		
		glUniform3f(lightPosLoc, lightX, lightY, lightZ);
		glUniform3f(lightAmbientLoc, lightAmbX, lightAmbY, lightAmbZ);
		glUniform3f(lightDiffuseLoc, lightDifX, lightDifY, lightDifZ);
		glUniform3f(lightSpecularLoc, lightSpecX, lightSpecY, lightSpecZ);
		glUniform1f(coefSpecLoc, scene.getCoefSpec());
		
		glUniform3f(viewPosLoc, camera.Position.x, camera.Position.y, camera.Position.z);

		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)WIDTH / (float)HEIGHT, 0.1f, 100.0f);		
		glm::mat4 view = camera.GetViewMatrix();
		GLint modelLoc = glGetUniformLocation(ourShader.Program, "model");
		GLint viewLoc = glGetUniformLocation(ourShader.Program, "view");
		GLint projLoc = glGetUniformLocation(ourShader.Program, "projection");
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
		
		v = 0;
		for (int i = 0; i < scene.getObjects().size(); i++)
		{
			Object object = scene.getObjects()[i];
			Obj obj = object.getObj();

			posX = object.getPosX();
			posY = object.getPosY();
			posZ = object.getPosZ();
			scl = object.getScale();
			rot = object.getRot();
			x = object.getX();
			y = object.getY();
			z = object.getZ();

			if (object.getAnimation()->curves.size() > 0) {
				t += 0.01;
				if (t > 1.0f) {
					t = 0.0f;
					lastAnimation++;
					if (lastAnimation >= object.getAnimation()->curves.size())
						lastAnimation = 0;
				}
				
				glm::vec2 p = object.getAnimation()->curves.at(lastAnimation).calc(t);

				posX = p.x;
				posY = p.y;
			}

			glm::mat4 model;

			model = glm::translate(model, glm::vec3(posX, posY, posZ));
			model = glm::rotate(model, rot, glm::vec3(x, y, z));
			model = glm::scale(model, glm::vec3(scl));
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

			for (int i = 0; i < obj.getGroups()->size(); i++) {
				if (obj.getGroups()->at(i).indicesVT.size() > 0) {
					glBindTexture(GL_TEXTURE_2D, textures[v]);
				}

				if (obj.getGroups()->at(i).material.getName() != "") {
					glUniform3f(ambient, obj.getGroups()->at(i).material.getKaX(), obj.getGroups()->at(i).material.getKaY(), obj.getGroups()->at(i).material.getKaZ());
					glUniform3f(diffuse, obj.getGroups()->at(i).material.getKdX(), obj.getGroups()->at(i).material.getKdY(), obj.getGroups()->at(i).material.getKdZ());
					glUniform3f(specular, obj.getGroups()->at(i).material.getKsX(), obj.getGroups()->at(i).material.getKsY(), obj.getGroups()->at(i).material.getKsZ());
				}

				glBindVertexArray(VAO[v]);
				glDrawArrays(GL_TRIANGLES, 0, obj.getGroups()->at(i).vertices.size() / 3);
				v++;
			}
		}
		glfwSwapBuffers(window);
	}
	for (int i = 0; i < numVAOs; i++)
		glDeleteVertexArrays(1, &VAO[i]);
	glDeleteBuffers(1, &vVBO);
	glDeleteBuffers(1, &nVBO);
	glDeleteBuffers(1, &tVBO);
	glfwTerminate();
	return 0;
}

GLuint LoadTexture(const char* file)
{
	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	int width, height;
	unsigned char* image = SOIL_load_image(file, &width, &height, 0, SOIL_LOAD_RGB);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	glGenerateMipmap(GL_TEXTURE_2D);
	SOIL_free_image_data(image);
	glBindTexture(GL_TEXTURE_2D, 0); 
	return textureID;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
			keys[key] = true;
		else if (action == GLFW_RELEASE)
			keys[key] = false;
	}
}

void do_movement()
{
	if (keys[GLFW_KEY_W])
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (keys[GLFW_KEY_S])
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (keys[GLFW_KEY_A])
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (keys[GLFW_KEY_D])
		camera.ProcessKeyboard(RIGHT, deltaTime);
}

bool firstMouse = true;
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	GLfloat xoffset = xpos - lastX;
	GLfloat yoffset = lastY - ypos;

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}