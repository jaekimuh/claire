/*************************************************************************
 *  Copyright (c) 2016.
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

#ifndef _SCAFIELD_CPP_
#define _SCAFIELD_CPP_

#include "ScaField.hpp"

namespace reg {
  
/********************************************************************
 * @brief default constructor
 *******************************************************************/
/*ScaField::ScaField() {
    this->Initialize();
}*/

/********************************************************************
 * @brief default destructor
 *******************************************************************/
ScaField::~ScaField() {
    this->ClearMemory();
}

/********************************************************************
 * @brief constructor
 *******************************************************************/
ScaField::ScaField(RegOpt *opt, bool multicomponent, bool fullnewton) {
    this->Initialize();
    
    this->m_Opt = opt;
    
    this->m_Dim[0] = opt->m_Domain.nl;
    this->m_Dim[1] = multicomponent ? opt->m_Domain.nc : 1;
    this->m_Dim[2] = fullnewton ? opt->m_Domain.nt + 1 : 1;
    
    this->m_Size[0] = this->m_Dim[0];
    this->m_Size[1] = this->m_Dim[0]*this->m_Dim[1];
    this->m_Size[2] = this->m_Dim[0]*this->m_Dim[1]*this->m_Dim[2];
    
    IntType global = this->m_Dim[1]*this->m_Dim[2]*opt->m_Domain.ng;
    
    this->Allocate(global, this->m_Size[2]);
}

/********************************************************************
 * @brief constructor
 *******************************************************************/
ScaField::ScaField(RegOpt *opt, ScalarType value, bool multicomponent, bool fullnewton) {
    this->Initialize();
    
    this->m_Opt = opt;
    
    this->m_Dim[0] = opt->m_Domain.nl;
    this->m_Dim[1] = multicomponent ? opt->m_Domain.nc : 1;
    this->m_Dim[2] = fullnewton ? opt->m_Domain.nt + 1 : 1;
    
    this->m_Size[0] = this->m_Dim[0];
    this->m_Size[1] = this->m_Dim[0]*this->m_Dim[1];
    this->m_Size[2] = this->m_Dim[0]*this->m_Dim[1]*this->m_Dim[2];
    
    IntType global = this->m_Dim[1]*this->m_Dim[2]*opt->m_Domain.ng;
    
    this->Allocate(global, this->m_Size[2]);
    
    this->Set(value);
}

/********************************************************************
 * @brief constructor
 *******************************************************************/
ScaField::ScaField(RegOpt *opt, Vec vec, bool multicomponent, bool fullnewton) {
    this->Initialize();
    
    this->m_Opt = opt;
    
    this->m_Dim[0] = opt->m_Domain.nl;
    this->m_Dim[1] = multicomponent ? opt->m_Domain.nc : 1;
    this->m_Dim[2] = fullnewton ? opt->m_Domain.nt + 1 : 1;
    
    this->m_Size[0] = this->m_Dim[0];
    this->m_Size[1] = this->m_Dim[0]*this->m_Dim[1];
    this->m_Size[2] = this->m_Dim[0]*this->m_Dim[1]*this->m_Dim[2];
    
    IntType global = this->m_Dim[1]*this->m_Dim[2]*opt->m_Domain.ng;
    
    this->Assign(vec);
}

/********************************************************************
 * @brief init variables
 *******************************************************************/
PetscErrorCode ScaField::Initialize(void) {
    PetscErrorCode ierr = 0;
    PetscFunctionBegin;

    this->m_X = nullptr;
    this->m_Ptr = nullptr;
    this->m_Type = AccessType::None;
    this->m_Allocated = 0;
    this->m_Opt = nullptr;
    
    this->m_Dim[0] = this->m_Dim[1] = this->m_Dim[2] = 0;
    this->m_Size[0] = this->m_Size[1] = 0;

    PetscFunctionReturn(ierr);
}

/********************************************************************
 * @brief clean up
 *******************************************************************/
PetscErrorCode ScaField::ClearMemory() {
    PetscErrorCode ierr = 0;
    PetscFunctionBegin;
    
    if (this->m_X != nullptr && this->m_Allocated > 0) {
        ierr = VecDestroy(&this->m_X); CHKERRQ(ierr);
    }
    this->m_X = nullptr;
    this->m_Allocated = 0;
    this->m_Ptr = nullptr;
    this->m_Type = AccessType::None;

    PetscFunctionReturn(ierr);
}

/********************************************************************
 * @brief function to allocate vector field
 *******************************************************************/
PetscErrorCode ScaField::Allocate(IntType ng, IntType nl) {
    PetscErrorCode ierr = 0;
    PetscFunctionBegin;

    ierr = this->ClearMemory(); CHKERRQ(ierr);

    // allocate vector field
    ierr = VecCreate(PETSC_COMM_WORLD, &this->m_X); CHKERRQ(ierr);
    ierr = VecSetSizes(this->m_X, nl, ng); CHKERRQ(ierr);
    #ifdef REG_HAS_CUDA
        ierr = VecSetType(this->m_X, VECCUDA); CHKERRQ(ierr);
    #else
        ierr = VecSetFromOptions(this->m_X); CHKERRQ(ierr);
    #endif
    
    this->m_Allocated = nl;

    PetscFunctionReturn(ierr);
}

/********************************************************************
 * @brief set the vector
 *******************************************************************/
PetscErrorCode ScaField::SetVector(Vec vec) {
  PetscErrorCode ierr = 0;
  PetscFunctionBegin;
  
  ierr = this->Assign(vec); CHKERRQ(ierr);
  
  PetscFunctionReturn(ierr);
}

/********************************************************************
 * @brief allocate the vector
 *******************************************************************/
PetscErrorCode ScaField::SetSize(IntType ng, IntType nl, IntType nc, IntType nt) {
  PetscErrorCode ierr = 0;
  PetscFunctionBegin;
  
  this->m_Dim[0] = nl;
  this->m_Dim[1] = nc;
  this->m_Dim[2] = nt;
  
  this->m_Size[0] = nl;
  this->m_Size[1] = nl*nc;
  this->m_Size[2] = nl*nc*nt;
  
  ierr = this->Allocate(ng*nc*nt, nl*nc*nt); CHKERRQ(ierr);
  
  PetscFunctionReturn(ierr);
}

/********************************************************************
 * @brief function to assign a vector field
 *******************************************************************/
PetscErrorCode ScaField::Assign(Vec vec) {
    PetscErrorCode ierr = 0;
    PetscFunctionBegin;
    
    ierr = ClearMemory(); CHKERRQ(ierr);
    
    IntType vnl;
    ierr = VecGetLocalSize(vec, &vnl); CHKERRQ(ierr);
    ierr = Assert(vnl == this->m_Size[2], "dimensions do not match"); CHKERRQ(ierr);

    this->m_X = vec;

    PetscFunctionReturn(ierr);
}

/********************************************************************
 * @brief cast operator to Petsc vector
 *******************************************************************/
ScaField::operator Vec& () {
  return this->m_X;
}

/********************************************************************
 * @brief get raw read/write access to array
 *******************************************************************/
PetscErrorCode ScaField::GetArray(ScalarType*& ptr, IntType j, IntType k, IntType i) {
  PetscErrorCode ierr = 0;
  PetscFunctionBegin;
  
  if (this->m_Type == AccessType::None) {
    ierr = Assert(i >= 0 && i < this->m_Dim[0], "index out of range"); CHKERRQ(ierr);
    ierr = Assert(j >= 0 && j < this->m_Dim[1], "index out of range"); CHKERRQ(ierr);
    ierr = Assert(k >= 0 && k < this->m_Dim[2], "index out of range"); CHKERRQ(ierr);
  
    ierr = GetRawPointerReadWrite(this->m_X, &this->m_Ptr); CHKERRQ(ierr);
    this->m_Type = AccessType::ReadWrite;
  } else {
    ierr = Assert(this->m_Type == AccessType::ReadWrite, "can't access with different types"); CHKERRQ(ierr);
  }
  
  ptr = m_Ptr + i + j*m_Size[0] + k*m_Size[1];
  
  PetscFunctionReturn(ierr);
}

/********************************************************************
 * @brief get raw read access to array
 *******************************************************************/
PetscErrorCode ScaField::GetArrayRead(const ScalarType*& ptr, IntType j, IntType k, IntType i) {
  PetscErrorCode ierr = 0;
  PetscFunctionBegin;
  
  if (this->m_Type == AccessType::None) {
    ierr = Assert(i >= 0 && i < this->m_Dim[0], "index out of range"); CHKERRQ(ierr);
    ierr = Assert(j >= 0 && j < this->m_Dim[1], "index out of range"); CHKERRQ(ierr);
    ierr = Assert(k >= 0 && k < this->m_Dim[2], "index out of range"); CHKERRQ(ierr);
    
    GetRawPointerRead(this->m_X, &this->m_ConstPtr); CHKERRQ(ierr);
    this->m_Type = AccessType::Read;
  } else {
    ierr = Assert(this->m_Type == AccessType::Read, "can't access with different types"); CHKERRQ(ierr);
  }
  
  ptr = m_ConstPtr + i + j*m_Size[0] + k*m_Size[1];
  
  PetscFunctionReturn(ierr);
}

/********************************************************************
 * @brief get raw write access to array
 *******************************************************************/
PetscErrorCode ScaField::GetArrayWrite(ScalarType*& ptr, IntType j, IntType k, IntType i) {
  PetscErrorCode ierr = 0;
  PetscFunctionBegin;
  
  if (this->m_Type == AccessType::None) {
    ierr = Assert(i >= 0 && i < this->m_Dim[0], "index out of range"); CHKERRQ(ierr);
    ierr = Assert(j >= 0 && j < this->m_Dim[1], "index out of range"); CHKERRQ(ierr);
    ierr = Assert(k >= 0 && k < this->m_Dim[2], "index out of range"); CHKERRQ(ierr);
    
    ierr = GetRawPointerWrite(this->m_X, &this->m_Ptr); CHKERRQ(ierr);
    this->m_Type = AccessType::Write;
  } else {
    ierr = Assert(this->m_Type == AccessType::Write, "can't access with different types"); CHKERRQ(ierr);
  }
  
  ptr = m_Ptr + i + j*m_Size[0] + k*m_Size[1];
  
  PetscFunctionReturn(ierr);
}

/********************************************************************
 * @brief get raw read/write access to array
 *******************************************************************/
PetscErrorCode ScaField::GetArrayReadWrite(ScalarType*& ptr, IntType j, IntType k, IntType i) {
  PetscErrorCode ierr = 0;
  PetscFunctionBegin;
  
  if (this->m_Type == AccessType::None) {
    ierr = Assert(i >= 0 && i < this->m_Dim[0], "index out of range"); CHKERRQ(ierr);
    ierr = Assert(j >= 0 && j < this->m_Dim[1], "index out of range"); CHKERRQ(ierr);
    ierr = Assert(k >= 0 && k < this->m_Dim[2], "index out of range"); CHKERRQ(ierr);
    
    ierr = GetRawPointerReadWrite(this->m_X, &this->m_Ptr); CHKERRQ(ierr);
    this->m_Type = AccessType::ReadWrite;
  } else {
    ierr = Assert(this->m_Type == AccessType::ReadWrite, "can't access with different types"); CHKERRQ(ierr);
  }
  
  ptr = m_Ptr + i + j*m_Size[0] + k*m_Size[1];
  
  PetscFunctionReturn(ierr);
}

/********************************************************************
 * @brief restore raw access
 *******************************************************************/
PetscErrorCode ScaField::RestoreArray() {
  PetscErrorCode ierr = 0;
  PetscFunctionBegin;
  
  ierr = Assert(this->m_Type != AccessType::None, "can't restore without access"); CHKERRQ(ierr);
  
  switch (this->m_Type) {
  case AccessType::Read:
    ierr = RestoreRawPointerRead(this->m_X, &this->m_ConstPtr); CHKERRQ(ierr);
    this->m_Ptr = nullptr;
    this->m_Type = AccessType::None;
    break;
  case AccessType::Write:
    ierr = RestoreRawPointerWrite(this->m_X, &this->m_Ptr); CHKERRQ(ierr);
    this->m_Ptr = nullptr;
    this->m_Type = AccessType::None;
    break;
  case AccessType::ReadWrite:
    ierr = RestoreRawPointerReadWrite(this->m_X, &this->m_Ptr); CHKERRQ(ierr);
    this->m_Ptr = nullptr;
    this->m_Type = AccessType::None;
    break;
  };
  
  PetscFunctionReturn(ierr);
}

/********************************************************************
 * @brief copy vector
 *******************************************************************/
PetscErrorCode ScaField::Copy(Vec vec) {
  PetscErrorCode ierr = 0;
  PetscFunctionBegin;
  
  VecCopy(vec, this->m_X);
  this->m_Ptr = nullptr;
  this->m_Type = AccessType::None;
  
  PetscFunctionReturn(ierr);
}

/********************************************************************
 * @brief copy a single frame to all timesteps
 *******************************************************************/
PetscErrorCode ScaField::CopyFrame(IntType src) {
  PetscErrorCode ierr = 0;
  ScalarType *ptr = nullptr, *orig = nullptr, *dest = nullptr;
  PetscFunctionBegin;
  
  ierr = Assert(this->m_Type == AccessType::None, "can't copy with ongoing access"); CHKERRQ(ierr);
  ierr = Assert(src >= 0 && src < this->m_Dim[2], "index out of range"); CHKERRQ(ierr);
  
  ierr = GetRawPointerReadWrite(this->m_X, &ptr); CHKERRQ(ierr);
  
  orig = ptr + src*this->m_Size[1];
  
  for (IntType i = 0; i < this->m_Dim[2]; ++i) {
    if (i == src) continue;
    dest = ptr + i*this->m_Size[1];
#ifndef REG_HAS_CUDA
    std::copy(orig, orig + this->m_Size[1], dest);
#else
    cudaMemcpy((void*)dest,(void*)orig,sizeof(ScalarType)*this->m_Size[1],cudaMemcpyDeviceToDevice);
#endif
  }
#ifdef REG_HAS_CUDA
  cudaDeviceSynchronize();
#endif
  
  ierr = RestoreRawPointerReadWrite(this->m_X, &ptr); CHKERRQ(ierr);
  
  PetscFunctionReturn(ierr);
}

/********************************************************************
 * @brief set value
 *******************************************************************/
PetscErrorCode ScaField::Set(ScalarType value) {
  PetscErrorCode ierr = 0;
  PetscFunctionBegin;
  
  VecSet(this->m_X, value);
  
  PetscFunctionReturn(ierr);
}
  
} // namespace reg

#endif // _SCAFIELD_CPP_
