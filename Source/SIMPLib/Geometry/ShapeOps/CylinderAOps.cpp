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

#include "CylinderAOps.h"

#include "SIMPLib/Math/SIMPLibMath.h"

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
CylinderAOps::CylinderAOps() = default;

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
CylinderAOps::~CylinderAOps() = default;

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
float CylinderAOps::radcur1(QMap<ArgName, float> args)
{
  float radcur1 = 0.0f;

  float volcur = args[VolCur];
  float bovera = args[B_OverA];
  float covera = args[C_OverA];

  // the equation for volume for an A cylinder is pi*b*c*h where b and c are semi axis lengths, but
  // h is a full axis length - meaning h = 2a. However, since our aspect ratios relate semi axis lengths, the 2.0
  // factor can be ingored in this part
  radcur1 = static_cast<float>((volcur * SIMPLib::Constants::k_1OverPiD * (1.0f / bovera) * (1.0f / covera)));
  radcur1 = powf(radcur1, 0.333333333333f);
  return radcur1;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
float CylinderAOps::inside(float axis1comp, float axis2comp, float axis3comp)
{
  float inside = -1.0;
  if(fabs(axis1comp) <= 1.0)
  {
    // inside = 1.0;
    axis2comp = fabs(axis2comp);
    axis3comp = fabs(axis3comp);
    axis2comp = axis2comp * axis2comp;
    axis3comp = axis3comp * axis3comp;
    inside = static_cast<float>(1.0 - axis2comp - axis3comp);
  }
  return inside;
}

// -----------------------------------------------------------------------------
CylinderAOps::Pointer CylinderAOps::NullPointer()
{
  return Pointer(static_cast<Self*>(nullptr));
}

// -----------------------------------------------------------------------------
QString CylinderAOps::getNameOfClass() const
{
  return QString("CylinderAOps");
}

// -----------------------------------------------------------------------------
QString CylinderAOps::ClassName()
{
  return QString("CylinderAOps");
}

// -----------------------------------------------------------------------------
CylinderAOps::Pointer CylinderAOps::New()
{
  Pointer sharedPtr(new(CylinderAOps));
  return sharedPtr;
}
