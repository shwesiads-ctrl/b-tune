#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <vector>

namespace matale {
constexpr float pi = 3.14159265358979323846f;
inline float db(float x) { return 20.f * std::log10(std::max(1.e-9f, x)); }
inline float gain(float x) { return std::pow(10.f, x / 20.f); }
inline float clean(float x) { return std::isfinite(x) ? x : 0.f; }
inline float nearestNote(float hz, int root, int scale) {
    if (!(hz > 0)) return 0;
    constexpr int masks[] = {4095, 2741, 1453, 661, 1193};
    float m = 69.f + 12.f * std::log2(hz / 440.f), best = 1.e6f;
    int chosen = int(std::round(m));
    for (int n = int(std::floor(m)) - 12; n <= int(std::ceil(m)) + 12; ++n) {
        int pc = ((n - root) % 12 + 12) % 12;
        if ((masks[std::clamp(scale, 0, 4)] & (1 << pc)) && std::abs(n - m) < best) {
            best = std::abs(n - m); chosen = n;
        }
    }
    return 440.f * std::pow(2.f, (chosen - 69.f) / 12.f);
}

// Block-independent, anti-aliased decimated YIN/CMNDF. Buffers are fixed.
// Analysis rate <= 12 kHz; range 70..1000 Hz; ~64 ms history.
class Yin {
    std::array<float, 2048> history{};
    std::array<float, 256> difference{};
    int write=0, count=0, decimation=4, decCount=0, hop=0;
    float lp1=0, lp2=0, coefficient=.5f, rate=12000;
public:
    float hz=0, confidence=0;
    void prepare(double sr) {
        decimation=std::max(1, int(std::ceil(sr / 12000.0))); rate=float(sr / decimation);
        coefficient=1.f-std::exp(-2.f*pi*2000.f/float(sr));
        history.fill(0); difference.fill(0); write=count=decCount=hop=0; lp1=lp2=hz=confidence=0;
    }
    bool push(float x) {
        lp1+=coefficient*(x-lp1); lp2+=coefficient*(lp1-lp2);
        if (++decCount < decimation) return false;
        decCount=0; history[write]=lp2; write=(write+1)%2048; count=std::min(1024,count+1);
        if (++hop<128 || count<1024) return false;
        hop=0; detect(); return true;
    }
private:
    float at(int back) const { return history[(write-1-back+4096)%2048]; }
    void detect() {
        const int lo=std::max(2,int(rate/1000)), hi=std::min(254,int(rate/70));
        double energy=0;
        for (int i=0;i<512;++i) energy+=double(at(i))*at(i);
        hz=confidence=0;
        if (energy/512 < 1.e-7) return;
        difference[0]=1; double sum=0;
        for (int lag=1;lag<=hi;++lag) {
            double d=0;
            for (int i=0;i<512;++i) { double v=at(i)-at(i+lag); d+=v*v; }
            sum+=d; difference[lag]=sum>1.e-20 ? float(d*lag/sum) : 1.f;
        }
        for (int lag=lo;lag<hi-1;++lag) {
            if (difference[lag] < .15f) {
                while (lag+1<hi && difference[lag+1]<difference[lag]) ++lag;
                const float a=difference[lag-1], b=difference[lag], c=difference[lag+1];
                const float denominator=a-2*b+c;
                float offset=std::abs(denominator)>1.e-9f ? .5f*(a-c)/denominator : 0;
                hz=rate/(lag+std::clamp(offset,-.5f,.5f)); confidence=1-b; return;
            }
        }
    }
};

struct Biquad {
    float b0=1,b1=0,b2=0,a1=0,a2=0,z1=0,z2=0;
    void reset() { z1=z2=0; }
    void peak(double sr,float f,float q,float dB) {
        float A=std::pow(10.f,dB/40.f), w=2*pi*std::clamp(f,20.f,float(sr*.45))/float(sr);
        float alpha=std::sin(w)/(2*std::clamp(q,.2f,10.f)), c=std::cos(w), a0=1+alpha/A;
        b0=(1+alpha*A)/a0; b1=-2*c/a0; b2=(1-alpha*A)/a0;
        a1=-2*c/a0; a2=(1-alpha/A)/a0;
    }
    float process(float x) {
        float y=b0*x+z1; z1=b1*x-a1*y+z2; z2=b2*x-a2*y;
        if (std::abs(z1)<1.e-25f) z1=0;
        if (std::abs(z2)<1.e-25f) z2=0;
        return y;
    }
};

// Two overlapping resampling grains with normalized correlation alignment
// at grain launches (SOLA-style). Not pitch-synchronous PSOLA.
// Fixed nominal latency 2048 frames. Residual time modulation is inherent.
class Sola {
    static constexpr int size=16384, grain=2048, latency=2048;
    std::array<float,size> ring{};
    struct Head { double position=0; int age=0; };
    Head a,b;
    int write=0;
    float read(double p) const {
        p-=std::floor(p/size)*size;
        int i=int(p); float f=float(p-i);
        return ring[i]+f*(ring[(i+1)%size]-ring[i]);
    }
    void launch(Head& head,const Head& other,float ratio) {
        const double anchor=write-latency-grain*.5*(ratio-1);
        int bestOffset=0; double best=-2;
        // Bounded search, every 1024 samples; no heap allocation.
        for(int offset=-96;offset<=96;offset+=4) {
            double xy=0,xx=1.e-15,yy=1.e-15;
            for(int j=0;j<64;j+=4) {
                float x=read(other.position+j*ratio), y=read(anchor+offset+j*ratio);
                xy+=x*y; xx+=x*x; yy+=y*y;
            }
            double score=xy/std::sqrt(xx*yy)-.0001*std::abs(offset);
            if(score>best) {best=score; bestOffset=offset;}
        }
        head.position=anchor+bestOffset; head.age=0;
    }
public:
    void reset() { ring.fill(0); write=0; a={-latency,0}; b={-latency,grain/2}; }
    float process(float x,float ratio) {
        ring[write]=x;
        if(a.age>=grain) launch(a,b,ratio);
        if(b.age>=grain) launch(b,a,ratio);
        float wa=.5f-.5f*std::cos(2*pi*a.age/grain);
        float wb=.5f-.5f*std::cos(2*pi*b.age/grain);
        float y=(wa*read(a.position)+wb*read(b.position))/std::max(.001f,wa+wb);
        a.position+=ratio; b.position+=ratio; ++a.age; ++b.age;
        write=(write+1)%size;
        // Keep double read coordinates bounded across long sessions.
        if(a.position>size*2) a.position-=size;
        if(b.position>size*2) b.position-=size;
        return y;
    }
};

struct Params {
    int key=0,scale=0,style=0;
    float power=1,retune=35,humanize=20,formant=0,correction=100;
    float threshold=-18,ratio=3,attack=10,release=120,makeup=0;
    std::array<float,5> frequency{100,300,1200,3500,10000}, eqGain{0,0,0,0,0}, q{.7f,1,1,1,.7f};
    float delayMix=0,delayBeats=.5f,feedback=.25f,reverbMix=0,room=.45f,output=0,bpm=120;
};

struct AutoResult { float threshold=-18,makeup=0,retune=35,humanize=40,low=0,presence=0,delayMix=.07f,reverbMix=.1f; bool valid=false; };

// Single audio-thread ownership. Publish results via atomics in the adapter.
class AutoAnalyser {
    double sum=0,high=0;
    float peak=0,previous=0;
    int count=0,target=144000,voiced=0,observations=0;
public:
    bool running=false;
    void start(double rate) {sum=high=0;peak=previous=0;count=voiced=observations=0;target=int(rate*3);running=true;}
    bool push(float x,bool observed,float hz,AutoResult& result) {
        if(!running) return false;
        sum+=double(x)*x; high+=double(x-previous)*(x-previous); previous=x;peak=std::max(peak,std::abs(x));
        if(observed) {++observations;if(hz>0)++voiced;}
        if(++count<target) return false;
        running=false;result={};
        float rms=float(std::sqrt(sum/count));
        result.valid=db(rms)>-50 && voiced>=3;
        if(!result.valid) return true;
        result.threshold=std::clamp(db(rms)-4.f,-36.f,-8.f);
        result.makeup=std::clamp(-20.f-db(rms),0.f,9.f);
        float crest=db(peak)-db(rms);
        result.retune=crest>12?55:35;result.humanize=crest>12?65:40;
        result.low=high/(sum+1.e-12)<.06 ? -2.f:0;
        result.presence=high/(sum+1.e-12)<.12 ? 1.5f:0;
        return true;
    }
    float progress() const {return running?float(count)/target:0;}
};

class Engine {
    double sr=48000;
    Yin yin;
    std::array<Sola,2> shifters;
    std::array<std::array<Biquad,5>,2> eq;
    std::array<std::array<Biquad,2>,2> formants;
    std::array<std::vector<float>,2> delay;
    std::array<std::array<std::vector<float>,4>,2> comb;
    std::array<std::array<int,4>,2> combPos{};
    std::array<std::array<float,4>,2> damping{};
    std::array<std::array<float,2048>,2> dry{};
    int dryPos=0,delayPos=0,control=0;
    float envelope=0,reduction=0,ratio=1,from=1,target=1;
    int ramp=0,rampLength=1;
    float bypassMix=1,tuneMix=0,delayFrames=24000;
    Params smoothed;
    AutoAnalyser analyser;
public:
    static constexpr int latency=2048;
    float inputPeak=0,outputPeak=0;
    AutoResult autoResult;
    bool autoReady=false;
    float pitch() const {return yin.hz;}
    float autoProgress() const {return analyser.progress();}
    bool analysing() const {return analyser.running;}
    void beginAuto() {analyser.start(sr);autoReady=false;}
    void prepare(double rate) {
        sr=rate;yin.prepare(sr);
        for(auto& s:shifters)s.reset();
        for(auto& c:eq)for(auto& b:c)b.reset();
        for(auto& c:formants)for(auto& b:c)b.reset();
        constexpr int lengths[]={1493,1601,1747,1867};
        for(int c=0;c<2;++c) {
            delay[c].assign(size_t(sr*4+1),0);
            for(int j=0;j<4;++j) comb[c][j].assign(size_t((lengths[j]+c*37)*sr/44100),0);
            dry[c].fill(0);neutralDelay[c].fill(0);combPos[c].fill(0);damping[c].fill(0);
        }
        dryPos=delayPos=control=0;envelope=reduction=0;ratio=from=target=1;ramp=0;rampLength=1;
        bypassMix=1;tuneMix=0;delayFrames=float(sr*.25);smoothed={};
        smoothedDelay.fill(0);smoothedReverb.fill(0);smoothedOutput.fill(0);
        analyser={}; autoResult={}; autoReady=false;
    }
    void process(float* left,float* right,int frames,const Params& p) {
        inputPeak=outputPeak=0;
        for(int n=0;n<frames;++n) {
            float l=clean(left[n]),r=right?clean(right[n]):l;
            inputPeak=std::max(inputPeak,std::max(std::abs(l),std::abs(r)));
            bool observed=yin.push((l+r)*.5f);
            if(analyser.push((l+r)*.5f,observed,yin.hz,autoResult))autoReady=true;
            if(observed) {
                float wanted=yin.hz>0?nearestNote(yin.hz,p.key,p.scale)/yin.hz:1;
                wanted=std::pow(std::clamp(wanted,.5f,2.f),std::clamp(p.correction/100.f,0.f,1.f));
                if(std::abs(wanted-target)>.0003f) {
                    from=ratio;target=wanted;ramp=0;
                    rampLength=std::max(1,int(sr*.001*(p.style?1:p.retune*(1+p.humanize*.02f))));
                }
            }
            float t=std::min(1.f,float(++ramp)/rampLength); ratio=from+(target-from)*(t*t*(3-2*t));
            if(ramp>rampLength)ramp=rampLength;
            const float smoothing=1.f-std::exp(-1.f/float(sr*.02));
            bypassMix+=smoothing*(std::clamp(p.power,0.f,1.f)-bypassMix);
            float tuneTarget=(yin.hz>0 && p.correction>.01f && std::abs(target-1)>.0001f)?1.f:0.f;
            tuneMix+=smoothing*(tuneTarget-tuneMix);
            if(control++%64==0) {
                float k=1.f-std::exp(-64.f/float(sr*.025));
                for(int j=0;j<5;++j) {
                    smoothed.frequency[j]+=k*(p.frequency[j]-smoothed.frequency[j]);
                    smoothed.eqGain[j]+=k*(p.eqGain[j]-smoothed.eqGain[j]);
                    smoothed.q[j]+=k*(p.q[j]-smoothed.q[j]);
                    for(int c=0;c<2;++c) eq[c][j].peak(sr,smoothed.frequency[j],smoothed.q[j],smoothed.eqGain[j]);
                }
                smoothed.formant+=k*(p.formant-smoothed.formant);
                for(int c=0;c<2;++c) {
                    formants[c][0].peak(sr,700, .65f,-smoothed.formant*.5f);
                    formants[c][1].peak(sr,2400,.65f, smoothed.formant*.5f);
                }
                control=1;
            }
            float level=std::max(std::abs(l),std::abs(r));
            float envCoeff=1.f-std::exp(-1.f/float(sr*.001*(level>envelope?std::max(.1f,p.attack):std::max(10.f,p.release))));
            envelope+=envCoeff*(level-envelope);
            float over=db(envelope)-p.threshold, knee=6, gr=0, slope=1.f/std::max(1.f,p.ratio)-1.f;
            if(over>knee/2)gr=slope*over;
            else if(over>-knee/2)gr=slope*(over+knee/2)*(over+knee/2)/(2*knee);
            reduction+=smoothing*(gr-reduction);
            float compGain=gain(reduction+p.makeup);
            float wantedDelay=float(sr*60/std::clamp(p.bpm,30.f,300.f)*std::clamp(p.delayBeats,.125f,2.f));
            delayFrames+=smoothing*(std::clamp(wantedDelay,1.f,float(delay[0].size()-2))-delayFrames);
            for(int c=0;c<2;++c) {
                float raw=c?r:l;
                float dryOut=dry[c][dryPos];dry[c][dryPos]=raw;
                float x=raw*compGain;
                for(auto& filter:eq[c])x=filter.process(x);
                float shifted=shifters[c].process(x,ratio);
                // Delay the processed neutral path independently; use shifter
                // at unity too (nominal latency) only while voiced correction is active.
                // neutral history is stored separately below.
                float neutral=neutralDelay[c][dryPos];neutralDelay[c][dryPos]=x;
                x=neutral+(shifted-neutral)*tuneMix;
                for(auto& filter:formants[c])x=filter.process(x);
                float readPos=delayPos-delayFrames;
                if(readPos<0)readPos+=float(delay[c].size());
                int i=int(readPos),j=(i+1)%int(delay[c].size());
                float echo=delay[c][i]+(delay[c][j]-delay[c][i])*(readPos-i);
                delay[c][delayPos]=x+echo*std::clamp(p.feedback,0.f,.85f);
                smoothedDelay[c]+=smoothing*(std::clamp(p.delayMix,0.f,.6f)-smoothedDelay[c]);
                x+=echo*smoothedDelay[c];
                float rev=0;
                for(int k=0;k<4;++k) {
                    auto& buf=comb[c][k];int& pos=combPos[c][k];
                    float y=buf[pos];damping[c][k]+=.3f*(y-damping[c][k]);
                    buf[pos]=x*.2f+damping[c][k]*(.55f+.35f*std::clamp(p.room,0.f,1.f));
                    if(++pos==int(buf.size()))pos=0;
                    rev+=y*.25f;
                }
                smoothedReverb[c]+=smoothing*(std::clamp(p.reverbMix,0.f,.6f)-smoothedReverb[c]);
                x+=rev*smoothedReverb[c];
                smoothedOutput[c]+=smoothing*(p.output-smoothedOutput[c]);
                x*=gain(smoothedOutput[c]);
                // Unity below -1 dB; soft saturation of peaks, asymptote +/-1.
                if(std::abs(x)>.891f)x=std::copysign(.891f+.109f*std::tanh((std::abs(x)-.891f)/.109f),x);
                x=clean(dryOut+(x-dryOut)*bypassMix);
                if(c==0)left[n]=x;else if(right)right[n]=x;
                outputPeak=std::max(outputPeak,std::abs(x));
            }
            dryPos=(dryPos+1)%latency;
            if(++delayPos==int(delay[0].size()))delayPos=0;
        }
    }
private:
    std::array<std::array<float,2048>,2> neutralDelay{};
    std::array<float,2> smoothedDelay{},smoothedReverb{},smoothedOutput{};
};
} // namespace matale
