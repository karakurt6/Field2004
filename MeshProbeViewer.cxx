//****************************************************************************
//                                                                         
// demonstration class to get probe informations with 3D-Data-Master  
//                                                                         
//  author : J-Michel Godinaud                                             
//****************************************************************************
#include "MeshProbeViewer.h"
#include "commonAuditor.h"

SoText2                 *l_CellIndexText;
SoText2                 *l_ProbeValueText;
SoSwitch                *l_textSwitch;
SoSeparator             *l_sceneProbe ;
SoGroup                 *l_probeTextInfoGroup;

//------------------------------------------------------------------------
MeshProbeViewer::MeshProbeViewer()
{
}/*---------------------------------------------------------------------------*/

void MeshProbeViewer::buildSceneGraph(PoMeshProperty *mesh_node, PoDomain *domain, 
				      SoGroup *scene_probe,
				      SoSFInt32 *which_dataset,
				      SoSFEnum *which_coloring_type,
				      SoJackDragger *dragger,
				      SoGroup *probe_text_info_group
				      )
{
  l_sceneProbe = (SoSeparator*)scene_probe ;
  l_probeTextInfoGroup = probe_text_info_group ; 

  // define a cell to draw, it is defined by the position of the probe
  v_CellFacets = new PoCellFacets;
  v_CellFacets->valuesIndex.connectFrom(which_dataset);
  v_CellFacets->coloringType.connectFrom(which_coloring_type);

  v_CellIndices = new PoCellIndices;
  v_CellIndices->cellIndex.connectFrom(&v_CellFacets->cellIndex);

  SoGroup *group_cell_indices = new SoGroup;
  group_cell_indices->addChild(mesh_node);
  group_cell_indices->addChild(v_CellIndices);

  SoLevelOfDetail *cell_indices_detail = new SoLevelOfDetail;
  cell_indices_detail->screenArea.setValue(2000.);
  SoGroup *empty_group = new SoGroup;
  cell_indices_detail->addChild(group_cell_indices);
  cell_indices_detail->addChild(empty_group);
   
  v_CellEdges = new PoCellEdges;
  v_CellEdges->cellIndex.connectFrom(&v_CellFacets->cellIndex);
   
  // define a point-probe to visualize a cell
  v_MeshProbePoint = new PoMeshProbePoint;
  v_MeshProbePoint->position.connectFrom(&dragger->translation);
  v_MeshProbePoint->position.enableConnection(v_IsDraggerConnected);
  v_MeshProbePoint->valuesIndex.connectFrom(which_dataset);
  v_MeshProbePoint->setChangeCellCallback((PoMeshProbePoint::PoProbeCB *)change_cell_probeCB,v_CellFacets);
  v_MeshProbePoint->setMotionCallback((PoMeshProbePoint::PoProbeCB *)motion_probeCB,NULL);
  v_MeshProbePoint->setEnterMeshCallback((PoMeshProbePoint::PoProbeCB *)enter_probeCB,NULL);
  v_MeshProbePoint->setLeaveMeshCallback((PoMeshProbePoint::PoProbeCB *)leave_probeCB,NULL);

  v_CellFacetsSwitch = new SoSwitch;
  v_CellFacetsSwitch->addChild(v_CellFacets) ;
  v_CellFacetsSwitch->whichChild = SO_SWITCH_NONE;
  
  v_CellEdgesSwitch = new SoSwitch;
  v_CellEdgesSwitch->addChild(v_CellEdges) ;
  v_CellEdgesSwitch->whichChild = SO_SWITCH_NONE;
  
  v_CellIndicesSwitch = new SoSwitch;
  v_CellIndicesSwitch->addChild(cell_indices_detail) ;
  v_CellIndicesSwitch->whichChild = SO_SWITCH_NONE;

  SoPickStyle *pick_style = new SoPickStyle;
  pick_style->style = SoPickStyle::UNPICKABLE;

  scene_probe->addChild(pick_style);
  scene_probe->addChild(v_MeshProbePoint);
  scene_probe->addChild(v_CellFacetsSwitch);
  scene_probe->addChild(v_CellEdgesSwitch);
  scene_probe->addChild(v_CellIndicesSwitch);

  // build probe information text
  l_CellIndexText = new SoText2;
  l_CellIndexText->justification = SoText2::LEFT;

  SoTranslation *trans2 = new SoTranslation;
  trans2->translation.setValue(0.F, -0.05F, 0);
  l_ProbeValueText = new SoText2;
  l_ProbeValueText->justification = SoText2::LEFT;
  
  SoGroup *group_probe_text = new SoGroup;
  group_probe_text->addChild(l_CellIndexText);
  group_probe_text->addChild(trans2);
  group_probe_text->addChild(l_ProbeValueText);
  
  SoText2 *text_probe_out = new SoText2;
  text_probe_out->string = "probe out of mesh !";

  SoBlinker *text_blinker = new SoBlinker;
  text_blinker->addChild(text_probe_out);
  text_blinker->whichChild = SO_SWITCH_NONE;

  l_textSwitch = new SoSwitch;
  l_textSwitch->addChild(group_probe_text);
  l_textSwitch->addChild(text_blinker);
  l_textSwitch->whichChild = 0;

  SoTranslation *trans1 = new SoTranslation;
  trans1->translation.setValue(-1.9F, 1.90F, 0);
  probe_text_info_group->addChild(trans1);
  probe_text_info_group->addChild(l_textSwitch);
}/*---------------------------------------------------------------------------*/

enum event_name {
  TOGGLE_DRAGGER_CONNECT,
  TOGGLE_CELL_FACETS,
  TOGGLE_CELL_EDGES,
  TOGGLE_CELL_INDICES,
  SLIDER_CELL_IND_OFFSET,
  NUM_EVENT
};

SoDialogComponent *
MeshProbeViewer::buildDialogBox (const PbMesh *pb_mesh)
{
  HRSRC hRC = FindResource(NULL, "GuiTabProbe", "IV");
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
  // cell facets check
  SoDialogCheckBox* cellFacetsCheck = (SoDialogCheckBox *)v_DialogBox->searchForAuditorId("CellFacets");
  cellFacetsCheck->addAuditor(new CheckSwitchAuditor(v_CellFacetsSwitch));
  cellFacetsCheck->state = (v_CellFacetsSwitch->whichChild.getValue() == SO_SWITCH_ALL);

  //////////// 
  // cell edge check
  SoDialogCheckBox* cellEdgeCheck = (SoDialogCheckBox *)v_DialogBox->searchForAuditorId("CellEdge");
  cellEdgeCheck->addAuditor(new CheckSwitchAuditor(v_CellEdgesSwitch));
  cellEdgeCheck->state = (v_CellEdgesSwitch->whichChild.getValue() == SO_SWITCH_ALL);

  //////////// 
  // cell indices check
  SoDialogCheckBox* cellIndicesCheck = (SoDialogCheckBox *)v_DialogBox->searchForAuditorId("CellIndices");
  cellIndicesCheck->addAuditor(new CheckSwitchAuditor(v_CellIndicesSwitch));
  cellIndicesCheck->state = (v_CellIndicesSwitch->whichChild.getValue() == SO_SWITCH_ALL);

  //////////// 
  // cell indices offset
  SoDialogRealSlider* cellIndicesOffset = (SoDialogRealSlider *)v_DialogBox->searchForAuditorId("CellIndicesOffset");
  cellIndicesOffset->addAuditor(new RealSliderAuditor(&v_CellIndices->offset));
  cellIndicesOffset->value = v_CellIndices->offset.getValue();

  return v_DialogBox;
}/*---------------------------------------------------------------------------*/

void MeshProbeViewer::updateAllConnectedToDragger(SoJackDragger *dragger, SbVec3f plane_normal){
  // the connection is realized by the Inventor's field connexion
}/*---------------------------------------------------------------------------*/

void change_cell_probeCB(void *userData, PoMeshProbePoint *probe, const PbCell *cell) {
  PoCellFacets *v_CellFacets = (PoCellFacets *)userData;

  l_sceneProbe->enableNotify(FALSE) ;
  if (!cell)
    v_CellFacets->cellIndex = -1;
  else {	  
    v_CellFacets->cellIndex = cell->getIndex();
	  
    char str[80];
    sprintf(str,"cell index : %d",cell->getIndex());
	  
    l_probeTextInfoGroup->enableNotify(FALSE) ;
    l_CellIndexText->string = str;
    l_probeTextInfoGroup->enableNotify(TRUE) ;
  }
  l_sceneProbe->enableNotify(TRUE) ;
}/*---------------------------------------------------------------------------*/

void motion_probeCB(void *userData, PoMeshProbePoint *probe, const PbCell *cell) {
  if (!cell) return;
  char str[80];

  const PbMesh *mesh = probe->getMesh();
  int scalar_data_index = probe->valuesIndex.getValue();
  const float *scalar_data = mesh->getValuesSet(scalar_data_index);
  if (scalar_data) {
    SbVec3f pcoord;
    cell->locatePoint(probe->position.getValue(),0.0,pcoord);
    float probe_value = cell->getValue(pcoord,scalar_data);
    sprintf(str,"value at probe : %.3f",probe_value);
  } else
    sprintf(str,"no value mapped");
    
  l_probeTextInfoGroup->enableNotify(FALSE) ;
  l_ProbeValueText->string = str;
  l_probeTextInfoGroup->enableNotify(TRUE) ;
}/*---------------------------------------------------------------------------*/

void enter_probeCB(void *userData, PoMeshProbePoint *probe, const PbCell *cell) {
  l_probeTextInfoGroup->enableNotify(FALSE);
  l_textSwitch->whichChild = 0;
  l_probeTextInfoGroup->enableNotify(TRUE);
}/*---------------------------------------------------------------------------*/

void leave_probeCB(void *userData, PoMeshProbePoint *probe, const PbCell *cell) {
  l_probeTextInfoGroup->enableNotify(FALSE);
  l_textSwitch->whichChild = 1;
  l_probeTextInfoGroup->enableNotify(TRUE);
}/*---------------------------------------------------------------------------*/

void MeshProbeViewer::preWriteAction() {
  enableConnection(v_CellFacets,FALSE);
  enableConnection(v_CellIndices,FALSE);
  enableConnection(v_CellEdges,FALSE);
}/*---------------------------------------------------------------------------*/

void MeshProbeViewer::postWriteAction() {
  enableConnection(v_CellFacets,TRUE);
  enableConnection(v_CellIndices,TRUE);
  enableConnection(v_CellEdges,TRUE);
}/*---------------------------------------------------------------------------*/

void MeshProbeViewer::enableConnection(SoNode *node, SbBool flag) {
  SoFieldList fields;
  int num_fields = node->getFields(fields);
  for (int i=0; i<num_fields; i++) {
    SoField *field = fields[i];
    if (field->isConnected()) field->enableConnection(flag);
  }
}/*---------------------------------------------------------------------------*/
