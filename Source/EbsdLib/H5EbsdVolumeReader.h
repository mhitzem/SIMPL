/* ============================================================================
 * Copyright (c) 2010, Michael A. Jackson (BlueQuartz Software)
 * Copyright (c) 2010, Dr. Michael A. Groeber (US Air Force Research Laboratories)
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice, this
 * list of conditions and the following disclaimer in the documentation and/or
 * other materials provided with the distribution.
 *
 * Neither the name of Michael A. Groeber, Michael A. Jackson, the US Air Force,
 * BlueQuartz Software nor the names of its contributors may be used to endorse
 * or promote products derived from this software without specific prior written
 * permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *  This code was written under United States Air Force Contract number
 *                           FA8650-07-D-5800
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

#ifndef _H5EBSDVOLUMEREADER_H_
#define _H5EBSDVOLUMEREADER_H_

#include <string>
#include <vector>


#include "EbsdLib/EbsdSetGetMacros.h"
#include "EbsdLib/EbsdLib.h"
#include "EbsdLib/EbsdConstants.h"
#include "EbsdLib/H5EbsdVolumeInfo.h"



/**
 * @class H5EbsdVolumeReader H5EbsdVolumeReader EbsdLib/H5EbsdVolumeReader.h
 * @brief  This class defines the C++ interface that subclasses must implement
 * in order to be able to load EBSD data into the DREAM.3D programs.
 * @author Michael A. Jackson for BlueQuartz Software
 * @date May 23, 2011
 * @version 1.0
 */
class EbsdLib_EXPORT H5EbsdVolumeReader : public H5EbsdVolumeInfo
{

  public:
    EBSD_SHARED_POINTERS(H5EbsdVolumeReader)
    EBSD_TYPE_MACRO_SUPER(H5EbsdVolumeReader, H5EbsdVolumeInfo)
    EBSD_STATIC_NEW_MACRO(H5EbsdVolumeReader)

    virtual ~H5EbsdVolumeReader();

    EBSD_INSTANCE_PROPERTY(bool, Cancel)

    /**
     * @brief This is the actual starting slice that the user wants to start
     * from which may be different from the "ZStartIndex" which is saved in the
     * HDF5 file.
     */
    EBSD_INSTANCE_PROPERTY(int, SliceStart)

    /**
     * @brief This is the actual ending slice that the user wants to start
     * from which may be different from the "ZEndIndex" which is saved in the
     * HDF5 file.
     */
    EBSD_INSTANCE_PROPERTY(int, SliceEnd)

    /**
     * @brief This method does the actual loading of the OIM data from the data
     * source (files, streams, etc) into the data structures. Subclasses need to
     * fully implement this. This is a skeleton method that simply returns an error.
     * @param euler1s The first set of euler angles (phi1)
     * @param euler2s The second set of euler angles (Phi)
     * @param euler3s The third set of euler angles (phi2)
     * @param phases The phase index data
     * @param xpoints The number of x voxels
     * @param ypoints The number of y voxels
     * @param zpoints The number of z voxels
     * @param filters The Quality Metric Filters
     * @return
     */
    virtual int loadData(int64_t xpoints, int64_t ypoints, int64_t zpoints,
                         Ebsd::RefFrameZDir ZDir);


    /** @brief Will this class be responsible for deallocating the memory for the data arrays */
    EBSD_INSTANCE_PROPERTY(bool, ManageMemory)

    /** @brief The number of elements in a column of data. This should be rows * columns */
    EBSD_INSTANCE_PROPERTY(size_t, NumberOfElements)

    /**
    * @brief Returns the pointer to the data for a given field
    * @param fieldName The name of the field to return the pointer to.
    */
    virtual void* getPointerByName(const std::string &fieldName);

    /**
    * @brief Returns an enumeration value that depicts the numerical
    * primitive type that the data is stored as (Int, Float, etc).
    * @param fieldName The name of the field.
    */
    virtual Ebsd::NumType getPointerType(const std::string &fieldName);

    /** @brief Allocates the proper amount of memory (after reading the header portion of the file)
    * and then splats '0' across all the bytes of the memory allocation
    */
    virtual void initPointers(size_t numElements);

    /** @brief 'free's the allocated memory and sets the pointer to NULL
    */
    virtual void deletePointers();

  protected:
    H5EbsdVolumeReader();

  private:
    H5EbsdVolumeReader(const H5EbsdVolumeReader&); // Copy Constructor Not Implemented
    void operator=(const H5EbsdVolumeReader&); // Operator '=' Not Implemented

};



#endif /* _H5EBSDVOLUMEREADER_H_  */
