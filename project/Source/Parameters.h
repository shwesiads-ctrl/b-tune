#pragma once
#include "VocalDSP.h"
namespace matale {
struct Parameter {const char* id;const char* name;float lo,hi,step,initial;int page;};
// IDs key/scale/style/auto/retune/humanize/formant/correction preserved from v0.1.
inline constexpr Parameter parameterList[]={
{"key","Key",0,11,1,0,0},{"scale","Scale",0,4,1,0,0},{"style","Style",0,1,1,0,0},
{"auto","Processing On",0,1,1,1,0},
{"retune","Retune (ms)",1,150,1,35,0},{"humanize","Humanize (%)",0,100,1,20,0},
{"formant","Formant colour",-12,12,.1f,0,0},{"correction","Correction (%)",0,100,1,100,0},
{"threshold","Threshold (dB)",-60,0,.1f,-18,1},{"ratio","Ratio",1,12,.1f,3,1},
{"attack","Attack (ms)",.1f,100,.1f,10,1},{"release","Release (ms)",10,1000,1,120,1},
{"makeup","Makeup (dB)",0,18,.1f,0,1},
{"eq1f","Band 1 Hz",30,500,1,100,2},{"eq1g","Band 1 dB",-12,12,.1f,0,2},{"eq1q","Band 1 Q",.2f,10,.1f,.7f,2},
{"eq2f","Band 2 Hz",100,1500,1,300,2},{"eq2g","Band 2 dB",-12,12,.1f,0,2},{"eq2q","Band 2 Q",.2f,10,.1f,1,2},
{"eq3f","Band 3 Hz",400,5000,1,1200,2},{"eq3g","Band 3 dB",-12,12,.1f,0,2},{"eq3q","Band 3 Q",.2f,10,.1f,1,2},
{"eq4f","Band 4 Hz",1000,12000,1,3500,2},{"eq4g","Band 4 dB",-12,12,.1f,0,2},{"eq4q","Band 4 Q",.2f,10,.1f,1,2},
{"eq5f","Band 5 Hz",3000,20000,1,10000,2},{"eq5g","Band 5 dB",-12,12,.1f,0,2},{"eq5q","Band 5 Q",.2f,10,.1f,.7f,2},
{"delayMix","Delay wet",0,.6f,.01f,0,3},{"delayBeats","Delay beats",.125f,2,.125f,.5f,3},
{"feedback","Delay feedback",0,.85f,.01f,.25f,3},{"reverbMix","Reverb wet",0,.6f,.01f,0,3},
{"room","Room",0,1,.01f,.45f,3},{"output","Output (dB)",-24,12,.1f,0,3},
{"bpm","Fallback tempo",30,300,1,120,3}
};
inline constexpr int parameterCount=int(sizeof(parameterList)/sizeof(parameterList[0]));
inline void setParam(Params& p,int i,float v) {
    if(i<0||i>=parameterCount||!std::isfinite(v))return;
    v=std::clamp(v,parameterList[i].lo,parameterList[i].hi);
    if(i>=13 && i<=27) {
        int band=(i-13)/3;
        switch((i-13)%3){case 0:p.frequency[band]=v;break;case 1:p.eqGain[band]=v;break;case 2:p.q[band]=v;break;}
        return;
    }
    switch(i) {
    case 0:p.key=int(v);break;case 1:p.scale=int(v);break;case 2:p.style=int(v);break;
    case 3:p.power=v;break;case 4:p.retune=v;break;case 5:p.humanize=v;break;
    case 6:p.formant=v;break;case 7:p.correction=v;break;case 8:p.threshold=v;break;
    case 9:p.ratio=v;break;case 10:p.attack=v;break;case 11:p.release=v;break;case 12:p.makeup=v;break;
    case 28:p.delayMix=v;break;case 29:p.delayBeats=v;break;case 30:p.feedback=v;break;
    case 31:p.reverbMix=v;break;case 32:p.room=v;break;case 33:p.output=v;break;case 34:p.bpm=v;break;
    }
}
}
