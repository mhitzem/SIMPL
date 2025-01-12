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
#include "CreateFeatureArrayFromElementArray.h"

#include <QtCore/QTextStream>

#include "SIMPLib/SIMPLibVersion.h"
#include "SIMPLib/Common/Constants.h"
#include "SIMPLib/Common/TemplateHelpers.h"
#include "SIMPLib/DataContainers/DataContainerArray.h"
#include "SIMPLib/FilterParameters/AbstractFilterParametersReader.h"
#include "SIMPLib/FilterParameters/AttributeMatrixSelectionFilterParameter.h"
#include "SIMPLib/FilterParameters/DataArraySelectionFilterParameter.h"
#include "SIMPLib/FilterParameters/LinkedPathCreationFilterParameter.h"
#include "SIMPLib/FilterParameters/SeparatorFilterParameter.h"
#include "SIMPLib/FilterParameters/StringFilterParameter.h"

enum createdPathID : RenameDataPath::DataID_t
{
  FeatureArrayID = 1
};

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
CreateFeatureArrayFromElementArray::CreateFeatureArrayFromElementArray() = default;

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
CreateFeatureArrayFromElementArray::~CreateFeatureArrayFromElementArray() = default;

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void CreateFeatureArrayFromElementArray::setupFilterParameters()
{
  FilterParameterVectorType parameters;
  parameters.push_back(SeparatorFilterParameter::Create("Element Data", FilterParameter::Category::RequiredArray));
  {
    DataArraySelectionFilterParameter::RequirementType req =
        DataArraySelectionFilterParameter::CreateCategoryRequirement(SIMPL::Defaults::AnyPrimitive, SIMPL::Defaults::AnyComponentSize, AttributeMatrix::Category::Element);
    parameters.push_back(SIMPL_NEW_DA_SELECTION_FP("Element Data to Copy to Feature Data", SelectedCellArrayPath, FilterParameter::Category::RequiredArray, CreateFeatureArrayFromElementArray, req));
  }
  {
    DataArraySelectionFilterParameter::RequirementType req = DataArraySelectionFilterParameter::CreateCategoryRequirement(SIMPL::TypeNames::Int32, 1, AttributeMatrix::Category::Element);
    parameters.push_back(SIMPL_NEW_DA_SELECTION_FP("Feature Ids", FeatureIdsArrayPath, FilterParameter::Category::RequiredArray, CreateFeatureArrayFromElementArray, req));
  }
  parameters.push_back(SeparatorFilterParameter::Create("Feature Data", FilterParameter::Category::CreatedArray));
  {
    AttributeMatrixSelectionFilterParameter::RequirementType req = AttributeMatrixSelectionFilterParameter::CreateRequirement(AttributeMatrix::Type::CellFeature, IGeometry::Type::Any);
    parameters.push_back(SIMPL_NEW_AM_SELECTION_FP("Feature Attribute Matrix", CellFeatureAttributeMatrixName, FilterParameter::Category::CreatedArray, CreateFeatureArrayFromElementArray, req));
  }
  parameters.push_back(SIMPL_NEW_DA_WITH_LINKED_AM_FP("Copied Attribute Array", CreatedArrayName, CellFeatureAttributeMatrixName, CellFeatureAttributeMatrixName,
                                                      FilterParameter::Category::CreatedArray, CreateFeatureArrayFromElementArray));
  setFilterParameters(parameters);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void CreateFeatureArrayFromElementArray::readFilterParameters(AbstractFilterParametersReader* reader, int index)
{
  reader->openFilterGroup(this, index);
  setCellFeatureAttributeMatrixName(reader->readDataArrayPath("CellFeatureAttributeMatrixName", getCellFeatureAttributeMatrixName()));
  setFeatureIdsArrayPath(reader->readDataArrayPath("FeatureIdsArrayPath", getFeatureIdsArrayPath()));
  setSelectedCellArrayPath(reader->readDataArrayPath("SelectedCellArrayPath", getSelectedCellArrayPath()));
  setCreatedArrayName(reader->readString("CreatedArrayName", getCreatedArrayName()));
  reader->closeFilterGroup();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void CreateFeatureArrayFromElementArray::initialize()
{
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void CreateFeatureArrayFromElementArray::dataCheck()
{
  clearErrorCode();
  clearWarningCode();

  if(getCreatedArrayName().isEmpty())
  {
    setErrorCondition(-11002, "The new Feature Array name must be set");
    return;
  }

  std::vector<size_t> cDims(1, 1);
  m_FeatureIdsPtr = getDataContainerArray()->getPrereqArrayFromPath<DataArray<int32_t>>(this, getFeatureIdsArrayPath(), cDims);
  if(nullptr != m_FeatureIdsPtr.lock())
  {
    m_FeatureIds = m_FeatureIdsPtr.lock()->getPointer(0);
  } /* Now assign the raw pointer to data from the DataArray<T> object */

  m_InArrayPtr = getDataContainerArray()->getPrereqIDataArrayFromPath(this, getSelectedCellArrayPath());

  getDataContainerArray()->getPrereqAttributeMatrixFromPath(this, getCellFeatureAttributeMatrixName(), -301);

  if(getErrorCode() < 0)
  {
    return;
  }

  DataArrayPath tempPath(getCellFeatureAttributeMatrixName().getDataContainerName(), getCellFeatureAttributeMatrixName().getAttributeMatrixName(), getCreatedArrayName());
  TemplateHelpers::CreateNonPrereqArrayFromArrayType()(this, tempPath, m_InArrayPtr.lock()->getComponentDimensions(), m_InArrayPtr.lock(), FeatureArrayID);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
template <typename T>
IDataArray::Pointer copyCellData(AbstractFilter* filter, IDataArray::Pointer inputData, int32_t features, int32_t* featureIds, const QString& createdArrayName)
{
  QString featureArrayName = inputData->getName();

  typename DataArray<T>::Pointer cell = std::dynamic_pointer_cast<DataArray<T>>(inputData);
  if(nullptr == cell)
  {
    return IDataArray::NullPointer();
  }

  std::vector<size_t> dims = inputData->getComponentDimensions();
  typename DataArray<T>::Pointer feature = DataArray<T>::CreateArray(features, dims, createdArrayName, true);

  T* fPtr = feature->getPointer(0);
  T* cPtr = cell->getPointer(0);

  size_t numComp = static_cast<size_t>(cell->getNumberOfComponents());
  int32_t featureIdx = 0;

  size_t cells = inputData->getNumberOfTuples();

  QMap<int32_t, T*> featureMap;
  bool warningThrown = false;

  for(size_t i = 0; i < cells; ++i)
  {
    // Get the feature id (or what ever the user has selected as their "Feature" identifier
    featureIdx = featureIds[i];

    // Now get the pointer to the start of the tuple for the Cell Array at the given Feature Id Index value
    T* cSourcePtr = cPtr + (numComp * i);

    // Store the first value(s) with this feature id in the map
    if(featureMap.contains(featureIdx) == false)
    {
      featureMap.insert(featureIdx, cSourcePtr);
    }

    // Check that the values that are currently being pointed to by the source pointer match the first values
    T* currentDataPtr = featureMap.value(featureIdx);
    for(int j = 0; j < numComp; j++)
    {
      if(currentDataPtr[j] != cSourcePtr[j] && !warningThrown)
      {
        // The values are inconsistent with the first values for this feature id, so throw a warning
        QString ss = QObject::tr("Elements from Feature %1 do not all have the same value. The last value copied into Feature %1 will be used").arg(featureIdx);
        filter->setWarningCondition(-1000, ss);
        warningThrown = true;
      }
    }

    // Now get the pointer to the start of the tuple for the Feature Array at the proper index
    T* fDestPtr = fPtr + (numComp * featureIdx);

    // Now just raw copy the bytes from the source to the destination
    ::memcpy(fDestPtr, cSourcePtr, sizeof(T) * numComp);
  }
  return feature;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void CreateFeatureArrayFromElementArray::execute()
{
  dataCheck();
  if(getErrorCode() < 0)
  {
    return;
  }

  // Validate that the selected InArray has tuples equal to the largest
  // Feature Id; the filter would not crash otherwise, but the user should
  // be notified of unanticipated behavior ; this cannot be done in the dataCheck since
  // we don't have acces to the data yet
  int32_t totalFeatures = getDataContainerArray()->getAttributeMatrix(m_CellFeatureAttributeMatrixName)->getNumberOfTuples();
  bool mismatchedFeatures = false;
  int32_t largestFeature = 0;
  size_t totalPoints = m_FeatureIdsPtr.lock()->getNumberOfTuples();
  for(size_t i = 0; i < totalPoints; i++)
  {
    if(m_FeatureIds[i] > largestFeature)
    {
      largestFeature = m_FeatureIds[i];
      if(largestFeature >= totalFeatures)
      {
        mismatchedFeatures = true;
        break;
      }
    }
  }

  if(mismatchedFeatures)
  {
    QString ss = QObject::tr("Attribute Matrix %1 has %2 tuples but the input array %3 has a Feature ID value of at least %4")
                     .arg(m_CellFeatureAttributeMatrixName.serialize("/"))
                     .arg(totalFeatures)
                     .arg(getFeatureIdsArrayPath().serialize("/"))
                     .arg(largestFeature);
    setErrorCondition(-5555, ss);
    return;
  }

  if(largestFeature != (totalFeatures - 1))
  {
    QString ss = QObject::tr("The number of Features in the InArray array (%1) does not match the largest Feature Id in the FeatureIds array").arg(totalFeatures);
    setErrorCondition(-5556, ss);
    return;
  }

  IDataArray::Pointer p = IDataArray::NullPointer();

  if(TemplateHelpers::CanDynamicCast<Int8ArrayType>()(m_InArrayPtr.lock()))
  {
    p = copyCellData<int8_t>(this, m_InArrayPtr.lock(), totalFeatures, m_FeatureIds, getCreatedArrayName());
  }
  else if(TemplateHelpers::CanDynamicCast<UInt8ArrayType>()(m_InArrayPtr.lock()))
  {
    p = copyCellData<uint8_t>(this, m_InArrayPtr.lock(), totalFeatures, m_FeatureIds, getCreatedArrayName());
  }
  else if(TemplateHelpers::CanDynamicCast<Int16ArrayType>()(m_InArrayPtr.lock()))
  {
    p = copyCellData<int16_t>(this, m_InArrayPtr.lock(), totalFeatures, m_FeatureIds, getCreatedArrayName());
  }
  else if(TemplateHelpers::CanDynamicCast<UInt16ArrayType>()(m_InArrayPtr.lock()))
  {
    p = copyCellData<uint16_t>(this, m_InArrayPtr.lock(), totalFeatures, m_FeatureIds, getCreatedArrayName());
  }
  else if(TemplateHelpers::CanDynamicCast<Int32ArrayType>()(m_InArrayPtr.lock()))
  {
    p = copyCellData<int32_t>(this, m_InArrayPtr.lock(), totalFeatures, m_FeatureIds, getCreatedArrayName());
  }
  else if(TemplateHelpers::CanDynamicCast<UInt32ArrayType>()(m_InArrayPtr.lock()))
  {
    p = copyCellData<uint32_t>(this, m_InArrayPtr.lock(), totalFeatures, m_FeatureIds, getCreatedArrayName());
  }
  else if(TemplateHelpers::CanDynamicCast<Int64ArrayType>()(m_InArrayPtr.lock()))
  {
    p = copyCellData<int64_t>(this, m_InArrayPtr.lock(), totalFeatures, m_FeatureIds, getCreatedArrayName());
  }
  else if(TemplateHelpers::CanDynamicCast<UInt64ArrayType>()(m_InArrayPtr.lock()))
  {
    p = copyCellData<uint64_t>(this, m_InArrayPtr.lock(), totalFeatures, m_FeatureIds, getCreatedArrayName());
  }
  else if(TemplateHelpers::CanDynamicCast<FloatArrayType>()(m_InArrayPtr.lock()))
  {
    p = copyCellData<float>(this, m_InArrayPtr.lock(), totalFeatures, m_FeatureIds, getCreatedArrayName());
  }
  else if(TemplateHelpers::CanDynamicCast<DoubleArrayType>()(m_InArrayPtr.lock()))
  {
    p = copyCellData<double>(this, m_InArrayPtr.lock(), totalFeatures, m_FeatureIds, getCreatedArrayName());
  }
  else if(TemplateHelpers::CanDynamicCast<BoolArrayType>()(m_InArrayPtr.lock()))
  {
    p = copyCellData<bool>(this, m_InArrayPtr.lock(), totalFeatures, m_FeatureIds, getCreatedArrayName());
  }
  else
  {
    QString ss = QObject::tr("The selected array was of unsupported type. The path is %1").arg(m_SelectedCellArrayPath.serialize());
    setErrorCondition(-14000, ss);
  }

  if(p.get() != nullptr)
  {
    getDataContainerArray()->getAttributeMatrix(m_CellFeatureAttributeMatrixName)->insertOrAssign(p);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
AbstractFilter::Pointer CreateFeatureArrayFromElementArray::newFilterInstance(bool copyFilterParameters) const
{
  CreateFeatureArrayFromElementArray::Pointer filter = CreateFeatureArrayFromElementArray::New();
  if(copyFilterParameters)
  {
    copyFilterParameterInstanceVariables(filter.get());
  }
  return filter;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString CreateFeatureArrayFromElementArray::getCompiledLibraryName() const
{
  return Core::CoreBaseName;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString CreateFeatureArrayFromElementArray::getBrandingString() const
{
  return "SIMPLib Core Filter";
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString CreateFeatureArrayFromElementArray::getFilterVersion() const
{
  QString version;
  QTextStream vStream(&version);
  vStream << SIMPLib::Version::Major() << "." << SIMPLib::Version::Minor() << "." << SIMPLib::Version::Patch();
  return version;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString CreateFeatureArrayFromElementArray::getGroupName() const
{
  return SIMPL::FilterGroups::CoreFilters;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QUuid CreateFeatureArrayFromElementArray::getUuid() const
{
  return QUuid("{94438019-21bb-5b61-a7c3-66974b9a34dc}");
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString CreateFeatureArrayFromElementArray::getSubGroupName() const
{
  return SIMPL::FilterSubGroups::MemoryManagementFilters;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString CreateFeatureArrayFromElementArray::getHumanLabel() const
{
  return "Create Feature Array from Element Array";
}

// -----------------------------------------------------------------------------
CreateFeatureArrayFromElementArray::Pointer CreateFeatureArrayFromElementArray::NullPointer()
{
  return Pointer(static_cast<Self*>(nullptr));
}

// -----------------------------------------------------------------------------
std::shared_ptr<CreateFeatureArrayFromElementArray> CreateFeatureArrayFromElementArray::New()
{
  struct make_shared_enabler : public CreateFeatureArrayFromElementArray
  {
  };
  std::shared_ptr<make_shared_enabler> val = std::make_shared<make_shared_enabler>();
  val->setupFilterParameters();
  return val;
}

// -----------------------------------------------------------------------------
QString CreateFeatureArrayFromElementArray::getNameOfClass() const
{
  return QString("CreateFeatureArrayFromElementArray");
}

// -----------------------------------------------------------------------------
QString CreateFeatureArrayFromElementArray::ClassName()
{
  return QString("CreateFeatureArrayFromElementArray");
}

// -----------------------------------------------------------------------------
void CreateFeatureArrayFromElementArray::setCellFeatureAttributeMatrixName(const DataArrayPath& value)
{
  m_CellFeatureAttributeMatrixName = value;
}

// -----------------------------------------------------------------------------
DataArrayPath CreateFeatureArrayFromElementArray::getCellFeatureAttributeMatrixName() const
{
  return m_CellFeatureAttributeMatrixName;
}

// -----------------------------------------------------------------------------
void CreateFeatureArrayFromElementArray::setSelectedCellArrayPath(const DataArrayPath& value)
{
  m_SelectedCellArrayPath = value;
}

// -----------------------------------------------------------------------------
DataArrayPath CreateFeatureArrayFromElementArray::getSelectedCellArrayPath() const
{
  return m_SelectedCellArrayPath;
}

// -----------------------------------------------------------------------------
void CreateFeatureArrayFromElementArray::setCreatedArrayName(const QString& value)
{
  m_CreatedArrayName = value;
}

// -----------------------------------------------------------------------------
QString CreateFeatureArrayFromElementArray::getCreatedArrayName() const
{
  return m_CreatedArrayName;
}

// -----------------------------------------------------------------------------
void CreateFeatureArrayFromElementArray::setFeatureIdsArrayPath(const DataArrayPath& value)
{
  m_FeatureIdsArrayPath = value;
}

// -----------------------------------------------------------------------------
DataArrayPath CreateFeatureArrayFromElementArray::getFeatureIdsArrayPath() const
{
  return m_FeatureIdsArrayPath;
}
