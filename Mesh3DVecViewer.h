//****************************************************************************
//                                                                         
// demonstration class to visualize vector data on a volume mesh 
// with 3D-Data-Master  
//                                                                         
//  author : J-Michel Godinaud                                             
//****************************************************************************

#include <DataViz/3Ddata/PbMesh3D.h>
#include <DataViz/nodes/PoDomain.h>
#include <DataViz/nodes/PoDataMapping.h>

#include <DataViz/3Ddata/PoMesh3DVecGridCrossSection.h>

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
//   on a volume mesh (derived from PbMesh3D).
//
//   The associated dialog box controls 
//(      . the visibility of the vectors on a cross section inside the mesh.
//         (see PoMesh3DVecGridCrossSection)
//       . the fineness of the grid on which the vectors are drawn.
//       . the length of the vectors.
//       . the type of projection of the vectors.
//)
//   The position of the cross section on which the vectors are drawn is 
//   relative to the position and orientation of the dragger. But this one
//   can be deconnected with a toggle button. In this case, the position 
//   of the cross section is not affected by the motion of the dragger.
//
//   This class can be easily extended to manage more attributes of the
//   vector representation (see the different field members of PoMesh3DVec)
/******************************************************************************/
class Mesh3DVecViewer : public MeshViewerComponent {
public:
  Mesh3DVecViewer();
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
  
  void updateAllConnectedToDragger(SoJackDragger *dragger, SbVec3f plane_normal);
    // When the dragger is connected, update the cross section position 

  void preWriteAction();
    // Disables the field's connection during writing an Iv file

  void postWriteAction();
    // Re-enables the field's connection after writing an Iv file

  void setLengthValue(float val);
    // change vector length

  void setProjectionType(PoMesh3DVecGridCrossSection::ProjectionType type);
    // change vector projection type

private:
  void enableConnection(SoNode *node, SbBool flag);

  static void motionCallback(void *userData, SoDragger *dragger);
  
  SoDialogGroup               *v_DialogBox;

  PoMesh3DVecGridCrossSection     *v_MeshVecField;
  SoSwitch                        *v_MeshVecFieldSwitch;

  float                           v_VecLengthFactor;

};
