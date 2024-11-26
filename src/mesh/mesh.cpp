#include "mesh.hpp"
#include <GLFW/glfw3.h>
#include <iostream>
#include <fstream>  // Added for ifstream
#include <sstream>  // Added for istringstream
#include <limits>
#include <cmath>
#include <algorithm>

void Mesh::computeBoundingBox(Vector3& min, Vector3& max) {
    if (vertices.empty()) return;

    min = max = vertices[0];
    for (const auto& v : vertices) {
        min.x = std::min(min.x, v.x);
        min.y = std::min(min.y, v.y);
        min.z = std::min(min.z, v.z);
        max.x = std::max(max.x, v.x);
        max.y = std::max(max.y, v.y);
        max.z = std::max(max.z, v.z);
    }
}

void Mesh::centerAndScale() {
    if (vertices.empty()) return;

    // Compute bounding box
    Vector3 min, max;
    computeBoundingBox(min, max);

    // Compute center
    centerPoint.x = (min.x + max.x) / 2.0f;
    centerPoint.y = (min.y + max.y) / 2.0f;
    centerPoint.z = (min.z + max.z) / 2.0f;

    // Compute scale factor
    float dx = max.x - min.x;
    float dy = max.y - min.y;
    float dz = max.z - min.z;
    scale = std::max({dx, dy, dz});
    if (scale < 1e-6f) scale = 1.0f;

    // Center and scale vertices
    for (auto& v : vertices) {
        v.x = (v.x - centerPoint.x) / scale;
        v.y = (v.y - centerPoint.y) / scale;
        v.z = (v.z - centerPoint.z) / scale;
    }

    std::cout << "Mesh centered and scaled. Scale factor: " << scale << std::endl;
}

void Mesh::computeNormals() {
    for (auto& face : faces) {
        const Vector3& v1 = vertices[face.v1];
        const Vector3& v2 = vertices[face.v2];
        const Vector3& v3 = vertices[face.v3];

        // Compute edges
        Vector3 edge1{v2.x - v1.x, v2.y - v1.y, v2.z - v1.z};
        Vector3 edge2{v3.x - v1.x, v3.y - v1.y, v3.z - v1.z};

        // Cross product for normal
        face.normal.x = edge1.y * edge2.z - edge1.z * edge2.y;
        face.normal.y = edge1.z * edge2.x - edge1.x * edge2.z;
        face.normal.z = edge1.x * edge2.y - edge1.y * edge2.x;

        // Normalize
        float len = sqrt(face.normal.x * face.normal.x + 
                        face.normal.y * face.normal.y + 
                        face.normal.z * face.normal.z);
        if (len > 1e-6f) {
            face.normal.x /= len;
            face.normal.y /= len;
            face.normal.z /= len;
        }
    }
}

bool Mesh::loadFromPLY(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << filename << std::endl;
        return false;
    }

    vertices.clear();
    faces.clear();

    // Read header
    std::string line;
    int nVertices = 0, nFaces = 0;
    bool binary = false;

    std::cout << "Starting to read PLY file: " << filename << std::endl;

    // Process header
    std::getline(file, line);
    if (line != "ply") {
        std::cerr << "Error: Not a PLY file" << std::endl;
        return false;
    }

    while (std::getline(file, line)) {
        if (line == "end_header") break;

        std::istringstream iss(line);
        std::string keyword;
        iss >> keyword;

        if (keyword == "format") {
            std::string format;
            iss >> format;
            binary = (format == "binary_little_endian");
            std::cout << "File format: " << format << std::endl;
        }
        else if (keyword == "element") {
            std::string element;
            int count;
            iss >> element >> count;
            if (element == "vertex") {
                nVertices = count;
                std::cout << "Number of vertices: " << nVertices << std::endl;
            }
            else if (element == "face") {
                nFaces = count;
                std::cout << "Number of faces: " << nFaces << std::endl;
            }
        }
    }

    // Read vertices
    std::cout << "Reading vertices..." << std::endl;
    vertices.reserve(nVertices);
    for (int i = 0; i < nVertices; i++) {
        std::string line;
        std::getline(file, line);
        std::istringstream iss(line);
        
        float x, y, z;
        iss >> x >> y >> z;
        vertices.emplace_back(x, y, z);
        
        if (i % 10000 == 0) {
            std::cout << "Processed " << i << " vertices\r" << std::flush;
        }
    }
    std::cout << "\nFinished reading vertices" << std::endl;

    // Read faces
    std::cout << "Reading faces..." << std::endl;
    faces.reserve(nFaces);
    for (int i = 0; i < nFaces; i++) {
        std::string line;
        std::getline(file, line);
        std::istringstream iss(line);
        
        int nVerticesInFace;
        iss >> nVerticesInFace;
        
        if (nVerticesInFace != 3) {
            std::cerr << "Error: Only triangular faces are supported" << std::endl;
            return false;
        }

        unsigned int v1, v2, v3;
        iss >> v1 >> v2 >> v3;
        faces.emplace_back(v1, v2, v3);
        
        if (i % 10000 == 0) {
            std::cout << "Processed " << i << " faces\r" << std::flush;
        }
    }
    std::cout << "\nFinished reading faces" << std::endl;

    std::cout << "Successfully loaded PLY file\n";
    std::cout << "Vertices loaded: " << vertices.size() << std::endl;
    std::cout << "Faces loaded: " << faces.size() << std::endl;

    // Center and scale the mesh
    centerAndScale();
    computeNormals();

    return true;
}
void Mesh::render() const {
    glBegin(GL_TRIANGLES);
    for (const auto& face : faces) {
        glNormal3f(face.normal.x, face.normal.y, face.normal.z);
        
        const Vector3& v1 = vertices[face.v1];
        const Vector3& v2 = vertices[face.v2];
        const Vector3& v3 = vertices[face.v3];

        glVertex3f(v1.x, v1.y, v1.z);
        glVertex3f(v2.x, v2.y, v2.z);
        glVertex3f(v3.x, v3.y, v3.z);
    }
    glEnd();
}

void Mesh::debugPrint() const {
    std::cout << "\nMesh Debug Information:\n";
    std::cout << "Number of vertices: " << vertices.size() << "\n";
    std::cout << "Number of faces: " << faces.size() << "\n";
    std::cout << "Center point: (" << centerPoint.x << ", " 
              << centerPoint.y << ", " << centerPoint.z << ")\n";
    std::cout << "Scale factor: " << scale << "\n";
    
    if (!vertices.empty()) {
        const auto& v = vertices[0];
        std::cout << "First vertex: (" << v.x << ", " << v.y << ", " << v.z << ")\n";
    }
    
    if (!faces.empty()) {
        const auto& f = faces[0];
        std::cout << "First face indices: " << f.v1 << ", " << f.v2 << ", " << f.v3 << "\n";
        std::cout << "First face normal: (" << f.normal.x << ", " 
                  << f.normal.y << ", " << f.normal.z << ")\n";
    }
}