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

#include "TransformationStatsData.h"

#include <QtCore/QString>
#include <vector>

#include "H5Support/H5Utilities.h"

#include <QtCore/QDebug>

#include "SIMPLib/HDF5/H5TransformationStatsDataDelegate.h"

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
TransformationStatsData::TransformationStatsData() = default;

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
TransformationStatsData::~TransformationStatsData() = default;

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString TransformationStatsData::getStatsType()
{
  return SIMPL::StringConstants::TransformationStatsData;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
PhaseType::Type TransformationStatsData::getPhaseType()
{
  return PhaseType::Type::Transformation;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
StatsData::Pointer TransformationStatsData::deepCopy()
{
  TransformationStatsData::Pointer ptr = TransformationStatsData::New();
  ptr->setBoundaryArea(getBoundaryArea());
  ptr->setPhaseFraction(getPhaseFraction());
  ptr->setParentPhase(getParentPhase());
  ptr->setName(getName());

  std::array<float, 3> diamInfo = getFeatureDiameterInfo();
  ptr->setFeatureDiameterInfo(diamInfo);

  SD_DEEP_COPY_VECTOR(FeatureSizeDistribution);

  if(nullptr != m_BinNumbers)
  {
    ptr->setBinNumbers(std::dynamic_pointer_cast<FloatArrayType>(getBinNumbers()->deepCopy()));
  }

  if(nullptr != m_ODF)
  {
    ptr->setODF(std::dynamic_pointer_cast<FloatArrayType>(getODF()->deepCopy()));
  }

  if(nullptr != m_MisorientationBins)
  {
    ptr->setMisorientationBins(std::dynamic_pointer_cast<FloatArrayType>(getMisorientationBins()->deepCopy()));
  }

  if(nullptr != m_AxisOrientation)
  {
    ptr->setAxisOrientation(std::dynamic_pointer_cast<FloatArrayType>(getAxisOrientation()->deepCopy()));
  }

  ptr->setBinStepSize(getBinStepSize());

  ptr->setBOverA_DistType(getBOverA_DistType());
  SD_DEEP_COPY_VECTOR(FeatureSize_BOverA)

  ptr->setCOverA_DistType(getCOverA_DistType());
  SD_DEEP_COPY_VECTOR(FeatureSize_COverA)

  ptr->setOmegas_DistType(getOmegas_DistType());
  SD_DEEP_COPY_VECTOR(FeatureSize_Omegas)

  ptr->setNeighbors_DistType(getNeighbors_DistType());
  SD_DEEP_COPY_VECTOR(FeatureSize_Neighbors)

  // Miso Bins
  SD_DEEP_COPY_VECTOR(MDF_Weights)

  // ODF
  SD_DEEP_COPY_VECTOR(ODF_Weights)

  // Axis ODF
  SD_DEEP_COPY_VECTOR(AxisODF_Weights)

  return ptr;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
FloatArrayType::Pointer TransformationStatsData::generateBinNumbers()
{
  float featureDiameterInfo[3];
  getFeatureDiameterInfo(featureDiameterInfo);
  QVector<float> bins;
  float d = featureDiameterInfo[2];
  while(d <= featureDiameterInfo[1])
  {
    //  qDebug() << d << "\n";
    bins.push_back(d);
    d = d + featureDiameterInfo[0];
  }
  if(!bins.empty())
  {
    // Copy this into the DataArray<float>
    m_BinNumbers = FloatArrayType::CreateArray(bins.size(), SIMPL::StringConstants::BinNumber, true);
    ::memcpy(m_BinNumbers->getVoidPointer(0), &(bins.front()), bins.size() * sizeof(float));
  }
  return m_BinNumbers;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int TransformationStatsData::writeHDF5Data(hid_t groupId)
{
  int err = 0;
  H5TransformationStatsDataDelegate::Pointer writer = H5TransformationStatsDataDelegate::New();
  err = writer->writeTransformationStatsData(this, groupId);
  return err;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int TransformationStatsData::readHDF5Data(hid_t groupId)
{
  int err = 0;
  H5TransformationStatsDataDelegate::Pointer reader = H5TransformationStatsDataDelegate::New();
  err = reader->readTransformationStatsData(this, groupId);
  return err;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void TransformationStatsData::writeJson(QJsonObject& json)
{
  // Write the name of the phase
  json.insert(SIMPL::StringConstants::Name, getName());

  json.insert(SIMPL::StringConstants::PhaseType, PhaseType::TransformationStr());

  // Write the boundary fraction
  json.insert(SIMPL::StringConstants::BoundaryArea, getBoundaryArea());
  // Write the Phase Fraction
  json.insert(SIMPL::StringConstants::PhaseFraction, getPhaseFraction());
  // Write the Parent Phase
  json.insert(SIMPL::StringConstants::ParentPhase, getParentPhase());

  // Write the Feature Diameter Info
  float diamInfo[3];
  getFeatureDiameterInfo(diamInfo);
  QJsonArray diamInfoArray = {diamInfo[0], diamInfo[1], diamInfo[2]};
  json.insert(SIMPL::StringConstants::Feature_Diameter_Info, diamInfoArray);
  // Write the Feature Size Distribution
  QJsonObject avgSizeDist;
  for(size_t i = 0; i < m_FeatureSizeDistribution.size(); i++)
  {
    avgSizeDist.insert(m_FeatureSizeDistribution[i]->getName(), m_FeatureSizeDistribution[i]->getValue(0));
  }
  json.insert(SIMPL::StringConstants::Feature_Size_Distribution, avgSizeDist);

  // Write the Bin Numbers
  if(nullptr == getBinNumbers().get())
  {
    generateBinNumbers();
  }
  json.insert(SIMPL::StringConstants::BinNumber, generateJsonArrayFromDataArray<float>(getBinNumbers()));
  json.insert(SIMPL::StringConstants::BinCount, static_cast<double>(getNumberOfBins()));
  //  json.insert(SIMPL::StringConstants::BinStepSize, static_cast<double>(getBinStepSize()));

  // Write the B Over A
  writeJsonDistributionArrays(json, getFeatureSize_BOverA(), SIMPL::StringConstants::Feature_SizeVBoverA_Distributions, getBOverA_DistType());

  // Write the C Over A
  writeJsonDistributionArrays(json, getFeatureSize_COverA(), SIMPL::StringConstants::Feature_SizeVCoverA_Distributions, getCOverA_DistType());

  // Write the Neighbors
  writeJsonDistributionArrays(json, getFeatureSize_Neighbors(), SIMPL::StringConstants::Feature_SizeVNeighbors_Distributions, getNeighbors_DistType());

  // Write the Omegas
  writeJsonDistributionArrays(json, getFeatureSize_Omegas(), SIMPL::StringConstants::Feature_SizeVOmega3_Distributions, getOmegas_DistType());

  // Write the Misorientation Bins
  writeJsonDistributionArrays(json, getMDF_Weights(), SIMPL::StringConstants::MDFWeights, SIMPL::DistributionType::UnknownDistributionType);

  // Write the ODF
  writeJsonDistributionArrays(json, getODF_Weights(), SIMPL::StringConstants::ODFWeights, SIMPL::DistributionType::UnknownDistributionType);

  // Write the Axis ODF
  writeJsonDistributionArrays(json, getAxisODF_Weights(), SIMPL::StringConstants::AxisODFWeights, SIMPL::DistributionType::UnknownDistributionType);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void TransformationStatsData::readJson(const QJsonObject& json)
{
  // Read the boundary area
  QJsonValue jsonValue = json[SIMPL::StringConstants::BoundaryArea];
  if(!jsonValue.isUndefined() && jsonValue.isDouble())
  {
    setBoundaryArea(jsonValue.toDouble(0.0));
  }
  //
  jsonValue = json[SIMPL::StringConstants::Name];
  if(!jsonValue.isUndefined() && jsonValue.isString())
  {
    setName(jsonValue.toString("Transformation"));
  }
  if(getName().compare(SIMPL::StringConstants::StatsData) == 0)
  {
    setName("Transformation");
  }

  // Read the Phase Fraction
  jsonValue = json[SIMPL::StringConstants::PhaseFraction];
  if(!jsonValue.isUndefined() && jsonValue.isDouble())
  {
    setPhaseFraction(jsonValue.toDouble(0.0));
  }

  // Read the precipitate boundary fraction
  jsonValue = json[SIMPL::StringConstants::ParentPhase];
  if(!jsonValue.isUndefined() && jsonValue.isDouble())
  {
    setParentPhase(jsonValue.toDouble(0.0));
  }

  // Read the Feature Diameter Info
  std::array<float, 3> fVec3 = {0.0f, 0.0f, 0.0f};
  if(ParseFloat3Vec(json, SIMPL::StringConstants::Feature_Diameter_Info, fVec3, 0.0) == -1)
  {
    // Throw warning
  }
  setFeatureDiameterInfo(fVec3);

  // Read the Feature Size Distribution
  jsonValue = json[SIMPL::StringConstants::Feature_Size_Distribution];
  if(!jsonValue.isUndefined() && jsonValue.isObject())
  {
    QJsonObject avgSizeDist = jsonValue.toObject();
    QStringList keys = avgSizeDist.keys();
    VectorOfFloatArray arrays;
    for(const QString& key : keys)
    {
      FloatArrayType::Pointer fArray = FloatArrayType::CreateArray(1, key, true);
      fArray->setValue(0, avgSizeDist[key].toDouble());
      arrays.push_back(fArray);
    }
    setFeatureSizeDistribution(arrays);
  }

  // Read the Bin Numbers, Bin Step Size
  jsonValue = json[SIMPL::StringConstants::BinNumber];
  QJsonArray jArray = jsonValue.toArray();
  FloatArrayType::Pointer binNumbers = FloatArrayType::CreateArray(jArray.count(), SIMPL::StringConstants::BinNumber, true);
  for(int i = 0; i < jArray.count(); i++)
  {
    binNumbers->setValue(i, jArray[i].toDouble());
  }
  setBinNumbers(binNumbers);
  //  jsonValue = json[SIMPL::StringConstants::BinStepSize];
  //  if(!jsonValue.isUndefined() && jsonValue.isDouble()) { setBinStepSize(jsonValue.toInt(0)); }

  // Read the B Over A Distribution
  int disType = SIMPL::DistributionType::UnknownDistributionType;
  VectorOfFloatArray arrays = ReadJsonDistributionArrays(json, SIMPL::StringConstants::Feature_SizeVBoverA_Distributions, disType);
  setBOverA_DistType(disType);
  setFeatureSize_BOverA(arrays);

  // Read the C Over A
  disType = SIMPL::DistributionType::UnknownDistributionType;
  arrays = ReadJsonDistributionArrays(json, SIMPL::StringConstants::Feature_SizeVCoverA_Distributions, disType);
  setCOverA_DistType(disType);
  setFeatureSize_COverA(arrays);

  // Read the Omegas
  disType = SIMPL::DistributionType::UnknownDistributionType;
  arrays = ReadJsonDistributionArrays(json, SIMPL::StringConstants::Feature_SizeVOmega3_Distributions, disType);
  setOmegas_DistType(disType);
  setFeatureSize_Omegas(arrays);

  // Read the Neighbors
  disType = SIMPL::DistributionType::UnknownDistributionType;
  arrays = ReadJsonDistributionArrays(json, SIMPL::StringConstants::Feature_SizeVNeighbors_Distributions, disType);
  setNeighbors_DistType(disType);
  setFeatureSize_Neighbors(arrays);

  // Read the Misorientation Bins
  arrays = ReadJsonVectorOfFloatsArrays(json, SIMPL::StringConstants::MDFWeights);
  setMDF_Weights(arrays);

  // Read the ODF
  arrays = ReadJsonVectorOfFloatsArrays(json, SIMPL::StringConstants::ODFWeights);
  setODF_Weights(arrays);

  // Read the Axis ODF
  arrays = ReadJsonVectorOfFloatsArrays(json, SIMPL::StringConstants::AxisODFWeights);
  setAxisODF_Weights(arrays);
}

// -----------------------------------------------------------------------------
TransformationStatsData::Pointer TransformationStatsData::NullPointer()
{
  return Pointer(static_cast<Self*>(nullptr));
}

// -----------------------------------------------------------------------------
TransformationStatsData::Pointer TransformationStatsData::New()
{
  Pointer sharedPtr(new(TransformationStatsData));
  return sharedPtr;
}

// -----------------------------------------------------------------------------
QString TransformationStatsData::getNameOfClass() const
{
  return QString("TransformationStatsData");
}

// -----------------------------------------------------------------------------
QString TransformationStatsData::ClassName()
{
  return QString("TransformationStatsData");
}

// -----------------------------------------------------------------------------
void TransformationStatsData::setBoundaryArea(float value)
{
  m_BoundaryArea = value;
}

// -----------------------------------------------------------------------------
float TransformationStatsData::getBoundaryArea() const
{
  return m_BoundaryArea;
}

// -----------------------------------------------------------------------------
void TransformationStatsData::setParentPhase(float value)
{
  m_ParentPhase = value;
}

// -----------------------------------------------------------------------------
float TransformationStatsData::getParentPhase() const
{
  return m_ParentPhase;
}

// -----------------------------------------------------------------------------
void TransformationStatsData::setFeatureSizeDistribution(const VectorOfFloatArray& value)
{
  m_FeatureSizeDistribution = value;
}

// -----------------------------------------------------------------------------
VectorOfFloatArray TransformationStatsData::getFeatureSizeDistribution() const
{
  return m_FeatureSizeDistribution;
}

// -----------------------------------------------------------------------------
void TransformationStatsData::setFeatureSize_DistType(uint32_t value)
{
  m_FeatureSize_DistType = value;
}

// -----------------------------------------------------------------------------
uint32_t TransformationStatsData::getFeatureSize_DistType() const
{
  return m_FeatureSize_DistType;
}

// -----------------------------------------------------------------------------
void TransformationStatsData::setBinNumbers(const FloatArrayType::Pointer& value)
{
  m_BinNumbers = value;
}

// -----------------------------------------------------------------------------
FloatArrayType::Pointer TransformationStatsData::getBinNumbers() const
{
  return m_BinNumbers;
}

// -----------------------------------------------------------------------------
void TransformationStatsData::setFeatureSize_BOverA(const VectorOfFloatArray& value)
{
  m_FeatureSize_BOverA = value;
}

// -----------------------------------------------------------------------------
VectorOfFloatArray TransformationStatsData::getFeatureSize_BOverA() const
{
  return m_FeatureSize_BOverA;
}

// -----------------------------------------------------------------------------
void TransformationStatsData::setBOverA_DistType(uint32_t value)
{
  m_BOverA_DistType = value;
}

// -----------------------------------------------------------------------------
uint32_t TransformationStatsData::getBOverA_DistType() const
{
  return m_BOverA_DistType;
}

// -----------------------------------------------------------------------------
void TransformationStatsData::setFeatureSize_COverA(const VectorOfFloatArray& value)
{
  m_FeatureSize_COverA = value;
}

// -----------------------------------------------------------------------------
VectorOfFloatArray TransformationStatsData::getFeatureSize_COverA() const
{
  return m_FeatureSize_COverA;
}

// -----------------------------------------------------------------------------
void TransformationStatsData::setCOverA_DistType(uint32_t value)
{
  m_COverA_DistType = value;
}

// -----------------------------------------------------------------------------
uint32_t TransformationStatsData::getCOverA_DistType() const
{
  return m_COverA_DistType;
}

// -----------------------------------------------------------------------------
void TransformationStatsData::setFeatureSize_Neighbors(const VectorOfFloatArray& value)
{
  m_FeatureSize_Neighbors = value;
}

// -----------------------------------------------------------------------------
VectorOfFloatArray TransformationStatsData::getFeatureSize_Neighbors() const
{
  return m_FeatureSize_Neighbors;
}

// -----------------------------------------------------------------------------
void TransformationStatsData::setNeighbors_DistType(uint32_t value)
{
  m_Neighbors_DistType = value;
}

// -----------------------------------------------------------------------------
uint32_t TransformationStatsData::getNeighbors_DistType() const
{
  return m_Neighbors_DistType;
}

// -----------------------------------------------------------------------------
void TransformationStatsData::setFeatureSize_Omegas(const VectorOfFloatArray& value)
{
  m_FeatureSize_Omegas = value;
}

// -----------------------------------------------------------------------------
VectorOfFloatArray TransformationStatsData::getFeatureSize_Omegas() const
{
  return m_FeatureSize_Omegas;
}

// -----------------------------------------------------------------------------
void TransformationStatsData::setOmegas_DistType(uint32_t value)
{
  m_Omegas_DistType = value;
}

// -----------------------------------------------------------------------------
uint32_t TransformationStatsData::getOmegas_DistType() const
{
  return m_Omegas_DistType;
}

// -----------------------------------------------------------------------------
void TransformationStatsData::setMisorientationBins(const FloatArrayType::Pointer& value)
{
  m_MisorientationBins = value;
}

// -----------------------------------------------------------------------------
FloatArrayType::Pointer TransformationStatsData::getMisorientationBins() const
{
  return m_MisorientationBins;
}

// -----------------------------------------------------------------------------
void TransformationStatsData::setMDF_Weights(const VectorOfFloatArray& value)
{
  m_MDF_Weights = value;
}

// -----------------------------------------------------------------------------
VectorOfFloatArray TransformationStatsData::getMDF_Weights() const
{
  return m_MDF_Weights;
}

// -----------------------------------------------------------------------------
void TransformationStatsData::setODF(const FloatArrayType::Pointer& value)
{
  m_ODF = value;
}

// -----------------------------------------------------------------------------
FloatArrayType::Pointer TransformationStatsData::getODF() const
{
  return m_ODF;
}

// -----------------------------------------------------------------------------
void TransformationStatsData::setODF_Weights(const VectorOfFloatArray& value)
{
  m_ODF_Weights = value;
}

// -----------------------------------------------------------------------------
VectorOfFloatArray TransformationStatsData::getODF_Weights() const
{
  return m_ODF_Weights;
}

// -----------------------------------------------------------------------------
void TransformationStatsData::setAxisOrientation(const FloatArrayType::Pointer& value)
{
  m_AxisOrientation = value;
}

// -----------------------------------------------------------------------------
FloatArrayType::Pointer TransformationStatsData::getAxisOrientation() const
{
  return m_AxisOrientation;
}

// -----------------------------------------------------------------------------
void TransformationStatsData::setAxisODF_Weights(const VectorOfFloatArray& value)
{
  m_AxisODF_Weights = value;
}

// -----------------------------------------------------------------------------
VectorOfFloatArray TransformationStatsData::getAxisODF_Weights() const
{
  return m_AxisODF_Weights;
}

// -----------------------------------------------------------------------------
void TransformationStatsData::setFeatureDiameterInfo(const std::array<float, 3>& value)
{
  m_FeatureDiameterInfo = value;
}

// -----------------------------------------------------------------------------
void TransformationStatsData::setFeatureDiameterInfo(float v0, float v1, float v2)
{
  m_FeatureDiameterInfo[0] = v0;
  m_FeatureDiameterInfo[1] = v1;
  m_FeatureDiameterInfo[2] = v2;
}

// -----------------------------------------------------------------------------
std::array<float, 3> TransformationStatsData::getFeatureDiameterInfo() const
{
  return m_FeatureDiameterInfo;
}

// -----------------------------------------------------------------------------
void TransformationStatsData::getFeatureDiameterInfo(float* data) const
{
  data[0] = m_FeatureDiameterInfo[0];
  data[1] = m_FeatureDiameterInfo[1];
  data[2] = m_FeatureDiameterInfo[2];
}
