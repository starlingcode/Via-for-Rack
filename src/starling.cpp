#include "starling.hpp"


Plugin *pluginInstance;

void init(rack::Plugin *p) {
	
	pluginInstance = p;

    p->addModel(modelMeta);
    p->addModel(modelGateseq);
    p->addModel(modelScanner);
    p->addModel(modelSync);
    p->addModel(modelAtsr);
    p->addModel(modelOsc3);
    p->addModel(modelSync3);
    p->addModel(modelSync3XL);

}
