#include <GLFW/glfw3.h>
#include <iostream>
#include "mesh/mesh.hpp"
#include "visualization/camera.hpp"
#include <cmath>
#include "algorithms/vertex_clustering.hpp"

// Global variables
Camera camera;
bool leftMousePressed = false;
double lastX, lastY;
bool wireframeMode = false;

Mesh* originalMesh = nullptr;
Mesh* simplifiedMesh = nullptr;
bool showSimplified = false;


// Helper function to set perspective projection
void setPerspective(float fovy, float aspect, float zNear, float zFar) {
    float f = 1.0f / tan(fovy * 3.14159f / 360.0f);
    float m[16] = {
        f/aspect, 0, 0, 0,
        0, f, 0, 0,
        0, 0, (zFar+zNear)/(zNear-zFar), -1,
        0, 0, (2*zFar*zNear)/(zNear-zFar), 0
    };
    glMultMatrixf(m);
}

// Add to key callback
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_W && action == GLFW_PRESS) {
        wireframeMode = !wireframeMode;
        if (wireframeMode)
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        else
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
    else if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
        showSimplified = !showSimplified;
        std::cout << "Showing " << (showSimplified ? "simplified" : "original") << " mesh\n";
    }
    else if (key == GLFW_KEY_S && action == GLFW_PRESS) {
        // Simplify with different grid sizes
        static int gridSize = 16;
        VertexClustering clustering(gridSize);
        delete simplifiedMesh;
        simplifiedMesh = new Mesh(clustering.simplify(*originalMesh));
        std::cout << "Simplified with grid size: " << gridSize << std::endl;
        gridSize = (gridSize == 16) ? 32 : (gridSize == 32) ? 8 : 16;  // Cycle through grid sizes
    }
}

// Mouse callbacks remain the same
void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
    if (leftMousePressed) {
        double deltaX = xpos - lastX;
        double deltaY = ypos - lastY;
        camera.rotate(deltaX * 0.5f, -deltaY * 0.5f);
    }
    lastX = xpos;
    lastY = ypos;
}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        leftMousePressed = (action == GLFW_PRESS);
    }
}

void scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    camera.zoom(-yoffset * 0.5f);
}

void setupLighting() {
    // Light position
    float lightPos[] = {5.0f, 5.0f, 5.0f, 1.0f};
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
    
    // Light properties
    float ambientLight[] = {0.2f, 0.2f, 0.2f, 1.0f};
    float diffuseLight[] = {0.8f, 0.8f, 0.8f, 1.0f};
    float specularLight[] = {1.0f, 1.0f, 1.0f, 1.0f};
    
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambientLight);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseLight);
    glLightfv(GL_LIGHT0, GL_SPECULAR, specularLight);
    
    // Material properties
    float materialSpecular[] = {0.5f, 0.5f, 0.5f, 1.0f};
    float materialShininess[] = {50.0f};
    
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, materialSpecular);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, materialShininess);
}



int main() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    GLFWwindow* window = glfwCreateWindow(800, 600, "Mesh Simplification", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwSetKeyCallback(window, keyCallback);
    glfwSetCursorPosCallback(window, mouseCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetScrollCallback(window, scrollCallback);
    glfwMakeContextCurrent(window);

    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;

    // Setup OpenGL
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_NORMALIZE); // Add this for better normal handling


    // Load mesh
// Load mesh (Stanford bunny)
    originalMesh = new Mesh();
    if (!originalMesh->loadFromPLY("models/bunny/reconstruction/bun_zipper.ply")) {
        std::cerr << "Failed to load mesh" << std::endl;
        glfwTerminate();
        return -1;
    }

    // Initial simplification
    VertexClustering clustering(16);  // Start with 16x16x16 grid
    simplifiedMesh = new Mesh(clustering.simplify(*originalMesh));

    // Setup projection matrix
    setupLighting();
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    setPerspective(45.0f, 800.0f/600.0f, 0.1f, 100.0f);

    // Main loop
// Main loop
    while (!glfwWindowShouldClose(window)) {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height);
        
        // Clear buffers with darker background
        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        // Setup view
        glMatrixMode(GL_MODELVIEW);
        camera.apply();

        // Update lighting
        setupLighting();

        // Render the appropriate mesh
        if (showSimplified && simplifiedMesh) {
            // Set color for simplified mesh (e.g., slightly reddish)
            glColor3f(1.0f, 1.0f, 1.0f);
            simplifiedMesh->render();
        } else if (originalMesh) {
            // Set color for original mesh (white)
            glColor3f(1.0f, 1.0f, 1.0f);
            originalMesh->render();
        }

        // Display mesh information
        if (showSimplified && simplifiedMesh) {
            std::cout << "\rShowing simplified mesh: " 
                     << simplifiedMesh->getVertexCount() << " vertices, "
                     << simplifiedMesh->getFaceCount() << " faces     \r" << std::flush;
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup
    delete originalMesh;
    delete simplifiedMesh;

    glfwTerminate();
    return 0;
}