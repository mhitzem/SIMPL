/* ============================================================================
 * Copyright (c) 2010, Michael A. Jackson (BlueQuartz Software)
 * Copyright (c) 2010, Dr. Michael A. Groeber (US Air Force Research Laboratories
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

#include "H5VoxelReader.h"

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
H5VoxelReader::H5VoxelReader()
{

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
H5VoxelReader::~H5VoxelReader()
{

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int H5VoxelReader::getSizeAndResolution(int volDims[3], float spacing[3])
{
  int err = 0;

  if(m_FileName.empty() == true)
  {
    std::cout << "H5ReconVolumeReader Error; Filename was empty" << std::endl;
    return -1;
  }

  OPEN_HDF5_FILE(fileId, m_FileName)OPEN_RECONSTRUCTION_GROUP(reconGid, DREAM3D::HDF5::VoxelDataName.c_str(), fileId)

  err = H5Lite::readPointerDataset(reconGid, H5_DIMENSIONS, volDims);
  if(err < 0)
  {
    std::cout << "H5ReconVolumeReader Error Reading the Dimensions" << std::endl;
    err = H5Gclose(reconGid);
    err = H5Fclose(fileId);
    return err;
  }
  err = H5Lite::readPointerDataset(reconGid, H5_SPACING, spacing);
  if(err < 0)
  {
    std::cout << "H5ReconVolumeReader Error Reading the Spacing (Resolution)" << std::endl;
    err = H5Gclose(reconGid);
    err = H5Fclose(fileId);
    return err;
  }

  err = H5Gclose(reconGid);
  err = H5Fclose(fileId);

  return err;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int H5VoxelReader::readHyperSlab(int xdim, int ydim, int zIndex, int* fileVoxelLayer)
{
  int err = 0;

  if(m_FileName.empty() == true)
  {
    std::cout << "H5ReconVolumeReader Error; Filename was empty" << std::endl;
    return -1;
  }OPEN_HDF5_FILE(fileId, m_FileName)OPEN_RECONSTRUCTION_GROUP(reconGid, DREAM3D::HDF5::VoxelDataName.c_str(), fileId)OPEN_RECONSTRUCTION_GROUP(scalarGid, H5_SCALAR_DATA_GROUP_NAME, reconGid)

  hid_t dataset;
  hid_t filespace;
  hid_t memspace;

  hsize_t col_dims[1];
  hsize_t count[2];
  hsize_t offset[2];
  herr_t status;
  hsize_t rankc = 1;

  dataset = H5Dopen(scalarGid, DREAM3D::VTK::GrainIdScalarName.c_str(), H5P_DEFAULT);
  filespace = H5Dget_space(dataset); /* Get filespace handle first. */
  col_dims[0] = xdim * ydim;
  memspace = H5Screate_simple(rankc, col_dims, NULL);

  offset[0] = zIndex * xdim * ydim;
  count[0] = xdim * ydim;

  status = H5Sselect_hyperslab(filespace, H5S_SELECT_SET, offset, NULL, count, NULL);
  status = H5Dread(dataset, H5T_NATIVE_INT, memspace, filespace, H5P_DEFAULT, fileVoxelLayer);

  H5Sclose(memspace);
  H5Dclose(dataset);
  H5Sclose(filespace);

  err = H5Gclose(scalarGid);
  err = H5Gclose(reconGid);
  err = H5Fclose(fileId);

  return err;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int H5VoxelReader::readVoxelData(AIMArray<int>::Pointer grain_indicies,
                  AIMArray<int>::Pointer phases,
                  AIMArray<float>::Pointer euler1s,
                  AIMArray<float>::Pointer euler2s,
                  AIMArray<float>::Pointer euler3s,
                  std::vector<Ebsd::CrystalStructure> &crystruct,
                  int totalpoints)
{
  int err = 0;
  if(m_FileName.empty() == true)
  {
    m_ErrorMessage = "H5ReconVolumeReader Error; Filename was empty";
    return -1;
  }

  OPEN_HDF5_FILE(fileId, m_FileName);
  OPEN_RECONSTRUCTION_GROUP(reconGid, DREAM3D::HDF5::VoxelDataName.c_str(), fileId);
  OPEN_RECONSTRUCTION_GROUP(scalarGid, H5_SCALAR_DATA_GROUP_NAME, reconGid);

  int* iData = (int*)(malloc(totalpoints * sizeof(int)));

// Read in the Grain ID data
  err = H5Lite::readPointerDataset(scalarGid, DREAM3D::VTK::GrainIdScalarName, iData);
  if(err < 0)
  {
    m_ErrorMessage = "H5ReconVolumeReader Error Reading the Grain IDs";
    free(iData);
    err = H5Gclose(scalarGid);
    err = H5Gclose(reconGid);
    err = H5Fclose(fileId);
    return err;
  }
  for (int i = 0; i < totalpoints; ++i)
  {
    grain_indicies->SetValue(i, iData[i]);
  }

// Read the Phase ID data
  err = H5Lite::readPointerDataset(scalarGid, DREAM3D::VTK::PhaseIdScalarName, iData);
  if(err < 0)
  {
    m_ErrorMessage = "H5ReconVolumeReader Error Reading the Phase IDs";
    free(iData);
    err = H5Gclose(scalarGid);
    err = H5Gclose(reconGid);
    err = H5Fclose(fileId);
    return err;
  }
  for (int i = 0; i < totalpoints; ++i)
  {
    phases->SetValue(i, iData[i]);
  }
  free(iData);

// Read in the Euler Angles Data
  float* fData = (float*)(malloc(totalpoints * 3 * sizeof(float)));
  err = H5Lite::readPointerDataset(scalarGid, DREAM3D::VTK::EulerAnglesName, fData);
  if(err < 0)
  {
    m_ErrorMessage = "H5ReconVolumeReader Error Reading the Euler Angles";
    free(fData);
    err = H5Gclose(scalarGid);
    err = H5Gclose(reconGid);
    err = H5Fclose(fileId);
    return err;
  }
  for (int i = 0; i < totalpoints; ++i)
  {
    euler1s->SetValue(i, fData[i * 3]);
    euler2s->SetValue(i, fData[i * 3 + 1]);
    euler3s->SetValue(i, fData[i * 3 + 2]);
  }
  free(fData);
// Close the group as we are done with it.
  err = H5Gclose(scalarGid);

// Open the Field Data Group
  OPEN_RECONSTRUCTION_GROUP(fieldGid, H5_FIELD_DATA_GROUP_NAME, reconGid);

// Read the CrystalStructure Field Data
  std::vector<unsigned int> xtals;
  err = H5Lite::readVectorDataset(fieldGid, DREAM3D::VTK::CrystalStructureName, xtals);
  if(err < 0)
  {
    m_ErrorMessage = "H5ReconVolumeReader Error Reading the Crystal Structure Field Data";
    err = H5Gclose(fieldGid);
    err = H5Gclose(reconGid);
    err = H5Fclose(fileId);
    return err;
  }
  crystruct.resize(xtals.size());
  for (size_t i = 0; i < xtals.size(); ++i)
  {
    crystruct[i] = static_cast<Ebsd::CrystalStructure>(xtals[i]);
  }

// Close all the HDF5 Groups and close the file
  err = H5Gclose(fieldGid);
  err = H5Gclose(reconGid);
  err = H5Fclose(fileId);

  return err;
}

