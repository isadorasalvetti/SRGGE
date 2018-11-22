//Isadora Salvetti, 2018

#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <algorithm>

#include "vertex_clustering.h"
#include "mesh_io.h"


//utils
void vertClust::Add3Items(float i1, float i2, float i3, std::vector<float> *vector) {
  (*vector).push_back(i1);
  (*vector).push_back(i2);
  (*vector).push_back(i3);
}

void vertClust::Add3Ints(int i1, int i2, int i3, std::vector<int> *vector) {
  (*vector).push_back(i1);
  (*vector).push_back(i2);
  (*vector).push_back(i3);
}

void vertClust::ComputeVertexNormals(const std::vector<float> &vertices,
                          const std::vector<int> &faces,
                          std::vector<float> *normals) {
  const int kFaces = faces.size();
  std::vector<float> face_normals(kFaces, 0);

  for (int i = 0; i < kFaces; i += 3) {
    Eigen::Vector3d v1(vertices[faces[i] * 3], vertices[faces[i] * 3 + 1],
                       vertices[faces[i] * 3 + 2]);
    Eigen::Vector3d v2(vertices[faces[i + 1] * 3],
                       vertices[faces[i + 1] * 3 + 1],
                       vertices[faces[i + 1] * 3 + 2]);
    Eigen::Vector3d v3(vertices[faces[i + 2] * 3],
                       vertices[faces[i + 2] * 3 + 1],
                       vertices[faces[i + 2] * 3 + 2]);
    Eigen::Vector3d v1v2 = v2 - v1;
    Eigen::Vector3d v1v3 = v3 - v1;
    Eigen::Vector3d normal = v1v2.cross(v1v3);

    if (normal.norm() < 0.00001) {
      normal = Eigen::Vector3d(0.0, 0.0, 0.0);
    } else {
      normal.normalize();
    }

    for (int j = 0; j < 3; ++j) face_normals[i + j] = normal[j];
  }

  const int kVertices = vertices.size();
  normals->resize(kVertices, 0);
  for (int i = 0; i < kFaces; i += 3) {
    for (int j = 0; j < 3; ++j) {
      int idx = faces[i + j];
      Eigen::Vector3d v1(vertices[faces[i + j] * 3],
                         vertices[faces[i + j] * 3 + 1],
                         vertices[faces[i + j] * 3 + 2]);
      Eigen::Vector3d v2(vertices[faces[i + (j + 1) % 3] * 3],
                         vertices[faces[i + (j + 1) % 3] * 3 + 1],
                         vertices[faces[i + (j + 1) % 3] * 3 + 2]);
      Eigen::Vector3d v3(vertices[faces[i + (j + 2) % 3] * 3],
                         vertices[faces[i + (j + 2) % 3] * 3 + 1],
                         vertices[faces[i + (j + 2) % 3] * 3 + 2]);

      Eigen::Vector3d v1v2 = v2 - v1;
      Eigen::Vector3d v1v3 = v3 - v1;
      double angle = acos(v1v2.dot(v1v3) / (v1v2.norm() * v1v3.norm()));

      if (angle == angle) {
        for (int k = 0; k < 3; ++k) {
          (*normals)[idx * 3 + k] += face_normals[i + k] * angle;
        }
      }
    }
  }

  const int kNormals = normals->size();
  for (int i = 0; i < kNormals; i += 3) {
    Eigen::Vector3d normal((*normals)[i], (*normals)[i + 1], (*normals)[i + 2]);
    if (normal.norm() > 0.00001) {
      normal.normalize();
    } else {
      normal = Eigen::Vector3d(0, 0, 0);
    }

    for (int j = 0; j < 3; ++j) (*normals)[i + j] = normal[j];
  }
}

/*
    //////
    NOT NEEDED
    //////

//..............................................
// Vertex cluster - Average all verts in cell + octree
//··············································

void vertClust::Cluster(const std::vector<float> &verts, const std::vector<int> &faces, Eigen::Vector3f &min, Eigen::Vector3f &max){
    //set grid size to scene size
    //get mesh bounding box
    float step = (max[0] - min[0])/4; //set step to be based on bounding box
    int dimX = (int) ((max[0] - min[0])/step + 0.5) + 1;
    int dimY = (int) ((max[1] - min[1])/step + 0.5) + 1;
    int dimZ = (int) ((max[2] - min[2])/step + 0.5) + 1;

    int totalCells = dimX*dimY*dimZ;
    const unsigned int numVertices = verts.size() / 3;
    new_coords.resize(totalCells, {0, 0, 0});
    cellMin.resize(totalCells);
    verts_in_cell.resize(totalCells);
    old_to_new_next.resize(numVertices);
    old_to_new.resize(numVertices);

    std::cout << "Started vertex cluster. Initial grid dimmensions: " << dimX << " x " << dimY << " x " << dimZ << std::endl;
    std::cout << "Initial number of vertices: " << numVertices << std::endl;

    std::cout << "Bounding box x: " << min[0] << ", " << max[0] << std::endl;
    std::cout << "Bounding box y: " << min[1] << ", " << max[1] << std::endl;
    std::cout << "Bounding box z: " << min[2] << ", " << max[2] << std::endl;

    unsigned int curCell = 0; //track current cell
    for (unsigned int v = 0; v < numVertices; v++){ //For each vert
        curCell = 0;
        bool stop = 0;
        for(float x = min[0]; x <= (max[0] + step) && stop == 0; x+=step){
            for(float y = min[1]; y <= (max[1] + step) && stop == 0; y+=step){
                for(float z = min[2]; z <= (max[2] + step) && stop == 0; z+=step){
                if (verts[v*3] >= x && verts[v*3] < (x + step) //vert.x
                && verts[v*3+1] >= y && verts[v*3+1] < (y + step) //vert.y
                && verts[v*3+2] >= z && verts[v*3+2] < (z + step) && stop == 0) //vert.z
                    { //vertex is inside cell. Add to count. Do nothing on empty cells
                    old_to_new[v] = curCell;
                    if (cellMin[curCell][0]==0) {cellMin[curCell] = {x, y, z};}
                    new_coords[curCell][0] += verts[v*3]; new_coords[curCell][1] += verts[v*3+1]; new_coords[curCell][2] += verts[(v*3)+2];
                    verts_in_cell[curCell].push_back(v);
                    stop = 1; //cell was found. Stop cell loop.
                    } // End of veretices condition
                curCell ++; //look into each grid cell. - current cell
                }}} //End of cell loop (and stop condition)

    }//End of vert loop. Verts mapped to cells.

    unsigned int currentLevel = 0;
    unsigned int curCellwVert = 0; //track cur cell - ignores empties
    int newCurCell=0; //appending the new octree cells to the arrays.

    for(int i = 0; i < totalCells; i++){ //cell loop
        if (verts_in_cell[i].size() > 0){ //compute current octree level verts
            cellToVertIndex[i] = curCellwVert;
            float vecX = 0; float vecY = 0; float vecZ = 0;
            vecX = new_coords[i][0]; vecY = new_coords[i][1]; vecZ = new_coords[i][2];
            if (i >= vertsInLastLOD){
                vecX = vecX/verts_in_cell[i].size(); vecY = vecY/verts_in_cell[i].size(); vecZ = vecZ/verts_in_cell[i].size(); //this is the new vert
                new_coords[i] = {vecX, vecY, vecZ};
            }
            if (currentLevel == 0) {Add3Items(vecX, vecY, vecZ, &LODverts);} // add it to the final list
            else if (currentLevel == 1) {Add3Items(vecX, vecY, vecZ, &LODverts1);} // add it to the final list
            else if (currentLevel == 2) {Add3Items(vecX, vecY, vecZ, &LODverts2);} // add it to the final list
            else if (currentLevel == 3) {Add3Items(vecX, vecY, vecZ, &LODverts3);} // add it to the final list

            // -----------------------------------------------------------------
            //compute lower octree level if there are enough verts in this cell

            if (verts_in_cell[i].size() > 16 && currentLevel < 4){
                float lStep = step/((currentLevel+1)*2); //step for next LOD
                for (int c=0; c<8 ;c++){verts_in_cell.push_back({}); //before loop: push back vectors
                                        new_coords.push_back({0, 0, 0});
                                        cellMin.push_back({0, 0, 0});} //add 8 new verts

                for (int v=0; v<verts_in_cell[i].size(); v++){//loop through verts inside this cell
                    int stop = 0; //stop checking when cell is found
                    int subCellndex = 0; //index of subcell
                    int vertIndex = verts_in_cell[i][v];
                    float xV = verts[3*vertIndex+0]; float yV = verts[3*vertIndex+1]; float zV = verts[3*vertIndex+2];
                    for(float x = cellMin[i][0]; x < (cellMin[i][0] + lStep + 0.001); x+=lStep){
                    for(float y = cellMin[i][1]; y < (cellMin[i][1] + lStep + 0.001); y+=lStep){
                    for(float z = cellMin[i][2]; z < (cellMin[i][2] + lStep + 0.001); z+=lStep){
                        if (stop == 0 && xV >= x && xV < (x + lStep + 0.001) //vert.x
                        && yV >= y && yV < (y + lStep + 0.001) //vert.y
                        && zV >= z && zV < (z + lStep + 0.001)){ //if vert is in this subcell
                            int newCellInd = totalCells+newCurCell+subCellndex;
                            new_coords[newCellInd][0] += xV; new_coords[newCellInd][1] += yV; new_coords[newCellInd][2] += zV;
                            verts_in_cell[newCellInd].push_back(vertIndex);
                            old_to_new_next[vertIndex] = newCellInd;
                            if(cellMin[newCellInd][0] == 0){cellMin[newCellInd] = {x, y, z};}
                            stop = 1; //cell found. Stop condition, loop must run to the end.
                        }
                    subCellndex ++;

                    if (subCellndex > 8)
                    {std::cout<< subCellndex <<std::endl;}

                    }}}
                }//End of verst in new cells loop
                newCurCell += 8;
                verts_in_cell[i] = {}; //all vertices have been assigned to new cells. Set vertex_per_cell[i] to 0
            }
            else { //when the cell is not subdivided
                for (int v=0; v<verts_in_cell[i].size(); v++){
                    old_to_new_next[verts_in_cell[i][v]] = i;}
            }
            curCellwVert ++;
            }
        //---------------------------------------------------------------------------

        // On the last index = compute faces and normals
    if(i == totalCells -1 && currentLevel < 4){
        i = 0;
        vertsInLastLOD = totalCells;
        totalCells = verts_in_cell.size();
        computeFacesNormals(currentLevel, faces, old_to_new);
        currentLevel++;
        curCellwVert = 0; //track cur cell - ignores empties
        newCurCell = 0;
        old_to_new = old_to_new_next;
        } //change the LOD level

    }//End of cell loop. New positions defined.
    std::cout << "End of vertex clustering." << std::endl;
}

void vertClust::computeFacesNormals(int LOD, const std::vector<int> &faces, const std::vector<int> &oldNew){
    const unsigned int numFaces = faces.size() / 3;
    int faceVerts[4] = {0, 0, 0, 0}; //face verts. [3] = weather or not this face is kept.
    for (unsigned int f = 0; f < numFaces; f++){ //loop through faces. Find where their verts are. Change the indices to the new verts.
        faceVerts[0] = cellToVertIndex[oldNew[faces[f*3+0]]]; faceVerts[1] = cellToVertIndex[oldNew[faces[f*3+1]]]; faceVerts[2] = cellToVertIndex[oldNew[faces[f*3+2]]]; faceVerts[3] = 0;
        if (faceVerts[0] != faceVerts[1]) {faceVerts[3] += 1;} if (faceVerts[0] != faceVerts[2]) {faceVerts[3] += 1;}
        if (faceVerts[1] != faceVerts[2]) {faceVerts[3] += 1;} if (faceVerts[3] == 3){ //this face is marked to be kept.
             if (LOD == 0){Add3Ints(faceVerts[0], faceVerts[1], faceVerts[2], &LODfaces);}
             else if (LOD == 1){Add3Ints(faceVerts[0], faceVerts[1], faceVerts[2], &LODfaces1);}
             else if (LOD == 2){Add3Ints(faceVerts[0], faceVerts[1], faceVerts[2], &LODfaces2);}
             else if (LOD == 3){Add3Ints(faceVerts[0], faceVerts[1], faceVerts[2], &LODfaces3);}
        }
    }// End of face loop. New faces generated.
     if (LOD == 0){ComputeVertexNormals(LODverts, LODfaces, &LODnormals);}
     else if (LOD == 1){ComputeVertexNormals(LODverts1, LODfaces1, &LODnormals1);}
     else if (LOD == 2){ComputeVertexNormals(LODverts2, LODfaces2, &LODnormals2);}
     else if (LOD == 3){ComputeVertexNormals(LODverts3, LODfaces3, &LODnormals3);}
}
*/
