#ifndef ENVPLOTPROCESSOR_H
#define ENVPLOTPROCESSOR_H

#include <iostream>
#include <vector>

#include "TriggerContainer.h"
#include "eegContainer.h"
#include "ProvFile.h"
#include "optionsParameters.h"

namespace InsermLibrary
{
    class EnvplotProcessor
    {
    public:
        void Process(TriggerContainer* triggerContainer, eegContainer* myeegContainer, ProvFile* myprovFile, std::string freqFolder, picOption* picOption);

    private :        
        std::string GetEnv2PlotMapsFolder(std::string freqFolder, ProvFile* myprovFile);
        std::string PrepareFolderAndPathsPlot(std::string mapsFolder, eegContainer* myeegContainer);
    };
}

#endif
