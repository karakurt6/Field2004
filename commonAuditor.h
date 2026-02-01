class CheckSwitchAuditor : public SoDialogCheckBoxAuditor {
  SoSwitch *sw;
public:
  CheckSwitchAuditor(SoSwitch *sw) { this->sw = sw; }
  void dialogCheckBox(SoDialogCheckBox *checkBox) {
    sw->whichChild = (checkBox->state.getValue() == TRUE)
      ? SO_SWITCH_ALL 
      : SO_SWITCH_NONE;
  }
};


class RealSliderAuditor : public SoDialogRealSliderAuditor {
  SoSFFloat* field;
public:
  RealSliderAuditor(SoSFFloat* field) { this->field = field; }
  void dialogRealSlider(SoDialogRealSlider *slider) {
    *field = slider->value;
  }
};
