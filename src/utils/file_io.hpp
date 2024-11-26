#pragma once
#include <string>
#include <vector>
#include "../mesh/mesh.hpp"

class FileIO {
public:
    static bool loadOBJ(const std::string& filename, 
                       std::vector<Vector3>& vertices,
                       std::vector<Face>& faces);
                       
    static bool loadPLY(const std::string& filename,
                       std::vector<Vector3>& vertices,
                       std::vector<Face>& faces);
private:
    static bool readPLYHeader(std::ifstream& file, int& nVertices, int& nFaces);
};