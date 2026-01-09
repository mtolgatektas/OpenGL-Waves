#include "playground.h"
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <iostream>


#include <glfw3.h>
GLFWwindow* window;


#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include <common/shader.hpp>
//forward declerations for the camera. I should just do a header...
//TODO: Header Class
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);


// Globals

GLuint MatrixID, ModelMatrixID, ViewMatrixID;
GLuint TimeID, AmpID, FreqID, SpeedID, LightToggleID, ColorToggleID;
GLuint CameraPosID;

// Camera Variables
vec3 cameraPos = vec3(15.0f, 15.0f, 15.0f);
vec3 cameraFront = vec3(-1.0f, -1.0f, -1.0f);
vec3 cameraUp = vec3(0.0f, 1.0f, 0.0f);

bool firstMouse = true;
float yaw = -135.0f;
float pitch = -35.0f;
float lastX = 1024.0f / 2.0f;
float lastY = 768.0f / 2.0f;
float fov = 45.0f;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

// Wave Parameters
float waveAmplitude = 0.5f;
float waveFrequency = 1.0f;
float waveSpeed = 2.0f;
int toggleLighting = 1; // 1 = On, 0 = Off
int toggleColor = 1;    // 1 = Height Color, 0 = Solid

//Grid settings
const int GRID_SIZE = 40; // 40x40 grid
const float GRID_SPACING = 0.5f;

int vertexCount = 0;


void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
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

    yaw += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f) pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;

    vec3 front;
    front.x = cos(radians(yaw)) * cos(radians(pitch));
    front.y = sin(radians(pitch));
    front.z = sin(radians(yaw)) * cos(radians(pitch));
    cameraFront = normalize(front);
}


int main(void)
{
    // Initialize window
    bool windowInitialized = initializeWindow();
    if (!windowInitialized) return -1;

    // Camera 
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Enable Depth Test
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    // Initialize vertex buffer 
    bool vertexbufferInitialized = initializeVertexbuffer();
    if (!vertexbufferInitialized) return -1;

    
    programID = LoadShaders("SimpleVertexShader.vertexshader", "SimpleFragmentShader.fragmentshader");

    // Handles for shader variables
    MatrixID = glGetUniformLocation(programID, "MVP");
    ModelMatrixID = glGetUniformLocation(programID, "M");
    ViewMatrixID = glGetUniformLocation(programID, "V");
    TimeID = glGetUniformLocation(programID, "time");

    // Control Uniforms
    AmpID = glGetUniformLocation(programID, "u_amplitude");
    FreqID = glGetUniformLocation(programID, "u_frequency");
    SpeedID = glGetUniformLocation(programID, "u_speed");
    LightToggleID = glGetUniformLocation(programID, "u_toggleLighting");
    ColorToggleID = glGetUniformLocation(programID, "u_toggleColor");

    // Start animation loop
    do {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        updateAnimationLoop();
    } while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
        glfwWindowShouldClose(window) == 0);

    cleanupVertexbuffer();
    glDeleteProgram(programID);
    closeWindow();

    return 0;
}

//Im moving all camera and key controls under a single method.
//this project has implemented the camera controls well already: https://github.com/czartur/ocean_fft/blob/main/src/cgp_custom.cpp


// Same 



void updateAnimationLoop()
{
    // Clear the screen and Depth buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(programID);

    // Camera Movement: 
    float cameraSpeed = 10.0f * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraPos += cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraPos -= cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cameraPos -= normalize(cross(cameraFront, cameraUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraPos += normalize(cross(cameraFront, cameraUp)) * cameraSpeed;


    // Controls
    // Amplitude: Up/Down Arrows
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) waveAmplitude += 0.01f;
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) waveAmplitude -= 0.01f;

    // Frequency: Right/Left Arrows
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) waveFrequency += 0.01f;
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) waveFrequency -= 0.01f;

    // Toggles (Simple debounce logic would be better, but direct press works for rapid toggle)
    // This makes it easier to see what is happening on a shader level to the waves.
    if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS) toggleLighting = 1;
    if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS) toggleLighting = 0;

    if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS) toggleColor = 1;
    if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS) toggleColor = 0;




    // Camera Matrices
  
 // UPDATE: Camera now moves. yay!


    mat4 Projection = perspective(radians(45.0f), 4.0f / 3.0f, 0.1f, 100.0f);
    mat4 View = lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
    mat4 Model = mat4(1.0f);
    mat4 MVP = Projection * View * Model;

    glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
    glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &Model[0][0]);
    glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &View[0][0]);
    glUniform3fv(CameraPosID, 1, &cameraPos[0]);

    float time = (float)glfwGetTime();
    glUniform1f(TimeID, time);
    glUniform1f(AmpID, waveAmplitude);
    glUniform1f(FreqID, waveFrequency);
    glUniform1f(SpeedID, waveSpeed);
    glUniform1i(LightToggleID, toggleLighting);
    glUniform1i(ColorToggleID, toggleColor);

    // Draw
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

    // Draw grid as triangles
    glDrawArrays(GL_TRIANGLES, 0, vertexCount);

    glDisableVertexAttribArray(0);

    glfwSwapBuffers(window);
    glfwPollEvents();
}

bool initializeVertexbuffer()
{
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);

    std::vector<glm::vec3> vertices;

	// Mesh Grid generation, 2x2 squares made of 2 triangles each
    float offset = (GRID_SIZE * GRID_SPACING) / 2.0f; // To center the grid

    for (int z = 0; z < GRID_SIZE; z++) {
        for (int x = 0; x < GRID_SIZE; x++) {

            float x_pos = (x * GRID_SPACING) - offset;
            float z_pos = (z * GRID_SPACING) - offset;
            float spacing = GRID_SPACING;

            // Triangle 1
            vertices.push_back({ x_pos, 0.0f, z_pos });
            vertices.push_back({ x_pos, 0.0f, z_pos + spacing });
            vertices.push_back({ x_pos + spacing, 0.0f, z_pos });

            // Triangle 2
            vertices.push_back({ x_pos + spacing, 0.0f, z_pos + spacing });
            vertices.push_back({ x_pos + spacing, 0.0f, z_pos });
            vertices.push_back({ x_pos, 0.0f, z_pos + spacing });
        }
    }

    vertexCount = (int)vertices.size();
    vertexbuffer_size = vertexCount * sizeof(glm::vec3);

    glGenBuffers(1, &vertexbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glBufferData(GL_ARRAY_BUFFER, vertexbuffer_size, &vertices[0], GL_STATIC_DRAW);

    return true;
}

bool initializeWindow()
{
    // Initialise GLFW
    if (!glfwInit())
    {
        fprintf(stderr, "Failed to initialize GLFW\n");
        return false;
    }

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Open a window and create its OpenGL context
    window = glfwCreateWindow(1024, 768, "Wave Simulation", NULL, NULL);
    if (window == NULL) {
        fprintf(stderr, "Failed to open GLFW window.\n");
        glfwTerminate();
        return false;
    }
    glfwMakeContextCurrent(window);

    // Initialize GLEW
    glewExperimental = true;
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        glfwTerminate();
        return false;
    }

    // Ensure we can capture the escape key being pressed below
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

    // BACKGROUND COLOR!!
    glClearColor(0.0f, 0.0f, 0.4f, 0.0f);
    return true;
}

bool cleanupVertexbuffer()
{
    // Cleanup VBO
    glDeleteBuffers(1, &vertexbuffer);
    glDeleteVertexArrays(1, &VertexArrayID);
    return true;
}

bool closeWindow()
{
    glfwTerminate();
    return true;
}
