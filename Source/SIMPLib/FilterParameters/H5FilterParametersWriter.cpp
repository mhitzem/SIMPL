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

#include "H5FilterParametersWriter.h"

#include <tuple>

#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QTextStream>

#include "H5Support/H5ScopedSentinel.h"
#include "H5Support/QH5Lite.h"
#include "H5Support/QH5Utilities.h"

#include "SIMPLib/SIMPLibVersion.h"
#include "SIMPLib/Common/Constants.h"
#include "SIMPLib/FilterParameters/H5FilterParametersConstants.h"
#include "SIMPLib/FilterParameters/JsonFilterParametersWriter.h"
#include "SIMPLib/Messages/GenericErrorMessage.h"
#include "SIMPLib/Messages/PipelineErrorMessage.h"

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
H5FilterParametersWriter::H5FilterParametersWriter() = default;

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
H5FilterParametersWriter::~H5FilterParametersWriter() = default;

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
hid_t H5FilterParametersWriter::getCurrentGroupId() const
{
  return m_CurrentGroupId;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int H5FilterParametersWriter::writePipelineToFile(FilterPipeline::Pointer pipeline, QString filePath, QString pipelineName, bool expandPipeline, QList<IObserver*> obs)
{
  if(nullptr == pipeline.get())
  {
    if(!obs.empty())
    {
      GenericErrorMessage::Pointer pm = GenericErrorMessage::New(QObject::tr("%1: %2").arg(JsonFilterParametersWriter::ClassName()).arg("FilterPipeline Object was nullptr for writing"), -1);

      for(int i = 0; i < obs.size(); i++)
      {
        if(obs[i] != nullptr)
        {
          obs[i]->processPipelineMessage(pm);
        }
      }
    }
    return -1;
  }

  // WRITE THE PIPELINE TO THE HDF5 FILE
  hid_t fileId = -1;

  QFileInfo fi(filePath);
  if(fi.exists())
  {
    fileId = QH5Utilities::openFile(filePath);
  }
  else
  {
    fileId = QH5Utilities::createFile(filePath);
  }

  if(fileId < 0)
  {
    if(!obs.empty())
    {
      PipelineErrorMessage::Pointer pm =
          PipelineErrorMessage::New(pipeline->getName(), QObject::tr("%1: %2").arg(JsonFilterParametersWriter::ClassName()).arg("Output .dream3d file could not be created."), -1);

      for(int i = 0; i < obs.size(); i++)
      {
        if(obs[i] != nullptr)
        {
          obs[i]->processPipelineMessage(pm);
        }
      }
    }
    return -1;
  }

  // This will make sure if we return early from this method that the HDF5 File is properly closed.
  // This will also take care of making sure all groups and file ids are closed
  // before this method returns.
  H5ScopedFileSentinel scopedFileSentinel(fileId, true);

  // Write our File Version string to the Root "/" group
  QH5Lite::writeStringAttribute(fileId, "/", SIMPL::HDF5::FileVersionName, SIMPL::HDF5::FileVersion);
  QH5Lite::writeStringAttribute(fileId, "/", SIMPL::HDF5::DREAM3DVersion, SIMPLib::Version::Complete());

  hid_t pipelineGroupId = QH5Utilities::createGroup(fileId, SIMPL::StringConstants::PipelineGroupName);
  scopedFileSentinel.addGroupId(pipelineGroupId);
  setGroupId(pipelineGroupId);

  QH5Lite::writeScalarAttribute(pipelineGroupId, "/" + SIMPL::StringConstants::PipelineGroupName, SIMPL::StringConstants::PipelineVersionName, 2);
  QH5Lite::writeStringAttribute(pipelineGroupId, "/" + SIMPL::StringConstants::PipelineGroupName, SIMPL::StringConstants::PipelineCurrentName, pipelineName);

  JsonFilterParametersWriter::Pointer jsonWriter = JsonFilterParametersWriter::New();
  QString jsonString = jsonWriter->writePipelineToString(pipeline, pipelineName, expandPipeline, obs);
  QH5Lite::writeStringDataset(pipelineGroupId, pipelineName, jsonString);

  return 0;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int H5FilterParametersWriter::openFilterGroup(AbstractFilter* filter, int index)
{
  // We are piggy-backing off of the Json writer, so we don't need this function
  Q_UNUSED(filter)
  Q_UNUSED(index)

  return 0;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int H5FilterParametersWriter::closeFilterGroup()
{
  // We are piggy-backing off of the Json writer, so we don't need this function
  return 0;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int H5FilterParametersWriter::writeValue(const QString& name, const QString& value)
{
  int err = 0;
  QString repl = value;
  repl.replace("\\", QString("/"));
  err = QH5Lite::writeStringDataset(m_CurrentGroupId, name, repl);
  return err;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int H5FilterParametersWriter::writeValue(const QString& name, const QVector<QString>& value)
{
  int vectorSize = value.size();
  int err = QH5Lite::writeScalarDataset(m_CurrentGroupId, name, vectorSize);
  for(int i = 0; i < vectorSize; i++)
  {
    QString ss = QString::number(i, 10);
    QString repl = value[i];
    repl.replace("\\", QString("/"));
    err = QH5Lite::writeStringAttribute(m_CurrentGroupId, name, ss, repl);
    ss.clear();
  }

  return err;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int H5FilterParametersWriter::writeValue(const QString& name, const QStringList& value)
{
  int vectorSize = value.size();
  int err = QH5Lite::writeScalarDataset(m_CurrentGroupId, name, vectorSize);
  for(int i = 0; i < vectorSize; i++)
  {
    QString ss = QString::number(i, 10);
    err = QH5Lite::writeStringAttribute(m_CurrentGroupId, name, ss, value[i]);
    ss.clear();
  }

  return err;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int H5FilterParametersWriter::writeValue(const QString& name, int8_t value)
{
  int err = 0;
  err = QH5Lite::writeScalarDataset(m_CurrentGroupId, name, value);
  return err;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int H5FilterParametersWriter::writeValue(const QString& name, int16_t value)
{
  int err = 0;
  err = QH5Lite::writeScalarDataset(m_CurrentGroupId, name, value);
  return err;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int H5FilterParametersWriter::writeValue(const QString& name, int32_t value)
{
  int err = 0;
  err = QH5Lite::writeScalarDataset(m_CurrentGroupId, name, value);
  return err;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int H5FilterParametersWriter::writeValue(const QString& name, int64_t value)
{
  int err = 0;
  err = QH5Lite::writeScalarDataset(m_CurrentGroupId, name, value);
  return err;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int H5FilterParametersWriter::writeValue(const QString& name, uint8_t value)
{
  int err = 0;
  err = QH5Lite::writeScalarDataset(m_CurrentGroupId, name, value);
  return err;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int H5FilterParametersWriter::writeValue(const QString& name, uint16_t value)
{
  int err = 0;
  err = QH5Lite::writeScalarDataset(m_CurrentGroupId, name, value);
  return err;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int H5FilterParametersWriter::writeValue(const QString& name, uint32_t value)
{
  int err = 0;
  err = QH5Lite::writeScalarDataset(m_CurrentGroupId, name, value);
  return err;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int H5FilterParametersWriter::writeValue(const QString& name, uint64_t value)
{
  int err = 0;
  err = QH5Lite::writeScalarDataset(m_CurrentGroupId, name, value);
  return err;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int H5FilterParametersWriter::writeValue(const QString& name, float value)
{
  int err = 0;
  err = QH5Lite::writeScalarDataset(m_CurrentGroupId, name, value);
  return err;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int H5FilterParametersWriter::writeValue(const QString& name, double value)
{
  int err = 0;
  err = QH5Lite::writeScalarDataset(m_CurrentGroupId, name, value);
  return err;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int H5FilterParametersWriter::writeValue(const QString& name, QVector<int8_t> value)
{
  int vectorSize = value.size();
  int err = QH5Lite::writeScalarDataset(m_CurrentGroupId, name, vectorSize);
  for(int i = 0; i < vectorSize; i++)
  {
    QString ss = QString::number(i, 10);
    err = QH5Lite::writeScalarAttribute(m_CurrentGroupId, name, ss, value[i]);
  }

  return err;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int H5FilterParametersWriter::writeValue(const QString& name, QVector<int16_t> value)
{
  int vectorSize = value.size();
  int err = QH5Lite::writeScalarDataset(m_CurrentGroupId, name, vectorSize);
  for(int i = 0; i < vectorSize; i++)
  {
    QString ss = QString::number(i, 10);
    err = QH5Lite::writeScalarAttribute(m_CurrentGroupId, name, ss, value[i]);
  }

  return err;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int H5FilterParametersWriter::writeValue(const QString& name, QVector<int32_t> value)
{
  int vectorSize = value.size();
  int err = QH5Lite::writeScalarDataset(m_CurrentGroupId, name, vectorSize);
  for(int i = 0; i < vectorSize; i++)
  {
    QString ss = QString::number(i, 10);
    err = QH5Lite::writeScalarAttribute(m_CurrentGroupId, name, ss, value[i]);
  }

  return err;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int H5FilterParametersWriter::writeValue(const QString& name, QVector<int64_t> value)
{
  int vectorSize = value.size();
  int err = QH5Lite::writeScalarDataset(m_CurrentGroupId, name, vectorSize);
  for(int i = 0; i < vectorSize; i++)
  {
    QString ss = QString::number(i, 10);
    err = QH5Lite::writeScalarAttribute(m_CurrentGroupId, name, ss, value[i]);
  }

  return err;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int H5FilterParametersWriter::writeValue(const QString& name, QVector<uint8_t> value)
{
  int vectorSize = value.size();
  int err = QH5Lite::writeScalarDataset(m_CurrentGroupId, name, vectorSize);
  for(int i = 0; i < vectorSize; i++)
  {
    QString ss = QString::number(i, 10);
    err = QH5Lite::writeScalarAttribute(m_CurrentGroupId, name, ss, value[i]);
  }

  return err;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int H5FilterParametersWriter::writeValue(const QString& name, QVector<uint16_t> value)
{
  int vectorSize = value.size();
  int err = QH5Lite::writeScalarDataset(m_CurrentGroupId, name, vectorSize);
  for(int i = 0; i < vectorSize; i++)
  {
    QString ss = QString::number(i, 10);
    err = QH5Lite::writeScalarAttribute(m_CurrentGroupId, name, ss, value[i]);
  }

  return err;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int H5FilterParametersWriter::writeValue(const QString& name, QVector<uint32_t> value)
{
  int vectorSize = value.size();
  int err = QH5Lite::writeScalarDataset(m_CurrentGroupId, name, vectorSize);
  for(int i = 0; i < vectorSize; i++)
  {
    QString ss = QString::number(i, 10);
    err = QH5Lite::writeScalarAttribute(m_CurrentGroupId, name, ss, value[i]);
  }

  return err;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int H5FilterParametersWriter::writeValue(const QString& name, QVector<uint64_t> value)
{
  int vectorSize = value.size();
  int err = QH5Lite::writeScalarDataset(m_CurrentGroupId, name, vectorSize);
  for(int i = 0; i < vectorSize; i++)
  {
    QString ss = QString::number(i, 10);
    err = QH5Lite::writeScalarAttribute(m_CurrentGroupId, name, ss, value[i]);
  }

  return err;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int H5FilterParametersWriter::writeValue(const QString& name, QVector<float> value)
{
  int vectorSize = value.size();
  int err = QH5Lite::writeScalarDataset(m_CurrentGroupId, name, vectorSize);
  for(int i = 0; i < vectorSize; i++)
  {
    QString ss = QString::number(i, 10);
    err = QH5Lite::writeScalarAttribute(m_CurrentGroupId, name, ss, value[i]);
  }

  return err;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int H5FilterParametersWriter::writeValue(const QString& name, QVector<double> value)
{
  int vectorSize = value.size();
  int err = QH5Lite::writeScalarDataset(m_CurrentGroupId, name, vectorSize);
  for(int i = 0; i < vectorSize; i++)
  {
    QString ss = QString::number(i, 10);
    err = QH5Lite::writeScalarAttribute(m_CurrentGroupId, name, ss, value[i]);
  }

  return err;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int H5FilterParametersWriter::writeValue(const QString& name, IntVec3Type v)
{
  int err = 0;
  int32_t rank = 1;
  hsize_t dims[1] = {3};
  err = QH5Lite::writePointerDataset<int32_t>(m_CurrentGroupId, name, rank, dims, reinterpret_cast<int32_t*>(&v));
  return err;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int H5FilterParametersWriter::writeValue(const QString& name, FloatVec3Type v)
{
  int err = 0;
  int32_t rank = 1;
  hsize_t dims[1] = {3};
  err = QH5Lite::writePointerDataset<float>(m_CurrentGroupId, name, rank, dims, reinterpret_cast<float*>(&v));
  return err;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int H5FilterParametersWriter::writeValue(const QString& name, Float2ndOrderPolynomial v)
{
  int err = 0;
  int32_t rank = 1;
  hsize_t dims[1] = {6};
  err = QH5Lite::writePointerDataset<float>(m_CurrentGroupId, name, rank, dims, reinterpret_cast<float*>(&v));
  return err;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int H5FilterParametersWriter::writeValue(const QString& name, Float3rdOrderPoly_t v)
{
  int err = 0;
  int32_t rank = 1;
  hsize_t dims[1] = {21};
  err = QH5Lite::writePointerDataset<float>(m_CurrentGroupId, name, rank, dims, reinterpret_cast<float*>(&v));
  return err;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int H5FilterParametersWriter::writeValue(const QString& name, Float4thOrderPolynomial v)
{
  int err = 0;
  int32_t rank = 1;
  hsize_t dims[1] = {21};
  err = QH5Lite::writePointerDataset<float>(m_CurrentGroupId, name, rank, dims, reinterpret_cast<float*>(&v));
  return err;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int H5FilterParametersWriter::writeValue(const QString& name, StackFileListInfo v)
{
  int err = 0;
  hid_t dcaGid = QH5Utilities::createGroup(m_CurrentGroupId, name);

  err = QH5Lite::writeScalarDataset<qint32>(dcaGid, "EndIndex", v.EndIndex);
  err = QH5Lite::writeScalarDataset<qint32>(dcaGid, "StartIndex", v.StartIndex);
  err = QH5Lite::writeScalarDataset<qint32>(dcaGid, "PaddingDigits", v.PaddingDigits);
  err = QH5Lite::writeScalarDataset<quint32>(dcaGid, "Ordering", v.Ordering);
  err = QH5Lite::writeStringDataset(dcaGid, "FileExtension", v.FileExtension);
  err = QH5Lite::writeStringDataset(dcaGid, "FilePrefix", v.FilePrefix);
  err = QH5Lite::writeStringDataset(dcaGid, "FileSuffix", v.FileSuffix);
  err = QH5Lite::writeStringDataset(dcaGid, "InputPath", v.InputPath);
  dcaGid = H5Gclose(dcaGid);
  return err;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int H5FilterParametersWriter::writeValue(const QString& name, ComparisonInput_t comp)
{
  ComparisonInputs v;
  v.addInput(comp);
  return writeValue(name, v);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int H5FilterParametersWriter::writeValue(const QString& name, ComparisonInputs v)
{
  int numQFilters = static_cast<int>(v.size());
  int err = 0;
  QString buf;
  QTextStream ss(&buf);

  // Create a string that is comma delimited for each comparison operator and new line delimited
  // to mark the end of each ComparisonOpertor struct
  for(int i = 0; i < numQFilters; i++)
  {
    ss << v[i].dataContainerName << "," << v[i].attributeMatrixName << "," << v[i].attributeArrayName << "," << v[i].compOperator << "," << v[i].compValue << "\n";
  }

  // Write the data set to the file and attach an attribute that says how many there were.
  if(numQFilters > 0)
  {
    err = QH5Lite::writeStringDataset(m_CurrentGroupId, name, buf);
    if(err < 0)
    {
      return err;
    }
    err = QH5Lite::writeScalarAttribute(m_CurrentGroupId, name, "NumInputs", numQFilters);
    if(err < 0)
    {
      return err;
    }
  }

  return err;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int H5FilterParametersWriter::writeValue(const QString& name, AxisAngleInput v)
{
  int numQFilters = static_cast<int>(1);
  int err = writeValue(name, numQFilters);
  if(err < 0)
  {
    return err;
  }
  QString parseOrder = "Angle, H, K, L";
  err = QH5Lite::writeStringAttribute(m_CurrentGroupId, name, "Data Order", parseOrder);
  if(err < 0)
  {
    return err;
  }
  err = writeValue(name, v, 0);
  if(err < 0)
  {
    return err;
  }
  return err;
}

// -----------------------------------------------------------------------------
// These are actually written as a binary 4x1 float array as an attribute
// -----------------------------------------------------------------------------
int H5FilterParametersWriter::writeValue(const QString& name, AxisAngleInput v, int vectorPos)
{
  int err = 0;
  int32_t rank = 1;
  hsize_t dims[1] = {4};

  QString ss = QString::number(vectorPos) + H5FilterParameter::AxisAngleInput;

  err = QH5Lite::writePointerAttribute<float>(m_CurrentGroupId, name, ss, rank, dims, reinterpret_cast<float*>(&v));
  return err;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int H5FilterParametersWriter::writeValue(const QString& name, QVector<AxisAngleInput> v)
{
  int numQFilters = static_cast<int>(v.size());
  int err = writeValue(name, numQFilters);
  if(err < 0)
  {
    return err;
  }
  QString parseOrder = "Angle, H, K, L";
  err = QH5Lite::writeStringAttribute(m_CurrentGroupId, name, "Data Order", parseOrder);
  if(err < 0)
  {
    return err;
  }
  for(int i = 0; i < numQFilters; i++)
  {
    err = writeValue(name, v[i], i);
    if(err < 0)
    {
      return err;
    }
  }
  return err;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int H5FilterParametersWriter::writeArraySelections(const QString& name, QSet<QString> v)
{
  size_t size = v.size();
  herr_t err = 0;
  if(size > 0)
  {
    QString setStr = "";
    QSet<QString>::iterator iter = v.begin();
    for(; iter != v.end(); iter++)
    {
      setStr.append(*iter).append("\n");
    }

    err = QH5Lite::writeStringDataset(m_CurrentGroupId, name, setStr);
    if(err < 0)
    {
      return err;
    }
  }
  if(size > 0)
  {
    err = QH5Lite::writeScalarAttribute(m_CurrentGroupId, name, "NumArrays", size);
    if(err < 0)
    {
      return err;
    }
  }
  return err;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int H5FilterParametersWriter::writeValue(const QString& name, DataContainerArrayProxy& dcaProxy)
{
  int err = 0;
  DataContainerArrayProxy::StorageType& dcProxies = dcaProxy.getDataContainers();
  hid_t dcaGid = QH5Utilities::createGroup(m_CurrentGroupId, name);

  for(auto& dcProxy : dcProxies)
  {
    if(dcProxy.getFlag() == Qt::Unchecked)
    {
      continue; // Skip to the next DataContainer if we are not reading this one.
    }
    hid_t dcGid = QH5Utilities::createGroup(dcaGid, dcProxy.getName());

    QStringList flat;
    DataContainerProxy::StorageType& amMap = dcProxy.getAttributeMatricies();

    for(auto& amProxy : amMap)
    {

      if(amProxy.getFlag() == Qt::Unchecked)
      {
        continue; // Skip to the next AttributeMatrix if not reading this one
      }
      hid_t amGid = QH5Utilities::createGroup(dcGid, amProxy.getName());

      AttributeMatrixProxy::StorageType& daMap = amProxy.getDataArrays();
      for(auto& daProxy : daMap)
      {
        if(daProxy.getFlag() == SIMPL::Unchecked)
        {
          continue; // Skip to the next DataArray if not reading this one
        }

        flat << daProxy.getName();
      }
      QString data = flat.join(QString('\n'));
      err = QH5Lite::writeStringDataset(amGid, QString::fromLatin1("Arrays"), data);

      H5Gclose(amGid);
    }

    H5Gclose(dcGid);
  }

  H5Gclose(dcaGid);
  return err;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int H5FilterParametersWriter::writeValue(const QString& name, const DataArrayPath& v)
{
  int err = 0;
  QString value = v.serialize();
  err = QH5Lite::writeStringDataset(m_CurrentGroupId, name, value);
  return err;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int H5FilterParametersWriter::writeValue(const QString& name, const QVector<DataArrayPath>& paths)
{
  int err = 0;
  QString pathStr;
  QTextStream ss(&pathStr);
  char sep = '\n'; // Use a new line to separate each record.
  for(const auto& path : paths)
  {
    ss << path.serialize("|") << sep;
  }
  err = QH5Lite::writeStringDataset(m_CurrentGroupId, name, pathStr);
  return err;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int H5FilterParametersWriter::writeValue(const QString& name, const DynamicTableData& v)
{
  int err = 0;

  QVector<double> flat = v.flattenData();
  QString rHeaders = v.serializeRowHeaders('|');
  QString cHeaders = v.serializeColumnHeaders('|');

  QVector<hsize_t> dims(2);
  dims[0] = v.getNumRows();
  dims[1] = v.getNumCols();

  err = QH5Lite::writeVectorDataset(m_CurrentGroupId, name, dims, flat);
  if(err < 0)
  {
    return err;
  }
  err = QH5Lite::writeStringAttribute(m_CurrentGroupId, name, "RowHeaders", rHeaders);
  if(err < 0)
  {
    return err;
  }
  err = QH5Lite::writeStringAttribute(m_CurrentGroupId, name, "ColHeaders", cHeaders);

  return err;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int H5FilterParametersWriter::writeValue(const QString& name, const QPair<double, double>& v)
{
  int err = QH5Lite::writeScalarDataset(m_CurrentGroupId, name + "_Min", v.first);
  if(err < 0)
  {
    return err;
  }
  err = QH5Lite::writeScalarDataset(m_CurrentGroupId, name + "_Max", v.second);

  return err;
}

// -----------------------------------------------------------------------------
H5FilterParametersWriter::Pointer H5FilterParametersWriter::NullPointer()
{
  return Pointer(static_cast<Self*>(nullptr));
}

// -----------------------------------------------------------------------------
H5FilterParametersWriter::Pointer H5FilterParametersWriter::New()
{
  Pointer sharedPtr(new(H5FilterParametersWriter));
  return sharedPtr;
}

// -----------------------------------------------------------------------------
QString H5FilterParametersWriter::getNameOfClass() const
{
  return QString("H5FilterParametersWriter");
}

// -----------------------------------------------------------------------------
QString H5FilterParametersWriter::ClassName()
{
  return QString("H5FilterParametersWriter");
}

// -----------------------------------------------------------------------------
void H5FilterParametersWriter::setPipelineGroupId(hid_t value)
{
  m_PipelineGroupId = value;
}

// -----------------------------------------------------------------------------
hid_t H5FilterParametersWriter::getPipelineGroupId() const
{
  return m_PipelineGroupId;
}

// -----------------------------------------------------------------------------
void H5FilterParametersWriter::setGroupId(hid_t value)
{
  m_GroupId = value;
}

// -----------------------------------------------------------------------------
hid_t H5FilterParametersWriter::getGroupId() const
{
  return m_GroupId;
}
