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

#include "FloatVec3FilterParameter.h"

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
FloatVec3FilterParameter::FloatVec3FilterParameter() = default;

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
FloatVec3FilterParameter::~FloatVec3FilterParameter() = default;

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
FloatVec3FilterParameter::Pointer FloatVec3FilterParameter::Create(const QString& humanLabel, const QString& propertyName, const FloatVec3Type& defaultValue, Category category,
                                                                   const SetterCallbackType& setterCallback, const GetterCallbackType& getterCallback, int groupIndex)
{

  FloatVec3FilterParameter::Pointer ptr = FloatVec3FilterParameter::New();
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
QString FloatVec3FilterParameter::getWidgetType() const
{
  return QString("FloatVec3Widget");
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void FloatVec3FilterParameter::readJson(const QJsonObject& json)
{
  QJsonValue jsonValue = json[getPropertyName()];
  if(jsonValue.isUndefined())
  {
    jsonValue = json[getLegacyPropertyName()];
  }
  if(!jsonValue.isUndefined() && m_SetterCallback)
  {
    QJsonObject json = jsonValue.toObject();
    FloatVec3Type floatVec3;
    if(json["x"].isDouble() && json["y"].isDouble() && json["z"].isDouble())
    {
      floatVec3[0] = static_cast<float>(json["x"].toDouble());
      floatVec3[1] = static_cast<float>(json["y"].toDouble());
      floatVec3[2] = static_cast<float>(json["z"].toDouble());
    }
    m_SetterCallback(floatVec3);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void FloatVec3FilterParameter::writeJson(QJsonObject& obj) const
{
  if(m_GetterCallback)
  {
    FloatVec3Type floatVec3 = m_GetterCallback();
    QJsonObject json;
    json["x"] = static_cast<double>(floatVec3.getX());
    json["y"] = static_cast<double>(floatVec3.getY());
    json["z"] = static_cast<double>(floatVec3.getZ());
    obj[getPropertyName()] = json;
  }
}

// -----------------------------------------------------------------------------
FloatVec3FilterParameter::Pointer FloatVec3FilterParameter::NullPointer()
{
  return Pointer(static_cast<Self*>(nullptr));
}

// -----------------------------------------------------------------------------
FloatVec3FilterParameter::Pointer FloatVec3FilterParameter::New()
{
  Pointer sharedPtr(new(FloatVec3FilterParameter));
  return sharedPtr;
}

// -----------------------------------------------------------------------------
QString FloatVec3FilterParameter::getNameOfClass() const
{
  return QString("FloatVec3FilterParameter");
}

// -----------------------------------------------------------------------------
QString FloatVec3FilterParameter::ClassName()
{
  return QString("FloatVec3FilterParameter");
}

// -----------------------------------------------------------------------------
void FloatVec3FilterParameter::setSetterCallback(const FloatVec3FilterParameter::SetterCallbackType& value)
{
  m_SetterCallback = value;
}

// -----------------------------------------------------------------------------
FloatVec3FilterParameter::SetterCallbackType FloatVec3FilterParameter::getSetterCallback() const
{
  return m_SetterCallback;
}

// -----------------------------------------------------------------------------
void FloatVec3FilterParameter::setGetterCallback(const FloatVec3FilterParameter::GetterCallbackType& value)
{
  m_GetterCallback = value;
}

// -----------------------------------------------------------------------------
FloatVec3FilterParameter::GetterCallbackType FloatVec3FilterParameter::getGetterCallback() const
{
  return m_GetterCallback;
}
