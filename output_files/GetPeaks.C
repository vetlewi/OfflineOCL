

void GetPeaks()
{
	TFile *file = TFile::Open("beam_Dy.root");

	TH2 *m = (TH2 *)file->Get("de_align_time");

	TSpectrum spec;

	char tmp[1024];

	double peakPos[1024];


	for (int i = 1 ; i < 65 ; ++i ){
		sprintf(tmp, "px_%d", i);
		TH1 *h = m->ProjectionX(tmp, i, i);
		h->Draw();
		spec.Search(h);
		int n_peaks = spec.GetNPeaks();
		cout << i << ": " << n_peaks << endl;
		//peakPos[i-1] = spec.GetPositionX()[0];

		double param[3] = {200., spec.GetPositionX()[0], 1.0};
		TF1 *fit = new TF1("total","gaus(0)",spec.GetPositionX()[0]-1.5,spec.GetPositionX()[0]+2.0);
		
		for (Int_t k=0; k<3; k++) {
        fit->SetParameter(k, param[k]);
    }

		h->Fit("total", "bR");
		peakPos[i-1] = fit->GetParameter(1);
		fit->Draw("same");

	}

	for (int i = 0 ; i < 64 ; ++i){
		cout << -peakPos[i] << " ";
	}

	cout << endl;



}