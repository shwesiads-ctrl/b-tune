#include "Parameters.h"
#include <memory>
// Emscripten standalone reactor ABI. Init allocates before audio processing.
namespace {
std::unique_ptr<matale::Engine> engine;
matale::Params parameters;
float left[128]{},right[128]{};
}
extern "C" {
void mt_init(double rate) {engine=std::make_unique<matale::Engine>();engine->prepare(rate);parameters={};}
float* mt_left() {return left;}
float* mt_right() {return right;}
void mt_set(int index,float value) {matale::setParam(parameters,index,value);}
void mt_process() {if(engine)engine->process(left,right,128,parameters);}
float mt_pitch(){return engine?engine->pitch():0;}
float mt_input(){return engine?engine->inputPeak:0;}
float mt_output(){return engine?engine->outputPeak:0;}
void mt_auto(){if(engine)engine->beginAuto();}
float mt_progress(){return engine?engine->autoProgress():0;}
int mt_auto_ready(){return engine&&engine->autoReady?(engine->autoResult.valid?1:-1):0;}
float mt_auto_value(int i){
    if(!engine)return 0;
    auto& a=engine->autoResult;
    switch(i){case 8:return a.threshold;case 12:return a.makeup;case 4:return a.retune;case 5:return a.humanize;
    case 17:return a.low;case 23:return a.presence;case 28:return a.delayMix;case 31:return a.reverbMix;default:return 0;}
}
void mt_auto_ack(){if(engine)engine->autoReady=false;}
}
