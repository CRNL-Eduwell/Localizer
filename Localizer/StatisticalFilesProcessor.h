#ifndef STATISTICALFILESPROCESSOR_H
#define STATISTICALFILESPROCESSOR_H

#include <iostream>
#include <vector>
#include <filesystem>

#include "TriggerContainer.h"
#include "eegContainer.h"
#include "ProvFile.h"
#include "optionsParameters.h"
#include "Stats.h"

namespace InsermLibrary
{
    class StatisticalFilesProcessor
    {
    public:
        void Process(TriggerContainer* triggerContainer, eegContainer* eegContainer, int smoothingID, ProvFile* myprovFile, std::string freqFolder, statOption* statOption);

    private:
        std::vector<PVALUECOORD> loadPValues(vec3<double>& pValues3D);
        std::vector<PVALUECOORD> loadPValues(vec3<double>& pValues3D, float pLimit);
        std::vector<PVALUECOORD_KW> loadPValues_KW(vec4<double>& pValues4D);
        std::vector<PVALUECOORD_KW> loadPValues_KW(vec4<double>& pValues4D, float pLimit);
        void WriteResultFile(std::vector<std::vector<double>> ChannelDataToWrite, std::vector<std::pair<int, int>> posSampleCodeToWrite, TriggerContainer* triggerContainer, eegContainer* eegContainer, int smoothingID, std::string freqFolder);
    };
}

#endif // STATISTICALFILESPROCESSOR_H
