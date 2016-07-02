/**
 *  Copyright (c) 2015-2016.
 *  All rights reserved.
 *  This file is part of the XXX library.
 *
 *  XXX is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  XXX is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with XXX.  If not, see <http://www.gnu.org/licenses/>.
 *
*/


#ifndef _OPTIMALCONTROLREGISTRATIONBASE_H_
#define _OPTIMALCONTROLREGISTRATIONBASE_H_

#include "RegOpt.hpp"
#include "RegUtils.hpp"
#include "VecField.hpp"
#include "PreProcReg.hpp"
#include "ReadWriteReg.hpp"
#include "SemiLagrangian.hpp"
#include "RegularizationRegistration.hpp"
#include "OptimizationProblem.hpp"
//#include "SemiLagrangianGPU.hpp"


namespace reg
{

class OptimalControlRegistrationBase : public OptimizationProblem
{

public:

    typedef ReadWriteReg ReadWriteType;
    typedef OptimalControlRegistrationBase Self;
    typedef OptimizationProblem SuperClass;
    typedef RegularizationRegistration RegularizationType;
    typedef SemiLagrangian SemiLagrangianType;
    //typedef SemiLagrangianFastPlanerGPU SemiLagrangianType;

    OptimalControlRegistrationBase(void);
    OptimalControlRegistrationBase(RegOpt*);
    ~OptimalControlRegistrationBase(void);

    /*! set io object */
    PetscErrorCode SetReadWrite(ReadWriteType*);

    /*! set template image */
    PetscErrorCode SetTemplateImage(Vec);

    /*! set reference image */
    PetscErrorCode SetReferenceImage(Vec);

    /*! set template image */
    PetscErrorCode GetTemplateImage(Vec&);

    /*! set reference image */
    PetscErrorCode GetReferenceImage(Vec&);

    /*! set velocity field */
    PetscErrorCode SetVelocityField(Vec);

    /*! set velocity field */
    PetscErrorCode SetVelocityField(VecField*);

    /*! set velocity field to zero */
    PetscErrorCode SetVelocity2Zero();

    /*! compute determinant of deformation gradient, i.e.
        the jacobian of the deformation map */
    PetscErrorCode ComputeDetDefGrad();

    /*! compute deformation map */
    PetscErrorCode ComputeDeformationMap();

    /*! compute synthetic test problem */
    PetscErrorCode SetupSyntheticProb();

    /*! evaluate objective, gradient and distance measure for initial guess */
    virtual PetscErrorCode InitializeOptimization() = 0;

    /*! evaluate l2-distance between observed and predicted state */
    virtual PetscErrorCode EvaluateDistanceMeasure(ScalarType*) = 0;

    /*! evaluate objective functional J(v) */
    virtual PetscErrorCode EvaluateObjective(ScalarType*,Vec) = 0;

    /*! evaluate gradient of Lagrangian L(v) */
    virtual PetscErrorCode EvaluateGradient(Vec,Vec) = 0;

    /*! apply Hessian matvec H\tilde{\vect{v}} */
    virtual PetscErrorCode HessianMatVec(Vec,Vec) = 0;

    /*! apply preconditioner for KKT system */
    virtual PetscErrorCode PrecondMatVec(Vec, Vec) = 0;

    /*! solve forward problem */
    virtual PetscErrorCode SolveForwardProblem(Vec) = 0;

    /*! solve the current iteration */
    virtual PetscErrorCode FinalizeIteration(Vec) = 0;

    /*! finalize the registration */
    virtual PetscErrorCode Finalize(VecField*) = 0;

    /*! function that checks bounds in parameter continuation */
    virtual PetscErrorCode CheckBounds(Vec,bool&);


protected:

    PetscErrorCode Initialize(void);
    PetscErrorCode ClearMemory(void);
    PetscErrorCode CopyToAllTimePoints(Vec,Vec);
    PetscErrorCode IsVelocityZero(void);

    PetscErrorCode ComputeDetDefGradRK2(); ///< implemented via RK2 time integrator
    PetscErrorCode ComputeDetDefGradSL(); ///< implemented via SL time integrator
    PetscErrorCode ComputeDeformationMapRK2(); ///< implementation via RK2 time integrator
    PetscErrorCode ComputeDeformationMapSL(); ///< implementation via SL time integrator

    /* ! apply 2 level preconditioner */
    PetscErrorCode ComputeCFLCondition();

    Vec m_TemplateImage; ///< data container for reference image mR
    Vec m_ReferenceImage; ///< data container for template image mT

    Vec m_WorkScaField1; ///< work scalar field
    Vec m_WorkScaField2; ///< work scalar field
    Vec m_WorkScaField3; ///< work scalar field
    Vec m_WorkScaField4; ///< work scalar field

    VecField* m_WorkVecField1; ///< data container for vector field (temporary variable)
    VecField* m_WorkVecField2; ///< data container for vector field (temporary variable)
    VecField* m_WorkVecField3; ///< data container for vector field (temporary variable)
    VecField* m_WorkVecField4; ///< data container for vector field (temporary variable)

    // regularization model
    ReadWriteType* m_ReadWrite;
    RegularizationType* m_Regularization;

    VecField* m_VelocityField; ///< data container for velocity field (control variable)
    VecField* m_IncVelocityField; ///< data container for incremental velocity field (incremental control variable)

    bool m_VelocityIsZero;

    SemiLagrangianType* m_SemiLagrangianMethod;

private:



};

} // end of namespace


#endif// _OPTIMALCONTROLREGISTRATIONBASE_H_
