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

#ifndef _REGTOOLSOPT_H_
#define _REGTOOLSOPT_H_

#include "RegOpt.hpp"




namespace reg {




struct ResamplingPara {
    ScalarType gridscale;
    bool enabled;
};


struct PostProcPara {
    bool enabled;           ///< run post processing
    bool computedeffields;  ///< compute deformation fields (deformation gradient, displacement field, ...)
    bool computegrad;       ///< compute gradient of scalar field
    bool computesynvel;     ///< compute synthetic velocity field
    bool tscafield;         ///< transport scalar field (forward problem)
    bool tlabelmap;         ///< transport label map (solve forward problem)
};


struct RegToolsFlags {
    bool readvecfield;  ///< read vector field
    bool readscafield;  ///< read scalar field
};




class RegToolsOpt : public RegOpt {
 public:
    typedef RegToolsOpt Self;
    typedef RegOpt SuperClass;

    RegToolsOpt();
    RegToolsOpt(int, char**);
    RegToolsOpt(const RegToolsOpt&);
    ~RegToolsOpt();

    std::string GetVecFieldFN(int, int);
    std::string GetScaFieldFN(int);
    virtual PetscErrorCode DisplayOptions(void);
    inline RegToolsFlags GetFlags() {return this->m_RegToolsFlags;}
    inline ResamplingPara GetResamplingPara() {return this->m_ResamplingPara;}
    inline PostProcPara GetPostProcPara() {return this->m_PostProcPara;}

 protected:
    virtual PetscErrorCode Initialize(void);
    virtual PetscErrorCode ClearMemory(void);
    virtual PetscErrorCode ParseArguments(int, char**);
    virtual PetscErrorCode Usage(bool advanced = false);
    virtual PetscErrorCode CheckArguments(void);

    PostProcPara m_PostProcPara;
    RegToolsFlags m_RegToolsFlags;
    ResamplingPara m_ResamplingPara;

    std::string m_RFN;
    std::string m_TFN;

    std::string m_iVecFieldX1FN;    ///< x1 vector field file name
    std::string m_iVecFieldX2FN;    ///< x2 vector field file name
    std::string m_iVecFieldX3FN;    ///< x3 vector field file name
    std::string m_iScaFieldFN;      ///< input file name

    std::string m_xVecFieldX1FN;    ///< x1 vector field file name
    std::string m_xVecFieldX2FN;    ///< x2 vector field file name
    std::string m_xVecFieldX3FN;    ///< x3 vector field file name
    std::string m_xScaFieldFN;      ///< input file name
};




}  // namespace reg




#endif  // _REGTOOLSOPT_H_