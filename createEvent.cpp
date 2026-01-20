//////////////////////////////////////////////////////////////////////////////////////////
//  Steve Quinn     October 2013                                                        //
//  Alex Dombos             2014         Added Traces, TACs, PINs, DSSD, and Veto	      //
//  Antonius Torode   11-29-2017         Modified for use with e17503                   //
//  Caley Harris    October 2018         Modified for use with e16033                   //
//  Mallory Smith   October 2018         Modified to use on fireside                    //
//                  March 2019           Modified for use with e17028                   //
//////////////////////////////////////////////////////////////////////////////////////////
// PURPOSE: Take the output ROOT file from ddasdumper and turn it into another ROOT     //
//   file with a pretty SuN tree.                                                       //
//                                                                                      //
// OPTIONS: Change the variable "timewindow" to the amount of time that you want to     //
//   consider for coincidences.  For example a time window of 300 ns means              //
//   that you group everything up to 300 ns after the trigger as a single event.        //
//   (This is assuming you are using the 100 MSPS modules)                              //
//                                                                                      //
// HOW TO RUN:                                                                          //
//   1.  To uncompile  <terminal> make clean                                            //
//   2.  To compile    <terminal> make                                                  //
//   3.  Before compiling, make sure that the timewindow, input file, and output file   //
//        are what you want them to be.                                                 //
//                                                                                      //
// IMPORTANT: Have Fun =)                                                               //
//////////////////////////////////////////////////////////////////////////////////////////

#include <iomanip>
#include <fstream>

#include "ddaschannel.h"
#include "DDASEvent.h"

#include <iostream>
#include <sstream>
#include <cmath>
#include <vector>
#include <TFile.h>
#include <TTree.h>
#include <TBranch.h>
#include "analysePulseShape.C"

using namespace std;

int main(int argc, char* argv[])
{
  
  //Flags for not sorting all detecors (Set true for sorting) 
  bool LENDA = false; 
  bool LScin = false;
  bool pulsers = true;
  bool monitors = true;
  bool mcp = true;
  bool IC = true;
  bool IC_calibration = false;
  bool Si = true;
  bool Si_calibration = false; // not in use

  //........... Si and IC calibration ...........//
  //Si calibration parameters
  int Si_side[64];
  int Si_strip[64];
  int Si_mod[64];
  int Si_chan[64];
  double p0[64], p1[64], p2[64];
  double gain_offset[64], peak[64];
  double IC_dE_gain = 1.0;
  double IC_dE_offset = 0.0;
  
 
  
  if(Si)
  {
    // There was a DSSD map change in spring 2024 for the front channels
    // Runs before that time have the odd front dssd channels at first slot and the even at the second
    ifstream f_SiMap("DSSD_map_2024.dat"); 
    //ifstream f_SiMap("../Fe_pn_2024/calibration/codes/DSSD_map.dat");
     
    // ifstream f_SiCal("../Fe_pn_2024/calibration/codes/Si_calibration_2nd_tar_corr.dat"); 
     //ifstream f_SiGain("../root_scripts/gain_match_DSSD.txt"); 
    // ifstream f_SiCal("../calibration/codes/SiCal_corrected_all_new.dat");
    // ifstream f_SiCal("../calibration/codes/SiMap_corrected_all.dat"); //linear calibration
    
    for(int i=0; i<64; i++)
    {
      //f_SiCal >> Si_side[i] >> Si_strip[i] >> p0[i] >> p1[i] >> p2[i];
      //f_SiCal >> Si_side[i] >> Si_strip[i] >> p0[i] >> p1[i] >> p0_err[i] >> p1_err[i];
      //f_SiGain >> Si_side[i] >> Si_strip[i] >> peak[i] >> gain_offset[i];
      f_SiMap >> Si_mod[i] >> Si_chan[i] >> Si_strip[i] >> Si_side[i];
    }
  }

  //IC calibration parameters
  if(IC_calibration)
  {
    // IC_dE_gain = 0.009631;
    // IC_dE_offset = -1.624;
    IC_dE_gain = 0.0099;
    IC_dE_offset = -0.8873;
  }

  //***** TO CHANGE THE COINCIDENCE WINDOW *****//
  int timewindow = 10000;  // (nanoseconds for 100 MSPS module)
  //............................................//

  std::cout << "runConverter is working on: " << argv[1] << std::endl;

  // Filenames
  TFile *fIn  = new TFile(Form("/mnt/analysis/e20008/dumpedfiles/run%s.root", argv[1]), "READ");
  TFile *fOut = new TFile(Form("/mnt/analysis/e20008/rootfiles/run%s.root", argv[1]), "RECREATE");
  std::cout << "Now processing run " << argv[1] << std::endl;

  // Variables for input file
  int crate;                // event crate number
  int slot;                 // event slot number
  int chan;                 // event channel number
  int energy;               // event energy
  double time;              // event time
  vector<uint16_t> trace;   // trace
  double refT = 0.0;        // placeholder for time of previous trigger
  double deltaT = 0.0;      // difference in time between events
  int nEntries;             // number of entries
  int nEvents;              // number of events per entry
  
  // Variables for output file
  double t_RFQ;
  double t_EBIT;

  int e_Pulser_JENSA;
  double t_Pulser_JENSA;
  int e_Pulser_FP;
  double t_Pulser_FP;
  double gl_Pulser_FP;
  double gs_Pulser_FP;
  double ph_Pulser_FP;
  // double dt_Pulser_FP;

  int e_raw_SiMon1;
  double e_SiMon1;
  double t_SiMon1;
  int m_SiMon1;
  int e_raw_SiMon2;
  double e_SiMon2;
  double t_SiMon2;
  int m_SiMon2;

  int c_LENDA_t[21];
  int e_LENDA_t[21];
  double t_LENDA_t[21];
  int m_LENDA_t[21];
  int m_sum_LENDA_t;
  int c_LENDA_b[21];
  int e_LENDA_b[21];
  double t_LENDA_b[21];
  int m_LENDA_b[21];
  int m_sum_LENDA_b;

  int c_LScin[5];
  int e_LScin[5];
  double t_LScin[5];
  double gl_LScin[5];
  double gs_LScin[5];
  int m_LScin[5];
  int m_sum_LScin;
  double ph_LScin[5];

  int e_UMCP;
  double t_UMCP;
  int pos_UMCP[4];
  int m_UMCP;

  int e_DMCP;
  double t_DMCP;
  int pos_DMCP[4];
  int m_DMCP;

  int e_TAC;

  int e_IC_dE;
  double e_IC_dE_cal;
  double t_IC_dE;
  int m_IC_dE;

  int e_Si[64];
  double e_Si_cal[64];
  double t_Si[64];
  int m_Si[64];
  int m_sum_Si;
  
  int e_SiF[32];
  double e_SiF_cal[32];
  double t_SiF[32];
  int m_SiF[32];
  int m_sum_SiF;
  int m_sum_SiF_corr;

  int e_SiB[32];
  double e_SiB_cal[32];
  double t_SiB[32];
  int m_SiB[32];
  int m_sum_SiB;
  int m_sum_SiB_corr;
  
  double t_trigger = 0.0; // trigger time (the time of the first event in an entry)
  //double max = 0.0;
  //double tmax = 0.0;
  int counter = 10; // counter for status bar

  // Create tree and branches for output file
  TTree *tOut = new TTree("t","FP Tree");
  
  tOut->Branch("time_RFQ", &t_RFQ, "t_RFQ/D");
  tOut->Branch("time_EBIT", &t_EBIT, "t_EBIT/D");

  if (pulsers)
  {    
    tOut->Branch("energy_Pulser_JENSA", &e_Pulser_JENSA, "e_Pulser_JENSA/I");
    tOut->Branch("time_Pulser_JENSA", &t_Pulser_JENSA, "t_Pulser_JENSA/D");
    tOut->Branch("energy_Pulser_FP", &e_Pulser_FP, "e_Pulser_FP/I");
    tOut->Branch("time_Pulser_FP", &t_Pulser_FP, "t_Pulser_FP/D");
    tOut->Branch("gate_Pulser_FP", &gl_Pulser_FP, "gl_Pulser_FP/D");
    tOut->Branch("gate_Pulser_FP", &gs_Pulser_FP, "gs_Pulser_FP/D");
    tOut->Branch("ph_Pulser_FP", &ph_Pulser_FP, "ph_Pulser_FP/D");
    // tOut->Branch("timediff_Pulser_FP", &dt_Pulser_FP, "dt_Pulser_FP/D");
  }
  if (monitors)
  {  
    tOut->Branch("energy_raw_SiMon1", &e_raw_SiMon1, "e_raw_SiMon1/D");
    tOut->Branch("energy_SiMon1", &e_SiMon1, "e_SiMon1/D");
    tOut->Branch("time_SiMon1", &t_SiMon1, "t_SiMon1/D");
    tOut->Branch("mult_SiMon1",&m_SiMon1,"m_SiMon1/I");

    tOut->Branch("energy_raw_SiMon2", &e_raw_SiMon2, "e_raw_SiMon2/D");
    tOut->Branch("energy_SiMon2", &e_SiMon2, "e_SiMon2/D");
    tOut->Branch("time_SiMon2", &t_SiMon2, "t_SiMon2/D");
    tOut->Branch("mult_SiMon2",&m_SiMon2,"m_SiMon2/I");
  }
  if (LENDA)
  {
    //LENDA top
    tOut->Branch("chan_LENDA_t", &c_LENDA_t, "c_LENDA_t[21]/I");
    tOut->Branch("energy_LENDA_t", &e_LENDA_t, "e_LENDA_t[21]/I");
    tOut->Branch("time_LENDA_t", &t_LENDA_t, "t_LENDA_t[21]/D");
    tOut->Branch("mult_LENDA_t",&m_LENDA_t,"m_LENDA_t[21]/I");
    tOut->Branch("mult_sum_LENDA_t",&m_sum_LENDA_t,"m_sum_LENDA_t/I");

    //LENDA bottom
    tOut->Branch("chan_LENDA_b", &c_LENDA_b, "c_LENDA_b[21]/I");
    tOut->Branch("energy_LENDA_b", &e_LENDA_b, "e_LENDA_b[21]/I");
    tOut->Branch("time_LENDA_b", &t_LENDA_b, "t_LENDA_b[21]/D");
    tOut->Branch("mult_LENDA_b",&m_LENDA_b,"m_LENDA_b[21]/I");
    tOut->Branch("mult_sum_LENDA_b",&m_sum_LENDA_b,"m_sum_LENDA_b/I");
  }
  if (LScin)
  {
    //Liquid Scintillators
    tOut->Branch("chan_LScin", &c_LScin, "c_LScin[5]/I");
    tOut->Branch("energy_LScin", &e_LScin, "e_LScin[5]/I");
    tOut->Branch("time_LScin", &t_LScin, "t_LScin[5]/D");
    tOut->Branch("gate_long_LScin", &gl_LScin, "gl_LScin[5]/D");
    tOut->Branch("gate_short_LScin", &gs_LScin, "gs_LScin[5]/D");
    tOut->Branch("mult_LScin",&m_LScin,"m_LScin[5]/I");
    tOut->Branch("mult_sum_LScin",&m_sum_LScin,"m_sum_LScin/I");
    tOut->Branch("ph_LScin", &ph_LScin, "ph_LScin[5]/D");
  }
  if (mcp)
  {
    tOut->Branch("energy_UMCP", &e_UMCP, "e_UMCP/I");
    tOut->Branch("time_UMCP", &t_UMCP, "t_UMCP/D");
    tOut->Branch("position_UMCP", &pos_UMCP, "pos_UMCP[4]/I");
    tOut->Branch("mult_UMCP",&m_UMCP, "m_UMCP/I");
    tOut->Branch("energy_DMCP", &e_DMCP, "e_DMCP/I");
    tOut->Branch("time_DMCP", &t_DMCP, "t_DMCP/D");
    tOut->Branch("position_DMCP", &pos_DMCP, "pos_DMCP[4]/I");
    tOut->Branch("mult_DMCP",&m_DMCP, "m_DMCP/I");
    tOut->Branch("energy_TAC", &e_TAC, "e_TAC/I");
  }
  if (IC)
  {
    tOut->Branch("mult_IC",&m_IC_dE,"m_IC_dE/I");
    tOut->Branch("energy_IC_raw", &e_IC_dE, "e_IC_dE/I");
    tOut->Branch("energy_IC_cal", &e_IC_dE_cal, "e_IC_dE_cal/D");
    tOut->Branch("time_IC", &t_IC_dE, "t_IC_dE/D");
  }
  if (Si)
  {
    tOut->Branch("energy_Si_raw", &e_Si, "e_Si[64]/I");
    tOut->Branch("energy_Si_cal", &e_Si_cal, "e_Si_cal[64]/D");
    tOut->Branch("time_Si", &t_Si, "t_Si[64]/D");
    tOut->Branch("mult_Si",&m_Si,"m_Si[64]/I");
    tOut->Branch("mult_sum_Si",&m_sum_Si,"m_sum_Si/I");

    tOut->Branch("energy_SiF_raw", &e_SiF, "e_SiF[32]/I");
    tOut->Branch("energy_SiF_cal", &e_SiF_cal, "e_SiF_cal[32]/D");
    tOut->Branch("time_SiF", &t_SiF, "t_SiF[32]/D");
    tOut->Branch("mult_SiF",&m_SiF,"m_SiF[32]/I");
    tOut->Branch("mult_sum_SiF",&m_sum_SiF,"m_sum_SiF/I");
    tOut->Branch("mult_sum_SiF_corr",&m_sum_SiF_corr,"m_sum_SiF_corr/I");

    tOut->Branch("energy_SiB_raw", &e_SiB, "e_SiB[32]/I");
    tOut->Branch("energy_SiB_cal", &e_SiB_cal, "e_SiB_cal[32]/D");
    tOut->Branch("time_SiB", &t_SiB, "t_SiB[32]/D");
    tOut->Branch("mult_SiB",&m_SiB,"m_SiB[32]/I");
    tOut->Branch("mult_sum_SiB",&m_sum_SiB,"m_sum_SiB/I");
    tOut->Branch("mult_sum_SiB_corr",&m_sum_SiB_corr,"m_sum_SiB_corr/I");
  }
  //tOut->Branch("tmax",&tmax,"tmax/D");
  tOut->Branch("trigger",&t_trigger,"t_trigger/D");

 // Get DDAS tree from input file
  fIn->cd();
  TTree *tIn = (TTree*)fIn->Get("dchan");
  DDASEvent *dEvent = new DDASEvent();
  tIn->SetBranchAddress("ddasevent",&dEvent);

  nEntries = tIn->GetEntries();

  std::vector<bool> first_slot = {true,true,true,true,true,true,true,true};

  // Read in data, entry-by-entry
  for (int i=0; i<nEntries; i++)
  // for (int i=260045; i<260100; i++)
  {
    tIn->GetEntry(i);
    // status bar
    if (i % (int)(0.1*nEntries) == 0)
    {
      std::cerr << counter << " ";
      if (counter==0){std::cerr << std::endl;}
      counter --;
    }
    
    // Readout now has correlations, so entries may have multiple events
    nEvents = dEvent->GetNEvents();
    
    // Read in data event-by-event
    for (int j=0; j<nEvents; j++)
    {
      // Get all the input variables we need
      ddaschannel *dchan = dEvent->GetData()[j];
      crate  = dchan->GetCrateID();
      slot   = dchan->GetSlotID();
      chan   = dchan->GetChannelID();
      energy = dchan->GetEnergy();
      time   = dchan->GetTime();
      trace  = dchan->GetTrace();

      deltaT = time - refT;

      // dt_Pulser_FP = 0;
      // double time_temp = 0;

      // If the new time is outside of the timewindow, calculate the multiplicities, fill the tree,
      // save the new timestamp, and set everything back to zero
      if (i==0 || deltaT > timewindow)
      {
        // zero all variable for the first entry in each new time window
        tOut->Fill();

        refT = time;
        //max = 0.0;
        //tmax = 0.0;

        t_RFQ = 0;
        t_EBIT = 0;
        
        if(pulsers)
        {
          e_Pulser_JENSA = 0;
          t_Pulser_JENSA = 0;
          e_Pulser_FP = 0;
          t_Pulser_FP = 0;
	        gl_Pulser_FP = 0;
          gs_Pulser_FP = 0;
          ph_Pulser_FP = 0;
        }
        if(monitors)
        {
          e_raw_SiMon1 = 0;
          e_SiMon1 = 0;
          t_SiMon1 = 0;
          m_SiMon1 = 0;

          e_raw_SiMon2 = 0;
          e_SiMon2 = 0;
          t_SiMon2 = 0;
          m_SiMon2 = 0;
        }
        if(LENDA)
        {
          memset(c_LENDA_t, -2, sizeof(c_LENDA_t));
          memset(e_LENDA_t, 0, sizeof(e_LENDA_t));
          memset(t_LENDA_t, 0, sizeof(t_LENDA_t));
          memset(m_LENDA_t, 0, sizeof(m_LENDA_t));
          m_sum_LENDA_t = 0;
          
          memset(c_LENDA_b, -2, sizeof(c_LENDA_b));
          memset(e_LENDA_b, 0, sizeof(e_LENDA_b));
          memset(t_LENDA_b, 0, sizeof(t_LENDA_b));
          memset(m_LENDA_b, 0, sizeof(m_LENDA_b));
          m_sum_LENDA_b = 0;
        }
        if(LScin)
        {
          memset(c_LScin, 0, sizeof(c_LScin));
          memset(e_LScin, 0, sizeof(e_LScin));
          memset(t_LScin, 0, sizeof(t_LScin));
          memset(gl_LScin, 0, sizeof(gl_LScin));
          memset(gs_LScin, 0, sizeof(gs_LScin));
          memset(ph_LScin, 0, sizeof(ph_LScin));
          memset(m_LScin, 0, sizeof(m_LScin));
          m_sum_LScin = 0;
        }
        if (mcp)
        {
          e_UMCP = 0;
          t_UMCP = 0;
          memset(pos_UMCP, 0, sizeof(pos_UMCP));
          m_UMCP = 0;
          e_DMCP = 0;
          t_DMCP = 0;
          memset(pos_DMCP, 0, sizeof(pos_DMCP));
          m_DMCP = 0;
          e_TAC = 0;
        }
        if (IC)
        {
          /*memset(e_IC_dE, 0, sizeof(e_IC_dE));
          memset(e_IC_dE_cal, 0, sizeof(e_IC_dE_cal));
          memset(t_IC_dE, 0, sizeof(t_IC_dE));*/
          e_IC_dE = 0.0;
          e_IC_dE_cal = 0.0;
          t_IC_dE = 0.0;
          m_IC_dE = 0;
        }
        if (Si)
        {
          memset(e_Si, 0, sizeof(e_Si));
          memset(e_Si_cal, 0.0, sizeof(e_Si_cal));
          memset(t_Si, 0, sizeof(t_Si));
          memset(m_Si, 0, sizeof(m_Si));
          m_sum_Si = 0;
          //m_sum_Si_corr = 0;

          memset(e_SiF, 0, sizeof(e_SiF));
          memset(e_SiF_cal, 0.0, sizeof(e_SiF_cal));
          memset(t_SiF, 0.0, sizeof(t_SiF));
          memset(m_SiF, 0, sizeof(m_SiF));
          m_sum_SiF = 0;
          m_sum_SiF_corr = 0;

          memset(e_SiB, 0, sizeof(e_SiB));
          memset(e_SiB_cal, 0.0, sizeof(e_SiB_cal));
          memset(t_SiB, 0.0, sizeof(t_SiB));
          memset(m_SiB, 0, sizeof(m_SiB));
          m_sum_SiB = 0;
          m_sum_SiB_corr = 0;
        }
      }
      //..................Set tree variables to correct values..................//
      int det_chan = 0;
      int temp_chan = 0;
      int strip_num = -1;

      if(crate==0)
      {
        /*if(slot>=2 && slot<=4)
        {
          if(LENDA)
          {
            if(det_chan%2 == 0)
            {
              temp_chan = det_chan/2;
              if(energy > e_LENDA_t[temp_chan])
              {
                c_LENDA_t[temp_chan] = temp_chan;
                e_LENDA_t[temp_chan] = energy;
                t_LENDA_t[temp_chan] = time;
              }
              m_LENDA_t[temp_chan]++;
              m_sum_LENDA_t++;
            }
            else 
            {
              temp_chan = (det_chan-1)/2;
              if(energy > e_LENDA_b[temp_chan])
              {
                c_LENDA_b[temp_chan] = temp_chan;
                e_LENDA_b[temp_chan] = energy;
                t_LENDA_b[temp_chan] = time;
              }
              m_LENDA_b[temp_chan]++;
              m_sum_LENDA_b++;
            }
          }
        }*/
        //if(slot==4)
        if(slot==3)
        {
          if(monitors)
          {
            //if(chan==14) //SiMon1
            if(chan==10)
            {
              if(energy>0)
              {
                //double r = (rand()%100)/100;
                //double energy_cal = (energy+r)*SiMon1_gain + SiMon1_offset;
                e_raw_SiMon1 = energy;
                e_SiMon1 = energy; //energy_cal
                t_SiMon1 = time;
                m_SiMon1++;
              }
            }
            //else if(chan==15) //SiMon2
            else if(chan==12) //SiMon2
            {
              if(energy>0)
              {
                //double r = (rand()%100)/100;
                //double energy_cal = (energy+r)*SiMon2_gain + SiMon2_offset;
                e_raw_SiMon2 = energy;
                e_SiMon2 = energy; //energy_cal;
                t_SiMon2 = time;
                m_SiMon2++;
              }
            }
          }
          /*if(chan==10) //JENSA pulser
          {
            e_Pulser_JENSA = energy;
            t_Pulser_JENSA = time;
          }
          else if(chan==3) //11
          {
            t_RFQ = time;
          }
          else if(chan==4) //12
          {
            t_EBIT = time;
          }*/
        }
        else if(slot==5)
        {
          if(LScin)
          {
            if(energy > 0)  
            {
              c_LScin[chan] = chan;
              e_LScin[chan] = energy;
              t_LScin[chan] = time;
              if(trace.size()>0)
              {
                vector<double> temp_gates = analysePulseShape(trace);
                gs_LScin[chan] = temp_gates[0];
                gl_LScin[chan] = temp_gates[1];
                ph_LScin[chan] = temp_gates[2];
              }
            }
            m_LScin[chan]++;
            m_sum_LScin++;
          }
        } 
      }
      else if(crate==1)
      {
        if(slot==2)
        {
          if (IC)
          {
            if(chan==13)//IC_dE
            {
              double r = (rand()%100)/100;
              double energy_cal = (energy+r)*IC_dE_gain + IC_dE_offset;
              e_IC_dE = energy;
              e_IC_dE_cal = energy_cal;
              t_IC_dE = time;
              m_IC_dE++;
              
            }
          }
          if(mcp)
          { //(channel 0 and 3 are broken)
      	    if(chan==1) //UMCP timing
      	    {	
      		    e_UMCP = energy;
              t_UMCP = time;
              m_UMCP++;
      	    }
            if(chan==2) //DMCP timing
      	    {
		          e_DMCP = energy;
              t_DMCP = time;
              m_DMCP++;
            }
            if(chan>3 && chan<8) //Positions Upstream A - D 
            { 
              if(energy>0) pos_UMCP[chan-4] = energy;
            }
            if(chan>7 && chan<12) //Positions Downstream A - D
            {
              if(energy>0) pos_DMCP[chan-8] = energy;
            }
            if(chan==12) //TAC timing
            {
              e_TAC = energy;
            }
          }
          if(pulsers)
          {
            if(chan==15) //FP pulser
            {
              e_Pulser_FP = energy;
              t_Pulser_FP = time;
	            if(trace.size()>0)
              {
          		  gl_Pulser_FP;
          	    gs_Pulser_FP;
            		ph_Pulser_FP;
          		}
            }
          }
        }
        else if(slot>=3 && slot<=6)
        {
          double energy_cal = 0;
          if(Si)
          {
            
            det_chan = (slot-3)*16+chan;
            double r = (rand()%100)/100;
            int max_chan = 0;
            strip_num = Si_strip[det_chan]; // convert detector channel (0-64) to strip number (0-32)
            
            //Because of the 2nd order equation used for the calibration anything above the maximum will flip and appear at a low energy
            //We exclude these events as we don't expect any useful signals at high channels
            
            //energy_cal = (energy*energy)*p2[det_chan] + (energy+r)*p1[det_chan] + p0[det_chan]; //p2x^2+p1x+p0=0
            energy_cal = energy;
            
            if(energy>0) 
            {
              // Separate the events per side and per strip
              //Note: Below I use the strip_num as the counter because I have set the variables as 32 sized arrays. 
              //The gain_offset/calibration parameters though is a 64 and should be assigned to the det_chan counter that is of size 64.
              if(slot==3 || slot==4) // Front
              { 
                e_SiF[strip_num] = energy;
                e_SiF_cal[strip_num] = energy_cal;
                t_SiF[strip_num] = time;
                m_SiF[strip_num]++;
                m_sum_SiF++;
                if(energy_cal>1 && strip_num>1 && strip_num<31) m_sum_SiF_corr++;
              }
              else if(slot==5 || slot==6) // Back
              { 
                e_SiB[strip_num] = energy;
                e_SiB_cal[strip_num] = energy_cal;
                t_SiB[strip_num] = time;
                m_SiB[strip_num]++;
                m_sum_SiB++;
              }

              e_Si[det_chan] = energy;
              t_Si[det_chan] = time;
              m_Si[det_chan]++; 
              if(e_SiF_cal[strip_num]>1 && e_SiB_cal[strip_num]>5)
              {
                e_Si_cal[det_chan] = energy_cal; 
                m_sum_Si++; 
              }
            }
          }
        }
      }
      t_trigger = refT; // Trigger time
    } // finish loop over j=events
  } // finish loop over i=entries
  
  // Write to output file
  fOut->cd();
  tOut->Write();
  fOut->Close();
  return 0;
}
