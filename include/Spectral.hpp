/*************************************************************************
 *  Copyright (c) 2018.
 *  All rights reserved.
 *  This file is part of the CLAIRE library.
 *
 *  CLAIRE is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  CLAIRE is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with CLAIRE. If not, see <http://www.gnu.org/licenses/>.
 ************************************************************************/

#ifndef _SPECTRAL_HPP_
#define _SPECTRAL_HPP_

#include "CLAIREUtils.hpp"
#include "SpectralKernel.hpp"

#ifdef REG_HAS_CUDA
#include "mpicufft.hpp"
#include <cufft.h>
#include "cuda_helper.hpp"
#endif

namespace reg {
  
class RegOpt;
class FourierTransform;

class Spectral {
 public:
    typedef Spectral Self;
    
    Spectral(RegOpt *opt, FourierTransform* fft);
    virtual ~Spectral();
    
    PetscErrorCode InitFFT();
    PetscErrorCode SetDomain();
    
    PetscErrorCode FFT_R2C(const ScalarType *real, ComplexType *complex);
    PetscErrorCode FFT_C2R(const ComplexType *complex, ScalarType *real);
    
    PetscErrorCode LowPassFilter(ComplexType *xHat, ScalarType pct);
    PetscErrorCode HighPassFilter(ComplexType *xHat, ScalarType pct);
    
    PetscErrorCode Restrict(ComplexType *xc, const ComplexType *xf, const IntType nxc[3]);
    PetscErrorCode Prolong(ComplexType *xf, const ComplexType *xc, const IntType nxc[3]);
    PetscErrorCode ProlongNonZero(ComplexType *xf, const ComplexType *xc, const IntType nxc[3]);
    
    PetscErrorCode Scale(ComplexType *x, ScalarType scale);
    
 protected:
    PetscErrorCode Initialize();
    PetscErrorCode ClearMemory();
    
    PetscErrorCode SetupFFT();
    
    SpectralKernel m_kernel;

#ifdef REG_HAS_CUDA
    MPIcuFFT<ScalarType> *m_plan;
#else
    FFTPlanType *m_plan;
#endif

    RegOpt *m_Opt;
    FourierTransform *m_FFT;
};




}  // end of namespace




#endif  // _SPECTRAL_HPP_
