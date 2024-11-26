#pragma once
#include "../mesh/mesh.hpp"
#include <unordered_map>
#include <vector>

class VertexClustering {
public:
    struct Grid3D {
        int x, y, z;
        
        // Hash function for grid coordinates
        size_t hash() const {
            return (size_t)x * 73856093 + 
                   (size_t)y * 19349663 + 
                   (size_t)z * 83492791;
        }

        bool operator==(const Grid3D& other) const {
            return x == other.x && y == other.y && z == other.z;
        }
    };

    struct GridHasher {
        size_t operator()(const Grid3D& grid) const {
            return grid.hash();
        }
    };

    // Constructor takes the number of grid cells per dimension
    VertexClustering(int gridSize) : gridSize(gridSize) {}

    // Main simplification function
    Mesh simplify(const Mesh& inputMesh);

private:
    int gridSize;  // Number of grid cells per dimension

    // Convert 3D position to grid cell coordinates
    Grid3D positionToGrid(const Vector3& pos, const Vector3& min, const Vector3& max);
};