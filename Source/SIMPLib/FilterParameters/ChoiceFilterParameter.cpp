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

#include "ChoiceFilterParameter.h"

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
ChoiceFilterParameter::ChoiceFilterParameter()
: m_Editable(false)
{
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
ChoiceFilterParameter::~ChoiceFilterParameter() = default;

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
ChoiceFilterParameter::Pointer ChoiceFilterParameter::Create(const QString& humanLabel, const QString& propertyName, int defaultValue, Category category, const SetterCallbackType& setterCallback,
                                                             const GetterCallbackType& getterCallback, const std::vector<QString>& choices, bool editable, int groupIndex)

{
  ChoiceFilterParameter::Pointer ptr = ChoiceFilterParameter::New();
  ptr->setHumanLabel(humanLabel);
  ptr->setPropertyName(propertyName);
  ptr->setDefaultValue(defaultValue);
  ptr->setCategory(category);
  ptr->setChoices(choices);
  ptr->setEditable(editable);
  ptr->setGroupIndex(groupIndex);
  ptr->setSetterCallback(setterCallback);
  ptr->setGetterCallback(getterCallback);

  return ptr;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString ChoiceFilterParameter::getWidgetType() const
{
  return QString("ChoiceWidget");
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ChoiceFilterParameter::readJson(const QJsonObject& json)
{
  QJsonValue jsonValue = json[getPropertyName()];
  if(jsonValue.isUndefined())
  {
    jsonValue = json[getLegacyPropertyName()];
  }
  if(!jsonValue.isUndefined() && m_SetterCallback)
  {
    m_SetterCallback(jsonValue.toInt(0.0));
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ChoiceFilterParameter::writeJson(QJsonObject& json) const
{
  if(m_GetterCallback)
  {
    json[getPropertyName()] = m_GetterCallback();
  }
}

// -----------------------------------------------------------------------------
ChoiceFilterParameter::Pointer ChoiceFilterParameter::NullPointer()
{
  return Pointer(static_cast<Self*>(nullptr));
}

// -----------------------------------------------------------------------------
ChoiceFilterParameter::Pointer ChoiceFilterParameter::New()
{
  Pointer sharedPtr(new(ChoiceFilterParameter));
  return sharedPtr;
}

// -----------------------------------------------------------------------------
QString ChoiceFilterParameter::getNameOfClass() const
{
  return QString("ChoiceFilterParameter");
}

// -----------------------------------------------------------------------------
QString ChoiceFilterParameter::ClassName()
{
  return QString("ChoiceFilterParameter");
}

// -----------------------------------------------------------------------------
void ChoiceFilterParameter::setChoices(const std::vector<QString>& value)
{
  m_Choices = value;
}

// -----------------------------------------------------------------------------
std::vector<QString> ChoiceFilterParameter::getChoices() const
{
  return m_Choices;
}

// -----------------------------------------------------------------------------
void ChoiceFilterParameter::setEditable(bool value)
{
  m_Editable = value;
}

// -----------------------------------------------------------------------------
bool ChoiceFilterParameter::getEditable() const
{
  return m_Editable;
}

// -----------------------------------------------------------------------------
void ChoiceFilterParameter::setSetterCallback(const ChoiceFilterParameter::SetterCallbackType& value)
{
  m_SetterCallback = value;
}

// -----------------------------------------------------------------------------
ChoiceFilterParameter::SetterCallbackType ChoiceFilterParameter::getSetterCallback() const
{
  return m_SetterCallback;
}

// -----------------------------------------------------------------------------
void ChoiceFilterParameter::setGetterCallback(const ChoiceFilterParameter::GetterCallbackType& value)
{
  m_GetterCallback = value;
}

// -----------------------------------------------------------------------------
ChoiceFilterParameter::GetterCallbackType ChoiceFilterParameter::getGetterCallback() const
{
  return m_GetterCallback;
}
