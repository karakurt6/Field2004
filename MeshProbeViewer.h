//****************************************************************************
//                                                                         
// demonstration class to get probe informations with 3D-Data-Master  
//                                                                         
//  author : J-Michel Godinaud                                             
//****************************************************************************

#include <DataViz/3Ddata/PbMesh.h>
#include <DataViz/3Ddata/PbCell.h>
#include <DataViz/nodes/PoDomain.h>
#include <DataViz/nodes/PoDataMapping.h>
#include <DataViz/nodes/PoMeshProperty.h>

#include <DataViz/3Ddata/PoMeshProbePoint.h>
#include <DataViz/3Ddata/PoCellFacets.h>
#include <DataViz/3Ddata/PoCellIndices.h>
#include <DataViz/3Ddata/PoCellEdges.h>

#include <Inventor/nodes/SoSwitch.h>
#include <Inventor/nodes/SoLevelOfDetail.h>
#include <Inventor/nodes/SoText2.h>
#include <Inventor/nodes/SoTranslation.h>
#include <Inventor/nodes/SoPickStyle.h>
#include <Inventor/nodes/SoBlinker.h>
#include <Inventor/draggers/SoJackDragger.h>

#include <DialogViz/SoDialogVizAll.h>

#include "MeshViewerComponent.h"

/******************************************************************************/
//: Demonstration class to get probe informations with 3D-Data-Master.
//   This is only a demonstration class, given with source code. 
//   As it uses only some of the features of 3D-data master, the limitations
//   of this class are not necessary limitation of 3D-data Master.
// 
//   This class is a part of the MeshViewer class. It cannot be instantiate
//   apart from MeshViewer. It controls the visualization of the cell that
//   contains the position of the dragger, and it gets probe informations
//   (the value interpolated at the dragger position and the index of the cell
//   that contains this position). The probe informations are drawn at the top
//   left corner of the graphic window.
//
//   The associated dialog box controls 
//(      . the visibility of the facets of the cell that contains the probe 
//         position (see PoCellFacets)
//       . the visibility of the edges of the cell that contains the probe 
//         position (see PoCellIndices)
//       . the visibility of the node indices of the cell that contains the  
//         probe position (see PoCellEdges)
//       . the gap between the cell nodes and the annotated indices
//)
//   The dragger can be deconnected with a toggle button. In this case, the cell
//   that is drawn and the probe informations at the top left corner of the 
//   graphic window are not affected by the motion of the dragger.
/******************************************************************************/
class MeshProbeViewer : public MeshViewerComponent {
public:
  MeshProbeViewer();
    // Constructor.

  void buildSceneGraph(PoMeshProperty *mesh_node,
		       PoDomain *domain, 
		       SoGroup *root,
		       SoSFInt32 *which_dataset,
		       SoSFEnum *which_coloring_type,
		       SoJackDragger *dragger,
		       SoGroup *probe_text_info_group);
    // Builds the scene graph parts corresponding to the probe informations

  SoDialogComponent *buildDialogBox (const PbMesh *);
    // Creates the dialog box.
  
  void updateAllConnectedToDragger(SoJackDragger *dragger, SbVec3f plane_normal);
    // When the dragger is connected, update the text probe informations at the 
    // top left corner of the graphic window, and if the dragger as moved to 
    // another cell draw this new cell and clear the previous one.

  void preWriteAction();
    // Disables the field's connection during writing an Iv file

  void postWriteAction();
    // Re-enables the field's connection after writing an Iv file

private:
  void enableConnection(SoNode *node, SbBool flag);

  static void motionCallback(void *userData, SoDragger *dragger);
  
  SoDialogGroup                   *v_DialogBox;

  PoMeshProbePoint                *v_MeshProbePoint;
  PoCellFacets                    *v_CellFacets;
  PoCellIndices                   *v_CellIndices;
  PoCellEdges                     *v_CellEdges;

  SoSwitch                        *v_CellFacetsSwitch;
  SoSwitch                        *v_CellEdgesSwitch;
  SoSwitch                        *v_CellIndicesSwitch;

};

void change_cell_probeCB(void *userData, PoMeshProbePoint *probe, const PbCell *cell);
void motion_probeCB(void *userData, PoMeshProbePoint *probe, const PbCell *cell);
void enter_probeCB(void *userData, PoMeshProbePoint *probe, const PbCell *cell);
void leave_probeCB(void *userData, PoMeshProbePoint *probe, const PbCell *cell);


