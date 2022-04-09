
#if !defined(__Dtw_HEADER_INCLUDED__)
#define __Dtw_HEADER_INCLUDED__

#pragma once

#include "ErrorDef.h"

/*! \brief basic dynamic time warping 
*       see https://github.com/alexanderlerch/pyACA/blob/master/pyACA/ToolSimpleDtw.py or
*       https://github.com/alexanderlerch/ACA-Code/blob/master/ToolSimpleDtw.m
*/
class CDtw {
public:
    enum MatrixDimension_t {
        kRow,
        kCol,
        kNumMatrixDimensions
    };

    CDtw();
    virtual ~CDtw();

    /*! initializes the class with the size of the distance matrix
    \param iNumRows
    \param iNumCols
    \return Error_t
    */
    Error_t init(int iNumRows, int iNumCols);
    
    /*! resets all internal class members
    \return Error_t
    */
    Error_t reset();

    /*! computes cost and path w/o back-tracking
    \param ppfDistanceMatrix (dimensions [rows][columns])
    \return Error_t
    */
    Error_t process(float **ppfDistanceMatrix);
 
    /*! returns the length of the path
    \return int
    */
    int getPathLength();
    
    /*! returns the overall cost
    \return float
    */
    float getPathCost() const;
    
    /*! returns the path \sa getPathLength
    \param ppiPathResult pointer to memory the result is written to (dimensions [2][length_of_path])
    \return Error_t
    */
    Error_t getPath(int **ppiPathResult) const;

private:
    int idx(int row, int col) const;

    bool m_bIsInitialized;
    int rows, cols;
    float *costMatrix = nullptr;
    int *parentMatrix = nullptr;
};


#endif
