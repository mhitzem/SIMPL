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

#include "ThirdOrderPolynomialFilterParameter.h"

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
ThirdOrderPolynomialFilterParameter::ThirdOrderPolynomialFilterParameter() = default;

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
ThirdOrderPolynomialFilterParameter::~ThirdOrderPolynomialFilterParameter() = default;

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
ThirdOrderPolynomialFilterParameter::Pointer ThirdOrderPolynomialFilterParameter::Create(const QString& humanLabel, const QString& propertyName, const Float3rdOrderPoly_t& defaultValue,
                                                                                         Category category, const SetterCallbackType& setterCallback, const GetterCallbackType& getterCallback,
                                                                                         int groupIndex)
{

  ThirdOrderPolynomialFilterParameter::Pointer ptr = ThirdOrderPolynomialFilterParameter::New();
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
QString ThirdOrderPolynomialFilterParameter::getWidgetType() const
{
  return QString("ThirdOrderPolynomialWidget");
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ThirdOrderPolynomialFilterParameter::readJson(const QJsonObject& json)
{
  QJsonValue jsonValue = json[getPropertyName()];
  if(jsonValue.isUndefined())
  {
    jsonValue = json[getLegacyPropertyName()];
  }
  if(!jsonValue.isUndefined() && m_SetterCallback)
  {
    QJsonObject obj = jsonValue.toObject();
    Float3rdOrderPoly_t poly;
    poly.readJson(obj);
    m_SetterCallback(poly);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ThirdOrderPolynomialFilterParameter::writeJson(QJsonObject& json) const
{
  if(m_GetterCallback)
  {
    Float3rdOrderPoly_t poly = m_GetterCallback();
    QJsonObject obj;
    poly.writeJson(obj);
    json[getPropertyName()] = obj;
  }
}

// -----------------------------------------------------------------------------
ThirdOrderPolynomialFilterParameter::Pointer ThirdOrderPolynomialFilterParameter::NullPointer()
{
  return Pointer(static_cast<Self*>(nullptr));
}

// -----------------------------------------------------------------------------
ThirdOrderPolynomialFilterParameter::Pointer ThirdOrderPolynomialFilterParameter::New()
{
  Pointer sharedPtr(new(ThirdOrderPolynomialFilterParameter));
  return sharedPtr;
}

// -----------------------------------------------------------------------------
QString ThirdOrderPolynomialFilterParameter::getNameOfClass() const
{
  return QString("ThirdOrderPolynomialFilterParameter");
}

// -----------------------------------------------------------------------------
QString ThirdOrderPolynomialFilterParameter::ClassName()
{
  return QString("ThirdOrderPolynomialFilterParameter");
}

// -----------------------------------------------------------------------------
void ThirdOrderPolynomialFilterParameter::setSetterCallback(const ThirdOrderPolynomialFilterParameter::SetterCallbackType& value)
{
  m_SetterCallback = value;
}

// -----------------------------------------------------------------------------
ThirdOrderPolynomialFilterParameter::SetterCallbackType ThirdOrderPolynomialFilterParameter::getSetterCallback() const
{
  return m_SetterCallback;
}

// -----------------------------------------------------------------------------
void ThirdOrderPolynomialFilterParameter::setGetterCallback(const ThirdOrderPolynomialFilterParameter::GetterCallbackType& value)
{
  m_GetterCallback = value;
}

// -----------------------------------------------------------------------------
ThirdOrderPolynomialFilterParameter::GetterCallbackType ThirdOrderPolynomialFilterParameter::getGetterCallback() const
{
  return m_GetterCallback;
}
