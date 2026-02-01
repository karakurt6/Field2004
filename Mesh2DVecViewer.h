//****************************************************************************
//                                                                         
// demonstration class to visualize vector data on a surface mesh 
// with 3D-Data-Master  
//                                                                         
//  author : J-Michel Godinaud                                             
//****************************************************************************

#include <DataViz/3Ddata/PbMesh.h>
#include <DataViz/nodes/PoDomain.h>
#include <DataViz/nodes/PoDataMapping.h>

#include <DataViz/3Ddata/PoMesh2DVec.h>

#include <Inventor/nodes/SoSwitch.h>
#include <Inventor/draggers/SoJackDragger.h>
#include <Inventor/nodes/SoPickStyle.h>

#include <DialogViz/SoDialogVizAll.h>

#include "MeshViewerComponent.h"

/******************************************************************************/
//: Demonstration class to visualize vector data on a surface mesh.
//   This is only a demonstration class, given with source code. 
//   As it uses only some of the features of 3D-data master, the limitations
//   of this class are not necessary limitation of 3D-data Master.
// 
//   This class is a part of the MeshViewer class. It cannot be instantiate
//   apart from MeshViewer. It controls the representation of a vector field
//   on a surface mesh (derived from PbMesh2D).
//
//   The associated dialog box controls 
//(      . the visibility of an instance of PoMesh2DVec
//       . the length of the vectors
//       . the shape type and sizes at the end of the vectors
//)
//   This class can be easily extended to manage more attributes of the
//   vector representation (see the different field members of PoMesh2DVec)
/******************************************************************************/
class Mesh2DVecViewer : public MeshViewerComponent {
public:
  Mesh2DVecViewer();
    // Constructor.

  void buildSceneGraph(const PbMesh *mesh, 
		       PoDomain *domain, 
		       SoGroup *root,
		       SoSFInt32 *which_dataset,
		       SoSFEnum *which_coloring_type,
		       PoDataMapping *module_data_mapping);
    // Builds the scene graph parts corresponding to the vector's field

  SoDialogComponent *buildDialogBox (const PbMesh *);
    // Creates the dialog box.

  void setLengthValue(float val);
    // change vector length value

  void setArrowType(int val);
    // change arrow type

  void setArrowHeightType(int val);
    // change qrrow height type

  void preWriteAction();
    // Disables the field's connection during writing an Iv file

  void postWriteAction();
    // Re-enables the field's connection after writing an Iv file

private:
  void enableConnection(SoNode *node, SbBool flag);

  static void motionCallback(void *userData, SoDragger *dragger);
  
  SoDialogComponent               *v_DialogBox;

  PoMesh2DVec                     *v_MeshVecField;
  SoSwitch                        *v_MeshVecFieldSwitch;

  float                           v_VecLengthFactor;

};
