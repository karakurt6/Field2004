//------------------------------------------------------------------------
//                                                                        
// demonstration classes to visualize 3D volume mesh with 3D-Data-Master 
//                                                                        
//  author : J-Michel Godinaud                                            
//------------------------------------------------------------------------
#include "Mesh3DScalarViewer.h"
#include "commonAuditor.h"

void set_transparenciesCB(void *user_data, PoBase *mesh_skin) {
  float *transparency = (float *)user_data;
  if(*transparency != 0.0) {
    SoMaterial *material = SO_GET_PART(mesh_skin,"appearance.material",SoMaterial);
    for (int i=0; i<material->diffuseColor.getNum(); i++)
      material->transparency.set1Value(i,*transparency);
  }
}/*---------------------------------------------------------------------------*/

class CheckFieldConnectAuditor : public SoDialogCheckBoxAuditor {
  SoSFEnum *field;
public:
  CheckFieldConnectAuditor(SoSFEnum *field) { this->field = field; }
  void dialogCheckBox(SoDialogCheckBox *checkBox) {
    field->enableConnection(checkBox->state.getValue());
    if (checkBox->state.getValue() == FALSE) *field = PoMesh::COLOR_INHERITED;
    field->touch();
  }
};

class CheckLinkAuditor : public SoDialogCheckBoxAuditor {
  MeshViewerComponent *mvc;
public:
  CheckLinkAuditor(MeshViewerComponent *mvc) { this->mvc = mvc; }
  void dialogCheckBox(SoDialogCheckBox *checkBox) {
    mvc->connectDragger(checkBox->state.getValue());
  }
};

class TranspSliderAuditor : public SoDialogRealSliderAuditor {
  Mesh3DScalarViewer *mViewer;
public:
  TranspSliderAuditor(Mesh3DScalarViewer *mViewer) { this->mViewer = mViewer; }
  void dialogRealSlider(SoDialogRealSlider *slider) {
    mViewer->setTransparencyValue(slider->value.getValue());
  }  
};

class TranspTypeAuditor : public SoDialogChoiceAuditor {
  Mesh3DScalarViewer *mViewer;
public:
  TranspTypeAuditor(Mesh3DScalarViewer *mViewer) { this->mViewer = mViewer; }
  void dialogChoice (SoDialogChoice *dc) {
    mViewer->setTransparencyType((SoGLRenderAction::TransparencyType)dc->selectedItem.getValue());
  }
};

class DataIsovalChoice  : public SoDialogChoiceAuditor {
  Mesh3DScalarViewer *mViewer;
public:
  DataIsovalChoice(Mesh3DScalarViewer *mViewer) { this->mViewer = mViewer; }
  void dialogChoice (SoDialogChoice *dc) {
    mViewer->setDatasetForLevelSurf(dc->selectedItem.getValue());
  }
};

/*---------------------------------------------------------------------------*/

Mesh3DScalarViewer::Mesh3DScalarViewer(SoTopLevelDialog *dialogWindow, SoXtExaminerViewer *viewer) :
  v_TransparencyValue(0),
  v_TransparencyType(SoGLRenderAction::SORTED_LAYERS_BLEND),
  v_DataSetIndexOfLevelSurface(0)
{
  v_Viewer = viewer;
  v_DialogWindow = dialogWindow;
  setTransparencyType(v_TransparencyType);
}/*---------------------------------------------------------------------------*/

void Mesh3DScalarViewer::buildSceneGraph(const PbMesh *pb_mesh, PoDomain *domain, 
				   SoGroup *scene_mesh3D,
				   SoSFInt32 *which_dataset,
				   SoSFEnum* &which_coloring_type)
{
  v_DataSetNum = pb_mesh->getNumValuesSet();
  v_Vmin = new float [v_DataSetNum];
  v_Vmax = new float [v_DataSetNum];
  for (int i=0; i<v_DataSetNum; i++) {
    pb_mesh->getMinValuesSet(i,v_Vmin[i]);
    pb_mesh->getMaxValuesSet(i,v_Vmax[i]);
  }

  v_DataSetIndex = which_dataset->getValue();
  v_Domain = domain;

  // define a mesh cross section
  v_MeshCrossSection = new PoMeshCrossSection;
  v_MeshCrossSection->valuesIndex.connectFrom(which_dataset);
  v_MeshCrossSection->coloringType = PoMesh::COLOR_MAPPING;
  which_coloring_type = &(v_MeshCrossSection->coloringType);
//  v_MeshCrossSection->setCrossSectionMethod(PoMeshCrossSection::ISOSURFACE);
//  v_MeshCrossSection->setCrossSectionMethod(PoMeshCrossSection::INTERSECTION);

  // define the mesh skin
  v_MeshSkin = new PoMeshSkin;
  v_MeshSkin->valuesIndex.connectFrom(which_dataset);
  v_MeshSkin->coloringType.connectFrom(which_coloring_type);
  v_MeshSkin->addPostRebuildCallback((PoBase::PoRebuildCB *)set_transparenciesCB,&v_TransparencyValue);

  // define the mesh skeleton
  v_MeshSkeleton = new PoMeshSkeleton;
  v_MeshSkeleton->numXContour = 2;
  v_MeshSkeleton->numYContour = 2;
  v_MeshSkeleton->numZContour = 2;
  v_MeshSkeleton->set("appearance.material","diffuseColor 0.5 0.25 0.25");

  // define the mesh isosurface
  v_MeshLevelSurf = new PoMeshLevelSurf;
  v_MeshLevelSurf->valuesIndex.connectFrom(which_dataset);
  v_MeshLevelSurf->coloringType.connectFrom(which_coloring_type);
  v_MeshLevelSurf->valuesIndexForLevel.setValue(v_DataSetIndexOfLevelSurface);
  v_MeshLevelSurf->levelValue.setValue((v_Vmin[v_DataSetIndexOfLevelSurface]+v_Vmax[v_DataSetIndexOfLevelSurface])/2);

  // define a mesh cross contour
  v_MeshCrossContour = new PoMeshCrossContour;
  v_MeshCrossContour->valuesIndex.connectFrom(which_dataset);
  v_MeshCrossContour->coloringType.connectFrom(which_coloring_type);
  v_MeshCrossContour->set("appearance.drawStyle","lineWidth 2");

  v_ClipPlane = new SoClipPlane;
  v_ClipPlane->on.setValue(FALSE);

  v_MeshSkinSwitch = new SoSwitch;
  v_MeshSkinSwitch->addChild(v_MeshSkin) ;
  v_MeshSkinSwitch->whichChild = SO_SWITCH_ALL;

  v_MeshCrossSectionSwitch = new SoSwitch;
  v_MeshCrossSectionSwitch->addChild(v_MeshCrossSection) ;
  v_MeshCrossSectionSwitch->whichChild = SO_SWITCH_ALL;

  v_MeshSkeletonSwitch = new SoSwitch;
  v_MeshSkeletonSwitch->addChild(v_MeshSkeleton) ;
  v_MeshSkeletonSwitch->whichChild = SO_SWITCH_NONE;

  v_MeshLevelSurfSwitch = new SoSwitch;
  v_MeshLevelSurfSwitch->addChild(v_MeshLevelSurf) ;
  v_MeshLevelSurfSwitch->whichChild = SO_SWITCH_ALL;

  v_MeshCrossContourSwitch = new SoSwitch;
  v_MeshCrossContourSwitch->addChild(v_MeshCrossContour) ;
  v_MeshCrossContourSwitch->whichChild = SO_SWITCH_NONE;

  SoPickStyle *pick_style = new SoPickStyle;
  pick_style->style = SoPickStyle::UNPICKABLE;

  scene_mesh3D->addChild(pick_style);
  scene_mesh3D->addChild(v_MeshCrossSectionSwitch);
  scene_mesh3D->addChild(v_MeshCrossContourSwitch);
  scene_mesh3D->addChild(v_ClipPlane);
  scene_mesh3D->addChild(v_MeshSkinSwitch);
  scene_mesh3D->addChild(v_MeshSkeletonSwitch);
  scene_mesh3D->addChild(v_MeshLevelSurfSwitch);

}/*---------------------------------------------------------------------------*/

void Mesh3DScalarViewer::setDatasetForLevelSurf(int datasetid) {
  v_DataSetIndexOfLevelSurface = datasetid;
  v_MeshLevelSurf->valuesIndexForLevel = v_DataSetIndexOfLevelSurface;
  int data_ind = (v_DataSetIndexOfLevelSurface == -1) ? v_DataSetIndex : v_DataSetIndexOfLevelSurface;
  v_MeshLevelSurf->levelValue.setValue((v_Vmin[data_ind]+v_Vmax[data_ind])/2);

  SoDialogRealSlider* isoValSlider = (SoDialogRealSlider *)v_DialogBox->searchForAuditorId("LevelSlider");
  isoValSlider->min.setValue(v_Vmin[data_ind]);
  isoValSlider->max.setValue(v_Vmax[data_ind]);
  isoValSlider->value.setValue((v_Vmin[data_ind]+v_Vmax[data_ind])/2);
}/*---------------------------------------------------------------------------*/

void Mesh3DScalarViewer::setTransparencyValue(float transparencyValue) {
  v_TransparencyValue = transparencyValue;
  SoMaterial *material = SO_GET_PART(v_MeshSkin,"appearance.material",SoMaterial);
  if(transparencyValue == 0.0) {
    material->transparency.setNum(1);
    material->transparency.set1Value(0, 0.0);
  } else {
    for (int i=0; i<material->diffuseColor.getNum(); i++)
      material->transparency.set1Value(i,transparencyValue);
  }
}/*---------------------------------------------------------------------------*/

void Mesh3DScalarViewer::setTransparencyType(SoGLRenderAction::TransparencyType type) {
  v_TransparencyType = type;
  v_Viewer->setTransparencyType(type);
}/*---------------------------------------------------------------------------*/

SoDialogComponent *
Mesh3DScalarViewer::buildDialogBox (const PbMesh *pb_mesh, const char **dataset_name)
{
  HRSRC hRC = FindResource(NULL, "GuiTabScalar3D", "IV");
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

  SoInput myInput;
  myInput.setBuffer(pData, cbRC);
  SoGroup *myGroup = SoDB::readAll( &myInput );
  if (myGroup == NULL)
  {
    UnlockResource(hMem);
    FreeResource(hRC);
    throw Error();
  }
  UnlockResource(hMem);
  FreeResource(hRC);

  v_DialogBox = (SoDialogGroup *)myGroup->getChild(0);

  //////////// 
  // skin check
  SoDialogCheckBox* skinCheck = (SoDialogCheckBox *)v_DialogBox->searchForAuditorId("SkinCheck");
  skinCheck->addAuditor(new CheckSwitchAuditor(v_MeshSkinSwitch));
  skinCheck->state = (v_MeshSkinSwitch->whichChild.getValue() == SO_SWITCH_ALL);
  
  // skin check
  SoDialogCheckBox* skinColorCheck = (SoDialogCheckBox *)v_DialogBox->searchForAuditorId("SkinColorCheck");
  skinColorCheck->addAuditor(new CheckFieldConnectAuditor(&v_MeshSkin->coloringType));
  skinColorCheck->state = v_MeshSkin->coloringType.isConnectionEnabled();
  
  // skin level of transparency
  SoDialogRealSlider* transpSlider = (SoDialogRealSlider *)v_DialogBox->searchForAuditorId("SkinTransparency");
  transpSlider->addAuditor(new TranspSliderAuditor(this));
  transpSlider->value = v_TransparencyValue; 

  SoDialogComboBox* transpType = (SoDialogComboBox *)v_DialogBox->searchForAuditorId("TypeTransparency");
  transpType->addAuditor(new TranspTypeAuditor(this));
  transpType->selectedItem = v_TransparencyType;

  //////////// 
  // connect cross section check
  SoDialogCheckBox* crossSectionCheck = (SoDialogCheckBox *)v_DialogBox->searchForAuditorId("CrossSectionCheck");
  crossSectionCheck->addAuditor(new CheckSwitchAuditor(v_MeshCrossSectionSwitch));
  crossSectionCheck->state = (v_MeshCrossSectionSwitch->whichChild.getValue() == SO_SWITCH_ALL);
  
  // connect cross contour check
  SoDialogCheckBox* crossContourCheck = (SoDialogCheckBox *)v_DialogBox->searchForAuditorId("CrossContourCheck");
  crossContourCheck->addAuditor(new CheckSwitchAuditor(v_MeshCrossContourSwitch));
  crossContourCheck->state = (v_MeshCrossContourSwitch->whichChild.getValue() == SO_SWITCH_ALL);
  
  // connect dragger to crossection
  SoDialogCheckBox* crossSectionLink = (SoDialogCheckBox *)v_DialogBox->searchForAuditorId("CrossSectionLink");
  crossSectionLink->addAuditor(new CheckLinkAuditor(this));
  crossSectionLink->state = TRUE;
  
  // connect dragger to crossection
  SoDialogCheckBox* clippingCheck = (SoDialogCheckBox *)v_DialogBox->searchForAuditorId("ClippingCheck");
  v_ClipPlane->on.connectFrom(&clippingCheck->state);
  
  //////////// 
  // connect level surf check
  SoDialogCheckBox* levelSurfCheck = (SoDialogCheckBox *)v_DialogBox->searchForAuditorId("LevelSurfCheck");
  levelSurfCheck->addAuditor(new CheckSwitchAuditor(v_MeshLevelSurfSwitch));
  levelSurfCheck->state = (v_MeshLevelSurfSwitch->whichChild.getValue() == SO_SWITCH_ALL);

  // level surf value
  SoDialogRealSlider* isovalSlider = (SoDialogRealSlider *)v_DialogBox->searchForAuditorId("LevelSlider");
  int ind = (v_DataSetIndexOfLevelSurface != -1) ? v_DataSetIndexOfLevelSurface : v_DataSetIndex;
  isovalSlider->min = v_Vmin[ind];
  isovalSlider->max = v_Vmax[ind];
  isovalSlider->value = v_MeshLevelSurf->levelValue.getValue();
  isovalSlider->addAuditor(new RealSliderAuditor(&v_MeshLevelSurf->levelValue));

  // level surf dataset
  SoDialogComboBox* dataIsovalChoice = (SoDialogComboBox *)v_DialogBox->searchForAuditorId("LevelIsovalChoice");
  for (int i=0; i<pb_mesh->getNumValuesSet(); i++)
    dataIsovalChoice->items.set1Value(i,*pb_mesh->getValuesSetName(i));
  dataIsovalChoice->addAuditor(new DataIsovalChoice(this));

  //////////// 
  // connect skeleton check
  SoDialogCheckBox* skeletonCheck = (SoDialogCheckBox *)v_DialogBox->searchForAuditorId("SkeletonCheck");
  skeletonCheck->addAuditor(new CheckSwitchAuditor(v_MeshSkeletonSwitch));
  skeletonCheck->state = (v_MeshSkeletonSwitch->whichChild.getValue() == SO_SWITCH_ALL);
  
  return v_DialogBox;
}/*---------------------------------------------------------------------------*/


void Mesh3DScalarViewer::updateAllConnectedToDragger(SoJackDragger *dragger, SbVec3f plane_normal){
  if (!v_IsDraggerConnected) return;

  // get the dragger position
  SbVec3f drager_position = dragger->translation.getValue();

  // get the dragger scale factor
  SbVec3f drager_scale = dragger->scaleFactor.getValue();

  // rotate the plane's normal by the dragger rotation
  SbRotation rotation = dragger->rotation.getValue();
  rotation.multVec(SbVec3f(0,1,0),plane_normal);

  // translate cross section and cross contour
  v_MeshCrossSection->plane.setValue(SbPlane(plane_normal,drager_position));
  v_MeshCrossContour->plane.setValue(SbPlane(plane_normal,drager_position));

  // translate clip plane
  SbVec3f clip_norm = plane_normal; clip_norm.negate();
  v_ClipPlane->plane.setValue(SbPlane(clip_norm,drager_position));

}/*---------------------------------------------------------------------------*/


void Mesh3DScalarViewer::preWriteAction() {
  enableConnection(v_MeshSkin,FALSE);
  enableConnection(v_MeshSkeleton,FALSE);
  enableConnection(v_MeshLevelSurf,FALSE);
  enableConnection(v_MeshCrossSection,FALSE);
  enableConnection(v_MeshCrossContour,FALSE);
}/*---------------------------------------------------------------------------*/

void Mesh3DScalarViewer::postWriteAction() {
  enableConnection(v_MeshSkin,TRUE);
  enableConnection(v_MeshSkeleton,TRUE);
  enableConnection(v_MeshLevelSurf,TRUE);
  enableConnection(v_MeshCrossSection,TRUE);
  enableConnection(v_MeshCrossContour,TRUE);
}/*---------------------------------------------------------------------------*/

void Mesh3DScalarViewer::enableConnection(SoNode *node, SbBool flag) {
  SoFieldList fields;
  int num_fields = node->getFields(fields);
  for (int i=0; i<num_fields; i++) {
    SoField *field = fields[i];
    if (field->isConnected()) field->enableConnection(flag);
  }
}/*---------------------------------------------------------------------------*/
