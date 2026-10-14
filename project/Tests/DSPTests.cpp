#include "Parameters.h"
#include <atomic>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <new>
#include <random>
#include <stdexcept>
#include <string>
std::atomic<bool> watch{false};
std::atomic<int> allocations{0};
void* operator new(std::size_t n) {if(watch)++allocations;if(void* p=std::malloc(n?n:1))return p;throw std::bad_alloc();}
void* operator new[](std::size_t n){return ::operator new(n);}
void operator delete(void* p) noexcept {std::free(p);}
void operator delete[](void* p) noexcept {std::free(p);}
void operator delete(void* p,std::size_t) noexcept {std::free(p);}
void operator delete[](void* p,std::size_t) noexcept {std::free(p);}
void check(bool condition,const char* message){if(!condition)throw std::runtime_error(message);}
float sine(int n,double sr,float hz,float amp=.2f){return amp*std::sin(2*matale::pi*float(n/sr)*hz);}
void yinTest() {
    for(double sr:{44100.,48000.,96000.})for(float f:{80.f,130.8128f,220.f,440.f,880.f}){
        matale::Yin y;y.prepare(sr);
        for(int n=0;n<int(sr*.25);++n)y.push(sine(n,sr,f));
        check(std::abs(1200*std::log2(y.hz/f))<8,"YIN exceeds 8 cents");
    }
    matale::Yin y;y.prepare(48000);
    for(int n=0;n<12000;++n)y.push(0);
    check(y.hz==0,"Silence voiced");
    std::mt19937 gen(123);std::uniform_real_distribution<float>d(-.3f,.3f);
    int detected=0;
    for(int n=0;n<24000;++n)if(y.push(d(gen))&&y.hz>0)++detected;
    check(detected<3,"Noise incorrectly voiced");
}
void scaleTest() {
    check(std::abs(matale::nearestNote(445,9,1)-440)<.01f,"A major mapping");
    check(std::abs(matale::nearestNote(261.6256f,0,1)-261.6256f)<.01f,"C major mapping");
    check(matale::nearestNote(0,0,0)==0,"Silence quantization");
    check(std::abs(matale::nearestNote(277,0,1)-261.6256f)<.1f,"Excluded scale tone");
    for(int key=0;key<12;++key)for(int sc=0;sc<5;++sc)for(float f=70;f<1000;f+=7)
        check(std::isfinite(matale::nearestNote(f,key,sc)),"Mapping nonfinite");
}
void shiftTest() {
    for(float ratio:{.85f,1.122462f,1.25f}) {
        matale::Sola s;s.reset();matale::Yin y;y.prepare(48000);
        for(int n=0;n<48000;++n) y.push(s.process(sine(n,48000,220),ratio));
        float error=std::abs(1200*std::log2(y.hz/(220*ratio)));
        std::cout<<"ratio "<<ratio<<" measured "<<y.hz<<" cents "<<error<<"\n";
        check(error<30,"SOLA shifted frequency outside 30 cents");
    }
}
double renderRms(matale::Params p) {
    auto e=std::make_unique<matale::Engine>();e->prepare(48000);
    double sum=0;
    for(int n=0;n<48000;++n) {float x=sine(n,48000,440,.5f);e->process(&x,nullptr,1,p);if(n>24000)sum+=x*x;}
    return std::sqrt(sum/23999);
}
void dynamicsTest(){
    matale::Params p;p.correction=0;p.ratio=1;
    double dry=renderRms(p);p.ratio=8;p.threshold=-30;
    check(renderRms(p)<dry*.5,"Compressor does not reduce signal");
}
void effectsTest(){
    matale::Params p;p.ratio=1;p.correction=0;
    double flat=renderRms(p);p.frequency[2]=440;p.eqGain[2]=-12;
    check(renderRms(p)<flat*.5,"EQ ineffective");
    p={};p.ratio=1;p.correction=0;p.delayMix=.5;p.feedback=.4;p.reverbMix=.4;
    auto e=std::make_unique<matale::Engine>();e->prepare(48000);
    double tail=0;
    for(int n=0;n<48000;++n){float x=n==0?.5f:0;e->process(&x,nullptr,1,p);if(n>10000)tail+=x*x;}
    check(tail>1.e-5,"No effects tail");
    p={};p.ratio=1;p.correction=0;p.formant=12;
    check(std::abs(renderRms(p)-flat)>.01,"Formant colour ineffective");
}
void autoTest(){
    matale::AutoAnalyser a;matale::AutoResult r;a.start(48000);
    for(int n=0;n<144000;++n)a.push(sine(n,48000,220),n%512==0,220,r);
    check(r.valid&&r.threshold<-15,"Auto voice analysis");
    a.start(48000);
    for(int n=0;n<144000;++n)a.push(0,n%512==0,0,r);
    check(!r.valid,"Auto silence rejected");
}
void stressTest(){
    for(double sr:{44100.,48000.,96000.,192000.}){
        auto e=std::make_unique<matale::Engine>();e->prepare(sr);
        matale::Params p;std::array<float,4096> l{},r{};
        for(int block:{1,7,64,127,256,511,1024,4096}) {
            for(int n=0;n<block;++n){l[n]=sine(n,sr,220);r[n]=l[n];}
            e->process(l.data(),r.data(),block,p);
            for(int n=0;n<block;++n)check(std::isfinite(l[n])&&std::isfinite(r[n])&&std::abs(l[n])<=1.001,"Invalid output");
        }
        // Prepare reset must erase signal/effect histories.
        e->prepare(sr);l.fill(0);r.fill(0);e->process(l.data(),r.data(),4096,p);
        for(float x:l)check(x==0,"Prepare did not reset");
    }
    // Full chain corrects a sharp A4 to chromatic A4.
    {
        auto e=std::make_unique<matale::Engine>();e->prepare(48000);
        matale::Params p;p.ratio=1;p.humanize=0;
        matale::Yin output;output.prepare(48000);
        for(int n=0;n<96000;++n){float x=sine(n,48000,445);e->process(&x,nullptr,1,p);output.push(x);}
        check(std::abs(1200*std::log2(output.hz/440))<20,"Full chain correction inaccurate");
    }
    // Unity and bypass are both delayed by exactly 2048 frames.
    for(bool bypass:{false,true}){
        auto e=std::make_unique<matale::Engine>();e->prepare(48000);
        matale::Params p;p.ratio=1;p.correction=0;p.power=bypass?0:1;
        for(int n=0;n<6000;++n){float x=n==0?.1f:0;e->process(&x,nullptr,1,p);
            if(n==2048)check(std::abs(x-.1f)<1.e-5,"Latency incorrect");
            else check(std::abs(x)<1.e-6,"Unexpected neutral output");}
    }
}
void realtimeTest(){
    auto e=std::make_unique<matale::Engine>();e->prepare(48000);e->beginAuto();
    matale::Params p;std::array<float,256> l{},r{};
    watch=true;
    for(int b=0;b<600;++b){
        p.formant=float(b%24)-12;p.eqGain[2]=float(b%12)-6;
        for(int n=0;n<256;++n)l[n]=r[n]=sine(b*256+n,48000,220);
        e->process(l.data(),r.data(),256,p);
    }
    watch=false;check(allocations==0,"Audio processing allocated");
}
int main(int argc,char** argv){
    try{
        std::string t=argc>1?argv[1]:"";
        if(t=="yin")yinTest();else if(t=="scale")scaleTest();else if(t=="shift")shiftTest();
        else if(t=="dynamics")dynamicsTest();else if(t=="effects")effectsTest();else if(t=="auto")autoTest();
        else if(t=="stress")stressTest();else if(t=="realtime")realtimeTest();else throw std::runtime_error("Unknown test");
        std::cout<<"PASS "<<t<<"\n";return 0;
    }catch(const std::exception& e){watch=false;std::cerr<<"FAIL "<<e.what()<<"\n";return 1;}
}
