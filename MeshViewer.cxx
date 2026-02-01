//------------------------------------------------------------------------
//                                                                        
// abstract demonstration class to visualize mesh with 3D-Data-Master 
//                                                                        
//  author : J-Michel Godinaud                                            
//------------------------------------------------------------------------

#include "MeshViewer.h"
#include <DataViz/nodes/PoMeshProperty.h>
#include <DataViz/3Ddata/PoMeshFilled.h>
#include <Inventor/nodes/SoPolygonOffset.h>
#include <Inventor/actions/SoSearchAction.h>
#include <Inventor/SoPath.h>

MeshViewer::MeshViewer(Widget widget) :
  v_DataSetNamesAlloc(0),
  v_DataSetIndex(0),
  v_DraggerSwitch(NULL),
  v_ScaleFactor(1),
  v_NumIsoValues(16),
  v_DraggerNormal(0,0,1),
  v_Mesh2DScalarViewer(NULL),
  v_Mesh3DScalarViewer(NULL),
  v_Mesh2DVecViewer(NULL),
  v_Mesh3DVecViewer(NULL),
  v_MeshStreamViewer(NULL),
  v_MeshProbeViewer(NULL),
  v_MaxThresholdIsEnable(FALSE),
  v_TabWindow(NULL),
  v_Root(NULL)
{
  // Initialize Open-Inventor classes
  v_BaseWidget = widget;

  v_Dragger = new SoJackDragger;
  strcpy(v_WindowTitle,"mesh_viewer");
  strcpy(v_CourtesyTitle," ");
  strcpy(v_IvAsciiFileName,"mesh.iv");
  strcpy(v_IvBinaryFileName,"mesh_bin.iv");

}/*---------------------------------------------------------------------------*/

void MeshViewer::setDataSetNames(int num_names, char **names) {
  int i;
  if (v_DataSetNamesAlloc) {
    for (i=0; i<v_DataSetNamesAlloc; i++) free(v_DataSetNames[i]);
    free(v_DataSetNames);
  }

  v_DataSetNamesAlloc = num_names+1;
  v_DataSetNames = ALLOC(v_DataSetNamesAlloc,char *);
  v_DataSetNames[0] = ALLOC(15,char); strcpy(v_DataSetNames[0],"no data");
  for (i=1; i<v_DataSetNamesAlloc; i++) {
    v_DataSetNames[i] = ALLOC(strlen(names[i-1])+1,char);
    strcpy(v_DataSetNames[i],names[i-1]);
  }
}/*---------------------------------------------------------------------------*/

void MeshViewer::setDraggerNormal(SbVec3f normal) {
  v_DraggerNormal = normal;
  setDraggerPos(v_DraggerPos);
}/*---------------------------------------------------------------------------*/

void MeshViewer::setIvFileName(const char *filename) {
  if (filename) strcpy(v_IvAsciiFileName,filename);
}/*---------------------------------------------------------------------------*/

void MeshViewer::setWindowTitle(char *window_title) {
  if (window_title) {
    strcpy(v_WindowTitle,window_title);
    strcat(strcpy(v_IvAsciiFileName,window_title),".iv");
  }
  
}/*---------------------------------------------------------------------------*/

void MeshViewer::setCourtesyTitle(char *courtesy_title) {
  if (courtesy_title) strcpy(v_CourtesyTitle,courtesy_title);
}/*---------------------------------------------------------------------------*/

void MeshViewer::setNumIsoValues(int num_iso) {
  if (num_iso>0) v_NumIsoValues = num_iso;
}/*---------------------------------------------------------------------------*/

void MeshViewer::setDataSetIndex(int index) {
  v_DataSetIndex = index;
}/*---------------------------------------------------------------------------*/

void MeshViewer::enableMaxThreshold(SbBool is_enable) {
  v_MaxThresholdIsEnable = is_enable;
}/*---------------------------------------------------------------------------*/

void MeshViewer::buildDataMappings(const PbMesh *mesh) {

  SbColor colors[5] = {SbColor(0.0,0.0,1.0), SbColor(0.0,1.0,1.0), SbColor(0.0,1.,0.), 
		       SbColor(1.0,1.0,0.0), SbColor(1.0,0.0,0.0)} ;

  v_DataSetNum = mesh->getNumValuesSet();
  v_Vmin = ALLOC(v_DataSetNum,float);
  v_Vmax = ALLOC(v_DataSetNum,float);

  // alloc the data names if necessary
  if (!v_DataSetNamesAlloc) {
    v_DataSetNamesAlloc = v_DataSetNum+1;
    v_DataSetNames = ALLOC(v_DataSetNum+1,char *);
    v_DataSetNames[0] = ALLOC(15,char); strcpy(v_DataSetNames[0],"no data");
    for (int i=1; i<v_DataSetNum+1; i++) {
      v_DataSetNames[i] = ALLOC(15,char);
      sprintf(v_DataSetNames[i],"Data set %d",i);
    }
  } 

  // define the data-mapping associated to each set of data-set
  float val,dv;
  int j;
  v_DataSwitch = new SoSwitch;
  v_DataSwitch->whichChild = v_DataSetIndex;

  for (int i=0; i<v_DataSetNum; i++) {
    // define the data-mapping associated to i-th data-set
    v_DataMapping = new PoNonLinearDataMapping2;
    mesh->getMinValuesSet(i,v_Vmin[i]);
    mesh->getMaxValuesSet(i,v_Vmax[i]);
    if (v_MaxThresholdIsEnable==TRUE) {
      // assume that the greatest value in the current data set
      // represents an "undefined" value. Define a data mapping
      // in order that the representations ignore all the cells
      // that contains at least one "undefined" node.
      float undef_value = v_Vmax[i];
      v_DataMapping->maxThreshold = undef_value;
      v_DataMapping->maxThresholdEnabled = TRUE;
      v_DataMapping->transparencyEnabled = TRUE;
      v_DataMapping->maxThresholdTransparency = 1.0;
      // find the greatest "valid" value (i.e. the greatest
      // one but lower than undef_value) in order to have
      // a correct coloration of the valid regions.
      v_Vmax[i] = -1E30F;
      const float *v = mesh->getValuesSet(i);
      for (j=0; j<mesh->getNumNodes(); j++) 
	if (v_Vmax[i] < v[j] && v[j] < undef_value) v_Vmax[i] = v[j];
    }
    v_DataMapping->type = PoNonLinearDataMapping2::LINEAR_PER_LEVEL;
    float dv = (v_Vmax[i]-v_Vmin[i])/4;
    float val = v_Vmin[i];
    for (j=0; j<5; j++,val+=dv) {
      v_DataMapping->value.set1Value(j,val);
      v_DataMapping->color.set1Value(j,colors[j]);
    }

    // define the list of iso-value associated to i-th data-set
    v_IsoValues = new PoIsovaluesList;
    v_IsoValues->setRegularIsoList(v_Vmin[i],v_Vmax[i],v_NumIsoValues);

    SoGroup *group = new SoGroup;
    group->addChild(v_DataMapping);
    group->addChild(v_IsoValues);
    
    v_DataSwitch->addChild(group);

  }     

  // define the data-mapping associated to the module of vectors
  float vmin,vmax;
  v_ModuleDataMapping = new PoNonLinearDataMapping2;
  v_ModuleDataMapping->type = PoNonLinearDataMapping2::LINEAR_PER_LEVEL;
  if (mesh->getMinVecsSet(0,vmin) && mesh->getMaxVecsSet(0,vmax)) {
    dv = (vmax-vmin)/4;
    val = vmin;
    for (j=0; j<5; j++,val+=dv) {
      v_ModuleDataMapping->value.set1Value(j,val);
      v_ModuleDataMapping->color.set1Value(j,colors[j]);
    }
  }

}/*---------------------------------------------------------------------------*/

void MeshViewer::buildScene2D(SoGroup *probe_text_info_group)
{
  // build courtesy title
  SoFont *font = new SoFont;
  font->size = 12;
  SoTranslation *courtesy_trans = new SoTranslation;
  courtesy_trans->translation.setValue(0.95F, -0.95F, -1);
  SoText2 *courtesy_title = new SoText2;
  courtesy_title->string = v_CourtesyTitle;
  courtesy_title->justification = SoText2::RIGHT;

  // build colored legend
  SoSeparator *legend_sep = myLegend("Courier");

  v_DataLegendSwitch = new SoSwitch;
  v_DataLegendSwitch->addChild(legend_sep) ;
  v_DataLegendSwitch->whichChild = SO_SWITCH_ALL;

  // pick style node
  SoPickStyle *pick_style = new SoPickStyle;
  pick_style->style = SoPickStyle::UNPICKABLE;

  // Text property for the 2D scene
  SoAnnoText3Property *annoText3Property = new SoAnnoText3Property ;
  annoText3Property->renderPrintType = SoAnnoText3Property::RENDER2D_PRINT_RASTER ;
  annoText3Property->fontSizeHint = SoAnnoText3Property::ANNOTATION ;

  v_Scene2D = new SoAnnotation ;
  v_Scene2D->addChild(pick_style);
  v_Scene2D->addChild(annoText3Property);
  v_Scene2D->addChild(v_DataLegendSwitch);
  v_Scene2D->addChild(font);
  v_Scene2D->addChild(courtesy_trans);
  v_Scene2D->addChild(courtesy_title);
  v_Scene2D->addChild(probe_text_info_group);
}/*---------------------------------------------------------------------------*/


PoMeshProperty*
MeshViewer::getMeshFromIvFile(const char *ivFileName) {
  SoInput myInput;
  SoMessageDialog *msgBox;

  if (!myInput.openFile(ivFileName)) {
    msgBox = new SoMessageDialog(ivFileName,
                                 "File not found",SoMessageDialog::MD_WARNING);
    msgBox->show();    
    return NULL;
  }

  SoGroup *myGroup = SoDB::readAll( &myInput );
  if (myGroup == NULL) {
    msgBox = new SoMessageDialog(ivFileName,
                                 "Invalid file format",SoMessageDialog::MD_WARNING);
    msgBox->show();    
    return NULL;
  }

  myGroup->ref();

  SoSearchAction s_action;
  s_action.setType(PoMeshProperty::getClassTypeId());
  s_action.apply(myGroup);

  SoPath *path = s_action.getPath();

  //  myGroup->unrefNoDelete();

  if (path == NULL) {
    SoMessageDialog *msgBox = 
      new SoMessageDialog("This iv file do not contain any mesh definition",
                          "Incorrect iv file",SoMessageDialog::MD_WARNING);
    msgBox->show();    
    return NULL;
  }

  PoMeshProperty *mesh_node = (PoMeshProperty *)path->getTail();
  return mesh_node;
}

/*---------------------------------------------------------------------------*/
void MeshViewer::show() {
  show(NULL,NULL);
}

/*---------------------------------------------------------------------------*/
void MeshViewer::show(char *ivFileName) {
  PoMeshProperty *mesh_node = getMeshFromIvFile(ivFileName);
  show(mesh_node,NULL);
}

/*---------------------------------------------------------------------------*/
void MeshViewer::show(PoMeshProperty *mesh_node, PoDomain *domain) {
  const PbMesh *pb_mesh = NULL;

  // build the dialog box
  buildMainWindow();

  if (mesh_node != NULL) {
    pb_mesh = mesh_node->getMesh();
    buildTabWindow(mesh_node,domain);
  } else {
    enableMenuBar(FALSE);
  }
  
  addAuditors(pb_mesh);
  startShow(v_BaseWidget);
}

/*---------------------------------------------------------------------------*/
void 
MeshViewer::closeTabWindow()
{
  //if (v_TabWindow) v_TabWindow->removeAllChildren();
  if (v_TabWindow) v_TabWindow->close();
}

/*---------------------------------------------------------------------------*/
void 
MeshViewer::buildTabWindow(PoMeshProperty *mesh_node, PoDomain *domain)
{
  const PbMesh *pb_mesh =  mesh_node->getMesh();

//  if (v_TabWindow == NULL) 
    v_TabWindow = new SoTopLevelDialog();

  v_TabDialog = new SoTabDialog();
  v_TabWindow->addChild(v_TabDialog);
  
  buildDataMappings(pb_mesh);

  // get the mesh bounding box
  if (domain) {
    v_BoundingBox.setBounds(domain->min.getValue(),domain->max.getValue());
  } else 
    v_BoundingBox = pb_mesh->getBoundingBox();
  SbVec3f bounding_box_center = v_BoundingBox.getCenter();

  float sx,sy,sz; v_BoundingBox.getSize(sx,sy,sz);
  float maxs = (sx > sy) ? sx : sy; if (sz > maxs) maxs = sz;

  v_ScaleFactor = maxs/10;

  // build the dragger
  v_Dragger->addMotionCallback((SoDraggerCB *)&MeshViewer::motionCallback,this);

  v_DraggerSwitch = new SoSwitch;
  v_DraggerSwitch->addChild(v_Dragger) ;
  v_DraggerSwitch->whichChild = SO_SWITCH_ALL;
   
  // build the scene to visualize scalar data 
  v_SceneScalar = new SoSeparator;

  v_SceneVec = new SoSeparator;

  if (mesh_node->isOfType(PoCartesianGrid2D::getClassTypeId()) ||
      mesh_node->isOfType(PoParalCartesianGrid2D ::getClassTypeId()) ||
      mesh_node->isOfType(PoRegularCartesianGrid2D::getClassTypeId()) ||
      mesh_node->isOfType(PoPolarGrid2D::getClassTypeId()) ||
      mesh_node->isOfType(PoIndexedMesh2D::getClassTypeId()) ||
      mesh_node->isOfType(PoQuadrangleMesh2D::getClassTypeId()) ||
      mesh_node->isOfType(PoTriangleMesh2D::getClassTypeId())
      ) {
    v_Mesh2DScalarViewer = new Mesh2DScalarViewer;
    ((SoMenuPushButton*)v_MainWindow->searchForAuditorId("launchScalar"))->enable = TRUE;

    v_Mesh2DScalarViewer->buildSceneGraph(pb_mesh,
					  domain,
					  v_SceneScalar,
					  &v_DataSwitch->whichChild,
					  v_ColoringTypeField);
    v_DialogBoxScalar = v_Mesh2DScalarViewer->buildDialogBox(pb_mesh,
    							     (const char **)v_DataSetNames);
    v_TabDialog->addChild(v_DialogBoxScalar);

    if (pb_mesh->getNumVecsSet() > 0) {
      v_Mesh2DVecViewer = new Mesh2DVecViewer;
      ((SoMenuPushButton*)v_MainWindow->searchForAuditorId("launchVectors"))->enable = TRUE;
      v_Mesh2DVecViewer->buildSceneGraph(pb_mesh,
					 domain,
					 v_SceneVec,
					 &v_DataSwitch->whichChild,
					 v_ColoringTypeField,
					 v_ModuleDataMapping);
      v_DialogBoxVec = v_Mesh2DVecViewer->buildDialogBox(pb_mesh);
      v_TabDialog->addChild(v_DialogBoxVec);
    }
  } else {
    v_Mesh3DScalarViewer = new Mesh3DScalarViewer(v_TabWindow,v_Viewer);
    ((SoMenuPushButton*)v_MainWindow->searchForAuditorId("launchScalar"))->enable = TRUE;
    v_Mesh3DScalarViewer->buildSceneGraph(pb_mesh,
					  domain,
					  v_SceneScalar,
					  &v_DataSwitch->whichChild,
					  v_ColoringTypeField);
    v_DialogBoxScalar = v_Mesh3DScalarViewer->buildDialogBox(pb_mesh,
							     (const char **)v_DataSetNames);
    v_TabDialog->addChild(v_DialogBoxScalar);

    if (pb_mesh->getNumVecsSet() > 0) {
      v_Mesh3DVecViewer = new Mesh3DVecViewer;
      ((SoMenuPushButton*)v_MainWindow->searchForAuditorId("launchVectors"))->enable = TRUE;
      v_Mesh3DVecViewer->buildSceneGraph(pb_mesh,
					 domain,
					 v_SceneVec,
					 &v_DataSwitch->whichChild,
					 v_ColoringTypeField,
					 v_ModuleDataMapping);
      v_DialogBoxVec = v_Mesh3DVecViewer->buildDialogBox(pb_mesh);
      v_TabDialog->addChild(v_DialogBoxVec);
    }
  }

  v_SceneStream = new SoSeparator;

  if (pb_mesh->getNumVecsSet() > 0) {
    v_MeshStreamViewer = new MeshStreamViewer;
    ((SoMenuPushButton*)v_MainWindow->searchForAuditorId("launchStream"))->enable = TRUE;
    v_MeshStreamViewer->buildSceneGraph(pb_mesh,
					domain,
					v_SceneStream,
					&v_DataSwitch->whichChild,
					v_ColoringTypeField,
					v_ModuleDataMapping);
    v_DialogBoxStream = v_MeshStreamViewer->buildDialogBox(pb_mesh);
    v_TabDialog->addChild(v_DialogBoxStream);
  }

  v_SceneProbe = new SoSeparator;
  v_SceneProbe->boundingBoxCaching = SoSeparator::OFF;

  SoGroup *probe_text_info = new SoGroup;

  v_MeshProbeViewer = new MeshProbeViewer;
  ((SoMenuPushButton*)v_MainWindow->searchForAuditorId("launchProbe"))->enable = TRUE;
  v_MeshProbeViewer->buildSceneGraph(mesh_node,
				     domain,
				     v_SceneProbe,
				     &v_DataSwitch->whichChild,
				     v_ColoringTypeField,
				     v_Dragger,
                                     probe_text_info);
  v_DialogBoxProbe = v_MeshProbeViewer->buildDialogBox(pb_mesh);
  v_TabDialog->addChild(v_DialogBoxProbe);

  // Build the tab panel dialog
  v_TabWindow->buildDialog(v_BaseWidget,FALSE);
  
  // move the dragger at its initial position
  setDraggerPos(v_BoundingBox.getCenter());

  // build the legend, courtesy title ...
  buildScene2D(probe_text_info);

  SoPerspectiveCamera *camera3D = new SoPerspectiveCamera ;

  SoOrthographicCamera *camera2D = new SoOrthographicCamera ;
  camera2D->viewportMapping = SoCamera::LEAVE_ALONE;

  v_Scene3D = new SoSeparator ;
  v_Scene3D->setName("Scene3D");
  v_Scene3D->addChild(camera3D);

  v_Scene3D->addChild(mesh_node);
  v_Scene3D->addChild(v_DataSwitch);
  v_Scene3D->addChild(v_DraggerSwitch);
  v_Scene3D->addChild(new SoPolygonOffset);
  v_Scene3D->addChild(v_SceneScalar);
  v_Scene3D->addChild(v_SceneStream);
  v_Scene3D->addChild(v_SceneProbe);
  v_Scene3D->addChild(v_SceneVec);

  // Text property for the 2D scene
  SoAnnoText3Property *annoText3Property = new SoAnnoText3Property ;
  annoText3Property->renderPrintType = SoAnnoText3Property::RENDER2D_PRINT_RASTER ;
  annoText3Property->fontSizeHint = SoAnnoText3Property::ANNOTATION ;

  v_Root = new SoSeparator ;
  v_Root->ref() ;
  v_Root->addChild(v_Scene3D);
  v_Root->addChild(camera2D);
  v_Root->addChild(annoText3Property);
  v_Root->addChild(v_DataSwitch);
  v_Root->addChild(v_Scene2D);

}/*---------------------------------------------------------------------------*/

void MeshViewer::startShow(Widget v_BaseWidget)
{
  v_Viewer->setSceneGraph(v_Root);
  v_Viewer->setTitle(v_WindowTitle);
  v_Viewer->setTransparencyType(SoGLRenderAction::DELAYED_ADD);
  if (v_Root) v_Viewer->viewAll();
  v_Viewer->show();

  v_MainWindow->show();
  if (v_TabWindow) v_TabWindow->show();

  SoXt::show(v_BaseWidget);
  SoXt::mainLoop();
}/*---------------------------------------------------------------------------*/

SoSeparator * MeshViewer::myLegend(char * fontName) {
  
  // build the legend and set the background legend box invisible.
  v_DataLegend = new PoNonLinearValueLegend3(SbVec2f(-0.95F,-0.95F), SbVec2f(-0.75F, 0.95F)) ;
  v_DataLegend->set("backgroundApp.drawStyle", "style INVISIBLE");
  v_DataLegend->set("backgroundBorderApp.drawStyle", "style INVISIBLE") ;

  // define the font used by the legend
  PoMiscTextAttr * legend_text_attr = new PoMiscTextAttr;
  legend_text_attr->fontName.setValue(fontName);

  SoSeparator *legend_sep = new SoSeparator;
  legend_sep->addChild(legend_text_attr);
  legend_sep->addChild(v_DataLegend);

  return legend_sep;
}/*---------------------------------------------------------------------------*/

PoXtExaminerViewer::PoXtExaminerViewer(Widget parent, const char *name,
				       SbBool buildInsideParent, 
				       SoXtFullViewer::BuildFlag flag,
				       SoXtViewer::Type typeViewer) :
  
  SoXtExaminerViewer(parent,
                     name, buildInsideParent, flag,
		     typeViewer)
{
}/*---------------------------------------------------------------------------*/

void
PoXtExaminerViewer::viewAll()
{
  SoCamera *my_camera = getCamera();
  my_camera->viewAll(SoNode::getByName("Scene3D"), getViewportRegion()) ;
}/*---------------------------------------------------------------------------*/

class EnumFieldAuditor : public SoMenuRadioButtonsAuditor {
  SoSFEnum *field;
public:
  EnumFieldAuditor(SoSFEnum *field) { this->field = field; }
  void  menuRadioButtons(SoMenuRadioButtons *rb) {
    field->setValue(rb->selectedItem.getValue());
  }
};

class IntSwitchAuditor : public SoMenuRadioButtonsAuditor {
  SoSwitch *sw;
public:
  IntSwitchAuditor(SoSwitch *sw) { this->sw = sw; }
  void  menuRadioButtons(SoMenuRadioButtons *rb) {
    sw->whichChild = rb->selectedItem.getValue();
  }
};

class CheckMenuSwitchAuditor : public SoMenuCheckBoxAuditor {
  SoSwitch *sw;
public:
  CheckMenuSwitchAuditor(SoSwitch *sw) { this->sw = sw; }
  void menuCheckBox(SoMenuCheckBox *checkBox) {
    sw->whichChild = (checkBox->state.getValue() == TRUE)
      ? SO_SWITCH_ALL 
      : SO_SWITCH_NONE;
  }
};

class OrientAuditor : public SoMenuRadioButtonsAuditor {
  MeshViewer *mv;
public:
  OrientAuditor(MeshViewer *mv) { this->mv = mv; }
  void  menuRadioButtons(SoMenuRadioButtons *rb) {
    int choice_val = rb->selectedItem.getValue();
    switch (choice_val) {
    case 0 : mv->setDraggerNormal(SbVec3f(1,0,0)); break;
    case 1 : mv->setDraggerNormal(SbVec3f(0,1,0)); break;
    case 2 : mv->setDraggerNormal(SbVec3f(0,0,1)); break;
    }
  }
};

class SelectTabWindowAuditor : public SoMenuPushButtonAuditor {
  MeshViewer *mv;
public:
  SelectTabWindowAuditor(MeshViewer *mv) { this->mv = mv; }
  void menuPushButton(SoMenuPushButton* pb)
  {
    mv->setTabWindowSelectedPage(pb->auditorID.getValue());
  }
};

class MergeWindowsCheckAuditor : public SoMenuCheckBoxAuditor {
  MeshViewer *mv;
public:
  MergeWindowsCheckAuditor(MeshViewer *mv) { this->mv = mv; }
  void menuCheckBox(SoMenuCheckBox *cb) {
    mv->setWindowsDisposition(cb->state.getValue());
  }
};

class SaveIvFileAuditor : public SoMenuFileSelectionAuditor {
  MeshViewer *mv;
  SbBool binary;
public:
  SaveIvFileAuditor(MeshViewer *mv, SbBool binary) { 
    this->mv = mv; 
    this->binary = binary;
  }
  void menuFileSelection(SoMenuFileSelection* fs) {
    mv->saveIvFile(fs->filename.getValue(),binary);
  }
};
class OpenIvFileAuditor : public SoMenuFileSelectionAuditor {
  MeshViewer *mv;
public:
  OpenIvFileAuditor(MeshViewer *mv) { this->mv = mv; }
  void menuFileSelection(SoMenuFileSelection* fs) {
    mv->openIvFile(fs->filename.getValue());
  }
};

void
MeshViewer::setWindowsDisposition(SbBool state)
{
  if (state)
  {
  }
  else
  {
  }
}

void
MeshViewer::setTabWindowSelectedPage(SbString id)
{
  int sel = 0;
  v_TabWindow->show();
  SoDialogGroup* group = NULL;
  if (id == "launchStream")
    group = (SoDialogGroup*)v_TabDialog->searchForAuditorId("StreamTab");
  else if (id == "launchProbe")
    group = (SoDialogGroup*)v_TabDialog->searchForAuditorId("ProbeTab");
  else if (id == "launchVectors")
    group = (SoDialogGroup*)v_TabDialog->searchForAuditorId("VectorTab");
  else if (id == "launchScalar")
    group = (SoDialogGroup*)v_TabDialog->searchForAuditorId("ScalarTab");

  if (group)
    sel = v_TabDialog->findChild(group);

  v_TabDialog->selectedPage.setValue(sel);
}

/*---------------------------------------------------------------------------*/
void
MeshViewer::saveIvFile(SbString filename, SbBool binary)
{
  // Save the iv file in "FOLD" format, 
  // Attention : This saved file cannot be read by an application that 
  // is not linked with DataViz (used UNFOLD for that!)
  PoBase::setNodeWriteFormat(PoBase::FOLD_NODE_WRITE_FORMAT);      
  preWriteAction();
  SoWriteAction myAction ;
  myAction.getOutput()->openFile(filename.getString());
  myAction.getOutput()->setBinary(binary) ;
  myAction.apply(v_Root) ;
  myAction.getOutput()->closeFile() ;
  postWriteAction();
}

/*---------------------------------------------------------------------------*/
void
MeshViewer::openIvFile(SbString filename)
{
  PoMeshProperty *mesh_node = getMeshFromIvFile(filename.getString());

  if (mesh_node != NULL) {
    closeTabWindow();
    buildTabWindow(mesh_node,NULL);
    enableMenuBar(TRUE);
    updateAuditors(mesh_node->getMesh());
    startShow(v_BaseWidget);
  } 
}

/*---------------------------------------------------------------------------*/
void 
MeshViewer::buildMainWindow()
{
  v_MainWindow = new SoTopLevelDialog;
  v_MainWindow->position.setValue(300,0);
  v_MainWindow->label = "Mesh Viewer";
  
  HRSRC hRC = FindResource(NULL, "GuiMenuBar", "IV");
  if (hRC == NULL)
    throw Error();

  DWORD cbRC = SizeofResource(NULL, hRC);
  if (cbRC == 0)
  {
    FreeResource(hRC);
    throw Error();
  }

  HGLOBAL hMem = LoadResource(NULL, hRC);
  if (hMem == NULL)
  {
    FreeResource(hRC);
    throw Error();
  }

  LPVOID pData = LockResource(hMem);
  if (pData == NULL)
  {
    UnlockResource(hMem);
    FreeResource(hRC);
    throw Error();
  }

  TCHAR path[_MAX_PATH];
  GetTempPath(_MAX_PATH, path);
  GetTempFileName(path, TEXT("fld"), 0, path);
  HANDLE file = CreateFile(path, GENERIC_WRITE, FILE_SHARE_READ, 
    NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
  if (file == INVALID_HANDLE_VALUE)
  {
    UnlockResource(hMem);
    FreeResource(hRC);
    throw Error();
  }

  DWORD cbWritten;
  if (!WriteFile(file, pData, cbRC, &cbWritten, NULL) || cbRC != cbWritten)
  {
    CloseHandle(file);
    DeleteFile(path);
    UnlockResource(hMem);
    FreeResource(hRC);
    throw Error();
  }
  CloseHandle(file);

  v_MenuBar = (SoMenuBar *)SoDialogViz::loadFromFile(path);

  v_ParentViewer = new SoDialogCustom();
  v_ParentViewer->height = 400;
  v_ParentViewer->width = 600;

  v_MainWindow->addChild(v_MenuBar);
  v_MainWindow->addChild(v_ParentViewer);

  v_MainWindow->buildDialog(v_BaseWidget,FALSE);

  v_Viewer = new PoXtExaminerViewer(v_ParentViewer->getWidget());

  DeleteFile(path);
  UnlockResource(hMem);
  FreeResource(hRC);
}


/*---------------------------------------------------------------------------*/
void
MeshViewer::enableMenuBar(SbBool state)
{
  int num_child = v_MenuBar->getNumChildren();
  int i;
  for (i=1; i<num_child; i++) {
    SoMenuItem *item = (SoMenuItem *)v_MenuBar->getChild(i);
    item->enable = state;
  }
}

/*---------------------------------------------------------------------------*/
void MeshViewer::updateAuditors(const PbMesh *mesh)
{
  SoMenuRadioButtons* coloringPopup = (SoMenuRadioButtons *)v_MainWindow->searchForAuditorId("Coloring");
  coloringPopup->addAuditor(new EnumFieldAuditor(v_ColoringTypeField));
  coloringPopup->selectedItem = (int)v_ColoringTypeField->getValue();

  SoMenuCheckBox* draggerCheck = (SoMenuCheckBox *)v_MainWindow->searchForAuditorId("Dragger");
  if (v_DraggerSwitch != NULL) {
    draggerCheck->addAuditor(new CheckMenuSwitchAuditor(v_DraggerSwitch));
    draggerCheck->state = (v_DraggerSwitch->whichChild.getValue() == SO_SWITCH_ALL) ? TRUE : FALSE;
  }

  SoMenuRadioButtons* dataSetChoice = (SoMenuRadioButtons *)v_MainWindow->searchForAuditorId("DataSet used");
  dataSetChoice->addAuditor(new IntSwitchAuditor(v_DataSwitch));
  SbString dataSet = "";
  for (int i=0; i<mesh->getNumValuesSet(); i++)
  {
    dataSet = *mesh->getValuesSetName(i);
    if (dataSet.getLength() == 0)
    {
      dataSet = "Dataset ";
      dataSet += i;
    }
    dataSetChoice->items.set1Value(i, dataSet);
  }
  dataSetChoice->selectedItem = v_DataSwitch->whichChild.getValue();
}

/*---------------------------------------------------------------------------*/
void MeshViewer::addAuditors(const PbMesh *mesh) {

  SoMenuFileSelection* openButton = (SoMenuFileSelection*)v_MainWindow->searchForAuditorId("OpenFile");
  openButton->addAuditor(new OpenIvFileAuditor(this));

  SoMenuFileSelection* saveAsciiButton = (SoMenuFileSelection*)v_MainWindow->searchForAuditorId("SaveAscii");
  saveAsciiButton->addAuditor(new SaveIvFileAuditor(this,FALSE));
  saveAsciiButton->filename = v_IvAsciiFileName;

  SoMenuFileSelection* saveBinaryButton = (SoMenuFileSelection*)v_MainWindow->searchForAuditorId("SaveBinary");
  saveBinaryButton->addAuditor(new SaveIvFileAuditor(this,TRUE));
  saveBinaryButton->filename = v_IvBinaryFileName;

  SoMenuRadioButtons* orientChoice = (SoMenuRadioButtons *)v_MainWindow->searchForAuditorId("Dragger Orientation");
  orientChoice->addAuditor(new OrientAuditor(this));

  SelectTabWindowAuditor* mySelectTabAuditor = new SelectTabWindowAuditor(this);
  SoMenuPushButton* tabScalarSelection = (SoMenuPushButton*)v_MainWindow->searchForAuditorId("launchScalar");
  tabScalarSelection->addAuditor(mySelectTabAuditor);
  SoMenuPushButton* tabStreamSelection = (SoMenuPushButton*)v_MainWindow->searchForAuditorId("launchStream");
  tabStreamSelection->addAuditor(mySelectTabAuditor);
  SoMenuPushButton* tabVectorsSelection = (SoMenuPushButton*)v_MainWindow->searchForAuditorId("launchVectors");
  tabVectorsSelection->addAuditor(mySelectTabAuditor);
  SoMenuPushButton* tabProbeSelection = (SoMenuPushButton*)v_MainWindow->searchForAuditorId("launchProbe");
  tabProbeSelection->addAuditor(mySelectTabAuditor);

  SoMenuCheckBox* mergeWindowsCheck = (SoMenuCheckBox *)v_MainWindow->searchForAuditorId("MergeWindowsCheck");
  mergeWindowsCheck->addAuditor(new MergeWindowsCheckAuditor(this));

  // set the initial state of check box, choice etc.
  if (v_DraggerNormal == SbVec3f(1,0,0)) 
    orientChoice->selectedItem = 0;
  else if (v_DraggerNormal == SbVec3f(0,1,0)) 
    orientChoice->selectedItem = 1;
  else
    orientChoice->selectedItem = 2;

  if (v_DraggerSwitch != NULL)
    updateAuditors(mesh);

}/*---------------------------------------------------------------------------*/

void MeshViewer::setDraggerPos (const SbVec3f &pos) {
  // set the dragger position
  v_DraggerPos = pos;
  v_Dragger->translation.setValue(pos);
  v_Dragger->rotation = SbRotation(SbVec3f(0,1,0),v_DraggerNormal);
  v_Dragger->scaleFactor.setValue(v_ScaleFactor,v_ScaleFactor,v_ScaleFactor);

  //  updateAllConnectedToDragger(v_Dragger);
  if (v_Mesh3DScalarViewer) v_Mesh3DScalarViewer->updateAllConnectedToDragger(v_Dragger,v_DraggerNormal);
  if (v_MeshStreamViewer) v_MeshStreamViewer->updateAllConnectedToDragger(v_Dragger,v_DraggerNormal);
  if (v_MeshProbeViewer) v_MeshProbeViewer->updateAllConnectedToDragger(v_Dragger,v_DraggerNormal);
  if (v_Mesh3DVecViewer) v_Mesh3DVecViewer->updateAllConnectedToDragger(v_Dragger,v_DraggerNormal);
}/*---------------------------------------------------------------------------*/


void MeshViewer::motionCallback(void *user_data, SoJackDragger *dragger) {
  MeshViewer *viewer = (MeshViewer *)user_data;
  viewer->v_DraggerPos = dragger->translation.getValue();
  if (viewer->v_Mesh3DScalarViewer) viewer->v_Mesh3DScalarViewer->updateAllConnectedToDragger(dragger,viewer->v_DraggerNormal);
  if (viewer->v_MeshStreamViewer) viewer->v_MeshStreamViewer->updateAllConnectedToDragger(dragger,viewer->v_DraggerNormal);
  if (viewer->v_MeshProbeViewer) viewer->v_MeshProbeViewer->updateAllConnectedToDragger(dragger,viewer->v_DraggerNormal);
  if (viewer->v_Mesh3DVecViewer) viewer->v_Mesh3DVecViewer->updateAllConnectedToDragger(dragger,viewer->v_DraggerNormal);
}/*---------------------------------------------------------------------------*/

void MeshViewer::preWriteAction() {
  // disables the connection of each field of the viewers. This is done to 
  // allow the SoDB::readAll to read correctly this iv file. In fact, when a 
  // field is connected to a field of a DataViz class derived from SoBaseKit
  // (eg PoMeshSkin), SoDB::readAll do not run correctly.
  if (v_Mesh2DScalarViewer) v_Mesh2DScalarViewer->preWriteAction();
  if (v_Mesh2DVecViewer) v_Mesh2DVecViewer->preWriteAction();
  if (v_Mesh3DScalarViewer) v_Mesh3DScalarViewer->preWriteAction();
  if (v_Mesh3DVecViewer) v_Mesh3DVecViewer->preWriteAction();
  if (v_MeshStreamViewer) v_MeshStreamViewer->preWriteAction();
  if (v_MeshProbeViewer) v_MeshProbeViewer->preWriteAction();
}/*---------------------------------------------------------------------------*/

void MeshViewer::postWriteAction() {
  // re-enable the connection of the fields of the viewers. See previous comments.
  if (v_Mesh2DScalarViewer) v_Mesh2DScalarViewer->postWriteAction();
  if (v_Mesh2DVecViewer) v_Mesh2DVecViewer->postWriteAction();
  if (v_Mesh3DScalarViewer) v_Mesh3DScalarViewer->postWriteAction();
  if (v_Mesh3DVecViewer) v_Mesh3DVecViewer->postWriteAction();
  if (v_MeshStreamViewer) v_MeshStreamViewer->postWriteAction();
  if (v_MeshProbeViewer) v_MeshProbeViewer->postWriteAction();
}/*---------------------------------------------------------------------------*/
