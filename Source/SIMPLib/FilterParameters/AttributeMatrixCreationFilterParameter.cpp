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

#include "AttributeMatrixCreationFilterParameter.h"
#include "SIMPLib/Filtering/AbstractFilter.h"

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
AttributeMatrixCreationFilterParameter::AttributeMatrixCreationFilterParameter() = default;

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
AttributeMatrixCreationFilterParameter::~AttributeMatrixCreationFilterParameter() = default;

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
AttributeMatrixCreationFilterParameter::Pointer AttributeMatrixCreationFilterParameter::Create(const QString& humanLabel, const QString& propertyName, const DataArrayPath& defaultValue,
                                                                                               Category category, const SetterCallbackType& setterCallback, const GetterCallbackType& getterCallback,
                                                                                               const RequirementType& req, int groupIndex)
{
  AttributeMatrixCreationFilterParameter::Pointer ptr = AttributeMatrixCreationFilterParameter::New();
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
QString AttributeMatrixCreationFilterParameter::getWidgetType() const
{
  return QString("AttributeMatrixCreationWidget");
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void AttributeMatrixCreationFilterParameter::readJson(const QJsonObject& json)
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
    dap.readJson(obj);
    m_SetterCallback(dap);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void AttributeMatrixCreationFilterParameter::writeJson(QJsonObject& json) const
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
void AttributeMatrixCreationFilterParameter::dataArrayPathRenamed(AbstractFilter* filter, const DataArrayPath::RenameType& renamePath)
{
  QVariant var = filter->property(qPrintable(getPropertyName()));
  if(var.isValid() && var.canConvert<DataArrayPath>())
  {
    DataArrayPath path = var.value<DataArrayPath>();
    if(path.updatePath(renamePath))
    {
      if(m_SetterCallback)
      {
        m_SetterCallback(path);
      }
      Q_EMIT filter->dataArrayPathUpdated(getPropertyName(), renamePath);
    }
  }
}

// -----------------------------------------------------------------------------
AttributeMatrixCreationFilterParameter::Pointer AttributeMatrixCreationFilterParameter::NullPointer()
{
  return Pointer(static_cast<Self*>(nullptr));
}

// -----------------------------------------------------------------------------
AttributeMatrixCreationFilterParameter::Pointer AttributeMatrixCreationFilterParameter::New()
{
  Pointer sharedPtr(new(AttributeMatrixCreationFilterParameter));
  return sharedPtr;
}

// -----------------------------------------------------------------------------
QString AttributeMatrixCreationFilterParameter::getNameOfClass() const
{
  return QString("AttributeMatrixCreationFilterParameter");
}

// -----------------------------------------------------------------------------
QString AttributeMatrixCreationFilterParameter::ClassName()
{
  return QString("AttributeMatrixCreationFilterParameter");
}

// -----------------------------------------------------------------------------
void AttributeMatrixCreationFilterParameter::setDefaultGeometryTypes(const IGeometry::Types& value)
{
  m_DefaultGeometryTypes = value;
}

// -----------------------------------------------------------------------------
IGeometry::Types AttributeMatrixCreationFilterParameter::getDefaultGeometryTypes() const
{
  return m_DefaultGeometryTypes;
}

// -----------------------------------------------------------------------------
void AttributeMatrixCreationFilterParameter::setSetterCallback(const AttributeMatrixCreationFilterParameter::SetterCallbackType& value)
{
  m_SetterCallback = value;
}

// -----------------------------------------------------------------------------
AttributeMatrixCreationFilterParameter::SetterCallbackType AttributeMatrixCreationFilterParameter::getSetterCallback() const
{
  return m_SetterCallback;
}

// -----------------------------------------------------------------------------
void AttributeMatrixCreationFilterParameter::setGetterCallback(const AttributeMatrixCreationFilterParameter::GetterCallbackType& value)
{
  m_GetterCallback = value;
}

// -----------------------------------------------------------------------------
AttributeMatrixCreationFilterParameter::GetterCallbackType AttributeMatrixCreationFilterParameter::getGetterCallback() const
{
  return m_GetterCallback;
}
