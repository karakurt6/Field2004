//****************************************************************************
//                                                                         
// demonstration class to visualize scalar data on a surface mesh 
// with 3D-Data-Master  
//                                                                         
//  author : J-Michel Godinaud                                             
//****************************************************************************

#include <DataViz/3Ddata/PbMesh2D.h>
#include <DataViz/nodes/PoDomain.h>

#include <DataViz/3Ddata/PoMeshFilled.h>
#include <DataViz/3Ddata/PoMeshContouring.h>
#include <DataViz/3Ddata/PoMeshSides.h>
#include <DataViz/3Ddata/PoMeshLimit.h>
#include <DataViz/3Ddata/PoMeshLines.h>

#include <Inventor/nodes/SoSwitch.h>
#include <Inventor/nodes/SoPickStyle.h>

#include <DialogViz/SoDialogVizAll.h>

#include "MeshViewerComponent.h"

/******************************************************************************/
//: Demonstration class to visualize scalar data on a surface mesh.
//   This is only a demonstration class, given with source code. 
//   As it uses only some of the features of 3D-data master, the limitations
//   of this class are not necessary limitation of 3D-data Master.
// 
//   This class is a part of the MeshViewer class. It cannot be instantiate
//   apart from MeshViewer. It controls the visualization of nodes that
//   depend on scalar data of a surface mesh (derived from PbMesh2D).
//
//   The associated dialog box controls 
//(      . the choice of a scalar data set used as Z value to produce
//         a representation of the mesh with an elevation.
//       . the visibility of an instance of PoMeshFilled
//       . the visibility of an instance of PoMeshSides
//       . the visibility of an instance of PoMeshLimit
//       . the visibility of an instance of PoMeshLines
//       . the visibility of an instance of PoMeshContouring
//       . the visibility of the annotation of isovalues on contouring lines.
//       . differents attributes to place these annotations
//)
/******************************************************************************/
class Mesh2DScalarViewer : public MeshViewerComponent {
public:
  Mesh2DScalarViewer();
    // Constructor.

  void buildSceneGraph(const PbMesh *mesh, PoDomain *domain, 
		       SoGroup *root,
		       SoSFInt32 *which_dataset,
		       SoSFEnum* &which_coloring_type);
    // Builds the scene graph parts corresponding to scalar data on the mesh

  SoDialogComponent *buildDialogBox (const PbMesh *, const char **dataset_name);
    // Creates the dialog box.
  
  void setAltitudeDataSet(int dataset);
    // change used dataset

  void setAltitudeEnable(SbBool enable);
    // Allow viewing altitude or not
  void setAnnoPath(int sel);
    // change annot path

  void setCrossStat(int sel);
    // change cross stat

  void setMeshFilled(SbBool state);
    // fill mesh

  void setEnableContourAnnot(SbBool enable);
    // hide or show contour ressources

  void preWriteAction();
    // Disables the field's connection during writing an Iv file

  void postWriteAction();
    // Re-enables the field's connection after writing an Iv file

private:
  void enableConnection(SoNode *node, SbBool flag);

  SoDialogGroup       *v_DialogBox;

  PoMeshFilled            *v_MeshFilled;
  PoMeshLimit             *v_MeshLimit;
  PoMeshLines             *v_MeshLines;
  PoMeshContouring        *v_MeshContouring;
  PoMeshSides             *v_MeshSides;

  SoSwitch                *v_MeshFilledSwitch;
  SoSwitch                *v_MeshLimitSwitch;
  SoSwitch                *v_MeshLinesSwitch;
  SoSwitch                *v_MeshContouringSwitch;
  SoSwitch                *v_MeshSidesSwitch;

  PoDomain                *v_Domain;
  float                   *v_Vmin, *v_Vmax;
  int                     v_DataSetNum;
  int                     v_ZDataSetIndex;

};
