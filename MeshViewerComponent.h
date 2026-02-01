#ifndef _MESH_VIEWER_COMPONENT_
#define _MESH_VIEWER_COMPONENT_
#include <Inventor/SbLinear.h>

class MeshViewerComponent {

 public:
  void connectDragger(SbBool flg) { v_IsDraggerConnected = flg; }

  struct Error
  {
  };

 protected:
  MeshViewerComponent() : 
    v_IsDraggerConnected(TRUE) {
  }

  SbBool   v_IsDraggerConnected;

};

#endif
