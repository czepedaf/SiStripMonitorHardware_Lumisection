#include "DataFormats/FEDRawData/interface/FEDNumbering.h"
#include "DataFormats/SiStripCommon/interface/SiStripFedKey.h"
#include "DataFormats/SiStripCommon/interface/ConstantsForHardwareSystems.h"

#include "DQM/SiStripMonitorHardware/interface/CMHistograms.hh"



CMHistograms::CMHistograms()
{
  dqm_ = 0;

  for (unsigned int i(0); i<500; i++){
    doFed_[i] = false;
  }

}

CMHistograms::~CMHistograms()
{
}
  
void CMHistograms::initialise(const edm::ParameterSet& iConfig,
			      std::ostringstream* pDebugStream
			      )
{
  getConfigForHistogram(medianAPV1vsAPV0_,"MedianAPV1vsAPV0",iConfig,pDebugStream);
  getConfigForHistogram(medianAPV0minusAPV1_,"MedianAPV0minusAPV1",iConfig,pDebugStream);

  getConfigForHistogram(meanCMPerFedvsFedId_,"MeanCMPerFedvsFedId",iConfig,pDebugStream);
  getConfigForHistogram(meanCMPerFedvsTime_,"MeanCMPerFedvsTime",iConfig,pDebugStream);
  getConfigForHistogram(meanCMPerFedvsLumisection_,"MeanCMPerFedvsLumisection",iConfig,pDebugStream);
  getConfigForHistogram(variationsPerFedvsFedId_,"VariationsPerFedvsFedId",iConfig,pDebugStream);
  getConfigForHistogram(variationsPerFedvsTime_,"VariationsPerFedvsTime",iConfig,pDebugStream);
  getConfigForHistogram(variationsPerFedvsLumisection_,"VariationsPerFedvsLumisection",iConfig,pDebugStream);

  getConfigForHistogram(medianAPV1vsAPV0perFED_,"MedianAPV1vsAPV0perFED",iConfig,pDebugStream);
  getConfigForHistogram(medianAPV0minusAPV1perFED_,"MedianAPV0minusAPV1perFED",iConfig,pDebugStream);

  getConfigForHistogram(medianperChannel_,"MedianperChannel",iConfig,pDebugStream);
  getConfigForHistogram(medianAPV0minusAPV1perChannel_,"MedianAPV0minusAPV1perChannel",iConfig,pDebugStream);

  
  getConfigForHistogram(tkMapConfig_,"TkHistoMap",iConfig,pDebugStream);

  if (iConfig.exists("FedIdVec")){
    std::vector<unsigned int> lIdVec = iConfig.getUntrackedParameter<std::vector<unsigned int> >("FedIdVec");
    for (unsigned int i(0); i<lIdVec.size(); i++){
      if (lIdVec.at(i) < 500) doFed_[lIdVec.at(i)] = true;
    }
  }
}

void CMHistograms::fillHistograms(const std::vector<CMvalues>& aVec, float aTime,unsigned int aFedId) //float aLumisection,unsigned int aFedId)
{

  if (doFed_[aFedId]){
    bookFEDHistograms(aFedId);
  }

  float lMean = 0;
  float lPrevMean = 0;

  for (unsigned iEle(0); iEle<aVec.size(); iEle++){//loop on elements

    CMvalues lVal = aVec[iEle];

    if (lVal.Medians.first >= 1024 || lVal.Medians.second >= 1024) {
      std::cout << "----- WARNING ! New max found: " << lVal.Medians.first << " " << lVal.Medians.second << " " << __FILE__ << " " << __LINE__ << std::endl;
    }

    fillHistogram(medianAPV1vsAPV0_,lVal.Medians.first,lVal.Medians.second);
    fillHistogram(medianAPV0minusAPV1_,lVal.Medians.first-lVal.Medians.second);

    lMean += lVal.Medians.first + lVal.Medians.second;
    lPrevMean += lVal.PreviousMedians.first + lVal.PreviousMedians.second;

    if (doFed_[aFedId]) {
      fillHistogram(medianAPV1vsAPV0perFEDMap_[aFedId],lVal.Medians.first,lVal.Medians.second);
      fillHistogram(medianAPV0minusAPV1perFEDMap_[aFedId],lVal.Medians.first-lVal.Medians.second);
      fillHistogram(medianperChannelMap_[aFedId][lVal.ChannelID],lVal.Medians.first);
      fillHistogram(medianAPV0minusAPV1perChannelMap_[aFedId][lVal.ChannelID],lVal.Medians.first-lVal.Medians.second);
    }

  }//loop on elements

  if (aVec.size() > 0) {
    lMean = lMean / (2*aVec.size());
    lPrevMean = lPrevMean / (2*aVec.size());
  }

  fillHistogram(meanCMPerFedvsFedId_,aFedId,lMean);
  fillHistogram(meanCMPerFedvsTime_,aTime,lMean);
  //  fillHistogram(meanCMPerFedvsLumisection_,aLumisection,lMean);
  fillHistogram(variationsPerFedvsFedId_,aFedId,lMean-lPrevMean);
  fillHistogram(variationsPerFedvsTime_,aTime,lMean-lPrevMean);
  //  fillHistogram(variationsPerFedvsLumisection_,aLumisection,lMean-lPrevMean);


}


void CMHistograms::bookTopLevelHistograms(DQMStore* dqm)
{
  //get the pointer to the dqm object
  dqm_ = dqm;


  //book FED level histograms
  //get FED IDs
  //const unsigned int siStripFedIdMin = FEDNumbering::MINSiStripFEDID;
  const unsigned int siStripFedIdMax = FEDNumbering::MAXSiStripFEDID;
  histosBooked_.resize(siStripFedIdMax+1,false);

  std::string lDir = dqm_->pwd()+"/";

  dqm_->cd(lDir);

  //std::cout << "Folder: " << lDir+categories_[i] << std::endl;
    
  dqm_->setCurrentFolder(lDir);

  book2DHistogram(medianAPV1vsAPV0_,
		  "MedianAPV1vsAPV0",
		  "median APV1 vs APV0",
		  250,0,1024,250,0,1024,
		  "median APV0","median APV1");

  bookHistogram(medianAPV0minusAPV1_,
		"MedianAPV0minusAPV1",
		"median APV0 - median APV1",
		500,-500,500,
		"median APV0 - median APV1");
  

  bookProfile(meanCMPerFedvsFedId_,
	      "MeanCMPerFedvsFedId",
	      "<CM> vs fedID",
	      440,50,490,-1000,1000,
	      "fedID","<CM>^{FED}");
  
  bookProfile(meanCMPerFedvsTime_,
	      "MeanCMPerFedvsTime",
	      "<CM> vs time",
	      0,1000,
	      "Time","<CM>^{FED}");

  bookProfile(meanCMPerFedvsLumisection_,
	      "MeanCMPerFedvsLumisection",
	      "<CM> vs lumisection",
	      0,1000,
	      "Lumisection","<CM>^{FED}");
  
  bookProfile(variationsPerFedvsFedId_,
	      "VariationsPerFedvsFedId",
	      "<CM> vs fedID",
	      440,50,490,-1000,1000,
	      "fedID","<CM>^{FED}_{t}-<CM>^{FED}_{t-1}");
  
  bookProfile(variationsPerFedvsTime_,
	      "VariationsPerFedvsTime",
	      "<CM> vs time",
	      0,1000,
	      "Time","<CM>^{FED}_{t}-<CM>^{FED}_{t-1}");

  bookProfile(variationsPerFedvsLumisection_,
	      "VariationsPerFedvsLumisection",
	      "<CM> vs lumisection",
	      0,1000,
	      "Lumisection","<CM>^{FED}_{t}-<CM>^{FED}_{t-1}");
  

  
  dqm_->cd(lDir);
    
  //book map after, as it creates a new folder...
  if (tkMapConfig_.enabled){
    //const std::string dqmPath = dqm_->pwd();
    tkmapCM_[0] = new TkHistoMap("SiStrip/TkHisto","TkHMap_MeanCMAPV",0.,500);
    tkmapCM_[1] = new TkHistoMap("SiStrip/TkHisto","TkHMap_RmsCMAPV",0.,500);
    tkmapCM_[2] = new TkHistoMap("SiStrip/TkHisto","TkHMap_MeanCMAPV0minusAPV1",-500.,500);
    tkmapCM_[3] = new TkHistoMap("SiStrip/TkHisto","TkHMap_RmsCMAPV0minusAPV1",-500.,500);
  }
  else {
    tkmapCM_[0] = 0;
    tkmapCM_[1] = 0;
    tkmapCM_[2] = 0;
    tkmapCM_[3] = 0;
  }


}


void CMHistograms::bookFEDHistograms(unsigned int fedId)
{
  if (!histosBooked_[fedId]) {
    //will do that only once

    SiStripFedKey fedKey(fedId,0,0,0);
    std::stringstream fedIdStream;
    fedIdStream << fedId;

    dqm_->setCurrentFolder(fedKey.path());
    
    book2DHistogram(medianAPV1vsAPV0perFED_,
		    medianAPV1vsAPV0perFEDMap_[fedId],
		    "MedianAPV1vsAPV0forFED"+fedIdStream.str(),
		    "median APV1 vs APV0 for FED "+fedIdStream.str(),
		    250,0,500,250,0,500,
		    "APV0","APV1");
    
    bookHistogram(medianAPV0minusAPV1perFED_,
		  medianAPV0minusAPV1perFEDMap_[fedId],
		  "MedianAPV0minusAPV1forFED"+fedIdStream.str(),
		  "median APV0 - median APV1 for FED "+fedIdStream.str(),
		  500,-500,500,
		  "#Delta(medians)");


    bookChannelsHistograms(fedId);

    histosBooked_[fedId] = true;
  }
}

void CMHistograms::bookChannelsHistograms(unsigned int fedId)
{
  SiStripFedKey fedKey(fedId,0,0,0);
  std::stringstream fedIdStream;
  fedIdStream << fedId;

  dqm_->setCurrentFolder(fedKey.path());
  medianperChannelMap_[fedId].resize(sistrip::FEDCH_PER_FED,0);
  medianAPV0minusAPV1perChannelMap_[fedId].resize(sistrip::FEDCH_PER_FED,0);

  for (unsigned int iCh(0); iCh < sistrip::FEDCH_PER_FED; iCh++){

    std::ostringstream lName0,lTitle0,lName1,lTitle1,lName2,lTitle2;
    lName0 << "MedianForFed" << fedId << "Channel" << iCh;
    lTitle0 << "Median for FED/Ch " << fedId << "/" << iCh ;
    lName2 << "MedianAPV0minusAPV1ForFed" << fedId << "Channel" << iCh;
    lTitle2 << "Median APV0-APV1 for FED/Ch " << fedId << "/" << iCh ;

    bookHistogram(medianperChannel_,
		  medianperChannelMap_[fedId][iCh],
		  lName0.str(),
		  lTitle0.str(),
		  250,0,500,
		  "median APVs");
    

    bookHistogram(medianAPV0minusAPV1perChannel_,
		  medianAPV0minusAPV1perChannelMap_[fedId][iCh],
		  lName2.str(),
		  lTitle2.str(),
		  250,-500,500,
		  "median APV0-APV1");
    
  }

}

void CMHistograms::bookAllFEDHistograms()
{
  //get FED IDs
  const unsigned int siStripFedIdMin = FEDNumbering::MINSiStripFEDID;
  const unsigned int siStripFedIdMax = FEDNumbering::MAXSiStripFEDID;
  //book them
  for (unsigned int iFed = siStripFedIdMin; iFed <= siStripFedIdMax; iFed++) {
    bookFEDHistograms(iFed);
  }
}

bool CMHistograms::tkHistoMapEnabled(unsigned int aIndex){
  return tkMapConfig_.enabled;
}

TkHistoMap * CMHistograms::tkHistoMapPointer(unsigned int aIndex){
  assert(aIndex < 4);
  return tkmapCM_[aIndex];
}
