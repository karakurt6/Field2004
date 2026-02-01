//------------------------------------------------------------------------
//                                                                        
// demonstration classes to visualize 3D volumic mesh with 3D-Data-Master 
//                                                                        
//  author : J-Michel Godinaud                                            
//------------------------------------------------------------------------
#include "Mesh3DVecViewer.h"
#include "commonAuditor.h"

class LengthSliderAuditor : public SoDialogRealSliderAuditor {
  Mesh3DVecViewer *mViewer;
public:
  LengthSliderAuditor(Mesh3DVecViewer *mViewer) { this->mViewer = mViewer; }
  void dialogRealSlider(SoDialogRealSlider *slider) {
    mViewer->setLengthValue(slider->value.getValue());
  }  
};

class ProjectionAuditor : public SoDialogChoiceAuditor {
  Mesh3DVecViewer *mViewer;
public:
  ProjectionAuditor(Mesh3DVecViewer *mViewer) { this->mViewer = mViewer; }
  void dialogChoice (SoDialogChoice *dc) {
    mViewer->setProjectionType((PoMesh3DVecGridCrossSection::ProjectionType)dc->selectedItem.getValue());
  }
};

//------------------------------------------------------------------------
Mesh3DVecViewer::Mesh3DVecViewer()
{
}/*---------------------------------------------------------------------------*/

void Mesh3DVecViewer::buildSceneGraph(const PbMesh *pb_mesh, PoDomain *domain, 
				      SoGroup *scene_mesh3D,
				      SoSFInt32 *which_dataset,
				      SoSFEnum *which_coloring_type,
				      PoDataMapping *module_data_mapping)
{
  if (pb_mesh->getNumVecsSet() < 1) return;

  SbBox3f mesh_bounding_box;

  // get the mesh bounding box
  if (domain) {
    mesh_bounding_box.setBounds(domain->min.getValue(),domain->max.getValue());
  } else 
    mesh_bounding_box = pb_mesh->getBoundingBox();

  float sx,sy,sz; mesh_bounding_box.getSize(sx,sy,sz);
  float maxs = (sx > sy) ? sx : sy; if (sz > maxs) maxs = sz;

  float max_vec; pb_mesh->getMaxVecsSet(0, max_vec);

  v_VecLengthFactor = 0.05*maxs;

  // define the vector field
  v_MeshVecField = new PoMesh3DVecGridCrossSection;
  v_MeshVecField->vecsIndex.setValue(0);
  v_MeshVecField->bodyLengthFactor = v_VecLengthFactor;
  v_MeshVecField->bodyLengthType = PoMesh3DVec::CONSTANT_LENGTH;
  v_MeshVecField->endArrowHeightFactor = 0.25;
  v_MeshVecField->bodyColoringType = PoMesh3DVec::MODULE_MAPPING_COLOR;

  v_MeshVecFieldSwitch = new SoSwitch;
  v_MeshVecFieldSwitch->addChild(v_MeshVecField) ;
  v_MeshVecFieldSwitch->whichChild = SO_SWITCH_NONE;

  SoPickStyle *pick_style = new SoPickStyle;
  pick_style->style = SoPickStyle::UNPICKABLE;

  scene_mesh3D->addChild(pick_style);
  scene_mesh3D->addChild(module_data_mapping);
  scene_mesh3D->addChild(v_MeshVecFieldSwitch);

}/*---------------------------------------------------------------------------*/

void Mesh3DVecViewer::setLengthValue(float val)
{
  v_MeshVecField->bodyLengthFactor.setValue(v_VecLengthFactor = val);
}/*---------------------------------------------------------------------------*/

void Mesh3DVecViewer::setProjectionType(PoMesh3DVecGridCrossSection::ProjectionType type)
{
  v_MeshVecField->projectionType.setValue(type);
}/*---------------------------------------------------------------------------*/

SoDialogComponent *
Mesh3DVecViewer::buildDialogBox (const PbMesh *pb_mesh)
{
  HRSRC hRC = FindResource(NULL, "GuiTabVector3D", "IV");
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
  // vector check enable
  SoDialogCheckBox* vectorCheckEnable = (SoDialogCheckBox *)v_DialogBox->searchForAuditorId("VectorEnable");
  vectorCheckEnable->addAuditor(new CheckSwitchAuditor(v_MeshVecFieldSwitch));
  vectorCheckEnable->state = (v_MeshVecFieldSwitch->whichChild.getValue() == SO_SWITCH_ALL);

  // vector grid spacing
  SoDialogRealSlider* vectorGridSpacing = (SoDialogRealSlider *)v_DialogBox->searchForAuditorId("VectorGridSpacing");
  vectorGridSpacing->addAuditor(new RealSliderAuditor(&v_MeshVecField->gridSpacing));
  vectorGridSpacing->value = v_MeshVecField->gridSpacing.getValue(); 

  // vector length
  SoDialogRealSlider* vectorLength = (SoDialogRealSlider *)v_DialogBox->searchForAuditorId("VectorLength");
  vectorLength->addAuditor(new LengthSliderAuditor(this));
  vectorLength->value = v_MeshVecField->bodyLengthFactor.getValue(); 

  // vector projection
  SoDialogComboBox* vectorProjection = (SoDialogComboBox *)v_DialogBox->searchForAuditorId("VectorProjection");
  vectorProjection->addAuditor(new ProjectionAuditor(this));
  vectorProjection->selectedItem = (int)v_MeshVecField->projectionType.getValue(); 

  return v_DialogBox;
}/*---------------------------------------------------------------------------*/

void Mesh3DVecViewer::updateAllConnectedToDragger(SoJackDragger *dragger, SbVec3f plane_normal){
  if (!v_IsDraggerConnected) return;

  // get the dragger position
  SbVec3f drager_position = dragger->translation.getValue();

  // get the dragger scale factor
  SbVec3f drager_scale = dragger->scaleFactor.getValue();

  // rotate the plane's normal by the dragger rotation
  SbRotation rotation = dragger->rotation.getValue();
  rotation.multVec(SbVec3f(0,1,0),plane_normal);

  // translate vec-field
  v_MeshVecField->plane.setValue(SbPlane(plane_normal,drager_position));

}/*---------------------------------------------------------------------------*/

void Mesh3DVecViewer::preWriteAction() {
  enableConnection(v_MeshVecField,FALSE);
}/*---------------------------------------------------------------------------*/

void Mesh3DVecViewer::postWriteAction() {
  enableConnection(v_MeshVecField,TRUE);
}/*---------------------------------------------------------------------------*/

void Mesh3DVecViewer::enableConnection(SoNode *node, SbBool flag) {
  SoFieldList fields;
  int num_fields = node->getFields(fields);
  for (int i=0; i<num_fields; i++) {
    SoField *field = fields[i];
    if (field->isConnected()) field->enableConnection(flag);
  }
}/*---------------------------------------------------------------------------*/
