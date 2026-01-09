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

// Globals

GLuint MatrixID, ModelMatrixID, ViewMatrixID;
GLuint TimeID, AmpID, FreqID, SpeedID, LightToggleID, ColorToggleID;

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

int main(void)
{
    // Initialize window
    bool windowInitialized = initializeWindow();
    if (!windowInitialized) return -1;

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
        updateAnimationLoop();
    } while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
        glfwWindowShouldClose(window) == 0);

    cleanupVertexbuffer();
    glDeleteProgram(programID);
    closeWindow();

    return 0;
}

void updateAnimationLoop()
{
    // Clear the screen and Depth buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(programID);

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
  
    glm::mat4 Projection = glm::perspective(glm::radians(45.0f), 4.0f / 3.0f, 0.1f, 100.0f);

    //TODO: Bring in some camera control
    // Camera looking at the grid from above and to the side
    glm::mat4 View = glm::lookAt(
        glm::vec3(15, 15, 15), // Camera is at (15,15,15)
        glm::vec3(0, 0, 0),    // and looks at the origin
        glm::vec3(0, 1, 0)     // Head is up
    );

    glm::mat4 Model = glm::mat4(1.0f); // Identity matrix
    glm::mat4 MVP = Projection * View * Model;

    // Shader Communication:
    // Send them to shader
    glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
    glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &Model[0][0]);
    glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &View[0][0]);

    // Send Time
    float time = (float)glfwGetTime();
    glUniform1f(TimeID, time);

    // Send Controls
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

