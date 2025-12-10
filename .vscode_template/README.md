# Setup build with VS Code 

## Linux (Ubuntu)

### Install dependencies 

**Git**
```
sudo apt install git
```

**Compiler** 
```
sudo apt install build-essential
sudo apt install clang
```

**Debugger** 
```
sudo apt install lldb
```

**Qt** - Download the Qt online installer and install it using it: 
* Qt Desktop 
* Qt Additional Libraries: Qt 5 Compatibility Module, Qt Network Authorization, Qt State Machines 
* Build Tools: CMake 
* Build Tools: Ninja

Edit `$HOME/.bashrc`, specify the current Qt, for example
```
export QT_DIR=$HOME/Qt/6.10.1/gcc_64

export PATH=$HOME/Qt/Tools/CMake/bin:$PATH
export PATH=$HOME/Qt/Tools/Ninja:$PATH
```

### Install VS Code 
Download the `deb` file from the official website. https://code.visualstudio.com 
Install (version for example)
```
sudo dpkg -i code_1.106.3-1764110892_amd64.deb
```
   
Don't use the `flatpak` version because it has limited permissions; for example, it won't recognize the `clang` compiler. Or learn how to grant it the necessary permissions. 

### Clone repository 

* Add the `ssh` key if it is not already added.
* Fork the MuseScore repository
* Clone your fork

### Setup VS Code 

* In the root of the repository, copy of the `.vscode_template` folder and rename it to `.vscode`
* Launch VS Code
* Add a new clean profile, name it C++ for example
* Open the repository folder
* Install the suggested extensions (they are listed in the `extensions.json` file)
* There will be a choice of cmake preset, select `Qt Clang Debug`.
* There will be buttons on the bottom panel: Build, Debug, Run
* Enjoy!

