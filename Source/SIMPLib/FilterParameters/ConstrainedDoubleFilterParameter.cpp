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

#include "ConstrainedDoubleFilterParameter.h"

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
ConstrainedDoubleFilterParameter::ConstrainedDoubleFilterParameter() = default;

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
ConstrainedDoubleFilterParameter::~ConstrainedDoubleFilterParameter() = default;

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
ConstrainedDoubleFilterParameter::Pointer ConstrainedDoubleFilterParameter::Create(const QString& humanLabel, const QString& propertyName, double min, double max, double defaultValue,
                                                                                   Category category, const SetterCallbackType& setterCallback, const GetterCallbackType& getterCallback,
                                                                                   int groupIndex)
{

  ConstrainedDoubleFilterParameter::Pointer ptr = ConstrainedDoubleFilterParameter::New();
  ptr->setHumanLabel(humanLabel);
  ptr->setPropertyName(propertyName);
  ptr->setDefaultValue(defaultValue);
  ptr->setCategory(category);
  ptr->setGroupIndex(groupIndex);
  ptr->setSetterCallback(setterCallback);
  ptr->setGetterCallback(getterCallback);
  ptr->setMinimum(min);
  ptr->setMaximum(max);

  return ptr;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString ConstrainedDoubleFilterParameter::getWidgetType() const
{
  return QString("ConstrainedDoubleWidget");
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ConstrainedDoubleFilterParameter::readJson(const QJsonObject& json)
{
  QJsonValue jsonValue = json[getPropertyName()];
  if(!jsonValue.isUndefined() && m_SetterCallback)
  {
    m_SetterCallback(jsonValue.toDouble());
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ConstrainedDoubleFilterParameter::writeJson(QJsonObject& json) const
{
  if(m_GetterCallback)
  {
    json[getPropertyName()] = m_GetterCallback();
  }
}

// -----------------------------------------------------------------------------
ConstrainedDoubleFilterParameter::Pointer ConstrainedDoubleFilterParameter::NullPointer()
{
  return Pointer(static_cast<Self*>(nullptr));
}

// -----------------------------------------------------------------------------
ConstrainedDoubleFilterParameter::Pointer ConstrainedDoubleFilterParameter::New()
{
  Pointer sharedPtr(new(ConstrainedDoubleFilterParameter));
  return sharedPtr;
}

// -----------------------------------------------------------------------------
QString ConstrainedDoubleFilterParameter::getNameOfClass() const
{
  return QString("ConstrainedDoubleFilterParameter");
}

// -----------------------------------------------------------------------------
QString ConstrainedDoubleFilterParameter::ClassName()
{
  return QString("ConstrainedDoubleFilterParameter");
}

// -----------------------------------------------------------------------------
void ConstrainedDoubleFilterParameter::setSetterCallback(const ConstrainedDoubleFilterParameter::SetterCallbackType& value)
{
  m_SetterCallback = value;
}

// -----------------------------------------------------------------------------
ConstrainedDoubleFilterParameter::SetterCallbackType ConstrainedDoubleFilterParameter::getSetterCallback() const
{
  return m_SetterCallback;
}

// -----------------------------------------------------------------------------
void ConstrainedDoubleFilterParameter::setGetterCallback(const ConstrainedDoubleFilterParameter::GetterCallbackType& value)
{
  m_GetterCallback = value;
}

// -----------------------------------------------------------------------------
ConstrainedDoubleFilterParameter::GetterCallbackType ConstrainedDoubleFilterParameter::getGetterCallback() const
{
  return m_GetterCallback;
}
