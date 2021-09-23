TARGET = Localizer
TEMPLATE = app

####################################### CONFIG
macx:CONFIG += app_bundle
CONFIG += qt
CONFIG += c++1z
CONFIG -= console
QT += core gui widgets

####################################### COMPILATION FLAGS
#Windows is done by VisualStudio
unix:!macx{
    QMAKE_CXXFLAGS += -fopenmp -O3 -fpermissive
    QMAKE_CXXFLAGS_WARN_ON += -Wno-maybe-uninitialized -Wno-unused-result -Wno-unused-function -Wno-unused-parameter -Wno-comment -Wno-sign-compare -Wno-deprecated-declarations -Wno-unused-variable -Wno-unused-local-typedefs -Wno-reorder -Wno-switch #-Wfatal-errors -Werror
    QMAKE_LFLAGS += "-Wl,-rpath,\'\$$ORIGIN\'"
}
macx{
    QMAKE_CXXFLAGS += -O3
    QMAKE_CXXFLAGS_WARN_ON += -Wno-maybe-uninitialized -Wno-unused-result -Wno-unused-function -Wno-unused-parameter -Wno-comment -Wno-sign-compare -Wno-deprecated-declarations -Wno-unused-variable -Wno-unused-local-typedefs -Wno-reorder -Wno-switch #-Wfatal-errors -Werror
}

macx{
####################################### INCLUDEPATH
    INCLUDEPATH += "/usr/local/Cellar/boost/1.76.0/include"
    INCLUDEPATH += "/usr/local/Cellar/fftw/3.3.8_2/include"
    INCLUDEPATH += "/Users/florian/Documents/Arbeit/Repository/C++/EEGFormat/EEGFormat"
    INCLUDEPATH += "/Users/florian/Documents/Arbeit/Repository/C++/Framework/Framework"
    INCLUDEPATH += "/Users/florian/Documents/Arbeit/Repository/C++/Localizer/Localizer"

####################################### LIBRAIRIES
    LIBS += -L"/usr/local/Cellar/boost/1.76.0/lib" -lboost_filesystem
    LIBS += -L"/usr/local/Cellar/fftw/3.3.8_2/lib" -lfftw3f.3 -lfftw3f_threads.3
    LIBS += -L"/Users/florian/Documents/Arbeit/Repository/C++/EEGFormat/EEGFormat" -lEEGFormat
    LIBS += -L"/Users/florian/Documents/Arbeit/Repository/C++/Framework/Framework" -lFramework
}

unix:!macx{
####################################### INCLUDEPATH
    INCLUDEPATH += "/home/bbontemps/software/boost_1_70_0"
    INCLUDEPATH += "/home/bbontemps/software/EEGFormat/EEGFormat"
    INCLUDEPATH += "/home/bbontemps/software/Framework/Framework"
    INCLUDEPATH += "/home/bbontemps/software/Localizer/Localizer"

####################################### LIBRAIRIES
    LIBS += -L"/home/bbontemps/software/boost_1_70_0" -lboost_filesystem
    LIBS += -L"/home/bbontemps/software/EEGFormat/EEGFormat" -lEEGFormat
    LIBS += -L"/home/bbontemps/software/build-Framework-Desktop_Qt_5_14_2_GCC_64bit-Release" -lFramework
    LIBS += -L"/usr/local/lib" -lfftw3f -lfftw3f_threads
    LIBS += -fopenmp
}

####################################### PROJECT FILES
RESOURCES += \
    Localizer/localizer.qrc

FORMS += \
    Localizer/AboutDycog.ui \
    Localizer/ConnectCleaner.ui \
    Localizer/ErpProcessor.ui \
    Localizer/FileConverterProcessor.ui \
    Localizer/GeneralOptionsWindow.ui \
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
    Localizer/GeneralOptionsFile.h \
    Localizer/GeneralOptionsWindow.h \
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
    Localizer/GeneralOptionsFile.cpp \
    Localizer/GeneralOptionsWindow.cpp \
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

### Define Icon on Mac : https://doc.qt.io/qt-5/appicon.html
### Windows is done with vs studio and an rc file per the doc
macx{
    ICON = Localizer/Resources/localizer_mac.icns
}
