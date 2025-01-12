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

#include "ComparisonSelectionFilterParameter.h"

#include <QtCore/QJsonArray>

#include "SIMPLib/Filtering/AbstractFilter.h"

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
ComparisonSelectionFilterParameter::ComparisonSelectionFilterParameter()
: m_ShowOperators(true)
{
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
ComparisonSelectionFilterParameter::~ComparisonSelectionFilterParameter() = default;

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
ComparisonSelectionFilterParameter::Pointer ComparisonSelectionFilterParameter::Create(const QString& humanLabel, const QString& propertyName, ComparisonInputs defaultValue, Category category,
                                                                                       const SetterCallbackType& setterCallback, const GetterCallbackType& getterCallback,
                                                                                       const std::vector<QString>& choices, bool showOperators, int groupIndex)
{
  ComparisonSelectionFilterParameter::Pointer ptr = ComparisonSelectionFilterParameter::New();
  ptr->setHumanLabel(humanLabel);
  ptr->setPropertyName(propertyName);
  QVariant var;
  var.setValue(defaultValue);
  ptr->setDefaultValue(var);
  ptr->setCategory(category);
  ptr->setChoices(choices);
  ptr->setShowOperators(showOperators);
  ptr->setGroupIndex(groupIndex);
  ptr->setSetterCallback(setterCallback);
  ptr->setGetterCallback(getterCallback);

  return ptr;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString ComparisonSelectionFilterParameter::getWidgetType() const
{
  return QString("ComparisonSelectionWidget");
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ComparisonSelectionFilterParameter::readJson(const QJsonObject& json)
{
  QJsonValue jsonValue = json[getPropertyName()];
  if(jsonValue.isUndefined())
  {
    jsonValue = json[getLegacyPropertyName()];
  }
  if(!jsonValue.isUndefined() && m_SetterCallback)
  {
    QJsonArray jsonArray = jsonValue.toArray();

    ComparisonInputs inputs;
    for(int i = 0; i < jsonArray.size(); i++)
    {
      QJsonObject comparisonObj = jsonArray[i].toObject();
      ComparisonInput_t input;
      input.readJson(comparisonObj);
      inputs.addInput(input);
    }

    m_SetterCallback(inputs);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ComparisonSelectionFilterParameter::writeJson(QJsonObject& json) const
{
  if(m_GetterCallback)
  {
    QJsonArray inputsArray;

    ComparisonInputs inputs = m_GetterCallback();
    for(int i = 0; i < inputs.size(); i++)
    {
      ComparisonInput_t input = inputs[i];
      QJsonObject obj;
      input.writeJson(obj);
      inputsArray.push_back(obj);
    }

    json[getPropertyName()] = inputsArray;
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ComparisonSelectionFilterParameter::dataArrayPathRenamed(AbstractFilter* filter, const DataArrayPath::RenameType& renamePath)
{
  DataArrayPath oldPath;
  DataArrayPath newPath;
  std::tie(oldPath, newPath) = renamePath;

  ComparisonInputs inputs = m_GetterCallback();
  bool hasChanges = false;

  int count = inputs.size();
  for(int i = 0; i < count; i++)
  {
    ComparisonInput_t& input = inputs.getInput(i);

    bool hasAttributeMatrix = !oldPath.getAttributeMatrixName().isEmpty();
    bool hasDataArray = !oldPath.getDataArrayName().isEmpty();

    bool sameDC = input.dataContainerName == oldPath.getDataContainerName();
    bool sameAM = input.attributeMatrixName == oldPath.getAttributeMatrixName();
    bool sameDA = input.attributeArrayName == oldPath.getDataArrayName();
    if(sameDC && (!hasAttributeMatrix || (sameAM && (!hasDataArray || sameDA))))
    {
      input.dataContainerName = newPath.getDataContainerName();

      if(hasAttributeMatrix)
      {
        input.attributeMatrixName = newPath.getAttributeMatrixName();
        if(hasDataArray)
        {
          input.attributeArrayName = newPath.getDataArrayName();
        }
      }

      hasChanges = true;
    }
  }

  if(hasChanges)
  {
    m_SetterCallback(inputs);
    Q_EMIT filter->dataArrayPathUpdated(getPropertyName(), renamePath);
  }
}

// -----------------------------------------------------------------------------
ComparisonSelectionFilterParameter::Pointer ComparisonSelectionFilterParameter::NullPointer()
{
  return Pointer(static_cast<Self*>(nullptr));
}

// -----------------------------------------------------------------------------
ComparisonSelectionFilterParameter::Pointer ComparisonSelectionFilterParameter::New()
{
  Pointer sharedPtr(new(ComparisonSelectionFilterParameter));
  return sharedPtr;
}

// -----------------------------------------------------------------------------
QString ComparisonSelectionFilterParameter::getNameOfClass() const
{
  return QString("ComparisonSelectionFilterParameter");
}

// -----------------------------------------------------------------------------
QString ComparisonSelectionFilterParameter::ClassName()
{
  return QString("ComparisonSelectionFilterParameter");
}

// -----------------------------------------------------------------------------
void ComparisonSelectionFilterParameter::setChoices(const std::vector<QString>& value)
{
  m_Choices = value;
}

// -----------------------------------------------------------------------------
std::vector<QString> ComparisonSelectionFilterParameter::getChoices() const
{
  return m_Choices;
}

// -----------------------------------------------------------------------------
void ComparisonSelectionFilterParameter::setShowOperators(bool value)
{
  m_ShowOperators = value;
}

// -----------------------------------------------------------------------------
bool ComparisonSelectionFilterParameter::getShowOperators() const
{
  return m_ShowOperators;
}

// -----------------------------------------------------------------------------
void ComparisonSelectionFilterParameter::setDefaultGeometryTypes(const IGeometry::Types& value)
{
  m_DefaultGeometryTypes = value;
}

// -----------------------------------------------------------------------------
IGeometry::Types ComparisonSelectionFilterParameter::getDefaultGeometryTypes() const
{
  return m_DefaultGeometryTypes;
}

// -----------------------------------------------------------------------------
void ComparisonSelectionFilterParameter::setDefaultAttributeMatrixTypes(const AttributeMatrix::Types& value)
{
  m_DefaultAttributeMatrixTypes = value;
}

// -----------------------------------------------------------------------------
AttributeMatrix::Types ComparisonSelectionFilterParameter::getDefaultAttributeMatrixTypes() const
{
  return m_DefaultAttributeMatrixTypes;
}

// -----------------------------------------------------------------------------
void ComparisonSelectionFilterParameter::setSetterCallback(const ComparisonSelectionFilterParameter::SetterCallbackType& value)
{
  m_SetterCallback = value;
}

// -----------------------------------------------------------------------------
ComparisonSelectionFilterParameter::SetterCallbackType ComparisonSelectionFilterParameter::getSetterCallback() const
{
  return m_SetterCallback;
}

// -----------------------------------------------------------------------------
void ComparisonSelectionFilterParameter::setGetterCallback(const ComparisonSelectionFilterParameter::GetterCallbackType& value)
{
  m_GetterCallback = value;
}

// -----------------------------------------------------------------------------
ComparisonSelectionFilterParameter::GetterCallbackType ComparisonSelectionFilterParameter::getGetterCallback() const
{
  return m_GetterCallback;
}
