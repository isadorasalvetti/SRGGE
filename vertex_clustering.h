#ifndef VERTEX_CLUSTERING_H
#define VERTEX_CLUSTERING_H

#include "glwidget.h"
#include "./mesh_io.h"
#include "./triangle_mesh.h"

class vertClust {
/*
    OBSOLETE -> vertClustQ can do the same thing. This class needs to be removed,
    move utils to vertClusterQ. Old functions deleted.
*/
public:
    void Add3Items(float i1, float i2, float i3, std::vector<float> *vector);
    void Add3Ints(int i1, int i2, int i3, std::vector<int> *vector);
    void ComputeVertexNormals(const std::vector<float> &vertices,
                              const std::vector<int> &faces,
                              std::vector<float> *normals);
};

class vertClustQ {
/*
    Vertex clustering in an octree using quadratic error metrics / vertex average.
*/
public:
    void Cluster(std::unique_ptr<data_representation::TriangleMesh> &mesh);
    void QcomputeFacesNormals(int LOD, const std::vector<int> &faces, const std::vector<int> &oldNew);
    void GenQMatrix(const std::vector<float> &verts, const std::vector<float> &normals, const std::vector<int> &faces);

    //send to buffer
    std::vector<float> LODverts;
    std::vector<int> LODfaces;
    std::vector<float> LODnormals;

    std::vector<float> LODverts1;
    std::vector<int> LODfaces1;
    std::vector<float> LODnormals1;

    std::vector<float> LODverts2;
    std::vector<int> LODfaces2;
    std::vector<float> LODnormals2;

    std::vector<float> LODverts3;
    std::vector<int> LODfaces3;
    std::vector<float> LODnormals3;

private:
    bool ComputeError(std::vector<int> vecs);
    std::vector<Eigen::Matrix4f> QMat;
    //rebuild faces
    std::vector<std::vector<int>> verts_in_cell;
    //std::map <unsigned int, std::vector<int>> verts_in_cell;
    std::vector<int> old_to_new; //new verts. index = old verts index
    std::vector<std::array <float, 3>> cellMin; //store cell min position of a cell. Max = min + step.
    std::vector<std::array <float,3>> new_coords;
    std::map <int, int> cellToVertIndex; //Cell ids (includes empties) to vert coords (no empties)
    std::vector<int> old_to_new_next;
    int vertsInLastLOD = 0; //verts from last lod have already been divided. Dont to it again.
};

#endif // VERTEX_CLUSTERING_H
