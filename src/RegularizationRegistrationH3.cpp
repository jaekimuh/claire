#ifndef _REGULARIZATIONREGISTRATIONH3_CPP_
#define _REGULARIZATIONREGISTRATIONH3_CPP_

#include "RegularizationRegistrationH3.hpp"




namespace reg
{




/********************************************************************
 * @brief default constructor
 *******************************************************************/
#undef __FUNCT__
#define __FUNCT__ "RegularizationRegistrationH3"
RegularizationRegistrationH3::RegularizationRegistrationH3() : SuperClass()
{

}




/********************************************************************
 * @brief default destructor
 *******************************************************************/
#undef __FUNCT__
#define __FUNCT__ "~RegularizationRegistrationH3"
RegularizationRegistrationH3::~RegularizationRegistrationH3(void)
{
    this->ClearMemory();
}




/********************************************************************
 * @brief constructor
 *******************************************************************/
#undef __FUNCT__
#define __FUNCT__ "RegularizationRegistrationH3"
RegularizationRegistrationH3::RegularizationRegistrationH3(RegOpt* opt) : SuperClass(opt)
{

}




/********************************************************************
 * @brief evaluates the functional
 *******************************************************************/
#undef __FUNCT__
#define __FUNCT__ "EvaluateFunctional"
PetscErrorCode RegularizationRegistrationH3::EvaluateFunctional(ScalarType* R, VecField* v)
{
    PetscErrorCode ierr;
    ScalarType *p_v1=NULL,*p_v2=NULL,*p_v3=NULL,
                *p_Lv1=NULL,*p_Lv2=NULL,*p_Lv3=NULL;
    ScalarType sqrtbeta[2],ipxi,scale;
    int nx[3];
    double timer[5]={0,0,0,0,0};
    PetscFunctionBegin;

    // get regularization weight
    sqrtbeta[0] = sqrt(this->m_Opt->GetRegNorm().beta[0]);
    sqrtbeta[1] = sqrt(this->m_Opt->GetRegNorm().beta[1]);

    *R = 0.0;

    // if regularization weight is zero, do noting
    if ( sqrtbeta[0] != 0.0 && sqrtbeta[1] != 0.0 ){

        ierr=Assert(v != NULL,"null pointer"); CHKERRQ(ierr);

        ierr=this->Allocate(0); CHKERRQ(ierr);
        ierr=this->Allocate(2); CHKERRQ(ierr);

        if (this->m_WorkVecField==NULL){
            try{this->m_WorkVecField = new VecField(this->m_Opt);}
            catch (std::bad_alloc&){
                ierr=reg::ThrowError("allocation failed"); CHKERRQ(ierr);
            }
        }

        nx[0] = static_cast<int>(this->m_Opt->GetNumGridPoints(0));
        nx[1] = static_cast<int>(this->m_Opt->GetNumGridPoints(1));
        nx[2] = static_cast<int>(this->m_Opt->GetNumGridPoints(2));

        scale = this->m_Opt->ComputeFFTScale();

        // compute forward fft
        ierr=v->GetArrays(p_v1,p_v2,p_v3); CHKERRQ(ierr);
        accfft_execute_r2c_t<ScalarType,FFTScaType>(this->m_Opt->GetFFT().plan,p_v1,this->m_v1hat,timer);
        accfft_execute_r2c_t<ScalarType,FFTScaType>(this->m_Opt->GetFFT().plan,p_v2,this->m_v2hat,timer);
        accfft_execute_r2c_t<ScalarType,FFTScaType>(this->m_Opt->GetFFT().plan,p_v3,this->m_v3hat,timer);
        ierr=v->RestoreArrays(p_v1,p_v2,p_v3); CHKERRQ(ierr);

        this->m_Opt->IncrementCounter(FFT,3);

#pragma omp parallel
{
        long int w[3];
        ScalarType lapik,regop[6],gradik[3];
        IntType i,i1,i2,i3;

        #pragma omp for
        for (i1 = 0; i1 < this->m_Opt->GetFFT().osize[0]; ++i1){
            for (i2 = 0; i2 < this->m_Opt->GetFFT().osize[1]; ++i2){
                for (i3 = 0; i3 < this->m_Opt->GetFFT().osize[2]; ++i3){

                    w[0] = static_cast<long int>(i1 + this->m_Opt->GetFFT().ostart[0]);
                    w[1] = static_cast<long int>(i2 + this->m_Opt->GetFFT().ostart[1]);
                    w[2] = static_cast<long int>(i3 + this->m_Opt->GetFFT().ostart[2]);

                    CheckWaveNumbers(w,nx);

                    // compute bilaplacian operator
                    lapik = -static_cast<ScalarType>(w[0]*w[0] + w[1]*w[1] + w[2]*w[2]);

                    if(w[0] == nx[0]/2) w[0] = 0;
                    if(w[1] == nx[1]/2) w[1] = 0;
                    if(w[2] == nx[2]/2) w[2] = 0;

                    // compute gradient operator
                    gradik[0] = static_cast<ScalarType>(w[0]);
                    gradik[1] = static_cast<ScalarType>(w[1]);
                    gradik[2] = static_cast<ScalarType>(w[2]);

                    // compute regularization operator
                    regop[0] = scale*( sqrtbeta[0]*gradik[0]*lapik + sqrtbeta[1]);
                    regop[1] = scale*(-sqrtbeta[0]*gradik[0]*lapik + sqrtbeta[1]);

                    regop[2] = scale*( sqrtbeta[0]*gradik[1]*lapik + sqrtbeta[1]);
                    regop[3] = scale*(-sqrtbeta[0]*gradik[1]*lapik + sqrtbeta[1]);

                    regop[4] = scale*( sqrtbeta[0]*gradik[2]*lapik + sqrtbeta[1]);
                    regop[5] = scale*(-sqrtbeta[0]*gradik[2]*lapik + sqrtbeta[1]);

                    i=GetLinearIndex(i1,i2,i3,this->m_Opt->GetFFT().osize);

                    // apply to individual components
                    this->m_Lv1hat[i][0] = regop[0]*this->m_v1hat[i][0];
                    this->m_Lv1hat[i][1] = regop[1]*this->m_v1hat[i][1];

                    this->m_Lv2hat[i][0] = regop[2]*this->m_v2hat[i][0];
                    this->m_Lv2hat[i][1] = regop[3]*this->m_v2hat[i][1];

                    this->m_Lv3hat[i][0] = regop[4]*this->m_v3hat[i][0];
                    this->m_Lv3hat[i][1] = regop[5]*this->m_v3hat[i][1];

                }
            }
        }

}// pragma omp parallel

        // compute inverse fft
        ierr=this->m_WorkVecField->GetArrays(p_Lv1,p_Lv2,p_Lv3); CHKERRQ(ierr);
        accfft_execute_c2r_t<FFTScaType,ScalarType>(this->m_Opt->GetFFT().plan,this->m_Lv1hat,p_Lv1,timer);
        accfft_execute_c2r_t<FFTScaType,ScalarType>(this->m_Opt->GetFFT().plan,this->m_Lv2hat,p_Lv2,timer);
        accfft_execute_c2r_t<FFTScaType,ScalarType>(this->m_Opt->GetFFT().plan,this->m_Lv3hat,p_Lv3,timer);
        ierr=this->m_WorkVecField->RestoreArrays(p_Lv1,p_Lv2,p_Lv3); CHKERRQ(ierr);

        this->m_Opt->IncrementCounter(FFT,3);

        // compute inner product
        ierr=VecTDot(this->m_WorkVecField->m_X1,this->m_WorkVecField->m_X1,&ipxi); CHKERRQ(ierr); *R += ipxi;
        ierr=VecTDot(this->m_WorkVecField->m_X2,this->m_WorkVecField->m_X2,&ipxi); CHKERRQ(ierr); *R += ipxi;
        ierr=VecTDot(this->m_WorkVecField->m_X3,this->m_WorkVecField->m_X3,&ipxi); CHKERRQ(ierr); *R += ipxi;

        // increment fft timer
        this->m_Opt->IncreaseFFTTimers(timer);

        // multiply with regularization weight
        *R *= 0.5;
    }

    PetscFunctionReturn(0);
}




/********************************************************************
 * @brief evaluates first variation of regularization norm
 *******************************************************************/
#undef __FUNCT__
#define __FUNCT__ "EvaluateGradient"
PetscErrorCode RegularizationRegistrationH3::EvaluateGradient(VecField* dvR, VecField* v)
{
    PetscErrorCode ierr;
    int nx[3];
    ScalarType *p_v1=NULL,*p_v2=NULL,*p_v3=NULL,
                *p_Lv1=NULL,*p_Lv2=NULL,*p_Lv3=NULL;
    ScalarType beta[2],scale;
    double timer[5]={0,0,0,0,0};

    PetscFunctionBegin;

    ierr=Assert(v!=NULL,"null pointer"); CHKERRQ(ierr);
    ierr=Assert(dvR!=NULL,"null pointer"); CHKERRQ(ierr);

    // get regularization weight
    beta[0] = this->m_Opt->GetRegNorm().beta[0];
    beta[1] = this->m_Opt->GetRegNorm().beta[1];

    // if regularization weight is zero, do noting
    if ( (beta[0] == 0.0) && (beta[1] == 0.0) ){
        ierr=dvR->SetValue(0.0); CHKERRQ(ierr);
    }
    else{

        ierr=this->Allocate(0); CHKERRQ(ierr);
        ierr=this->Allocate(1); CHKERRQ(ierr);

        nx[0] = static_cast<int>(this->m_Opt->GetNumGridPoints(0));
        nx[1] = static_cast<int>(this->m_Opt->GetNumGridPoints(1));
        nx[2] = static_cast<int>(this->m_Opt->GetNumGridPoints(2));

        scale = this->m_Opt->ComputeFFTScale();

        // compute forward fft
        ierr=v->GetArrays(p_v1,p_v2,p_v3); CHKERRQ(ierr);
        accfft_execute_r2c_t<ScalarType,FFTScaType>(this->m_Opt->GetFFT().plan,p_v1,this->m_v1hat,timer);
        accfft_execute_r2c_t<ScalarType,FFTScaType>(this->m_Opt->GetFFT().plan,p_v2,this->m_v2hat,timer);
        accfft_execute_r2c_t<ScalarType,FFTScaType>(this->m_Opt->GetFFT().plan,p_v3,this->m_v3hat,timer);
        ierr=v->RestoreArrays(p_v1,p_v2,p_v3); CHKERRQ(ierr);

        this->m_Opt->IncrementCounter(FFT,3);

#pragma omp parallel
{
        long int w[3];
        ScalarType trihik,regop;
        IntType i,i1,i2,i3;

#pragma omp for
        for (i1 = 0; i1 < this->m_Opt->GetFFT().osize[0]; ++i1){
            for (i2 = 0; i2 < this->m_Opt->GetFFT().osize[1]; ++i2){
                for (i3 = 0; i3 < this->m_Opt->GetFFT().osize[2]; ++i3){

                    w[0] = static_cast<long int>(i1 + this->m_Opt->GetFFT().ostart[0]);
                    w[1] = static_cast<long int>(i2 + this->m_Opt->GetFFT().ostart[1]);
                    w[2] = static_cast<long int>(i3 + this->m_Opt->GetFFT().ostart[2]);

                    CheckWaveNumbers(w,nx);

                    if(w[0] == nx[0]/2) w[0] = 0;
                    if(w[1] == nx[1]/2) w[1] = 0;
                    if(w[2] == nx[2]/2) w[2] = 0;

                    trihik = pow(w[0],6.0) + pow(w[1],6.0) + pow(w[2],6.0)
                            + 3.0*( pow(w[0],4.0)*pow(w[1],2.0)
                            +       pow(w[0],2.0)*pow(w[1],4.0)
                            +       pow(w[0],4.0)*pow(w[2],2.0)
                            +       pow(w[0],2.0)*pow(w[2],4.0)
                            +       pow(w[1],4.0)*pow(w[2],2.0)
                            +       pow(w[1],2.0)*pow(w[2],4.0) );


                    // compute regularization operator
                    regop = scale*(-beta[0]*trihik + beta[1]);

                    // get linear index
                    i=GetLinearIndex(i1,i2,i3,this->m_Opt->GetFFT().osize);

                    // apply to individual components
                    this->m_Lv1hat[i][0] = regop*this->m_v1hat[i][0];
                    this->m_Lv1hat[i][1] = regop*this->m_v1hat[i][1];

                    this->m_Lv2hat[i][0] = regop*this->m_v2hat[i][0];
                    this->m_Lv2hat[i][1] = regop*this->m_v2hat[i][1];

                    this->m_Lv3hat[i][0] = regop*this->m_v3hat[i][0];
                    this->m_Lv3hat[i][1] = regop*this->m_v3hat[i][1];

                }
            }
        }
}// pragma omp parallel


        // compute inverse fft
        ierr=dvR->GetArrays(p_Lv1,p_Lv2,p_Lv3); CHKERRQ(ierr);
        accfft_execute_c2r_t<FFTScaType,ScalarType>(this->m_Opt->GetFFT().plan,this->m_Lv1hat,p_Lv1,timer);
        accfft_execute_c2r_t<FFTScaType,ScalarType>(this->m_Opt->GetFFT().plan,this->m_Lv2hat,p_Lv2,timer);
        accfft_execute_c2r_t<FFTScaType,ScalarType>(this->m_Opt->GetFFT().plan,this->m_Lv3hat,p_Lv3,timer);
        ierr=dvR->RestoreArrays(p_Lv1,p_Lv2,p_Lv3); CHKERRQ(ierr);

        this->m_Opt->IncrementCounter(FFT,3);


        // increment fft timer
        this->m_Opt->IncreaseFFTTimers(timer);
    }

    PetscFunctionReturn(0);
}




/********************************************************************
 * @brief applies second variation of regularization norm to
 * a vector
 *******************************************************************/
#undef __FUNCT__
#define __FUNCT__ "HessianMatVec"
PetscErrorCode RegularizationRegistrationH3::HessianMatVec(VecField* dvvR, VecField* vtilde)
{
    PetscErrorCode ierr;
    ScalarType beta;
    PetscFunctionBegin;

    ierr=Assert(vtilde != NULL,"null pointer"); CHKERRQ(ierr);
    ierr=Assert(dvvR != NULL,"null pointer"); CHKERRQ(ierr);

    beta = this->m_Opt->GetRegNorm().beta[0];

    // if regularization weight is zero, do noting
    if (beta == 0.0){
        ierr=dvvR->SetValue(0.0); CHKERRQ(ierr);
    }
    else{ ierr=this->EvaluateGradient(dvvR,vtilde); CHKERRQ(ierr); }

    PetscFunctionReturn(0);
}




/********************************************************************
 * @brief apply the inverse of the regularization operator; we
 * can invert this operator analytically due to the spectral
 * discretization
 *******************************************************************/
#undef __FUNCT__
#define __FUNCT__ "ApplyInvOp"
PetscErrorCode RegularizationRegistrationH3::ApplyInvOp(VecField* Ainvx, VecField* x, bool applysqrt)
{
    PetscErrorCode ierr;
    int nx[3];
    ScalarType *p_x1=NULL,*p_x2=NULL,*p_x3=NULL,
                *p_Lv1=NULL,*p_Lv2=NULL,*p_Lv3=NULL;
    ScalarType beta[2],scale;
    double timer[5]={0,0,0,0,0};
    PetscFunctionBegin;

    ierr=Assert(x != NULL,"null pointer"); CHKERRQ(ierr);
    ierr=Assert(Ainvx != NULL,"null pointer"); CHKERRQ(ierr);

    beta[0] = this->m_Opt->GetRegNorm().beta[0];
    beta[1] = this->m_Opt->GetRegNorm().beta[1];

    // if regularization weight is zero, do noting
    if ( beta[0] == 0.0 && beta[1] == 0.0 ){
        ierr=VecCopy(x->m_X1,Ainvx->m_X1); CHKERRQ(ierr);
        ierr=VecCopy(x->m_X2,Ainvx->m_X2); CHKERRQ(ierr);
        ierr=VecCopy(x->m_X3,Ainvx->m_X3); CHKERRQ(ierr);
    }
    else{

        ierr=this->Allocate(0); CHKERRQ(ierr);
        ierr=this->Allocate(1); CHKERRQ(ierr);

        nx[0] = static_cast<int>(this->m_Opt->GetNumGridPoints(0));
        nx[1] = static_cast<int>(this->m_Opt->GetNumGridPoints(1));
        nx[2] = static_cast<int>(this->m_Opt->GetNumGridPoints(2));

        scale = this->m_Opt->ComputeFFTScale();

        // compute forward fft
        ierr=x->GetArrays(p_x1,p_x2,p_x3); CHKERRQ(ierr);
        accfft_execute_r2c_t<ScalarType,FFTScaType>(this->m_Opt->GetFFT().plan,p_x1,this->m_v1hat,timer);
        accfft_execute_r2c_t<ScalarType,FFTScaType>(this->m_Opt->GetFFT().plan,p_x2,this->m_v2hat,timer);
        accfft_execute_r2c_t<ScalarType,FFTScaType>(this->m_Opt->GetFFT().plan,p_x3,this->m_v3hat,timer);
        ierr=x->RestoreArrays(p_x1,p_x2,p_x3); CHKERRQ(ierr);

        this->m_Opt->IncrementCounter(FFT,3);

#pragma omp parallel
{
        long int w[3];
        ScalarType trihik,regop;
        IntType i,i1,i2,i3;

#pragma omp for
        for (i1 = 0; i1 < this->m_Opt->GetFFT().osize[0]; ++i1){
            for (i2 = 0; i2 < this->m_Opt->GetFFT().osize[1]; ++i2){
                for (i3 = 0; i3 < this->m_Opt->GetFFT().osize[2]; ++i3){

                    w[0] = static_cast<long int>(i1 + this->m_Opt->GetFFT().ostart[0]);
                    w[1] = static_cast<long int>(i2 + this->m_Opt->GetFFT().ostart[1]);
                    w[2] = static_cast<long int>(i3 + this->m_Opt->GetFFT().ostart[2]);

                    CheckWaveNumbersInv(w,nx);

                    trihik = pow(w[0],6.0) + pow(w[1],6.0) + pow(w[2],6.0)
                            + 3.0*( pow(w[0],4.0)*pow(w[1],2.0)
                            +       pow(w[0],2.0)*pow(w[1],4.0)
                            +       pow(w[0],4.0)*pow(w[2],2.0)
                            +       pow(w[0],2.0)*pow(w[2],4.0)
                            +       pow(w[1],4.0)*pow(w[2],2.0)
                            +       pow(w[1],2.0)*pow(w[2],4.0) );

                    // compute regularization operator
                    regop = -beta[0]*trihik + beta[1];

                    if (applysqrt) regop = sqrt(regop);
                    regop = scale/regop;

                    i=GetLinearIndex(i1,i2,i3,this->m_Opt->GetFFT().osize);

                    // apply to individual components
                    this->m_Lv1hat[i][0] = regop*this->m_v1hat[i][0];
                    this->m_Lv1hat[i][1] = regop*this->m_v1hat[i][1];

                    this->m_Lv2hat[i][0] = regop*this->m_v2hat[i][0];
                    this->m_Lv2hat[i][1] = regop*this->m_v2hat[i][1];

                    this->m_Lv3hat[i][0] = regop*this->m_v3hat[i][0];
                    this->m_Lv3hat[i][1] = regop*this->m_v3hat[i][1];

                }
            }
        }

}// pragma omp parallel


        // compute inverse fft
        ierr=Ainvx->GetArrays(p_Lv1,p_Lv2,p_Lv3); CHKERRQ(ierr);
        accfft_execute_c2r_t<FFTScaType,ScalarType>(this->m_Opt->GetFFT().plan,this->m_Lv1hat,p_Lv1,timer);
        accfft_execute_c2r_t<FFTScaType,ScalarType>(this->m_Opt->GetFFT().plan,this->m_Lv2hat,p_Lv2,timer);
        accfft_execute_c2r_t<FFTScaType,ScalarType>(this->m_Opt->GetFFT().plan,this->m_Lv3hat,p_Lv3,timer);
        ierr=Ainvx->RestoreArrays(p_Lv1,p_Lv2,p_Lv3); CHKERRQ(ierr);

        this->m_Opt->IncrementCounter(FFT,3);

        // increment fft timer
        this->m_Opt->IncreaseFFTTimers(timer);
    }

    PetscFunctionReturn(0);
}




/********************************************************************
 * @brief computes the largest and smallest eigenvalue of
 * the inverse regularization operator
 *******************************************************************/
#undef __FUNCT__
#define __FUNCT__ "GetExtremeEigValsInvOp"
PetscErrorCode RegularizationRegistrationH3::GetExtremeEigValsInvOp(ScalarType& emin, ScalarType& emax)
{
    PetscErrorCode ierr=0;
    ScalarType w[3],beta1,beta2,trihik,regop;

    PetscFunctionBegin;

    beta1=this->m_Opt->GetRegNorm().beta[0];
    beta2=this->m_Opt->GetRegNorm().beta[1];

    // get max value
    w[0] = static_cast<ScalarType>(this->m_Opt->GetDomainPara().nx[0])/2.0;
    w[1] = static_cast<ScalarType>(this->m_Opt->GetDomainPara().nx[1])/2.0;
    w[2] = static_cast<ScalarType>(this->m_Opt->GetDomainPara().nx[2])/2.0;

    // compute largest value for operator
    trihik = pow(w[0],6.0) + pow(w[1],6.0) + pow(w[2],6.0)
            + 3.0*( pow(w[0],4.0)*pow(w[1],2.0)
            +       pow(w[0],2.0)*pow(w[1],4.0)
            +       pow(w[0],4.0)*pow(w[2],2.0)
            +       pow(w[0],2.0)*pow(w[2],4.0)
            +       pow(w[1],4.0)*pow(w[2],2.0)
            +       pow(w[1],2.0)*pow(w[2],4.0) );

    // compute regularization operator
    regop = -beta1*trihik + beta2;
    emin = 1.0/regop;
    emax = 1.0/beta2; // 1/(0*beta_1 + beta_2)

    PetscFunctionReturn(ierr);
}




} // end of name space

#endif //_REGULARIZATIONREGISTRATIONH2_CPP_