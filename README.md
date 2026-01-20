# SECAR Root Analysis Pipeline
This repository contains a basic pipeline for converting and analalyzing SECAR data using ROOT on fishtank.

## Overview
This project includes scripts to:

- process SECAR runs with the provided scripts
- create raw ROOT files from data
- sorted ROOT outputs

- The first step is to convert from evt to ROOT and this is happenning using the ```bash create_rawROOT.sh ``` script. 
The next step is to create sorted ROOT trees for each run that is done using the ```bash create_sortROOT.sh ``` script. 
#### Use the sort_run.sh script to run either or both for one or multiple runs.
```bash
source sort_run.sh
```



## Requirements
Before running any of the scripts, you must set up the analysis environment.

### Setup Environment
On a machine where you run this analysis (fishtank):
```bash
source environment.sh
source environment.sh
```
#### It's important to source the environment script twice in order to load the root version!!

### Edit paths

Make sure that the
```bash
create_rawROOT.sh 
create_sortROOT.sh
```
scripts have the correct experiment number and paths to the data


