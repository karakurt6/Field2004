//****************************************************************************
//                                                                        
// demonstration class to visualize scalar data on a surface mesh 
// with 3D-Data-Master  
//                                                                        
//  author : J-Michel Godinaud                                            
//****************************************************************************
#include <windows.h>
#include "Mesh2DScalarViewer.h"
#include "commonAuditor.h"

class AltitudeDataSetUsedChoice : public SoDialogChoiceAuditor {
  Mesh2DScalarViewer *mViewer;
public:
  AltitudeDataSetUsedChoice(Mesh2DScalarViewer *mViewer) { this->mViewer = mViewer; }
  void dialogChoice (SoDialogChoice *dc) {
    mViewer->setAltitudeDataSet(dc->selectedItem.getValue());
  }
};

class ContourAnnoPathChoice : public SoDialogChoiceAuditor {
  Mesh2DScalarViewer *mViewer;
public:
  ContourAnnoPathChoice(Mesh2DScalarViewer *mViewer) { this->mViewer = mViewer; }
  void dialogChoice (SoDialogChoice *dc) {
    mViewer->setAnnoPath(dc->selectedItem.getValue());
  }
};

class ContourCrossStatChoice : public SoDialogChoiceAuditor {
  Mesh2DScalarViewer *mViewer;
public:
  ContourCrossStatChoice(Mesh2DScalarViewer *mViewer) { this->mViewer = mViewer; }
  void dialogChoice (SoDialogChoice *dc) {
    mViewer->setCrossStat(dc->selectedItem.getValue());
  }
};

class MeshFilledCheck : public SoDialogCheckBoxAuditor {
  Mesh2DScalarViewer *mViewer;
public:
  MeshFilledCheck(Mesh2DScalarViewer *mViewer) { this->mViewer = mViewer; }
  void dialogCheckBox(SoDialogCheckBox *cb) {
    mViewer->setMeshFilled(cb->state.getValue());
  }
};

class AltitudeEnableCheck : public SoDialogCheckBoxAuditor {
  Mesh2DScalarViewer *mViewer;
public:
  AltitudeEnableCheck(Mesh2DScalarViewer *mViewer) { this->mViewer = mViewer; }
  void dialogCheckBox(SoDialogCheckBox *cb) {
    mViewer->setAltitudeEnable(cb->state.getValue());
  }
};

class ContourAnnotEnableAuditor : public SoDialogCheckBoxAuditor {
  Mesh2DScalarViewer *mViewer;
public:
  ContourAnnotEnableAuditor(Mesh2DScalarViewer *mViewer) { this->mViewer = mViewer; }
  void dialogCheckBox(SoDialogCheckBox *cb) {
    mViewer->setEnableContourAnnot(cb->state.getValue());
  }
};

//------------------------------------------------------------------------
Mesh2DScalarViewer::Mesh2DScalarViewer() :
  v_ZDataSetIndex(-1)
{
}/*---------------------------------------------------------------------------*/

void Mesh2DScalarViewer::buildSceneGraph(const PbMesh *pb_mesh, PoDomain *domain, 
				   SoGroup *scene_mesh2D,
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

  v_Domain = domain;

  // define a mesh filled
  v_MeshFilled = new PoMeshFilled;
  v_MeshFilled->valuesIndex.connectFrom(which_dataset);
  v_MeshFilled->zValuesIndex.setValue(v_ZDataSetIndex);
  v_MeshFilled->coloringType = PoMesh::COLOR_MAPPING;
  which_coloring_type = &(v_MeshFilled->coloringType);

  // define the mesh limit
  v_MeshLimit = new PoMeshLimit;
  v_MeshLimit->zValuesIndex.connectFrom(&v_MeshFilled->zValuesIndex);

  // define the mesh contour lines
  v_MeshContouring = new PoMeshContouring;
  v_MeshContouring->zValuesIndex.connectFrom(&v_MeshFilled->zValuesIndex);
  v_MeshContouring->valuesIndex.connectFrom(which_dataset);
  v_MeshContouring->annotGap = 0.3F;
  v_MeshContouring->annotIsContourClip = TRUE;
  v_MeshContouring->set("minorContourLineApp.drawStyle", "linePattern 0xF0F0") ;
  v_MeshContouring->set("annotTextApp.material", "diffuseColor 1. 1. 1.") ;   
  v_MeshContouring->set("annotBackgroundApp.material", "diffuseColor 0.4 0.4 0.4") ;
  v_MeshContouring->set("annotBackgroundBorderApp.material", "diffuseColor 1 1 1") ;

  // define a mesh lines
  v_MeshLines = new PoMeshLines;
  v_MeshLines->valuesIndex = -1;
  v_MeshLines->zValuesIndex.connectFrom(&v_MeshFilled->zValuesIndex);

  // define the mesh sides
  v_MeshSides = new PoMeshSides;
  v_MeshSides->thresholdType = PoMeshSides::THRESHOLD_VALUE;
  v_MeshSides->zValuesIndex.connectFrom(&v_MeshFilled->zValuesIndex);
  if (v_ZDataSetIndex >=0) {
    float zmin,zmax;
    pb_mesh->getMinValuesSet(v_ZDataSetIndex,zmin);
    pb_mesh->getMaxValuesSet(v_ZDataSetIndex,zmax);
    v_MeshSides->thresholdValue.setValue(zmin-(zmax-zmin)/20);
  } else 
    v_MeshSides->thresholdValue.setValue(0);

  // define the nodes to allow the visibility switch
  v_MeshLimitSwitch = new SoSwitch;
  v_MeshLimitSwitch->addChild(v_MeshLimit) ;
  v_MeshLimitSwitch->whichChild = SO_SWITCH_ALL;

  v_MeshContouringSwitch = new SoSwitch;
  v_MeshContouringSwitch->addChild(v_MeshContouring) ;
  v_MeshContouringSwitch->whichChild = SO_SWITCH_ALL;

  v_MeshFilledSwitch = new SoSwitch;
  v_MeshFilledSwitch->addChild(v_MeshFilled) ;
  v_MeshFilledSwitch->whichChild = SO_SWITCH_ALL;

  v_MeshLinesSwitch = new SoSwitch;
  v_MeshLinesSwitch->addChild(v_MeshLines) ;
  v_MeshLinesSwitch->whichChild = SO_SWITCH_NONE;

  v_MeshSidesSwitch = new SoSwitch;
  v_MeshSidesSwitch->addChild(v_MeshSides) ;
  v_MeshSidesSwitch->whichChild = SO_SWITCH_NONE;

  SoPickStyle *pick_style = new SoPickStyle;
  pick_style->style = SoPickStyle::UNPICKABLE;

  scene_mesh2D->addChild(pick_style);
  if (v_Domain) scene_mesh2D->addChild(v_Domain);
  scene_mesh2D->addChild(v_MeshFilledSwitch);
  scene_mesh2D->addChild(v_MeshLimitSwitch);
  scene_mesh2D->addChild(v_MeshContouringSwitch);
  scene_mesh2D->addChild(v_MeshLinesSwitch);
  scene_mesh2D->addChild(v_MeshSidesSwitch);
}/*---------------------------------------------------------------------------*/

void Mesh2DScalarViewer::setAltitudeDataSet(int dataset)
{
  SoDialogCheckBox* altitudeEnable = (SoDialogCheckBox *)v_DialogBox->searchForAuditorId("AltitudeEnable");
  v_ZDataSetIndex = dataset;
  if (altitudeEnable->state.getValue())
  {
    // select another data set
    v_MeshFilled->zValuesIndex.setValue(v_ZDataSetIndex);
    float zmin = v_Vmin[v_ZDataSetIndex]; 
    float zmax = v_Vmax[v_ZDataSetIndex];
    v_MeshSides->thresholdValue.setValue(zmin-(zmax-zmin)/20);
    if (v_Domain) {
      // the domain is set to the smallest cube, including the 
      // 3D mesh representation
      SbVec3f min = v_Domain->min.getValue();  min[2] = zmin;
      SbVec3f max = v_Domain->max.getValue();  max[2] = zmax;
      v_Domain->setValues(min,max,PoDomain::MIN_BOUNDING_CUBE);
    }
  }
}/*---------------------------------------------------------------------------*/

void Mesh2DScalarViewer::setAnnoPath(int sel)
{
  v_MeshContouring->annotPath = (PoMeshContouring::AnnotPath)sel;
}/*---------------------------------------------------------------------------*/

void Mesh2DScalarViewer::setCrossStat(int sel)
{
  v_MeshContouring->annotCrossStatus = (PoMeshContouring::AnnotCrossStatus)sel;
}/*---------------------------------------------------------------------------*/

void Mesh2DScalarViewer::setMeshFilled(SbBool state)
{
  if (!state)
  {
    v_MeshFilledSwitch->whichChild = SO_SWITCH_NONE;
	  v_MeshContouring->coloringType = PoMesh::COLOR_MAPPING;
  } else {
	  v_MeshFilledSwitch->whichChild = SO_SWITCH_ALL;
	  v_MeshContouring->coloringType = PoMesh::COLOR_INHERITED;
  }
}/*---------------------------------------------------------------------------*/

void Mesh2DScalarViewer::setAltitudeEnable(SbBool enable)
{
  if (enable)
  {
    setAltitudeDataSet(v_ZDataSetIndex);
  }
  else
  {
    v_MeshFilled->zValuesIndex.setValue(-1);
    v_MeshSides->thresholdValue.setValue(0);
  }
}/*---------------------------------------------------------------------------*/

void Mesh2DScalarViewer::setEnableContourAnnot(SbBool enable)
{
  if (enable)
  {
/*    SoRowDialog* contourAnnot = (SoRowDialog*)v_DialogBox->searchForAuditorId("ContourAnnot");
    SoDialogCheckBox* contourAnnotBackground = (SoDialogCheckBox *)v_DialogBox->searchForAuditorId("ContourAnnotBackground");
    contourAnnot->addChild(new SoDialogCheckBox);//contourAnnotBackground);
    SoDialogComboBox* contourAnnoPathChoice = (SoDialogComboBox *)v_DialogBox->searchForAuditorId("ContourAnnotPath");
    contourAnnot->addChild(contourAnnoPathChoice);
    SoDialogRealSlider* contourAnnotGapSlider = (SoDialogRealSlider *)v_DialogBox->searchForAuditorId("ContourAnnotGap");
    contourAnnot->addChild(contourAnnotGapSlider);
    SoDialogRealSlider* contourAnnotFontSizeSlider = (SoDialogRealSlider *)v_DialogBox->searchForAuditorId("ContourAnnotFontSize");
    contourAnnot->addChild(contourAnnotFontSizeSlider);
    SoDialogComboBox* contourAnnoCrossing = (SoDialogComboBox *)v_DialogBox->searchForAuditorId("ContourAnnotCrossing");
    contourAnnot->addChild(contourAnnoCrossing);
*/
  }
  else
  {
/*    SoRowDialog* contourAnnot = (SoRowDialog*)v_DialogBox->searchForAuditorId("ContourAnnot");
    contourAnnot->getChild(2)->ref();
    contourAnnot->getChild(3)->ref();
    contourAnnot->getChild(4)->ref();
    contourAnnot->getChild(5)->ref();
    contourAnnot->getChild(6)->ref();
    contourAnnot->removeChild(6);
    contourAnnot->removeChild(5);
    contourAnnot->removeChild(4);
    contourAnnot->removeChild(3);
    contourAnnot->removeChild(2);
*/  }
}/*---------------------------------------------------------------------------*/


SoDialogComponent *
Mesh2DScalarViewer::buildDialogBox (const PbMesh *pb_mesh, const char **dataset_name)
{
  HRSRC hRC = FindResource(NULL, "GuiTabScalar2D", "IV");
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
  // Altitude Enable check
  SoDialogCheckBox* altitudeEnable = (SoDialogCheckBox *)v_DialogBox->searchForAuditorId("AltitudeEnable");
  altitudeEnable->addAuditor(new AltitudeEnableCheck(this));
  altitudeEnable->state = (v_ZDataSetIndex == -1) ? FALSE : TRUE;

  // Altitude DataSet Used
  SoDialogComboBox* altitudeDataSetUsedChoice = (SoDialogComboBox *)v_DialogBox->searchForAuditorId("AltitudeDatasetUsed");
  SbString dataSet = "";
  for (int i=0; i<v_DataSetNum; i++)
  {
    dataSet = *pb_mesh->getValuesSetName(i);
    if (dataSet.getLength() == 0)
    {
      dataSet = "Dataset ";
      dataSet += i;
    }
    altitudeDataSetUsedChoice->items.set1Value(i, dataSet);
  }
  altitudeDataSetUsedChoice->addAuditor(new AltitudeDataSetUsedChoice(this));
  altitudeDataSetUsedChoice->selectedItem.setValue((v_ZDataSetIndex = 0));

  //////////// 
  // Mesh Lines Enable check
  SoDialogCheckBox* meshLinesEnable = (SoDialogCheckBox *)v_DialogBox->searchForAuditorId("MeshLinesEnable");
  meshLinesEnable->addAuditor(new CheckSwitchAuditor(v_MeshLinesSwitch));
  meshLinesEnable->state = (v_MeshLinesSwitch->whichChild.getValue() == SO_SWITCH_ALL);

  //////////// 
  // Mesh Limit check
  SoDialogCheckBox* meshLimitEnable = (SoDialogCheckBox *)v_DialogBox->searchForAuditorId("MeshLimitEnable");
  meshLimitEnable->addAuditor(new CheckSwitchAuditor(v_MeshLimitSwitch));
  meshLimitEnable->state = (v_MeshLimitSwitch->whichChild.getValue() == SO_SWITCH_ALL);

  //////////// 
  // Mesh Filled check
  SoDialogCheckBox* meshFilledEnable = (SoDialogCheckBox *)v_DialogBox->searchForAuditorId("MeshFilledEnable");
  meshFilledEnable->addAuditor(new MeshFilledCheck(this));
  meshFilledEnable->state = (v_MeshFilledSwitch->whichChild.getValue() == SO_SWITCH_ALL);

  //////////// 
  // Mesh Sides check
  SoDialogCheckBox* meshSidesEnable = (SoDialogCheckBox *)v_DialogBox->searchForAuditorId("MeshSidesEnable");
  meshSidesEnable->addAuditor(new CheckSwitchAuditor(v_MeshSidesSwitch));
  meshSidesEnable->state = (v_MeshSidesSwitch->whichChild.getValue() == SO_SWITCH_ALL);

  //////////// 
  // Contour Lines Enable check
  SoDialogCheckBox* contourLinesEnable = (SoDialogCheckBox *)v_DialogBox->searchForAuditorId("ContourLinesEnable");
  contourLinesEnable->addAuditor(new CheckSwitchAuditor(v_MeshContouringSwitch));
  contourLinesEnable->state = (v_MeshContouringSwitch->whichChild.getValue() == SO_SWITCH_ALL);

  //////////// 
  // Contour Annot enable check
  SoDialogCheckBox* contourAnnotEnable = (SoDialogCheckBox *)v_DialogBox->searchForAuditorId("ContourAnnotEnable");
  contourAnnotEnable->addAuditor(new ContourAnnotEnableAuditor(this));
  v_MeshContouring->annotIsVisible.connectFrom(&contourAnnotEnable->state);

  //////////// 
  // Contour Annot background check
  SoDialogCheckBox* contourAnnotBackground = (SoDialogCheckBox *)v_DialogBox->searchForAuditorId("ContourAnnotBackground");
  v_MeshContouring->annotIsBackground.connectFrom(&contourAnnotBackground->state);

  //////////// 
  // Contour Anno Path Choice
  SoDialogComboBox* contourAnnoPathChoice = (SoDialogComboBox *)v_DialogBox->searchForAuditorId("ContourAnnotPath");
  contourAnnoPathChoice->addAuditor(new ContourAnnoPathChoice(this));
  contourAnnoPathChoice->selectedItem.setValue(v_MeshContouring->annotPath.getValue());

  //////////// 
  // contour Annot Gap Slider
  SoDialogRealSlider* contourAnnotGapSlider = (SoDialogRealSlider *)v_DialogBox->searchForAuditorId("ContourAnnotGap");
  contourAnnotGapSlider->addAuditor(new RealSliderAuditor(&v_MeshContouring->annotGap));
  contourAnnotGapSlider->value = v_MeshContouring->annotGap.getValue(); 

  //////////// 
  // contour Annot Font Size Slider
  SoDialogRealSlider* contourAnnotFontSizeSlider = (SoDialogRealSlider *)v_DialogBox->searchForAuditorId("ContourAnnotFontSize");
  contourAnnotFontSizeSlider->addAuditor(new RealSliderAuditor(&v_MeshContouring->annotFontSize));
  contourAnnotFontSizeSlider->value = v_MeshContouring->annotFontSize.getValue(); 

  //////////// 
  // Contour Anno Crossing Choice
  SoDialogComboBox* contourAnnoCrossing = (SoDialogComboBox *)v_DialogBox->searchForAuditorId("ContourAnnotCrossing");
  contourAnnoCrossing->addAuditor(new ContourCrossStatChoice(this));
  contourAnnoCrossing->selectedItem.setValue(v_MeshContouring->annotPath.getValue());

  setEnableContourAnnot(FALSE);

  return v_DialogBox;
}/*---------------------------------------------------------------------------*/

/*
void Mesh2DScalarViewer::manageDialog (PoXtDialog::PoXtDialogDataCB * data) {
  const PoXtElementData * xtElt=data->dialog->get1Value(data->widgetNumber);

  case  PoXtElementData::TOGGLE_BUTTON : {
    switch (data->widgetNumber) {
    case TOGGLE_LIMIT:
      v_MeshLimitSwitch->whichChild = (v_MeshLimitSwitch->whichChild.getValue() == SO_SWITCH_ALL) 
	? SO_SWITCH_NONE
	: SO_SWITCH_ALL;
      break;
    case TOGGLE_CONTOUR_LINES:
      v_MeshContouringSwitch->whichChild = (v_MeshContouringSwitch->whichChild.getValue() == SO_SWITCH_ALL) 
	? SO_SWITCH_NONE
	: SO_SWITCH_ALL;
      break;
    case TOGGLE_FILLED:
      if (v_MeshFilledSwitch->whichChild.getValue() == SO_SWITCH_ALL) {
	v_MeshFilledSwitch->whichChild = SO_SWITCH_NONE;
	v_MeshContouring->coloringType = PoMesh::COLOR_MAPPING;
      } else {
	v_MeshFilledSwitch->whichChild = SO_SWITCH_ALL;
	v_MeshContouring->coloringType = PoMesh::COLOR_INHERITED;
      }
      break;
  } break;
  }

}
*/

void Mesh2DScalarViewer::preWriteAction() {
  enableConnection(v_MeshFilled,FALSE);
  enableConnection(v_MeshLimit,FALSE);
  enableConnection(v_MeshLines,FALSE);
  enableConnection(v_MeshContouring,FALSE);
  enableConnection(v_MeshSides,FALSE);
}/*---------------------------------------------------------------------------*/

void Mesh2DScalarViewer::postWriteAction() {
  enableConnection(v_MeshFilled,TRUE);
  enableConnection(v_MeshLimit,TRUE);
  enableConnection(v_MeshLines,TRUE);
  enableConnection(v_MeshContouring,TRUE);
  enableConnection(v_MeshSides,TRUE);
}/*---------------------------------------------------------------------------*/

void Mesh2DScalarViewer::enableConnection(SoNode *node, SbBool flag) {
  SoFieldList fields;
  int num_fields = node->getFields(fields);
  for (int i=0; i<num_fields; i++) {
    SoField *field = fields[i];
    if (field->isConnected()) field->enableConnection(flag);
  }
}/*---------------------------------------------------------------------------*/
