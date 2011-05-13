/* ============================================================================
 * Copyright (c) 2010, Michael A. Jackson (BlueQuartz Software)
 * Copyright (c) 2010, Dr. Michael A. Grober (US Air Force Research Laboratories
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
 * Neither the name of Michael A. Jackson nor the names of its contributors may
 * be used to endorse or promote products derived from this software without
 * specific prior written permission.
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
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

#include "GrainGeneratorVoxel.h"

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
GrainGeneratorVoxel::GrainGeneratorVoxel() :
grainname(0),
ellipfunc(0.0),
alreadychecked(0),
euler1(-1.0),
euler2(-1.0),
euler3(-1.0),
phase(0),
neighbor(-1),
numowners(0),
surfacevoxel(0),
unassigned(0)
{
  nearestneighbor[0] = 0; nearestneighbor[1] = 0; nearestneighbor[2] = 0;
  nearestneighbordistance[0] = 0.0; nearestneighbordistance[1] = 0.0; nearestneighbordistance[2] = 0.0;
  quat[0] = 1.0; quat[1] = 0.0; quat[2] = 0.0; quat[3] = 0.0; quat[4] = 0.0;
  grainlist = IntVectorType(new std::vector<int>(0) );
  ellipfunclist = FloatVectorType(new std::vector<float>(0) );
  neighborlist = IntVectorType(new std::vector<int>(0) );
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
GrainGeneratorVoxel::~GrainGeneratorVoxel()
{
#if 0
  if (NULL != grainlist) { delete grainlist; grainlist = NULL;}
  if (NULL != ellipfunclist) { delete ellipfunclist; ellipfunclist = NULL;}
  if (NULL != neighborlist) { delete neighborlist; neighborlist = NULL;}
#endif
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void GrainGeneratorVoxel::deepCopy(GrainGeneratorVoxel* graingeneratorvoxel)
{
  if (graingeneratorvoxel == this) { return; } // The pointers are the same just return

   grainname = graingeneratorvoxel->grainname;
   ellipfunc = graingeneratorvoxel->ellipfunc;
   alreadychecked = graingeneratorvoxel->alreadychecked;
   COPY_ARRAY_3(nearestneighbor, graingeneratorvoxel);

   COPY_ARRAY_3(nearestneighbordistance, graingeneratorvoxel);
   euler1 = graingeneratorvoxel->euler1;
   euler2 = graingeneratorvoxel->euler2;
   euler3 = graingeneratorvoxel->euler3;
   neighbor = graingeneratorvoxel->neighbor;
   numowners = graingeneratorvoxel->numowners;
   surfacevoxel = graingeneratorvoxel->surfacevoxel;
   unassigned = graingeneratorvoxel->unassigned;

   COPY_ARRAY_5(quat, graingeneratorvoxel);
   DEEP_COPY_SHARED_VECTOR(grainlist, graingeneratorvoxel, IntVectorType, int)
   DEEP_COPY_SHARED_VECTOR(ellipfunclist, graingeneratorvoxel, FloatVectorType, float)
   DEEP_COPY_SHARED_VECTOR(neighborlist, graingeneratorvoxel, IntVectorType, int)

}

