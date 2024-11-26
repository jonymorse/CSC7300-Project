#pragma once
#include <GLFW/glfw3.h>
#include <cmath>

class Camera {
public:
    Camera() : radius(5.0f), theta(0.0f), phi(0.0f) {}

    void rotate(float deltaTheta, float deltaPhi) {
        theta += deltaTheta;
        phi += deltaPhi;
        if (phi > 89.0f) phi = 89.0f;
        if (phi < -89.0f) phi = -89.0f;
    }

    void zoom(float delta) {
        radius += delta;
        if (radius < 1.0f) radius = 1.0f;
    }

    void apply() const {
        glLoadIdentity();
        
        // Convert spherical to Cartesian coordinates
        float x = radius * cos(phi * 3.14159f/180.0f) * cos(theta * 3.14159f/180.0f);
        float y = radius * sin(phi * 3.14159f/180.0f);
        float z = radius * cos(phi * 3.14159f/180.0f) * sin(theta * 3.14159f/180.0f);
        
        // Create a simple look-at matrix
        float forward[3] = {-x, -y, -z};  // Camera points toward origin
        float up[3] = {0.0f, 1.0f, 0.0f};
        
        // Normalize forward vector
        float len = sqrt(forward[0]*forward[0] + forward[1]*forward[1] + forward[2]*forward[2]);
        forward[0] /= len;
        forward[1] /= len;
        forward[2] /= len;
        
        // Right vector = up × forward
        float right[3] = {
            up[1]*forward[2] - up[2]*forward[1],
            up[2]*forward[0] - up[0]*forward[2],
            up[0]*forward[1] - up[1]*forward[0]
        };
        
        // Recompute up vector = forward × right
        up[0] = forward[1]*right[2] - forward[2]*right[1];
        up[1] = forward[2]*right[0] - forward[0]*right[2];
        up[2] = forward[0]*right[1] - forward[1]*right[0];
        
        // Create view matrix
        float m[16] = {
            right[0], up[0], -forward[0], 0,
            right[1], up[1], -forward[1], 0,
            right[2], up[2], -forward[2], 0,
            0, 0, 0, 1
        };
        
        glMultMatrixf(m);
        glTranslatef(-x, -y, -z);
    }

private:
    float radius;  // Distance from origin
    float theta;   // Horizontal angle
    float phi;     // Vertical angle
};
