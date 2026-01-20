vector<double> analysePulseShape(vector<uint16_t> trace)
{
    int size = trace.size();

    std::vector<int> x(size);
    std::vector<int> y(size);

    int maxY = 0;
	int maxX = 0;
    double pulse_h = 0.;

    for (int i=0; i<size; i++)
    {   // Loop over the whole trace
        x[i] = i;
        y[i] = trace[i];
        
        if(y[i]>maxY)
        {   //Find the highest value of the trace
            maxY = y[i]; //assign the maximum trace value
            maxX = i; //assign the bin of the maximum trace value
            pulse_h = y[i];
        }
    }
    
    double baseline = 0.;
    for(int j=(size-100); j<size; j++) 
    { //Set the baseline by taking the part of the trace after the peak
        baseline = y[j]+baseline; 
    }
    baseline = baseline/100; //calculate the baseline per bin
      
    double shortG=0.0, longG=0.0; 
    for(int k=maxX-3; k<(maxX+20); k++)
    {   
        //120 //Loop over the peak of the trace starting from 4 bins before the maximum and finish 45 bins after
        if(k>(maxX+7)) shortG = shortG + y[k] - baseline; //Changed to set the short gate as the tail of the peak...
        //if(k<maxX+14) shortG = shortG + y[k] - baseline;//Set the short gate as the 4+14 bins around the peak 
        longG = longG + y[k] - baseline; //Set the long gate as 4+45 bins around the peak
        //These numbers were resulted by the work done with the Cf source and can be optimized using the equation of merit
        //if(k>maxX && y[k]-baseline<-5.) break;
    }
    // double psd = (double)(b-a)/b;
    vector<double> gates;
    //if(shortG>0.0 && longG>0.0)
    //{
        gates.push_back(shortG);
        gates.push_back(longG);
        gates.push_back(pulse_h);
    //}

    // cout << gates[0] << "\t" << gates[1] << "\t" << gates[2] << endl; 

    return gates;
}