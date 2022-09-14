#ifndef STATISTICALFILESPROCESSOR_H
#define STATISTICALFILESPROCESSOR_H

#include <iostream>
#include <vector>
#include <filesystem>

#include "TriggerContainer.h"
#include "eegContainer.h"
#include "PROV.h"
#include "optionsParameters.h"
#include "Stats.h"

namespace InsermLibrary
{
    class StatisticalFilesProcessor
    {
    public:
        void Process(TriggerContainer* triggerContainer, eegContainer* eegContainer, PROV* myprovFile, std::string freqFolder, statOption* statOption);

    private:
        std::vector<PVALUECOORD> loadPValues(vec3<double>& pValues3D);
        std::vector<PVALUECOORD> loadPValues(vec3<double>& pValues3D, float pLimit);
        std::vector<PVALUECOORD_KW> loadPValues_KW(vec4<double>& pValues4D);
        std::vector<PVALUECOORD_KW> loadPValues_KW(vec4<double>& pValues4D, float pLimit);
    };
}

#endif // STATISTICALFILESPROCESSOR_H
