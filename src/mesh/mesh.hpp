#pragma once
#include <vector>
#include <string>
#include <array>

struct Vector3 {
    float x, y, z;
    
    Vector3(float x = 0.0f, float y = 0.0f, float z = 0.0f) 
        : x(x), y(y), z(z) {}

    void operator+=(const Vector3& other) {
        x += other.x;
        y += other.y;
        z += other.z;
    }

    void operator/=(float scalar) {
        x /= scalar;
        y /= scalar;
        z /= scalar;
    }
};

struct Face {
    unsigned int v1, v2, v3;
    Vector3 normal;
    
    Face(unsigned int v1, unsigned int v2, unsigned int v3) 
        : v1(v1), v2(v2), v3(v3) {}
};

class Mesh {
public:
    Mesh() = default;
    ~Mesh() = default;

    bool loadFromPLY(const std::string& filename);
    void render() const;
    void debugPrint() const;

    // Getters
    size_t getVertexCount() const { return vertices.size(); }
    size_t getFaceCount() const { return faces.size(); }
    // Add these getter/setter methods
    const std::vector<Vector3>& getVertices() const { return vertices; }
    const std::vector<Face>& getFaces() const { return faces; }
    void setVertices(const std::vector<Vector3>& newVertices) { vertices = newVertices; }
    void setFaces(const std::vector<Face>& newFaces) { faces = newFaces; }

private:
    std::vector<Vector3> vertices;
    std::vector<Face> faces;
    Vector3 centerPoint{0, 0, 0};
    float scale = 1.0f;

    void computeBoundingBox(Vector3& min, Vector3& max);
    void centerAndScale();
    void computeNormals();
};
