#include "IPlugEffect.h"
#include "IPlug_include_in_plug_src.h"
#include "IControls.h"

IPlugEffect::IPlugEffect(const InstanceInfo& info)
: Plugin(info, MakeConfig(kNumParams, kNumPresets))
{
  GetParam(kGain)->InitDouble("Volume", 0., 100., 200.0, 0.01, "%");
  GetParam(kThreshold)->InitDouble("Threshold", 100.0, 0.1, 100.0, 50., "%");
  GetParam(kBypass)->InitBool("Bypass",false);
  //GetParam(kWet)->InitDouble("Wet", 0., -100., 100.0, 0.01, "%");
  //GetParam(kDry)->InitDouble("Dry", 0., -100., 100.0, 0.01, "%");

#if IPLUG_EDITOR // http://bit.ly/2S64BDd
  mMakeGraphicsFunc = [&]() {
    return MakeGraphics(*this, 300, 300, PLUG_FPS, GetScaleForScreen(PLUG_WIDTH, PLUG_HEIGHT));
  };
  
  mLayoutFunc = [&](IGraphics* pGraphics) {
    pGraphics->AttachCornerResizer(EUIResizerMode::Scale, false);
    pGraphics->AttachPanelBackground(COLOR_GRAY);
    pGraphics->LoadFont("Roboto-Regular", ROBOTO_FN);
    const IRECT b = pGraphics->GetBounds();
    pGraphics->AttachControl(new IVKnobControl(b.GetGridCell(1, 0, 3, 3).GetCentredInside(100), kGain));
    pGraphics->AttachControl(new IVKnobControl(b.GetGridCell(1, 2, 3, 3).GetCentredInside(100), kThreshold));
    pGraphics->AttachControl(new IVSlideSwitchControl(b.GetGridCell(1, 1, 3, 3).GetCentredInside(80), kBypass, "Bypass",
      DEFAULT_STYLE.WithShowValue(false).WithDrawShadows(false), false));
    //pGraphics->AttachControl(new IVKnobControl(b.GetGridCell(1, 0, 2, 3).GetCentredInside(100), kWet));
    //pGraphics->AttachControl(new IVKnobControl(b.GetGridCell(1, 2, 2, 3).GetCentredInside(100), kDry));
  };
    
#endif
}
#if IPLUG_DSP
void IPlugEffect::ProcessBlock(sample** inputs, sample** outputs, int nFrames)
{
  const double gain = GetParam(kGain)->Value() / 100.f;
  const double threshold = GetParam(kThreshold)->Value() / 100.f;

  int const channelCount = 2;

  const int nChans = NOutChansConnected();

  bool Bypass = GetParam(kBypass)->Value();
  //int nAvailableSamples = std::min(mEngine.Avail(nFrames), nFrames);
 
  for (int i = 0; i < channelCount; i++)
  {
    double* input = inputs[i];
    double* output = outputs[i];

    for (int s = 0; s < nFrames; ++s, ++input, ++output)
    {
      if (Bypass == true)
      {
        outputs[i][s] = inputs[i][s];
        continue;
      }
      else if (*input > 0)
      {
        // Make sure positive values can't go above the threshold:
        *output = fmin(*input, threshold)*gain;
      }
      else
      {
        // Make sure negative values can't go below the threshold:
        *output = fmax(*input, -threshold)*gain;
      }
      *output /= threshold;
    }
  }
}
#endif
