# MuseScore Visual Tests

The idea is to export the scores to png with the reference build of the MuseScore and the current one, and compare these pictures.  
  
The main purpose is to run on CI for pull requests to the main branch.  
Can also be used locally for development.

## Requirements
In order to generate the diff between the reference
file and the generated one, *Image Magick* needs to be
installed and "compare" should be in the `PATH`.

## How to use

To create png files and compare them you can use the `vtest.sh` script.  

### Generate reference png 
If you do not have any generated reference png, then first you need to generate them.

* You need to build yourself or take anywhere a reference MuseScore build.
* Call the script from the repository root 
```
vtest/vtest.sh -c gen_ref -m path/to/ref/mscore
```
* You can see the created files in `vtest.artifacts/ref`

### Generate current png 
* You need to build yourself a MuseScore test build.
* Call the script from the repository root 
```
vtest/vtest.sh -c gen_cur -m path/to/mscore
```
* You can see the created files in `vtest.artifacts/current`

### Compare 
* Call the script from the repository root 
```
vtest/vtest.sh -c compare
```
* You can see the created files in `vtest.artifacts/compare`

You can specify some paths explicitly, see `vtest.sh` source.  
For Windows, try using Git Bash

## Add new test
Just put the new score in the `vtest/scores` directory.   
Score can be in `mscx` and `mscz` format