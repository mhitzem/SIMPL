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

#include "PhaseTypeSelectionFilterParameter.h"

#include <QtCore/QJsonArray>

#include "SIMPLib/Filtering/AbstractFilter.h"

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
PhaseTypeSelectionFilterParameter::PhaseTypeSelectionFilterParameter() = default;

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
PhaseTypeSelectionFilterParameter::~PhaseTypeSelectionFilterParameter() = default;

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
PhaseTypeSelectionFilterParameter::Pointer PhaseTypeSelectionFilterParameter::Create(const QString& humanLabel, const QString& phaseTypeDataProperty, const DataArrayPath& attributeMatrixDefault,
                                                                                     Category category, const SetterCallbackType& setterCallback, const GetterCallbackType& getterCallback,
                                                                                     const QString& PhaseTypesArrayName, const QString& phaseTypeCountProperty, const QString& attributeMatrixProperty,
                                                                                     const std::vector<QString>& phaseListChoices, int groupIndex)
{
  PhaseTypeSelectionFilterParameter::Pointer ptr = PhaseTypeSelectionFilterParameter::New();
  ptr->setHumanLabel(humanLabel);
  ptr->setPropertyName(PhaseTypesArrayName);
  ptr->setCategory(category);
  ptr->setGroupIndex(groupIndex);
  ptr->setPhaseListChoices(phaseListChoices);
  ptr->setPhaseTypeCountProperty(phaseTypeCountProperty);
  ptr->setPhaseTypeDataProperty(phaseTypeDataProperty);
  ptr->setAttributeMatrixPathProperty(attributeMatrixProperty);
  ptr->setAttributeMatrixPathDefault(attributeMatrixDefault);
  ptr->setSetterCallback(setterCallback);
  ptr->setGetterCallback(getterCallback);

  return ptr;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString PhaseTypeSelectionFilterParameter::getWidgetType() const
{
  return QString("PhaseTypeSelectionWidget");
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PhaseTypeSelectionFilterParameter::readJson(const QJsonObject& json)
{
  QJsonValue jsonValue = json[getPropertyName()];
  if(jsonValue.isUndefined())
  {
    jsonValue = json[getLegacyPropertyName()];
  }
  if(!jsonValue.isUndefined() && m_SetterCallback)
  {
    QJsonArray jsonArray = jsonValue.toArray();
    PhaseType::Types vec;
    for(int i = 0; i < jsonArray.size(); i++)
    {
      vec.push_back(static_cast<PhaseType::Type>(jsonArray[i].toInt()));
    }
    m_SetterCallback(vec);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PhaseTypeSelectionFilterParameter::writeJson(QJsonObject& json) const
{
  if(m_GetterCallback)
  {
    PhaseType::Types vec = m_GetterCallback();
    QJsonArray jsonArray;

    for(int i = 0; i < vec.size(); i++)
    {
      jsonArray.push_back(static_cast<int>(vec[i]));
    }

    json[getPropertyName()] = jsonArray;
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PhaseTypeSelectionFilterParameter::dataArrayPathRenamed(AbstractFilter* filter, const DataArrayPath::RenameType& renamePath)
{
  QVariant var = filter->property(qPrintable(getAttributeMatrixPathProperty()));
  if(var.isValid() && var.canConvert<DataArrayPath>())
  {
    DataArrayPath path = var.value<DataArrayPath>();
    if(path.updatePath(renamePath))
    {
      var.setValue(path);
      filter->setProperty(qPrintable(getAttributeMatrixPathProperty()), var);
      Q_EMIT filter->dataArrayPathUpdated(getAttributeMatrixPathProperty(), renamePath);
    }
  }
}

// -----------------------------------------------------------------------------
PhaseTypeSelectionFilterParameter::Pointer PhaseTypeSelectionFilterParameter::NullPointer()
{
  return Pointer(static_cast<Self*>(nullptr));
}

// -----------------------------------------------------------------------------
PhaseTypeSelectionFilterParameter::Pointer PhaseTypeSelectionFilterParameter::New()
{
  Pointer sharedPtr(new(PhaseTypeSelectionFilterParameter));
  return sharedPtr;
}

// -----------------------------------------------------------------------------
QString PhaseTypeSelectionFilterParameter::getNameOfClass() const
{
  return QString("PhaseTypeSelectionFilterParameter");
}

// -----------------------------------------------------------------------------
QString PhaseTypeSelectionFilterParameter::ClassName()
{
  return QString("PhaseTypeSelectionFilterParameter");
}

// -----------------------------------------------------------------------------
void PhaseTypeSelectionFilterParameter::setPhaseTypeCountProperty(const QString& value)
{
  m_PhaseTypeCountProperty = value;
}

// -----------------------------------------------------------------------------
QString PhaseTypeSelectionFilterParameter::getPhaseTypeCountProperty() const
{
  return m_PhaseTypeCountProperty;
}

// -----------------------------------------------------------------------------
void PhaseTypeSelectionFilterParameter::setPhaseTypeDataProperty(const QString& value)
{
  m_PhaseTypeDataProperty = value;
}

// -----------------------------------------------------------------------------
QString PhaseTypeSelectionFilterParameter::getPhaseTypeDataProperty() const
{
  return m_PhaseTypeDataProperty;
}

// -----------------------------------------------------------------------------
void PhaseTypeSelectionFilterParameter::setAttributeMatrixPathProperty(const QString& value)
{
  m_AttributeMatrixPathProperty = value;
}

// -----------------------------------------------------------------------------
QString PhaseTypeSelectionFilterParameter::getAttributeMatrixPathProperty() const
{
  return m_AttributeMatrixPathProperty;
}

// -----------------------------------------------------------------------------
void PhaseTypeSelectionFilterParameter::setAttributeMatrixPathDefault(const DataArrayPath& value)
{
  m_AttributeMatrixPathDefault = value;
}

// -----------------------------------------------------------------------------
DataArrayPath PhaseTypeSelectionFilterParameter::getAttributeMatrixPathDefault() const
{
  return m_AttributeMatrixPathDefault;
}

// -----------------------------------------------------------------------------
void PhaseTypeSelectionFilterParameter::setPhaseListChoices(const std::vector<QString>& value)
{
  m_PhaseListChoices = value;
}

// -----------------------------------------------------------------------------
std::vector<QString> PhaseTypeSelectionFilterParameter::getPhaseListChoices() const
{
  return m_PhaseListChoices;
}

// -----------------------------------------------------------------------------
void PhaseTypeSelectionFilterParameter::setDefaultGeometryTypes(const IGeometry::Types& value)
{
  m_DefaultGeometryTypes = value;
}

// -----------------------------------------------------------------------------
IGeometry::Types PhaseTypeSelectionFilterParameter::getDefaultGeometryTypes() const
{
  return m_DefaultGeometryTypes;
}

// -----------------------------------------------------------------------------
void PhaseTypeSelectionFilterParameter::setDefaultAttributeMatrixTypes(const AttributeMatrix::Types& value)
{
  m_DefaultAttributeMatrixTypes = value;
}

// -----------------------------------------------------------------------------
AttributeMatrix::Types PhaseTypeSelectionFilterParameter::getDefaultAttributeMatrixTypes() const
{
  return m_DefaultAttributeMatrixTypes;
}

// -----------------------------------------------------------------------------
void PhaseTypeSelectionFilterParameter::setSetterCallback(const PhaseTypeSelectionFilterParameter::SetterCallbackType& value)
{
  m_SetterCallback = value;
}

// -----------------------------------------------------------------------------
PhaseTypeSelectionFilterParameter::SetterCallbackType PhaseTypeSelectionFilterParameter::getSetterCallback() const
{
  return m_SetterCallback;
}

// -----------------------------------------------------------------------------
void PhaseTypeSelectionFilterParameter::setGetterCallback(const PhaseTypeSelectionFilterParameter::GetterCallbackType& value)
{
  m_GetterCallback = value;
}

// -----------------------------------------------------------------------------
PhaseTypeSelectionFilterParameter::GetterCallbackType PhaseTypeSelectionFilterParameter::getGetterCallback() const
{
  return m_GetterCallback;
}
