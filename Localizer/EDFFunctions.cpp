#include "EDFFunctions.h"

void InsermLibrary::EDFFunctions::deleteOneOrSeveralElectrodesAndData(EDFFile *myEDFFile, vector<int> indexToDelete)
{
	if (indexToDelete.size() > 0)
	{
		if (myEDFFile->eegData().size() == 0)
		{
			for (int i = (int)indexToDelete.size() - 1; i >= 0; i--)
			{
				myEDFFile->Electrodes().erase(myEDFFile->Electrodes().begin() + indexToDelete[i]);
			}
		}
		else
		{
			for (int i = (int)indexToDelete.size() - 1; i >= 0; i--)
			{
				myEDFFile->Electrodes().erase(myEDFFile->Electrodes().begin() + indexToDelete[i]);
				myEDFFile->eegData().erase(myEDFFile->eegData().begin() + indexToDelete[i]);
			}
		}
	}
}