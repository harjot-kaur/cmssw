// $Id: TtHadKinFitter.cc,v 1.2 2007/10/07 11:20:07 mfhansen Exp $ 

#include "TopQuarkAnalysis/TopKinFitter/interface/TtHadKinFitter.h"

#include "PhysicsTools/KinFitter/interface/TKinFitter.h"
#include "PhysicsTools/KinFitter/interface/TAbsFitParticle.h"
#include "PhysicsTools/KinFitter/interface/TFitConstraintM.h"
#include "PhysicsTools/KinFitter/interface/TFitParticleEMomDev.h"
#include "PhysicsTools/KinFitter/interface/TFitParticleEScaledMomDev.h"
#include "PhysicsTools/KinFitter/interface/TFitParticleEtEtaPhi.h"
#include "PhysicsTools/KinFitter/interface/TFitParticleEtThetaPhi.h"
/* other parametrizations and constraints - for later maybe?
#include "PhysicsTools/KinFitter/interface/TFitParticleESpher.h"
#include "PhysicsTools/KinFitter/interface/TFitParticleMCPInvSpher.h"
#include "PhysicsTools/KinFitter/interface/TFitConstraintMGaus.h"
#include "PhysicsTools/KinFitter/interface/TFitConstraintEp.h"*/


/// default constructor
TtHadKinFitter::TtHadKinFitter() :
    jetParam_(EMom), maxNrIter_(200), maxDeltaS_(5e-5), maxF_(1e-4) {
  setupFitter();
}


/// constructor from configurables
TtHadKinFitter::TtHadKinFitter(int jetParam, int maxNrIter, double maxDeltaS, double maxF, 
			       std::vector<int> constraints) :
    jetParam_((Parametrization) jetParam), maxNrIter_(maxNrIter), maxDeltaS_(maxDeltaS),
    maxF_(maxF), constraints_(constraints) {
  setupFitter();
}


/// constructor from configurables
TtHadKinFitter::TtHadKinFitter(Parametrization jetParam, int maxNrIter, double maxDeltaS, 
			       double maxF, std::vector<int> constraints) :
    jetParam_(jetParam), maxNrIter_(maxNrIter), maxDeltaS_(maxDeltaS), maxF_(maxF), 
    constraints_(constraints) {
  setupFitter();
}

/// destructor
TtHadKinFitter::~TtHadKinFitter() {
  delete cons1_; delete cons2_; delete cons3_; delete cons4_;
  delete fitHadb_; delete fitHadp_; delete fitHadq_;
  delete fitHadbbar_; delete fitHadj_; delete fitHadk_;
  delete theFitter_;
}


/// runs the fitter, and adds the altered kinematics to the event solution
TtHadEvtSolution TtHadKinFitter::addKinFitInfo(TtHadEvtSolution * asol) {

  TtHadEvtSolution fitsol(*asol);

  TMatrixD m1(3,3),  m2(3,3),  m3(3,3),  m4(3,3);
  TMatrixD m1b(4,4), m2b(4,4), m3b(4,4), m4b(4,4);
  TMatrixD m5(3,3),  m5b(3,3), m6(3,3), m6b(3,3);
  m1.Zero();  m2.Zero();  m3.Zero();  m4.Zero();
  m1b.Zero(); m2b.Zero(); m3b.Zero(); m4b.Zero();
  m5.Zero();  m5b.Zero(), m6.Zero(), m6b.Zero();

  // initialize particles
  TLorentzVector hadpVec(fitsol.getCalHadp().px(), fitsol.getCalHadp().py(),
                         fitsol.getCalHadp().pz(), fitsol.getCalHadp().energy());
  TLorentzVector hadqVec(fitsol.getCalHadq().px(), fitsol.getCalHadq().py(),
                      	 fitsol.getCalHadq().pz(), fitsol.getCalHadq().energy());
  TLorentzVector hadbVec(fitsol.getCalHadb().px(), fitsol.getCalHadb().py(),
                         fitsol.getCalHadb().pz(), fitsol.getCalHadb().energy());
  TLorentzVector hadjVec(fitsol.getCalHadj().px(), fitsol.getCalHadj().py(),
                         fitsol.getCalHadj().pz(), fitsol.getCalHadj().energy());
  TLorentzVector hadkVec(fitsol.getCalHadk().px(), fitsol.getCalHadk().py(),
                      	 fitsol.getCalHadk().pz(), fitsol.getCalHadk().energy());
  TLorentzVector hadbbarVec(fitsol.getCalHadbbar().px(), fitsol.getCalHadbbar().py(),
			    fitsol.getCalHadbbar().pz(), fitsol.getCalHadbbar().energy());
  // jet resolutions
  if (jetParam_ == EMom) {
    m1b(0,0) = pow(fitsol.getCalHadp().getResA(), 2);
    m1b(1,1) = pow(fitsol.getCalHadp().getResB(), 2);
    m1b(2,2) = pow(fitsol.getCalHadp().getResC(), 2);
    m1b(3,3) = pow(fitsol.getCalHadp().getResD(), 2);
    m2b(0,0) = pow(fitsol.getCalHadq().getResA(), 2); 
    m2b(1,1) = pow(fitsol.getCalHadq().getResB(), 2); 
    m2b(2,2) = pow(fitsol.getCalHadq().getResC(), 2);
    m2b(3,3) = pow(fitsol.getCalHadq().getResD(), 2);
    m3b(0,0) = pow(fitsol.getCalHadb().getResA(), 2); 
    m3b(1,1) = pow(fitsol.getCalHadb().getResB(), 2); 
    m3b(2,2) = pow(fitsol.getCalHadb().getResC(), 2);
    m3b(3,3) = pow(fitsol.getCalHadb().getResD(), 2);
    m4b(0,0) = pow(fitsol.getCalHadj().getResA(), 2);
    m4b(1,1) = pow(fitsol.getCalHadj().getResB(), 2);
    m4b(2,2) = pow(fitsol.getCalHadj().getResC(), 2);
    m4b(3,3) = pow(fitsol.getCalHadj().getResD(), 2);
    m5b(0,0) = pow(fitsol.getCalHadk().getResA(), 2); 
    m5b(1,1) = pow(fitsol.getCalHadk().getResB(), 2); 
    m5b(2,2) = pow(fitsol.getCalHadk().getResC(), 2);
    m5b(3,3) = pow(fitsol.getCalHadk().getResD(), 2);
    m6b(0,0) = pow(fitsol.getCalHadbbar().getResA(), 2); 
    m6b(1,1) = pow(fitsol.getCalHadbbar().getResB(), 2); 
    m6b(2,2) = pow(fitsol.getCalHadbbar().getResC(), 2);
    m6b(3,3) = pow(fitsol.getCalHadbbar().getResD(), 2);
  } else if (jetParam_ == EtEtaPhi) {
    m1(0,0) = pow(fitsol.getCalHadp().getResET(), 2);
    m1(1,1) = pow(fitsol.getCalHadp().getResEta(), 2);
    m1(2,2) = pow(fitsol.getCalHadp().getResPhi(), 2);
    m2(0,0) = pow(fitsol.getCalHadq().getResET(), 2); 
    m2(1,1) = pow(fitsol.getCalHadq().getResEta(), 2); 
    m2(2,2) = pow(fitsol.getCalHadq().getResPhi(), 2);
    m3(0,0) = pow(fitsol.getCalHadb().getResET(), 2); 
    m3(1,1) = pow(fitsol.getCalHadb().getResEta(), 2); 
    m3(2,2) = pow(fitsol.getCalHadb().getResPhi(), 2);
    m4(0,0) = pow(fitsol.getCalHadj().getResET(), 2);
    m4(1,1) = pow(fitsol.getCalHadj().getResEta(), 2);
    m4(2,2) = pow(fitsol.getCalHadj().getResPhi(), 2);
    m5(0,0) = pow(fitsol.getCalHadk().getResET(), 2); 
    m5(1,1) = pow(fitsol.getCalHadk().getResEta(), 2); 
    m5(2,2) = pow(fitsol.getCalHadk().getResPhi(), 2);
    m6(0,0) = pow(fitsol.getCalHadbbar().getResET(), 2); 
    m6(1,1) = pow(fitsol.getCalHadbbar().getResEta(), 2); 
    m6(2,2) = pow(fitsol.getCalHadbbar().getResPhi(), 2);
  } else if (jetParam_ == EtThetaPhi) {
    m1(0,0) = pow(fitsol.getCalHadp().getResET(), 2);
    m1(1,1) = pow(fitsol.getCalHadp().getResTheta(), 2);
    m1(2,2) = pow(fitsol.getCalHadp().getResPhi(), 2);
    m2(0,0) = pow(fitsol.getCalHadq().getResET(), 2); 
    m2(1,1) = pow(fitsol.getCalHadq().getResTheta(), 2); 
    m2(2,2) = pow(fitsol.getCalHadq().getResPhi(), 2);
    m3(0,0) = pow(fitsol.getCalHadb().getResET(), 2); 
    m3(1,1) = pow(fitsol.getCalHadb().getResTheta(), 2); 
    m3(2,2) = pow(fitsol.getCalHadb().getResPhi(), 2);
    m4(0,0) = pow(fitsol.getCalHadj().getResET(), 2);
    m4(1,1) = pow(fitsol.getCalHadj().getResTheta(), 2);
    m4(2,2) = pow(fitsol.getCalHadj().getResPhi(), 2);
    m5(0,0) = pow(fitsol.getCalHadk().getResET(), 2); 
    m5(1,1) = pow(fitsol.getCalHadk().getResTheta(), 2); 
    m5(2,2) = pow(fitsol.getCalHadk().getResPhi(), 2);
    m6(0,0) = pow(fitsol.getCalHadbbar().getResET(), 2); 
    m6(1,1) = pow(fitsol.getCalHadbbar().getResTheta(), 2); 
    m6(2,2) = pow(fitsol.getCalHadbbar().getResPhi(), 2);
  }
  
  // set the kinematics of the objects to be fitted
  fitHadp_->setIni4Vec(&hadpVec);
  fitHadq_->setIni4Vec(&hadqVec);
  fitHadb_->setIni4Vec(&hadbVec);
  fitHadj_->setIni4Vec(&hadjVec);
  fitHadk_->setIni4Vec(&hadkVec);
  fitHadbbar_->setIni4Vec(&hadbbarVec);
  
  if (jetParam_ == EMom) {
    fitHadp_->setCovMatrix(&m1b);
    fitHadq_->setCovMatrix(&m2b);
    fitHadb_->setCovMatrix(&m3b);
    fitHadj_->setCovMatrix(&m4b);
    fitHadk_->setCovMatrix(&m5b);
    fitHadbbar_->setCovMatrix(&m6b);
  } else {
    fitHadp_->setCovMatrix(&m1);
    fitHadq_->setCovMatrix(&m2);
    fitHadb_->setCovMatrix(&m3);
    fitHadj_->setCovMatrix(&m4);
    fitHadk_->setCovMatrix(&m5);
    fitHadbbar_->setCovMatrix(&m6);
  }
  
  // perform the fit!
  theFitter_->fit();
  
  // add fitted information to the solution
  if (theFitter_->getStatus() == 0) {
    // read back the jet kinematics and resolutions
    TopParticle aFitHadp(reco::Particle(0, math::XYZTLorentzVector(fitHadp_->getCurr4Vec()->X(), fitHadp_->getCurr4Vec()->Y(), fitHadp_->getCurr4Vec()->Z(), fitHadp_->getCurr4Vec()->E()), math::XYZPoint()));
    TopParticle aFitHadq(reco::Particle(0, math::XYZTLorentzVector(fitHadq_->getCurr4Vec()->X(), fitHadq_->getCurr4Vec()->Y(), fitHadq_->getCurr4Vec()->Z(), fitHadq_->getCurr4Vec()->E()), math::XYZPoint()));
    TopParticle aFitHadb(reco::Particle(0, math::XYZTLorentzVector(fitHadb_->getCurr4Vec()->X(), fitHadb_->getCurr4Vec()->Y(), fitHadb_->getCurr4Vec()->Z(), fitHadb_->getCurr4Vec()->E()), math::XYZPoint()));
    TopParticle aFitHadj(reco::Particle(0, math::XYZTLorentzVector(fitHadj_->getCurr4Vec()->X(), fitHadj_->getCurr4Vec()->Y(), fitHadj_->getCurr4Vec()->Z(), fitHadj_->getCurr4Vec()->E()), math::XYZPoint()));
    TopParticle aFitHadk(reco::Particle(0, math::XYZTLorentzVector(fitHadk_->getCurr4Vec()->X(), fitHadk_->getCurr4Vec()->Y(), fitHadk_->getCurr4Vec()->Z(), fitHadk_->getCurr4Vec()->E()), math::XYZPoint()));
    TopParticle aFitHadbbar(reco::Particle(0, math::XYZTLorentzVector(fitHadbbar_->getCurr4Vec()->X(), fitHadbbar_->getCurr4Vec()->Y(), fitHadbbar_->getCurr4Vec()->Z(), fitHadbbar_->getCurr4Vec()->E()), math::XYZPoint()));
    
    if (jetParam_ == EMom) {
      TMatrixD Vp(4,4);  Vp  = (*fitHadp_->getCovMatrixFit()); 
      TMatrixD Vq(4,4);  Vq  = (*fitHadq_->getCovMatrixFit()); 
      TMatrixD Vbh(4,4); Vbh = (*fitHadb_->getCovMatrixFit()); 
      TMatrixD Vj(4,4);  Vj  = (*fitHadj_->getCovMatrixFit()); 
      TMatrixD Vk(4,4);  Vk  = (*fitHadk_->getCovMatrixFit()); 
      TMatrixD Vbbar(4,4); Vbbar = (*fitHadbbar_->getCovMatrixFit()); 
      
      aFitHadp.setCovM(this->translateCovM(Vp));
      aFitHadq.setCovM(this->translateCovM(Vq));
      aFitHadb.setCovM(this->translateCovM(Vbh));
      aFitHadj.setCovM(this->translateCovM(Vj));
      aFitHadk.setCovM(this->translateCovM(Vk));
      aFitHadbbar.setCovM(this->translateCovM(Vbbar));
      
      aFitHadp.setResA(sqrt(Vp(0,0)));  
      aFitHadp.setResB(sqrt(Vp(1,1)));
      aFitHadp.setResC(sqrt(Vp(2,2))); 
      aFitHadp.setResD(sqrt(Vp(3,3))); 
      aFitHadq.setResA(sqrt(Vq(0,0)));  
      aFitHadq.setResB(sqrt(Vq(1,1)));
      aFitHadq.setResC(sqrt(Vq(2,2)));
      aFitHadq.setResD(sqrt(Vq(3,3)));
      aFitHadb.setResA(sqrt(Vbh(0,0)));  
      aFitHadb.setResB(sqrt(Vbh(1,1)));
      aFitHadb.setResC(sqrt(Vbh(2,2)));
      aFitHadb.setResD(sqrt(Vbh(3,3)));
      aFitHadj.setResA(sqrt(Vj(0,0)));  
      aFitHadj.setResB(sqrt(Vj(1,1)));
      aFitHadj.setResC(sqrt(Vj(2,2))); 
      aFitHadj.setResD(sqrt(Vj(3,3))); 
      aFitHadk.setResA(sqrt(Vk(0,0)));  
      aFitHadk.setResB(sqrt(Vk(1,1)));
      aFitHadk.setResC(sqrt(Vk(2,2)));
      aFitHadk.setResD(sqrt(Vk(3,3)));
      aFitHadbbar.setResA(sqrt(Vbbar(0,0)));  
      aFitHadbbar.setResB(sqrt(Vbbar(1,1)));
      aFitHadbbar.setResC(sqrt(Vbbar(2,2)));
      aFitHadbbar.setResD(sqrt(Vbbar(3,3)));

    } else if (jetParam_ == EtEtaPhi) {
      TMatrixD Vp(3,3);  Vp  = (*fitHadp_->getCovMatrixFit()); 
      TMatrixD Vq(3,3);  Vq  = (*fitHadq_->getCovMatrixFit()); 
      TMatrixD Vbh(3,3); Vbh = (*fitHadb_->getCovMatrixFit()); 
      TMatrixD Vj(3,3);  Vj  = (*fitHadj_->getCovMatrixFit()); 
      TMatrixD Vk(3,3);  Vk  = (*fitHadk_->getCovMatrixFit()); 
      TMatrixD Vbbar(3,3); Vbbar = (*fitHadbbar_->getCovMatrixFit()); 

      aFitHadp.setCovM(this->translateCovM(Vp));
      aFitHadq.setCovM(this->translateCovM(Vq));
      aFitHadb.setCovM(this->translateCovM(Vbh));
      aFitHadj.setCovM(this->translateCovM(Vj));
      aFitHadk.setCovM(this->translateCovM(Vk));
      aFitHadbbar.setCovM(this->translateCovM(Vbbar));

      aFitHadp.setResET (sqrt(Vp(0,0)));  
      aFitHadp.setResEta(sqrt(Vp(1,1)));
      aFitHadp.setResPhi(sqrt(Vp(2,2)));
      aFitHadq.setResET (sqrt(Vq(0,0)));  
      aFitHadq.setResEta(sqrt(Vq(1,1)));
      aFitHadq.setResPhi(sqrt(Vq(2,2)));
      aFitHadb.setResET (sqrt(Vbh(0,0)));  
      aFitHadb.setResEta(sqrt(Vbh(1,1)));
      aFitHadb.setResPhi(sqrt(Vbh(2,2)));
      aFitHadj.setResET (sqrt(Vj(0,0)));  
      aFitHadj.setResEta(sqrt(Vj(1,1)));
      aFitHadj.setResPhi(sqrt(Vj(2,2)));
      aFitHadk.setResET (sqrt(Vk(0,0)));  
      aFitHadk.setResEta(sqrt(Vk(1,1)));
      aFitHadk.setResPhi(sqrt(Vk(2,2)));
      aFitHadbbar.setResET (sqrt(Vbbar(0,0)));  
      aFitHadbbar.setResEta(sqrt(Vbbar(1,1)));
      aFitHadbbar.setResPhi(sqrt(Vbbar(2,2)));

    } else if (jetParam_ == EtThetaPhi) {
      TMatrixD Vp(3,3);  Vp  = (*fitHadp_->getCovMatrixFit()); 
      TMatrixD Vq(3,3);  Vq  = (*fitHadq_->getCovMatrixFit()); 
      TMatrixD Vbh(3,3); Vbh = (*fitHadb_->getCovMatrixFit()); 
      TMatrixD Vj(3,3);  Vj  = (*fitHadj_->getCovMatrixFit()); 
      TMatrixD Vk(3,3);  Vk  = (*fitHadk_->getCovMatrixFit()); 
      TMatrixD Vbbar(3,3); Vbbar = (*fitHadbbar_->getCovMatrixFit()); 

      aFitHadp.setCovM(this->translateCovM(Vp));
      aFitHadq.setCovM(this->translateCovM(Vq));
      aFitHadb.setCovM(this->translateCovM(Vbh));
      aFitHadj.setCovM(this->translateCovM(Vj));
      aFitHadk.setCovM(this->translateCovM(Vk));
      aFitHadbbar.setCovM(this->translateCovM(Vbbar));

      aFitHadp.setResET (sqrt(Vp(0,0)));  
      aFitHadp.setResTheta(sqrt(Vp(1,1)));
      aFitHadp.setResPhi(sqrt(Vp(2,2)));
      aFitHadq.setResET (sqrt(Vq(0,0)));  
      aFitHadq.setResTheta(sqrt(Vq(1,1)));
      aFitHadq.setResPhi(sqrt(Vq(2,2)));
      aFitHadb.setResET (sqrt(Vbh(0,0)));  
      aFitHadb.setResTheta(sqrt(Vbh(1,1)));
      aFitHadb.setResPhi(sqrt(Vbh(2,2)));
      aFitHadj.setResET (sqrt(Vj(0,0)));  
      aFitHadj.setResTheta(sqrt(Vj(1,1)));
      aFitHadj.setResPhi(sqrt(Vj(2,2)));
      aFitHadk.setResET (sqrt(Vk(0,0)));  
      aFitHadk.setResTheta(sqrt(Vk(1,1)));
      aFitHadk.setResPhi(sqrt(Vk(2,2)));
      aFitHadbbar.setResET (sqrt(Vbbar(0,0)));  
      aFitHadbbar.setResTheta(sqrt(Vbbar(1,1)));
      aFitHadbbar.setResPhi(sqrt(Vbbar(2,2)));
    }

    // finally fill the fitted particles
    fitsol.setFitHadb(aFitHadb);
    fitsol.setFitHadp(aFitHadp);
    fitsol.setFitHadq(aFitHadq);
    fitsol.setFitHadk(aFitHadj);
    fitsol.setFitHadj(aFitHadk);
    fitsol.setFitHadbbar(aFitHadbbar);

    // store the fit's chi2 probability
    fitsol.setProbChi2(TMath::Prob(theFitter_->getS(), theFitter_->getNDF()));
  }

  return fitsol;

}


/// Method to setup the fitter
void TtHadKinFitter::setupFitter() {
  
  // FIXME: replace by messagelogger!!!

  cout<<endl<<endl<<"+++++++++++ KINFIT SETUP ++++++++++++"<<endl;
  cout<<"  jet parametrisation:     ";
  if(jetParam_ == EMom) cout<<"EMomDev"<<endl;
  if(jetParam_ == EtEtaPhi) cout<<"EtEtaPhi"<<endl;
  if(jetParam_ == EtThetaPhi) cout<<"EtThetaPhi"<<endl;

  cout<<"  constraints:  "<<endl;
  for(unsigned int i=0; i<constraints_.size(); i++){
    if(constraints_[i] == 1) cout<<"    - hadronic W1 W-mass"<<endl;
    if(constraints_[i] == 2) cout<<"    - hadronic W2 W-mass"<<endl;
    if(constraints_[i] == 3) cout<<"    - hadronic top1 mass"<<endl;
    if(constraints_[i] == 4) cout<<"    - hadronic top2 mass"<<endl;
  }
  cout<<"Max. number of iterations: "<<maxNrIter_<<endl;
  cout<<"Max. deltaS: "<<maxDeltaS_<<endl;
  cout<<"Max. F: "<<maxF_<<endl;
  cout<<"++++++++++++++++++++++++++++++++++++++++++++"<<endl<<endl<<endl;
  
  theFitter_ = new TKinFitter("TtFit", "TtFit");

  TMatrixD empty3(3,3); TMatrixD empty4(4,4);
  if (jetParam_ == EMom) {
    fitHadb_ = new TFitParticleEMomDev("Jet1", "Jet1", 0, &empty4);
    fitHadp_ = new TFitParticleEMomDev("Jet2", "Jet2", 0, &empty4);
    fitHadq_ = new TFitParticleEMomDev("Jet3", "Jet3", 0, &empty4);
    fitHadbbar_ = new TFitParticleEMomDev("Jet4", "Jet4", 0, &empty4);
    fitHadj_ = new TFitParticleEMomDev("Jet5", "Jet5", 0, &empty4);
    fitHadk_ = new TFitParticleEMomDev("Jet6", "Jet6", 0, &empty4);
  } else if (jetParam_ == EtEtaPhi) {
    fitHadb_ = new TFitParticleEtEtaPhi("Jet1", "Jet1", 0, &empty3);
    fitHadp_ = new TFitParticleEtEtaPhi("Jet2", "Jet2", 0, &empty3);
    fitHadq_ = new TFitParticleEtEtaPhi("Jet3", "Jet3", 0, &empty3);
    fitHadbbar_ = new TFitParticleEtEtaPhi("Jet4", "Jet4", 0, &empty4);
    fitHadj_ = new TFitParticleEtEtaPhi("Jet5", "Jet5", 0, &empty4);
    fitHadk_ = new TFitParticleEtEtaPhi("Jet6", "Jet6", 0, &empty4);
  } else if (jetParam_ == EtThetaPhi) {
    fitHadb_ = new TFitParticleEtThetaPhi("Jet1", "Jet1", 0, &empty3);
    fitHadp_ = new TFitParticleEtThetaPhi("Jet2", "Jet2", 0, &empty3);
    fitHadq_ = new TFitParticleEtThetaPhi("Jet3", "Jet3", 0, &empty3);
    fitHadbbar_ = new TFitParticleEtThetaPhi("Jet4", "Jet4", 0, &empty4);
    fitHadj_ = new TFitParticleEtThetaPhi("Jet5", "Jet5", 0, &empty4);
    fitHadk_ = new TFitParticleEtThetaPhi("Jet6", "Jet6", 0, &empty4);
  }

  cons1_ = new TFitConstraintM("MassConstraint", "Mass-Constraint", 0, 0 , 80.35);
  cons1_->addParticles1(fitHadp_, fitHadq_);
  cons2_ = new TFitConstraintM("MassConstraint", "Mass-Constraint", 0, 0 , 80.35);
  cons2_->addParticles1(fitHadj_, fitHadk_);
  cons3_ = new TFitConstraintM("MassConstraint", "Mass-Constraint", 0, 0, 175.);
  cons3_->addParticles1(fitHadp_, fitHadq_, fitHadb_);
  cons4_ = new TFitConstraintM("MassConstraint", "Mass-Constraint", 0, 0, 175.);
  cons4_->addParticles1(fitHadj_, fitHadk_, fitHadbbar_);

  for(unsigned int i=0; i<constraints_.size(); i++){
    if(constraints_[i] == 1) theFitter_->addConstraint(cons1_);
    if(constraints_[i] == 2) theFitter_->addConstraint(cons2_);
    if(constraints_[i] == 3) theFitter_->addConstraint(cons3_);
    if(constraints_[i] == 4) theFitter_->addConstraint(cons4_);
  }
  theFitter_->addMeasParticle(fitHadb_);
  theFitter_->addMeasParticle(fitHadp_);
  theFitter_->addMeasParticle(fitHadq_);
  theFitter_->addMeasParticle(fitHadbbar_);
  theFitter_->addMeasParticle(fitHadj_);
  theFitter_->addMeasParticle(fitHadk_);

  theFitter_->setMaxNbIter(maxNrIter_);
  theFitter_->setMaxDeltaS(maxDeltaS_);
  theFitter_->setMaxF(maxF_);
  theFitter_->setVerbosity(0);

}

vector<double> TtHadKinFitter::translateCovM(TMatrixD &V){
  vector<double> covM; 
  for(int ii=0; ii<V.GetNrows(); ii++){
    for(int jj=0; jj<V.GetNcols(); jj++) covM.push_back(V(ii,jj));
  }
  return covM;
}

