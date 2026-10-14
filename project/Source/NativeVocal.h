#pragma once
#include "Parameters.h"
#include <limits>

namespace matale {
// Native counterparts of the browser vocal stages. Storage is allocated only
// in prepare(); processing and Auto feature collection never allocate or lock.
struct NativeParameter {const char* id;const char* name;float lo,hi,step,initial;bool boolean;};
inline constexpr NativeParameter nativeParameters[]={
 {"vocalSmooth","Smooth amount",0,100,1,20,false},{"harshControl","Harsh control",0,1,1,1,true},
 {"vocalWarmth","Vocal warmth",-100,100,1,0,false},{"vocalPresence","Presence",-100,100,1,0,false},
 {"deessAmount","De-Esser amount",0,100,1,25,false},{"deessHz","Sibilance frequency",3000,10000,100,6000,false},
 {"deessThreshold","De-Esser threshold",-48,-6,1,-28,false},{"denoiseAmount","Denoise amount",0,100,1,0,false},
 {"noiseThreshold","Noise threshold",-72,-12,1,-45,false},{"optoMode","Opto compressor",0,1,1,1,true},
 {"optoMix","Compressor mix",0,100,1,100,false},{"sidechainHz","Compressor sidechain",20,250,5,80,false},
 {"limitMode","Limit mode",0,1,1,0,true},{"outputSafety","Output safety",0,1,1,1,true},
 {"autoKey","Auto detect key",0,1,1,1,true},
 {"includeSmooth","Auto Vocal Smooth",0,1,1,1,true},{"includeDynamics","Auto Dynamics",0,1,1,1,true},
 {"includeDeess","Auto De-Esser",0,1,1,1,true},{"includeEq","Auto EQ",0,1,1,1,true},
 {"includePitch","Auto Pitch",0,1,1,1,true},{"includeReverb","Auto Reverb",0,1,1,1,true},
 {"includeDelay","Auto Delay",0,1,1,1,true}
};
inline constexpr int nativeParameterCount=int(sizeof(nativeParameters)/sizeof(nativeParameters[0]));
enum NativeIndex {Smooth,Harsh,Warmth,Presence,Deess,DeessHz,DeessThreshold,Denoise,NoiseThreshold,Opto,OptoMix,SidechainHz,Limit,Safety,AutoKey,IncludeSmooth,IncludeDynamics,IncludeDeess,IncludeEq,IncludePitch,IncludeReverb,IncludeDelay};
using NativeValues=std::array<float,nativeParameterCount>;
inline NativeValues nativeDefaults(){NativeValues a{};for(int i=0;i<nativeParameterCount;++i)a[size_t(i)]=nativeParameters[i].initial;return a;}
struct KeyEstimate {int root=0,scale=0;float score=0,margin=0;bool confident=false;};
inline KeyEstimate estimateKey(const std::array<double,12>& histogram) {
    constexpr double major[]={6.35,2.23,3.48,2.33,4.38,4.09,2.52,5.19,2.39,3.66,2.29,2.88};
    constexpr double minor[]={6.33,2.68,3.52,5.38,2.60,3.53,2.54,4.75,3.98,2.69,3.34,3.17};
    double total=0;int distinct=0;for(auto h:histogram){total+=h;if(h>.06)++distinct;}
    KeyEstimate result;if(total<1.5||distinct<3)return result;
    double mean=total/12,second=-2,best=-2;
    for(int mode=0;mode<2;++mode)for(int root=0;root<12;++root){const double* profile=mode?minor:major;double pm=0;for(int i=0;i<12;++i)pm+=profile[i]/12;double xy=0,xx=0,yy=0;for(int i=0;i<12;++i){double x=histogram[size_t(i)]-mean,y=profile[(i-root+12)%12]-pm;xy+=x*y;xx+=x*x;yy+=y*y;}double score=xy/std::sqrt(std::max(1.e-20,xx*yy));if(score>best){second=best;best=score;result.root=root;result.scale=mode?2:1;}else if(score>second)second=score;}
    result.score=float(best);result.margin=float(best-second);result.confident=best>=.5&&best-second>=.035;return result;
}
struct NativeProposal {std::array<float,parameterCount> core{};NativeValues extra{};std::array<bool,parameterCount> coreChanged{};std::array<bool,nativeParameterCount> extraChanged{};KeyEstimate key;bool valid=false;float rmsDb=-180,crest=0;};
class NativeAuto {
    double rate=48000,sum=0;float peak=0;std::array<double,8> low{};std::array<double,5> bands{};std::array<double,4> coefficients{};
    std::array<double,12> chroma{};std::array<int,91> levelHistogram{};double frameEnergy=0;int frameCount=0;double voiced=0;
public:
    bool running=false;int count=0,target=144000;
    void prepare(double sr){rate=sr;for(int i=0;i<4;++i){constexpr double frequencies[]={150,500,2500,6000};coefficients[size_t(i)]=1-std::exp(-2*pi*frequencies[i]/sr);}running=false;}
    void start(){sum=0;peak=0;count=0;target=int(rate*3);low.fill(0);bands.fill(0);chroma.fill(0);levelHistogram.fill(0);frameEnergy=0;frameCount=0;voiced=0;running=true;}
    float progress()const{return running?std::min(1.f,float(count)/float(target)):0;}
    void push(float l,float r){if(!running||count>=target)return;double e=(double(l)*l+double(r)*r)*.5;sum+=e;frameEnergy+=e;peak=std::max(peak,std::max(std::abs(l),std::abs(r)));++count;if(++frameCount>=int(rate*.01)){float level=db(float(std::sqrt(frameEnergy/frameCount)));++levelHistogram[size_t(std::clamp(int(std::round(level))+90,0,90))];frameEnergy=0;frameCount=0;}for(int c=0;c<2;++c){double x=c?r:l,previous=0;for(int b=0;b<4;++b){auto i=size_t(c*4+b);low[i]+=coefficients[size_t(b)]*(x-low[i]);double band=low[i]-previous;bands[size_t(b)]+=band*band*.5;previous=low[i];}bands[4]+=(x-previous)*(x-previous)*.5;}}
    void pitch(float hz,float confidence){if(!running||count>=target||hz<=0||confidence<.75f)return;double duration=128*std::ceil(rate/12000)/rate;voiced+=duration;int midi=int(std::round(69+12*std::log2(hz/440)));chroma[size_t((midi%12+12)%12)]+=duration;}
    bool ready()const{return running&&count>=target;}
    NativeProposal finish(const std::array<float,parameterCount>& original,const NativeValues& settings){running=false;NativeProposal out;out.core=original;out.extra=settings;out.rmsDb=db(float(std::sqrt(sum/std::max(1,count))));out.crest=db(peak)-out.rmsDb;out.valid=out.rmsDb>-50&&voiced>=.2;out.key=estimateKey(chroma);if(!out.valid)return out;
      double total=0;for(auto e:bands)total+=e;std::array<float,5> ratios{};for(int i=0;i<5;++i)ratios[size_t(i)]=float(bands[size_t(i)]/std::max(1.e-12,total));
      auto core=[&](int i,float v){out.core[size_t(i)]=v;out.coreChanged[size_t(i)]=true;};auto extra=[&](int i,float v){out.extra[size_t(i)]=v;out.extraChanged[size_t(i)]=true;};core(3,1);
      if(settings[IncludePitch]>.5f){core(2,0);core(4,out.crest>14?45.f:25.f);core(5,35);core(7,100);if(settings[AutoKey]>.5f&&int(original[1])==0&&out.key.confident){core(0,float(out.key.root));core(1,float(out.key.scale));}}
      if(settings[IncludeDynamics]>.5f){float threshold=std::clamp(out.rmsDb-5,-36.f,-10.f),gr=std::max(0.f,out.rmsDb-threshold)*.75f;core(8,threshold);core(9,4);core(10,10);core(11,180);core(12,std::clamp(std::min(-20-out.rmsDb+gr,-2-db(peak)+gr),0.f,9.f));extra(Opto,1);extra(OptoMix,100);extra(SidechainHz,80);extra(Limit,0);extra(Safety,1);}
      if(settings[IncludeEq]>.5f){constexpr float freq[]={90,300,1200,3500,10000},q[]={.7f,.9f,1,.8f,.7f};float gains[]={ratios[0]>.35f?-3.f:0.f,std::clamp((ratios[1]-.22f)*-10,-4.f,0.f),0,ratios[3]<.08f?2.f:ratios[3]>.4f?-2.f:0.f,ratios[4]<.04f?1.f:0.f};for(int i=0;i<5;++i){core(13+i*3,freq[i]);core(14+i*3,gains[i]);core(15+i*3,q[i]);}}
      if(settings[IncludeSmooth]>.5f){extra(Smooth,ratios[3]+ratios[4]>.3f?40.f:20.f);extra(Harsh,1);extra(Warmth,ratios[0]<.08f?8.f:0.f);extra(Presence,0);int totalFrames=0;for(auto h:levelHistogram)totalFrames+=h;int accumulated=0,floor=-90;for(int i=0;i<91;++i){accumulated+=levelHistogram[size_t(i)];if(accumulated>=std::max(1,totalFrames/10)){floor=i-90;break;}}bool quiet=floor<out.rmsDb-12&&floor>-78;extra(Denoise,quiet?30.f:0.f);extra(NoiseThreshold,std::clamp(float(floor+6),-60.f,-30.f));}
      if(settings[IncludeDeess]>.5f){extra(Deess,std::clamp(std::round(ratios[4]*180),15.f,55.f));extra(DeessHz,6000);extra(DeessThreshold,std::clamp(out.rmsDb-8,-40.f,-18.f));}if(settings[IncludeReverb]>.5f)core(31,.04f);if(settings[IncludeDelay]>.5f)core(28,.02f);return out;
    }
};
class NativeVocal {
    double rate=48000;float slew=0,attack=0,release=0,wet=0,envFull=0,envHigh=0,lp[2]{},smoothed[4]{},cut=0;int control=0;
    std::array<std::array<Biquad,3>,2> filters{};
    float gateEnv=0,gateGain=1,optoEnergy=0,optoSlow=0,optoReduction=0,optoLp[2]{},makeup=0,mix=1;
    std::array<std::array<float,128>,2> safetyDelay{};std::array<float,256> peaks{};std::array<int64_t,256> indices{};int64_t head=0,tail=0,n=0;float safetyGain=1;
    std::array<std::array<Biquad,2>,2> weighting{};std::vector<double> energy;size_t energyPos=0,energyCount=0;double energySum=0;bool mono=false;
    void configureWeighting(){for(auto& channel:weighting){double k=std::tan(pi*1681.974450955533/rate),q=.7071752369554196,vh=std::pow(10.,3.999843853973347/20),vb=std::pow(vh,.4996667741545416),a=1+k/q+k*k;auto& s=channel[0];s.b0=float((vh+vb*k/q+k*k)/a);s.b1=float(2*(k*k-vh)/a);s.b2=float((vh-vb*k/q+k*k)/a);s.a1=float(2*(k*k-1)/a);s.a2=float((1-k/q+k*k)/a);k=std::tan(pi*38.13547087602444/rate);q=.5003270373238773;a=1+k/q+k*k;auto& h=channel[1];h.b0=1;h.b1=-2;h.b2=1;h.a1=float(2*(k*k-1)/a);h.a2=float((1-k/q+k*k)/a);}}
public:
    float left=0,right=0,deessReduction=0,smoothReduction=0,compressReduction=0,guardReduction=0,heldPeak=0,leftPeak=0,rightPeak=0;
    void prepare(double sr,int channels=2){*this=NativeVocal{};mono=channels==1;rate=sr;slew=1-std::exp(-1/float(sr*.02));attack=1-std::exp(-1/float(sr*.002));release=1-std::exp(-1/float(sr*.08));energy.assign(size_t(sr*.4),0);configureWeighting();}
    void resetPeak(){heldPeak=0;}
    void beginBlock(){leftPeak=rightPeak=0;}
    void input(float l,float r,const Params& p,const NativeValues& x){l=clean(l);r=clean(r);bool on=p.power>.5f;float peak=std::max(std::abs(l),std::abs(r));gateEnv+=(peak>gateEnv?attack:1-std::exp(-1/float(rate*.12)))*(peak-gateEnv);float wanted=on?gain(std::max(-48.f,std::min(0.f,db(gateEnv)-x[NoiseThreshold])*2)*x[Denoise]/100):1;gateGain+=(wanted>gateGain?attack:1-std::exp(-1/float(rate*.12)))*(wanted-gateGain);float gate=x[Denoise]>0&&on?gateGain:1;l*=gate;r*=gate;
      float k=1-std::exp(-2*pi*x[SidechainHz]/float(rate));optoLp[0]+=k*(l-optoLp[0]);optoLp[1]+=k*(r-optoLp[1]);float a=l-optoLp[0],b=r-optoLp[1];optoEnergy+=(1-std::exp(-1/float(rate*.01)))*((a*a+b*b)*.5f-optoEnergy);bool opto=x[Opto]>.5f&&p.ratio>1&&on;float over=db(std::sqrt(optoEnergy))-p.threshold,slope=x[Limit]>.5f?.95f:.75f,desired=!opto||over<=-3?0:over>=3?over*slope:slope*(over+3)*(over+3)/12;optoSlow+=(1-std::exp(-1/float(rate*1.5)))*(desired-optoSlow);float target=std::max(desired,optoSlow*.55f);optoReduction+=(target>optoReduction?1-std::exp(-1/float(rate*.01)):release)*(target-optoReduction);makeup+=slew*(p.makeup-makeup);mix+=slew*(x[OptoMix]/100-mix);float comp=opto?((1-mix)+mix*gain(-optoReduction))*gain(makeup):1;compressReduction=opto?optoReduction:0;left=l*comp;right=r*comp;
    }
    void output(float l,float r,const Params& p,const NativeValues& x){bool on=p.power>.5f;wet+=slew*((on?1.f:0.f)-wet);const int ids[]={Smooth,Warmth,Presence,Deess};for(int i=0;i<4;++i)smoothed[i]+=slew*(x[size_t(ids[i])]-smoothed[i]);float lpCoef=1-std::exp(-2*pi*x[DeessHz]/float(rate));lp[0]+=lpCoef*(l-lp[0]);lp[1]+=lpCoef*(r-lp[1]);float hl=l-lp[0],hr=r-lp[1],full=(l*l+r*r)*.5f,high=(hl*hl+hr*hr)*.5f;envFull+=slew*(full-envFull);envHigh+=(high>envHigh?attack:release)*(high-envHigh);float dominance=envHigh/std::max(1.e-12f,envFull),wanted=dominance>.08f?std::clamp((db(std::sqrt(envHigh))-x[DeessThreshold])*.8f,0.f,18.f)*smoothed[3]/100:0;deessReduction+=(wanted>deessReduction?attack:release)*(wanted-deessReduction);float highGain=gain(-deessReduction),a=l-hl*(1-highGain),b=r-hr*(1-highGain);float wantedCut=-(x[Harsh]>.5f?9.f:4.f)*smoothed[0]/100*std::clamp(dominance*3,0.f,1.f);cut+=slew*(wantedCut-cut);smoothReduction=-cut*wet;
      if(control++%64==0)for(auto& channel:filters){channel[0].peak(rate,3300,.8f,cut);channel[1].peak(rate,180,.7f,smoothed[1]*.06f);channel[2].peak(rate,3200,.7f,smoothed[2]*.06f);}for(auto& f:filters[0])a=f.process(a);for(auto& f:filters[1])b=f.process(b);l+=wet*(a-l);r+=wet*(b-r);
      int pos=int(n%128);float dl=safetyDelay[0][size_t(pos)],dr=safetyDelay[1][size_t(pos)];safetyDelay[0][size_t(pos)]=clean(l);safetyDelay[1][size_t(pos)]=clean(r);float peak=std::max(std::abs(l),std::abs(r));while(head<tail&&indices[size_t(head%256)]<n-128)++head;while(tail>head&&peaks[size_t((tail-1)%256)]<=peak)--tail;peaks[size_t(tail%256)]=peak;indices[size_t(tail%256)]=n;++tail;++n;float maximum=peaks[size_t(head%256)],target=maximum>.95f?.95f/maximum:1;safetyGain=target<safetyGain?target:safetyGain+release*(target-safetyGain);bool guard=on&&x[Safety]>.5f;left=clean(dl*(guard?safetyGain:1));right=clean(dr*(guard?safetyGain:1));guardReduction=guard?-db(safetyGain):0;leftPeak=std::max(leftPeak,std::abs(left));rightPeak=std::max(rightPeak,std::abs(right));heldPeak=std::max(heldPeak,std::max(leftPeak,rightPeak));float kl=left,kr=right;for(auto& f:weighting[0])kl=f.process(kl);for(auto& f:weighting[1])kr=f.process(kr);double e=double(kl)*kl+(mono?0:double(kr)*kr);energySum+=e-energy[energyPos];energy[energyPos]=e;energyPos=(energyPos+1)%energy.size();energyCount=std::min(energy.size(),energyCount+1);
    }
    float lufs()const{return energyCount==energy.size()&&energySum>1.e-12?float(-.691+10*std::log10(energySum/double(energyCount))):-180.f;}
};
} // namespace matale
