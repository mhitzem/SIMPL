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

#include "BooleanFilterParameter.h"

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
BooleanFilterParameter::BooleanFilterParameter() = default;

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
BooleanFilterParameter::~BooleanFilterParameter() = default;

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
BooleanFilterParameter::Pointer BooleanFilterParameter::Create(const QString& humanLabel, const QString& propertyName, const bool& defaultValue, Category category,
                                                               const SetterCallbackType& setterCallback, const GetterCallbackType& getterCallback, int groupIndex)
{

  BooleanFilterParameter::Pointer ptr = BooleanFilterParameter::New();
  ptr->setHumanLabel(humanLabel);
  ptr->setPropertyName(propertyName);
  ptr->setDefaultValue(defaultValue);
  ptr->setCategory(category);
  ptr->setGroupIndex(groupIndex);
  ptr->setSetterCallback(setterCallback);
  ptr->setGetterCallback(getterCallback);

  return ptr;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString BooleanFilterParameter::getWidgetType() const
{
  return QString("BooleanWidget");
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void BooleanFilterParameter::readJson(const QJsonObject& json)
{
  QJsonValue jsonValue = json[getPropertyName()];
  if(jsonValue.isUndefined())
  {
    jsonValue = json[getLegacyPropertyName()];
  }
  if(!jsonValue.isUndefined() && m_SetterCallback)
  {
    m_SetterCallback(static_cast<bool>(jsonValue.toInt()));
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void BooleanFilterParameter::writeJson(QJsonObject& json) const
{
  if(m_GetterCallback)
  {
    json[getPropertyName()] = static_cast<int>(m_GetterCallback());
  }
}

// -----------------------------------------------------------------------------
BooleanFilterParameter::Pointer BooleanFilterParameter::NullPointer()
{
  return Pointer(static_cast<Self*>(nullptr));
}

// -----------------------------------------------------------------------------
BooleanFilterParameter::Pointer BooleanFilterParameter::New()
{
  Pointer sharedPtr(new(BooleanFilterParameter));
  return sharedPtr;
}

// -----------------------------------------------------------------------------
QString BooleanFilterParameter::getNameOfClass() const
{
  return QString("BooleanFilterParameter");
}

// -----------------------------------------------------------------------------
QString BooleanFilterParameter::ClassName()
{
  return QString("BooleanFilterParameter");
}

// -----------------------------------------------------------------------------
void BooleanFilterParameter::setSetterCallback(const BooleanFilterParameter::SetterCallbackType& value)
{
  m_SetterCallback = value;
}

// -----------------------------------------------------------------------------
BooleanFilterParameter::SetterCallbackType BooleanFilterParameter::getSetterCallback() const
{
  return m_SetterCallback;
}

// -----------------------------------------------------------------------------
void BooleanFilterParameter::setGetterCallback(const BooleanFilterParameter::GetterCallbackType& value)
{
  m_GetterCallback = value;
}

// -----------------------------------------------------------------------------
BooleanFilterParameter::GetterCallbackType BooleanFilterParameter::getGetterCallback() const
{
  return m_GetterCallback;
}
