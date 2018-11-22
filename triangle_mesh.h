// Author: Marc Comino 2018

#ifndef TRIANGLE_MESH_H_
#define TRIANGLE_MESH_H_

#include <libraries/eigen3/Eigen/Geometry>
#include <libraries/eigen3/Eigen/Core>

#include <vector>

namespace data_representation {

class TriangleMesh {
 public:
  /**
   * @brief TriangleMesh Constructor of the class. Calls clear.
   */
  TriangleMesh();

  /**
   * @brief ~TriangleMesh Destructor of the class.
   */
  ~TriangleMesh() {}

  /**
   * @brief Clear Empties the data arrays and resets the bounding box vertices.
   */
  void Clear();

 public:
  std::vector<float> vertices_;
  std::vector<int> faces_;
  std::vector<float> normals_;
  std::vector<float> faceNormals_;

  /**
   * @brief min The minimum point of the bounding box.
   */
  Eigen::Vector3f min_;

  /**
   * @brief max The maximum point of the bounding box.
   */
  Eigen::Vector3f max_;
};

}  // namespace data_representation

#endif  //  TRIANGLE_MESH_H_
