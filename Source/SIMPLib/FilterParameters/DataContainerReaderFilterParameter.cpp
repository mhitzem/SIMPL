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

#include "DataContainerReaderFilterParameter.h"

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
DataContainerReaderFilterParameter::DataContainerReaderFilterParameter()
: m_DefaultFlagValue(Qt::Checked)
{
  setFileExtension(".dream3d");
  setFileType("");
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
DataContainerReaderFilterParameter::~DataContainerReaderFilterParameter() = default;

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
DataContainerReaderFilterParameter::Pointer DataContainerReaderFilterParameter::Create(const QString& humanLabel, const QString& propertyName, const QString& defaultValue, Category category,
                                                                                       DataContainerReader* filter, int groupIndex)
{
  DataContainerReaderFilterParameter::Pointer ptr = DataContainerReaderFilterParameter::New();
  ptr->setHumanLabel(humanLabel);
  ptr->setPropertyName(ptr->getInputFileProperty());
  ptr->setDefaultValue(defaultValue);
  ptr->setCategory(category);
  ptr->setGroupIndex(groupIndex);
  ptr->setFileExtension(".dream3d");
  ptr->setFileType("");
  ptr->setFilter(filter);

  return ptr;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString DataContainerReaderFilterParameter::getWidgetType() const
{
  return QString("DataContainerReaderWidget");
}

// -----------------------------------------------------------------------------
// THIS IS A SPECIAL CASE AND IS NOT STANDARD.  DO NOT COPY THIS CODE.
// -----------------------------------------------------------------------------
void DataContainerReaderFilterParameter::readJson(const QJsonObject& json)
{
  QJsonValue jsonValue = json[getPropertyName()];
  if(jsonValue.isUndefined())
  {
    jsonValue = json[getLegacyPropertyName()];
  }
  if(!jsonValue.isUndefined())
  {
    QJsonObject jsonObject = jsonValue.toObject();
    DataContainerArrayProxy proxy;
    proxy.readJson(jsonObject);
    m_Filter->setInputFileDataContainerArrayProxy(proxy);
  }

  m_Filter->setInputFile(json["InputFile"].toString());
}

// -----------------------------------------------------------------------------
// THIS IS A SPECIAL CASE AND IS NOT STANDARD.  DO NOT COPY THIS CODE.
// -----------------------------------------------------------------------------
void DataContainerReaderFilterParameter::writeJson(QJsonObject& json) const
{
  DataContainerArrayProxy proxy = m_Filter->getInputFileDataContainerArrayProxy();
  QJsonObject obj;
  proxy.writeJson(obj);
  json[getPropertyName()] = obj;
  json["InputFile"] = m_Filter->getInputFile();
}

// -----------------------------------------------------------------------------
DataContainerReaderFilterParameter::Pointer DataContainerReaderFilterParameter::NullPointer()
{
  return Pointer(static_cast<Self*>(nullptr));
}

// -----------------------------------------------------------------------------
DataContainerReaderFilterParameter::Pointer DataContainerReaderFilterParameter::New()
{
  Pointer sharedPtr(new(DataContainerReaderFilterParameter));
  return sharedPtr;
}

// -----------------------------------------------------------------------------
QString DataContainerReaderFilterParameter::getNameOfClass() const
{
  return QString("DataContainerReaderFilterParameter");
}

// -----------------------------------------------------------------------------
QString DataContainerReaderFilterParameter::ClassName()
{
  return QString("DataContainerReaderFilterParameter");
}

// -----------------------------------------------------------------------------
void DataContainerReaderFilterParameter::setDefaultFlagValue(const Qt::CheckState& value)
{
  m_DefaultFlagValue = value;
}

// -----------------------------------------------------------------------------
Qt::CheckState DataContainerReaderFilterParameter::getDefaultFlagValue() const
{
  return m_DefaultFlagValue;
}

// -----------------------------------------------------------------------------
void DataContainerReaderFilterParameter::setInputFileProperty(const QString& value)
{
  m_InputFileProperty = value;
}

// -----------------------------------------------------------------------------
QString DataContainerReaderFilterParameter::getInputFileProperty() const
{
  return m_InputFileProperty;
}

// -----------------------------------------------------------------------------
void DataContainerReaderFilterParameter::setFileExtension(const QString& value)
{
  m_FileExtension = value;
}

// -----------------------------------------------------------------------------
QString DataContainerReaderFilterParameter::getFileExtension() const
{
  return m_FileExtension;
}

// -----------------------------------------------------------------------------
void DataContainerReaderFilterParameter::setFileType(const QString& value)
{
  m_FileType = value;
}

// -----------------------------------------------------------------------------
QString DataContainerReaderFilterParameter::getFileType() const
{
  return m_FileType;
}

// -----------------------------------------------------------------------------
void DataContainerReaderFilterParameter::setFilter(DataContainerReader* value)
{
  m_Filter = value;
}

// -----------------------------------------------------------------------------
DataContainerReader* DataContainerReaderFilterParameter::getFilter() const
{
  return m_Filter;
}
