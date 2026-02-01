//------------------------------------------------------------------------
//                                                                        
// demonstration classes to visualize stream on a mesh with 3D-Data-Master 
//                                                                        
//  author : J-Michel Godinaud                                            
//------------------------------------------------------------------------
#include "MeshStreamViewer.h"
#include "commonAuditor.h"
#include <Inventor/nodes/SoPointSet.h>

#define WIDTH_FACTOR 200
#define DEF_MAX_STEP_NUM 10000

class StreamLinesStyleChoice  : public SoDialogChoiceAuditor {
  MeshStreamViewer *mViewer;
public:
  StreamLinesStyleChoice(MeshStreamViewer *mViewer) { this->mViewer = mViewer;}
  void dialogChoice (SoDialogChoice *dc) {
    mViewer->setStreamLinesStyle(dc->selectedItem.getValue());
  }
};

class SourceShapeChoice : public SoDialogChoiceAuditor {
  MeshStreamViewer *mViewer;
public:
  SourceShapeChoice(MeshStreamViewer *mViewer) { this->mViewer = mViewer; }
  void dialogChoice (SoDialogChoice *dc) {
    mViewer->setSourceShape(dc->selectedItem.getValue());
  }
};

class SourceShapeSizeSlider : public SoDialogRealSliderAuditor {
  MeshStreamViewer *mViewer;
public:
  SourceShapeSizeSlider(MeshStreamViewer *mViewer) { this->mViewer = mViewer; }
  void dialogRealSlider (SoDialogRealSlider *rs) {
    mViewer->setSourceShapeSize(rs->value.getValue());
  }
};

class SourceStartPointsSlider : public SoDialogIntegerSliderAuditor {
  MeshStreamViewer *mViewer;
public:
  SourceStartPointsSlider(MeshStreamViewer *mViewer) { this->mViewer = mViewer; }
  void dialogIntegerSlider (SoDialogIntegerSlider *rs) {
    mViewer->setSourceStartPoints(rs->value.getValue());
  }
};

class AnimationLengthOfTadpoleSlider : public SoDialogRealSliderAuditor {
  MeshStreamViewer *mViewer;
public:
  AnimationLengthOfTadpoleSlider(MeshStreamViewer *mViewer) { this->mViewer = mViewer; }
  void dialogRealSlider (SoDialogRealSlider *rs) {
    mViewer->setAnimationLengthOfTadpole(rs->value.getValue());
  }
};

class AnimationFrame : public SoDialogPushButtonAuditor {
  MeshStreamViewer *mViewer;
public:
  AnimationFrame(MeshStreamViewer *mViewer) { this->mViewer = mViewer; }
  void dialogPushButton (SoDialogPushButton *cb) {
    mViewer->setAnimationFrame();
  }
};/*---------------------------------------------------------------------------*/

MeshStreamViewer::MeshStreamViewer() :
  v_ViewFrame(0),
  v_NumStartPoints(7),
  v_SourceShape(SOURCE_CIRCLE),
  v_SourceSizeFactor(0.25)
{
}/*---------------------------------------------------------------------------*/

void MeshStreamViewer::buildSceneGraph(const PbMesh *pb_mesh, PoDomain *domain, 
				       SoGroup *scene_stream,
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
  SbVec3f bounding_box_center = mesh_bounding_box.getCenter();

  float sx,sy,sz; mesh_bounding_box.getSize(sx,sy,sz);
  float maxs = (sx > sy) ? sx : sy; if (sz > maxs) maxs = sz;

  float max_vec; pb_mesh->getMaxVecsSet(0, max_vec);
  float time_step = 0.05*maxs/max_vec;

  v_TadpoleLengthFactor = 0.1*maxs/max_vec;

#define NUM_COLR 7
  SbColor *particle_colors = new SbColor[NUM_COLR];
  particle_colors[0] = SbColor(1,1,1);
  particle_colors[1] = SbColor(1,0,0);
  particle_colors[2] = SbColor(0,1,0);
  particle_colors[3] = SbColor(0,0,1);
  particle_colors[4] = SbColor(1,1,0);
  particle_colors[5] = SbColor(0,1,1);
  particle_colors[6] = SbColor(1,0,1);

  // define StreamLine
  v_MeshStreamLines = new PoStreamLine;
  v_MeshStreamLines->vecsIndex.setValue(0);
  v_MeshStreamLines->coloringType = PoMesh::COLOR_MAPPING;
  v_MeshStreamLines->integrationMaxStepNumber = DEF_MAX_STEP_NUM;
  v_MeshStreamLines->colors.setValues(0,NUM_COLR,particle_colors);

  v_MeshStreamPoint = new PoStreamPointMotion;
  v_MeshStreamPoint->vecsIndex.setValue(0);
  v_MeshStreamPoint->coloringType = PoMesh::COLOR_MAPPING;
  v_MeshStreamPoint->startPoints.connectFrom(&v_MeshStreamLines->startPoints);
  v_MeshStreamPoint->pulseFrequency = 15;
  v_MeshStreamPoint->timeStep = time_step;
  v_MeshStreamPoint->colors.connectFrom(&v_MeshStreamLines->colors);
  v_MeshStreamPoint->integrationMaxStepNumber.connectFrom(&v_MeshStreamLines->integrationMaxStepNumber);

  v_MeshStreamSphere = new PoStreamSphereMotion;
  v_MeshStreamSphere->vecsIndex.setValue(0);
  v_MeshStreamSphere->coloringType = PoMesh::COLOR_MAPPING;
  v_MeshStreamSphere->startPoints.connectFrom(&v_MeshStreamLines->startPoints);
  v_MeshStreamSphere->pulseFrequency.connectFrom(&v_MeshStreamPoint->pulseFrequency);
  v_MeshStreamSphere->timeStep.connectFrom(&v_MeshStreamPoint->timeStep);
  v_MeshStreamSphere->sphereRadius = maxs/150;
  v_MeshStreamSphere->colors.connectFrom(&v_MeshStreamLines->colors);
  v_MeshStreamSphere->integrationMaxStepNumber.connectFrom(&v_MeshStreamLines->integrationMaxStepNumber);

  v_MeshStreamLineParticle = new PoStreamLineMotion;
  v_MeshStreamLineParticle->vecsIndex.setValue(0);
  v_MeshStreamLineParticle->coloringType = PoMesh::COLOR_MAPPING;
  v_MeshStreamLineParticle->startPoints.connectFrom(&v_MeshStreamLines->startPoints);
  v_MeshStreamLineParticle->pulseFrequency.connectFrom(&v_MeshStreamPoint->pulseFrequency);
  v_MeshStreamLineParticle->timeStep.connectFrom(&v_MeshStreamPoint->timeStep);
  v_MeshStreamLineParticle->integrationMaxStepNumber.connectFrom(&v_MeshStreamLines->integrationMaxStepNumber);

  v_MeshStreamTadpole = new PoStreamTadpoleMotion;
  v_MeshStreamTadpole->vecsIndex.setValue(0);
  v_MeshStreamTadpole->coloringType = PoMesh::COLOR_MAPPING;
  v_MeshStreamTadpole->startPoints.connectFrom(&v_MeshStreamLines->startPoints);
  v_MeshStreamTadpole->pulseFrequency.connectFrom(&v_MeshStreamPoint->pulseFrequency);
  v_MeshStreamTadpole->timeStep.connectFrom(&v_MeshStreamPoint->timeStep);
  v_MeshStreamTadpole->lengthFactor = v_TadpoleLengthFactor;
  v_MeshStreamTadpole->integrationMaxStepNumber.connectFrom(&v_MeshStreamLines->integrationMaxStepNumber);

  v_MeshStreamSurfaces = new PoStreamSurface;
  v_MeshStreamSurfaces->vecsIndex.setValue(0);
  v_MeshStreamSurfaces->coloringType = PoMesh::COLOR_MAPPING;
  v_MeshStreamSurfaces->startPoints.set1Value(0,bounding_box_center);
  v_MeshStreamSurfaces->rakeOrientation.set1Value(0,SbVec3f(1,0,0));
  v_MeshStreamSurfaces->rakeLength = 2*maxs/WIDTH_FACTOR;
  v_MeshStreamSurfaces->numLinesPerRake = 5;
  v_MeshStreamSurfaces->integrationMaxStepNumber.connectFrom(&v_MeshStreamLines->integrationMaxStepNumber);
  v_MeshStreamSurfaces->colors.connectFrom(&v_MeshStreamLines->colors);

  // define stream sources representation
  SoMaterial *material = new SoMaterial;
  material->diffuseColor.set1Value(0, 1.,0.,0.);
  SoDrawStyle *draw_style = new SoDrawStyle;
  draw_style->pointSize = 4;
  SoCoordinate3 *source_coord = new SoCoordinate3;
  source_coord->point.connectFrom(&v_MeshStreamLines->startPoints);
  SoPointSet *source_points = new SoPointSet;
  source_points->numPoints = -1; // use all points in source_coord
  v_StreamSource = new SoSeparator;
  v_StreamSource->addChild(material);
  v_StreamSource->addChild(draw_style);
  v_StreamSource->addChild(source_coord);
  v_StreamSource->addChild(source_points);

  v_MeshStreamRepSwitch = new SoSwitch;
  v_MeshStreamRepSwitch->addChild(v_MeshStreamLines);
  v_MeshStreamRepSwitch->addChild(v_MeshStreamLineParticle);
  v_MeshStreamRepSwitch->addChild(v_MeshStreamTadpole);
  v_MeshStreamRepSwitch->addChild(v_MeshStreamPoint);
  v_MeshStreamRepSwitch->addChild(v_MeshStreamSphere);
  v_MeshStreamRepSwitch->addChild(v_MeshStreamSurfaces);
  v_MeshStreamRepSwitch->whichChild = SO_SWITCH_NONE;

  v_StreamSourceSwitch = new SoSwitch;
  v_StreamSourceSwitch->addChild(v_StreamSource);
  v_StreamSourceSwitch->whichChild = SO_SWITCH_ALL;

  v_MeshStreamSwitch = new SoSwitch;
  v_MeshStreamSwitch->addChild(v_StreamSourceSwitch);
  v_MeshStreamSwitch->addChild(v_MeshStreamRepSwitch);
  v_MeshStreamSwitch->whichChild = SO_SWITCH_NONE;
  
  SoPickStyle *pick_style = new SoPickStyle;
  pick_style->style = SoPickStyle::UNPICKABLE;

  scene_stream->addChild(pick_style);
  scene_stream->addChild(module_data_mapping);
  scene_stream->addChild(v_MeshStreamSwitch);

}/*---------------------------------------------------------------------------*/

void MeshStreamViewer::setSourceShape(int selected)
{
  v_SourceShape = (MeshStreamViewer::SourceShapeType) selected;
  moveStreamSource();
}/*---------------------------------------------------------------------------*/

void MeshStreamViewer::setStreamLinesStyle(int style)
{
  v_MeshStreamRepSwitch->whichChild = style;
  if (style == 2)
    v_animationLengthOfTadpole->enable = TRUE;
  else
    v_animationLengthOfTadpole->enable = FALSE;
}/*---------------------------------------------------------------------------*/

void MeshStreamViewer::setSourceShapeSize(float size)
{
  v_SourceSizeFactor = size;
  moveStreamSource();
}/*---------------------------------------------------------------------------*/

void MeshStreamViewer::setSourceStartPoints(int num)
{
  v_NumStartPoints = num;
  moveStreamSource();
}/*---------------------------------------------------------------------------*/

void MeshStreamViewer::setAnimationLengthOfTadpole(float length)
{
  v_MeshStreamTadpole->lengthFactor = v_TadpoleLengthFactor = length;
}/*---------------------------------------------------------------------------*/

void MeshStreamViewer::setAnimationFrame()
{
  v_MeshStreamPoint->viewFrame = v_ViewFrame;
  v_MeshStreamSphere->viewFrame = v_ViewFrame;
  v_MeshStreamLineParticle->viewFrame = v_ViewFrame;
  v_MeshStreamTadpole->viewFrame = v_ViewFrame;
  v_ViewFrame++;
}/*---------------------------------------------------------------------------*/

SoDialogComponent *
MeshStreamViewer::buildDialogBox (const PbMesh *pb_mesh)
{
  HRSRC hRC = FindResource(NULL, "GuiTabStream", "IV");
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

  SoDialogGroup *v_DialogBox = (SoDialogGroup *)myGroup->getChild(0);

  //////////// 
  // StreamLines check
  SoDialogCheckBox* streamLinesCheck = (SoDialogCheckBox *)v_DialogBox->searchForAuditorId("StreamLinesEnable");
  streamLinesCheck->addAuditor(new CheckSwitchAuditor(v_MeshStreamSwitch));
  streamLinesCheck->state = (v_MeshStreamSwitch->whichChild.getValue() == SO_SWITCH_ALL);

  //////////// 
  // StreamLines style choice
  SoDialogComboBox* streamLinesStyle = (SoDialogComboBox *)v_DialogBox->searchForAuditorId("StreamLinesStyle");
  streamLinesStyle->addAuditor(new StreamLinesStyleChoice(this));
  v_MeshStreamRepSwitch->whichChild = streamLinesStyle->selectedItem.getValue();

  //////////// 
  // Source Shape Type check
  SoDialogChoice* sourceShapeTypeChoice = (SoDialogChoice *)v_DialogBox->searchForAuditorId("SourceShapeType");
  sourceShapeTypeChoice->addAuditor(new SourceShapeChoice(this));

  //////////// 
  // Source Shape Size Slider
  SoDialogRealSlider* sourceShapeSize = (SoDialogRealSlider *)v_DialogBox->searchForAuditorId("SourceShapeSize");
  sourceShapeSize->addAuditor(new SourceShapeSizeSlider(this));
  sourceShapeSize->value = v_SourceSizeFactor;

  //////////// 
  // Source Start Points Slider
  SoDialogIntegerSlider* sourceStart = (SoDialogIntegerSlider *)v_DialogBox->searchForAuditorId("SourceStartPoints");
  sourceStart->addAuditor(new SourceStartPointsSlider(this));
  sourceStart->value = v_NumStartPoints;

  //////////// 
  // Animation Random Start Slider
  SoDialogCheckBox* animationRandomStart = (SoDialogCheckBox *)v_DialogBox->searchForAuditorId("AnimationRandomStart");
  animationRandomStart->state = (v_MeshStreamPoint->isStartRandomized.getValue());
	v_MeshStreamPoint->isStartRandomized.connectFrom(&animationRandomStart->state);
	v_MeshStreamSphere->isStartRandomized.connectFrom(&animationRandomStart->state);
	v_MeshStreamLineParticle->isStartRandomized.connectFrom(&animationRandomStart->state);
	v_MeshStreamTadpole->isStartRandomized.connectFrom(&animationRandomStart->state);

  //////////// 
  // Animation paricul Time Step Slider
  SoDialogRealSlider* animationParticuleTimeStep = (SoDialogRealSlider *)v_DialogBox->searchForAuditorId("AnimationParticuleTimeStep");
  animationParticuleTimeStep->addAuditor(new RealSliderAuditor(&v_MeshStreamPoint->timeStep));

  SbBox3f box = pb_mesh->getBoundingBox();
  float dx,dy,dz;
  box.getSize(dx,dy,dz);
  float max_vec; pb_mesh->getMaxVecsSet(0, max_vec);
  float maxs = (dx > dy) ? dx : dy; if (dz > maxs) maxs = dz;
  float time_step = 0.05*maxs/max_vec;

  // compute a correct format style for value display. add 2 digits after significant digit.
  SbString style = "%.";
  style += int (3 + log10(1.0/time_step));
  style += "f";

  animationParticuleTimeStep->min = 0.3*time_step;
  animationParticuleTimeStep->max = 3.0*time_step;
  animationParticuleTimeStep->value = time_step;
  animationParticuleTimeStep->format = style;

  //////////// 
  // Animation paricul Time Step Slider
  v_animationLengthOfTadpole = (SoDialogRealSlider *)v_DialogBox->searchForAuditorId("AnimationLengthOfTadpole");
  v_animationLengthOfTadpole->addAuditor(new AnimationLengthOfTadpoleSlider(this));
  v_animationLengthOfTadpole->min = v_TadpoleLengthFactor/4;
  v_animationLengthOfTadpole->max = v_TadpoleLengthFactor*7/4;
  v_animationLengthOfTadpole->value = v_TadpoleLengthFactor;

  // compute a correct format style for value display. add 2 digits after significant digit.
  style = "%.";
  style += int (3 + log10(1.0/v_TadpoleLengthFactor));
  style += "f";
  v_animationLengthOfTadpole->format = style;
  setStreamLinesStyle(streamLinesStyle->selectedItem.getValue());  // update 
  
    //////////// 
  // Animation Blinker Check
  SoDialogCheckBox* animationBlinker = (SoDialogCheckBox *)v_DialogBox->searchForAuditorId("AnimationBlinker");
  animationBlinker->state = (v_MeshStreamPoint->isBlinking.getValue());
	v_MeshStreamPoint->isBlinking.connectFrom(&animationBlinker->state);
	v_MeshStreamSphere->isBlinking.connectFrom(&animationBlinker->state);
	v_MeshStreamLineParticle->isBlinking.connectFrom(&animationBlinker->state);
	v_MeshStreamTadpole->isBlinking.connectFrom(&animationBlinker->state);

  //////////// 
  // Animation Frame Button
  SoDialogPushButton* animationFrame = (SoDialogPushButton *)v_DialogBox->searchForAuditorId("AnimationFrame");
  animationFrame->addAuditor(new AnimationFrame(this));

  return v_DialogBox;
}/*---------------------------------------------------------------------------*/

void MeshStreamViewer::updateAllConnectedToDragger(SoJackDragger *dragger, SbVec3f plane_normal){
  if (!v_IsDraggerConnected) return;

  // get the dragger position
  v_DraggerPos = dragger->translation.getValue();

  // get the dragger scale factor
  v_DraggerScale = dragger->scaleFactor.getValue();

  // rotate the plane's normal by the dragger rotation
  v_DraggerRotation = dragger->rotation.getValue();
  v_DraggerRotation.multVec(SbVec3f(0,1,0),plane_normal);

  // move start-points of streamline
  SbVec3f n=plane_normal;
  float nx,ny,nz; n.getValue(nx,ny,nz);
  if (nz!=0)
    n.setValue(-nz,0,nx);
  else if (nx != 0)
    n.setValue(-ny,nx,0);
  else
    n.setValue(1,0,0);
  
  // move stream surface
  v_MeshStreamSurfaces->startPoints.set1Value(0,v_DraggerPos);
  v_MeshStreamSurfaces->rakeOrientation.set1Value(0,n);
  v_MeshStreamSurfaces->rakeLength.setValue(v_DraggerScale[0]*v_SourceSizeFactor);

  // move stream lines, steam points & stream spheres
  moveStreamSource();
}/*---------------------------------------------------------------------------*/

void MeshStreamViewer::moveStreamSource(){
  // move stream lines, steam points & stream spheres

  SbVec3f start_point;

  SbMatrix m;
  m.setTransform (v_DraggerPos, v_DraggerRotation, v_DraggerScale);
  v_MeshStreamLines->startPoints.setNum(v_NumStartPoints);

  if (v_SourceShape == SOURCE_CIRCLE) {
    double alpha = 0;
    double d_alpha = 2*3.1415927/v_NumStartPoints;
    for (int i=0; i<v_NumStartPoints; i++, alpha += d_alpha) {
      start_point.setValue (v_SourceSizeFactor * cos(alpha), 0, v_SourceSizeFactor * sin(alpha));
      m.multVecMatrix(start_point,start_point);
      v_MeshStreamLines->startPoints.set1Value(i,start_point);
    } 
  } else {
    double xc,dx;
    xc = -v_SourceSizeFactor; dx = 2*v_SourceSizeFactor/float(v_NumStartPoints-1);
    for (int i=0; i<v_NumStartPoints; i++, xc += dx) {
      start_point.setValue(xc,0,0);
      m.multVecMatrix(start_point,start_point);
      v_MeshStreamLines->startPoints.set1Value(i,start_point);
    } 
  }

}/*---------------------------------------------------------------------------*/


/*
void MeshStreamViewer::manageDialog (PoXtDialog::PoXtDialogDataCB * data) {
  const PoXtElementData * xtElt=data->dialog->get1Value(data->widgetNumber);

  switch (xtElt->getType()) {
  case  PoXtElementData::TOGGLE_BUTTON: {
    switch (data->widgetNumber) {
    case TOGGLE_DRAGGER_CONNECT:
      v_IsDraggerConnected = (v_IsDraggerConnected) ? FALSE : TRUE;
      break;
    }
  } break;

  case  PoXtElementData::TRIGGER_BUTTON: 
    break;

  case  PoXtElementData::INTEGER_SLIDER: {
    PoXtIntSliderData *slider = (PoXtIntSliderData *)xtElt;
    int value = slider->getValue();
    switch (data->widgetNumber) {
#ifdef MORE_OPTIONS
    case SLIDER_INTEGRATION_MAX_NUMBER:
      v_MeshStreamLines->integrationMaxStepNumber = value;
      break;
#endif
    }
  } break;
  
  case  PoXtElementData::REAL_SLIDER : {
    PoXtRealSliderData *slider = (PoXtRealSliderData *)xtElt;
    float value = slider->getValue();
    switch (data->widgetNumber) {
#ifdef MORE_OPTIONS
    case SLIDER_INTEGRATION_STEP:
      v_MeshStreamSurfaces->integrationStepLengthFactor = value;
      v_MeshStreamLines->integrationStepLengthFactor = value;
      v_MeshStreamSphere->integrationStepLengthFactor = value;
      v_MeshStreamPoint->integrationStepLengthFactor = value;
      v_MeshStreamLineParticle->integrationStepLengthFactor = value;
      v_MeshStreamTadpole->integrationStepLengthFactor = value;
      break;
    case SLIDER_MAX_LIFETIME:
      v_MeshStreamSurfaces->maxLifetime = value;
      v_MeshStreamLines->maxLifetime = value;
      v_MeshStreamSphere->maxLifetime = value;
      v_MeshStreamPoint->maxLifetime = value;
      v_MeshStreamLineParticle->maxLifetime = value;
      v_MeshStreamTadpole->maxLifetime = value;
      break;
    case SLIDER_MAX_LENGTH:
      v_MeshStreamSurfaces->maxLength = value;
      v_MeshStreamLines->maxLength = value;
      v_MeshStreamSphere->maxLength = value;
      v_MeshStreamPoint->maxLength = value;
      v_MeshStreamLineParticle->maxLength = value;
      v_MeshStreamTadpole->maxLength = value;
      break;
    case SLIDER_MIN_SPEED:
      v_MeshStreamSurfaces->minSpeed = value;
      v_MeshStreamLines->minSpeed = value;
      v_MeshStreamSphere->minSpeed = value;
      v_MeshStreamPoint->minSpeed = value;
      v_MeshStreamLineParticle->minSpeed = value;
      v_MeshStreamTadpole->minSpeed = value;
      break;
#endif

    }
  } break;

  }
}
*/
/*---------------------------------------------------------------------------*/

void MeshStreamViewer::preWriteAction() {
  enableConnection(v_MeshStreamLines,FALSE);
  enableConnection(v_MeshStreamPoint,FALSE);
  enableConnection(v_MeshStreamSphere,FALSE);
  enableConnection(v_MeshStreamLineParticle,FALSE);
  enableConnection(v_MeshStreamTadpole,FALSE);
  enableConnection(v_MeshStreamSurfaces,FALSE);
}/*---------------------------------------------------------------------------*/

void MeshStreamViewer::postWriteAction() {
  enableConnection(v_MeshStreamLines,TRUE);
  enableConnection(v_MeshStreamPoint,TRUE);
  enableConnection(v_MeshStreamSphere,TRUE);
  enableConnection(v_MeshStreamLineParticle,TRUE);
  enableConnection(v_MeshStreamTadpole,TRUE);
  enableConnection(v_MeshStreamSurfaces,TRUE);
}/*---------------------------------------------------------------------------*/

void MeshStreamViewer::enableConnection(SoNode *node, SbBool flag) {
  SoFieldList fields;
  int num_fields = node->getFields(fields);
  for (int i=0; i<num_fields; i++) {
    SoField *field = fields[i];
    if (field->isConnected()) field->enableConnection(flag);
  }
}/*---------------------------------------------------------------------------*/
