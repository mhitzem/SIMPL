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
#include "DataContainerWriter.h"

#include <QtCore/QDebug>
#include <QtCore/QDir>
#include <QtCore/QTextStream>

#include "H5Support/H5ScopedSentinel.h"
#include "H5Support/H5Utilities.h"
#include "H5Support/QH5Lite.h"
#include "H5Support/QH5Utilities.h"

#include "SIMPLib/SIMPLibVersion.h"
#include "SIMPLib/Common/Constants.h"
#include "SIMPLib/DataContainers/DataContainer.h"
#include "SIMPLib/DataContainers/DataContainerArray.h"
#include "SIMPLib/FilterParameters/AbstractFilterParametersReader.h"
#include "SIMPLib/FilterParameters/BooleanFilterParameter.h"
#include "SIMPLib/FilterParameters/H5FilterParametersWriter.h"
#include "SIMPLib/FilterParameters/OutputFileFilterParameter.h"
#include "SIMPLib/Utilities/FileSystemPathHelper.h"

#ifdef _WIN32
extern Q_CORE_EXPORT int qt_ntfs_permission_lookup;
#endif

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
DataContainerWriter::DataContainerWriter() = default;

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
DataContainerWriter::~DataContainerWriter() = default;

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void DataContainerWriter::setupFilterParameters()
{
  FilterParameterVectorType parameters;

  parameters.push_back(SIMPL_NEW_OUTPUT_FILE_FP("Output File", OutputFile, FilterParameter::Category::Parameter, DataContainerWriter, "*.dream3d", ""));
  parameters.push_back(SIMPL_NEW_BOOL_FP("Write Xdmf File", WriteXdmfFile, FilterParameter::Category::Parameter, DataContainerWriter));
  parameters.push_back(SIMPL_NEW_BOOL_FP("Include Xdmf Time Markers", WriteTimeSeries, FilterParameter::Category::Parameter, DataContainerWriter));

  setFilterParameters(parameters);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void DataContainerWriter::readFilterParameters(AbstractFilterParametersReader* reader, int index)
{
  reader->openFilterGroup(this, index);
  setOutputFile(reader->readString("OutputFile", getOutputFile()));
  setWriteXdmfFile(reader->readValue("WriteXdmfFile", getWriteXdmfFile()));
  reader->closeFilterGroup();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void DataContainerWriter::initialize()
{
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void DataContainerWriter::dataCheck()
{
  clearErrorCode();
  clearWarningCode();
  QString ss;

  QFileInfo fi(m_OutputFile);
  if(fi.suffix().compare("") == 0)
  {
    m_OutputFile.append(".dream3d");
  }
  FileSystemPathHelper::CheckOutputFile(this, "Output File Path", getOutputFile(), true);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void DataContainerWriter::execute()
{
  dataCheck();
  if(getErrorCode() < 0)
  {
    return;
  }

  // Make sure any directory path is also available as the user may have just typed
  // in a path without actually creating the full path
  QFileInfo fi(m_OutputFile);
  QString parentPath = fi.path();
  QDir dir;
  if(!dir.mkpath(parentPath))
  {
    QString ss = QObject::tr("Error creating parent path '%1'").arg(parentPath);
    setErrorCondition(-11110, ss);
    return;
  }

  hid_t fileId = -1;

  // Try to open a file to append data into
  if(m_AppendToExisting)
  {
    fileId = QH5Utilities::openFile(m_OutputFile, false);
  }
  // No file was found or we are writing new data only to a clean file
  if(!m_AppendToExisting || fileId < 0)
  {
    fileId = QH5Utilities::createFile(m_OutputFile);
  }

  if(fileId < 0)
  {
    QString ss = QObject::tr("The HDF5 file could not be opened or created.\n The given filename was:\n\t[%1]").arg(m_OutputFile);
    setErrorCondition(-11112, ss);
    return;
  }
  // qDebug() << "DREAM3D File: " << m_OutputFile;

  // This will make sure if we return early from this method that the HDF5 File is properly closed.
  H5ScopedFileSentinel scopedFileSentinel(fileId, true);

  // Write our File Version string to the Root "/" group
  QH5Lite::writeStringAttribute(fileId, "/", SIMPL::HDF5::FileVersionName, SIMPL::HDF5::FileVersion);
  QH5Lite::writeStringAttribute(fileId, "/", SIMPL::HDF5::DREAM3DVersion, SIMPLib::Version::Complete());
  QFile xdmfFile;
  QTextStream xdmfOut(&xdmfFile);
  if(m_WriteXdmfFile)
  {
    QFileInfo ofFi(m_OutputFile);
    QString name = ofFi.completeBaseName();
    if(parentPath.isEmpty())
    {
      name = name + ".xdmf";
    }
    else
    {
      name = parentPath + "/" + name + ".xdmf";
    }
    xdmfFile.setFileName(name);
    if(xdmfFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
      writeXdmfHeader(xdmfOut);
    }
  }

  // Write the Pipeline to the File
  int err = writePipeline();

  err = H5Utilities::createGroupsFromPath(SIMPL::StringConstants::DataContainerGroupName.toLatin1().data(), fileId);
  if(err < 0)
  {
    QString ss = QObject::tr("Error creating HDF5 Group '%1'").arg(SIMPL::StringConstants::DataContainerGroupName);
    setErrorCondition(-60, ss);
    return;
  }
  hid_t dcaGid = H5Gopen(fileId, SIMPL::StringConstants::DataContainerGroupName.toLatin1().data(), H5P_DEFAULT);
  scopedFileSentinel.addGroupId(dcaGid);

  QList<QString> dcNames = getDataContainerArray()->getDataContainerNames();
  for(int iter = 0; iter < getDataContainerArray()->getNumDataContainers(); iter++)
  {
    DataContainer::Pointer dc = getDataContainerArray()->getDataContainer(dcNames[iter]);
    IGeometry::Pointer geometry = dc->getGeometry();
    err = H5Utilities::createGroupsFromPath(dcNames[iter].toLatin1().data(), dcaGid);
    if(err < 0)
    {
      QString ss = QObject::tr("Error creating HDF5 Group '%1'").arg(dcNames[iter]);
      setErrorCondition(-60, ss);
      return;
    }

    hid_t dcGid = H5Gopen(dcaGid, dcNames[iter].toLatin1().data(), H5P_DEFAULT);
    H5ScopedGroupSentinel groupSentinel(dcGid, false);
    // QString ss = QObject::tr("Writing %2 DataContainer").arg(dcNames[iter]);

    // Have the DataContainer write all of its Attribute Matrices and its Mesh
    err = dc->writeAttributeMatricesToHDF5(dcGid);
    if(err < 0)
    {
      setErrorCondition(err, "Error writing DataContainer AttributeMatrices");
      return;
    }
    err = dc->writeMeshToHDF5(dcGid, m_WriteXdmfFile);
    if(err < 0)
    {
      setErrorCondition(err, "Error writing DataContainer Geometry");
      return;
    }
    if(m_WriteXdmfFile && geometry.get() != nullptr)
    {

      if(getWriteTimeSeries())
      {
        dc->getGeometry()->setEnableTimeSeries(true);
        dc->getGeometry()->setTimeValue(static_cast<float>(iter));
      }
#if 0
      dc->getGeometry()->addOrReplaceAttributeMatrix(SIMPL::StringConstants::MetaData, dc->getAttributeMatrix(SIMPL::StringConstants::MetaData));
      dc->getGeometry()->setTemporalDataPath(DataArrayPath(dc->getName(), SIMPL::StringConstants::MetaData, "Step #"));
#endif

      QString hdfFileName = QH5Utilities::fileNameFromFileId(fileId);
      err = dc->writeXdmf(xdmfOut, hdfFileName);
      if(err < 0)
      {
        setErrorCondition(err, "Error writing Xdmf File");
        return;
      }
    }
  }

  // Write the Data ContainerBundles
  err = writeDataContainerBundles(fileId);
  if(err < 0)
  {
    QString ss = QObject::tr("Error writing DataContainerBundles");
    setErrorCondition(-11113, ss);
    return;
  }

  // Write Montages
  err = writeMontages(fileId);
  if(err < 0)
  {
    QString ss = QObject::tr("Error writing montages");
    setErrorCondition(-11113, ss);
    return;
  }

  // Write the XDMF File
  if(m_WriteXdmfFile)
  {
    writeXdmfFooter(xdmfOut);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int DataContainerWriter::writeDataContainerBundles(hid_t fileId)
{
  int err = QH5Utilities::createGroupsFromPath(SIMPL::StringConstants::DataContainerBundleGroupName, fileId);
  if(err < 0)
  {
    QString ss = QObject::tr("Error creating HDF5 Group '%1'").arg(SIMPL::StringConstants::DataContainerBundleGroupName);
    setErrorCondition(-61, ss);
    return -1;
  }
  hid_t dcbGid = H5Gopen(fileId, SIMPL::StringConstants::DataContainerBundleGroupName.toLatin1().data(), H5P_DEFAULT);

  H5GroupAutoCloser groupCloser(dcbGid);

  QMap<QString, IDataContainerBundle::Pointer>& bundles = getDataContainerArray()->getDataContainerBundles();
  QMapIterator<QString, IDataContainerBundle::Pointer> iter(bundles);
  while(iter.hasNext())
  {
    iter.next();
    IDataContainerBundle::Pointer bundle = iter.value();
    err = bundle->writeH5Data(dcbGid);
    if(err < 0)
    {
      return err;
    }
  }
  return 0;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int DataContainerWriter::writeMontages(hid_t fileId)
{
  int err = QH5Utilities::createGroupsFromPath(SIMPL::StringConstants::MontageGroupName, fileId);
  if(err < 0)
  {
    QString ss = QObject::tr("Error creating HDF5 Group '%1'").arg(SIMPL::StringConstants::MontageGroupName);
    setErrorCondition(-62, ss);
    return -1;
  }
  hid_t dcbGid = H5Gopen(fileId, SIMPL::StringConstants::MontageGroupName.toLatin1().data(), H5P_DEFAULT);

  H5GroupAutoCloser groupCloser(dcbGid);

  DataContainerArray::MontageCollection montages = getDataContainerArray()->getMontageCollection();
  for(const auto& montage : montages)
  {
    err = montage->writeH5Data(dcbGid);
    if(err < 0)
    {
      return err;
    }
  }

  return 0;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void DataContainerWriter::writeXdmfHeader(QTextStream& xdmf)
{
  xdmf << "<?xml version=\"1.0\"?>"
       << "\n";
  xdmf << "<!DOCTYPE Xdmf SYSTEM \"Xdmf.dtd\"[]>"
       << "\n";
  xdmf << "<Xdmf xmlns:xi=\"http://www.w3.org/2003/XInclude\" Version=\"2.2\">"
       << "\n";
  xdmf << " <Domain>"
       << "\n";
  if(getWriteTimeSeries())
  {
    xdmf << "<Grid Name=\"CellTime\" GridType=\"Collection\" CollectionType=\"Temporal\">"
         << "\n";
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void DataContainerWriter::writeXdmfFooter(QTextStream& xdmf)
{
  if(getWriteTimeSeries())
  {
    xdmf << " </Grid>"
         << "\n";
  }
  xdmf << " </Domain>"
       << "\n";
  xdmf << "</Xdmf>"
       << "\n";
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int DataContainerWriter::writePipeline()
{
  // WRITE THE PIPELINE TO THE HDF5 FILE
  H5FilterParametersWriter::Pointer writer = H5FilterParametersWriter::New();

  // Now start walking BACKWARDS through the pipeline to find the first filter.
  AbstractFilter::Pointer previousFilter = getPreviousFilter().lock();
  while(previousFilter.get() != nullptr)
  {
    if(nullptr == previousFilter->getPreviousFilter().lock())
    {
      break;
    }
    previousFilter = previousFilter->getPreviousFilter().lock();
  }

  FilterPipeline::Pointer pipeline = FilterPipeline::New();

  // Now starting with the first filter in the pipeline, start the actual writing
  AbstractFilter::Pointer currentFilter = previousFilter;
  AbstractFilter::Pointer nextFilter;
  while(nullptr != currentFilter.get())
  {
    nextFilter = currentFilter->getNextFilter().lock();
    pipeline->pushBack(currentFilter);
    currentFilter = nextFilter;
  }

  return writer->writePipelineToFile(pipeline, m_OutputFile, SIMPL::StringConstants::PipelineGroupName, true);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
AbstractFilter::Pointer DataContainerWriter::newFilterInstance(bool copyFilterParameters) const
{
  DataContainerWriter::Pointer filter = DataContainerWriter::New();
  if(copyFilterParameters)
  {
    copyFilterParameterInstanceVariables(filter.get());
  }
  return filter;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString DataContainerWriter::getCompiledLibraryName() const
{
  return Core::CoreBaseName;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString DataContainerWriter::getBrandingString() const
{
  return "SIMPLib Core Filter";
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString DataContainerWriter::getFilterVersion() const
{
  QString version;
  QTextStream vStream(&version);
  vStream << SIMPLib::Version::Major() << "." << SIMPLib::Version::Minor() << "." << SIMPLib::Version::Patch();
  return version;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString DataContainerWriter::getGroupName() const
{
  return SIMPL::FilterGroups::IOFilters;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QUuid DataContainerWriter::getUuid() const
{
  return QUuid("{3fcd4c43-9d75-5b86-aad4-4441bc914f37}");
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString DataContainerWriter::getSubGroupName() const
{
  return SIMPL::FilterSubGroups::OutputFilters;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString DataContainerWriter::getHumanLabel() const
{
  return "Write DREAM.3D Data File";
}

// -----------------------------------------------------------------------------
DataContainerWriter::Pointer DataContainerWriter::NullPointer()
{
  return Pointer(static_cast<Self*>(nullptr));
}

// -----------------------------------------------------------------------------
std::shared_ptr<DataContainerWriter> DataContainerWriter::New()
{
  struct make_shared_enabler : public DataContainerWriter
  {
  };
  std::shared_ptr<make_shared_enabler> val = std::make_shared<make_shared_enabler>();
  val->setupFilterParameters();
  return val;
}

// -----------------------------------------------------------------------------
QString DataContainerWriter::getNameOfClass() const
{
  return QString("DataContainerWriter");
}

// -----------------------------------------------------------------------------
QString DataContainerWriter::ClassName()
{
  return QString("DataContainerWriter");
}

// -----------------------------------------------------------------------------
void DataContainerWriter::setOutputFile(const QString& value)
{
  m_OutputFile = value;
}

// -----------------------------------------------------------------------------
QString DataContainerWriter::getOutputFile() const
{
  return m_OutputFile;
}

// -----------------------------------------------------------------------------
void DataContainerWriter::setWritePipeline(bool value)
{
  m_WritePipeline = value;
}

// -----------------------------------------------------------------------------
bool DataContainerWriter::getWritePipeline() const
{
  return m_WritePipeline;
}

// -----------------------------------------------------------------------------
void DataContainerWriter::setWriteXdmfFile(bool value)
{
  m_WriteXdmfFile = value;
}

// -----------------------------------------------------------------------------
bool DataContainerWriter::getWriteXdmfFile() const
{
  return m_WriteXdmfFile;
}

// -----------------------------------------------------------------------------
void DataContainerWriter::setWriteTimeSeries(bool value)
{
  m_WriteTimeSeries = value;
}

// -----------------------------------------------------------------------------
bool DataContainerWriter::getWriteTimeSeries() const
{
  return m_WriteTimeSeries;
}

// -----------------------------------------------------------------------------
void DataContainerWriter::setAppendToExisting(bool value)
{
  m_AppendToExisting = value;
}

// -----------------------------------------------------------------------------
bool DataContainerWriter::getAppendToExisting() const
{
  return m_AppendToExisting;
}
