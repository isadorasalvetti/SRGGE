// Author: Marc Comino 2018

#include "./glwidget.h"

#include "./triangle_mesh.h"
#include "./mesh_io.h"
#include "./vertex_clustering.h"
#include "./museum_layout.h"

#include "QElapsedTimer"

QElapsedTimer timer;
int frameN;

//Amount of objects to render
const int n = 2; //grid scaler
const int s = (n*2-1); // s*s = amount of objects in the scene

bool allowPanning;

/*
 * TO DO:

 -Framerate Display DONE
 -Octree on vector cluster DONE
 -Quadradic error matrix vector cluster DONE
 -Dynamic LOD selection DONE
 -LOD selection with h transition DONE
 -Museum layout DONE
 -Add camera panning DONE
 -Multiple objects
 -Visibility culling

*/

namespace {

const float kFieldOfView = 60;
const float kZNear = 0.0001;
const float kZFar = 1000;

const char kVertexShaderFile[] = "../shaders/phong.vert";
const char kFragmentShaderFile[] = "../shaders/phong.frag";

const char msmVertexShaderFile[] = "../shaders/msm.vert";
const char msmFragmentShaderFile[] = "../shaders/msm.frag";

const int kVertexAttributeIdx = 0;
const int kNormalAttributeIdx = 1;

int numFaces; //number of faces/verts after clustering
int numFaces1; //LOD1
int numFaces2; //LOD2
int numFaces3; //LOD3s

int msmFaces;

float centerMsmX = 0;
float centerMsmZ = 0;


bool ReadFile(const std::string filename, std::string *shader_source) {
  std::ifstream infile(filename.c_str());

  if (!infile.is_open() || !infile.good()) {
    std::cerr << "Error " + filename + " not found." << std::endl;
    return false;
  }

  std::stringstream stream;
  stream << infile.rdbuf();
  infile.close();
  *shader_source = stream.str();
  return true;
}

}  // namespace

GLWidget::GLWidget(QWidget *parent)
    : QGLWidget(parent), initialized_(false), width_(0.0), height_(0.0) {
  setFocusPolicy(Qt::StrongFocus);
}

GLWidget::~GLWidget() {}

bool GLWidget::LoadModel(QString filename, QString filename2) {
    std::string file = filename.toUtf8().constData();
    uint pos = file.find_last_of(".");
    std::string type = file.substr(pos + 1);

    std::string file2 = filename2.toUtf8().constData();
    uint pos2 = file2.find_last_of(".");
    std::string type2 = file2.substr(pos2 + 1);

  std::unique_ptr<data_representation::TriangleMesh> mesh =
      std::unique_ptr<data_representation::TriangleMesh>(new data_representation::TriangleMesh);

  std::unique_ptr<data_representation::TriangleMesh> mesh2 =
      std::unique_ptr<data_representation::TriangleMesh>(new data_representation::TriangleMesh);


  bool res = false;
  if (type.compare("ply") == 0) {
    res = data_representation::ReadFromPly(file, mesh.get());
  }

  bool res2 = false;
  if (type2.compare("ply") == 0) {
    res2 = data_representation::ReadFromPly(file2, mesh2.get());
  }

  if (res && res2) {
    mesh_.reset(mesh.release());
    mesh2_.reset(mesh2.release());

    //************************
    // BUNNY DATA
    //************************

    //get new vertices from VERTEX CLUSTERING
    vertClustQ* vC = new vertClustQ;
    //vC.Cluster(mesh_->vertices_, mesh_->faces_, mesh_->normals_, mesh_->faceNormals_, mesh_->min_, mesh_->max_);
    vC->Cluster(mesh_);
    //VERTEX CLUSTERING END

    //LOD 0
    std::vector<float> verts = vC->LODverts; std::vector<int> faces = vC->LODfaces;
    std::vector<float> normals = vC->LODnormals; numFaces = faces.size() / 3;
    //LOD 1
    std::vector<float> verts1 = vC->LODverts1; std::vector<int> faces1 = vC->LODfaces1;
    std::vector<float> normals1 = vC->LODnormals1; numFaces1 = faces1.size() / 3;
    //LOD 2
    std::vector<float> verts2 = vC->LODverts2; std::vector<int> faces2 = vC->LODfaces2;
    std::vector<float> normals2 = vC->LODnormals2; numFaces2 = faces2.size() / 3;
    //LOD 3
    std::vector<float> verts3 = vC->LODverts3; std::vector<int> faces3 = vC->LODfaces3;
    std::vector<float> normals3 = vC->LODnormals3; numFaces3 = faces3.size() / 3;

    emit SetFaces(QString(std::to_string(verts3.size() / 3).c_str())); //Set BUNNY vertices
    //Vertex buffer objects

    //LOD 0
    glGenVertexArrays(1, &VAO); //generate VAO
    glGenBuffers(1, &coordBufferID); //VBO verts
    glGenBuffers(1, &indexBuffersID); //VBO faces
    glGenBuffers(1, &normBuffersID); //VBO faces
    //LOD 1
    glGenVertexArrays(1, &VAO1); //generate VAO
    glGenBuffers(1, &coordBufferID1); //VBO verts
    glGenBuffers(1, &indexBuffersID1); //VBO faces
    glGenBuffers(1, &normBuffersID1); //VBO faces
    //LOD 2
    glGenVertexArrays(1, &VAO2); //generate VAO
    glGenBuffers(1, &coordBufferID2); //VBO verts
    glGenBuffers(1, &indexBuffersID2); //VBO faces
    glGenBuffers(1, &normBuffersID2); //VBO faces
    //LOD 3
    glGenVertexArrays(1, &VAO3); //generate VAO
    glGenBuffers(1, &coordBufferID3); //VBO verts
    glGenBuffers(1, &indexBuffersID3); //VBO faces
    glGenBuffers(1, &normBuffersID3); //VBO faces

    //-----//
    //Bind/ pass data LOD 0
    glBindVertexArray(VAO); //bind VAO
    glBindBuffer(GL_ARRAY_BUFFER, coordBufferID); //bind coord buffer
    glBufferData(GL_ARRAY_BUFFER, verts.size()*sizeof(float), // sizeof(boxVerts)/sizeof(boxVerts[0]),
                   &verts[0], GL_STATIC_DRAW); // ONCE - pass data to buffer
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0); // VAO - set id and size
    glEnableVertexAttribArray(0); // Enable 0
    //Bind / pass data normals
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, normBuffersID); // VBO
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, normals.size()*sizeof(float),
                   &normals[0], GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(1);
    //Bind / pass data faces
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffersID); // VBO
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, faces.size()*sizeof(int),
                   &faces[0], GL_STATIC_DRAW);
    glBindVertexArray(0);//unbind VAO (!!)

    //Bind/ pass data LOD 1
    glBindVertexArray(VAO1); //bind VAO
    glBindBuffer(GL_ARRAY_BUFFER, coordBufferID1); //bind coord buffer
    glBufferData(GL_ARRAY_BUFFER, verts1.size()*sizeof(float), // sizeof(boxVerts)/sizeof(boxVerts[0]),
                   &verts1[0], GL_STATIC_DRAW); // ONCE - pass data to buffer
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0); // VAO - set id and size
    glEnableVertexAttribArray(0); // Enable 0
    //Bind / pass data normals
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, normBuffersID1); // VBO
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, normals1.size()*sizeof(float),
                   &normals1[0], GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(1);
    //Bind / pass data faces
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffersID1); // VBO
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, faces1.size()*sizeof(int),
                   &faces1[0], GL_STATIC_DRAW);
    glBindVertexArray(0);//unbind VAO (!!)

    //Bind/ pass data LOD 2
    glBindVertexArray(VAO2); //bind VAO
    glBindBuffer(GL_ARRAY_BUFFER, coordBufferID2); //bind coord buffer
    glBufferData(GL_ARRAY_BUFFER, verts2.size()*sizeof(float), // sizeof(boxVerts)/sizeof(boxVerts[0]),
                   &verts2[0], GL_STATIC_DRAW); // ONCE - pass data to buffer
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0); // VAO - set id and size
    glEnableVertexAttribArray(0); // Enable 0
    //Bind / pass data normals
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, normBuffersID2); // VBO
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, normals2.size()*sizeof(float),
                   &normals2[0], GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(1);
    //Bind / pass data faces
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffersID2); // VBO
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, faces2.size()*sizeof(int),
                   &faces2[0], GL_STATIC_DRAW);
    glBindVertexArray(0);//unbind VAO (!!)

    //Bind/ pass data LOD 3
    glBindVertexArray(VAO3); //bind VAO
    glBindBuffer(GL_ARRAY_BUFFER, coordBufferID3); //bind coord buffer
    glBufferData(GL_ARRAY_BUFFER, verts3.size()*sizeof(float), // sizeof(boxVerts)/sizeof(boxVerts[0]),
                   &verts3[0], GL_STATIC_DRAW); // ONCE - pass data to buffer
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0); // VAO - set id and size
    glEnableVertexAttribArray(0); // Enable 0
    //Bind / pass data normals
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, normBuffersID3); // VBO
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, normals3.size()*sizeof(float),
                   &normals3[0], GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(1);
    //Bind / pass data faces
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffersID3); // VBO
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, faces3.size()*sizeof(int),
                   &faces3[0], GL_STATIC_DRAW);
    glBindVertexArray(0);//unbind VAO (!!)
    // END BUNNY BUFFERS.

    //Vars for benefit computation
    Eigen::Vector3f dgVec = mesh_->min_ - mesh_->max_;
    diagonal = dgVec.norm();

    //number of faces per LOD
    LODpolys[0] = faces.size()/3;
    LODpolys[1] = faces1.size()/3;
    LODpolys[2] = faces2.size()/3;
    LODpolys[3] = faces3.size()/3;

    //************************
    // Armadillo DATA
    //************************

    //get new vertices from VERTEX CLUSTERING
    //vC.Cluster(mesh_->vertices_, mesh_->faces_, mesh_->normals_, mesh_->faceNormals_, mesh_->min_, mesh_->max_);
    vC = new vertClustQ;
    vC->Cluster(mesh2_);
    //VERTEX CLUSTERING END

    //LOD 0
    verts = vC->LODverts; faces = vC->LODfaces;
    normals = vC->LODnormals; numFaces = faces.size() / 3;
    //LOD 1
    verts1 = vC->LODverts1; faces1 = vC->LODfaces1;
    normals1 = vC->LODnormals1; numFaces1 = faces1.size() / 3;
    //LOD 2
    verts2 = vC->LODverts2; faces2 = vC->LODfaces2;
    normals2 = vC->LODnormals2; numFaces2 = faces2.size() / 3;
    //LOD 3
    verts3 = vC->LODverts3; faces3 = vC->LODfaces3;
    normals3 = vC->LODnormals3; numFaces3 = faces3.size() / 3;

    emit SetVertices(QString(std::to_string(verts3.size() / 3).c_str())); //Set ARMADILLO vertices
    //Vertex buffer objects

    //LOD 0
    glGenVertexArrays(1, &VAOn2); //generate VAO
    glGenBuffers(1, &coordBufferIDn2); //VBO verts
    glGenBuffers(1, &indexBuffersIDn2); //VBO faces
    glGenBuffers(1, &normBuffersIDn2); //VBO faces
    //LOD 1
    glGenVertexArrays(1, &VAO1n2); //generate VAO
    glGenBuffers(1, &coordBufferID1n2); //VBO verts
    glGenBuffers(1, &indexBuffersID1n2); //VBO faces
    glGenBuffers(1, &normBuffersID1n2); //VBO faces
    //LOD 2
    glGenVertexArrays(1, &VAO2n2); //generate VAO
    glGenBuffers(1, &coordBufferID2n2); //VBO verts
    glGenBuffers(1, &indexBuffersID2n2); //VBO faces
    glGenBuffers(1, &normBuffersID2n2); //VBO faces
    //LOD 3
    glGenVertexArrays(1, &VAO3n2); //generate VAO
    glGenBuffers(1, &coordBufferID3n2); //VBO verts
    glGenBuffers(1, &indexBuffersID3n2); //VBO faces
    glGenBuffers(1, &normBuffersID3n2); //VBO faces

    //-----//
    //Bind/ pass data LOD 0
    glBindVertexArray(VAOn2); //bind VAO
    glBindBuffer(GL_ARRAY_BUFFER, coordBufferIDn2); //bind coord buffer
    glBufferData(GL_ARRAY_BUFFER, verts.size()*sizeof(float), // sizeof(boxVerts)/sizeof(boxVerts[0]),
                   &verts[0], GL_STATIC_DRAW); // ONCE - pass data to buffer
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0); // VAO - set id and size
    glEnableVertexAttribArray(0); // Enable 0
    //Bind / pass data normals
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, normBuffersIDn2); // VBO
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, normals.size()*sizeof(float),
                   &normals[0], GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(1);
    //Bind / pass data faces
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffersIDn2); // VBO
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, faces.size()*sizeof(int),
                   &faces[0], GL_STATIC_DRAW);
    glBindVertexArray(0);//unbind VAO (!!)

    //Bind/ pass data LOD 1
    glBindVertexArray(VAO1n2); //bind VAO
    glBindBuffer(GL_ARRAY_BUFFER, coordBufferID1n2); //bind coord buffer
    glBufferData(GL_ARRAY_BUFFER, verts1.size()*sizeof(float), // sizeof(boxVerts)/sizeof(boxVerts[0]),
                   &verts1[0], GL_STATIC_DRAW); // ONCE - pass data to buffer
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0); // VAO - set id and size
    glEnableVertexAttribArray(0); // Enable 0
    //Bind / pass data normals
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, normBuffersID1n2); // VBO
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, normals1.size()*sizeof(float),
                   &normals1[0], GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(1);
    //Bind / pass data faces
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffersID1n2); // VBO
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, faces1.size()*sizeof(int),
                   &faces1[0], GL_STATIC_DRAW);
    glBindVertexArray(0);//unbind VAO (!!)

    //Bind/ pass data LOD 2
    glBindVertexArray(VAO2n2); //bind VAO
    glBindBuffer(GL_ARRAY_BUFFER, coordBufferID2n2); //bind coord buffer
    glBufferData(GL_ARRAY_BUFFER, verts2.size()*sizeof(float), // sizeof(boxVerts)/sizeof(boxVerts[0]),
                   &verts2[0], GL_STATIC_DRAW); // ONCE - pass data to buffer
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0); // VAO - set id and size
    glEnableVertexAttribArray(0); // Enable 0
    //Bind / pass data normals
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, normBuffersID2n2); // VBO
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, normals2.size()*sizeof(float),
                   &normals2[0], GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(1);
    //Bind / pass data faces
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffersID2n2); // VBO
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, faces2.size()*sizeof(int),
                   &faces2[0], GL_STATIC_DRAW);
    glBindVertexArray(0);//unbind VAO (!!)

    //Bind/ pass data LOD 3
    glBindVertexArray(VAO3n2); //bind VAO
    glBindBuffer(GL_ARRAY_BUFFER, coordBufferID3n2); //bind coord buffer
    glBufferData(GL_ARRAY_BUFFER, verts3.size()*sizeof(float), // sizeof(boxVerts)/sizeof(boxVerts[0]),
                   &verts3[0], GL_STATIC_DRAW); // ONCE - pass data to buffer
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0); // VAO - set id and size
    glEnableVertexAttribArray(0); // Enable 0
    //Bind / pass data normals
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, normBuffersID3n2); // VBO
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, normals3.size()*sizeof(float),
                   &normals3[0], GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(1);
    //Bind / pass data faces
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffersID3n2); // VBO
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, faces3.size()*sizeof(int),
                   &faces3[0], GL_STATIC_DRAW);
    glBindVertexArray(0);//unbind VAO (!!)
    // END BUNNY BUFFERS.

    //Vars for benefit computation
    dgVec = mesh2_->min_ - mesh2_->max_;
    diagonaln2 = dgVec.norm();

    //number of faces per LOD
    LODpolysn2[0] = faces.size()/3;
    LODpolysn2[1] = faces1.size()/3;
    LODpolysn2[2] = faces2.size()/3;
    LODpolysn2[3] = faces3.size()/3;

    //************************
    // MUSEUM DATA
    //************************


    mLayout msmGen;
    msmGen.genData();

    std::vector<float> vertsM; std::vector<float> normsM; std::vector<int> facesM;
    std::vector<float> vertsMF; std::vector<float> normsMF; std::vector<int> facesMF;
    vertsM = msmGen.coords; normsM = msmGen.normals; facesM = msmGen.faces; msmFaces = facesM.size() / 3;
    vertsMF = msmGen.coordsFloor; normsMF = msmGen.normalsFloor; facesMF = msmGen.facesFloor;
    bunnyPos = msmGen.bunny;
    armadilloPos = msmGen.armadillo;
    mMin = msmGen.min; mMax = msmGen.max;
    centerMsmX = msmGen.max[0] + msmGen.min[0];
    centerMsmZ = msmGen.max[2] + msmGen.min[2];


    //START MUSEUM BUFFERS
    glGenVertexArrays(1, &VAOfloor); //generate VAO
    glGenBuffers(1, &coordBufferMF); //VBO verts
    glGenBuffers(1, &indexBufferMF); //VBO faces
    glGenBuffers(1, &normBufferMF); //VBO faces

    glGenVertexArrays(1, &VAOmsm); //generate VAO
    glGenBuffers(1, &coordBufferM); //VBO verts
    glGenBuffers(1, &indexBufferM); //VBO faces
    glGenBuffers(1, &normBufferM); //VBO faces

    //FLOOR
    glBindVertexArray(VAOfloor); //bind VAO
    glBindBuffer(GL_ARRAY_BUFFER, coordBufferMF); //bind coord buffer
    glBufferData(GL_ARRAY_BUFFER, vertsMF.size()*sizeof(float), // sizeof(boxVerts)/sizeof(boxVerts[0]),
                   &vertsMF[0], GL_STATIC_DRAW); // ONCE - pass data to buffer
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0); // VAO - set id and size
    glEnableVertexAttribArray(0); // Enable 0
    //Bind / pass data normals
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, normBufferMF); // VBO
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, normsMF.size()*sizeof(float),
                   &normsMF[0], GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(1);
    //Bind / pass data faces
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferMF); // VBO
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, facesMF.size()*sizeof(int),
                   &facesMF[0], GL_STATIC_DRAW);
    glBindVertexArray(0);//unbind VAO (!!)

    //WALLS
    glBindVertexArray(VAOmsm); //bind VAO
    glBindBuffer(GL_ARRAY_BUFFER, coordBufferM); //bind coord buffer
    glBufferData(GL_ARRAY_BUFFER, vertsM.size()*sizeof(float), // sizeof(boxVerts)/sizeof(boxVerts[0]),
                   &vertsM[0], GL_STATIC_DRAW); // ONCE - pass data to buffer
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0); // VAO - set id and size
    glEnableVertexAttribArray(0); // Enable 0
    //Bind / pass data normals
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, normBufferM); // VBO
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, normsM.size()*sizeof(float),
                   &normsM[0], GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(1);
    //Bind / pass data faces
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferM); // VBO
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, facesM.size()*sizeof(int),
                   &facesM[0], GL_STATIC_DRAW);
    glBindVertexArray(0);//unbind VAO (!!)

    //Get mesh ammounts
    bunnyAmount = bunnyPos.size();
    armadilloAmount = armadilloPos.size();

    //Initialize timer for framerate display
    timer.start();
    frameN = 0;

    camera_.UpdateModel(mMin, mMax);

    return true;
  }

  return false;
}

void GLWidget::initializeGL() {
  glewInit();

  glEnable(GL_NORMALIZE);
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);
  glEnable(GL_DEPTH_TEST);

  std::string vertex_shader, fragment_shader, msm_vertex_shader, msm_fragment_shader;
  bool res = ReadFile(kVertexShaderFile, &vertex_shader) &&
             ReadFile(kFragmentShaderFile, &fragment_shader) &&
             ReadFile(msmVertexShaderFile, &msm_vertex_shader) &&
             ReadFile(msmFragmentShaderFile, &msm_fragment_shader);

  if (!res) exit(0);

  program_ = std::unique_ptr<QOpenGLShaderProgram>(new QOpenGLShaderProgram);
  program_->addShaderFromSourceCode(QOpenGLShader::Vertex,
                                    vertex_shader.c_str());
  program_->addShaderFromSourceCode(QOpenGLShader::Fragment,
                                    fragment_shader.c_str());
  program_->bindAttributeLocation("vertex", kVertexAttributeIdx);
  program_->bindAttributeLocation("normal", kNormalAttributeIdx);
  program_->link();

  program_msm_ = std::unique_ptr<QOpenGLShaderProgram>(new QOpenGLShaderProgram);
  program_msm_->addShaderFromSourceCode(QOpenGLShader::Vertex,
                                    msm_vertex_shader.c_str());
  program_msm_->addShaderFromSourceCode(QOpenGLShader::Fragment,
                                    msm_fragment_shader.c_str());
  program_msm_->bindAttributeLocation("vertex", kVertexAttributeIdx);
  program_msm_->bindAttributeLocation("normal", kNormalAttributeIdx);
  program_msm_->link();

  initialized_ = true;
}

void GLWidget::resizeGL(int w, int h) {
  if (h == 0) h = 1;
  width_ = w;
  height_ = h;

  camera_.SetViewport(0, 0, w, h);
  camera_.SetProjection(kFieldOfView, kZNear, kZFar);
}

void GLWidget::mousePressEvent(QMouseEvent *event) {
  if (allowPanning) {
      if (event->button() == Qt::RightButton) {
        camera_.StartPanning(event->x(), event->y());
      }
  }
  else{
      if (event->button() == Qt::LeftButton) {
        camera_.StartRotating(event->x(), event->y());
      }
      /*
      if (event->button() == Qt::RightButton) {
        camera_.StartZooming(event->x(), event->y());
      }*/
  }
  updateGL();
}

void GLWidget::mouseMoveEvent(QMouseEvent *event) {
  camera_.SetRotationX(event->y());
  camera_.SetRotationY(event->x());
  updateGL();
}

void GLWidget::mouseReleaseEvent(QMouseEvent *event) {
  if (event->button() == Qt::LeftButton) {
    camera_.StopRotating(event->x(), event->y());
  }
  updateGL();
}

void GLWidget::keyPressEvent(QKeyEvent *event) {
  if (event->key() == Qt::Key_Up) camera_.Walk(-1);
  if (event->key() == Qt::Key_Down) camera_.Walk(1);

  if (event->key() == Qt::Key_Left) camera_.Rotate(-10);
  if (event->key() == Qt::Key_Right) camera_.Rotate(10);

  if (event->key() == Qt::Key_W) camera_.Walk(-1);
  if (event->key() == Qt::Key_S) camera_.Walk(1);

  if (event->key() == Qt::Key_A) camera_.Rotate(-5);
  if (event->key() == Qt::Key_D) camera_.Rotate(5);

  if (event->key() == Qt::Key_Q) camera_.Fly(-.3);
  if (event->key() == Qt::Key_E) camera_.Fly(.3);

  updateGL();
}



void GLWidget::chooseLODBunny(int bnftLOD){
    if (bnftLOD == 3){
    glBindVertexArray(VAO3); //Draw
    glDrawElements(GL_TRIANGLES, LODpolys[3] * 3, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);} //unbind the VAO (!!)
    else if (bnftLOD == 2){
    glBindVertexArray(VAO2); //Draw
    glDrawElements(GL_TRIANGLES, LODpolys[2] * 3, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);} //unbind the VAO (!!)
    else if (bnftLOD == 1){
    glBindVertexArray(VAO1); //Draw
    glDrawElements(GL_TRIANGLES, LODpolys[1] * 3, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);} //unbind the VAO (!!)
    else{
    glBindVertexArray(VAO); //Draw
    glDrawElements(GL_TRIANGLES, LODpolys[0] * 3, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);} //unbind the VAO (!!)
}

void GLWidget::chooseLODArmadillo(int bnftLOD){
    if (bnftLOD == 3){
    glBindVertexArray(VAO3n2); //Draw
    glDrawElements(GL_TRIANGLES, LODpolysn2[3] * 3, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);} //unbind the VAO (!!)
    else if (bnftLOD == 2){
    glBindVertexArray(VAO2n2); //Draw
    glDrawElements(GL_TRIANGLES, LODpolysn2[2] * 3, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);} //unbind the VAO (!!)
    else if (bnftLOD == 1){
    glBindVertexArray(VAO1n2); //Draw
    glDrawElements(GL_TRIANGLES, LODpolysn2[1] * 3, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);} //unbind the VAO (!!)
    else{
    glBindVertexArray(VAOn2); //Draw
    glDrawElements(GL_TRIANGLES, LODpolysn2[0] * 3, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);} //unbind the VAO (!!)

}

std::vector<int> GLWidget::computeBnft(int objAmount){
 //Maximize Bnft, maintain scene bellow max triagle output.

 /* Choose the best LOD for the mesh:
    - Assume all meshes are in the lowest LOD. Add up the polys for all of them.

    (While scene tris < max tris)
    - Find the mesh withe the best bennefit gained by changing to a higher LOD (D*d/2^l1 - D*d/2^l2).
    - Increase the LOD of that mesh. Add the triangle increase top the total of the scene.

    - Draw scene with the chosen LODS.
 */

    //Compute faces for lower LODs:
    int polysInScene = objAmount*LODpolys[0];
    std::vector<int> OBJchosenLOD;
    OBJchosenLOD.resize(objAmount, 0); //reset LODS

    int maxIterations = objAmount*3; //Dont infinite loop.
    int curRun = 0;
    while (polysInScene < MaxPolys){
        int changeIndex = std::distance(std::begin(sortedBnfGain), std::max_element(std::begin(sortedBnfGain), std::end(sortedBnfGain))); //find LOD to change
        if (OBJchosenLOD[changeIndex] < 3){
            int facesToAdd = LODpolys[OBJchosenLOD[changeIndex]+1] - LODpolys[OBJchosenLOD[changeIndex]]; //get the ammount of faces to add
            polysInScene += facesToAdd;
            OBJchosenLOD[changeIndex] ++;
            sortedBnfGain[changeIndex] = LODbnfit[changeIndex][OBJchosenLOD[changeIndex]];
        } //if possible, increase the LOD of the found element
        else {sortedBnfGain[changeIndex] = 0;} //if not possible, there is no bennefit. Change value in bennefit list.

        curRun ++;
        if (curRun >= maxIterations){break;}
    }
    return OBJchosenLOD;
}


void GLWidget::paintGL() {
  glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  if (initialized_) {
    camera_.SetViewport();

    if (mesh_ != nullptr) {
        //Matrices
        Eigen::Matrix4f projection = camera_.SetProjection();
        Eigen::Matrix4f view = camera_.SetView();
        Eigen::Matrix4f modelBase = camera_.SetModel();

        frameN ++;

        //Uniform locations
        GLuint projection_location = program_->uniformLocation("projection");
        GLuint view_location = program_->uniformLocation("view");
        GLuint model_location = program_->uniformLocation("model");
        GLuint normal_matrix_location = program_->uniformLocation("normal_matrix");
        GLuint LOD_location = program_->uniformLocation("LOD");

    if (bunnyAmount > 0){
      //************************
      // BUNNY
      //************************

        program_->bind();
        glUniformMatrix4fv(projection_location, 1, GL_FALSE, projection.data());
        glUniformMatrix4fv(view_location, 1, GL_FALSE, view.data());

        float bScale = 0.8;
        float sz = (1/diagonal); //bunny scale

        LODbnfit.resize(bunnyAmount);
        sortedBnfGain.resize(bunnyAmount);
        thisFrameDist.resize(bunnyAmount);

        Eigen::Affine3f szTransform(Eigen::Scaling(sz));
        float meshBottom = -mesh_->min_[1];
        Eigen::Affine3f kFloor(Eigen::Translation3f(0, meshBottom, 0)); //moves object to 0, 0, 0.

        //float fl = 0;

        //Rendering objs:
        //ESTIMATE BENNEFIT:
        int renderedObj = 0;
        for (int pos = 0; pos < bunnyAmount; pos++){ //Estimete Bennefit for each drawn object. ATTENTION TO ORDER!
          int j = bunnyPos[pos].first; int i = bunnyPos[pos].second;
          Eigen::Matrix4f inverseView = view.inverse();
          Eigen::Vector4f camera (inverseView(0, 3), inverseView(1, 3), inverseView(2, 3), 1);
          Eigen::Affine3f kTranslation(Eigen::Translation3f(i, 0, j));
          Eigen::Matrix4f model = kTranslation * modelBase;
          Eigen::Vector4f vector (0, 0, 0, 1);
          vector = model * vector;
          Eigen::Vector4f dist = camera - vector;
          //Benefit of the object
          distance = dist.norm();
          thisFrameDist[renderedObj] = distance;
          std::array<float, 3> bnft; //DIFFERENCES. Store bennefit diff from loswest levels to the next (2 = 3->2 ... )
          for (int LOD = 0; LOD < 3; LOD++)
          {
            bnft[LOD] =(bScale / (distance * pow(2, LOD))) - (bScale / (distance * pow(2, LOD+1)));
            if (LOD == 0){sortedBnfGain[renderedObj]=bnft[LOD];} //add to initial sorted bnfit.
          }
          LODbnfit[renderedObj] = bnft; //Store beneffit differences for this object
          renderedObj ++; //Index for LOD beneffit /rendering
        }

      //RENDER OBJECT with best LOD.
      std::vector<int> OBJchosenLOD = computeBnft(bunnyAmount);
      if (dtSet == 0)
      {lastRenderedLOD = OBJchosenLOD; lastRenderedDist = thisFrameDist; dtSet = 1;} //set everything up on first frame = don't compare empties
      renderedObj = 0; //restar rendered obj count

      for (int pos = 0; pos < bunnyPos.size(); pos++){
          int j = bunnyPos[pos].first; int i = bunnyPos[pos].second; // Draw multiple copies for of an object. INDEX MUST BE THE SAME!
          Eigen::Affine3f kTranslation(Eigen::Translation3f(i, 0, j));
          //kTranslation * szTransform * kFloor *
          Eigen::Matrix4f modelBunnyF = camera_.SetIdentity();
          modelBunnyF = kTranslation * modelBase * szTransform * kFloor * modelBunnyF;
          Eigen::Matrix4f t = view * modelBunnyF;
          Eigen::Matrix3f normal;
          for (int i = 0; i < 3; ++i)
            for (int j = 0; j < 3; ++j) normal(i, j) = t(i, j);
          normal = normal.inverse().transpose();

          glUniformMatrix4fv(model_location, 1, GL_FALSE, modelBunnyF.data());
          normal = normal.inverse().transpose();
          glUniformMatrix3fv(normal_matrix_location, 1, GL_FALSE, normal.data());

          /*
          Smooth LOD transition:
            - keep track of distance viewer - obj where the LOD changed last.
            - check the absolute difference dst between the new distance and the distance in the last LOD change.
            - set threshold according to the scene's bounding box.
            - if dst > threshold allow LOD change. Else, draw mesh with last LOD.
          */

          int LOD;

          float distThreshold = 0.5;
          float dstDiff = fabs(lastRenderedDist[renderedObj] - thisFrameDist[renderedObj]);
          if (dstDiff >= distThreshold) {
              LOD = OBJchosenLOD[renderedObj];
              lastRenderedDist[renderedObj] = thisFrameDist[renderedObj];
              lastRenderedLOD[renderedObj] = OBJchosenLOD[renderedObj];
          } else {LOD = lastRenderedLOD[renderedObj];}

          glUniform1f(LOD_location, LOD);
          chooseLODBunny(LOD);
          renderedObj++;

          }

    }//END bunny

      if (armadilloAmount > 0){
      //************************
      // ARMADILLO
      //************************

        program_->bind();
        glUniformMatrix4fv(projection_location, 1, GL_FALSE, projection.data());
        glUniformMatrix4fv(view_location, 1, GL_FALSE, view.data());

        float aScale = 1.5;
        float sz = (1/diagonaln2) * aScale;//armadillo scale

        LODbnfit.resize(armadilloAmount);
        sortedBnfGain.resize(armadilloAmount);
        thisFrameDist.resize(armadilloAmount);

        Eigen::Affine3f szTransform = Eigen::Affine3f (Eigen::Scaling(sz));
        float meshBottom = -mesh2_->min_[1];
        Eigen::Affine3f kFloor = Eigen::Affine3f (Eigen::Translation3f(0, meshBottom, 0)); //moves object to 0, 0, 0.

        //float fl = 0;

        //Rendering objs:
        //ESTIMATE BENNEFIT:
        int renderedObj = 0;
        for (int pos = 0; pos < armadilloAmount; pos++){ //Estimete Bennefit for each drawn object. ATTENTION TO ORDER!
          int j = armadilloPos[pos].first; int i = armadilloPos[pos].second;
          Eigen::Matrix4f inverseView = view.inverse();
          Eigen::Vector4f camera (inverseView(0, 3), inverseView(1, 3), inverseView(2, 3), 1);
          Eigen::Affine3f kTranslation(Eigen::Translation3f(i, 0, j));
          Eigen::Matrix4f model = kTranslation * modelBase;
          Eigen::Vector4f vector (0, 0, 0, 1);
          vector = model * vector;
          Eigen::Vector4f dist = camera - vector;
          //Benefit of the object
          distance = dist.norm();
          thisFrameDist[renderedObj] = distance;
          std::array<float, 3> bnft; //DIFFERENCES. Store bennefit diff from loswest levels to the next (2 = 3->2 ... )
          for (int LOD = 0; LOD < 3; LOD++)
          {
            bnft[LOD] =(aScale / (distance * pow(2, LOD))) - (aScale / (distance * pow(2, LOD+1)));
            if (LOD == 0){sortedBnfGain[renderedObj]=bnft[LOD];} //add to initial sorted bnfit.
          }
          LODbnfit[renderedObj] = bnft; //Store beneffit differences for this object
          renderedObj ++; //Index for LOD beneffit /rendering
        }

      //RENDER OBJECT with best LOD.
      std::vector<int> OBJchosenLOD = computeBnft(armadilloAmount);
      if (dtSet2 == 0)
      {lastRenderedLOD = OBJchosenLOD; lastRenderedDist = thisFrameDist; dtSet2 = 1;} //set everything up on first frame = don't compare empties
      renderedObj = 0; //restar rendered obj count

      for (int pos = 0; pos < armadilloPos.size(); pos++){
          int j = armadilloPos[pos].first; int i = armadilloPos[pos].second; // Draw multiple copies for of an object. INDEX MUST BE THE SAME!
          Eigen::Affine3f kTranslation(Eigen::Translation3f(i, 0, j));
          //kTranslation * szTransform * kFloor *
          Eigen::Matrix4f modelAF = camera_.SetIdentity();
          modelAF = kTranslation * modelBase * szTransform * kFloor * modelAF;
          Eigen::Matrix4f t = view * modelAF;
          Eigen::Matrix3f normal;
          for (int i = 0; i < 3; ++i)
            for (int j = 0; j < 3; ++j) normal(i, j) = t(i, j);
          normal = normal.inverse().transpose();

          glUniformMatrix4fv(model_location, 1, GL_FALSE, modelAF.data());
          normal = normal.inverse().transpose();
          glUniformMatrix3fv(normal_matrix_location, 1, GL_FALSE, normal.data());

          /*
          Smooth LOD transition:
            - keep track of distance viewer - obj where the LOD changed last.
            - check the absolute difference dst between the new distance and the distance in the last LOD change.
            - set threshold according to the scene's bounding box.
            - if dst > threshold allow LOD change. Else, draw mesh with last LOD.
          */

          int LOD;

          float distThreshold = 0.5;
          float dstDiff = fabs(lastRenderedDist[renderedObj] - thisFrameDist[renderedObj]);
          if (dstDiff >= distThreshold) {
              LOD = OBJchosenLOD[renderedObj];
              lastRenderedDist[renderedObj] = thisFrameDist[renderedObj];
              lastRenderedLOD[renderedObj] = OBJchosenLOD[renderedObj];
          } else {LOD = lastRenderedLOD[renderedObj];}

          glUniform1f(LOD_location, LOD);
          chooseLODArmadillo(LOD);
          renderedObj++;

          }
    }//END Armadillo

      //************************
      // MUSEUM
      //************************

      program_msm_->bind();
      projection_location = program_msm_->uniformLocation("projection");
      view_location = program_msm_->uniformLocation("view");
      model_location = program_msm_->uniformLocation("model");
      normal_matrix_location = program_msm_->uniformLocation("normal_matrix");
      GLuint msmCol_location = program_msm_->uniformLocation("color");

      Eigen::Matrix4f t = view * modelBase;
      Eigen::Matrix3f normal;
      for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j) normal(i, j) = t(i, j);
      normal = normal.inverse().transpose();

      glUniformMatrix4fv(projection_location, 1, GL_FALSE, projection.data());
      glUniformMatrix4fv(view_location, 1, GL_FALSE, view.data());
      glUniformMatrix4fv(model_location, 1, GL_FALSE, modelBase.data());
      glUniformMatrix3fv(normal_matrix_location, 1, GL_FALSE, normal.data());
      glUniform1f(msmCol_location, 1.0); //walls color

    //Render museum structure:

    glDisable(GL_CULL_FACE);
    glBindVertexArray(VAOmsm); //Draw
    glDrawElements(GL_TRIANGLES, msmFaces * 3, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0); //unbind the VAO (!!)
    glEnable(GL_CULL_FACE);

    //************************
    // MUSEUM FLOOR
    //************************

    glUniform1f(msmCol_location, 0.3); //floor color
    glBindVertexArray(VAOfloor); //Draw
    glDrawElements(GL_TRIANGLES, 2 * 3, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0); //unbind the VAO (!!)

      //************************
      // FRAMERATE
      //************************

      int frameRate;
      double elapsedTime = (double)timer.elapsed()/1000.0;
      if (timer.elapsed() >= 1000){frameRate = frameN / elapsedTime;}
      emit SetFramerate(QString(std::to_string(frameRate).c_str()));

    }// END mesh !null.

  }

}
