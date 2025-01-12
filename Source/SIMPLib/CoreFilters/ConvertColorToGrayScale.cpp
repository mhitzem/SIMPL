/* ============================================================================
 * Copyright (c) 2009-2016 BlueQuartz Software, LLC
 *
 * Redistribution and use in source and binary forms, with or without
 * modification,
 * are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice, this
 * list of conditions and the following disclaimer in the documentation and/or
 * other materials provided with the distribution.
 *
 * Neither the name of BlueQuartz Software, the US Air Force, nor the names of
 * its
 * contributors may be used to endorse or promote products derived from this
 * software
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
#include "ConvertColorToGrayScale.h"

#include <algorithm>

#include <QtCore/QTextStream>

#include "SIMPLib/SIMPLibVersion.h"
#include "SIMPLib/Common/Constants.h"
#include "SIMPLib/DataContainers/DataContainer.h"
#include "SIMPLib/DataContainers/DataContainerArray.h"
#include "SIMPLib/FilterParameters/AbstractFilterParametersReader.h"
#include "SIMPLib/FilterParameters/DataArraySelectionFilterParameter.h"
#include "SIMPLib/FilterParameters/FloatVec3FilterParameter.h"
#include "SIMPLib/FilterParameters/IntFilterParameter.h"
#include "SIMPLib/FilterParameters/LinkedBooleanFilterParameter.h"
#include "SIMPLib/FilterParameters/LinkedChoicesFilterParameter.h"
#include "SIMPLib/FilterParameters/MultiDataArraySelectionFilterParameter.h"
#include "SIMPLib/FilterParameters/SeparatorFilterParameter.h"
#include "SIMPLib/FilterParameters/StringFilterParameter.h"
#include "SIMPLib/Utilities/ParallelDataAlgorithm.h"

class LuminosityImpl
{
private:
  uint8_t* m_ImageData;
  uint8_t* m_FlatImageData;
  FloatVec3Type m_ColorWeights;
  size_t m_NumComp;

public:
  LuminosityImpl(uint8_t* data, uint8_t* newdata, FloatVec3Type colorWeights, size_t comp)
  : m_ImageData(data)
  , m_FlatImageData(newdata)
  , m_ColorWeights(colorWeights)
  , m_NumComp(comp)
  {
  }
  LuminosityImpl(const LuminosityImpl&) = default;           // Copy Constructor Not Implemented
  LuminosityImpl(LuminosityImpl&&) = default;                // Move Constructor Not Implemented
  LuminosityImpl& operator=(const LuminosityImpl&) = delete; // Copy Assignment Not Implemented
  LuminosityImpl& operator=(LuminosityImpl&&) = delete;      // Move Assignment Not Implemented
  ~LuminosityImpl() = default;

  void convert(size_t start, size_t end) const
  {
    for(size_t i = start; i < end; i++)
    {
      m_FlatImageData[i] = static_cast<uint8_t>(
          roundf((m_ImageData[m_NumComp * i] * m_ColorWeights.getX()) + (m_ImageData[m_NumComp * i + 1] * m_ColorWeights.getY()) + (m_ImageData[m_NumComp * i + 2] * m_ColorWeights.getZ())));
    }
  }

  void operator()(const SIMPLRange& range) const
  {
    convert(range.min(), range.max());
  }
};

class LightnessImpl
{
private:
  uint8_t* m_ImageData;
  uint8_t* m_FlatImageData;
  size_t m_NumComp;

public:
  LightnessImpl(uint8_t* data, uint8_t* newdata, size_t comp)
  : m_ImageData(data)
  , m_FlatImageData(newdata)
  , m_NumComp(comp)
  {
  }
  LightnessImpl(const LightnessImpl&) = default;           // Copy Constructor Not Implemented
  LightnessImpl(LightnessImpl&&) = default;                // Move Constructor Not Implemented
  LightnessImpl& operator=(const LightnessImpl&) = delete; // Copy Assignment Not Implemented
  LightnessImpl& operator=(LightnessImpl&&) = delete;      // Move Assignment Not Implemented
  ~LightnessImpl() = default;

  void convert(size_t start, size_t end) const
  {
    for(size_t i = start; i < end; i++)
    {
      std::pair<uint8_t*, uint8_t*> minmax{std::minmax_element(m_ImageData + (i * m_NumComp), m_ImageData + (i * m_NumComp + 3))};
      m_FlatImageData[i] = static_cast<uint8_t>(roundf((minmax.first[0] + minmax.second[0]) / 2.0f));
    }
  }

  void operator()(const SIMPLRange& range) const
  {
    convert(range.min(), range.max());
  }
};

class SingleChannelImpl
{
private:
  uint8_t* m_ImageData;
  uint8_t* m_FlatImageData;
  size_t numComp;
  int32_t m_Channel;

public:
  SingleChannelImpl(uint8_t* data, uint8_t* newdata, size_t comp, int32_t channel)
  : m_ImageData(data)
  , m_FlatImageData(newdata)
  , numComp(comp)
  , m_Channel(channel)
  {
  }
  SingleChannelImpl(const SingleChannelImpl&) = default;           // Copy Constructor Not Implemented
  SingleChannelImpl(SingleChannelImpl&&) = default;                // Move Constructor Not Implemented
  SingleChannelImpl& operator=(const SingleChannelImpl&) = delete; // Copy Assignment Not Implemented
  SingleChannelImpl& operator=(SingleChannelImpl&&) = delete;      // Move Assignment Not Implemented
  ~SingleChannelImpl() = default;

  void convert(size_t start, size_t end) const
  {
    for(size_t i = start; i < end; i++)
    {
      m_FlatImageData[i] = static_cast<uint8_t>((m_ImageData[numComp * i + static_cast<size_t>(m_Channel)]));
    }
  }

  void operator()(const SIMPLRange& range) const
  {
    convert(range.min(), range.max());
  }
};

class ParallelWrapper
{
public:
  ~ParallelWrapper() = default;
  ParallelWrapper(const ParallelWrapper&) = delete;            // Copy Constructor Not Implemented
  ParallelWrapper(ParallelWrapper&&) = delete;                 // Move Constructor Not Implemented
  ParallelWrapper& operator=(const ParallelWrapper&) = delete; // Copy Assignment Not Implemented
  ParallelWrapper& operator=(ParallelWrapper&&) = delete;      // Move Assignment Not Implemented

  template <typename T>
  static void Run(T impl, size_t totalPoints)
  {
    ParallelDataAlgorithm dataAlg;
    dataAlg.setRange(0, totalPoints);
    dataAlg.execute(impl);
  }

protected:
  ParallelWrapper() = default;
};

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
ConvertColorToGrayScale::ConvertColorToGrayScale() = default;

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
ConvertColorToGrayScale::~ConvertColorToGrayScale() = default;

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ConvertColorToGrayScale::setupFilterParameters()
{

  FilterParameterVectorType parameters;
  {
    std::vector<QString> choices;
    choices.push_back("Luminosity");
    choices.push_back("Average");
    choices.push_back("Lightness");
    choices.push_back("SingleChannel");

    std::vector<QString> linkedProps;
    linkedProps.push_back("ColorWeights");
    linkedProps.push_back("ColorChannel");

    LinkedChoicesFilterParameter::Pointer parameter = LinkedChoicesFilterParameter::New();
    parameter->setHumanLabel("Conversion Algorithm");
    parameter->setPropertyName("ConversionAlgorithm");
    parameter->setSetterCallback(SIMPL_BIND_SETTER(ConvertColorToGrayScale, this, ConversionAlgorithm));
    parameter->setGetterCallback(SIMPL_BIND_GETTER(ConvertColorToGrayScale, this, ConversionAlgorithm));
    parameter->setChoices(choices);
    parameter->setLinkedProperties(linkedProps);
    parameter->setCategory(FilterParameter::Category::Parameter);
    parameters.push_back(parameter);
  }

  parameters.push_back(SIMPL_NEW_FLOAT_VEC3_FP("Color Weighting", ColorWeights, FilterParameter::Category::Parameter, ConvertColorToGrayScale, 0));

  parameters.push_back(SIMPL_NEW_INTEGER_FP("Color Channel", ColorChannel, FilterParameter::Category::Parameter, ConvertColorToGrayScale, 3));

  MultiDataArraySelectionFilterParameter::RequirementType req;
  req.dcGeometryTypes = IGeometry::Types(1, IGeometry::Type::Image);
  req.amTypes = AttributeMatrix::Types(1, AttributeMatrix::Type::Cell);
  parameters.push_back(SIMPL_NEW_MDA_SELECTION_FP("Input Attribute Arrays", InputDataArrayVector, FilterParameter::Category::RequiredArray, ConvertColorToGrayScale, req));

  std::vector<QString> linkedProps = {"OutputAttributeMatrixName"};
  parameters.push_back(SIMPL_NEW_LINKED_BOOL_FP("Create Attribute Matrix", CreateNewAttributeMatrix, FilterParameter::Category::Parameter, ConvertColorToGrayScale, linkedProps));
  parameters.push_back(SIMPL_NEW_STRING_FP("Output Cell Attribute Matrix", OutputAttributeMatrixName, FilterParameter::Category::CreatedArray, ConvertColorToGrayScale));

  parameters.push_back(SIMPL_NEW_STRING_FP("Output Array Prefix", OutputArrayPrefix, FilterParameter::Category::CreatedArray, ConvertColorToGrayScale));
  setFilterParameters(parameters);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ConvertColorToGrayScale::initialize()
{
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ConvertColorToGrayScale::dataCheck()
{
  clearErrorCode();
  clearWarningCode();
  if(!DataArrayPath::ValidateVector(getInputDataArrayVector()))
  {
    QString ss = QObject::tr("All Attribute Arrays must belong to the same Data Container and Attribute Matrix");
    setErrorCondition(-62100, ss);
  }

  if(getOutputArrayPrefix().isEmpty())
  {
    QString message = QObject::tr("Using a prefix (even a single alphanumeric value) is required so that the output Xdmf files can be written correctly");
    setErrorCondition(-62102, message);
  }

  if(getInputDataArrayVector().empty())
  {
    QString message = QObject::tr("At least one Attribute Array must be selected");
    setErrorCondition(-62103, message);
    return;
  }

  DataArrayPath inputAMPath = DataArrayPath::GetAttributeMatrixPath(getInputDataArrayVector());

  AttributeMatrix::Pointer inAM = getDataContainerArray()->getPrereqAttributeMatrixFromPath(this, inputAMPath, -301);
  if(getErrorCode() < 0 || nullptr == inAM.get())
  {
    return;
  }

  DataContainerArray::Pointer dca = getDataContainerArray();
  DataContainer::Pointer dc = dca->getDataContainer(inputAMPath.getDataContainerName());

  // Now create our output attributeMatrix which will contain all of our segmented images
  AttributeMatrix::Pointer outAM;
  if(m_CreateNewAttributeMatrix)
  {
    std::vector<size_t> tDims = inAM->getTupleDimensions();

    outAM = dc->createNonPrereqAttributeMatrix(this, getOutputAttributeMatrixName(), tDims, AttributeMatrix::Type::Cell);
    if(getErrorCode() < 0 || nullptr == outAM.get())
    {
      return;
    }
  }

  // Get the list of checked array names from the input m_Data arrays list
  std::vector<QString> arrayNames = DataArrayPath::GetDataArrayNames(getInputDataArrayVector());
  m_OutputArrayPaths.clear();
  m_OutputArrayPaths.resize(arrayNames.size());
  for(int32_t i = 0; i < arrayNames.size(); i++)
  {
    const QString& daName = arrayNames.at(i);
    QString newName = getOutputArrayPrefix() + arrayNames.at(i);
    inputAMPath.setDataArrayName(daName);

    if(getErrorCode() < 0)
    {
      return;
    }
    std::vector<size_t> outCDims(1, 1);
    if(outAM.get() == nullptr)
    {
      outAM = dc->getPrereqAttributeMatrix(this, inputAMPath.getAttributeMatrixName(), -62105);
    }
    outAM->createAndAddAttributeArray<UInt8ArrayType>(this, newName, 0, outCDims);
    DataArrayPath newPath(dc->getName(), outAM->getName(), newName);
    m_OutputArrayPaths[i] = newPath;
  }

  if(static_cast<ConversionType>(m_ConversionAlgorithm) == ConversionType::SingleChannel)
  {
    if(m_ColorChannel < 0 || m_ColorChannel > 2)
    {
      QString message = QObject::tr("The color channel should be 0, 1 or 2");
      setErrorCondition(-62104, message);
      return;
    }
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ConvertColorToGrayScale::execute()
{
  initialize();
  clearErrorCode();
  clearWarningCode();
  QString ss;
  dataCheck();
  if(getErrorCode() < 0)
  {
    ss = QObject::tr("DataCheck did not pass during execute");
    setErrorCondition(-62106, ss);
    return;
  }

  std::vector<DataArrayPath> inputArrayPaths = getInputDataArrayVector();
  qint32 size = inputArrayPaths.size();

  for(qint32 i = 0; i < size; i++)
  {
    DataArrayPath arrayPath = inputArrayPaths[i];

    DataContainer::Pointer m = getDataContainerArray()->getDataContainer(arrayPath.getDataContainerName());
    AttributeMatrix::Pointer attrMat = m->getAttributeMatrix(arrayPath);

    IDataArray::Pointer inputData = attrMat->getAttributeArray(arrayPath.getDataArrayName());
    UInt8ArrayType::Pointer inputColorData = std::dynamic_pointer_cast<UInt8ArrayType>(inputData);

    if(nullptr == inputColorData.get())
    {
      ss = QObject::tr("Input Color Data at ArrayPath '%1' was not available. This array will be skipped.").arg(arrayPath.serialize("/"));
      setErrorCondition(-62107, ss);
      continue;
    }

    DataArrayPath newPath = m_OutputArrayPaths[i];
    UInt8ArrayType::Pointer outputGrayData = getDataContainerArray()->getAttributeMatrix(newPath)->getAttributeArrayAs<UInt8ArrayType>(newPath.getDataArrayName());

    if(nullptr == outputGrayData.get())
    {
      ss = QObject::tr("Output Data at ArrayPath '%1' was not available. This array will be skipped.").arg(newPath.serialize("/"));
      setErrorCondition(-62108, ss);
      continue;
    }

    auto convType = static_cast<ConversionType>(getConversionAlgorithm());

    size_t comp = inputColorData->getNumberOfComponents();
    size_t totalPoints = inputColorData->getNumberOfTuples();

    switch(convType)
    {
    case ConversionType::Luminosity:
      ParallelWrapper::Run<LuminosityImpl>(LuminosityImpl(inputColorData->getPointer(0), outputGrayData->getPointer(0), m_ColorWeights, comp), totalPoints);
      break;
    case ConversionType::Average:
      ParallelWrapper::Run<LuminosityImpl>(LuminosityImpl(inputColorData->getPointer(0), outputGrayData->getPointer(0), {0.3333f, 0.3333f, 0.3333f}, comp), totalPoints);
      break;
    case ConversionType::Lightness:
      ParallelWrapper::Run<LightnessImpl>(LightnessImpl(inputColorData->getPointer(0), outputGrayData->getPointer(0), comp), totalPoints);
      break;
    case ConversionType::SingleChannel:
      ParallelWrapper::Run<SingleChannelImpl>(SingleChannelImpl(inputColorData->getPointer(0), outputGrayData->getPointer(0), comp, getColorChannel()), totalPoints);
      break;
    }
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
AbstractFilter::Pointer ConvertColorToGrayScale::newFilterInstance(bool copyFilterParameters) const
{
  ConvertColorToGrayScale::Pointer filter = ConvertColorToGrayScale::New();
  if(copyFilterParameters)
  {
    copyFilterParameterInstanceVariables(filter.get());
  }
  return std::move(filter);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString ConvertColorToGrayScale::getCompiledLibraryName() const
{
  return Core::CoreBaseName;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString ConvertColorToGrayScale::getBrandingString() const
{
  return "SIMPLib Core Filter";
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString ConvertColorToGrayScale::getFilterVersion() const
{
  QString version;
  QTextStream vStream(&version);
  vStream << SIMPLib::Version::Major() << "." << SIMPLib::Version::Minor() << "." << SIMPLib::Version::Patch();
  return version;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString ConvertColorToGrayScale::getGroupName() const
{
  return SIMPL::FilterGroups::CoreFilters;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QUuid ConvertColorToGrayScale::getUuid() const
{
  return QUuid("{eb5a89c4-4e71-59b1-9719-d10a652d961e}");
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString ConvertColorToGrayScale::getSubGroupName() const
{
  return SIMPL::FilterSubGroups::ImageFilters;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString ConvertColorToGrayScale::getHumanLabel() const
{
  return "Color to GrayScale";
}

// -----------------------------------------------------------------------------
ConvertColorToGrayScale::Pointer ConvertColorToGrayScale::NullPointer()
{
  return Pointer(static_cast<Self*>(nullptr));
}

// -----------------------------------------------------------------------------
std::shared_ptr<ConvertColorToGrayScale> ConvertColorToGrayScale::New()
{
  struct make_shared_enabler : public ConvertColorToGrayScale
  {
  };
  std::shared_ptr<make_shared_enabler> val = std::make_shared<make_shared_enabler>();
  val->setupFilterParameters();
  return val;
}

// -----------------------------------------------------------------------------
QString ConvertColorToGrayScale::getNameOfClass() const
{
  return QString("ConvertColorToGrayScale");
}

// -----------------------------------------------------------------------------
QString ConvertColorToGrayScale::ClassName()
{
  return QString("ConvertColorToGrayScale");
}

// -----------------------------------------------------------------------------
void ConvertColorToGrayScale::setConversionAlgorithm(int value)
{
  m_ConversionAlgorithm = value;
}

// -----------------------------------------------------------------------------
int ConvertColorToGrayScale::getConversionAlgorithm() const
{
  return m_ConversionAlgorithm;
}

// -----------------------------------------------------------------------------
void ConvertColorToGrayScale::setColorWeights(const FloatVec3Type& value)
{
  m_ColorWeights = value;
}

// -----------------------------------------------------------------------------
FloatVec3Type ConvertColorToGrayScale::getColorWeights() const
{
  return m_ColorWeights;
}

// -----------------------------------------------------------------------------
void ConvertColorToGrayScale::setColorChannel(int value)
{
  m_ColorChannel = value;
}

// -----------------------------------------------------------------------------
int ConvertColorToGrayScale::getColorChannel() const
{
  return m_ColorChannel;
}

// -----------------------------------------------------------------------------
void ConvertColorToGrayScale::setInputDataArrayVector(const std::vector<DataArrayPath>& value)
{
  m_InputDataArrayVector = value;
}

// -----------------------------------------------------------------------------
std::vector<DataArrayPath> ConvertColorToGrayScale::getInputDataArrayVector() const
{
  return m_InputDataArrayVector;
}

// -----------------------------------------------------------------------------
void ConvertColorToGrayScale::setCreateNewAttributeMatrix(bool value)
{
  m_CreateNewAttributeMatrix = value;
}

// -----------------------------------------------------------------------------
bool ConvertColorToGrayScale::getCreateNewAttributeMatrix() const
{
  return m_CreateNewAttributeMatrix;
}

// -----------------------------------------------------------------------------
void ConvertColorToGrayScale::setOutputAttributeMatrixName(const QString& value)
{
  m_OutputAttributeMatrixName = value;
}

// -----------------------------------------------------------------------------
QString ConvertColorToGrayScale::getOutputAttributeMatrixName() const
{
  return m_OutputAttributeMatrixName;
}

// -----------------------------------------------------------------------------
void ConvertColorToGrayScale::setOutputArrayPrefix(const QString& value)
{
  m_OutputArrayPrefix = value;
}

// -----------------------------------------------------------------------------
QString ConvertColorToGrayScale::getOutputArrayPrefix() const
{
  return m_OutputArrayPrefix;
}
