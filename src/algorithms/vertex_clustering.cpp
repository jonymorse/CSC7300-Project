#include "vertex_clustering.hpp"
#include <iostream>
#include <unordered_map>
#include <algorithm>
#include <cmath>

VertexClustering::Grid3D VertexClustering::positionToGrid(
    const Vector3& pos, const Vector3& min, const Vector3& max) {
    
    // Scale to [0,1]
    float x = (pos.x - min.x) / (max.x - min.x);
    float y = (pos.y - min.y) / (max.y - min.y);
    float z = (pos.z - min.z) / (max.z - min.z);

    // Convert to grid coordinates
    return Grid3D{
        static_cast<int>(x * gridSize),
        static_cast<int>(y * gridSize),
        static_cast<int>(z * gridSize)
    };
}

Mesh VertexClustering::simplify(const Mesh& inputMesh) {
    std::cout << "Starting vertex clustering simplification...\n";
    std::cout << "Input mesh: " << inputMesh.getVertexCount() << " vertices, "
              << inputMesh.getFaceCount() << " faces\n";

    // Get mesh data
    const auto& inputVertices = inputMesh.getVertices();
    const auto& inputFaces = inputMesh.getFaces();

    // Find bounding box
    Vector3 min = inputVertices[0], max = inputVertices[0];
    for (const auto& v : inputVertices) {
        min.x = std::min(min.x, v.x);
        min.y = std::min(min.y, v.y);
        min.z = std::min(min.z, v.z);
        max.x = std::max(max.x, v.x);
        max.y = std::max(max.y, v.y);
        max.z = std::max(max.z, v.z);
    }

    // Map grid cells to representative vertices
    std::unordered_map<Grid3D, Vector3, GridHasher> gridToVertex;
    std::unordered_map<Grid3D, int, GridHasher> gridToCount;
    std::vector<Grid3D> vertexToGrid(inputVertices.size());

    // First pass: accumulate vertices in grid cells
    for (size_t i = 0; i < inputVertices.size(); i++) {
        const auto& v = inputVertices[i];
        Grid3D grid = positionToGrid(v, min, max);
        vertexToGrid[i] = grid;

        if (gridToVertex.find(grid) == gridToVertex.end()) {
            gridToVertex[grid] = Vector3(0, 0, 0);
            gridToCount[grid] = 0;
        }

        // Accumulate vertices
        gridToVertex[grid].x += v.x;
        gridToVertex[grid].y += v.y;
        gridToVertex[grid].z += v.z;
        gridToCount[grid]++;
    }

    // Second pass: compute average positions
    for (auto& pair : gridToVertex) {
        const Grid3D& grid = pair.first;
        Vector3& avg = pair.second;
        int count = gridToCount[grid];
        
        avg.x /= count;
        avg.y /= count;
        avg.z /= count;
    }

    // Create mapping from old vertices to new indices
    std::unordered_map<Grid3D, int, GridHasher> gridToIndex;
    std::vector<Vector3> newVertices;
    for (const auto& pair : gridToVertex) {
        gridToIndex[pair.first] = newVertices.size();
        newVertices.push_back(pair.second);
    }

    // Create new faces, removing degenerate ones
    std::vector<Face> newFaces;
    for (const auto& face : inputFaces) {
        // Get new vertex indices
        int v1 = gridToIndex[vertexToGrid[face.v1]];
        int v2 = gridToIndex[vertexToGrid[face.v2]];
        int v3 = gridToIndex[vertexToGrid[face.v3]];

        // Skip degenerate triangles
        if (v1 != v2 && v2 != v3 && v3 != v1) {
            newFaces.emplace_back(v1, v2, v3);
        }
    }

    // Create simplified mesh
    Mesh simplifiedMesh;
    simplifiedMesh.setVertices(newVertices);
    simplifiedMesh.setFaces(newFaces);

    std::cout << "Simplification complete:\n";
    std::cout << "Output mesh: " << simplifiedMesh.getVertexCount() << " vertices, "
              << simplifiedMesh.getFaceCount() << " faces\n";
    std::cout << "Reduction ratio: " 
              << (float)simplifiedMesh.getVertexCount() / inputMesh.getVertexCount() * 100.0f 
              << "%\n";

    return simplifiedMesh;
}