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

#include "MultiAttributeMatrixSelectionFilterParameter.h"

#include <QtCore/QJsonArray>

#include "SIMPLib/Common/Constants.h"
#include "SIMPLib/Filtering/AbstractFilter.h"

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
MultiAttributeMatrixSelectionFilterParameter::MultiAttributeMatrixSelectionFilterParameter()
: m_DefaultPaths(std::vector<DataArrayPath>())
{
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
MultiAttributeMatrixSelectionFilterParameter::~MultiAttributeMatrixSelectionFilterParameter() = default;

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
MultiAttributeMatrixSelectionFilterParameter::Pointer MultiAttributeMatrixSelectionFilterParameter::Create(const QString& humanLabel, const QString& propertyName,
                                                                                                           const std::vector<DataArrayPath>& defaultValue, Category category,
                                                                                                           const SetterCallbackType& setterCallback, const GetterCallbackType& getterCallback,
                                                                                                           RequirementType req, int groupIndex)
{

  MultiAttributeMatrixSelectionFilterParameter::Pointer ptr = MultiAttributeMatrixSelectionFilterParameter::New();
  ptr->setHumanLabel(humanLabel);
  ptr->setPropertyName(propertyName);
  QVariant v;
  v.setValue(defaultValue);
  ptr->setDefaultValue(v);
  ptr->setCategory(category);
  ptr->setDefaultGeometryTypes(req.dcGeometryTypes);
  ptr->setDefaultAttributeMatrixTypes(req.amTypes);
  ptr->setDefaultAttributeArrayTypes(req.daTypes);
  ptr->setDefaultComponentDimensions(req.componentDimensions);
  ptr->setGroupIndex(groupIndex);
  ptr->setSetterCallback(setterCallback);
  ptr->setGetterCallback(getterCallback);

  return ptr;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString MultiAttributeMatrixSelectionFilterParameter::getWidgetType() const
{
  return QString("MultiAttributeMatrixSelectionWidget");
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
MultiAttributeMatrixSelectionFilterParameter::RequirementType MultiAttributeMatrixSelectionFilterParameter::CreateCategoryRequirement(const QString& primitiveType, size_t allowedCompDim,
                                                                                                                                      AttributeMatrix::Category attributeMatrixCategory)
{
  typedef std::vector<size_t> SizeTVectorType;
  MultiAttributeMatrixSelectionFilterParameter::RequirementType req;
  AttributeMatrix::Types amTypes;
  if(attributeMatrixCategory == AttributeMatrix::Category::Element)
  {
    amTypes.push_back(AttributeMatrix::Type::Cell);
    amTypes.push_back(AttributeMatrix::Type::Face);
    amTypes.push_back(AttributeMatrix::Type::Edge);
    amTypes.push_back(AttributeMatrix::Type::Vertex);
  }
  else if(attributeMatrixCategory == AttributeMatrix::Category::Feature)
  {
    amTypes.push_back(AttributeMatrix::Type::CellFeature);
    amTypes.push_back(AttributeMatrix::Type::FaceFeature);
    amTypes.push_back(AttributeMatrix::Type::EdgeFeature);
    amTypes.push_back(AttributeMatrix::Type::VertexFeature);
  }
  else if(attributeMatrixCategory == AttributeMatrix::Category::Ensemble)
  {
    amTypes.push_back(AttributeMatrix::Type::CellEnsemble);
    amTypes.push_back(AttributeMatrix::Type::FaceEnsemble);
    amTypes.push_back(AttributeMatrix::Type::EdgeEnsemble);
    amTypes.push_back(AttributeMatrix::Type::VertexEnsemble);
  }
  req.amTypes = amTypes;
  if(primitiveType.compare(SIMPL::Defaults::AnyPrimitive) != 0)
  {
    req.daTypes = std::vector<QString>(1, primitiveType);
  }
  if(SIMPL::Defaults::AnyComponentSize != allowedCompDim)
  {
    req.componentDimensions = std::vector<SizeTVectorType>(1, SizeTVectorType(1, allowedCompDim));
  }
  //  if(IGeometry::Type::Unknown != geometryType)
  //  {
  //    req.dcGeometryTypes = IGeometry::Types(1, geometryType);
  //  }
  return req;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
MultiAttributeMatrixSelectionFilterParameter::RequirementType MultiAttributeMatrixSelectionFilterParameter::CreateRequirement(const QString& primitiveType, size_t allowedCompDim,
                                                                                                                              AttributeMatrix::Type attributeMatrixType, IGeometry::Type geometryType)
{
  typedef std::vector<size_t> SizeTVectorType;
  MultiAttributeMatrixSelectionFilterParameter::RequirementType req;
  if(primitiveType.compare(SIMPL::Defaults::AnyPrimitive) != 0)
  {
    req.daTypes = std::vector<QString>(1, primitiveType);
  }
  if(SIMPL::Defaults::AnyComponentSize != allowedCompDim)
  {
    req.componentDimensions = std::vector<SizeTVectorType>(1, SizeTVectorType(1, allowedCompDim));
  }
  if(AttributeMatrix::Type::Any != attributeMatrixType)
  {
    AttributeMatrix::Types amTypes(1, attributeMatrixType);
    req.amTypes = amTypes;
  }
  if(IGeometry::Type::Any != geometryType)
  {
    req.dcGeometryTypes = IGeometry::Types(1, geometryType);
  }
  return req;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void MultiAttributeMatrixSelectionFilterParameter::readJson(const QJsonObject& json)
{
  QJsonValue jsonValue = json[getPropertyName()];
  if(jsonValue.isUndefined())
  {
    jsonValue = json[getLegacyPropertyName()];
  }
  if(!jsonValue.isUndefined() && m_SetterCallback)
  {
    QJsonArray arrayObj = jsonValue.toArray();
    std::vector<DataArrayPath> dapVec;
    for(int i = 0; i < arrayObj.size(); i++)
    {
      QJsonObject obj = arrayObj.at(i).toObject();
      DataArrayPath dap;
      dap.readJson(obj);
      dapVec.push_back(dap);
    }

    m_SetterCallback(dapVec);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void MultiAttributeMatrixSelectionFilterParameter::writeJson(QJsonObject& json) const
{
  if(m_GetterCallback)
  {
    std::vector<DataArrayPath> dapVec = m_GetterCallback();
    QJsonArray arrayObj;

    for(int i = 0; i < dapVec.size(); i++)
    {
      DataArrayPath dap = dapVec[i];
      QJsonObject obj;
      dap.writeJson(obj);
      arrayObj.push_back(obj);
    }

    json[getPropertyName()] = arrayObj;
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void MultiAttributeMatrixSelectionFilterParameter::dataArrayPathRenamed(AbstractFilter* filter, const DataArrayPath::RenameType& renamePath)
{
  DataArrayPath oldPath;
  DataArrayPath newPath;
  std::tie(oldPath, newPath) = renamePath;

  std::vector<DataArrayPath> paths = m_GetterCallback();
  size_t count = paths.size();
  bool updated = false;

  for(size_t i = 0; i < count; i++)
  {
    if(paths[i] == oldPath)
    {
      paths[i] = newPath;
      updated = true;
    }
  }

  m_SetterCallback(paths);
  Q_EMIT filter->dataArrayPathUpdated(getPropertyName(), renamePath);
}

// -----------------------------------------------------------------------------
MultiAttributeMatrixSelectionFilterParameter::Pointer MultiAttributeMatrixSelectionFilterParameter::NullPointer()
{
  return Pointer(static_cast<Self*>(nullptr));
}

// -----------------------------------------------------------------------------
MultiAttributeMatrixSelectionFilterParameter::Pointer MultiAttributeMatrixSelectionFilterParameter::New()
{
  Pointer sharedPtr(new(MultiAttributeMatrixSelectionFilterParameter));
  return sharedPtr;
}

// -----------------------------------------------------------------------------
QString MultiAttributeMatrixSelectionFilterParameter::getNameOfClass() const
{
  return QString("MultiAttributeMatrixSelectionFilterParameter");
}

// -----------------------------------------------------------------------------
QString MultiAttributeMatrixSelectionFilterParameter::ClassName()
{
  return QString("MultiAttributeMatrixSelectionFilterParameter");
}

// -----------------------------------------------------------------------------
void MultiAttributeMatrixSelectionFilterParameter::setDefaultPaths(const std::vector<DataArrayPath>& value)
{
  m_DefaultPaths = value;
}

// -----------------------------------------------------------------------------
std::vector<DataArrayPath> MultiAttributeMatrixSelectionFilterParameter::getDefaultPaths() const
{
  return m_DefaultPaths;
}

// -----------------------------------------------------------------------------
void MultiAttributeMatrixSelectionFilterParameter::setDefaultGeometryTypes(const IGeometry::Types& value)
{
  m_DefaultGeometryTypes = value;
}

// -----------------------------------------------------------------------------
IGeometry::Types MultiAttributeMatrixSelectionFilterParameter::getDefaultGeometryTypes() const
{
  return m_DefaultGeometryTypes;
}

// -----------------------------------------------------------------------------
void MultiAttributeMatrixSelectionFilterParameter::setDefaultAttributeMatrixTypes(const AttributeMatrix::Types& value)
{
  m_DefaultAttributeMatrixTypes = value;
}

// -----------------------------------------------------------------------------
AttributeMatrix::Types MultiAttributeMatrixSelectionFilterParameter::getDefaultAttributeMatrixTypes() const
{
  return m_DefaultAttributeMatrixTypes;
}

// -----------------------------------------------------------------------------
void MultiAttributeMatrixSelectionFilterParameter::setDefaultAttributeArrayTypes(const std::vector<QString>& value)
{
  m_DefaultAttributeArrayTypes = value;
}

// -----------------------------------------------------------------------------
std::vector<QString> MultiAttributeMatrixSelectionFilterParameter::getDefaultAttributeArrayTypes() const
{
  return m_DefaultAttributeArrayTypes;
}

// -----------------------------------------------------------------------------
void MultiAttributeMatrixSelectionFilterParameter::setDefaultComponentDimensions(const std::vector<std::vector<size_t>>& value)
{
  m_DefaultComponentDimensions = value;
}

// -----------------------------------------------------------------------------
std::vector<std::vector<size_t>> MultiAttributeMatrixSelectionFilterParameter::getDefaultComponentDimensions() const
{
  return m_DefaultComponentDimensions;
}

// -----------------------------------------------------------------------------
void MultiAttributeMatrixSelectionFilterParameter::setSetterCallback(const MultiAttributeMatrixSelectionFilterParameter::SetterCallbackType& value)
{
  m_SetterCallback = value;
}

// -----------------------------------------------------------------------------
MultiAttributeMatrixSelectionFilterParameter::SetterCallbackType MultiAttributeMatrixSelectionFilterParameter::getSetterCallback() const
{
  return m_SetterCallback;
}

// -----------------------------------------------------------------------------
void MultiAttributeMatrixSelectionFilterParameter::setGetterCallback(const MultiAttributeMatrixSelectionFilterParameter::GetterCallbackType& value)
{
  m_GetterCallback = value;
}

// -----------------------------------------------------------------------------
MultiAttributeMatrixSelectionFilterParameter::GetterCallbackType MultiAttributeMatrixSelectionFilterParameter::getGetterCallback() const
{
  return m_GetterCallback;
}
