TARGET = Localizer
TEMPLATE = app

####################################### CONFIG
macx:CONFIG += app_bundle
CONFIG += qt
CONFIG += c++1z
CONFIG -= console
QT += core gui widgets uitools

####################################### COMPILATION FLAGS
#Windows is done by VisualStudio
unix:!macx{
    QMAKE_CXXFLAGS += -fopenmp -O3 -fpermissive
    QMAKE_CXXFLAGS_WARN_ON += -Wno-maybe-uninitialized -Wno-unused-result -Wno-unused-function -Wno-unused-parameter -Wno-comment -Wno-sign-compare -Wno-deprecated-declarations -Wno-unused-variable -Wno-unused-local-typedefs -Wno-reorder -Wno-switch #-Wfatal-errors -Werror
}
macx{
    QMAKE_CXXFLAGS += -O3
    QMAKE_CXXFLAGS_WARN_ON += -Wno-maybe-uninitialized -Wno-unused-result -Wno-unused-function -Wno-unused-parameter -Wno-comment -Wno-sign-compare -Wno-deprecated-declarations -Wno-unused-variable -Wno-unused-local-typedefs -Wno-reorder -Wno-switch #-Wfatal-errors -Werror
}




####################################### INCLUDEPATH
INCLUDEPATH += "/usr/local/Cellar/boost/1.70.0/include"
INCLUDEPATH += "/usr/local/Cellar/fftw/3.3.8_1/include"
INCLUDEPATH += "/Users/fsipp/Documents/Arbeit/Repository/C++/EEGFormat/EEGFormat"
INCLUDEPATH += "/Users/fsipp/Documents/Arbeit/Repository/C++/Framework/Framework"
INCLUDEPATH += "/Users/fsipp/Documents/Arbeit/Repository/C++/Localizer/Localizer"

####################################### LIBRAIRIES
LIBS += -L"/usr/local/Cellar/boost/1.70.0/lib" -lboost_filesystem
LIBS += -L"/usr/local/Cellar/fftw/3.3.8_1/lib" -lfftw3f.3 -lfftw3f_threads.3
LIBS += -L"/Users/fsipp/Documents/Arbeit/Repository/C++/EEGFormat/EEGFormat" -lEEGFormat
LIBS += -L"/Users/fsipp/Documents/Arbeit/Repository/C++/Framework/Framework" -lFramework

####################################### PROJECT FILES
RESOURCES += \
    Localizer/localizer.qrc

FORMS += \
    Localizer/AboutDycog.ui \
    Localizer/ConnectCleaner.ui \
    Localizer/ErpProcessor.ui \
    Localizer/FileConverterProcessor.ui \
    Localizer/ProtocolWindow.ui \
    Localizer/displayprov.ui \
    Localizer/localizer.ui \
    Localizer/optionsPerf.ui \
    Localizer/optionsStats.ui \
    Localizer/picOptions.ui

HEADERS += \
    Localizer/AboutDycog.h \
    Localizer/AlgorithmCalculator.h \
    Localizer/AlgorithmStrategyFactory.h \
    Localizer/CleanConnectFile.h \
    Localizer/ConcatenationWorker.h \
    Localizer/ConnectCleaner.h \
    Localizer/DataContainer.h \
    Localizer/DeselectableTreeView.h \
    Localizer/ErpProcessor.h \
    Localizer/ErpWorker.h \
    Localizer/FileConverterProcessor.h \
    Localizer/FileConverterWorker.h \
    Localizer/FrequencyBand.h \
    Localizer/FrequencyBandAnalysisOpt.h \
    Localizer/FrequencyFile.h \
    Localizer/HilbertEnveloppe.h \
    Localizer/IAlgorithmStrategy.h \
    Localizer/ITxtFile.h \
    Localizer/IWorker.h \
    Localizer/LOCA.h \
    Localizer/PROV.h \
    Localizer/PROVParameters.h \
    Localizer/PatientFolderWorker.h \
    Localizer/ProtocolFile.h \
    Localizer/ProtocolWindow.h \
    Localizer/SingleFilesWorker.h \
    Localizer/Stats.h \
    Localizer/Trigger.h \
    Localizer/TriggerContainer.h \
    Localizer/Utility.h \
    Localizer/barsPlotsGenerator.h \
    Localizer/displayProv.h \
    Localizer/eegContainer.h \
    Localizer/eegContainerParameters.h \
    Localizer/localizer.h \
    Localizer/mapsGenerator.h \
    Localizer/optionsParameters.h \
    Localizer/optionsPerf.h \
    Localizer/optionsStats.h \
    Localizer/patientFolder.h \
    Localizer/picOptions.h \
    Localizer/singleFile.h

SOURCES += \
    Localizer/AboutDycog.cpp \
    Localizer/AlgorithmCalculator.cpp \
    Localizer/AlgorithmStrategyFactory.cpp \
    Localizer/CleanConnectFile.cpp \
    Localizer/ConcatenationWorker.cpp \
    Localizer/ConnectCleaner.cpp \
    Localizer/DataContainer.cpp \
    Localizer/DeselectableTreeView.cpp \
    Localizer/ErpProcessor.cpp \
    Localizer/ErpWorker.cpp \
    Localizer/FileConverterProcessor.cpp \
    Localizer/FileConverterWorker.cpp \
    Localizer/FrequencyBand.cpp \
    Localizer/FrequencyFile.cpp \
    Localizer/HilbertEnveloppe.cpp \
    Localizer/ITxtFile.cpp \
    Localizer/IWorker.cpp \
    Localizer/LOCA.cpp \
    Localizer/PROV.cpp \
    Localizer/PatientFolderWorker.cpp \
    Localizer/ProtocolFile.cpp \
    Localizer/ProtocolWindow.cpp \
    Localizer/SingleFilesWorker.cpp \
    Localizer/Stats.cpp \
    Localizer/Trigger.cpp \
    Localizer/TriggerContainer.cpp \
    Localizer/Utility.cpp \
    Localizer/barsPlotsGenerator.cpp \
    Localizer/displayProv.cpp \
    Localizer/eegContainer.cpp \
    Localizer/localizer.cpp \
    Localizer/main.cpp \
    Localizer/mapsGenerator.cpp \
    Localizer/optionsPerf.cpp \
    Localizer/optionsStats.cpp \
    Localizer/patientFolder.cpp \
    Localizer/picOptions.cpp \
    Localizer/singleFile.cpp
