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

#include "PreflightUpdatedValueFilterParameter.h"

#include <tuple>

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
PreflightUpdatedValueFilterParameter::PreflightUpdatedValueFilterParameter() = default;

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
PreflightUpdatedValueFilterParameter::~PreflightUpdatedValueFilterParameter() = default;

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
PreflightUpdatedValueFilterParameter::Pointer PreflightUpdatedValueFilterParameter::Create(const QString& humanLabel, const QString& propertyName, const QString& defaultValue, Category category,
                                                                                           GetterCallbackType getterCallback, int groupIndex)
{
  PreflightUpdatedValueFilterParameter::Pointer ptr = PreflightUpdatedValueFilterParameter::New();
  ptr->setHumanLabel(humanLabel);
  ptr->setPropertyName(propertyName);
  ptr->setDefaultValue(defaultValue);
  ptr->setCategory(category);
  ptr->setGroupIndex(groupIndex);
  ptr->setGetterCallback(getterCallback);

  return ptr;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString PreflightUpdatedValueFilterParameter::getWidgetType() const
{
  return QString("PreflightUpdatedValueWidget");
}

// -----------------------------------------------------------------------------
void PreflightUpdatedValueFilterParameter::readJson(const QJsonObject& json)
{
  // We don't want to read this value. Reading does no good since it is a calculated value
  std::ignore = json;
}

// -----------------------------------------------------------------------------
void PreflightUpdatedValueFilterParameter::writeJson(QJsonObject& json) const
{
  // We don't want to write this value. It serves no purpose saving the value
  std::ignore = json;
}

// -----------------------------------------------------------------------------
PreflightUpdatedValueFilterParameter::Pointer PreflightUpdatedValueFilterParameter::NullPointer()
{
  return Pointer(static_cast<Self*>(nullptr));
}

// -----------------------------------------------------------------------------
PreflightUpdatedValueFilterParameter::Pointer PreflightUpdatedValueFilterParameter::New()
{
  Pointer sharedPtr(new(PreflightUpdatedValueFilterParameter));
  return sharedPtr;
}

// -----------------------------------------------------------------------------
QString PreflightUpdatedValueFilterParameter::getNameOfClass() const
{
  return QString("PreflightUpdatedValueFilterParameter");
}

// -----------------------------------------------------------------------------
QString PreflightUpdatedValueFilterParameter::ClassName()
{
  return QString("PreflightUpdatedValueFilterParameter");
}

// -----------------------------------------------------------------------------
void PreflightUpdatedValueFilterParameter::setGetterCallback(const PreflightUpdatedValueFilterParameter::GetterCallbackType& value)
{
  m_GetterCallback = value;
}

// -----------------------------------------------------------------------------
PreflightUpdatedValueFilterParameter::GetterCallbackType PreflightUpdatedValueFilterParameter::getGetterCallback() const
{
  return m_GetterCallback;
}
