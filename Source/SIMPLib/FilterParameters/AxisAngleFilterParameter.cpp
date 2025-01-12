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

#include "AxisAngleFilterParameter.h"

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
AxisAngleFilterParameter::AxisAngleFilterParameter() = default;

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
AxisAngleFilterParameter::~AxisAngleFilterParameter() = default;

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
AxisAngleFilterParameter::Pointer AxisAngleFilterParameter::Create(const QString& humanLabel, const QString& propertyName, const AxisAngleInput& defaultValue, Category category,
                                                                   const SetterCallbackType& setterCallback, const GetterCallbackType& getterCallback, int groupIndex)
{
  AxisAngleFilterParameter::Pointer ptr = AxisAngleFilterParameter::New();
  ptr->setHumanLabel(humanLabel);
  ptr->setPropertyName(propertyName);
  QVariant v;
  v.setValue(defaultValue);
  ptr->setDefaultValue(v);
  ptr->setCategory(category);
  ptr->setGroupIndex(groupIndex);
  ptr->setSetterCallback(setterCallback);
  ptr->setGetterCallback(getterCallback);

  return ptr;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString AxisAngleFilterParameter::getWidgetType() const
{
  return QString("AxisAngleWidget");
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void AxisAngleFilterParameter::readJson(const QJsonObject& json)
{
  QJsonValue jsonValue = json[getPropertyName()];
  if(jsonValue.isUndefined())
  {
    jsonValue = json[getLegacyPropertyName()];
  }
  if(!jsonValue.isUndefined() && m_SetterCallback)
  {
    QJsonObject axisObj = jsonValue.toObject();
    AxisAngleInput axisAngle;
    axisAngle.readJson(axisObj);
    m_SetterCallback(axisAngle);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void AxisAngleFilterParameter::writeJson(QJsonObject& json) const
{
  if(m_GetterCallback)
  {
    QJsonObject axisObj;
    AxisAngleInput axisAngle = m_GetterCallback();
    axisAngle.writeJson(axisObj);
    json[getPropertyName()] = axisObj;
  }
}

// -----------------------------------------------------------------------------
AxisAngleFilterParameter::Pointer AxisAngleFilterParameter::NullPointer()
{
  return Pointer(static_cast<Self*>(nullptr));
}

// -----------------------------------------------------------------------------
AxisAngleFilterParameter::Pointer AxisAngleFilterParameter::New()
{
  Pointer sharedPtr(new(AxisAngleFilterParameter));
  return sharedPtr;
}

// -----------------------------------------------------------------------------
QString AxisAngleFilterParameter::getNameOfClass() const
{
  return QString("AxisAngleFilterParameter");
}

// -----------------------------------------------------------------------------
QString AxisAngleFilterParameter::ClassName()
{
  return QString("AxisAngleFilterParameter");
}

// -----------------------------------------------------------------------------
void AxisAngleFilterParameter::setSetterCallback(const AxisAngleFilterParameter::SetterCallbackType& value)
{
  m_SetterCallback = value;
}

// -----------------------------------------------------------------------------
AxisAngleFilterParameter::SetterCallbackType AxisAngleFilterParameter::getSetterCallback() const
{
  return m_SetterCallback;
}

// -----------------------------------------------------------------------------
void AxisAngleFilterParameter::setGetterCallback(const AxisAngleFilterParameter::GetterCallbackType& value)
{
  m_GetterCallback = value;
}

// -----------------------------------------------------------------------------
AxisAngleFilterParameter::GetterCallbackType AxisAngleFilterParameter::getGetterCallback() const
{
  return m_GetterCallback;
}
