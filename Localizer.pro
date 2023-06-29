TARGET = Localizer
TEMPLATE = app

####################################### CONFIG
macx:CONFIG += app_bundle
CONFIG += qt
CONFIG += c++1z
CONFIG -= console
QT += core gui widgets core5compat

COMPUTER = "flo_mac_work" # ben_win_work, flo_win_home, flo_win_work, flo_mac_work, ben_linux_work

####################################### COMPILATION FLAGS
win32{
    QMAKE_CXXFLAGS += /O2 /openmp /W3
    DEFINES += _CRT_SECURE_NO_WARNINGS
}
unix:!macx{
    QMAKE_CXXFLAGS += -fopenmp -O3 -fpermissive
    QMAKE_CXXFLAGS_WARN_ON += -Wno-maybe-uninitialized -Wno-unused-result -Wno-unused-function -Wno-unused-parameter -Wno-comment -Wno-sign-compare -Wno-deprecated-declarations -Wno-unused-variable -Wno-unused-local-typedefs -Wno-reorder -Wno-switch #-Wfatal-errors -Werror
    QMAKE_LFLAGS += "-Wl,-rpath,\'\$$ORIGIN\'"
}
macx{
    QMAKE_CXXFLAGS += -O3
    QMAKE_CXXFLAGS_WARN_ON += -Wno-maybe-uninitialized -Wno-unused-result -Wno-unused-function -Wno-unused-parameter -Wno-comment -Wno-sign-compare -Wno-deprecated-declarations -Wno-unused-variable -Wno-unused-local-typedefs -Wno-reorder -Wno-switch #-Wfatal-errors -Werror
    QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.15 #allow acces to C++ 17 std::filesystem
}

INCLUDE_BOOST = ""
INCLUDE_FFTW = ""
INCLUDE_EEGFORMAT = ""
INCLUDE_FRAMEWORK = ""
INCLUDE_LOCALIZER = ""
LIB_BOOST = ""
LIB_FFTW = ""
LIB_EEGFORMAT = ""
LIB_FRAMEWORK = ""
LIB_MISC = ""

equals(COMPUTER, "ben_win_work"){
    INCLUDE_BOOST = "C:/boost/boost_1_78_0"
    INCLUDE_FFTW = "D:/HBP/FFTW_3.3.4_x64"
    INCLUDE_EEGFORMAT = "D:/HBP/EEGFormat/EEGFormat/EEGFormat"
    INCLUDE_FRAMEWORK = "D:/HBP/Framework/Framework"
    INCLUDE_LOCALIZER = "D:/HBP/Localizer"
    LIB_BOOST = -L"C:/boost/boost_1_78_0/lib64-msvc-14.2" -l"libboost_filesystem-vc142-mt-x64-1_78"
    LIB_FFTW = -L"D:/HBP/FFTW_3.3.4_x64" -llibfftw3f-3
    LIB_EEGFORMAT = -L"D:/HBP/EEGFormat/EEGFormat/x64/Release/" -lEEGFormat
    LIB_FRAMEWORK = -L"D:/HBP/Framework_x64/release" -lFramework
    LIB_MISC = Comdlg32.lib shell32.lib
}

equals(COMPUTER, "ben_win_work_new"){
    INCLUDE_BOOST = "C:/local/boost_1_78_0"
    INCLUDE_FFTW = "C:/local/FFTW_3.3.4_x64"
    INCLUDE_EEGFORMAT = "C:/HBP/Software/EEGFormat/EEGFormat"
    INCLUDE_FRAMEWORK = "C:/HBP/Software/Framework/Framework"
    INCLUDE_LOCALIZER = "C:/HBP/Software/Localizer"
    LIB_BOOST = -L"C:/local/boost_1_78_0/lib64-msvc-14.2" -l"libboost_filesystem-vc142-mt-x64-1_78"
    LIB_FFTW = -L"C:/local/FFTW_3.3.4_x64" -llibfftw3f-3
    LIB_EEGFORMAT = -L"C:/HBP/Software/EEGFormat/x64/Release/" -lEEGFormat
    LIB_FRAMEWORK = -L"C:/HBP/Software/Framework_x64/release" -lFramework
    LIB_MISC = Comdlg32.lib shell32.lib
}

equals(COMPUTER, "flo_win_home"){
    INCLUDE_BOOST = ""
    INCLUDE_FFTW = "C:/Users/Florian/Documents/Arbeit/Software/DLL/FFTW_3.3.4_x64"
    INCLUDE_EEGFORMAT = "C:/Users/Florian/Documents/Arbeit/Repository/C++/EEGFormat/EEGFormat"
    INCLUDE_FRAMEWORK = "C:/Users/Florian/Documents/Arbeit/Repository/C++/Framework/Framework"
    INCLUDE_LOCALIZER = "C:/Users/Florian/Documents/Arbeit/Repository/C++/Localizer"
    LIB_BOOST = ""
    LIB_FFTW = -L"C:/Users/Florian/Documents/Arbeit/Software/DLL/FFTW_3.3.4_x64" -llibfftw3f-3
    LIB_EEGFORMAT = -L"C:/Users/Florian/Documents/Arbeit/Repository/C++/EEGFormat/x64/Release/" -lEEGFormat
    LIB_FRAMEWORK = -L"C:/Users/Florian/Documents/Arbeit/Repository/C++/Framework/x64/Release/" -lFramework
    LIB_MISC = Comdlg32.lib shell32.lib
}

equals(COMPUTER, "flo_win_work"){
    INCLUDE_BOOST = ""
    INCLUDE_FFTW = "D:/Users/Florian/Documents/Arbeit/Software/DLL/FFTW_3.3.4_x64"
    INCLUDE_EEGFORMAT = "D:/Users/Florian/Documents/Arbeit/Repository/C++/EEGFormat/EEGFormat"
    INCLUDE_FRAMEWORK = "D:/Users/Florian/Documents/Arbeit/Repository/C++/Framework/Framework"
    INCLUDE_LOCALIZER = "D:/Users/Florian/Documents/Arbeit/Repository/C++/Localizer"
    LIB_BOOST = ""
    LIB_FFTW = -L"D:/Users/Florian/Documents/Arbeit/Software/DLL/FFTW_3.3.4_x64" -llibfftw3f-3
    LIB_EEGFORMAT = -L"D:/Users/Florian/Documents/Arbeit/Repository/C++/EEGFormat/x64/Release/" -lEEGFormat
    LIB_FRAMEWORK = -L"D:/Users/Florian/Documents/Arbeit/Repository/C++/Framework/x64/Release/" -lFramework
    LIB_MISC = Comdlg32.lib shell32.lib
}

equals(COMPUTER, "flo_mac_work"){
    INCLUDE_BOOST = "/opt/homebrew/Cellar/boost/1.80.0/include"
    INCLUDE_FFTW = "/opt/homebrew/Cellar/fftw/3.3.10_1/include"
    INCLUDE_EEGFORMAT = "/Users/florian/Documents/Arbeit/Repository/C++/EEGFormat/EEGFormat"
    INCLUDE_FRAMEWORK = "/Users/florian/Documents/Arbeit/Repository/C++/Framework/Framework"
    INCLUDE_LOCALIZER = "/Users/florian/Documents/Arbeit/Repository/C++/Localizer/Localizer"
    LIB_BOOST = -L"/opt/homebrew/Cellar/boost/1.80.0/lib" -lboost_filesystem
    LIB_FFTW = -L"/opt/homebrew/Cellar/fftw/3.3.10_1/lib" -lfftw3f.3 -lfftw3f_threads.3
    LIB_EEGFORMAT = -L"/Users/florian/Documents/Arbeit/Repository/C++/EEGFormat/EEGFormat" -lEEGFormat
    LIB_FRAMEWORK = -L"/Users/florian/Documents/Arbeit/Repository/C++/Framework/Framework" -lFramework
    LIB_MISC = ""
}

equals(COMPUTER, "ben_linux_work"){
    INCLUDE_BOOST = ""
    INCLUDE_FFTW = ""
    INCLUDE_EEGFORMAT = "/home/hbp/software/EEGFormat/EEGFormat"
    INCLUDE_FRAMEWORK = "/home/hbp/software/Framework/Framework"
    INCLUDE_LOCALIZER = "/home/hbp/software/Localizer/Localizer"
    LIB_BOOST = -lboost_filesystem
    LIB_FFTW = -L"/usr/local/lib" -lfftw3f -lfftw3f_threads
    LIB_EEGFORMAT = -L"/home/hbp/software/EEGFormat/EEGFormat" -lEEGFormat
    LIB_FRAMEWORK = -L"/home/hbp/software/Framework_x64" -lFramework
    LIB_MISC = -fopenmp
}

INCLUDEPATH += $$INCLUDE_BOOST
INCLUDEPATH += $$INCLUDE_FFTW
INCLUDEPATH += $$INCLUDE_EEGFORMAT
INCLUDEPATH += $$INCLUDE_FRAMEWORK
INCLUDEPATH += $$INCLUDE_LOCALIZER

LIBS += $$LIB_BOOST
LIBS += $$LIB_FFTW
LIBS += $$LIB_EEGFORMAT
LIBS += $$LIB_FRAMEWORK
LIBS += $$LIB_MISC

####################################### PROJECT FILES
RESOURCES += \
    Localizer/localizer.qrc

FORMS += \
    Localizer/AboutDycog.ui \
    Localizer/BlocWindow.ui \
    Localizer/ChooseLocaWindow.ui \
    Localizer/ConnectCleaner.ui \
    Localizer/ErpProcessor.ui \
    Localizer/EventWindow.ui \
    Localizer/FileConverterProcessor.ui \
    Localizer/FileHealthCheckerWindow.ui \
    Localizer/FrequenciesWindow.ui \
    Localizer/FrequencyWindow.ui \
    Localizer/GeneralOptionsWindow.ui \
    Localizer/IconWindow.ui \
    Localizer/ProtocolWindow.ui \
    Localizer/ProtocolsWindow.ui \
    Localizer/SubBlocWindow.ui \
    Localizer/displayprov.ui \
    Localizer/localizer.ui \
    Localizer/optionsPerf.ui \
    Localizer/optionsStats.ui \
    Localizer/picOptions.ui

HEADERS += \
    Localizer/AboutDycog.h \
    Localizer/AlgorithmCalculator.h \
    Localizer/AlgorithmStrategyFactory.h \
    Localizer/AnalysisFolder.h \
    Localizer/BarplotProcessor.h \
    Localizer/BidsSubject.h \
    Localizer/BidsSubjectWorker.h \
    Localizer/Bloc.h \
    Localizer/BlocWindow.h \
    Localizer/BrainvisionFileInfo.h \
    Localizer/ChooseLocaWindow.h \
    Localizer/CleanConnectFile.h \
    Localizer/ConcatenationWorker.h \
    Localizer/ConnectCleaner.h \
    Localizer/CorrelationMapsProcessor.h \
    Localizer/DataContainer.h \
    Localizer/DeselectableTreeView.h \
    Localizer/EdfFileInfo.h \
    Localizer/ElanFileInfo.h \
    Localizer/EnvplotProcessor.h \
    Localizer/ErpProcessor.h \
    Localizer/ErpWorker.h \
    Localizer/Event.h \
    Localizer/EventWindow.h \
    Localizer/ExperimentFolder.h \
    Localizer/FileConverterProcessor.h \
    Localizer/FileConverterWorker.h \
    Localizer/FileHealthCheckerWindow.h \
    Localizer/FrequenciesWindow.h \
    Localizer/FrequencyBand.h \
    Localizer/FrequencyBandAnalysisOpt.h \
    Localizer/FrequencyFile.h \
    Localizer/FrequencyFolder.h \
    Localizer/FrequencyWindow.h \
    Localizer/GeneralOptionsFile.h \
    Localizer/GeneralOptionsWindow.h \
    Localizer/HilbertEnveloppe.h \
    Localizer/IAlgorithmStrategy.h \
    Localizer/IEegFileInfo.h \
    Localizer/ITxtFile.h \
    Localizer/IWorker.h \
    Localizer/Icon.h \
    Localizer/IconWindow.h \
    Localizer/LOCA.h \
    Localizer/MicromedFileInfo.h \
    Localizer/MultiSubjectWorker.h \
    Localizer/PatientFolderWorker.h \
    Localizer/Protocol.h \
    Localizer/ProtocolFile.h \
    Localizer/ProtocolWindow.h \
    Localizer/ProtocolsWindow.h \
    Localizer/ProvFile.h \
    Localizer/SingleFilesWorker.h \
    Localizer/StatisticalFilesProcessor.h \
    Localizer/Stats.h \
    Localizer/SubBloc.h \
    Localizer/SubBlocWindow.h \
    Localizer/SubjectFolder.h \
    Localizer/TrialMatricesProcessor.h \
    Localizer/Trigger.h \
    Localizer/TriggerContainer.h \
    Localizer/Utility.h \
    Localizer/Window.h \
    Localizer/barsPlotsGenerator.h \
    Localizer/displayProv.h \
    Localizer/eegContainer.h \
    Localizer/eegContainerParameters.h \
    Localizer/json.hpp \
    Localizer/localizer.h \
    Localizer/mapsGenerator.h \
    Localizer/optionsParameters.h \
    Localizer/optionsPerf.h \
    Localizer/optionsStats.h \
    Localizer/picOptions.h \
    Localizer/singleFile.h

SOURCES += \
    Localizer/AboutDycog.cpp \
    Localizer/AlgorithmCalculator.cpp \
    Localizer/AlgorithmStrategyFactory.cpp \
    Localizer/AnalysisFolder.cpp \
    Localizer/BarplotProcessor.cpp \
    Localizer/BidsSubject.cpp \
    Localizer/BidsSubjectWorker.cpp \
    Localizer/Bloc.cpp \
    Localizer/BlocWindow.cpp \
    Localizer/BrainvisionFileInfo.cpp \
    Localizer/ChooseLocaWindow.cpp \
    Localizer/CleanConnectFile.cpp \
    Localizer/ConcatenationWorker.cpp \
    Localizer/ConnectCleaner.cpp \
    Localizer/CorrelationMapsProcessor.cpp \
    Localizer/DataContainer.cpp \
    Localizer/DeselectableTreeView.cpp \
    Localizer/EdfFileInfo.cpp \
    Localizer/ElanFileInfo.cpp \
    Localizer/EnvplotProcessor.cpp \
    Localizer/ErpProcessor.cpp \
    Localizer/ErpWorker.cpp \
    Localizer/Event.cpp \
    Localizer/EventWindow.cpp \
    Localizer/ExperimentFolder.cpp \
    Localizer/FileConverterProcessor.cpp \
    Localizer/FileConverterWorker.cpp \
    Localizer/FileHealthCheckerWindow.cpp \
    Localizer/FrequenciesWindow.cpp \
    Localizer/FrequencyBand.cpp \
    Localizer/FrequencyFile.cpp \
    Localizer/FrequencyFolder.cpp \
    Localizer/FrequencyWindow.cpp \
    Localizer/GeneralOptionsFile.cpp \
    Localizer/GeneralOptionsWindow.cpp \
    Localizer/HilbertEnveloppe.cpp \
    Localizer/IEegFileInfo.cpp \
    Localizer/ITxtFile.cpp \
    Localizer/IWorker.cpp \
    Localizer/Icon.cpp \
    Localizer/IconWindow.cpp \
    Localizer/LOCA.cpp \
    Localizer/MicromedFileInfo.cpp \
    Localizer/MultiSubjectWorker.cpp \
    Localizer/PatientFolderWorker.cpp \
    Localizer/Protocol.cpp \
    Localizer/ProtocolFile.cpp \
    Localizer/ProtocolWindow.cpp \
    Localizer/ProtocolsWindows.cpp \
    Localizer/ProvFile.cpp \
    Localizer/SingleFilesWorker.cpp \
    Localizer/StatisticalFilesProcessor.cpp \
    Localizer/Stats.cpp \
    Localizer/SubBloc.cpp \
    Localizer/SubBlocWindow.cpp \
    Localizer/SubjectFolder.cpp \
    Localizer/TrialMatricesProcessor.cpp \
    Localizer/Trigger.cpp \
    Localizer/TriggerContainer.cpp \
    Localizer/Utility.cpp \
    Localizer/Window.cpp \
    Localizer/barsPlotsGenerator.cpp \
    Localizer/displayProv.cpp \
    Localizer/eegContainer.cpp \
    Localizer/localizer.cpp \
    Localizer/main.cpp \
    Localizer/mapsGenerator.cpp \
    Localizer/optionsPerf.cpp \
    Localizer/optionsStats.cpp \
    Localizer/picOptions.cpp \
    Localizer/singleFile.cpp

### Define Icon on Mac : https://doc.qt.io/qt-5/appicon.html
win32{
    RC_FILE = Localizer/myapp.rc
}
macx{
    ICON = Localizer/Resources/localizer_mac.icns
}
