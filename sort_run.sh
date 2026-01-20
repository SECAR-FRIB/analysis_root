current=0
reglom=0

#make clean && make
#cd ..
for i in {2165..2168}
do
	
	bash create_rawROOT.sh "$i" "$current" "$reglom"
	
	bash create_sortROOT.sh "$i" "$current"
	
	#runNum="$i"
	#hadd -f /mnt/analysis/e20008/rootfiles/total_run0${runNum}_10us.root /mnt/analysis/e20008/rootfiles/run0${runNum}-*.root

done
