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

#include "DataContainerSelectionFilterParameter.h"

#include "SIMPLib/Filtering/AbstractFilter.h"

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
DataContainerSelectionFilterParameter::DataContainerSelectionFilterParameter() = default;

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
DataContainerSelectionFilterParameter::~DataContainerSelectionFilterParameter() = default;

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
DataContainerSelectionFilterParameter::Pointer DataContainerSelectionFilterParameter::Create(const QString& humanLabel, const QString& propertyName, const DataArrayPath& defaultValue,
                                                                                             Category category, const SetterCallbackType& setterCallback, const GetterCallbackType& getterCallback,
                                                                                             const RequirementType& req, int groupIndex)
{
  DataContainerSelectionFilterParameter::Pointer ptr = DataContainerSelectionFilterParameter::New();
  ptr->setHumanLabel(humanLabel);
  ptr->setPropertyName(propertyName);
  QVariant v;
  v.setValue(defaultValue);
  ptr->setDefaultValue(v);
  ptr->setCategory(category);
  ptr->setDefaultGeometryTypes(req.dcGeometryTypes);
  ptr->setGroupIndex(groupIndex);
  ptr->setSetterCallback(setterCallback);
  ptr->setGetterCallback(getterCallback);

  return ptr;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString DataContainerSelectionFilterParameter::getWidgetType() const
{
  return QString("DataContainerSelectionWidget");
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void DataContainerSelectionFilterParameter::readJson(const QJsonObject& json)
{
  QJsonValue jsonValue = json[getPropertyName()];
  if(jsonValue.isUndefined())
  {
    jsonValue = json[getLegacyPropertyName()];
  }
  if(!jsonValue.isUndefined() && m_SetterCallback)
  {
    QJsonObject obj = jsonValue.toObject();
    DataArrayPath dap;
    if(dap.readJson(obj))
    {
      m_SetterCallback(dap);
    }
    else // this is in here for historical where we used to save the value as a string
    {
      QString dcName = jsonValue.toString("");
      DataArrayPath dap(dcName, "", "");
      m_SetterCallback(dap);
    }
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void DataContainerSelectionFilterParameter::writeJson(QJsonObject& json) const
{
  if(m_GetterCallback)
  {
    DataArrayPath dap = m_GetterCallback();
    QJsonObject obj;
    dap.writeJson(obj);
    json[getPropertyName()] = obj;
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void DataContainerSelectionFilterParameter::dataArrayPathRenamed(AbstractFilter* filter, const DataArrayPath::RenameType& renamePath)
{
  DataArrayPath oldPath;
  DataArrayPath newPath;
  std::tie(oldPath, newPath) = renamePath;

  if(oldPath == m_GetterCallback() && oldPath.getDataContainerName() != newPath.getDataContainerName())
  {
    m_SetterCallback(newPath);
    Q_EMIT filter->dataArrayPathUpdated(getPropertyName(), renamePath);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
DataContainerSelectionFilterParameter::RequirementType DataContainerSelectionFilterParameter::getRequirements()
{
  RequirementType reqs;
  reqs.dcGeometryTypes = getDefaultGeometryTypes();

  return reqs;
}

// -----------------------------------------------------------------------------
DataContainerSelectionFilterParameter::Pointer DataContainerSelectionFilterParameter::NullPointer()
{
  return Pointer(static_cast<Self*>(nullptr));
}

// -----------------------------------------------------------------------------
DataContainerSelectionFilterParameter::Pointer DataContainerSelectionFilterParameter::New()
{
  Pointer sharedPtr(new(DataContainerSelectionFilterParameter));
  return sharedPtr;
}

// -----------------------------------------------------------------------------
QString DataContainerSelectionFilterParameter::getNameOfClass() const
{
  return QString("DataContainerSelectionFilterParameter");
}

// -----------------------------------------------------------------------------
QString DataContainerSelectionFilterParameter::ClassName()
{
  return QString("DataContainerSelectionFilterParameter");
}

// -----------------------------------------------------------------------------
void DataContainerSelectionFilterParameter::setDefaultGeometryTypes(const IGeometry::Types& value)
{
  m_DefaultGeometryTypes = value;
}

// -----------------------------------------------------------------------------
IGeometry::Types DataContainerSelectionFilterParameter::getDefaultGeometryTypes() const
{
  return m_DefaultGeometryTypes;
}

// -----------------------------------------------------------------------------
void DataContainerSelectionFilterParameter::setSetterCallback(const DataContainerSelectionFilterParameter::SetterCallbackType& value)
{
  m_SetterCallback = value;
}

// -----------------------------------------------------------------------------
DataContainerSelectionFilterParameter::SetterCallbackType DataContainerSelectionFilterParameter::getSetterCallback() const
{
  return m_SetterCallback;
}

// -----------------------------------------------------------------------------
void DataContainerSelectionFilterParameter::setGetterCallback(const DataContainerSelectionFilterParameter::GetterCallbackType& value)
{
  m_GetterCallback = value;
}

// -----------------------------------------------------------------------------
DataContainerSelectionFilterParameter::GetterCallbackType DataContainerSelectionFilterParameter::getGetterCallback() const
{
  return m_GetterCallback;
}
