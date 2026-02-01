//****************************************************************************
//                                                                         
// demonstration classes to visualize stream on mesh with 3D-Data-Master  
//                                                                         
//  author : J-Michel Godinaud                                             
//****************************************************************************

#include <DataViz/3Ddata/PbMesh.h>
#include <DataViz/nodes/PoDomain.h>
#include <DataViz/nodes/PoDataMapping.h>

#include <DataViz/3Ddata/PoStreamLine.h>
#include <DataViz/3Ddata/PoStreamSphereMotion.h>
#include <DataViz/3Ddata/PoStreamLineMotion.h>
#include <DataViz/3Ddata/PoStreamTadpoleMotion.h>
#include <DataViz/3Ddata/PoStreamPointMotion.h>
#include <DataViz/3Ddata/PoStreamSurface.h>

#include <Inventor/nodes/SoSwitch.h>
#include <Inventor/draggers/SoJackDragger.h>
#include <Inventor/nodes/SoPickStyle.h>

#include <DialogViz/SoDialogVizAll.h>

#include "MeshViewerComponent.h"

/******************************************************************************/
//: Demonstration class to visualize stream on mesh with 3D-Data-Master.
//   This is only a demonstration class, given with source code. 
//   As it uses only some of the features of 3D-data master, the limitations
//   of this class are not necessary limitation of 3D-data Master.
// 
//   This class is a part of the MeshViewer class. It cannot be instantiate
//   apart from MeshViewer. It controls the visualization of streamlines or
//   the animation of particles along streamlines.
//
//   The associated dialog box controls 
//(      . the shape of the list sources of the streamlines
//         2 shapes (line or circle) are available here, but it would be very
//         easy to create other shapes
//       . the number of sources of streamline that compose the shape
//       . the size of the shape relative to the size of the dragger
//       . the type of representation of stream. streamline, streamsurface or
//         particle animation
//       . when the particle animation is chosen, it controls also
//(         . if each particle start at the same time from the sources (true if
//            the toggle button "random start" is set to no)
//          . the time step between 2 consecutives particles that comes from
//            the same source
//          . the possibility to stop the animation of particle and to see 
//            it frame after frame
//          . the length of "tadpoles" if the stream representation is
//            tadpole (see PoStreamTadpoleMotion)
//)
//   The position of the sources is relative to the position and orientation of 
//   the dragger. But this one can be deconnected with a toggle button. In this
//   case, the source and therefore the streamlines are not affected by the
//   motion of the dragger.
//
//   This class can be easily extended to manage more attributes of the
//   stream representation (see PoBaseStreamLine or PoStreamParticleMotion)
//   For example, it can be usefull to define some conditions to stop 
//   the computation of streamlines when some critical points exist in the
//   flow (attrictive focus or attractive nodes for example may produce infinite
//   streamlines).
/******************************************************************************/

class MeshStreamViewer : public MeshViewerComponent {
public:
  MeshStreamViewer();
    // Constructor.

  void buildSceneGraph(const PbMesh *mesh, PoDomain *domain, 
		       SoGroup *root,
		       SoSFInt32 *which_dataset,
		       SoSFEnum *which_coloring_type,
		       PoDataMapping *module_data_mapping);
    // Builds the scene graph parts corresponding to stream visualization.

  SoDialogComponent *buildDialogBox (const PbMesh *);
    // Creates the dialog box
  
  void updateAllConnectedToDragger(SoJackDragger *dragger, SbVec3f plane_normal);
    // When the dragger is connected, update the position of the stream sources,
    // and therefore the representation of the streamlines.

  void preWriteAction();
    // Disables the field's connection during writing an Iv file

  void postWriteAction();
    // Re-enables the field's connection after writing an Iv file

  void setAnimationFrame();
    // viewFrame ++

  void setAnimationLengthOfTadpole(float length);
    // change animation length of tadpole.

  void setSourceShape(int selected);
    // change source shape type
  
  void setSourceShapeSize(float size);
    // change source shape size

  void setSourceStartPoints(int num);
    // change number of source start points

  void setStreamLinesStyle(int style);
    // change stream lines style


private:
  void enableConnection(SoNode *node, SbBool flag);

  static void motionCallback(void *userData, SoDragger *dragger);

  void moveStreamSource();

  SbVec3f                         v_DraggerPos;
  SbVec3f                         v_DraggerScale;
  SbRotation                      v_DraggerRotation;

  PoStreamSurface                 *v_MeshStreamSurfaces;
  PoStreamLine                    *v_MeshStreamLines;
  PoStreamSphereMotion            *v_MeshStreamSphere;
  PoStreamTadpoleMotion           *v_MeshStreamTadpole;
  PoStreamLineMotion              *v_MeshStreamLineParticle;
  PoStreamPointMotion             *v_MeshStreamPoint;
  SoSeparator                     *v_StreamSource;

  SoSwitch                       *v_MeshStreamSwitch;
  SoSwitch                       *v_MeshStreamRepSwitch;
  SoSwitch                       *v_StreamSourceSwitch;

  SoDialogRealSlider             *v_animationLengthOfTadpole;

  float                          v_TadpoleLengthFactor;

  int                            v_ViewFrame;
  int                            v_NumStartPoints;

  enum SourceShapeType {
    SOURCE_CIRCLE,
    SOURCE_LINE
  };

  SourceShapeType                v_SourceShape;
  float                          v_SourceSizeFactor;            
};
