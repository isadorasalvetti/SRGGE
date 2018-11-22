// Author: Marc Comino 2018

#ifndef GLWIDGET_H_
#define GLWIDGET_H_

#include <libraries/glew/include/GL/glew.h>
#include <QGLWidget>
#include <QMouseEvent>
#include <QString>
#include <QOpenGLShaderProgram>

#include <fstream>
#include <iostream>
#include <memory>

#include "./camera.h"
#include "./triangle_mesh.h"

class GLWidget : public QGLWidget {
  Q_OBJECT

 public:
  explicit GLWidget(QWidget *parent = 0);
  ~GLWidget();

  /**
   * @brief LoadModel Loads a PLY model at the filename path into the mesh_ data
   * structure.
   * @param filename Path to the PLY model.
   * @return Whether it was able to load the model.
   */
  bool LoadModel(QString filename, QString filename2);

  //GL VAO -Bunny
  GLuint VAO = 1; GLuint VAO1 = 2;
  GLuint VAO2 = 3; GLuint VAO3 = 4;
  GLuint coordBufferID = 21; GLuint indexBuffersID = 22; GLuint normBuffersID = 23;
  GLuint coordBufferID1 = 24; GLuint indexBuffersID1 = 25; GLuint normBuffersID1 = 26;
  GLuint coordBufferID2 = 27; GLuint indexBuffersID2 = 28; GLuint normBuffersID2 = 29;
  GLuint coordBufferID3 = 30; GLuint indexBuffersID3 = 31; GLuint normBuffersID3 = 32;

  //GL VAO -Armadillo
  GLuint VAOn2; GLuint VAO1n2;
  GLuint VAO2n2; GLuint VAO3n2;
  GLuint coordBufferIDn2; GLuint indexBuffersIDn2; GLuint normBuffersIDn2;
  GLuint coordBufferID1n2; GLuint indexBuffersID1n2; GLuint normBuffersID1n2;
  GLuint coordBufferID2n2; GLuint indexBuffersID2n2; GLuint normBuffersID2n2;
  GLuint coordBufferID3n2; GLuint indexBuffersID3n2; GLuint normBuffersID3n2;

  //GL VAO -Museum
  GLuint VAOmsm = 5; GLuint VAOfloor = 6;
  GLuint coordBufferM = 11; GLuint normBufferM = 12; GLuint indexBufferM = 13;
  GLuint coordBufferMF = 14; GLuint normBufferMF = 15; GLuint indexBufferMF = 16;


  //LOD n cost calculation
  int MaxPolys = 30000; //Smaller value, check if LOD chosing is working.

  std::vector<std::pair<int, int>> bunnyPos;
  std::vector<std::pair<int, int>> armadilloPos;

  float distance;
  float diagonal;
  float diagonaln2;

  std::array<int, 4> LODpolys;
  std::array<int, 4> LODpolysn2;

  Eigen::Vector3f mMin; Eigen::Vector3f mMax;

  //smooth LOD selection
  bool dtSet = 0;
  bool dtSet2 = 0;

  std::vector<int> computeBnft(int objAmount);
  void chooseLODBunny(int bnftLOD);
  void chooseLODArmadillo(int bnftLOD);

  //Bunny
  int bunnyAmount;
  std::vector<std::array<float, 3>> LODbnfit;
  std::vector<float> sortedBnfGain;
  std::vector<float> lastRenderedDist;
  std::vector<int> lastRenderedLOD;
  std::vector<float> thisFrameDist;

  //Armadillo
  int armadilloAmount;
  std::vector<float> lastRenderedDist2;
  std::vector<int> lastRenderedLOD2;

 protected:
  /**
   * @brief initializeGL Initializes OpenGL variables and loads, compiles and
   * links shaders.
   */
  void initializeGL();

  /**
   * @brief resizeGL Resizes the viewport.
   * @param w New viewport width.
   * @param h New viewport height.
   */
  void resizeGL(int w, int h);

  void mousePressEvent(QMouseEvent *event);
  void mouseMoveEvent(QMouseEvent *event);
  void mouseReleaseEvent(QMouseEvent *event);
  void keyPressEvent(QKeyEvent *event);

 private:
  /**
   * @brief program_ A basic shader program.
   */
  std::unique_ptr<QOpenGLShaderProgram> program_;
  std::unique_ptr<QOpenGLShaderProgram> program_msm_;

  /**
   * @brief camera_ Class that computes the multiple camera transform matrices.
   */
  data_visualization::Camera camera_;

  /**
   * @brief mesh_ Data structure representing a triangle mesh.
   */
  std::unique_ptr<data_representation::TriangleMesh> mesh_;
  std::unique_ptr<data_representation::TriangleMesh> mesh2_;

  /**
   * @brief initialized_ Whether the widget has finished initializations.
   */
  bool initialized_;

  /**
   * @brief width_ Viewport current width.
   */
  float width_;

  /**
   * @brief height_ Viewport current height.
   */
  float height_;

 protected slots:
  /**
   * @brief paintGL Function that handles rendering the scene.
   */
  void paintGL();

 signals:
  /**
   * @brief SetFaces Signal that updates the interface label "Faces".
   */
  void SetFaces(QString);

  /**
   * @brief SetFaces Signal that updates the interface label "Vertices".
   */
  void SetVertices(QString);
  /**
   * @brief SetFaces Signal that updates the interface label "Framerate".
   */
  void SetFramerate(QString);
};

#endif  //  GLWIDGET_H_
