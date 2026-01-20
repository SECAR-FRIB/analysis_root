# Updated by MKS Mar 2019

#### USER REQUIRED INPUTS #####

# Change the experiment number
exp_number=e20008

#Change the ddas version if needed
ver=3.3

###################################

#remember where ddasdumper lives
DDAS_BIN=/usr/opt/ddas/${ver}/bin

# Loads Root
#module load root

# Prompt run number
run_number="$1"

if [ "$1" = "" ]
then
    echo -n "You must specify the run number!"
    echo -e "\n"
    exit
fi

echo -e "\n"

current_status="$2"

if [ $current_status = 0 ]
then
    evt_file_address=/mnt/rawdata/${exp_number}/experiment/run${run_number}
else
    evt_file_address=/mnt/rawdata/${exp_number}/experiment/current
fi

echo "$evt_file_address"

reglom_status="$3"

# Check how many files there are
number_of_segments=$(ls -f ${evt_file_address} | grep "evt" | wc -l)
echo "There are ${number_of_segments} evt files in run${run_number}."
#${number_of_segments};
for (( segment_number=0;
       segment_number<${number_of_segments};
              segment_number++ )); do

    if [ ${segment_number} -lt 10 ]
    then
        if [ ${run_number} -lt 1000 ]
        then
            if [ ${run_number} -lt 100 ]
            then
                if [ ${run_number} -lt 10 ]
                then
                file_number=000${run_number}-0${segment_number}
                else
	            file_number=00${run_number}-0${segment_number}
                fi
            else
	        file_number=0${run_number}-0${segment_number}
            fi
        else
	    file_number=${run_number}-0${segment_number}
        fi
    else
        if [ ${run_number} -lt 1000 ]
        then
            if [ ${run_number} -lt 100 ]
            then
                if [ ${run_number} -lt 10 ]
                then
                file_number=000${run_number}-${segment_number}
                else
	            file_number=00${run_number}-${segment_number}
                fi
            else
	        file_number=0${run_number}-${segment_number}
            fi
        else
	    file_number=${run_number}-${segment_number}
        fi
    fi

    # Convert .evt to dchan tree
    FILENAME=${evt_file_address}/run-${file_number}.evt
    FILESIZE=$(stat -c%s "${FILENAME}")
    echo "evt file size: ${FILESIZE}"

    if [ $reglom_status = 0 ]
    then
        ${DDAS_BIN}/ddasdumper --source=file://${evt_file_address}/run-${file_number}.evt --fileout=/mnt/analysis/${exp_number}/dumpedfiles/run${file_number}.root
    else
        # make an intermediate directory for the unglomming
        mkdir /mnt/analysis/${exp_number}/sorted_evt_files/${file_number}
        cd /mnt/analysis/${exp_number}/sorted_evt_files/${file_number}

        # unglom the evt file
        $DAQBIN/unglom file://${evt_file_address}/run-${file_number}.evt

        # check how many sid files there are
        sidfiles=$(ls | wc -l)
        
        # make a list of sids to eventually reglom
        # be sure to check that sid names below match those for your experiment
        # i.e. you may have sid-0 sid-1 instead of sid-1 sid-2 sid-3
        sid=""
        if [[ -f "sid-0" ]]
        then
        export sid="file:///mnt/analysis/${exp_number}/sorted_evt_files/${file_number}/sid-0 "$sid
        fi
        if [[ -f "sid-1" ]]
        then
        export sid="file:///mnt/analysis/${exp_number}/sorted_evt_files/${file_number}/sid-1 "$sid
        fi
        ls
        echo $sid

        # if only one sid file no need to reglom, just dump
        if [ $sidfiles -lt 2 ]
        then
            ${DDAS_BIN}/ddasdumper --source=file://${evt_file_address}/run-${file_number}.evt --fileout=/mnt/analysis/${exp_number}/dumpedfiles/run${file_number}.root 
        else
        # reglom all the sids using a time window of 200000 ns
        # see the nscl documentation for reglom for more details
        $DAQBIN/reglom ${sid} --dt 200000 --sourceid 0 --timestamp-policy latest --output /mnt/analysis/${exp_number}/sorted_evt_files/run-${file_number}.evt
        # after regloming, dump output to root file
        ${DDAS_BIN}/ddasdumper --source=file:///mnt/analysis/${exp_number}/sorted_evt_files/run-${file_number}.evt --fileout=/mnt/analysis/${exp_number}/dumpedfiles/run${file_number}.root
        fi
    fi

done
