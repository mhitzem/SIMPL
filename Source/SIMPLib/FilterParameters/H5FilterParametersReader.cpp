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

#include "H5FilterParametersReader.h"

#include <limits>

#include <QtCore/QStringList>

#include "H5Support/H5ScopedSentinel.h"
#include "H5Support/QH5Lite.h"
#include "H5Support/QH5Utilities.h"

#include "SIMPLib/Common/Constants.h"
#include "SIMPLib/Common/QtBackwardCompatibilityMacro.h"
#include "SIMPLib/FilterParameters/H5FilterParametersConstants.h"
#include "SIMPLib/FilterParameters/JsonFilterParametersReader.h"
#include "SIMPLib/FilterParameters/JsonFilterParametersWriter.h"
#include "SIMPLib/Filtering/FilterFactory.hpp"
#include "SIMPLib/Filtering/FilterManager.h"
#include "SIMPLib/Messages/PipelineErrorMessage.h"

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
H5FilterParametersReader::H5FilterParametersReader()
: m_PipelineGroupId(-1)
{
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
H5FilterParametersReader::~H5FilterParametersReader() = default;

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
hid_t H5FilterParametersReader::getCurrentGroupId() const
{
  return m_CurrentGroupId;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
H5FilterParametersReader::Pointer H5FilterParametersReader::OpenDREAM3DFileForReadingPipeline(QString filePath, hid_t& fid)
{
  fid = -1;
  fid = QH5Utilities::openFile(filePath);
  if(fid < 0)
  {
    return H5FilterParametersReader::NullPointer();
  }

  H5FilterParametersReader::Pointer reader = H5FilterParametersReader::New();
  hid_t pipelineGroupId = H5Gopen(fid, SIMPL::StringConstants::PipelineGroupName.toLatin1().data(), H5P_DEFAULT);
  if(pipelineGroupId < 0)
  {
    H5Fclose(fid);
    fid = -1;
    return H5FilterParametersReader::NullPointer();
  }
  reader->setPipelineGroupId(pipelineGroupId);
  return reader;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
FilterPipeline::Pointer H5FilterParametersReader::readPipelineFromFile(hid_t fid, IObserver* obs)
{

  // Open the Pipeline Group
  hid_t pipelineGroupId = H5Gopen(fid, SIMPL::StringConstants::PipelineGroupName.toLatin1().data(), H5P_DEFAULT);
  if(pipelineGroupId < 0)
  {
    H5Fclose(fid);
    fid = -1;
    return FilterPipeline::NullPointer();
  }

  H5ScopedGroupSentinel sentinel(pipelineGroupId, true);

  setPipelineGroupId(pipelineGroupId);

  m_Version = -1;

  // Create a FilterPipeline Object
  FilterPipeline::Pointer pipeline = FilterPipeline::New();

  if(QH5Lite::findAttribute(pipelineGroupId, SIMPL::StringConstants::PipelineVersionName) > 0)
  {
    herr_t err = QH5Lite::readScalarAttribute(pipelineGroupId, "/" + SIMPL::StringConstants::PipelineGroupName, SIMPL::StringConstants::PipelineVersionName, m_Version);
    if(err < 0)
    {
      return FilterPipeline::NullPointer();
    }

    if(m_Version == 2)
    {
      QString jsonString = "";
      herr_t err = QH5Lite::readStringDataset(pipelineGroupId, SIMPL::StringConstants::PipelineGroupName, jsonString);
      if(err < 0)
      {
        return FilterPipeline::NullPointer();
      }

      JsonFilterParametersReader::Pointer jsonReader = JsonFilterParametersReader::New();
      pipeline = jsonReader->readPipelineFromString(jsonString, nullptr);
    }
    else if(nullptr != obs)
    {
      QString ss = QObject::tr("The input file contains an unrecognizable pipeline version number, and is therefore incompatible and cannot be read.");
      PipelineErrorMessage::Pointer pm = PipelineErrorMessage::New("[NOT_READABLE]", QObject::tr("%1: %2").arg("H5FilterParametersReader::ReadPipelineFromFile(...)").arg(ss), -66066);
      obs->processPipelineMessage(pm);
      return FilterPipeline::NullPointer();
    }
  }
  else
  {
    // Use QH5Lite to ask how many "groups" are in the "Pipeline Group"
    QList<QString> groupList;
    herr_t err = QH5Utilities::getGroupObjects(pipelineGroupId, H5Utilities::CustomHDFDataTypes::Group, groupList);

    // Get a FilterManager Instance
    FilterManager* filterManager = FilterManager::Instance();

    // Loop over the items getting the "ClassName" attribute from each group
    QString classNameStr = "";
    for(int i = 0; i < groupList.size(); i++)
    {

      QString iStr = QString::number(i);
      err = QH5Lite::readStringAttribute(pipelineGroupId, iStr, "ClassName", classNameStr);

      // Instantiate a new filter using the FilterFactory based on the value of the className attribute
      IFilterFactory::Pointer ff = filterManager->getFactoryFromClassName(classNameStr);
      if(nullptr != ff)
      {
        AbstractFilter::Pointer filter = ff->create();
        if(nullptr != ff)
        {
          // Read the parameters
          filter->readFilterParameters(this, i);

          // Add filter to pipeline
          pipeline->pushBack(filter);
        }
      }
    }
  }
  return pipeline;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
FilterPipeline::Pointer H5FilterParametersReader::readPipelineFromFile(QString filePath, IObserver* obs)
{
  hid_t fid = -1;
  fid = QH5Utilities::openFile(filePath);
  if(fid < 0)
  {
    return FilterPipeline::NullPointer();
  }

  H5ScopedFileSentinel sentinel(fid, true);

  FilterPipeline::Pointer pipeline = readPipelineFromFile(fid, obs);
  return pipeline;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString H5FilterParametersReader::getJsonFromFile(QString filePath, IObserver* obs)
{
  hid_t fid = -1;
  fid = QH5Utilities::openFile(filePath);
  if(fid < 0)
  {
    return QString();
  }

  H5ScopedFileSentinel sentinel(fid, true);

  // Open the Pipeline Group
  hid_t pipelineGroupId = H5Gopen(fid, SIMPL::StringConstants::PipelineGroupName.toLatin1().data(), H5P_DEFAULT);
  if(pipelineGroupId < 0)
  {
    H5Fclose(fid);
    fid = -1;
    return QString();
  }

  sentinel.addGroupId(pipelineGroupId);

  setPipelineGroupId(pipelineGroupId);

  m_Version = -1;

  QString jsonString = "";
  if(QH5Lite::findAttribute(pipelineGroupId, SIMPL::StringConstants::PipelineVersionName) > 0)
  {
    herr_t err = QH5Lite::readScalarAttribute(pipelineGroupId, "/" + SIMPL::StringConstants::PipelineGroupName, SIMPL::StringConstants::PipelineVersionName, m_Version);
    if(err < 0)
    {
      return QString();
    }

    if(m_Version == 2)
    {
      herr_t err = QH5Lite::readStringDataset(pipelineGroupId, SIMPL::StringConstants::PipelineGroupName, jsonString);
      if(err < 0)
      {
        return QString();
      }
    }
    else if(nullptr != obs)
    {
      QString ss = QObject::tr("The input file contains an unrecognizable pipeline version number, and is therefore incompatible and cannot be read.");
      PipelineErrorMessage::Pointer pm = PipelineErrorMessage::New("[NOT_READABLE]", QObject::tr("%1: %2").arg("H5FilterParametersReader::ReadPipelineFromFile(...)").arg(ss), -66066);
      obs->processPipelineMessage(pm);
      return QString();
    }
  }
  else
  {
    // Use QH5Lite to ask how many "groups" are in the "Pipeline Group"
    QList<QString> groupList;
    herr_t err = QH5Utilities::getGroupObjects(pipelineGroupId, H5Utilities::CustomHDFDataTypes::Group, groupList);

    // Get a FilterManager Instance
    FilterManager* filterManager = FilterManager::Instance();

    // Loop over the items getting the "ClassName" attribute from each group
    QString classNameStr = "";
    FilterPipeline::Pointer pipeline = FilterPipeline::New();
    for(int i = 0; i < groupList.size(); i++)
    {

      QString iStr = QString::number(i);
      err = QH5Lite::readStringAttribute(pipelineGroupId, iStr, "ClassName", classNameStr);

      // Instantiate a new filter using the FilterFactory based on the value of the className attribute
      IFilterFactory::Pointer ff = filterManager->getFactoryFromClassName(classNameStr);
      if(nullptr != ff)
      {
        AbstractFilter::Pointer filter = ff->create();
        if(nullptr != ff)
        {
          // Read the parameters
          filter->readFilterParameters(this, i);

          // Add filter to pipeline
          pipeline->pushBack(filter);
        }
      }
    }

    JsonFilterParametersWriter::Pointer jsonWriter = JsonFilterParametersWriter::New();
    jsonString = jsonWriter->writePipelineToString(pipeline, "", false);
  }

  return jsonString;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int H5FilterParametersReader::openFilterGroup(AbstractFilter* filter, int index)
{
  int err = 0;
  std::ignore = filter;
  if(m_Version < 0)
  {
    if(m_PipelineGroupId <= 0)
    {
      return -1;
    }

    QString name = QString::number(index);
    m_CurrentGroupId = H5Gopen(m_PipelineGroupId, name.toLatin1().data(), H5P_DEFAULT);
  }
  else if(m_Version == 2)
  {
    QString numStr = QString::number(index);
    m_CurrentFilterObject = m_PipelineRoot[numStr].toObject();
    if(m_CurrentFilterObject.isEmpty())
    {
      m_CurrentFilterObject = m_PipelineRoot[numStr].toObject();
      if(m_CurrentFilterObject.isEmpty())
      {
        err = -1;
      }
    }
  }

  return err;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int H5FilterParametersReader::closeFilterGroup()
{
  if(m_Version < 0)
  {
    H5Gclose(m_CurrentGroupId);
    m_CurrentGroupId = -1;
  }
  else if(m_Version == 2)
  {
    Q_ASSERT(m_PipelineRoot.isEmpty() == false);
    m_CurrentFilterObject = QJsonObject();
    m_PipelineRoot = QJsonObject();
    m_Version = -1;
    m_CurrentIndex = -1;
  }

  return 0;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString H5FilterParametersReader::readString(const QString& name, QString value)
{
  QString defaultStr = value;
  value.clear();
  int err = 0;
  err = QH5Lite::readStringDataset(m_CurrentGroupId, name, value);
  if(err == 0)
  {
    return value;
  }

  return defaultStr;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QVector<QString> H5FilterParametersReader::readStrings(const QString& name, QVector<QString> value)
{
  int vectorSize = 0;
  QString str = "";
  int err = QH5Lite::readScalarDataset(m_CurrentGroupId, name, vectorSize);
  if(err < 0)
  {
  }
  for(int i = 0; i < vectorSize; i++)
  {
    QString ss = QString::number(i, 10);
    err = QH5Lite::readStringAttribute(m_CurrentGroupId, name, ss, str);
    value.push_back(str);
  }

  return value;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QStringList H5FilterParametersReader::readStringList(const QString& name, QStringList value)
{
  int vectorSize = 0;
  QString str = "";
  int err = QH5Lite::readScalarDataset(m_CurrentGroupId, name, vectorSize);
  if(err < 0)
  {
  }
  for(int i = 0; i < vectorSize; i++)
  {
    QString ss = QString::number(i, 10);
    err = QH5Lite::readStringAttribute(m_CurrentGroupId, name, ss, str);
    value.push_back(str);
  }

  return value;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int8_t H5FilterParametersReader::readValue(const QString& name, int8_t value)
{
  QH5Lite::readScalarDataset(m_CurrentGroupId, name, value);
  return value;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int16_t H5FilterParametersReader::readValue(const QString& name, int16_t value)
{
  QH5Lite::readScalarDataset(m_CurrentGroupId, name, value);
  return value;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int32_t H5FilterParametersReader::readValue(const QString& name, int32_t value)
{
  QH5Lite::readScalarDataset(m_CurrentGroupId, name, value);
  return value;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int64_t H5FilterParametersReader::readValue(const QString& name, int64_t value)
{
  QH5Lite::readScalarDataset(m_CurrentGroupId, name, value);
  return value;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
uint8_t H5FilterParametersReader::readValue(const QString& name, uint8_t value)
{
  QH5Lite::readScalarDataset(m_CurrentGroupId, name, value);
  return value;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
uint16_t H5FilterParametersReader::readValue(const QString& name, uint16_t value)
{
  QH5Lite::readScalarDataset(m_CurrentGroupId, name, value);
  return value;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
uint32_t H5FilterParametersReader::readValue(const QString& name, uint32_t value)
{
  QH5Lite::readScalarDataset(m_CurrentGroupId, name, value);
  return value;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
uint64_t H5FilterParametersReader::readValue(const QString& name, uint64_t value)
{
  QH5Lite::readScalarDataset(m_CurrentGroupId, name, value);
  return value;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
float H5FilterParametersReader::readValue(const QString& name, float value)
{
  QH5Lite::readScalarDataset(m_CurrentGroupId, name, value);
  return value;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
double H5FilterParametersReader::readValue(const QString& name, double value)
{
  QH5Lite::readScalarDataset(m_CurrentGroupId, name, value);
  return value;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
bool H5FilterParametersReader::readValue(const QString& name, bool value)
{
  QH5Lite::readScalarDataset(m_CurrentGroupId, name, value);
  return value;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
AttributeMatrix::Type H5FilterParametersReader::readValue(const QString& name, AttributeMatrix::Type value)
{
  uint32_t v = static_cast<uint32_t>(value);
  QH5Lite::readScalarDataset(m_CurrentGroupId, name, v);
  value = static_cast<AttributeMatrix::Type>(v);
  return value;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QVector<int8_t> H5FilterParametersReader::readArray(const QString& name, QVector<int8_t> value)
{
  int vectorSize = 0;
  int8_t scalar = 0;
  int err = QH5Lite::readScalarDataset(m_CurrentGroupId, name, vectorSize);
  if(err < 0)
  {
    return value;
  }

  value.clear();

  for(int i = 0; i < vectorSize; i++)
  {
    QString ss = QString::number(i, 10);
    err = QH5Lite::readScalarAttribute(m_CurrentGroupId, name, ss, scalar);
    value.push_back(scalar);
  }

  return value;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QVector<int16_t> H5FilterParametersReader::readArray(const QString& name, QVector<int16_t> value)
{
  int vectorSize = 0;
  int16_t scalar = 0;
  int err = QH5Lite::readScalarDataset(m_CurrentGroupId, name, vectorSize);
  if(err < 0)
  {
    return value;
  }

  value.clear();

  for(int i = 0; i < vectorSize; i++)
  {
    QString ss = QString::number(i, 10);
    err = QH5Lite::readScalarAttribute(m_CurrentGroupId, name, ss, scalar);
    value.push_back(scalar);
  }

  return value;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QVector<int32_t> H5FilterParametersReader::readArray(const QString& name, QVector<int32_t> value)
{
  int vectorSize = 0;
  int32_t scalar = 0;
  int err = QH5Lite::readScalarDataset(m_CurrentGroupId, name, vectorSize);
  if(err < 0)
  {
    return value;
  }

  value.clear();

  for(int i = 0; i < vectorSize; i++)
  {
    QString ss = QString::number(i, 10);
    err = QH5Lite::readScalarAttribute(m_CurrentGroupId, name, ss, scalar);
    value.push_back(scalar);
    ss.clear();
  }

  return value;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QVector<int64_t> H5FilterParametersReader::readArray(const QString& name, QVector<int64_t> value)
{
  int vectorSize = 0;
  int64_t scalar = 0;
  int err = QH5Lite::readScalarDataset(m_CurrentGroupId, name, vectorSize);
  if(err < 0)
  {
    return value;
  }

  value.clear();

  for(int i = 0; i < vectorSize; i++)
  {
    QString ss = QString::number(i, 10);
    err = QH5Lite::readScalarAttribute(m_CurrentGroupId, name, ss, scalar);
    value.push_back(scalar);
  }

  return value;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QVector<uint8_t> H5FilterParametersReader::readArray(const QString& name, QVector<uint8_t> value)
{
  int vectorSize = 0;
  uint8_t scalar = 0;
  int err = QH5Lite::readScalarDataset(m_CurrentGroupId, name, vectorSize);
  if(err < 0)
  {
    return value;
  }

  value.clear();

  for(int i = 0; i < vectorSize; i++)
  {
    QString ss = QString::number(i, 10);
    err = QH5Lite::readScalarAttribute(m_CurrentGroupId, name, ss, scalar);
    value.push_back(scalar);
  }

  return value;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QVector<uint16_t> H5FilterParametersReader::readArray(const QString& name, QVector<uint16_t> value)
{
  int vectorSize = 0;
  uint16_t scalar = 0;
  int err = QH5Lite::readScalarDataset(m_CurrentGroupId, name, vectorSize);
  if(err < 0)
  {
    return value;
  }

  value.clear();

  for(int i = 0; i < vectorSize; i++)
  {
    QString ss = QString::number(i, 10);
    err = QH5Lite::readScalarAttribute(m_CurrentGroupId, name, ss, scalar);
    value.push_back(scalar);
  }

  return value;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QVector<uint32_t> H5FilterParametersReader::readArray(const QString& name, QVector<uint32_t> value)
{
  int vectorSize = 0;
  uint32_t scalar = 0;
  int err = QH5Lite::readScalarDataset(m_CurrentGroupId, name, vectorSize);
  if(err < 0)
  {
    return value;
  }

  value.clear();

  for(int i = 0; i < vectorSize; i++)
  {
    QString ss = QString::number(i, 10);
    err = QH5Lite::readScalarAttribute(m_CurrentGroupId, name, ss, scalar);
    value.push_back(scalar);
  }

  return value;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QVector<uint64_t> H5FilterParametersReader::readArray(const QString& name, QVector<uint64_t> value)
{
  int vectorSize = 0;
  uint64_t scalar = 0;
  int err = QH5Lite::readScalarDataset(m_CurrentGroupId, name, vectorSize);
  if(err < 0)
  {
    return value;
  }

  value.clear();

  for(int i = 0; i < vectorSize; i++)
  {
    QString ss = QString::number(i, 10);
    err = QH5Lite::readScalarAttribute(m_CurrentGroupId, name, ss, scalar);
    value.push_back(scalar);
  }

  return value;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QVector<float> H5FilterParametersReader::readArray(const QString& name, QVector<float> value)
{
  int vectorSize = 0;
  float scalar = 0.0f;
  int err = QH5Lite::readScalarDataset(m_CurrentGroupId, name, vectorSize);
  if(err < 0)
  {
    return value;
  }

  value.clear();

  for(int i = 0; i < vectorSize; i++)
  {
    QString ss = QString::number(i, 10);
    err = QH5Lite::readScalarAttribute(m_CurrentGroupId, name, ss, scalar);
    value.push_back(scalar);
  }

  return value;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QVector<double> H5FilterParametersReader::readArray(const QString& name, QVector<double> value)
{
  int vectorSize = 0;
  double scalar = 0.0;
  int err = QH5Lite::readScalarDataset(m_CurrentGroupId, name, vectorSize);
  if(err < 0)
  {
    return value;
  }

  value.clear();

  for(int i = 0; i < vectorSize; i++)
  {
    QString ss = QString::number(i, 10);
    err = QH5Lite::readScalarAttribute(m_CurrentGroupId, name, ss, scalar);
    value.push_back(scalar);
  }

  return value;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
IntVec3Type H5FilterParametersReader::readIntVec3(const QString& name, IntVec3Type defaultValue)
{
  int err = 0;
  IntVec3Type v;
  err = QH5Lite::readPointerDataset<int32_t>(m_CurrentGroupId, name, reinterpret_cast<int32_t*>(&v));
  if(err < 0)
  {
    return defaultValue;
  }

  return v;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
FloatVec3Type H5FilterParametersReader::readFloatVec3(const QString& name, FloatVec3Type defaultValue)
{
  int err = 0;
  FloatVec3Type v;
  err = QH5Lite::readPointerDataset<float>(m_CurrentGroupId, name, reinterpret_cast<float*>(&v));
  if(err < 0)
  {
    return defaultValue;
  }
  return v;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
Float2ndOrderPolynomial H5FilterParametersReader::readFloat2ndOrderPoly(const QString& name, Float2ndOrderPolynomial defaultValue)
{
  int err = 0;
  Float2ndOrderPolynomial v;
  err = QH5Lite::readPointerDataset<float>(m_CurrentGroupId, name, reinterpret_cast<float*>(&v));
  if(err < 0)
  {
    return defaultValue;
  }
  return v;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
Float3rdOrderPoly_t H5FilterParametersReader::readFloat3rdOrderPoly(const QString& name, Float3rdOrderPoly_t defaultValue)
{
  int err = 0;
  Float3rdOrderPoly_t v;
  err = QH5Lite::readPointerDataset<float>(m_CurrentGroupId, name, reinterpret_cast<float*>(&v));
  if(err < 0)
  {
    return defaultValue;
  }
  return v;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
Float4thOrderPolynomial H5FilterParametersReader::readFloat4thOrderPoly(const QString& name, Float4thOrderPolynomial defaultValue)
{
  int err = 0;
  Float4thOrderPolynomial v;
  err = QH5Lite::readPointerDataset<float>(m_CurrentGroupId, name, reinterpret_cast<float*>(&v));
  if(err < 0)
  {
    return defaultValue;
  }
  return v;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
StackFileListInfo H5FilterParametersReader::readFileListInfo(const QString& name, StackFileListInfo defaultValue)
{

  StackFileListInfo v;

  hid_t gid = QH5Utilities::openHDF5Object(m_CurrentGroupId, name);
  if(gid < 0)
  {
    return defaultValue;
  }

  int err = QH5Lite::readScalarDataset<qint32>(gid, "EndIndex", v.EndIndex);
  if(err < 0)
  {
  }
  if(err < 0)
  {
  }
  err = QH5Lite::readScalarDataset<qint32>(gid, "StartIndex", v.StartIndex);
  if(err < 0)
  {
  }
  err = QH5Lite::readScalarDataset<qint32>(gid, "PaddingDigits", v.PaddingDigits);
  if(err < 0)
  {
  }
  err = QH5Lite::readScalarDataset<quint32>(gid, "Ordering", v.Ordering);
  if(err < 0)
  {
  }
  err = QH5Lite::readStringDataset(gid, "FileExtension", v.FileExtension);
  if(err < 0)
  {
  }
  err = QH5Lite::readStringDataset(gid, "FilePrefix", v.FilePrefix);
  if(err < 0)
  {
  }
  err = QH5Lite::readStringDataset(gid, "FileSuffix", v.FileSuffix);
  if(err < 0)
  {
  }
  err = QH5Lite::readStringDataset(gid, "InputPath", v.InputPath);
  if(err < 0)
  {
  }
  H5Gclose(gid); // Close the Group Object before we return

  return v;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
ComparisonInput_t H5FilterParametersReader::readComparisonInput(const QString& name, ComparisonInput_t defaultValue, int vectorPos)
{
  // QVector<ComparisonInput_t> comps(1, defaultValue);
  ComparisonInputs comps;
  ComparisonInputs values = readComparisonInputs(name, comps);
  if(values.size() >= 1)
  {
    return values.getInput(0);
  }
  return defaultValue;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
ComparisonInputs H5FilterParametersReader::readComparisonInputs(const QString& name, ComparisonInputs defValue)
{
  int size = 0;
  QString strData = "";
  bool ok = false;
  // See if the data set actually exists, if it does NOT just return what the user passed in as a default value
  if(!QH5Lite::datasetExists(m_CurrentGroupId, name))
  {
    return defValue;
  }

  herr_t err = QH5Lite::readStringDataset(m_CurrentGroupId, name, strData);
  if(err < 0)
  {
    return defValue; // If the data set does not exist no point in going any further
  }

  // Now read the the attribute that says how many arrays are in the data set.
  err = QH5Lite::readScalarAttribute(m_CurrentGroupId, name, "NumInputs", size);

  QStringList strVector = strData.split('\n', QSTRING_SKIP_EMPTY_PARTS);
  qint32 strVecSize = strVector.size();
  if(strVecSize != size)
  {
    // Something has gone wrong in the tokenization and the number of tokens does not match what
    // was written to the HDF5 file.
    return defValue;
  }

  ComparisonInputs values;
  for(qint32 i = 0; i < size; ++i)
  {
    ComparisonInput_t v;
    QStringList tokens = strVector[i].split(',', QSTRING_SKIP_EMPTY_PARTS);
    if(!tokens.empty())
    {
      v.dataContainerName = tokens[0];
    }
    if(tokens.size() >= 2)
    {
      v.attributeMatrixName = tokens[1];
    }
    if(tokens.size() >= 3)
    {
      v.attributeArrayName = tokens[2];
    }
    if(tokens.size() >= 4)
    {
      v.compOperator = QString(tokens[3]).toInt(&ok, 10);
    }
    if(tokens.size() >= 5)
    {
      v.compValue = QString(tokens[4]).toDouble(&ok);
    }
    values.addInput(v);
  }
  return values;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
ComparisonInputsAdvanced H5FilterParametersReader::readComparisonInputsAdvanced(const QString& name, ComparisonInputsAdvanced defValue)
{
  /*
  int size = 0;
  QString strData = "";
  bool ok = false;
  // See if the data set actually exists, if it does NOT just return what the user passed in as a default value
  if(false == QH5Lite::datasetExists(m_CurrentGroupId, name))
  {
    return defValue;
  }

  herr_t err = QH5Lite::readStringDataset(m_CurrentGroupId, name, strData);
  if(err < 0)
  {
    return defValue; // If the data set does not exist no point in going any further
  }

  // Now read the the attribute that says how many arrays are in the data set.
  err = QH5Lite::readScalarAttribute(m_CurrentGroupId, name, "NumInputs", size);

  QStringList strVector = strData.split('\n', QSTRING_SKIP_EMPTY_PARTS);
  qint32 strVecSize = strVector.size();
  if(strVecSize != size)
  {
    // Something has gone wrong in the tokenization and the number of tokens does not match what
    // was written to the HDF5 file.
    return defValue;
  }
  */

  ComparisonInputsAdvanced inputs;

  return inputs;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
AxisAngleInput H5FilterParametersReader::readAxisAngle(const QString& name, AxisAngleInput v, int vectorPos)
{
  int err = 0;

  QString ss = QString::number(vectorPos) + H5FilterParameter::AxisAngleInput;
  err = QH5Lite::readPointerAttribute<float>(m_CurrentGroupId, name, ss, reinterpret_cast<float*>(&v));
  if(err < 0)
  {
    v.angle = std::numeric_limits<float>::quiet_NaN();
    v.h = std::numeric_limits<float>::quiet_NaN();
    v.k = std::numeric_limits<float>::quiet_NaN();
    v.l = std::numeric_limits<float>::quiet_NaN();
  }
  return v;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QVector<AxisAngleInput> H5FilterParametersReader::readAxisAngles(const QString& name, QVector<AxisAngleInput> v)
{
  std::ignore = v;
  QVector<AxisAngleInput> axisAngleInputsVector;
  AxisAngleInput axisAngleDummyInput;
  axisAngleDummyInput.angle = 0.0f;
  axisAngleDummyInput.h = 0.0f;
  axisAngleDummyInput.k = 0.0f;
  axisAngleDummyInput.l = 0.0f;
  int vectorSize = static_cast<int>(readValue(name, 0));
  for(int i = 0; i < vectorSize; i++)
  {
    axisAngleInputsVector.push_back(readAxisAngle(name, axisAngleDummyInput, i));
  }
  return axisAngleInputsVector;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QSet<QString> H5FilterParametersReader::readArraySelections(const QString& name, QSet<QString> v)
{
  int size = 0;
  QString strData = "";

  // See if the data set actually exists, if it does NOT just return what the user passed in as a default value
  if(!QH5Lite::datasetExists(m_CurrentGroupId, name))
  {
    return v;
  }

  herr_t err = QH5Lite::readStringDataset(m_CurrentGroupId, name, strData);
  if(err < 0)
  {
    return v; // If the data set does not exist no point in going any further
  }

  // Now read the the attribute that says how many arrays are in the data set.
  err = QH5Lite::readScalarAttribute(m_CurrentGroupId, name, "NumArrays", size);

  QStringList strVector = strData.split('\n', QSTRING_SKIP_EMPTY_PARTS);
  qint32 strVecSize = strVector.size();
  if(strVecSize != size)
  {
    // Something has gone wrong in the tokenization and the number of tokens does not match what
    // was written to the HDF5 file.
    return v;
  }
  for(int i = 0; i < strVector.size(); i++)
  {
    v.insert(strVector[i]);
  }
  return v;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
DataContainerArrayProxy H5FilterParametersReader::readDataContainerArrayProxy(const QString& name, DataContainerArrayProxy defValue)
{
  DataContainerArrayProxy dcaProxy;
  // Open the top level group that holds all the values for this filter parameter
  hid_t dcaGid = QH5Utilities::openHDF5Object(m_CurrentGroupId, name);
  H5ScopedGroupSentinel sentinel(dcaGid, true);
  if(dcaGid < 0)
  {
    return defValue;
  }
  QList<QString> dcaNames;
  int err = QH5Utilities::getGroupObjects(dcaGid, H5Utilities::CustomHDFDataTypes::Group, dcaNames);
  if(err < 0)
  {
    return defValue;
  }
  // Loop over all the data Containers
  for(const auto& dcName : dcaNames)
  {
    hid_t dcGid = QH5Utilities::openHDF5Object(dcaGid, dcName);
    H5ScopedGroupSentinel sentinal_dc(dcGid, false);
    DataContainerProxy dcProxy;
    dcProxy.setName(dcName);
    dcProxy.setFlag(Qt::Checked);
    // Loop over the attribute Matrices
    QList<QString> amNames;
    err = QH5Utilities::getGroupObjects(dcGid, H5Utilities::CustomHDFDataTypes::Group, amNames);
    if(err < 0)
    {
      return defValue;
    }

    for(const auto& amName : amNames)
    {
      hid_t amGid = QH5Utilities::openHDF5Object(dcGid, amName);
      H5ScopedGroupSentinel sentinal_am(amGid, false);
      AttributeMatrixProxy amProxy(amName, 1u);
      amProxy.setFlag(Qt::Checked);
      QString data; // Output will be read into this object
      err = QH5Lite::readStringDataset(amGid, "Arrays", data);
      if(err < 0)
      {
        return defValue;
      }
      QStringList arrayNames = data.split('\n');
      QString path = SIMPL::StringConstants::DataContainerGroupName + "/" + dcProxy.getName() + "/" + amProxy.getName();
      for(int k = 0; k < arrayNames.size(); k++)
      {
        DataArrayProxy daProxy(path, arrayNames.at(k), 1u);
        daProxy.setFlag(Qt::Checked);
        amProxy.insertDataArray(arrayNames.at(k), daProxy);
      }
      dcProxy.insertAttributeMatrix(amName, amProxy);
    }
    // Add this DataContainerProxy to the DataContainerArrayProxy
    dcaProxy.insertDataContainer(dcProxy.getName(), dcProxy);
  }
  return dcaProxy;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
DataArrayPath H5FilterParametersReader::readDataArrayPath(const QString& name, DataArrayPath def)
{
  QString value;
  int err = 0;
  err = QH5Lite::readStringDataset(m_CurrentGroupId, name, value);
  if(err == 0)
  {
    DataArrayPath path(value);
    return path;
  }

  return def;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QVector<DataArrayPath> H5FilterParametersReader::readDataArrayPathVector(const QString& name, QVector<DataArrayPath> def)
{
  QVector<DataArrayPath> defPaths;

  int err = 0;
  QString pathStr;
  err = QH5Lite::readStringDataset(m_CurrentGroupId, name, pathStr);
  if(err < 0)
  {
    return def;
  }
  QStringList tokens = pathStr.split('\n');

  for(const auto& str : tokens)
  {
    DataArrayPath path = DataArrayPath::Deserialize(str, "|");
    defPaths.push_back(path);
  }

  if(defPaths.isEmpty())
  {
    return def;
  }

  return defPaths;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
DynamicTableData H5FilterParametersReader::readDynamicTableData(const QString& name, DynamicTableData def)
{
  std::vector<double> dataVec;
  QString rHeadersStr, cHeadersStr;
  int err = 0;

  err = QH5Lite::readVectorDataset(m_CurrentGroupId, name, dataVec);
  err = QH5Lite::readStringAttribute(m_CurrentGroupId, name, "RowHeaders", rHeadersStr);
  err = QH5Lite::readStringAttribute(m_CurrentGroupId, name, "ColHeaders", cHeadersStr);

  if(err == 0)
  {
    QStringList rHeaders = DynamicTableData::DeserializeHeaders(rHeadersStr, '|');
    QStringList cHeaders = DynamicTableData::DeserializeHeaders(cHeadersStr, '|');
    std::vector<std::vector<double>> data = DynamicTableData::ExpandData(dataVec, rHeaders.size(), cHeaders.size());

    DynamicTableData tableData(data, rHeaders, cHeaders);
    return tableData;
  }

  return def;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
FPRangePair H5FilterParametersReader::readPairOfDoubles(const QString& name, const FPRangePair& v)
{
  /*int err = QH5Lite::writeScalarAttribute(m_CurrentGroupId, name, "Min", v.first);
  if (err < 0) { return err; }
  err = QH5Lite::writeScalarAttribute(m_CurrentGroupId, name, "Max", v.second);

  return err;*/

  double min = 0.0;
  double max = 0.0;
  int err = QH5Lite::readScalarDataset(m_CurrentGroupId, name + "_Min", min);
  if(err < 0)
  {
    return v;
  }
  err = QH5Lite::readScalarDataset(m_CurrentGroupId, name + "_Max", max);
  if(err < 0)
  {
    return v;
  }

  FPRangePair value;
  value.first = min;
  value.second = max;
  return value;
}

// -----------------------------------------------------------------------------
H5FilterParametersReader::Pointer H5FilterParametersReader::NullPointer()
{
  return Pointer(static_cast<Self*>(nullptr));
}

// -----------------------------------------------------------------------------
H5FilterParametersReader::Pointer H5FilterParametersReader::New()
{
  Pointer sharedPtr(new(H5FilterParametersReader));
  return sharedPtr;
}

// -----------------------------------------------------------------------------
QString H5FilterParametersReader::getNameOfClass() const
{
  return QString("H5FilterParametersReader");
}

// -----------------------------------------------------------------------------
QString H5FilterParametersReader::ClassName()
{
  return QString("H5FilterParametersReader");
}

// -----------------------------------------------------------------------------
void H5FilterParametersReader::setPipelineGroupId(hid_t value)
{
  m_PipelineGroupId = value;
}

// -----------------------------------------------------------------------------
hid_t H5FilterParametersReader::getPipelineGroupId() const
{
  return m_PipelineGroupId;
}
