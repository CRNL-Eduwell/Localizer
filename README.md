# Localizer

![](https://github.com/CRNL-Eduwell/Localizer/raw/master/Media/Icon.png "Localizer Icon")

Localizer is an application whose goal is to take raw brain recordings and process them to obtain the frequency responses using a combination of band pass filter and Hilbert Envelloppe. This software is developped using C++ for Windows, Mac and Linux.

## Getting Started

These instructions will allow you to run Localizer on your computer. 

### Windows

Unzip the zip file and execute Localizer.exe.

### Linux

Unzip the zip file to a target directory.
Then, you need to give execution rights to the executable, and execute HiBoP.x86_64.

```
cd <LOCALIZER_DIR>
chmod 755 ./Localizer.x86_64
./Localizer.x86_64
```

### MacOS

Unzip the zip file to a target directory.
Then, you need to give execution rights to the executable, remove external attributes, and execute Localizer.app.

```
cd <LOCALIZER_DIR>
chmod -R 755 Localizer.app
xattr -rc Localizer.app
```

## Supported file formats

*  **.TRC**: Micromed
*  **.eeg.ent/eeg**: ELAN
*  **.vhdr/.vmrk/.eeg**: BrainVision
*  **.edf**: European Data Format (EDF)

## License

This work is licensed under a [GPLv3](https://www.gnu.org/licenses/gpl-3.0.fr.html) License.

### Used third-party resources

#### C++

*  [FFTW](https://www.fftw.org/)
*  [Boost](https://www.boost.org/)