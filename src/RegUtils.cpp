#ifndef _REGUTILS_CPP_
#define _REGUTILS_CPP_


#include "RegUtils.hpp"



namespace reg
{




/********************************************************************
 * Name: Assert
 * Description: error handling: check if condition is valid, and if
 * not throw an error PETSc style
 *******************************************************************/
#undef __FUNCT__
#define __FUNCT__ "Assert"
PetscErrorCode Assert(bool condition,std::string msg)
{
    PetscErrorCode ierr;

    PetscFunctionBegin;

    if(condition == false){
        ierr=ThrowError(msg); CHKERRQ(ierr);
    }

    PetscFunctionReturn(0);
}




/********************************************************************
 * Name: GetFileName
 * Description: get the filename of an image
 ********************************************************************/
#undef __FUNCT__
#define __FUNCT__ "GetFileName"
PetscErrorCode GetFileName(std::string& filename, std::string file)
{
    std::string path;
    size_t sep;

    PetscFunctionBegin;

    sep = file.find_last_of("\\/");

    if (sep != std::string::npos){
        path=file.substr(0,sep);
        filename=file.substr(sep + 1);
    }

    if (filename == ""){ filename = file; }

    PetscFunctionReturn(0);
}




/********************************************************************
 * Name: FileExists
 * Description: check if file exists
 *******************************************************************/
#undef __FUNCT__
#define __FUNCT__ "FileExists"
bool FileExists(const std::string& filename)
{
    struct stat buffer;
    return (stat(filename.c_str(), &buffer) == 0);
}




/********************************************************************
 * Name: Msg
 * Description: print msg (interfaces petsc)
 *******************************************************************/
#undef __FUNCT__
#define __FUNCT__ "Msg"
PetscErrorCode Msg(std::string msg)
{
    PetscErrorCode ierr;
    std::stringstream ss;

    PetscFunctionBegin;

    ss << std::left << msg;
    msg = " "  + ss.str() + "\n";

    // display message
    ierr=PetscPrintf(PETSC_COMM_WORLD,msg.c_str()); CHKERRQ(ierr);

    PetscFunctionReturn(0);
}




/********************************************************************
 * Name: DbgMsg
 * Description: print msg (interfaces petsc)
 * Author: Andreas Mang
 *******************************************************************/
#undef __FUNCT__
#define __FUNCT__ "DbgMsg"
PetscErrorCode DbgMsg(std::string msg)
{
    PetscErrorCode ierr;
    std::stringstream ss;

    PetscFunctionBegin;

    ss << std::left << std::setw(98)<< msg;
    msg = "\x001b[90m[ "  + ss.str() + "]\x1b[0m\n";

    // display message
    ierr=PetscPrintf(PETSC_COMM_WORLD,msg.c_str()); CHKERRQ(ierr);

    PetscFunctionReturn(0);
}




/********************************************************************
 * Name: WrngMsg
 * Description: print warning msg (interfaces petsc)
 *******************************************************************/
#undef __FUNCT__
#define __FUNCT__ "WrngMsg"
PetscErrorCode WrngMsg(std::string msg)
{
    PetscErrorCode ierr;
    std::stringstream ss;

    PetscFunctionBegin;

    ss << std::left << std::setw(98)<< msg;
    msg = "\x1b[33m[ " + ss.str() + "]\x1b[0m\n";

    // display error
    ierr=PetscPrintf(PETSC_COMM_WORLD,msg.c_str()); CHKERRQ(ierr);

    PetscFunctionReturn(0);
}




/********************************************************************
 * Name: ThrowError
 * Description: throw error
 *******************************************************************/
#undef __FUNCT__
#define __FUNCT__ "ThrowError"
PetscErrorCode ThrowError(std::string msg)
{
    PetscErrorCode ierr;

    PetscFunctionBegin;

    std::string errmsg = "\x1b[31mERROR: " + msg + "\x1b[0m";
    ierr=PetscError(PETSC_COMM_WORLD,__LINE__,PETSC_FUNCTION_NAME,__FILE__,1,PETSC_ERROR_INITIAL,errmsg.c_str());

    PetscFunctionReturn(ierr);
}




/********************************************************************
 * Name: VecView
 * Description:
 *******************************************************************/
#undef __FUNCT__
#define __FUNCT__ "VecView"
PetscErrorCode VecView(Vec x)
{
    PetscErrorCode ierr;
    ScalarType *p_x=NULL;
    IntType nl;
    int procid;
    PetscFunctionBegin;

    ierr=VecGetLocalSize(x,&nl); CHKERRQ(ierr);
    ierr=VecGetArray(x,&p_x); CHKERRQ(ierr);

    MPI_Comm_rank(PETSC_COMM_WORLD,&procid);

    if (procid == 0){
        std::cout<< " VEC VIEW"<<std::endl;
        std::cout<< " ";
        for (IntType i = 0; i < nl; ++i){
            std::cout<< p_x[i] <<" ";
        }
        std::cout<<std::endl;
    }

    ierr=VecRestoreArray(x,&p_x); CHKERRQ(ierr);

    PetscFunctionReturn(0);
}




/********************************************************************
 * Name: Rescale
 * Description: rescale data to [xminout,xmaxout]
 * Author: Andreas Mang
 *******************************************************************/
#undef __FUNCT__
#define __FUNCT__ "Rescale"
PetscErrorCode Rescale(Vec x, ScalarType xminout, ScalarType xmaxout)
{
    PetscErrorCode ierr;
    ScalarType xmin,xmax,xshift,xscale;
    std::stringstream ss;

    PetscFunctionBegin;

    // get max and min values
    ierr=VecMin(x,NULL,&xmin); CHKERRQ(ierr);
    ierr=VecMax(x,NULL,&xmax); CHKERRQ(ierr);

    xshift = xminout - xmin;
    ierr=VecShift(x,xshift); CHKERRQ(ierr);

    xmax = (xmax != 0.0) ? xmax : 1.0;
    xscale = (xmaxout == 0.0) ? 1.0 : xmaxout / xmax;

    ierr=VecScale(x,xscale); CHKERRQ(ierr);

    PetscFunctionReturn(0);
}


/********************************************************************
 * Name: String2Vec
 * Description: parse string of NUMxNUMxNUM into a vector
 *******************************************************************/
std::vector<unsigned int> String2Vec( const std::string & str )
{
    std::vector<unsigned int> vect;
    int ival;
    std::string::size_type xpos = str.find('x',0);

    if (xpos == std::string::npos){
        // only one uint
        vect.push_back( static_cast<unsigned int>( atoi(str.c_str()) ));
        return vect;
    }

    // first uint$
    ival = atoi((str.substr(0,xpos)).c_str());
    vect.push_back( static_cast<unsigned int>(ival) );

    while(true){
        std::string::size_type newxpos = xpos;
        xpos = str.find('x',newxpos+1);

        if (xpos == std::string::npos){
            ival = atoi((str.substr(newxpos+1,str.length()-newxpos-1)).c_str());
            vect.push_back( static_cast<unsigned int>(ival) );
            return vect;
        }

        ival = atoi( (str.substr(newxpos+1,xpos-newxpos-1)).c_str() );
        vect.push_back( static_cast<unsigned int>(ival));
    }
}




} // end of name space

#endif
