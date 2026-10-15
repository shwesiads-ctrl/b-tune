#include "NativeVocal.h"
#include <iostream>
#include <stdexcept>
#include <atomic>
#include <cstdlib>
#include <new>
static std::atomic<bool> watch{false};static std::atomic<int> allocations{0};
void* operator new(std::size_t n){if(watch)++allocations;if(auto* p=std::malloc(n?n:1))return p;throw std::bad_alloc();}
void* operator new[](std::size_t n){return ::operator new(n);}void operator delete(void* p)noexcept{std::free(p);}void operator delete[](void* p)noexcept{std::free(p);}void operator delete(void* p,std::size_t)noexcept{std::free(p);}void operator delete[](void* p,std::size_t)noexcept{std::free(p);}
static void check(bool ok,const char* message){if(!ok)throw std::runtime_error(message);}
int main(){try{
 for(double sr:{44100.,48000.,96000.}){matale::NativeVocal fx;fx.prepare(sr);auto x=matale::nativeDefaults();matale::Params p;p.ratio=1;x[matale::Smooth]=x[matale::Deess]=0;watch=true;float peak=0;for(int i=0;i<int(sr);++i){float l=i%997==0?3.f:.2f*std::sin(2*matale::pi*440*float(i/sr));fx.input(l,l*.5f,p,x);fx.output(fx.left,fx.right,p,x);peak=std::max(peak,std::abs(fx.left));check(std::abs(fx.right-fx.left*.5f)<1.e-6f,"Safety stereo image changed");}watch=false;check(allocations==0,"Audio path allocates");check(peak<=.950001f,"Safety ceiling exceeded");
  fx.prepare(sr);p.power=0;for(int i=0;i<400;++i){fx.input(.2f,-.1f,p,x);fx.output(fx.left,fx.right,p,x);}check(std::abs(fx.left-.2f)<1.e-7&&std::abs(fx.right+.1f)<1.e-7,"Native bypass not aligned dry");
  fx.prepare(sr);p.power=1;for(int i=0;i<int(sr);++i){float tone=.1f*std::sin(2*matale::pi*1000*float(i/sr));fx.input(tone,tone,p,x);fx.output(fx.left,fx.right,p,x);}check(std::abs(fx.lufs()+20)<.2f,"Stereo K-weighted calibration mismatch");
  fx.prepare(sr,1);for(int i=0;i<int(sr);++i){float tone=.1f*std::sin(2*matale::pi*1000*float(i/sr));fx.input(tone,tone,p,x);fx.output(fx.left,fx.right,p,x);}check(std::abs(fx.lufs()+23.01f)<.2f,"Mono K-weighted calibration mismatch");
  matale::NativeAuto analyser;analyser.prepare(sr);analyser.start();std::array<float,matale::parameterCount> base{};for(int i=0;i<matale::parameterCount;++i)base[size_t(i)]=matale::parameterList[i].initial;
  watch=true;for(int i=0;i<int(sr*3);++i){float v=.1f*std::sin(2*matale::pi*220*float(i/sr));analyser.push(v,v);if(i%(128*int(std::ceil(sr/12000)))==0)analyser.pitch(i<sr*1.5?261.6256f:i<sr*2.4?329.6276f:391.9954f,1);}auto proposal=analyser.finish(base,x);watch=false;check(allocations==0,"Auto allocates on audio thread");check(proposal.valid,"Auto rejected voiced input");check(proposal.key.confident&&proposal.key.root==0&&proposal.key.scale==1,"Auto key mismatch");check(proposal.core[7]==100&&proposal.core[1]==1,"Auto correction/key not enabled");
  for(int character=0;character<5;++character){auto adjusted=proposal;matale::applyCharacter(adjusted,character);check(adjusted.valid&&adjusted.key.confident,"Character damaged analysis");for(int i=0;i<matale::parameterCount;++i)check(adjusted.core[size_t(i)]>=matale::parameterList[i].lo&&adjusted.core[size_t(i)]<=matale::parameterList[i].hi,"Character exceeded parameter range");if(character==4)check(adjusted.core[2]==1&&adjusted.core[4]==5&&adjusted.core[5]==0,"Hard Tune character not applied");}
  auto excluded=proposal;excluded.coreChanged.fill(false);excluded.extraChanged.fill(false);matale::applyCharacter(excluded,4);check(excluded.core==proposal.core&&excluded.extra==proposal.extra,"Character changed excluded groups");
  analyser.start();for(int i=0;i<int(sr*3);++i)analyser.push(0,0);check(!analyser.finish(base,x).valid,"Silence must not produce Auto settings");
  std::cout<<"PASS native stereo safety, aligned bypass, zero allocations, Auto/key and silence at "<<sr<<" Hz\n";
 }
}catch(const std::exception& e){watch=false;std::cerr<<e.what()<<'\n';return 1;}}
