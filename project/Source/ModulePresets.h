#pragma once
#include "Parameters.h"
#include <vector>
#include <utility>
namespace matale {
struct ModulePreset { const char* name; std::vector<std::pair<const char*,float>> settings; };
inline const std::vector<ModulePreset>& modulePresets(int page){
    static const std::vector<ModulePreset> eq=[] {
        std::vector<ModulePreset> recipes;
        auto add=[&](const char* name,std::array<float,5> gains){
            ModulePreset recipe{name,{}};
            for(int band=0;band<5;++band)for(int component=0;component<3;++component){int i=13+band*3+component;recipe.settings.emplace_back(parameterList[i].id,component==1?gains[size_t(band)]:parameterList[i].initial);}
            recipes.push_back(std::move(recipe));
        };
        add("Flat",{0,0,0,0,0});add("Clean",{-3,-2.5f,-.5f,1.5f,1});
        add("Warm",{1.5f,1,-.5f,0,-1});add("Presence",{-1,-1,0,2.5f,1});add("Air",{-1,-1,0,1,3});return recipes;
    }();
    static const std::vector<ModulePreset> compressor={
        {"Gentle Opto",{{"threshold",-14},{"ratio",4},{"attack",10},{"release",180},{"makeup",1},{"optoMode",1},{"optoMix",70},{"sidechainHz",100},{"limitMode",0}}},
        {"Vocal Level",{{"threshold",-22},{"ratio",4},{"attack",10},{"release",180},{"makeup",3},{"optoMode",1},{"optoMix",100},{"sidechainHz",80},{"limitMode",0}}},
        {"Punch",{{"threshold",-18},{"ratio",3},{"attack",25},{"release",100},{"makeup",2},{"optoMode",0},{"optoMix",100},{"sidechainHz",80},{"limitMode",0}}},
        {"Firm Limit",{{"threshold",-16},{"ratio",12},{"attack",2},{"release",80},{"makeup",1},{"optoMode",1},{"optoMix",100},{"sidechainHz",100},{"limitMode",1}}},
        {"Parallel",{{"threshold",-28},{"ratio",4},{"attack",10},{"release",180},{"makeup",2},{"optoMode",1},{"optoMix",40},{"sidechainHz",100},{"limitMode",0}}}
    };
    static const std::vector<ModulePreset> space={
        {"Dry",{{"delayMix",0},{"delayBeats",.5f},{"feedback",.25f},{"reverbMix",0},{"room",.45f}}},
        {"Small Room",{{"delayMix",0},{"delayBeats",.25f},{"feedback",.15f},{"reverbMix",.08f},{"room",.25f}}},
        {"Vocal Hall",{{"delayMix",.03f},{"delayBeats",.25f},{"feedback",.2f},{"reverbMix",.16f},{"room",.6f}}},
        {"Slapback",{{"delayMix",.12f},{"delayBeats",.25f},{"feedback",.08f},{"reverbMix",.03f},{"room",.25f}}},
        {"Quarter Delay",{{"delayMix",.14f},{"delayBeats",1},{"feedback",.35f},{"reverbMix",.05f},{"room",.45f}}}
    };
    static const std::vector<ModulePreset> cleanup={
        {"Off",{{"vocalSmooth",0},{"harshControl",0},{"vocalWarmth",0},{"vocalPresence",0},{"deessAmount",0},{"deessHz",6000},{"deessThreshold",-28},{"denoiseAmount",0},{"noiseThreshold",-45}}},
        {"Gentle",{{"vocalSmooth",20},{"harshControl",1},{"vocalWarmth",5},{"vocalPresence",0},{"deessAmount",20},{"deessHz",6500},{"deessThreshold",-28},{"denoiseAmount",15},{"noiseThreshold",-52}}},
        {"Clean Vocal",{{"vocalSmooth",40},{"harshControl",1},{"vocalWarmth",5},{"vocalPresence",10},{"deessAmount",40},{"deessHz",6000},{"deessThreshold",-30},{"denoiseAmount",30},{"noiseThreshold",-46}}},
        {"Strong Cleanup",{{"vocalSmooth",60},{"harshControl",1},{"vocalWarmth",0},{"vocalPresence",5},{"deessAmount",60},{"deessHz",5500},{"deessThreshold",-34},{"denoiseAmount",55},{"noiseThreshold",-42}}},
        {"Warm Smooth",{{"vocalSmooth",45},{"harshControl",1},{"vocalWarmth",20},{"vocalPresence",-5},{"deessAmount",30},{"deessHz",6000},{"deessThreshold",-28},{"denoiseAmount",20},{"noiseThreshold",-50}}}
    };
    static const std::vector<ModulePreset> empty;
    switch(page){case 1:return compressor;case 2:return eq;case 3:return space;case 4:return cleanup;default:return empty;}
}
}
