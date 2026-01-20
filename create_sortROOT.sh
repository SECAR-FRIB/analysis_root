# Updated by MKS Mar 2019

#### USER REQUIRED INPUTS #####

# Change the experiment number
exp_number=e20008

#Change the ddas version if needed
ver=3.3

# Change path to runConverter
runconverterpath=/user/${exp_number}/SOMEuser/createEvents

###################################

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

# Check how many files there are
number_of_segments=$(ls -f ${evt_file_address} | grep "evt" | wc -l)
echo "There are ${number_of_segments} evt files in run${run_number}."
#
for (( segment_number=0; segment_number<${number_of_segments};  segment_number++ )); do
#for (( segment_number=1; segment_number<2;  segment_number++ )); do
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

echo -e "\n"
    # Convert DDAS ROOT tree to SuN ROOT tree
    cd ${runconverterpath}
    ./runConverter ${file_number}

done