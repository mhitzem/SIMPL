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

#include <QtCore/QDir>
#include <QtCore/QEventLoop>
#include <QtCore/QFileInfo>
#include <QtCore/QJsonParseError>
#include <QtCore/QMimeDatabase>
#include <QtCore/QUrl>

#include <QtNetwork/QHostAddress>
#include <QtNetwork/QHttpMultiPart>
#include <QtNetwork/QHttpPart>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkInterface>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>

#include "QtWebApp/httpserver/ServerSettings.h"
#include "QtWebApp/httpserver/httplistener.h"
#include "QtWebApp/httpserver/httpsessionstore.h"

#include <QtCore/QDebug>
#include <QtCore/QTextStream>

#include "SIMPLib/SIMPLib.h"
#include "SIMPLib/SIMPLibVersion.h"
#include "SIMPLib/Common/QtBackwardCompatibilityMacro.h"
#include "SIMPLib/CoreFilters/CreateAttributeMatrix.h"
#include "SIMPLib/CoreFilters/CreateDataArray.h"
#include "SIMPLib/CoreFilters/CreateDataContainer.h"
#include "SIMPLib/CoreFilters/DataContainerWriter.h"
#include "SIMPLib/FilterParameters/JsonFilterParametersReader.h"
#include "SIMPLib/Filtering/FilterFactory.hpp"
#include "SIMPLib/Filtering/FilterManager.h"
#include "SIMPLib/Filtering/FilterPipeline.h"
#include "SIMPLib/Filtering/QMetaObjectUtilities.h"
#include "SIMPLib/Messages/AbstractErrorMessage.h"
#include "SIMPLib/Messages/AbstractWarningMessage.h"
#include "SIMPLib/Plugin/PluginManager.h"
#include "SIMPLib/Plugin/SIMPLPluginConstants.h"
#include "SIMPLib/Plugin/SIMPLibPluginLoader.h"
#include "SIMPLib/REST/PipelineListener.h"
#include "SIMPLib/REST/SIMPLRequestMapper.h"
#include "SIMPLib/REST/V1Controllers/SIMPLStaticFileController.h"
#include "SIMPLib/Testing/UnitTestSupport.hpp"

#include "RESTClient/SIMPLRestClient.h"

#include "SIMPLib/DataContainers/DataContainer.h"
#include "SIMPLib/DataContainers/DataContainerArray.h"
#include "SIMPLib/Testing/SIMPLTestFileLocations.h"

/**
 * @brief
 */
class RESTUnitTestObserver : public QObject
{
  Q_OBJECT

public:
  RESTUnitTestObserver() = default;

  ~RESTUnitTestObserver() override = default;

public Q_SLOTS:
  void processUploadProgress(qint64 bytesSent, qint64 bytesTotal)
  {
    double percent;
    if(bytesTotal == 0)
    {
      percent = 0;
    }
    else
    {
      percent = (static_cast<double>(bytesSent) / bytesTotal) * 100;
    }
    qDebug() << tr("Upload Progress: %1/%2 - %3%").arg(bytesSent).arg(bytesTotal).arg(percent);
  }

private:
  RESTUnitTestObserver(const RESTUnitTestObserver&); // Copy Constructor Not Implemented
  void operator=(const RESTUnitTestObserver&);       // Move assignment Not Implemented
};

#include "RESTUnitTest.moc"

/**
 * @brief The RESTUnitTest class
 */
class RESTUnitTest
{

public:
  RESTUnitTest() = default;
  virtual ~RESTUnitTest() = default;

  // -----------------------------------------------------------------------------
  //
  // -----------------------------------------------------------------------------
  void checkDREAM3DTestRequirements()
  {
    QString filtName = "EbsdToH5Ebsd";
    FilterManager* fm = FilterManager::Instance();
    IFilterFactory::Pointer filterFactory = fm->getFactoryFromClassName(filtName);
    if(nullptr == filterFactory.get())
    {
      m_RunDREAM3DTests = false;
    }
  }

  // -----------------------------------------------------------------------------
  //
  // -----------------------------------------------------------------------------
  void RemoveTestFiles()
  {
#if REMOVE_TEST_FILES
    bool success = QFile::remove(UnitTest::RestUnitTest::SmallIN100ArchiveOutputFilePath);
    success = QFile::remove(UnitTest::RestUnitTest::RESTFileIOInputDataFilePath);
    success = QFile::remove(UnitTest::RestUnitTest::RESTFileIOOutputDataFilePath);
    success = QFile::remove(UnitTest::TestTempDir + "/SmallIN100.h5ebsd");
#endif
  }

  // -----------------------------------------------------------------------------
  //
  // -----------------------------------------------------------------------------
  QUrl getConnectionURL()
  {
    QUrl url;
    for(auto address : QNetworkInterface::allAddresses())
    {
      if(address.protocol() == QAbstractSocket::IPv4Protocol && address != QHostAddress(QHostAddress::LocalHost))
      {
        url.setHost(address.toString());
        break;
      }
    }
    url.setScheme("http");
    url.setPort(8080);

    return url;
  }

  // -----------------------------------------------------------------------------
  //
  // -----------------------------------------------------------------------------
  QSharedPointer<QNetworkReply> sendRequest(QUrl url, QString contentType, QByteArray data)
  {
    QNetworkRequest netRequest(url);
    netRequest.setHeader(QNetworkRequest::ContentTypeHeader, contentType);

    QEventLoop waitLoop;
    QSharedPointer<QNetworkReply> reply = QSharedPointer<QNetworkReply>(m_Connection->post(netRequest, data));
    QObject::connect(reply.data(), SIGNAL(finished()), &waitLoop, SLOT(quit()));
    QObject::connect(reply.data(), SIGNAL(uploadProgress(qint64, qint64)), &m_Observer, SLOT(processUploadProgress(qint64, qint64)));
    waitLoop.exec();

    return reply;
  }

  // -----------------------------------------------------------------------------
  //
  // -----------------------------------------------------------------------------
  QSharedPointer<QNetworkReply> sendRequest(QUrl url, QHttpMultiPart* multiPart)
  {
    QNetworkRequest netRequest(url);

    QEventLoop waitLoop;
    QSharedPointer<QNetworkReply> reply = QSharedPointer<QNetworkReply>(m_Connection->post(netRequest, multiPart));
    multiPart->setParent(reply.data());
    QObject::connect(reply.data(), SIGNAL(finished()), &waitLoop, SLOT(quit()));
    QObject::connect(reply.data(), SIGNAL(uploadProgress(qint64, qint64)), &m_Observer, SLOT(processUploadProgress(qint64, qint64)));
    waitLoop.exec();

    return reply;
  }

  // -----------------------------------------------------------------------------
  //
  // -----------------------------------------------------------------------------
  void TestExecutePipeline()
  {
    QUrl url = getConnectionURL();

    url.setPath("/api/v1/ExecutePipeline");

    // Test 'Incorrect Content Type'
    {
      QByteArray data;

      QSharedPointer<QNetworkReply> reply = sendRequest(url, "text/plain", data);
      DREAM3D_REQUIRE_EQUAL(reply->error(), QNetworkReply::ProtocolInvalidOperationError);

      QJsonParseError jsonParseError;
      QByteArray jsonResponse = reply->readAll();
      QJsonDocument doc = QJsonDocument::fromJson(jsonResponse, &jsonParseError);
      DREAM3D_REQUIRE_EQUAL(jsonParseError.error, QJsonParseError::ParseError::NoError);

      QJsonObject responseObject = doc.object();
      DREAM3D_REQUIRE_EQUAL(responseObject.size(), 3);
      DREAM3D_REQUIRE_EQUAL(responseObject.contains(SIMPL::JSON::ErrorCode), true);
      DREAM3D_REQUIRE_EQUAL(responseObject[SIMPL::JSON::ErrorCode].toInt(), -20);
    }

    // Test 'JSON Parse Error'
    {
      QByteArray data = QByteArray::fromStdString("{ CreateAttributeMatrix");

      QSharedPointer<QNetworkReply> reply = sendRequest(url, "application/json", data);
      QString errString = reply->errorString();
      DREAM3D_REQUIRE_EQUAL(reply->error(), QNetworkReply::ProtocolInvalidOperationError);

      QJsonParseError jsonParseError;
      QByteArray jsonResponse = reply->readAll();
      QJsonDocument doc = QJsonDocument::fromJson(jsonResponse, &jsonParseError);
      DREAM3D_REQUIRE_EQUAL(jsonParseError.error, QJsonParseError::ParseError::NoError);

      QJsonObject responseObject = doc.object();
      DREAM3D_REQUIRE_EQUAL(responseObject.size(), 3);
      DREAM3D_REQUIRE_EQUAL(responseObject.contains(SIMPL::JSON::ErrorCode), true);
      DREAM3D_REQUIRE_EQUAL(responseObject[SIMPL::JSON::ErrorCode].toInt(), -40);
    }

    // Test 'Missing Pipeline Object'
    {
      QJsonObject rootObj;
      rootObj["Foo"] = "{ }";
      QJsonDocument doc(rootObj);
      QByteArray data = doc.toJson();

      QSharedPointer<QNetworkReply> reply = sendRequest(url, "application/json", data);
      DREAM3D_REQUIRE_EQUAL(reply->error(), QNetworkReply::ProtocolInvalidOperationError);

      QJsonParseError jsonParseError;
      QByteArray jsonResponse = reply->readAll();
      doc = QJsonDocument::fromJson(jsonResponse, &jsonParseError);
      DREAM3D_REQUIRE_EQUAL(jsonParseError.error, QJsonParseError::ParseError::NoError);

      QJsonObject responseObject = doc.object();
      DREAM3D_REQUIRE_EQUAL(responseObject.size(), 3);
      DREAM3D_REQUIRE_EQUAL(responseObject.contains(SIMPL::JSON::ErrorCode), true);
      DREAM3D_REQUIRE_EQUAL(responseObject[SIMPL::JSON::ErrorCode].toInt(), -50);
    }

    // Test 'Pipeline Could Not Be Created'
    {
      QJsonObject rootObj;
      rootObj[SIMPL::JSON::Pipeline] = 2;
      QJsonDocument doc(rootObj);
      QByteArray data = doc.toJson();

      QSharedPointer<QNetworkReply> reply = sendRequest(url, "application/json", data);
      DREAM3D_REQUIRE_EQUAL(reply->error(), QNetworkReply::ProtocolInvalidOperationError);

      QJsonParseError jsonParseError;
      QByteArray jsonResponse = reply->readAll();
      doc = QJsonDocument::fromJson(jsonResponse, &jsonParseError);
      DREAM3D_REQUIRE_EQUAL(jsonParseError.error, QJsonParseError::ParseError::NoError);

      QJsonObject responseObject = doc.object();
      DREAM3D_REQUIRE_EQUAL(responseObject.size(), 3);
      DREAM3D_REQUIRE_EQUAL(responseObject.contains(SIMPL::JSON::ErrorCode), true);
      DREAM3D_REQUIRE_EQUAL(responseObject[SIMPL::JSON::ErrorCode].toInt(), -50);
    }

    // Test Pipeline Execution
    {
      QFile file(UnitTest::RestUnitTest::RESTPipelineFilePath);
      DREAM3D_REQUIRE_EQUAL(file.open(QIODevice::ReadOnly), true);

      QTextStream in(&file);
      QString jsonString = in.readAll();
      QByteArray jsonByteArray = QByteArray::fromStdString(jsonString.toStdString());

      QSharedPointer<QNetworkReply> reply = sendRequest(url, "application/json", jsonByteArray);
      DREAM3D_REQUIRE_EQUAL(reply->error(), QNetworkReply::NoError);

      QJsonParseError jsonParseError;
      QByteArray jsonResponse = reply->readAll();
      QJsonDocument doc = QJsonDocument::fromJson(jsonResponse, &jsonParseError);
      DREAM3D_REQUIRE_EQUAL(jsonParseError.error, QJsonParseError::ParseError::NoError);

      QJsonObject responseObject = doc.object();
      DREAM3D_REQUIRE_EQUAL(responseObject.size(), 4);
      DREAM3D_REQUIRE_EQUAL(responseObject.contains(SIMPL::JSON::Completed), true);
      DREAM3D_REQUIRE_EQUAL(responseObject[SIMPL::JSON::Completed].isBool(), true);
      DREAM3D_REQUIRE_EQUAL(responseObject[SIMPL::JSON::Completed].toBool(), true);

      DREAM3D_REQUIRE_EQUAL(responseObject.contains(SIMPL::JSON::PipelineErrors), true);
      DREAM3D_REQUIRE_EQUAL(responseObject[SIMPL::JSON::PipelineErrors].isArray(), true);

      QJsonArray responseErrorsArray = responseObject[SIMPL::JSON::PipelineErrors].toArray();
      DREAM3D_REQUIRE_EQUAL(responseErrorsArray.size(), 0);

      DREAM3D_REQUIRE_EQUAL(responseObject[SIMPL::JSON::PipelineWarnings].isArray(), true);
      QJsonArray responseWarningsArray = responseObject[SIMPL::JSON::PipelineWarnings].toArray();
      DREAM3D_REQUIRE_EQUAL(responseWarningsArray.size(), 0);

      JsonFilterParametersReader::Pointer reader = JsonFilterParametersReader::New();
      FilterPipeline::Pointer pipeline = reader->readPipelineFromFile(UnitTest::RestUnitTest::RESTPipelineFilePath);

      PipelineListener listener(nullptr);
      pipeline->addMessageReceiver(&listener);

      pipeline->execute();

      std::vector<const AbstractWarningMessage*> warningMessages = listener.getWarningMessages();
      DREAM3D_REQUIRE_EQUAL(warningMessages.size(), 0);

      std::vector<const AbstractErrorMessage*> errorMessages = listener.getErrorMessages();
      DREAM3D_REQUIRE_EQUAL(errorMessages.size(), 0);
    }
  }

  // -----------------------------------------------------------------------------
  //
  // -----------------------------------------------------------------------------
  void TestExecutePipelineWithFolder()
  {
    QUrl url = getConnectionURL();

    url.setPath("/api/v1/ExecutePipeline");

    QDir smallIN100Dir(UnitTest::RestUnitTest::SmallIN100ArchiveInputFolderPath);
    DREAM3D_REQUIRE_EQUAL(smallIN100Dir.exists(), true);

    QStringList nameFilters;
    nameFilters.push_back("*.ang");
    QStringList fileNameList = smallIN100Dir.entryList(nameFilters, QDir::Filter::NoDotAndDotDot | QDir::Files);
    DREAM3D_REQUIRE_EQUAL(fileNameList.size(), 117);

    QStringList filePathList;
    for(QString fileName : fileNameList)
    {
      filePathList.push_back(QObject::tr("%1/%2").arg(smallIN100Dir.path()).arg(fileName));
    }

    QHttpMultiPart* multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

    QString pipelineData;
    {
      QFile inputFile(UnitTest::RestUnitTest::SmallIN100ArchivePipeline);
      DREAM3D_REQUIRE_EQUAL(inputFile.open(QIODevice::ReadOnly), true);

      QTextStream in(&inputFile);
      QString jsonString = in.readAll();
      inputFile.close();
      QByteArray jsonByteArray = QByteArray::fromStdString(jsonString.toStdString());

      QJsonDocument doc = QJsonDocument::fromJson(jsonByteArray);
      QJsonObject rootObj = doc.object();
      QJsonObject filterObj = rootObj["0"].toObject();
      filterObj["InputPath"] = UnitTest::RestUnitTest::SmallIN100ArchiveInputFolderPath;
      filterObj["OutputFile"] = UnitTest::RestUnitTest::SmallIN100ArchiveOutputFilePath;
      rootObj["0"] = filterObj;

      doc.setObject(rootObj);
      jsonByteArray = doc.toJson();

      pipelineData = QString::fromStdString(jsonByteArray.toStdString());

      QHttpPart jsonPart;
      jsonPart.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
      jsonPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"Pipeline\""));

      jsonPart.setBody(jsonByteArray);

      multiPart->append(jsonPart);
    }

    {
      QHttpPart pipelineReplacementLookupPart;
      pipelineReplacementLookupPart.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
      pipelineReplacementLookupPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"PipelineMetadata\""));

      QJsonObject rootObj;

      {
        QJsonObject filterMetadataObj;

        QJsonObject inputFileObj;
        inputFileObj["IO_Type"] = "Input";
        filterMetadataObj["InputPath"] = inputFileObj;

        QJsonObject outputFileObj;
        outputFileObj["IO_Type"] = "Output";
        filterMetadataObj["OutputFile"] = outputFileObj;

        rootObj["0"] = filterMetadataObj;
      }

      QJsonDocument doc(rootObj);

      pipelineReplacementLookupPart.setBody(doc.toJson());

      multiPart->append(pipelineReplacementLookupPart);
    }

    for(int i = 0; i < filePathList.size(); i++)
    {
      QString filePath = filePathList[i];

      QHttpPart dataPart;
      QMimeDatabase mimeDb;
      QMimeType mimeType = mimeDb.mimeTypeForFile(filePath);
      QString contentType = mimeType.name();
      dataPart.setHeader(QNetworkRequest::ContentTypeHeader, contentType);
      dataPart.setHeader(QNetworkRequest::ContentDispositionHeader, QObject::tr("form-data; name=\"%1\"").arg(filePath));
      QFile* file = new QFile(filePath);
      file->open(QIODevice::ReadOnly);
      dataPart.setBodyDevice(file);
      file->setParent(multiPart);

      multiPart->append(dataPart);
    }

    QSharedPointer<QNetworkReply> reply = sendRequest(url, multiPart);
    DREAM3D_REQUIRE_EQUAL(reply->error(), QNetworkReply::NoError);

    QList<QNetworkReply::RawHeaderPair> headerPairs = reply->rawHeaderPairs();

    bool hasContentTypeHeader = false;
    for(QNetworkReply::RawHeaderPair headerPair : headerPairs)
    {
      if(headerPair.first == "Content-Type")
      {
        DREAM3D_REQUIRE_EQUAL(headerPair.second.startsWith("multipart/form-data"), true);
        DREAM3D_REQUIRE_EQUAL(headerPair.second.contains("boundary=\""), true);
        hasContentTypeHeader = true;
      }
    }

    DREAM3D_REQUIRE_EQUAL(hasContentTypeHeader, true);

    QString boundary = "@@@@@@@@@@@@@@@@@@@@";

    QString bodyData = QString::fromStdString(reply->readAll().toStdString());

    QStringList bodyPartDataList = bodyData.split(boundary + "\r\n", QSTRING_SKIP_EMPTY_PARTS);
    DREAM3D_REQUIRE_EQUAL(bodyPartDataList.size(), 2);

    for(int i = 0; i < bodyPartDataList.size(); i++)
    {
      QString bodyPartData = bodyPartDataList[i];
      QTextStream textStream(&bodyPartData);

      QString line = textStream.readLine();
      DREAM3D_REQUIRE_EQUAL(line.startsWith("Content-Disposition:"), true);
      DREAM3D_REQUIRE_EQUAL(line.contains("form-data"), true);
      DREAM3D_REQUIRE_EQUAL(line.contains("name="), true);

      QString parameterNameLabel = "name=";

      int nameLabelIndex = line.indexOf(parameterNameLabel);
      QString bodyPartName = line.right(line.size() - nameLabelIndex - parameterNameLabel.size());
      if(bodyPartName.startsWith("\""))
      {
        bodyPartName.remove(0, 1);
      }
      if(bodyPartName.endsWith("\""))
      {
        bodyPartName.chop(1);
      }

      // Read empty line that separates part-specific headers with the part body
      textStream.readLine();

      // Read the part body
      QString bodyPartValue = textStream.read(bodyPartData.size());

      if(bodyPartName == "pipelineResponse")
      {
        QJsonParseError jsonParseError;
        QJsonDocument doc = QJsonDocument::fromJson(QByteArray::fromStdString(bodyPartValue.toStdString()), &jsonParseError);
        DREAM3D_REQUIRE_EQUAL(jsonParseError.error, QJsonParseError::ParseError::NoError);

        QJsonObject responseObject = doc.object();
        DREAM3D_REQUIRE_EQUAL(responseObject.size(), 4);
        DREAM3D_REQUIRE_EQUAL(responseObject.contains(SIMPL::JSON::Completed), true);
        DREAM3D_REQUIRE_EQUAL(responseObject[SIMPL::JSON::Completed].isBool(), true);
        DREAM3D_REQUIRE_EQUAL(responseObject[SIMPL::JSON::Completed].toBool(), true);

        DREAM3D_REQUIRE_EQUAL(responseObject.contains(SIMPL::JSON::PipelineErrors), true);
        DREAM3D_REQUIRE_EQUAL(responseObject[SIMPL::JSON::PipelineErrors].isArray(), true);

        QJsonArray responseErrorsArray = responseObject[SIMPL::JSON::PipelineErrors].toArray();
        DREAM3D_REQUIRE_EQUAL(responseErrorsArray.size(), 0);

        DREAM3D_REQUIRE_EQUAL(responseObject[SIMPL::JSON::PipelineWarnings].isArray(), true);
        QJsonArray responseWarningsArray = responseObject[SIMPL::JSON::PipelineWarnings].toArray();
        DREAM3D_REQUIRE_EQUAL(responseWarningsArray.size(), 0);

        JsonFilterParametersReader::Pointer reader = JsonFilterParametersReader::New();
        FilterPipeline::Pointer pipeline = reader->readPipelineFromString(pipelineData);

        PipelineListener listener(nullptr);
        pipeline->addMessageReceiver(&listener);

        pipeline->execute();

        std::vector<const AbstractWarningMessage*> warningMessages = listener.getWarningMessages();
        DREAM3D_REQUIRE_EQUAL(warningMessages.size(), 0);

        std::vector<const AbstractErrorMessage*> errorMessages = listener.getErrorMessages();
        DREAM3D_REQUIRE_EQUAL(errorMessages.size(), 0);
      }
      else
      {
        QDir dir;
        QFileInfo fi(bodyPartName);
        DREAM3D_REQUIRE_EQUAL(dir.mkpath(fi.path()), true);

        QFile file(bodyPartName);
        if(file.exists())
        {
          file.remove();
        }

        DREAM3D_REQUIRE_EQUAL(file.open(QFile::WriteOnly), true);

        QByteArray fileData = QByteArray::fromStdString(bodyPartValue.toStdString());
        fileData = QByteArray::fromBase64(fileData);

        file.write(fileData);
        file.close();
      }
    }
  }

  // -----------------------------------------------------------------------------
  //
  // -----------------------------------------------------------------------------
  void prepInputDataFile()
  {
    FilterPipeline::Pointer pipeline = FilterPipeline::New();

    CreateDataContainer::Pointer createDataContainer = CreateDataContainer::New();
    createDataContainer->setDataContainerName(DataArrayPath("DataContainer", "", ""));
    pipeline->pushBack(createDataContainer);

    CreateAttributeMatrix::Pointer createAttrMat = CreateAttributeMatrix::New();
    createAttrMat->setAttributeMatrixType(3);
    DataArrayPath dap("DataContainer", "AttributeMatrix", "");
    createAttrMat->setCreatedAttributeMatrix(dap);
    DynamicTableData dtd;
    dtd.setTableData({{10.0}});
    createAttrMat->setTupleDimensions(dtd);
    pipeline->pushBack(createAttrMat);

    CreateDataArray::Pointer createDataArray = CreateDataArray::New();
    createDataArray->setInitializationType(0);
    createDataArray->setInitializationValue("0");
    dap.setDataArrayName("DataArray");
    createDataArray->setNewArray(dap);
    createDataArray->setNumberOfComponents(1);
    createDataArray->setScalarType(SIMPL::ScalarTypes::Type::Int8);
    pipeline->pushBack(createDataArray);

    DataContainerWriter::Pointer writer = DataContainerWriter::New();
    writer->setOutputFile(UnitTest::RestUnitTest::RESTFileIOInputDataFilePath);
    writer->setWriteXdmfFile(false);
    pipeline->pushBack(writer);

    DataContainerArray::Pointer dca = pipeline->execute();
    int32_t err = pipeline->getErrorCode();
    DREAM3D_REQUIRE(err >= 0);
  }
  // -----------------------------------------------------------------------------
  //
  // -----------------------------------------------------------------------------
  void TestExecutePipelineWithFiles()
  {

    prepInputDataFile();

    QUrl url = getConnectionURL();

    url.setPath("/api/v1/ExecutePipeline");

    QStringList filePathList;
    filePathList.push_back(UnitTest::RestUnitTest::RESTFileIOInputDataFilePath);

    QHttpMultiPart* multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

    {
      QFile inputFile(UnitTest::RestUnitTest::RESTFileIOPipelineFilePath);
      DREAM3D_REQUIRE_EQUAL(inputFile.open(QIODevice::ReadOnly), true);

      QTextStream in(&inputFile);
      QString jsonString = in.readAll();
      inputFile.close();
      QByteArray jsonByteArray = QByteArray::fromStdString(jsonString.toStdString());

      QJsonDocument doc = QJsonDocument::fromJson(jsonByteArray);
      QJsonObject pipelineObj = doc.object();
      QJsonObject filterObj = pipelineObj["0"].toObject();
      filterObj["InputFile"] = UnitTest::RestUnitTest::RESTFileIOInputDataFilePath;
      pipelineObj["0"] = filterObj;

      filterObj = pipelineObj["1"].toObject();
      filterObj["OutputFile"] = UnitTest::RestUnitTest::RESTFileIOOutputDataFilePath;
      pipelineObj["1"] = filterObj;

      doc.setObject(pipelineObj);
      jsonByteArray = doc.toJson();

      QHttpPart jsonPart;
      jsonPart.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
      jsonPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"Pipeline\""));

      jsonPart.setBody(jsonByteArray);

      multiPart->append(jsonPart);
    }

    {
      QHttpPart pipelineReplacementLookupPart;
      pipelineReplacementLookupPart.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
      pipelineReplacementLookupPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"PipelineMetadata\""));

      QJsonObject rootObj;

      {
        QJsonObject filterMetadataObj;
        QJsonObject inputFileObj;
        inputFileObj["IO_Type"] = "Input";

        filterMetadataObj["InputFile"] = inputFileObj;

        rootObj["0"] = filterMetadataObj;
      }
      {
        QJsonObject filterMetadataObj;
        QJsonObject outputFileObj;
        outputFileObj["IO_Type"] = "Output";

        filterMetadataObj["OutputFile"] = outputFileObj;

        rootObj["1"] = filterMetadataObj;
      }

      QJsonDocument doc(rootObj);

      pipelineReplacementLookupPart.setBody(doc.toJson());

      multiPart->append(pipelineReplacementLookupPart);
    }

    for(int i = 0; i < filePathList.size(); i++)
    {
      QString filePath = filePathList[i];

      QHttpPart dataPart;
      QMimeDatabase mimeDb;
      QMimeType mimeType = mimeDb.mimeTypeForFile(filePath);
      QString contentType = mimeType.name();
      dataPart.setHeader(QNetworkRequest::ContentTypeHeader, contentType);
      dataPart.setHeader(QNetworkRequest::ContentDispositionHeader, QObject::tr("form-data; name=\"%1\"").arg(filePath));
      QFile* file = new QFile(filePath);
      file->open(QIODevice::ReadOnly);
      dataPart.setBodyDevice(file);
      file->setParent(multiPart);

      multiPart->append(dataPart);
    }

    QSharedPointer<QNetworkReply> reply = sendRequest(url, multiPart);
    DREAM3D_REQUIRE_EQUAL(reply->error(), QNetworkReply::NoError);

    QList<QNetworkReply::RawHeaderPair> headerPairs = reply->rawHeaderPairs();

    bool hasContentTypeHeader = false;
    for(QNetworkReply::RawHeaderPair headerPair : headerPairs)
    {
      if(headerPair.first == "Content-Type")
      {
        DREAM3D_REQUIRE_EQUAL(headerPair.second.startsWith("multipart/form-data"), true);
        DREAM3D_REQUIRE_EQUAL(headerPair.second.contains("boundary=\""), true);
        hasContentTypeHeader = true;
      }
    }

    DREAM3D_REQUIRE_EQUAL(hasContentTypeHeader, true);

    QString boundary = "@@@@@@@@@@@@@@@@@@@@";

    QString bodyData = QString::fromStdString(reply->readAll().toStdString());

    QStringList bodyPartDataList = bodyData.split(boundary + "\r\n", QSTRING_SKIP_EMPTY_PARTS);
    DREAM3D_REQUIRE_EQUAL(bodyPartDataList.size(), 2);

    for(int i = 0; i < bodyPartDataList.size(); i++)
    {
      QString bodyPartData = bodyPartDataList[i];
      QTextStream textStream(&bodyPartData);

      QString line = textStream.readLine();
      DREAM3D_REQUIRE_EQUAL(line.startsWith("Content-Disposition:"), true);
      DREAM3D_REQUIRE_EQUAL(line.contains("form-data"), true);
      DREAM3D_REQUIRE_EQUAL(line.contains("name="), true);

      QString parameterNameLabel = "name=";

      int nameLabelIndex = line.indexOf(parameterNameLabel);
      QString bodyPartName = line.right(line.size() - nameLabelIndex - parameterNameLabel.size());
      if(bodyPartName.startsWith("\""))
      {
        bodyPartName.remove(0, 1);
      }
      if(bodyPartName.endsWith("\""))
      {
        bodyPartName.chop(1);
      }

      // Read empty line that separates part-specific headers with the part body
      textStream.readLine();

      // Read the part body
      QString bodyPartValue = textStream.read(bodyPartData.size());

      if(bodyPartName == "pipelineResponse")
      {
        QJsonParseError jsonParseError;
        QJsonDocument doc = QJsonDocument::fromJson(QByteArray::fromStdString(bodyPartValue.toStdString()), &jsonParseError);
        DREAM3D_REQUIRE_EQUAL(jsonParseError.error, QJsonParseError::ParseError::NoError);

        QJsonObject responseObject = doc.object();
        DREAM3D_REQUIRE_EQUAL(responseObject.size(), 4);
        DREAM3D_REQUIRE_EQUAL(responseObject.contains(SIMPL::JSON::Completed), true);
        DREAM3D_REQUIRE_EQUAL(responseObject[SIMPL::JSON::Completed].isBool(), true);
        DREAM3D_REQUIRE_EQUAL(responseObject[SIMPL::JSON::Completed].toBool(), true);

        DREAM3D_REQUIRE_EQUAL(responseObject.contains(SIMPL::JSON::PipelineErrors), true);
        DREAM3D_REQUIRE_EQUAL(responseObject[SIMPL::JSON::PipelineErrors].isArray(), true);

        QJsonArray responseErrorsArray = responseObject[SIMPL::JSON::PipelineErrors].toArray();
        DREAM3D_REQUIRE_EQUAL(responseErrorsArray.size(), 0);

        DREAM3D_REQUIRE_EQUAL(responseObject[SIMPL::JSON::PipelineWarnings].isArray(), true);
        QJsonArray responseWarningsArray = responseObject[SIMPL::JSON::PipelineWarnings].toArray();
        DREAM3D_REQUIRE_EQUAL(responseWarningsArray.size(), 0);

        QFile inputFile(UnitTest::RestUnitTest::RESTFileIOPipelineFilePath);
        DREAM3D_REQUIRE_EQUAL(inputFile.open(QIODevice::ReadOnly), true);

        QTextStream in(&inputFile);
        QString jsonString = in.readAll();
        inputFile.close();
        QByteArray jsonByteArray = QByteArray::fromStdString(jsonString.toStdString());

        doc = QJsonDocument::fromJson(jsonByteArray);
        QJsonObject pipelineObj = doc.object();
        QJsonObject filterObj = pipelineObj["0"].toObject();
        filterObj["InputFile"] = UnitTest::RestUnitTest::RESTFileIOInputDataFilePath;
        pipelineObj["0"] = filterObj;

        filterObj = pipelineObj["1"].toObject();
        filterObj["OutputFile"] = UnitTest::RestUnitTest::RESTFileIOOutputDataFilePath;
        pipelineObj["1"] = filterObj;

        doc.setObject(pipelineObj);
        jsonByteArray = doc.toJson();

        JsonFilterParametersReader::Pointer reader = JsonFilterParametersReader::New();
        FilterPipeline::Pointer pipeline = reader->readPipelineFromString(QString::fromStdString(jsonByteArray.toStdString()));

        PipelineListener listener(nullptr);
        pipeline->addMessageReceiver(&listener);

        pipeline->execute();

        std::vector<const AbstractWarningMessage*> warningMessages = listener.getWarningMessages();
        DREAM3D_REQUIRE_EQUAL(warningMessages.size(), 0);

        std::vector<const AbstractErrorMessage*> errorMessages = listener.getErrorMessages();
        DREAM3D_REQUIRE_EQUAL(errorMessages.size(), 0);
      }
      else
      {
        QDir dir;
        QFileInfo fi(bodyPartName);
        DREAM3D_REQUIRE_EQUAL(dir.mkpath(fi.path()), true);

        QFile file(bodyPartName);
        DREAM3D_REQUIRE_EQUAL(file.open(QFile::WriteOnly), true);

        QByteArray fileData = QByteArray::fromStdString(bodyPartValue.toStdString());
        fileData = QByteArray::fromBase64(fileData);

        file.write(fileData);
        file.close();
      }
    }
  }

  // -----------------------------------------------------------------------------
  //
  // -----------------------------------------------------------------------------
  void TestListFilterParameters()
  {
    QUrl url = getConnectionURL();

    url.setPath("/api/v1/ListFilterParameters");

    // Test 'Incorrect Content Type'
    {
      QByteArray data;

      QSharedPointer<QNetworkReply> reply = sendRequest(url, "text/plain", data);
      DREAM3D_REQUIRE_EQUAL(reply->error(), 0);

      QJsonParseError jsonParseError;
      QByteArray jsonResponse = reply->readAll();
      QJsonDocument doc = QJsonDocument::fromJson(jsonResponse, &jsonParseError);
      DREAM3D_REQUIRE_EQUAL(jsonParseError.error, QJsonParseError::ParseError::NoError);

      QJsonObject responseObject = doc.object();
      DREAM3D_REQUIRE_EQUAL(responseObject.size(), 2);
      DREAM3D_REQUIRE_EQUAL(responseObject.contains(SIMPL::JSON::ErrorCode), true);
      DREAM3D_REQUIRE_EQUAL(responseObject[SIMPL::JSON::ErrorCode].toInt(), -20);
    }

    // Test 'JSON Parse Error'
    {
      QByteArray data = QByteArray::fromStdString("{ CreateAttributeMatrix");

      QSharedPointer<QNetworkReply> reply = sendRequest(url, "application/json", data);
      DREAM3D_REQUIRE_EQUAL(reply->error(), 0);

      QJsonParseError jsonParseError;
      QByteArray jsonResponse = reply->readAll();
      QJsonDocument doc = QJsonDocument::fromJson(jsonResponse, &jsonParseError);
      DREAM3D_REQUIRE_EQUAL(jsonParseError.error, QJsonParseError::ParseError::NoError);

      QJsonObject responseObject = doc.object();
      DREAM3D_REQUIRE_EQUAL(responseObject.size(), 2);
      DREAM3D_REQUIRE_EQUAL(responseObject.contains(SIMPL::JSON::ErrorCode), true);
      DREAM3D_REQUIRE_EQUAL(responseObject[SIMPL::JSON::ErrorCode].toInt(), -30);
    }

    // Test 'Missing Filter Class Name Object'
    {
      QJsonObject rootObj;
      rootObj["Foo"] = "CreateAttributeMatrix";
      QJsonDocument doc(rootObj);
      QByteArray data = doc.toJson();

      QSharedPointer<QNetworkReply> reply = sendRequest(url, "application/json", data);
      DREAM3D_REQUIRE_EQUAL(reply->error(), 0);

      QJsonParseError jsonParseError;
      QByteArray jsonResponse = reply->readAll();
      doc = QJsonDocument::fromJson(jsonResponse, &jsonParseError);
      DREAM3D_REQUIRE_EQUAL(jsonParseError.error, QJsonParseError::ParseError::NoError);

      QJsonObject responseObject = doc.object();
      DREAM3D_REQUIRE_EQUAL(responseObject.size(), 2);
      DREAM3D_REQUIRE_EQUAL(responseObject.contains(SIMPL::JSON::ErrorCode), true);
      DREAM3D_REQUIRE_EQUAL(responseObject[SIMPL::JSON::ErrorCode].toInt(), -40);
    }

    // Test 'Class Name Object Not a String'
    {
      QJsonObject rootObj;
      rootObj[SIMPL::JSON::ClassName] = 2;
      QJsonDocument doc(rootObj);
      QByteArray data = doc.toJson();

      QSharedPointer<QNetworkReply> reply = sendRequest(url, "application/json", data);
      DREAM3D_REQUIRE_EQUAL(reply->error(), 0);

      QJsonParseError jsonParseError;
      QByteArray jsonResponse = reply->readAll();
      doc = QJsonDocument::fromJson(jsonResponse, &jsonParseError);
      DREAM3D_REQUIRE_EQUAL(jsonParseError.error, QJsonParseError::ParseError::NoError);

      QJsonObject responseObject = doc.object();
      DREAM3D_REQUIRE_EQUAL(responseObject.size(), 2);
      DREAM3D_REQUIRE_EQUAL(responseObject.contains(SIMPL::JSON::ErrorCode), true);
      DREAM3D_REQUIRE_EQUAL(responseObject[SIMPL::JSON::ErrorCode].toInt(), -50);
    }

    // Test 'Empty Class Name'
    {
      QJsonObject rootObj;
      rootObj[SIMPL::JSON::ClassName] = "";
      QJsonDocument doc(rootObj);
      QByteArray data = doc.toJson();

      QSharedPointer<QNetworkReply> reply = sendRequest(url, "application/json", data);
      DREAM3D_REQUIRE_EQUAL(reply->error(), 0);

      QJsonParseError jsonParseError;
      QByteArray jsonResponse = reply->readAll();
      doc = QJsonDocument::fromJson(jsonResponse, &jsonParseError);
      DREAM3D_REQUIRE_EQUAL(jsonParseError.error, QJsonParseError::ParseError::NoError);

      QJsonObject responseObject = doc.object();
      DREAM3D_REQUIRE_EQUAL(responseObject.size(), 2);
      DREAM3D_REQUIRE_EQUAL(responseObject.contains(SIMPL::JSON::ErrorCode), true);
      DREAM3D_REQUIRE_EQUAL(responseObject[SIMPL::JSON::ErrorCode].toInt(), -60);
    }

    // Test 'Unidentified Filter Name'
    {
      QJsonObject rootObj;
      rootObj[SIMPL::JSON::ClassName] = "FooBarBaz";
      QJsonDocument doc(rootObj);
      QByteArray data = doc.toJson();

      QSharedPointer<QNetworkReply> reply = sendRequest(url, "application/json", data);
      DREAM3D_REQUIRE_EQUAL(reply->error(), 0);

      QJsonParseError jsonParseError;
      QByteArray jsonResponse = reply->readAll();
      doc = QJsonDocument::fromJson(jsonResponse, &jsonParseError);
      DREAM3D_REQUIRE_EQUAL(jsonParseError.error, QJsonParseError::ParseError::NoError);

      QJsonObject responseObject = doc.object();
      DREAM3D_REQUIRE_EQUAL(responseObject.size(), 2);
      DREAM3D_REQUIRE_EQUAL(responseObject.contains(SIMPL::JSON::ErrorCode), true);
      DREAM3D_REQUIRE_EQUAL(responseObject[SIMPL::JSON::ErrorCode].toInt(), -70);
    }

    // Test correct outcome
    {
      QJsonObject rootObj;
      rootObj[SIMPL::JSON::ClassName] = "CreateAttributeMatrix";
      QJsonDocument doc(rootObj);
      QByteArray data = doc.toJson();

      QSharedPointer<QNetworkReply> reply = sendRequest(url, "application/json", data);
      DREAM3D_REQUIRE_EQUAL(reply->error(), 0);

      QJsonParseError jsonParseError;
      QByteArray jsonResponse = reply->readAll();
      doc = QJsonDocument::fromJson(jsonResponse, &jsonParseError);
      DREAM3D_REQUIRE_EQUAL(jsonParseError.error, QJsonParseError::ParseError::NoError);

      QJsonObject responseObject = doc.object();
      DREAM3D_REQUIRE_EQUAL(responseObject.size(), 4);
      DREAM3D_REQUIRE_EQUAL(responseObject.contains(SIMPL::JSON::ErrorCode), true);
      DREAM3D_REQUIRE_EQUAL(responseObject[SIMPL::JSON::ErrorCode].toInt(), 0);

      DREAM3D_REQUIRE_EQUAL(responseObject.contains(SIMPL::JSON::FilterParameters), true);
      DREAM3D_REQUIRE_EQUAL(responseObject[SIMPL::JSON::FilterParameters].isArray(), true);

      QJsonArray responseFPArray = responseObject[SIMPL::JSON::FilterParameters].toArray();

      CreateAttributeMatrix::Pointer filter = CreateAttributeMatrix::New();
      FilterParameterVectorType parameters = filter->getFilterParameters();

      DREAM3D_REQUIRE_EQUAL(responseFPArray.size(), parameters.size());

      for(int i = 0; i < responseFPArray.size(); i++)
      {
        DREAM3D_REQUIRE_EQUAL(responseFPArray[i].isObject(), true);

        QJsonObject responseFPObject = responseFPArray[i].toObject();

        DREAM3D_REQUIRE_EQUAL(responseFPObject.contains(SIMPL::JSON::FilterParameterName), true);
        DREAM3D_REQUIRE_EQUAL(responseFPObject[SIMPL::JSON::FilterParameterName].isString(), true);
        DREAM3D_REQUIRE_EQUAL(responseFPObject[SIMPL::JSON::FilterParameterName].toString(), parameters[i]->getNameOfClass());

        DREAM3D_REQUIRE_EQUAL(responseFPObject.contains(SIMPL::JSON::FilterParameterWidget), true);
        DREAM3D_REQUIRE_EQUAL(responseFPObject[SIMPL::JSON::FilterParameterWidget].isString(), true);
        DREAM3D_REQUIRE_EQUAL(responseFPObject[SIMPL::JSON::FilterParameterWidget].toString(), parameters[i]->getWidgetType());

        DREAM3D_REQUIRE_EQUAL(responseFPObject.contains(SIMPL::JSON::FilterParameterCategory), true);
        DREAM3D_REQUIRE_EQUAL(responseFPObject[SIMPL::JSON::FilterParameterCategory].isDouble(), true);
        DREAM3D_REQUIRE_EQUAL(responseFPObject[SIMPL::JSON::FilterParameterCategory].toInt(), static_cast<int32_t>(parameters[i]->getCategory()));

        DREAM3D_REQUIRE_EQUAL(responseFPObject.contains(SIMPL::JSON::FilterParameterGroupIndex), true);
        DREAM3D_REQUIRE_EQUAL(responseFPObject[SIMPL::JSON::FilterParameterGroupIndex].isDouble(), true);
        DREAM3D_REQUIRE_EQUAL(responseFPObject[SIMPL::JSON::FilterParameterGroupIndex].toInt(), parameters[i]->getGroupIndex());

        DREAM3D_REQUIRE_EQUAL(responseFPObject.contains(SIMPL::JSON::FilterParameterHumanLabel), true);
        DREAM3D_REQUIRE_EQUAL(responseFPObject[SIMPL::JSON::FilterParameterHumanLabel].isString(), true);
        DREAM3D_REQUIRE_EQUAL(responseFPObject[SIMPL::JSON::FilterParameterHumanLabel].toString(), parameters[i]->getHumanLabel());

        DREAM3D_REQUIRE_EQUAL(responseFPObject.contains(SIMPL::JSON::FilterParameterPropertyName), true);
        DREAM3D_REQUIRE_EQUAL(responseFPObject[SIMPL::JSON::FilterParameterPropertyName].isString(), true);
        DREAM3D_REQUIRE_EQUAL(responseFPObject[SIMPL::JSON::FilterParameterPropertyName].toString(), parameters[i]->getPropertyName());

        DREAM3D_REQUIRE_EQUAL(responseFPObject.contains(SIMPL::JSON::FilterParameterReadOnly), true);
        DREAM3D_REQUIRE_EQUAL(responseFPObject[SIMPL::JSON::FilterParameterReadOnly].isBool(), true);
        DREAM3D_REQUIRE_EQUAL(responseFPObject[SIMPL::JSON::FilterParameterReadOnly].toBool(), parameters[i]->getReadOnly());
      }
    }
  }

  // -----------------------------------------------------------------------------
  //
  // -----------------------------------------------------------------------------
  void TestLoadedPlugins()
  {
    QUrl url = getConnectionURL();

    url.setPath("/api/v1/LoadedPlugins");

    QByteArray data; // No actual Application data is required.

    // Test incorrect content type
    {
      QSharedPointer<QNetworkReply> reply = sendRequest(url, "text/plain", data);
      DREAM3D_REQUIRE_EQUAL(reply->error(), 0);

      QJsonParseError jsonParseError;
      QByteArray jsonResponse = reply->readAll();
      QJsonDocument doc = QJsonDocument::fromJson(jsonResponse, &jsonParseError);
      DREAM3D_REQUIRE_EQUAL(jsonParseError.error, QJsonParseError::ParseError::NoError);

      QJsonObject responseObject = doc.object();
      DREAM3D_REQUIRE_EQUAL(responseObject.size(), 2);
      DREAM3D_REQUIRE_EQUAL(responseObject.contains(SIMPL::JSON::ErrorCode), true);
      DREAM3D_REQUIRE_EQUAL(responseObject[SIMPL::JSON::ErrorCode].toInt(), -20);
    }

    {
      QSharedPointer<QNetworkReply> reply = sendRequest(url, "application/json", data);
      DREAM3D_REQUIRE_EQUAL(reply->error(), 0);

      QJsonParseError jsonParseError;
      QByteArray jsonResponse = reply->readAll();
      QJsonDocument doc = QJsonDocument::fromJson(jsonResponse, &jsonParseError);
      DREAM3D_REQUIRE_EQUAL(jsonParseError.error, QJsonParseError::ParseError::NoError);

      QJsonObject responseObject = doc.object();
      DREAM3D_REQUIRE_EQUAL(responseObject.size(), 3);
      DREAM3D_REQUIRE_EQUAL(responseObject.contains(SIMPL::JSON::ErrorCode), true);
      DREAM3D_REQUIRE_EQUAL(responseObject[SIMPL::JSON::ErrorCode].toInt(), 0);
      DREAM3D_REQUIRE_EQUAL(responseObject.contains(SIMPL::JSON::Plugins), true);
      DREAM3D_REQUIRE_EQUAL(responseObject[SIMPL::JSON::Plugins].isArray(), true);

      QJsonArray responsePluginsArray = responseObject[SIMPL::JSON::Plugins].toArray();

      PluginManager* pluginManager = PluginManager::Instance();
      QJsonArray pluginManagerArray = pluginManager->toJsonArray();
      DREAM3D_REQUIRE_EQUAL(pluginManagerArray.size(), responsePluginsArray.size());

      for(int i = 0; i < responsePluginsArray.size(); i++)
      {
        DREAM3D_REQUIRE_EQUAL(responsePluginsArray[i].isObject(), true);

        QJsonObject responsePluginObject = responsePluginsArray[i].toObject();
        DREAM3D_REQUIRE_EQUAL(pluginManagerArray.contains(responsePluginObject), true);
      }
    }
  }

  // -----------------------------------------------------------------------------
  //
  // -----------------------------------------------------------------------------
  void TestNamesOfFilters()
  {
    QUrl url = getConnectionURL();

    url.setPath("/api/v1/AvailableFilters");

    QByteArray data; // No actual Application data is required.

    // Test incorrect content type
    {
      QSharedPointer<QNetworkReply> reply = sendRequest(url, "text/plain", data);
      DREAM3D_REQUIRE_EQUAL(reply->error(), 0);

      QJsonParseError jsonParseError;
      QByteArray jsonResponse = reply->readAll();
      QJsonDocument doc = QJsonDocument::fromJson(jsonResponse, &jsonParseError);
      DREAM3D_REQUIRE_EQUAL(jsonParseError.error, QJsonParseError::ParseError::NoError);

      QJsonObject responseObject = doc.object();
      DREAM3D_REQUIRE_EQUAL(responseObject.size(), 2);
      DREAM3D_REQUIRE_EQUAL(responseObject.contains(SIMPL::JSON::ErrorCode), true);
      DREAM3D_REQUIRE_EQUAL(responseObject[SIMPL::JSON::ErrorCode].toInt(), -20);
    }

    {
      QSharedPointer<QNetworkReply> reply = sendRequest(url, "application/json", data);
      DREAM3D_REQUIRE_EQUAL(reply->error(), 0);

      QJsonParseError jsonParseError;
      QByteArray jsonResponse = reply->readAll();
      QJsonDocument doc = QJsonDocument::fromJson(jsonResponse, &jsonParseError);
      DREAM3D_REQUIRE_EQUAL(jsonParseError.error, QJsonParseError::ParseError::NoError);

      QJsonObject responseObject = doc.object();
      DREAM3D_REQUIRE_EQUAL(responseObject.size(), 3);
      DREAM3D_REQUIRE_EQUAL(responseObject.contains(SIMPL::JSON::ErrorCode), true);
      DREAM3D_REQUIRE_EQUAL(responseObject[SIMPL::JSON::ErrorCode].toInt(), 0);
      DREAM3D_REQUIRE_EQUAL(responseObject.contains(SIMPL::JSON::Filters), true);
      DREAM3D_REQUIRE_EQUAL(responseObject[SIMPL::JSON::Filters].isArray(), true);

      QJsonArray responseFiltersArray = responseObject[SIMPL::JSON::Filters].toArray();

      FilterManager* fm = FilterManager::Instance();
      QJsonArray filterManagerArray = fm->toJsonArray();
      DREAM3D_REQUIRE_EQUAL(filterManagerArray.size(), responseFiltersArray.size());

      for(int i = 0; i < responseFiltersArray.size(); i++)
      {
        DREAM3D_REQUIRE_EQUAL(responseFiltersArray[i].isObject(), true);

        QJsonObject responseFilterObject = responseFiltersArray[i].toObject();
        DREAM3D_REQUIRE_EQUAL(filterManagerArray.contains(responseFilterObject), true);
      }
    }
  }

  // -----------------------------------------------------------------------------
  //
  // -----------------------------------------------------------------------------
  void TestNumFilters()
  {
    QUrl url = getConnectionURL();

    url.setPath("/api/v1/NumFilters");

    QByteArray data; // No actual Application data is required.

    // Test incorrect content type
    {
      QSharedPointer<QNetworkReply> reply = sendRequest(url, "text/plain", data);
      DREAM3D_REQUIRE_EQUAL(reply->error(), 0);

      QJsonParseError jsonParseError;
      QByteArray jsonResponse = reply->readAll();
      QJsonDocument doc = QJsonDocument::fromJson(jsonResponse, &jsonParseError);
      DREAM3D_REQUIRE_EQUAL(jsonParseError.error, QJsonParseError::ParseError::NoError);

      QJsonObject responseObject = doc.object();
      DREAM3D_REQUIRE_EQUAL(responseObject.size(), 2);
      DREAM3D_REQUIRE_EQUAL(responseObject.contains(SIMPL::JSON::ErrorCode), true);
      DREAM3D_REQUIRE_EQUAL(responseObject[SIMPL::JSON::ErrorCode].toInt(), -20);
    }

    {
      QSharedPointer<QNetworkReply> reply = sendRequest(url, "application/json", data);
      DREAM3D_REQUIRE_EQUAL(reply->error(), 0);

      QJsonParseError jsonParseError;
      QByteArray jsonResponse = reply->readAll();
      QJsonDocument doc = QJsonDocument::fromJson(jsonResponse, &jsonParseError);
      DREAM3D_REQUIRE_EQUAL(jsonParseError.error, QJsonParseError::ParseError::NoError);

      QJsonObject responseObject = doc.object();
      DREAM3D_REQUIRE_EQUAL(responseObject.size(), 3);
      DREAM3D_REQUIRE_EQUAL(responseObject.contains(SIMPL::JSON::ErrorCode), true);
      DREAM3D_REQUIRE_EQUAL(responseObject[SIMPL::JSON::ErrorCode].toInt(), 0);
      DREAM3D_REQUIRE_EQUAL(responseObject.contains(SIMPL::JSON::NumFilters), true);
      DREAM3D_REQUIRE_EQUAL(responseObject[SIMPL::JSON::NumFilters].isDouble(), true);

      int responseNumFilters = responseObject[SIMPL::JSON::NumFilters].toInt();

      FilterManager* fm = FilterManager::Instance();
      FilterManager::Collection factories = fm->getFactories();
      DREAM3D_REQUIRE_EQUAL(responseNumFilters, factories.size());
    }
  }

  // -----------------------------------------------------------------------------
  //
  // -----------------------------------------------------------------------------
  void TestPluginInfo()
  {
    QUrl url = getConnectionURL();

    url.setPath("/api/v1/PluginInfo");

    // Test 'Incorrect Content Type'
    {
      QByteArray data;

      QSharedPointer<QNetworkReply> reply = sendRequest(url, "text/plain", data);
      DREAM3D_REQUIRE_EQUAL(reply->error(), 0);

      QJsonParseError jsonParseError;
      QByteArray jsonResponse = reply->readAll();
      QJsonDocument doc = QJsonDocument::fromJson(jsonResponse, &jsonParseError);
      DREAM3D_REQUIRE_EQUAL(jsonParseError.error, QJsonParseError::ParseError::NoError);

      QJsonObject responseObject = doc.object();
      DREAM3D_REQUIRE_EQUAL(responseObject.size(), 2);
      DREAM3D_REQUIRE_EQUAL(responseObject.contains(SIMPL::JSON::ErrorCode), true);
      DREAM3D_REQUIRE_EQUAL(responseObject[SIMPL::JSON::ErrorCode].toInt(), -20);
    }

    // Test 'JSON Parse Error'
    {
      QByteArray data = QByteArray::fromStdString("{ CreateAttributeMatrix");

      QSharedPointer<QNetworkReply> reply = sendRequest(url, "application/json", data);
      DREAM3D_REQUIRE_EQUAL(reply->error(), 0);

      QJsonParseError jsonParseError;
      QByteArray jsonResponse = reply->readAll();
      QJsonDocument doc = QJsonDocument::fromJson(jsonResponse, &jsonParseError);
      DREAM3D_REQUIRE_EQUAL(jsonParseError.error, QJsonParseError::ParseError::NoError);

      QJsonObject responseObject = doc.object();
      DREAM3D_REQUIRE_EQUAL(responseObject.size(), 2);
      DREAM3D_REQUIRE_EQUAL(responseObject.contains(SIMPL::JSON::ErrorCode), true);
      DREAM3D_REQUIRE_EQUAL(responseObject[SIMPL::JSON::ErrorCode].toInt(), -30);
    }

    // Test 'Missing Filter Class Name Object'
    {
      QJsonObject rootObj;
      rootObj["Foo"] = "Generic";
      QJsonDocument doc(rootObj);
      QByteArray data = doc.toJson();

      QSharedPointer<QNetworkReply> reply = sendRequest(url, "application/json", data);
      DREAM3D_REQUIRE_EQUAL(reply->error(), 0);

      QJsonParseError jsonParseError;
      QByteArray jsonResponse = reply->readAll();
      doc = QJsonDocument::fromJson(jsonResponse, &jsonParseError);
      DREAM3D_REQUIRE_EQUAL(jsonParseError.error, QJsonParseError::ParseError::NoError);

      QJsonObject responseObject = doc.object();
      DREAM3D_REQUIRE_EQUAL(responseObject.size(), 2);
      DREAM3D_REQUIRE_EQUAL(responseObject.contains(SIMPL::JSON::ErrorCode), true);
      DREAM3D_REQUIRE_EQUAL(responseObject[SIMPL::JSON::ErrorCode].toInt(), -40);
    }

    // Test 'Class Name Object Not a String'
    {
      QJsonObject rootObj;
      rootObj[SIMPL::JSON::PluginBaseName] = 2;
      QJsonDocument doc(rootObj);
      QByteArray data = doc.toJson();

      QSharedPointer<QNetworkReply> reply = sendRequest(url, "application/json", data);
      DREAM3D_REQUIRE_EQUAL(reply->error(), 0);

      QJsonParseError jsonParseError;
      QByteArray jsonResponse = reply->readAll();
      doc = QJsonDocument::fromJson(jsonResponse, &jsonParseError);
      DREAM3D_REQUIRE_EQUAL(jsonParseError.error, QJsonParseError::ParseError::NoError);

      QJsonObject responseObject = doc.object();
      DREAM3D_REQUIRE_EQUAL(responseObject.size(), 2);
      DREAM3D_REQUIRE_EQUAL(responseObject.contains(SIMPL::JSON::ErrorCode), true);
      DREAM3D_REQUIRE_EQUAL(responseObject[SIMPL::JSON::ErrorCode].toInt(), -50);
    }

    // Test 'Empty Class Name'
    {
      QJsonObject rootObj;
      rootObj[SIMPL::JSON::PluginBaseName] = "";
      QJsonDocument doc(rootObj);
      QByteArray data = doc.toJson();

      QSharedPointer<QNetworkReply> reply = sendRequest(url, "application/json", data);
      DREAM3D_REQUIRE_EQUAL(reply->error(), 0);

      QJsonParseError jsonParseError;
      QByteArray jsonResponse = reply->readAll();
      doc = QJsonDocument::fromJson(jsonResponse, &jsonParseError);
      DREAM3D_REQUIRE_EQUAL(jsonParseError.error, QJsonParseError::ParseError::NoError);

      QJsonObject responseObject = doc.object();
      DREAM3D_REQUIRE_EQUAL(responseObject.size(), 2);
      DREAM3D_REQUIRE_EQUAL(responseObject.contains(SIMPL::JSON::ErrorCode), true);
      DREAM3D_REQUIRE_EQUAL(responseObject[SIMPL::JSON::ErrorCode].toInt(), -60);
    }

    // Test 'Unidentified Filter Name'
    {
      QJsonObject rootObj;
      rootObj[SIMPL::JSON::PluginBaseName] = "FooBarBaz";
      QJsonDocument doc(rootObj);
      QByteArray data = doc.toJson();

      QSharedPointer<QNetworkReply> reply = sendRequest(url, "application/json", data);
      DREAM3D_REQUIRE_EQUAL(reply->error(), 0);

      QJsonParseError jsonParseError;
      QByteArray jsonResponse = reply->readAll();
      doc = QJsonDocument::fromJson(jsonResponse, &jsonParseError);
      DREAM3D_REQUIRE_EQUAL(jsonParseError.error, QJsonParseError::ParseError::NoError);

      QJsonObject responseObject = doc.object();
      DREAM3D_REQUIRE_EQUAL(responseObject.size(), 2);
      DREAM3D_REQUIRE_EQUAL(responseObject.contains(SIMPL::JSON::ErrorCode), true);
      DREAM3D_REQUIRE_EQUAL(responseObject[SIMPL::JSON::ErrorCode].toInt(), -60);
    }

    // Test correct outcome
    {
      QJsonObject rootObj;
      QString pluginName = "Generic";
      rootObj[SIMPL::JSON::PluginBaseName] = pluginName;
      QJsonDocument doc(rootObj);
      QByteArray data = doc.toJson();

      QSharedPointer<QNetworkReply> reply = sendRequest(url, "application/json", data);
      DREAM3D_REQUIRE_EQUAL(reply->error(), 0);

      QJsonParseError jsonParseError;
      QByteArray jsonResponse = reply->readAll();
      doc = QJsonDocument::fromJson(jsonResponse, &jsonParseError);
      DREAM3D_REQUIRE_EQUAL(jsonParseError.error, QJsonParseError::ParseError::NoError);

      QJsonObject responseObject = doc.object();
      DREAM3D_REQUIRE_EQUAL(responseObject.size(), 4);
      DREAM3D_REQUIRE_EQUAL(responseObject.contains(SIMPL::JSON::ErrorCode), true);
      DREAM3D_REQUIRE_EQUAL(responseObject[SIMPL::JSON::ErrorCode].toInt(), 0);

      DREAM3D_REQUIRE_EQUAL(responseObject.contains(SIMPL::JSON::Plugin), true);
      DREAM3D_REQUIRE_EQUAL(responseObject[SIMPL::JSON::Plugin].isObject(), true);

      QJsonObject responseFPObj = responseObject[SIMPL::JSON::Plugin].toObject();

      PluginManager* pm = PluginManager::Instance();
      ISIMPLibPlugin* plugin = pm->findPlugin(pluginName);
      DREAM3D_REQUIRED_PTR(plugin, !=, nullptr);

      DREAM3D_REQUIRE_EQUAL(responseFPObj.contains(SIMPL::JSON::PluginFileName), true);
      DREAM3D_REQUIRE_EQUAL(responseFPObj[SIMPL::JSON::PluginFileName].isString(), true);
      DREAM3D_REQUIRE_EQUAL(responseFPObj[SIMPL::JSON::PluginFileName].toString(), plugin->getPluginFileName());

      DREAM3D_REQUIRE_EQUAL(responseFPObj.contains(SIMPL::JSON::DisplayName), true);
      DREAM3D_REQUIRE_EQUAL(responseFPObj[SIMPL::JSON::DisplayName].isString(), true);
      DREAM3D_REQUIRE_EQUAL(responseFPObj[SIMPL::JSON::DisplayName].toString(), plugin->getPluginDisplayName());

      DREAM3D_REQUIRE_EQUAL(responseFPObj.contains(SIMPL::JSON::Version), true);
      DREAM3D_REQUIRE_EQUAL(responseFPObj[SIMPL::JSON::Version].isString(), true);
      DREAM3D_REQUIRE_EQUAL(responseFPObj[SIMPL::JSON::Version].toString(), plugin->getVersion());

      DREAM3D_REQUIRE_EQUAL(responseFPObj.contains(SIMPL::JSON::CompatibilityVersion), true);
      DREAM3D_REQUIRE_EQUAL(responseFPObj[SIMPL::JSON::CompatibilityVersion].isString(), true);
      DREAM3D_REQUIRE_EQUAL(responseFPObj[SIMPL::JSON::CompatibilityVersion].toString(), plugin->getCompatibilityVersion());

      DREAM3D_REQUIRE_EQUAL(responseFPObj.contains(SIMPL::JSON::Vendor), true);
      DREAM3D_REQUIRE_EQUAL(responseFPObj[SIMPL::JSON::Vendor].isString(), true);
      DREAM3D_REQUIRE_EQUAL(responseFPObj[SIMPL::JSON::Vendor].toString(), plugin->getVendor());

      DREAM3D_REQUIRE_EQUAL(responseFPObj.contains(SIMPL::JSON::URL), true);
      DREAM3D_REQUIRE_EQUAL(responseFPObj[SIMPL::JSON::URL].isString(), true);
      DREAM3D_REQUIRE_EQUAL(responseFPObj[SIMPL::JSON::URL].toString(), plugin->getURL());

      DREAM3D_REQUIRE_EQUAL(responseFPObj.contains(SIMPL::JSON::Location), true);
      DREAM3D_REQUIRE_EQUAL(responseFPObj[SIMPL::JSON::Location].isString(), true);
      DREAM3D_REQUIRE_EQUAL(responseFPObj[SIMPL::JSON::Location].toString(), plugin->getLocation());

      DREAM3D_REQUIRE_EQUAL(responseFPObj.contains(SIMPL::JSON::Description), true);
      DREAM3D_REQUIRE_EQUAL(responseFPObj[SIMPL::JSON::Description].isString(), true);
      DREAM3D_REQUIRE_EQUAL(responseFPObj[SIMPL::JSON::Description].toString(), plugin->getDescription());

      DREAM3D_REQUIRE_EQUAL(responseFPObj.contains(SIMPL::JSON::Copyright), true);
      DREAM3D_REQUIRE_EQUAL(responseFPObj[SIMPL::JSON::Copyright].isString(), true);
      DREAM3D_REQUIRE_EQUAL(responseFPObj[SIMPL::JSON::Copyright].toString(), plugin->getCopyright());

      DREAM3D_REQUIRE_EQUAL(responseFPObj.contains(SIMPL::JSON::License), true);
      DREAM3D_REQUIRE_EQUAL(responseFPObj[SIMPL::JSON::License].isString(), true);
      DREAM3D_REQUIRE_EQUAL(responseFPObj[SIMPL::JSON::License].toString(), plugin->getLicense());
    }
  }

  // -----------------------------------------------------------------------------
  //
  // -----------------------------------------------------------------------------
  void TestPreflightPipeline()
  {
    QUrl url = getConnectionURL();

    url.setPath("/api/v1/PreflightPipeline");

    // Test 'Incorrect Content Type'
    {
      QByteArray data;

      QSharedPointer<QNetworkReply> reply = sendRequest(url, "text/plain", data);
      DREAM3D_REQUIRE_EQUAL(reply->error(), 0);

      QJsonParseError jsonParseError;
      QByteArray jsonResponse = reply->readAll();
      QJsonDocument doc = QJsonDocument::fromJson(jsonResponse, &jsonParseError);
      DREAM3D_REQUIRE_EQUAL(jsonParseError.error, QJsonParseError::ParseError::NoError);

      QJsonObject responseObject = doc.object();
      DREAM3D_REQUIRE_EQUAL(responseObject.size(), 3);
      DREAM3D_REQUIRE_EQUAL(responseObject.contains(SIMPL::JSON::ErrorCode), true);
      DREAM3D_REQUIRE_EQUAL(responseObject[SIMPL::JSON::ErrorCode].toInt(), -20);
    }

    // Test 'JSON Parse Error'
    {
      QByteArray data = QByteArray::fromStdString("{ CreateAttributeMatrix");

      QSharedPointer<QNetworkReply> reply = sendRequest(url, "application/json", data);
      DREAM3D_REQUIRE_EQUAL(reply->error(), 0);

      QJsonParseError jsonParseError;
      QByteArray jsonResponse = reply->readAll();
      QJsonDocument doc = QJsonDocument::fromJson(jsonResponse, &jsonParseError);
      DREAM3D_REQUIRE_EQUAL(jsonParseError.error, QJsonParseError::ParseError::NoError);

      QJsonObject responseObject = doc.object();
      DREAM3D_REQUIRE_EQUAL(responseObject.size(), 3);
      DREAM3D_REQUIRE_EQUAL(responseObject.contains(SIMPL::JSON::ErrorCode), true);
      DREAM3D_REQUIRE_EQUAL(responseObject[SIMPL::JSON::ErrorCode].toInt(), -30);
    }

    // Test 'Missing Pipeline Object'
    {
      QJsonObject rootObj;
      rootObj["Foo"] = "{ }";
      QJsonDocument doc(rootObj);
      QByteArray data = doc.toJson();

      QSharedPointer<QNetworkReply> reply = sendRequest(url, "application/json", data);
      DREAM3D_REQUIRE_EQUAL(reply->error(), 0);

      QJsonParseError jsonParseError;
      QByteArray jsonResponse = reply->readAll();
      doc = QJsonDocument::fromJson(jsonResponse, &jsonParseError);
      DREAM3D_REQUIRE_EQUAL(jsonParseError.error, QJsonParseError::ParseError::NoError);

      QJsonObject responseObject = doc.object();
      DREAM3D_REQUIRE_EQUAL(responseObject.size(), 3);
      DREAM3D_REQUIRE_EQUAL(responseObject.contains(SIMPL::JSON::ErrorCode), true);
      DREAM3D_REQUIRE_EQUAL(responseObject[SIMPL::JSON::ErrorCode].toInt(), -40);
    }

    // Test 'Pipeline Could Not Be Created'
    {
      QJsonObject rootObj;
      rootObj[SIMPL::JSON::Pipeline] = 2;
      QJsonDocument doc(rootObj);
      QByteArray data = doc.toJson();

      QSharedPointer<QNetworkReply> reply = sendRequest(url, "application/json", data);
      DREAM3D_REQUIRE_EQUAL(reply->error(), 0);

      QJsonParseError jsonParseError;
      QByteArray jsonResponse = reply->readAll();
      doc = QJsonDocument::fromJson(jsonResponse, &jsonParseError);
      DREAM3D_REQUIRE_EQUAL(jsonParseError.error, QJsonParseError::ParseError::NoError);

      QJsonObject responseObject = doc.object();
      DREAM3D_REQUIRE_EQUAL(responseObject.size(), 3);
      DREAM3D_REQUIRE_EQUAL(responseObject.contains(SIMPL::JSON::ErrorCode), true);
      DREAM3D_REQUIRE_EQUAL(responseObject[SIMPL::JSON::ErrorCode].toInt(), -50);
    }

    // Test pipeline that has errors
    {
      QFile file(UnitTest::RestUnitTest::RESTErrorPipelineFilePath);
      DREAM3D_REQUIRE_EQUAL(file.open(QIODevice::ReadOnly), true);

      QTextStream in(&file);
      QString jsonString = in.readAll();
      QByteArray jsonByteArray = QByteArray::fromStdString(jsonString.toStdString());
      QJsonDocument doc = QJsonDocument::fromJson(jsonByteArray);

      QJsonObject rootObj;
      rootObj[SIMPL::JSON::Pipeline] = doc.object();

      QJsonDocument doc2(rootObj);
      QByteArray data = doc2.toJson();

      QSharedPointer<QNetworkReply> reply = sendRequest(url, "application/json", data);
      DREAM3D_REQUIRE_EQUAL(reply->error(), 0);

      QJsonParseError jsonParseError;
      QByteArray jsonResponse = reply->readAll();
      doc = QJsonDocument::fromJson(jsonResponse, &jsonParseError);
      DREAM3D_REQUIRE_EQUAL(jsonParseError.error, QJsonParseError::ParseError::NoError);

      QJsonObject responseObject = doc.object();
      DREAM3D_REQUIRE_EQUAL(responseObject.size(), 4);
      DREAM3D_REQUIRE_EQUAL(responseObject.contains(SIMPL::JSON::Completed), true);
      DREAM3D_REQUIRE_EQUAL(responseObject[SIMPL::JSON::Completed].isBool(), true);
      DREAM3D_REQUIRE_EQUAL(responseObject[SIMPL::JSON::Completed].toBool(), false);

      DREAM3D_REQUIRE_EQUAL(responseObject.contains(SIMPL::JSON::PipelineErrors), true);
      DREAM3D_REQUIRE_EQUAL(responseObject[SIMPL::JSON::PipelineErrors].isArray(), true);

      QJsonArray responseErrorsArray = responseObject[SIMPL::JSON::PipelineErrors].toArray();
      DREAM3D_REQUIRE_EQUAL(responseErrorsArray.size(), 4);

      DREAM3D_REQUIRE_EQUAL(responseObject[SIMPL::JSON::PipelineWarnings].isArray(), true);
      QJsonArray responseWarningsArray = responseObject[SIMPL::JSON::PipelineWarnings].toArray();
      DREAM3D_REQUIRE_EQUAL(responseWarningsArray.size(), 0);

      JsonFilterParametersReader::Pointer reader = JsonFilterParametersReader::New();
      FilterPipeline::Pointer pipeline = reader->readPipelineFromFile(UnitTest::RestUnitTest::RESTErrorPipelineFilePath);

      PipelineListener listener(nullptr);
      pipeline->addMessageReceiver(&listener);

      pipeline->preflightPipeline();

      std::vector<const AbstractWarningMessage*> warningMessages = listener.getWarningMessages();
      DREAM3D_REQUIRE_EQUAL(warningMessages.size(), 0);

      std::vector<const AbstractErrorMessage*> errorMessages = listener.getErrorMessages();
      DREAM3D_REQUIRED(errorMessages.size(), >, 0);

      for(int i = 0; i < responseErrorsArray.size(); i++)
      {
        DREAM3D_REQUIRE_EQUAL(responseErrorsArray[i].isObject(), true);

        QJsonObject responseErrorObject = responseErrorsArray[i].toObject();
        DREAM3D_REQUIRE_EQUAL(responseErrorObject[SIMPL::JSON::Code].isDouble(), true);
        DREAM3D_REQUIRE_EQUAL(responseErrorObject[SIMPL::JSON::Message].isString(), true);
        DREAM3D_REQUIRE_EQUAL(responseErrorObject[SIMPL::JSON::FilterHumanLabel].isString(), true);
        DREAM3D_REQUIRE_EQUAL(responseErrorObject[SIMPL::JSON::FilterIndex].isDouble(), true);

        int responseErrorCode = responseErrorObject[SIMPL::JSON::Code].toInt();
        const AbstractErrorMessage* errorMessage = errorMessages[i];
        DREAM3D_REQUIRE_EQUAL(responseErrorCode, errorMessage->getCode());
      }
    }

    // Test pipeline with no errors
    {
      QFile file(UnitTest::RestUnitTest::RESTPipelineFilePath);
      DREAM3D_REQUIRE_EQUAL(file.open(QIODevice::ReadOnly), true);

      QTextStream in(&file);
      QString jsonString = in.readAll();
      QByteArray jsonByteArray = QByteArray::fromStdString(jsonString.toStdString());
      QJsonDocument doc = QJsonDocument::fromJson(jsonByteArray);

      QJsonObject rootObj;
      rootObj[SIMPL::JSON::Pipeline] = doc.object();

      QJsonDocument doc2(rootObj);
      QByteArray data = doc2.toJson();

      QSharedPointer<QNetworkReply> reply = sendRequest(url, "application/json", data);
      DREAM3D_REQUIRE_EQUAL(reply->error(), 0);

      QJsonParseError jsonParseError;
      QByteArray jsonResponse = reply->readAll();
      doc = QJsonDocument::fromJson(jsonResponse, &jsonParseError);
      DREAM3D_REQUIRE_EQUAL(jsonParseError.error, QJsonParseError::ParseError::NoError);

      QJsonObject responseObject = doc.object();
      DREAM3D_REQUIRE_EQUAL(responseObject.size(), 4);
      DREAM3D_REQUIRE_EQUAL(responseObject.contains(SIMPL::JSON::Completed), true);
      DREAM3D_REQUIRE_EQUAL(responseObject[SIMPL::JSON::Completed].isBool(), true);
      DREAM3D_REQUIRE_EQUAL(responseObject[SIMPL::JSON::Completed].toBool(), true);

      DREAM3D_REQUIRE_EQUAL(responseObject.contains(SIMPL::JSON::PipelineErrors), true);
      DREAM3D_REQUIRE_EQUAL(responseObject[SIMPL::JSON::PipelineErrors].isArray(), true);

      QJsonArray responseErrorsArray = responseObject[SIMPL::JSON::PipelineErrors].toArray();
      DREAM3D_REQUIRE_EQUAL(responseErrorsArray.size(), 0);

      DREAM3D_REQUIRE_EQUAL(responseObject[SIMPL::JSON::PipelineWarnings].isArray(), true);
      QJsonArray responseWarningsArray = responseObject[SIMPL::JSON::PipelineWarnings].toArray();
      DREAM3D_REQUIRE_EQUAL(responseWarningsArray.size(), 0);

      JsonFilterParametersReader::Pointer reader = JsonFilterParametersReader::New();
      FilterPipeline::Pointer pipeline = reader->readPipelineFromFile(UnitTest::RestUnitTest::RESTPipelineFilePath);

      PipelineListener listener(nullptr);
      pipeline->addMessageReceiver(&listener);

      pipeline->preflightPipeline();

      std::vector<const AbstractWarningMessage*> warningMessages = listener.getWarningMessages();
      DREAM3D_REQUIRE_EQUAL(warningMessages.size(), 0);

      std::vector<const AbstractErrorMessage*> errorMessages = listener.getErrorMessages();
      DREAM3D_REQUIRE_EQUAL(errorMessages.size(), 0);
    }
  }

  // -----------------------------------------------------------------------------
  //
  // -----------------------------------------------------------------------------
  void TestSIMPLibVersion()
  {
    QUrl url = getConnectionURL();

    url.setPath("/api/v1/SIMPLibVersion");

    QByteArray data; // No actual Application data is required.

    // Test incorrect content type
    {
      QSharedPointer<QNetworkReply> reply = sendRequest(url, "text/plain", data);
      DREAM3D_REQUIRE_EQUAL(reply->error(), 0);

      QJsonParseError jsonParseError;
      QByteArray jsonResponse = reply->readAll();
      QJsonDocument doc = QJsonDocument::fromJson(jsonResponse, &jsonParseError);
      DREAM3D_REQUIRE_EQUAL(jsonParseError.error, QJsonParseError::ParseError::NoError);

      QJsonObject responseObject = doc.object();
      DREAM3D_REQUIRE_EQUAL(responseObject.size(), 2);
      DREAM3D_REQUIRE_EQUAL(responseObject.contains(SIMPL::JSON::ErrorCode), true);
      DREAM3D_REQUIRE_EQUAL(responseObject[SIMPL::JSON::ErrorCode].toInt(), -20);
    }

    {
      QSharedPointer<QNetworkReply> reply = sendRequest(url, "application/json", data);
      DREAM3D_REQUIRE_EQUAL(reply->error(), 0);

      QJsonParseError jsonParseError;
      QByteArray jsonResponse = reply->readAll();
      QJsonDocument doc = QJsonDocument::fromJson(jsonResponse, &jsonParseError);
      DREAM3D_REQUIRE_EQUAL(jsonParseError.error, QJsonParseError::ParseError::NoError);

      QJsonObject responseObject = doc.object();
      DREAM3D_REQUIRE_EQUAL(responseObject.size(), 3);
      DREAM3D_REQUIRE_EQUAL(responseObject.contains(SIMPL::JSON::ErrorCode), true);
      DREAM3D_REQUIRE_EQUAL(responseObject[SIMPL::JSON::ErrorCode].toInt(), 0);
      DREAM3D_REQUIRE_EQUAL(responseObject.contains(SIMPL::JSON::Version), true);
      DREAM3D_REQUIRE_EQUAL(responseObject[SIMPL::JSON::Version].isString(), true);

      QString responseVersion = responseObject[SIMPL::JSON::Version].toString();

      DREAM3D_REQUIRE_EQUAL(responseVersion, SIMPLib::Version::Complete());
    }
  }

  // -----------------------------------------------------------------------------
  //
  // -----------------------------------------------------------------------------
  void registerFilters()
  {
    //    // Register all the filters including trying to load those from Plugins
    //    FilterManager* fm = FilterManager::Instance();
    //    SIMPLibPluginLoader::LoadPluginFilters(fm);
    //    //
    //    QMetaObjectUtilities::RegisterMetaTypes();
  }

  // -----------------------------------------------------------------------------
  //
  // -----------------------------------------------------------------------------
  void startServer()
  {
    // Find the configuration file
    QString configFileName = UnitTest::RestUnitTest::RestServerConfigFilePath;
    QSettings settings(configFileName, QSettings::IniFormat);
    // Configure session store
    m_SessionSettings = new ServerSettings(settings);
    m_RequestMapper = new SIMPLRequestMapper();
    m_SessionStore = QSharedPointer<HttpSessionStore>(HttpSessionStore::CreateInstance(m_SessionSettings));

    // Configure static file controller
    SIMPLStaticFileController::CreateInstance(m_SessionSettings);

    // Configure and start the TCP listener
    for(const QHostAddress& address : QNetworkInterface::allAddresses())
    {
      if(address.protocol() == QAbstractSocket::IPv4Protocol && address.isLoopback() == false)
      {
        QString localhostIP = address.toString();
        m_SessionSettings->host = localhostIP;
        break;
      }
    }

    m_HttpListener = QSharedPointer<HttpListener>(new HttpListener(m_SessionSettings, m_RequestMapper));
  }

  // -----------------------------------------------------------------------------
  //
  // -----------------------------------------------------------------------------
  void endServer()
  {
    m_SessionStore.clear();
    m_HttpListener.clear();
    delete m_SessionSettings;
    m_SessionSettings = nullptr;
    delete m_RequestMapper;
    m_RequestMapper = nullptr;
  }

  // -----------------------------------------------------------------------------
  //
  // -----------------------------------------------------------------------------
  void operator()()
  {
    std::cout << "#### RestUnitTest Starting ####" << std::endl;

    int err = EXIT_SUCCESS;

    DREAM3D_REGISTER_TEST(RemoveTestFiles());

    registerFilters();

    checkDREAM3DTestRequirements();

    m_Connection = QSharedPointer<QNetworkAccessManager>(new QNetworkAccessManager());

    startServer();

    if(m_RunDREAM3DTests)
    {
      DREAM3D_REGISTER_TEST(TestExecutePipelineWithFolder());
    }

    DREAM3D_REGISTER_TEST(TestExecutePipelineWithFiles());
    DREAM3D_REGISTER_TEST(TestExecutePipeline());

    DREAM3D_REGISTER_TEST(TestListFilterParameters());
    DREAM3D_REGISTER_TEST(TestLoadedPlugins());
    DREAM3D_REGISTER_TEST(TestNamesOfFilters());
    DREAM3D_REGISTER_TEST(TestNumFilters());
    DREAM3D_REGISTER_TEST(TestPluginInfo());
    DREAM3D_REGISTER_TEST(TestPreflightPipeline());
    DREAM3D_REGISTER_TEST(TestSIMPLibVersion());

    DREAM3D_REGISTER_TEST(RemoveTestFiles());

    endServer();
  }

private:
  ServerSettings* m_SessionSettings = nullptr;
  SIMPLRequestMapper* m_RequestMapper = nullptr;

  QSharedPointer<HttpSessionStore> m_SessionStore;
  QSharedPointer<HttpListener> m_HttpListener;

  QSharedPointer<QNetworkAccessManager> m_Connection;

  RESTUnitTestObserver m_Observer;

  bool m_RunDREAM3DTests = true;

  RESTUnitTest(const RESTUnitTest&);   // Copy Constructor Not Implemented
  void operator=(const RESTUnitTest&); // Move assignment Not Implemented
};
