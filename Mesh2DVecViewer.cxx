//------------------------------------------------------------------------
//                                                                        
// demonstration class to visualize a vectors field on a surface mesh 
// with 3D-Data-Master  
//                                                                        
//  author : J-Michel Godinaud                                            
//------------------------------------------------------------------------
#include "Mesh2DVecViewer.h"
#include "commonAuditor.h"

class VectorFieldLengthSlider : public SoDialogRealSliderAuditor {
  Mesh2DVecViewer *mViewer;
public:
  VectorFieldLengthSlider(Mesh2DVecViewer *mViewer) { this->mViewer = mViewer; }
  void dialogRealSlider(SoDialogRealSlider *slider) {
    mViewer->setLengthValue(slider->value.getValue());
  }  
};

class VectorFieldEndArrowTypeChoice : public SoDialogChoiceAuditor {
  Mesh2DVecViewer *mViewer;
public:
  VectorFieldEndArrowTypeChoice(Mesh2DVecViewer *mViewer) { this->mViewer = mViewer; }
  void dialogChoice (SoDialogChoice *dc) {
    mViewer->setArrowType(dc->selectedItem.getValue());
  }
};

class VectorFieldEndArrowHeightChoice : public SoDialogChoiceAuditor {
  Mesh2DVecViewer *mViewer;
public:
  VectorFieldEndArrowHeightChoice(Mesh2DVecViewer *mViewer) { this->mViewer = mViewer; }
  void dialogChoice (SoDialogChoice *dc) {
    mViewer->setArrowHeightType(dc->selectedItem.getValue());
  }
};

//------------------------------------------------------------------------
Mesh2DVecViewer::Mesh2DVecViewer()
{
}/*---------------------------------------------------------------------------*/

void Mesh2DVecViewer::buildSceneGraph(const PbMesh *pb_mesh, PoDomain *domain, 
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
  v_MeshVecField = new PoMesh2DVec;
  v_MeshVecField->vecsIndex.setValue(0);
  v_MeshVecField->bodyLengthFactor = v_VecLengthFactor;
  v_MeshVecField->bodyLengthType = PoMesh2DVec::CONSTANT_LENGTH;
  v_MeshVecField->endArrowHeightFactor = 0.25;
  v_MeshVecField->bodyColoringType = PoMesh2DVec::MODULE_MAPPING_COLOR;

  v_MeshVecFieldSwitch = new SoSwitch;
  v_MeshVecFieldSwitch->addChild(v_MeshVecField) ;
  v_MeshVecFieldSwitch->whichChild = SO_SWITCH_NONE;

  SoPickStyle *pick_style = new SoPickStyle;
  pick_style->style = SoPickStyle::UNPICKABLE;

  scene_mesh3D->addChild(pick_style);
  scene_mesh3D->addChild(module_data_mapping);
  scene_mesh3D->addChild(v_MeshVecFieldSwitch);

}/*---------------------------------------------------------------------------*/

void Mesh2DVecViewer::setLengthValue(float val)
{
  v_MeshVecField->bodyLengthFactor = v_VecLengthFactor = val;
}/*---------------------------------------------------------------------------*/

void Mesh2DVecViewer::setArrowType(int val)
{
  switch (val) {
    case 0 : v_MeshVecField->endArrowShape = PoMesh2DVec::NO_SHAPE; break;
    case 1 : v_MeshVecField->endArrowShape = PoMesh2DVec::CHEVRON; break;
    case 2 : v_MeshVecField->endArrowShape = PoMesh2DVec::TRIANGLE; break;
  }
}/*---------------------------------------------------------------------------*/

void Mesh2DVecViewer::setArrowHeightType(int val)
{
  switch (val) {
    case 0 : v_MeshVecField->endArrowHeightType = PoMesh2DVec::CONSTANT_HEIGHT; break;
    case 1 : v_MeshVecField->endArrowHeightType = PoMesh2DVec::RELATIVE_HEIGHT; break;
  }
}/*---------------------------------------------------------------------------*/

SoDialogComponent *
Mesh2DVecViewer::buildDialogBox (const PbMesh *pb_mesh)
{
  HRSRC hRC = FindResource(NULL, "GuiTabVector2D", "IV");
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

  SoDialogGroup* v_DialogBox = (SoDialogGroup *)myGroup->getChild(0);

  //////////// 
  // Vector Field Enable check
  SoDialogCheckBox* vectorFieldEnable = (SoDialogCheckBox *)v_DialogBox->searchForAuditorId("VectorFieldEnable");
  vectorFieldEnable->addAuditor(new CheckSwitchAuditor(v_MeshVecFieldSwitch));
  vectorFieldEnable->state = (v_MeshVecFieldSwitch->whichChild.getValue() == SO_SWITCH_ALL);

  //////////// 
  // Vector Field Length Slider
  SoDialogRealSlider* vectorFieldLengthSlider = (SoDialogRealSlider *)v_DialogBox->searchForAuditorId("VectorFieldLength");
  vectorFieldLengthSlider->addAuditor(new VectorFieldLengthSlider(this));
  vectorFieldLengthSlider->value = v_MeshVecField->bodyLengthFactor.getValue(); 

  //////////// 
  // Vector Field End Arrow Type choice
  SoDialogComboBox* vectorFieldEndArrowTypeChoice = (SoDialogComboBox *)v_DialogBox->searchForAuditorId("VectorFieldEndArrowType");
  vectorFieldEndArrowTypeChoice->addAuditor(new VectorFieldEndArrowTypeChoice(this));
  vectorFieldEndArrowTypeChoice->selectedItem = ((int)v_MeshVecField->endArrowShape.getValue() == 0) ? 0 : (int)v_MeshVecField->endArrowShape.getValue() - 1; 

  //////////// 
  // Vector Field End Arrow Height choice
  SoDialogComboBox* vectorFieldEndArrowHeightChoice = (SoDialogComboBox *)v_DialogBox->searchForAuditorId("VectorFieldEndArrowHeight");
  vectorFieldEndArrowHeightChoice->addAuditor(new VectorFieldEndArrowHeightChoice(this));
  vectorFieldEndArrowHeightChoice->selectedItem = (int)v_MeshVecField->endArrowHeightType.getValue(); 
  
  //////////// 
  // Vector Field End Arrow Ht Slider
  SoDialogRealSlider* vectorFieldEndArrowHtSlider = (SoDialogRealSlider *)v_DialogBox->searchForAuditorId("VectorFieldEndArrowHt");
  vectorFieldEndArrowHtSlider->addAuditor(new RealSliderAuditor(&v_MeshVecField->endArrowHeightFactor));
  vectorFieldEndArrowHtSlider->value = v_MeshVecField->endArrowHeightFactor.getValue(); 

  //////////// 
  // vector Field End Arrow Rd Slider
  SoDialogRealSlider* vectorFieldEndArrowRdSlider = (SoDialogRealSlider *)v_DialogBox->searchForAuditorId("VectorFieldEndArrowRd");
  vectorFieldEndArrowRdSlider->addAuditor(new RealSliderAuditor(&v_MeshVecField->endArrowRadiusFactor));
  vectorFieldEndArrowRdSlider->value = v_MeshVecField->endArrowRadiusFactor.getValue(); 

  return v_DialogBox;
}/*---------------------------------------------------------------------------*/

void Mesh2DVecViewer::preWriteAction() {
  enableConnection(v_MeshVecField,FALSE);
}/*---------------------------------------------------------------------------*/

void Mesh2DVecViewer::postWriteAction() {
  enableConnection(v_MeshVecField,TRUE);
}/*---------------------------------------------------------------------------*/

void Mesh2DVecViewer::enableConnection(SoNode *node, SbBool flag) {
  SoFieldList fields;
  int num_fields = node->getFields(fields);
  for (int i=0; i<num_fields; i++) {
    SoField *field = fields[i];
    if (field->isConnected()) field->enableConnection(flag);
  }
}/*---------------------------------------------------------------------------*/
