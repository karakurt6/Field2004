//****************************************************************************
//                                                                         
// abstract demonstration class to visualize mesh with 3D-Data-Master 
//                                                                         
//  author : J-Michel Godinaud                                             
//****************************************************************************
#include <string.h>

#include <DialogViz/SoDialogVizAll.h>

#include <Mesh2DScalarViewer.h>
#include <Mesh3DScalarViewer.h>
#include <Mesh2DVecViewer.h>
#include <Mesh3DVecViewer.h>
#include <MeshStreamViewer.h>
#include <MeshProbeViewer.h>

#include <DataViz/nodes/PoMeshProperty.h>
#include <DataViz/nodes/PoCartesianGrid2D.h>
#include <DataViz/nodes/PoParalCartesianGrid2D.h>
#include <DataViz/nodes/PoRegularCartesianGrid2D.h>
#include <DataViz/nodes/PoPolarGrid2D.h>
#include <DataViz/nodes/PoIndexedMesh2D.h>
#include <DataViz/nodes/PoQuadrangleMesh2D.h>
#include <DataViz/nodes/PoTriangleMesh2D.h>

#include <DataViz/nodes/PoDomain.h>
#include <DataViz/nodes/PoNonLinearDataMapping2.h>
#include <DataViz/nodes/PoIsovaluesList.h>
#include <DataViz/nodes/PoMiscTextAttr.h>

#include <DataViz/3Ddata/PbMesh.h>

#include <DataViz/graph/PoBase.h>
#include <DataViz/graph/PoNonLinearValueLegend3.h>

#include <Inventor/Xt/SoXt.h>
#include <Inventor/Xt/viewers/SoXtExaminerViewer.h>
#include <Inventor/draggers/SoJackDragger.h>
#include <Inventor/nodes/SoAnnotation.h>
#include <Inventor/nodes/SoAnnoText3Property.h>
#include <Inventor/nodes/SoTranslation.h>
#include <Inventor/nodes/SoSeparator.h>
#include <Inventor/nodes/SoSwitch.h>
#include <Inventor/nodes/SoFont.h>
#include <Inventor/nodes/SoText2.h>
#include <Inventor/nodes/SoAnnoText3Property.h>
#include <Inventor/actions/SoWriteAction.h>
#include <Inventor/SbLinear.h>
#include <Inventor/SbViewportRegion.h>

#include <Inventor/nodes/SoPerspectiveCamera.h>
#include <Inventor/nodes/SoOrthographicCamera.h>

#define ALLOC(_num,_type)           (_type *)malloc((_num)*sizeof(_type))

static char  Scene3DName[]="Scene3D";

/******************************************************************************/
//: Specific examiner viewer used by MeshViewer demonstration class.
//   This class differs from its inherited class SoXtExaminerViewer only by
//   its viewAll() method. Here, viewAll ignore the 2D part of the scene graph
//   (legend, courtesy title and probe informations) so the position of the
//   camera is calculated only to view entirely the representation of the mesh. 
/******************************************************************************/
class PoXtExaminerViewer : public SoXtExaminerViewer {
public:
  PoXtExaminerViewer(Widget parent=NULL, const char *name=NULL,
		     SbBool buildInsideParent=TRUE, 
		     SoXtFullViewer::BuildFlag flag=BUILD_ALL,
		     SoXtViewer::Type type=BROWSER) ;
  void viewAll() ;
    // Changes the position of the camera to view entirely the different
    // nodes that represent the mesh.
} ;

/******************************************************************************/
//: Demonstration class to visualize mesh with 3D-Data-Master.
//   This is only a demonstration class, given with source code. 
//   As it uses only some of the features of 3D-data master, the limitations
//   of this class are not necessary limitation of 3D-data Master.
//
//   It allows visualization of many scalar data sets on a mesh and one
//   vector data set. Any kind of mesh (surface of volume) available
//   with 3D-data Master can be visualized. The interactivity is managed
//   with Dialog Master.
//   Note that DataViz must be initialized before using this class.
//   (call PoBase::init()).
//   
//   This class uses one graphic window managed by PoXtExaminerViewer class and
//   at least 3 dialog boxes. The first dialog box (of which title is "Mesh 
//   Viewer") controls 
//(      . the visibility of the other dialog boxes
//       . the selection of the scalar data set to be visualized.
//       . the type of coloring 
//       . the orientation X,Y,Z of the dragger tools
//       . the visibility of the dragger tools and the legend
//       . the writing of an .iv file
//)
//   The "Scalar viewer" dialog box is managed either by the class 
//   Mesh2DScalarViewer if the mesh is a surface mesh (derived from PbMesh2D)
//   or by the class Mesh3DScalarViewer if the mesh is a volume mesh (derived 
//   from PbMesh3D). This box controls the representations that depend only 
//   to scalar data.
//
//   The "Probe viewer" dialog box is managed by the class MeshProbeViewer.
//   It controls the visibility of the mesh cell that contain the position
//   of the dragger.
//   
//   The "Vector viewer" dialog box is managed either by the class 
//   Mesh2DVecViewer if the mesh is a surface mesh (derived from PbMesh2D)
//   or by the class Mesh3DVecViewer if the mesh is a volume mesh (derived 
//   from PbMesh3D). This box controls the representation of a vector field.
//   This box is visible only if the mesh has a vector data set.
//
//   The "Stream viewer" dialog box is managed by the class MeshStreamViewer.
//   It controls the visibility and many attributes of streamlines.
//   This box is visible only if the mesh has a vector data set.
//
//   This viewer uses a SoJackDragger to control the position of different
//   representations of the mesh. The reference manual of Open Inventor
//   gives some information about handling this tools.
//
//   The dragger is used to
//(      . manage the position of the probe
//       . manage the position of streamline sources if the mesh has a vector
//         data set
//)
//   For a volume mesh, the dragger is also used to
//(      . manage the position of the cross section (see PoMeshCrossSection) and 
//         the cross contour (see PoMeshCrossContour)
//       . manage the position of a plane on which a vector's field
//         is represented (see PoMesh3DVecGridCrossSection)
//       . manage the position of the clipping plane (see SoClipPlane)
//)
/******************************************************************************/
class MeshViewer {
public:
  MeshViewer(Widget widget);
    // Constructor.

  struct Error
  {
  };

  void setDataSetNames(int num_names, char **names);
    // Sets the names of the data sets. They are used in a choice menu.

  void setWindowTitle(char *window_title);
    // Sets the title of the graphic window.

  void setCourtesyTitle(char *courtesy_title);
    // Sets the courtesy title displayed at the right-bottom of the graphic window.

  void setIvFileName(const char *filename);
    // Sets the filename of the .iv file used to save the current scene-graph.

  void setDataSetIndex(int index);
    // Sets the index of the data-set used by default. 0 by default.

  void setNumIsoValues(int num_iso);
    // Sets the number of iso-values. 16 by default.

  void enableMaxThreshold(SbBool is_enable);
    // Enables/disables a max threshold for data-mapping. When enable,
    // it assumes that the greatest value in the current data set
    // represents an "undefined" value, and the data mapping is defined
    // in order that the representations ignore all the cells
    // that contains at least one "undefined" node.

  void setDraggerNormal(SbVec3f normal);
    // Sets the initial orientation of the dragger. (0,0,1) by default.

  void setTabWindowSelectedPage(SbString id);
    // change selected tab page.

  void saveIvFile(SbString filename, SbBool binary);
    // save scenegraph in an Inventor file

  void openIvFile(SbString filename);
    // open scenegraph in an Inventor file

  void setWindowsDisposition(SbBool state);
    // change windows disposition : merge or separate.

  void show(PoMeshProperty *mesh_node, PoDomain *domain=NULL);
    // Shows the mesh.

  void show(char *iv_file_name);
    // Shows the mesh from an iv file

  void show();
    // Shows the mesh selected in a file selection box

private:
  void startShow(Widget my_window);
  void addAuditors(const PbMesh* mesh);
  void updateAuditors(const PbMesh *mesh);
  void enableMenuBar(SbBool state);

  void buildMainWindow();
  void closeTabWindow();
  void buildTabWindow(PoMeshProperty *mesh_node, PoDomain *domain);
  PoMeshProperty* getMeshFromIvFile(const char* iv_file_name);

  static void motionCallback(void *userData, SoJackDragger *dragger);

  void setDraggerPos (const SbVec3f &pos);

  /* methods used when writing an Iv file */
  void preWriteAction();
  void postWriteAction();

  /* methods used when reading an 3DMS data-file */
  void buildScene2D(SoGroup *probe_text_info_group);
  void buildDataMappings(const PbMesh *mesh);
  SoSeparator * myLegend(char * fontName);

  SoTopLevelDialog        *v_MainWindow;
  SoTopLevelDialog        *v_TabWindow;
  SoTabDialog             *v_TabDialog;
  SoDialogComponent       *v_DialogBoxScalar,
                          *v_DialogBoxProbe,
                          *v_DialogBoxVec,
                          *v_DialogBoxStream;

  PoNonLinearValueLegend3 *v_DataLegend;

  SoSwitch                *v_DataSwitch;
  SoSwitch                *v_DataLegendSwitch;
  SoSeparator             *v_Root, 
                          *v_Scene3D, 
                          *v_SceneScalar, 
                          *v_SceneStream, 
                          *v_SceneVec,
                          *v_SceneProbe;

  SoAnnotation            *v_Scene2D;

  PoXtExaminerViewer      *v_Viewer;
  SoSFEnum                *v_ColoringTypeField;
  SoJackDragger           *v_Dragger;
  SoSwitch                *v_DraggerSwitch;

  int                     v_DataSetNum;
  int                     v_DataSetIndex;
  float                   *v_Vmin, *v_Vmax;
  float                   v_ScaleFactor;
  float                   v_VecLengthFactor;

  int                     v_DataSetNamesAlloc;
  char                    **v_DataSetNames;
  char                    v_WindowTitle[80];
  char                    v_CourtesyTitle[80];
  int                     v_NumIsoValues;

  PbDomain                *v_Domain;
  PoIsovaluesList         *v_IsoValues;
  PoNonLinearDataMapping2 *v_DataMapping;
  PoNonLinearDataMapping2 *v_ModuleDataMapping;

  SbBool                  v_MaxThresholdIsEnable;
  SbBox3f                 v_BoundingBox;
  
  char                    v_IvAsciiFileName[100];
  char                    v_IvBinaryFileName[100];

  Mesh2DScalarViewer      *v_Mesh2DScalarViewer;
  Mesh3DScalarViewer      *v_Mesh3DScalarViewer;
  Mesh2DVecViewer         *v_Mesh2DVecViewer;
  Mesh3DVecViewer         *v_Mesh3DVecViewer;
  MeshStreamViewer        *v_MeshStreamViewer;
  MeshProbeViewer         *v_MeshProbeViewer;

  SbVec3f                 v_DraggerNormal;
  SbVec3f                 v_DraggerPos;

  Widget                  v_BaseWidget;
  SoDialogCustom          *v_ParentViewer;
  SoMenuBar               *v_MenuBar;
  
};

