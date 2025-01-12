/* ============================================================================
 * Copyright (c) 2009-2016 BlueQuartz Software, LLC
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
 * Neither the name of BlueQuartz Software, the US Air Force, nor the names of its
 * contributors may be used to endorse or promote products derived from this software
 * without specific prior written permission.
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
 * The code contained herein was partially funded by the following contracts:
 *    United States Air Force Prime Contract FA8650-07-D-5800
 *    United States Air Force Prime Contract FA8650-10-D-5210
 *    United States Prime Contract Navy N00173-07-C-2068
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

#pragma once

#include <array>
#include <vector>

#include "SIMPLib/SIMPLib.h"

/**
 * @brief The RadialDistributionFunction class can generate different types of distributions
 * for a Radial Distribution Function
 */
class SIMPLib_EXPORT RadialDistributionFunction
{
public:
  virtual ~RadialDistributionFunction();

  /**
   * @brief GenerateRandomDistribution This will generate a random distribution
   * binned up and normalized.
   * @param minDistance The minimum distance between objects
   * @param maxDistance The maximum distance between objects
   * @param numBins The number of bins to generate
   * @param boxdims
   * @param boxres
   * @return An array of values that are the frequency values for the histogram
   */
  static std::vector<float> GenerateRandomDistribution(float minDistance, float maxDistance, int numBins, std::array<float, 3>& boxdims, std::array<float, 3>& boxres);

protected:
  RadialDistributionFunction();

public:
  RadialDistributionFunction(const RadialDistributionFunction&) = delete;            // Copy Constructor Not Implemented
  RadialDistributionFunction(RadialDistributionFunction&&) = delete;                 // Move Constructor Not Implemented
  RadialDistributionFunction& operator=(const RadialDistributionFunction&) = delete; // Copy Assignment Not Implemented
  RadialDistributionFunction& operator=(RadialDistributionFunction&&) = delete;      // Move Assignment Not Implemented
};
