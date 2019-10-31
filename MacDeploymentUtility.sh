#!/bin/bash

#VARIABLES
MACDEPLOYQT=/Users/fsipp/Qt/5.9.8/clang_64/bin/macdeployqt
PROJECT_PATH=/Users/fsipp/Documents/Arbeit/Repository/C++/Localizer_x64_Release
APP_PATH="$PROJECT_PATH/Localizer.app"
LIB_PATH="$APP_PATH/Contents/Frameworks/"
EXEC_PATH="$APP_PATH/Contents/MacOS/Localizer"

#go to folder
cd $PROJECT_PATH

#use macdeploy qt
$MACDEPLOYQT $APP_PATH

#==== Copy other dependancies :
#    -EEGFormat : we change the rpath in the makefile of eegformat,
#                 this way boost dependancies have their rpatrh changed as well

#
