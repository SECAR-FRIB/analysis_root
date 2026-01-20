current=0
reglom=0

#make clean && make
#cd ..
for i in 2158
do
	bash create_rawROOT.sh "$i" "$current" "$reglom"

	bash create_sortROOT.sh "$i" "$current"
	#runNum="$i"
	#hadd -f /mnt/analysis/e20008/rootfiles/total_run0${runNum}_10us.root /mnt/analysis/e20008/rootfiles/run0${runNum}-*.roo
  
 	#cd root_scripts
 	#root -l "DSSD_gain_match.C(""$i"")"
 	#cd ../
  
	#cd secarAnalysis
	#python3 run_secarAnalysis_neut.py "$i"
	#cd ../
 
  #cd Kr_an_2025/acceptance_transmission/detector_yields
  #root -l -b -q "integrateSpectra.C(""$i"")"
  #cd ../../../

done
