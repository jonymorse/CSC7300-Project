#include "file_io.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <string>

bool FileIO::loadPLY(const std::string& filename,
                     std::vector<Vector3>& vertices,
                     std::vector<Face>& faces) {
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << filename << std::endl;
        return false;
    }

    // Clear existing data
    vertices.clear();
    faces.clear();

    // Read header
    int nVertices = 0, nFaces = 0;
    if (!readPLYHeader(file, nVertices, nFaces)) {
        std::cerr << "Error: Invalid PLY header" << std::endl;
        return false;
    }

    // Read vertices
    for (int i = 0; i < nVertices; i++) {
        float x, y, z;
        file.read(reinterpret_cast<char*>(&x), sizeof(float));
        file.read(reinterpret_cast<char*>(&y), sizeof(float));
        file.read(reinterpret_cast<char*>(&z), sizeof(float));
        vertices.emplace_back(x, y, z);
    }

    // Read faces
    for (int i = 0; i < nFaces; i++) {
        unsigned char nVerticesInFace;
        file.read(reinterpret_cast<char*>(&nVerticesInFace), sizeof(unsigned char));
        
        if (nVerticesInFace != 3) {
            std::cerr << "Error: Only triangular faces are supported" << std::endl;
            return false;
        }

        unsigned int v1, v2, v3;
        file.read(reinterpret_cast<char*>(&v1), sizeof(unsigned int));
        file.read(reinterpret_cast<char*>(&v2), sizeof(unsigned int));
        file.read(reinterpret_cast<char*>(&v3), sizeof(unsigned int));
        
        faces.emplace_back(v1, v2, v3);
    }

    std::cout << "Loaded PLY file: " << filename << std::endl;
    std::cout << "Vertices: " << vertices.size() << std::endl;
    std::cout << "Faces: " << faces.size() << std::endl;

    return true;
}

bool FileIO::readPLYHeader(std::ifstream& file, int& nVertices, int& nFaces) {
    std::string line;
    
    // Read PLY header
    std::getline(file, line);
    if (line != "ply") return false;
    
    bool inHeader = true;
    bool binary = false;
    
    while (inHeader && std::getline(file, line)) {
        std::istringstream iss(line);
        std::string keyword;
        iss >> keyword;
        
        if (keyword == "format") {
            std::string format;
            iss >> format;
            binary = (format == "binary_little_endian");
        }
        else if (keyword == "element") {
            std::string element;
            int count;
            iss >> element >> count;
            
            if (element == "vertex") nVertices = count;
            else if (element == "face") nFaces = count;
        }
        else if (keyword == "end_header") {
            inHeader = false;
        }
    }
    
    return !inHeader && binary;
}