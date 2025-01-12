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

#include "DynamicTableFilterParameter.h"

#include <QtCore/QSet>

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
DynamicTableFilterParameter::DynamicTableFilterParameter()
: m_ErrorCondition(0)
{
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
DynamicTableFilterParameter::~DynamicTableFilterParameter() = default;

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
DynamicTableFilterParameter::Pointer DynamicTableFilterParameter::Create(const QString& humanLabel, const QString& propertyName, DynamicTableData defaultTableData, FilterParameter::Category category,
                                                                         const SetterCallbackType& setterCallback, const GetterCallbackType& getterCallback, int groupIndex)
{
  DynamicTableFilterParameter::Pointer ptr = DynamicTableFilterParameter::New();
  ptr->setHumanLabel(humanLabel);
  ptr->setPropertyName(propertyName);
  ptr->setCategory(category);
  ptr->setReadOnly(false);
  ptr->setGroupIndex(groupIndex);
  ptr->setSetterCallback(setterCallback);
  ptr->setGetterCallback(getterCallback);

  // Check that all columns are initialized to the same size
  if(!defaultTableData.getTableData().empty())
  {
    QSet<int> colSizes;
    colSizes.insert(defaultTableData.getTableData()[0].size());
    for(int i = 1; i < defaultTableData.getTableData().size(); i++)
    {
      colSizes.insert(defaultTableData.getTableData()[i].size());
      if(colSizes.size() > 1)
      {
        ptr->setErrorCondition(-100);
        // Use HTML code in the error message, because this will be displayed in RichText format.
        ptr->setErrorMessage("Column " + QString::number(i) + " has a different size than the other columns.<br>Please make all columns the same size.");
        return ptr;
      }
    }
  }

  ptr->setErrorMessage("There is no error.");
  return ptr;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString DynamicTableFilterParameter::getWidgetType() const
{
  return QString("DynamicTableWidget");
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void DynamicTableFilterParameter::readJson(const QJsonObject& json)
{
  QJsonValue jsonValue = json[getPropertyName()];
  if(jsonValue.isUndefined())
  {
    jsonValue = json[getLegacyPropertyName()];
  }
  if(!jsonValue.isUndefined() && m_SetterCallback)
  {
    QJsonObject jsonObj = jsonValue.toObject();
    DynamicTableData dynamicData;
    dynamicData.readJson(jsonObj);
    m_SetterCallback(dynamicData);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void DynamicTableFilterParameter::writeJson(QJsonObject& json) const
{
  if(m_GetterCallback)
  {
    DynamicTableData dynamicData = m_GetterCallback();
    QJsonObject jsonObj;
    dynamicData.writeJson(jsonObj);
    json[getPropertyName()] = jsonObj;
  }
}

// -----------------------------------------------------------------------------
DynamicTableFilterParameter::Pointer DynamicTableFilterParameter::NullPointer()
{
  return Pointer(static_cast<Self*>(nullptr));
}

// -----------------------------------------------------------------------------
DynamicTableFilterParameter::Pointer DynamicTableFilterParameter::New()
{
  Pointer sharedPtr(new(DynamicTableFilterParameter));
  return sharedPtr;
}

// -----------------------------------------------------------------------------
QString DynamicTableFilterParameter::getNameOfClass() const
{
  return QString("DynamicTableFilterParameter");
}

// -----------------------------------------------------------------------------
QString DynamicTableFilterParameter::ClassName()
{
  return QString("DynamicTableFilterParameter");
}

// -----------------------------------------------------------------------------
void DynamicTableFilterParameter::setDefaultTableData(const DynamicTableData& value)
{
  m_DefaultTableData = value;
}

// -----------------------------------------------------------------------------
DynamicTableData DynamicTableFilterParameter::getDefaultTableData() const
{
  return m_DefaultTableData;
}

// -----------------------------------------------------------------------------
void DynamicTableFilterParameter::setErrorCondition(int value)
{
  m_ErrorCondition = value;
}

// -----------------------------------------------------------------------------
int DynamicTableFilterParameter::getErrorCondition() const
{
  return m_ErrorCondition;
}

// -----------------------------------------------------------------------------
void DynamicTableFilterParameter::setErrorMessage(const QString& value)
{
  m_ErrorMessage = value;
}

// -----------------------------------------------------------------------------
QString DynamicTableFilterParameter::getErrorMessage() const
{
  return m_ErrorMessage;
}

// -----------------------------------------------------------------------------
void DynamicTableFilterParameter::setSetterCallback(const DynamicTableFilterParameter::SetterCallbackType& value)
{
  m_SetterCallback = value;
}

// -----------------------------------------------------------------------------
DynamicTableFilterParameter::SetterCallbackType DynamicTableFilterParameter::getSetterCallback() const
{
  return m_SetterCallback;
}

// -----------------------------------------------------------------------------
void DynamicTableFilterParameter::setGetterCallback(const DynamicTableFilterParameter::GetterCallbackType& value)
{
  m_GetterCallback = value;
}

// -----------------------------------------------------------------------------
DynamicTableFilterParameter::GetterCallbackType DynamicTableFilterParameter::getGetterCallback() const
{
  return m_GetterCallback;
}
