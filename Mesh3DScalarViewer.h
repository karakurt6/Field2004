//****************************************************************************
//                                                                         
// demonstration class to visualize scalar data on a volume mesh 
// with 3D-Data-Master  
//                                                                         
//  author : J-Michel Godinaud                                             
//****************************************************************************

#include <DataViz/3Ddata/PbMesh3D.h>
#include <DataViz/nodes/PoDomain.h>

#include <DataViz/3Ddata/PoMeshSkin.h>
#include <DataViz/3Ddata/PoMeshSkeleton.h>
#include <DataViz/3Ddata/PoMeshLevelSurf.h>
#include <DataViz/3Ddata/PoMeshCrossSection.h>
#include <DataViz/3Ddata/PoMeshCrossContour.h>

#include <Inventor/Xt/viewers/SoXtExaminerViewer.h>
#include <Inventor/nodes/SoClipPlane.h>
#include <Inventor/nodes/SoSwitch.h>
#include <Inventor/draggers/SoJackDragger.h>
#include <Inventor/nodes/SoPickStyle.h>

#include <DialogViz/SoDialogVizAll.h>

#include "MeshViewerComponent.h"

/******************************************************************************/
//: Demonstration class to visualize scalar data on a volume mesh.
//   This is only a demonstration class, given with source code. 
//   As it uses only some of the features of 3D-data master, the limitations
//   of this class are not necessary limitation of 3D-data Master.
// 
//   This class is a part of the MeshViewer class. It cannot be instantiate
//   apart from MeshViewer. It controls the visualization of nodes that
//   depend on scalar data of a volume mesh (derived from PbMesh3D).
//
//   The associated dialog box controls 
//(      . the choice of a scalar data set used to compute a level surface.
//         (see PoMeshLevelSurf).
//       . the visibility and the value of this level surface.
//       . the visibility of the mesh skin (see PoMeshSkin).
//       . the visibility of the mesh skeleton (see PoMeshSkeleton).
//       . the visibility of a mesh cross contour (see PoMeshCrossContour).
//       . the visibility of a mesh cross section (see PoMeshCrossSection).
//       . the transparency of the skin.
//       . the activation of the clip plane (see SoClipPlane). When activated, 
//         the skin and the level surface are clipped.
//)
//   The position of the cross section, cross contour and clip plane are
//   relative to the position and orientation of the dragger. But this one
//   can be deconnected with a toggle button. In this case, the position 
//   of the cross section, cross contour and clip plane are not affected by
//   the motion of the dragger.
//    
/******************************************************************************/

class Mesh3DScalarViewer : public MeshViewerComponent {
public:
  Mesh3DScalarViewer(SoTopLevelDialog *dialogWindow, SoXtExaminerViewer *viewer);
    // Constructor.

  void buildSceneGraph(const PbMesh *mesh, PoDomain *domain, 
		       SoGroup *root,
		       SoSFInt32 *which_dataset,
		       SoSFEnum* &which_coloring_type);
    // Builds the scene graph parts corresponding to scalar data on the mesh

  SoDialogComponent *buildDialogBox (const PbMesh *, const char **dataset_name);
    // Creates the dialog box.
  
  void updateAllConnectedToDragger(SoJackDragger *dragger, SbVec3f plane_normal);
    // When the dragger is connected, update the cross section, cross contour 
    // and clip plane.

  void preWriteAction();
    // Disables the field's connection during writing an Iv file

  void postWriteAction();
    // Re-enables the field's connection after writing an Iv file

  void setTransparencyValue(float val);
    // change the mesh skin level of transparency

  void setTransparencyType(SoGLRenderAction::TransparencyType type);
    // change the transparency type

  void setDatasetForLevelSurf(int datasetid);
    // change the dataset used by the levelsurf

private:
  void enableConnection(SoNode *node, SbBool flag);

  SoDialogGroup                   *v_DialogBox;

  PoMeshSkin                      *v_MeshSkin;
  PoMeshCrossSection              *v_MeshCrossSection;
  PoMeshSkeleton                  *v_MeshSkeleton;
  PoMeshLevelSurf                 *v_MeshLevelSurf;
  PoMeshCrossContour              *v_MeshCrossContour;

  SoClipPlane                    *v_ClipPlane;

  SoSwitch                       *v_MeshSkinSwitch;
  SoSwitch                       *v_MeshCrossSectionSwitch;
  SoSwitch                       *v_MeshSkeletonSwitch;
  SoSwitch                       *v_MeshLevelSurfSwitch;
  SoSwitch                       *v_MeshCrossContourSwitch;

  int                            v_DataSetIndex;
  int                            v_DataSetIndexOfLevelSurface;

  float                          v_TransparencyValue;
  SoGLRenderAction::TransparencyType v_TransparencyType;

  PoDomain                       *v_Domain;

  float                          *v_Vmin, *v_Vmax;
  int                            v_DataSetNum;
  char                           **v_DataSetNames;

  SoXtExaminerViewer             *v_Viewer;
  SoTopLevelDialog               *v_DialogWindow;

};
