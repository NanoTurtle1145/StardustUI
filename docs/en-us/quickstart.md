# Quick Start
## First
Clone rhe repository
```bash
# Use http(s)
git clone https://github.com/xingji-studio/StardustUI
# or use ssh
git clone git@github.com:xingji-studio/StardustUI.git
```
## Secondly
Edit the settings
The default platform is detected automatically. You can also define a target
manually before including `settings.hpp`.

For XJ380
```cpp
// targe platform
#define XJ380
```
For Windows
```cpp
#define STARDUSTUI_WINDOWS
```
For Linux
```cpp
#define STARDUSTUI_LINUX
```
## And then
Make now🎆!
```bash
make
```
## next
Build the example
```bash
cd example/helloworld&&make
```
## Finally 
Run the complicated application 
```bash
cd build&&./helloworld.elf
```
