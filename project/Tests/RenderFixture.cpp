#include "Parameters.h"
#include <cstdio>
int main(){
    matale::Engine e;e.prepare(48000);matale::Params p;p.ratio=1;
    for(int b=0;b<400;++b){float l[128],r[128];
        for(int i=0;i<128;++i)l[i]=r[i]=.2f*std::sin(2*matale::pi*445.f*float(b*128+i)/48000.f);
        std::fwrite(l,sizeof(float),128,stdout);e.process(l,r,128,p);std::fwrite(l,sizeof(float),128,stdout);
    }
}
