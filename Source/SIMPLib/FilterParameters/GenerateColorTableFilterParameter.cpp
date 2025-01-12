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

#include "GenerateColorTableFilterParameter.h"

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
GenerateColorTableFilterParameter::GenerateColorTableFilterParameter() = default;

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
GenerateColorTableFilterParameter::~GenerateColorTableFilterParameter() = default;

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
GenerateColorTableFilterParameter::Pointer GenerateColorTableFilterParameter::Create(const QString& humanLabel, const QString& propertyName, const QString& defaultValue, Category category,
                                                                                     GenerateColorTable* filter, int groupIndex)
{
  GenerateColorTableFilterParameter::Pointer ptr = GenerateColorTableFilterParameter::New();
  ptr->setHumanLabel(humanLabel);
  ptr->setPropertyName(propertyName);
  ptr->setDefaultValue(defaultValue);
  ptr->setCategory(category);
  ptr->setGroupIndex(groupIndex);
  ptr->setFilter(filter);

  return ptr;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString GenerateColorTableFilterParameter::getWidgetType() const
{
  return QString("GenerateColorTableWidget");
}

// -----------------------------------------------------------------------------
// THIS IS A SPECIAL CASE AND IS NOT STANDARD.  DO NOT COPY THIS CODE.
// -----------------------------------------------------------------------------
void GenerateColorTableFilterParameter::readJson(const QJsonObject& json)
{
  QString presetName = json["SelectedPresetName"].toString();
  m_Filter->setSelectedPresetName(presetName);

  QJsonArray presetControlPoints = json["SelectedPresetControlPoints"].toArray();
  m_Filter->setSelectedPresetControlPoints(presetControlPoints);
}

// -----------------------------------------------------------------------------
// THIS IS A SPECIAL CASE AND IS NOT STANDARD.  DO NOT COPY THIS CODE.
// -----------------------------------------------------------------------------
void GenerateColorTableFilterParameter::writeJson(QJsonObject& json) const
{
  QString presetName = m_Filter->getSelectedPresetName();
  QJsonArray presetControlPoints = m_Filter->getSelectedPresetControlPoints();

  json["SelectedPresetName"] = presetName;
  json["SelectedPresetControlPoints"] = presetControlPoints;
}

// -----------------------------------------------------------------------------
GenerateColorTableFilterParameter::Pointer GenerateColorTableFilterParameter::NullPointer()
{
  return Pointer(static_cast<Self*>(nullptr));
}

// -----------------------------------------------------------------------------
GenerateColorTableFilterParameter::Pointer GenerateColorTableFilterParameter::New()
{
  Pointer sharedPtr(new(GenerateColorTableFilterParameter));
  return sharedPtr;
}

// -----------------------------------------------------------------------------
QString GenerateColorTableFilterParameter::getNameOfClass() const
{
  return QString("GenerateColorTableFilterParameter");
}

// -----------------------------------------------------------------------------
QString GenerateColorTableFilterParameter::ClassName()
{
  return QString("GenerateColorTableFilterParameter");
}

// -----------------------------------------------------------------------------
void GenerateColorTableFilterParameter::setFilter(GenerateColorTable* value)
{
  m_Filter = value;
}

// -----------------------------------------------------------------------------
GenerateColorTable* GenerateColorTableFilterParameter::getFilter() const
{
  return m_Filter;
}
