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

#include "DataArrayPathSelectionWidget.h"

#include <QtCore/QMimeData>
#include <QtCore/QTextStream>
#include <QtGui/QDrag>
#include <QtGui/QPainter>
#include <QtGui/QPainterPath>
#include <QtGui/QTextDocument>
#include <QtWidgets/QApplication>

#include "SIMPLib/DataContainers/DataContainer.h"
#include "SIMPLib/DataContainers/DataContainerArray.h"
#include "SIMPLib/Utilities/STLUtilities.hpp"

#include "SVWidgetsLib/FilterParameterWidgets/FilterParameterWidget.h"
#include "SVWidgetsLib/FilterParameterWidgets/FilterParameterWidgetUtils.hpp"
#include "SVWidgetsLib/QtSupport/QtSSettings.h"
#include "SVWidgetsLib/Widgets/SVStyle.h"

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
const QPixmap DataArrayPathSelectionWidget::CreateDragIcon(const DataArrayPath& path)
{
  return CreateDragIcon(path.serialize(Detail::Delimiter), path.getDataType());
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
const QPixmap DataArrayPathSelectionWidget::CreateDragIcon(const QString& text, DataArrayPathHelper::DataType dataType)
{
  return CreateDragIcon(text, GetActiveColor(dataType));
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
const QPixmap DataArrayPathSelectionWidget::CreateDragIcon(const QString& text, const QColor& backgroundColor)
{
  int minHeight = 26;

  QFont font;
  font.setBold(true);
  font.setWeight(QFont::Bold);
  font.setStyleStrategy(QFont::PreferAntialias);
  font.setFamily(SVStyle::Instance()->GetUIFont());

  QTextDocument* doc = new QTextDocument();
  doc->setDefaultFont(font);
  doc->setPlainText("  " + text + "  ");
  qreal textWidth = doc->idealWidth();
  qreal textHeight = doc->pageSize().height();
  if(textHeight < minHeight)
  {
    textHeight = minHeight;
  }

  QPen textPen;
  textPen.setColor(QColor(255, 255, 255));
  int radius = 5;
  int width = 4;
  int halfWidth = width / 2;
  QPen backgroundPen;
  backgroundPen.setColor(backgroundColor);
  backgroundPen.setWidth(width);
  QRect contentRect(halfWidth, halfWidth, textWidth - width, textHeight - width);

  QImage image(textWidth, textHeight, QImage::Format::Format_ARGB32);
  image.fill(QColor(0, 0, 0, 0));
  QPainter painter(&image);
  painter.setFont(font);
  painter.setPen(backgroundPen);
  painter.drawRoundedRect(contentRect, radius, radius);
  painter.fillRect(contentRect, backgroundColor);
  painter.setPen(textPen);
  painter.drawText(contentRect, Qt::AlignCenter | Qt::AlignVCenter, doc->toRawText());

  doc->deleteLater();

  QPixmap pixmap = QPixmap::fromImage(image);
  return pixmap;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
const QColor DataArrayPathSelectionWidget::GetActiveColor(DataArrayPathHelper::DataType type)
{
  QColor color;
  SVStyle* style = SVStyle::Instance();

  switch(type)
  {
  case DataArrayPathHelper::DataType::DataContainer:
    color = style->getDataArrayPath_DataContainer_color();
    break;
  case DataArrayPathHelper::DataType::AttributeMatrix:
    color = style->getDataArrayPath_AttributeMatrix_color();
    break;
  case DataArrayPathHelper::DataType::DataArray:
    color = style->getDataArrayPath_DataArray_color();
    break;
  case DataArrayPathHelper::DataType::None:
    color = style->getDataArrayPath_Invalid_color();
    break;
  }

  return color;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
const QColor DataArrayPathSelectionWidget::GetCheckedColor(DataArrayPathHelper::DataType type)
{
  QColor color;
  SVStyle* style = SVStyle::Instance();

  switch(type)
  {
  case DataArrayPathHelper::DataType::DataContainer:
    color = style->getDataArrayPath_DataContainer_background_color();
    break;
  case DataArrayPathHelper::DataType::AttributeMatrix:
    color = style->getDataArrayPath_AttributeMatrix_background_color();
    break;
  case DataArrayPathHelper::DataType::DataArray:
    color = style->getDataArrayPath_DataArray_background_color();
    break;
  case DataArrayPathHelper::DataType::None:
    color = style->getDataArrayPath_Invalid_background_color();
    break;
  }

  return color;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
bool DataArrayPathSelectionWidget::CheckPathRequirements(AbstractFilter* filter, const DataArrayPath& path, const DataContainerSelectionFilterParameter::RequirementType& reqs)
{
  if(DataArrayPathHelper::DataType::DataContainer != path.getDataType())
  {
    return false;
  }
  if(nullptr == filter)
  {
    return false;
  }
  if(nullptr == filter->getDataContainerArray())
  {
    return false;
  }

  // Check that the DataContainer exists
  DataContainer::Pointer dc = filter->getDataContainerArray()->getDataContainer(path);
  if(nullptr == dc)
  {
    return false;
  }

  // Check Geometry
  if(!reqs.dcGeometryTypes.empty())
  {
    // Unknown Geometry gets a pass
    if(!SIMPL::contains(reqs.dcGeometryTypes, IGeometry::Type::Unknown))
    {
      IGeometry::Pointer geom = dc->getGeometry();
      if(nullptr == geom)
      {
        return false;
      }
      if(!SIMPL::contains(reqs.dcGeometryTypes, geom->getGeometryType()) && !SIMPL::contains(reqs.dcGeometryTypes, IGeometry::Type::Any))
      {
        return false;
      }
    }
  }

  return true;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
bool DataArrayPathSelectionWidget::CheckPathRequirements(AbstractFilter* filter, const DataArrayPath& path, const AttributeMatrixSelectionFilterParameter::RequirementType& reqs)
{
  if(DataArrayPathHelper::DataType::AttributeMatrix != path.getDataType())
  {
    return false;
  }
  if(nullptr == filter)
  {
    return false;
  }
  if(nullptr == filter->getDataContainerArray())
  {
    return false;
  }

  // Check that the DataContainer exists
  DataContainer::Pointer dc = filter->getDataContainerArray()->getDataContainer(path);
  if(nullptr == dc)
  {
    return false;
  }

  // Check if geometry exists and matches the requirements only if a geometry is required
  if(!reqs.dcGeometryTypes.empty())
  {
    // Unknown Geometry gets a pass
    if(!SIMPL::contains(reqs.dcGeometryTypes, IGeometry::Type::Unknown))
    {
      IGeometry::Pointer geom = dc->getGeometry();
      if(nullptr == geom)
      {
        return false;
      }
      if(!SIMPL::contains(reqs.dcGeometryTypes, geom->getGeometryType()) && !SIMPL::contains(reqs.dcGeometryTypes, IGeometry::Type::Any))
      {
        return false;
      }
    }
  }

  // Check AttributeMatrix
  AttributeMatrix::Pointer am = dc->getAttributeMatrix(path);
  if(nullptr == am)
  {
    return false;
  }

  if(!SIMPL::contains(reqs.amTypes, AttributeMatrix::Type::Unknown))
  {
    if(!reqs.amTypes.empty() && !(SIMPL::contains(reqs.amTypes, am->getType()) || SIMPL::contains(reqs.amTypes, AttributeMatrix::Type::Any)))
    {
      return false;
    }
  }

  return true;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
bool DataArrayPathSelectionWidget::CheckPathRequirements(AbstractFilter* filter, const DataArrayPath& path, const DataArraySelectionFilterParameter::RequirementType& reqs)
{
  if(DataArrayPathHelper::DataType::DataArray != path.getDataType())
  {
    return false;
  }
  if(nullptr == filter)
  {
    return false;
  }
  if(nullptr == filter->getDataContainerArray())
  {
    return false;
  }

  // Check that the DataContainer exists
  DataContainer::Pointer dc = filter->getDataContainerArray()->getDataContainer(path);
  if(nullptr == dc)
  {
    return false;
  }

  // Check if geometry exists and matches the requirements only if a geometry is required
  if(!reqs.dcGeometryTypes.empty())
  {
    // Unknown Geometry gets a pass
    if(!SIMPL::contains(reqs.dcGeometryTypes, IGeometry::Type::Unknown))
    {
      IGeometry::Pointer geom = dc->getGeometry();
      if(nullptr == geom)
      {
        return false;
      }
      if(!SIMPL::contains(reqs.dcGeometryTypes, geom->getGeometryType()) && !SIMPL::contains(reqs.dcGeometryTypes, IGeometry::Type::Any))
      {
        return false;
      }
    }
  }

  // Check AttributeMatrix
  AttributeMatrix::Pointer am = dc->getAttributeMatrix(path);
  if(nullptr == am)
  {
    return false;
  }

  if(!SIMPL::contains(reqs.amTypes, AttributeMatrix::Type::Unknown))
  {
    if(!reqs.amTypes.empty() && !SIMPL::contains(reqs.amTypes, am->getType()) && !SIMPL::contains(reqs.amTypes, AttributeMatrix::Type::Any))
    {
      return false;
    }
  }

  // Check DataArray
  IDataArray::Pointer da = am->getAttributeArray(path.getDataArrayName());
  if(nullptr == da)
  {
    return false;
  }

  if(!reqs.daTypes.empty() && !SIMPL::contains(reqs.daTypes, da->getTypeAsString()))
  {
    return false;
  }
  using SizeTVectorType = std::vector<size_t>;

  if(reqs.componentDimensions.empty())
  {
    return true;
  }
  std::vector<SizeTVectorType> reqComps = reqs.componentDimensions;
  std::vector<size_t> daComp = da->getComponentDimensions();
  bool contains = false;
  for(const auto& reqComp : reqComps)
  {
    if(reqComp == daComp)
    {
      contains = true;
      break;
    }
  }

  // bool b = !FilterParameterWidgetUtils::VectorContains<size_t>(reqComp, daComp);
  return contains;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
DataArrayPathSelectionWidget::DataArrayPathSelectionWidget(QWidget* parent)
: QToolButton(parent)
{
  setupGui();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void DataArrayPathSelectionWidget::setupGui()
{
  setContextMenuPolicy(Qt::CustomContextMenu);
  setAcceptDrops(true);
  setCheckable(true);

  connect(this, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(showContextMenu(const QPoint&)));
  connect(this, &QAbstractButton::toggled, this, &DataArrayPathSelectionWidget::updateCheckState);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
DataArrayPathHelper::DataType DataArrayPathSelectionWidget::getDataType() const
{
  return m_DataType;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void DataArrayPathSelectionWidget::setDataContainerRequirements(const DataContainerSelectionFilterParameter::RequirementType& dcReqs)
{
  m_DataType = DataArrayPathHelper::DataType::DataContainer;
  m_DataContainerReqs = dcReqs;

  QString html;
  QTextStream ss(&html);
  ss << createTooltipHeader();
  ss << createGeomReqString(dcReqs.dcGeometryTypes);
  ss << createTooltipFooter();

  setToolTip(html);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void DataArrayPathSelectionWidget::setAttrMatrixRequirements(const AttributeMatrixSelectionFilterParameter::RequirementType& amReqs)
{
  m_DataType = DataArrayPathHelper::DataType::AttributeMatrix;
  m_AttrMatrixReqs = amReqs;

  QString html;
  QTextStream ss(&html);
  ss << createTooltipHeader();
  ss << createGeomReqString(amReqs.dcGeometryTypes);
  ss << createAttrMatrixReqString(amReqs.amTypes);
  ss << createTooltipFooter();

  setToolTip(html);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void DataArrayPathSelectionWidget::setDataArrayRequirements(const DataArraySelectionFilterParameter::RequirementType& daReqs)
{
  m_DataType = DataArrayPathHelper::DataType::DataArray;
  m_DataArrayReqs = daReqs;

  QString html;
  QTextStream ss(&html);
  ss << createTooltipHeader();
  ss << createGeomReqString(daReqs.dcGeometryTypes);
  ss << createAttrMatrixReqString(daReqs.amTypes);
  ss << createDataArrayTypeString(daReqs.daTypes);
  ss << createComponentReqString(daReqs.componentDimensions);
  ss << createTooltipFooter();

  setToolTip(html);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString DataArrayPathSelectionWidget::createTooltipHeader() const
{
  QString html;
  QTextStream ss(&html);
  ss << "<html><head></head>\n";
  ss << "<body>\n";
  ss << "<table cellpadding=\"4\" cellspacing=\"0\" border=\"0\">\n";
  ss << "<tbody>\n";
  ss << "<tr><td colspan=3><b><i>Drag and Drop from the Data Structure</i></b></td></tr>";

  return html;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString DataArrayPathSelectionWidget::createTooltipFooter() const
{
  QString html;
  QTextStream ss(&html);
  ss << "</tbody></table><br/>";
  ss << "</body></html>";

  return html;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString DataArrayPathSelectionWidget::createGeomReqString(const IGeometry::Types& geomTypes) const
{
  QString reqStr = "<tr><td><i>Required Geometries:</i></td>";
  if(geomTypes.empty())
  {
    reqStr += "<td>None</td>";
  }
  else
  {
    for(IGeometry::Type type : geomTypes)
    {
      switch(type)
      {
      case IGeometry::Type::Any:
        reqStr += "<td>Any</td>";
        break;
      case IGeometry::Type::Edge:
        reqStr += "<td>Edge</td>";
        break;
      case IGeometry::Type::Image:
        reqStr += "<td>Image</td>";
        break;
      case IGeometry::Type::Quad:
        reqStr += "<td>Quad</td>";
        break;
      case IGeometry::Type::RectGrid:
        reqStr += "<td>RectGrid</td>";
        break;
      case IGeometry::Type::Tetrahedral:
        reqStr += "<td>Tetrahedral</td>";
        break;
      case IGeometry::Type::Triangle:
        reqStr += "<td>Triangle</td>";
        break;
      case IGeometry::Type::Unknown:
        reqStr += "<td>Unknown</td>";
        break;
      case IGeometry::Type::Vertex:
        reqStr += "<td>Vertex</td>";
        break;
      case IGeometry::Type::Hexahedral:
        reqStr += "<td>Hexahedral</td>";
        break;
      }
    }
  }

  reqStr += "</tr>";
  return reqStr;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString DataArrayPathSelectionWidget::createAttrMatrixReqString(const AttributeMatrix::Types& amTypes) const
{
  QString reqStr = "<tr><td><i>Required Matrix Type:</i></td>";
  if(amTypes.empty())
  {
    reqStr += "<td>None</td>";
  }
  else
  {
    for(AttributeMatrix::Type type : amTypes)
    {
      switch(type)
      {
      case AttributeMatrix::Type::Any:
        reqStr += "<td>Any</td>";
        break;
      case AttributeMatrix::Type::Cell:
        reqStr += "<td>Cell</td>";
        break;
      case AttributeMatrix::Type::CellEnsemble:
        reqStr += "<td>Cell Ensemble</td>";
        break;
      case AttributeMatrix::Type::CellFeature:
        reqStr += "<td>Cell Feature</td>";
        break;
      case AttributeMatrix::Type::Edge:
        reqStr += "<td>Edge</td>";
        break;
      case AttributeMatrix::Type::EdgeEnsemble:
        reqStr += "<td>Edge Ensemble</td>";
        break;
      case AttributeMatrix::Type::EdgeFeature:
        reqStr += "<td>Edge Feature</td>";
        break;
      case AttributeMatrix::Type::Face:
        reqStr += "<td>Face</td>";
        break;
      case AttributeMatrix::Type::FaceEnsemble:
        reqStr += "<td>Face Ensemble</td>";
        break;
      case AttributeMatrix::Type::FaceFeature:
        reqStr += "<td>Face Feature</td>";
        break;
      case AttributeMatrix::Type::Generic:
        reqStr += "<td>Generic</td>";
        break;
      case AttributeMatrix::Type::MetaData:
        reqStr += "<td>MetaData</td>";
        break;
      case AttributeMatrix::Type::Unknown:
        reqStr += "<td>Unknown</td>";
        break;
      case AttributeMatrix::Type::Vertex:
        reqStr += "<td>Vertex</td>";
        break;
      case AttributeMatrix::Type::VertexEnsemble:
        reqStr += "<td>Vertex Ensemble</td>";
        break;
      case AttributeMatrix::Type::VertexFeature:
        reqStr += "<td>Vertex Feature</td>";
        break;
      }
    }
  }

  reqStr += "</tr>";
  return reqStr;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString DataArrayPathSelectionWidget::createDataArrayTypeString(const std::vector<QString>& daTypes) const
{
  QString reqStr = "<tr><td><i>Required Array Type:</i></td>";
  if(daTypes.empty())
  {
    reqStr += "<td>None</td>";
  }
  else
  {
    for(const QString& type : daTypes)
    {
      reqStr += "<td>" + type + "</td>";
    }
  }

  reqStr += "</tr>";
  return reqStr;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString DataArrayPathSelectionWidget::createComponentReqString(std::vector<std::vector<size_t>> comps) const
{
  QString reqStr = "<tr><td><i>Required Component Size:</i></td>";
  if(comps.empty())
  {
    reqStr += "<td>None</td>";
  }
  else
  {
    for(const std::vector<size_t>& comp : comps)
    {
      reqStr += "<td>[";

      int compSize = comp.size();
      for(int i = 0; i < compSize; i++)
      {
        if(i != 0)
        {
          reqStr += ",";
        }

        reqStr += QString::number(comp[i]);
      }

      reqStr += "]</tr></td>";
    }
  }

  reqStr += "</tr>";
  return reqStr;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
DataContainerSelectionFilterParameter::RequirementType DataArrayPathSelectionWidget::getDataContainerRequirements() const
{
  return m_DataContainerReqs;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
AttributeMatrixSelectionFilterParameter::RequirementType DataArrayPathSelectionWidget::getAttrMatrixRequirements() const
{
  return m_AttrMatrixReqs;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
DataArraySelectionFilterParameter::RequirementType DataArrayPathSelectionWidget::getDataArrayRequirements() const
{
  return m_DataArrayReqs;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void DataArrayPathSelectionWidget::setDataArrayPath(const DataArrayPath& path)
{
  // Do not check the path if there are no changes
  if(getDataArrayPath() == path)
  {
    return;
  }

  if(checkPathReqs(path))
  {
    setText(path.serialize(Detail::Delimiter));
    resetStyle();
    Q_EMIT pathChanged();
    Q_EMIT endViewPaths();
  }
  else
  {
    // Clear DataArrayPath
    switch(getDataType())
    {
    case DataArrayPathHelper::DataType::DataContainer:
      setText("");
      break;
    case DataArrayPathHelper::DataType::AttributeMatrix:
      setText("\t / \t");
      break;
    case DataArrayPathHelper::DataType::DataArray:
      setText("\t / \t / \t");
      break;
    default:
      setText("");
      break;
    }

    resetStyle();
    Q_EMIT pathChanged();
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void DataArrayPathSelectionWidget::setFilteredDataArrayPath(DataArrayPath path)
{
  if(!isChecked())
  {
    return;
  }

  if(checkPathReqs(path))
  {
    setDataArrayPath(path);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
DataArrayPath DataArrayPathSelectionWidget::getDataArrayPath() const
{
  return DataArrayPath::Deserialize(text(), Detail::Delimiter);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
bool DataArrayPathSelectionWidget::checkCurrentPath() const
{
  return checkPathReqs(getDataArrayPath());
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
bool DataArrayPathSelectionWidget::checkPathReqs(const DataArrayPath& path) const
{
  if(nullptr == m_Filter)
  {
    return false;
  }

  // Do not allow DataArrayPaths created by the current filter to be used as inputs for that same filter.
  std::list<DataArrayPath> createdPaths = m_Filter->getCreatedPaths();
  if(std::find(createdPaths.begin(), createdPaths.end(), path) != createdPaths.end())
  {
    return false;
  }

  // Check requirements
  switch(m_DataType)
  {
  case DataArrayPathHelper::DataType::DataContainer:
    return checkDataContainerReqs(path);
  case DataArrayPathHelper::DataType::AttributeMatrix:
    return checkAttributeMatrixReqs(path);
  case DataArrayPathHelper::DataType::DataArray:
    return checkDataArrayReqs(path);
  case DataArrayPathHelper::DataType::None:
    break;
  }

  return false;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
bool DataArrayPathSelectionWidget::checkDataContainerReqs(const DataArrayPath& path) const
{
  return CheckPathRequirements(m_Filter, path, m_DataContainerReqs);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
bool DataArrayPathSelectionWidget::checkAttributeMatrixReqs(const DataArrayPath& path) const
{
  return CheckPathRequirements(m_Filter, path, m_AttrMatrixReqs);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
bool DataArrayPathSelectionWidget::checkDataArrayReqs(const DataArrayPath& path) const
{
  return CheckPathRequirements(m_Filter, path, m_DataArrayReqs);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void DataArrayPathSelectionWidget::checkDragPath(DataArrayPath inputPath)
{
  m_FilteringPassed = checkPathReqs(inputPath);
  setEnabled(m_FilteringPassed);
  repaint();

  // Force update the styling
  style()->unpolish(this);
  style()->polish(this);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void DataArrayPathSelectionWidget::clearPathFiltering()
{
  setEnabled(true);
  setPathFiltering(false);

  // Force update the styling
  style()->unpolish(this);
  style()->polish(this);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void DataArrayPathSelectionWidget::endExternalFiltering()
{
  setEnabled(true);
  setPathFiltering(isChecked());

  // Force update the styling
  style()->unpolish(this);
  style()->polish(this);
  repaint();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void DataArrayPathSelectionWidget::selectionWidgetLocked(QToolButton* selection)
{
  DataArrayPathSelectionWidget* pathSelection = dynamic_cast<DataArrayPathSelectionWidget*>(selection);
  m_LockedSelection = pathSelection;

  // Set lock and check state based on the given selection.
  if(this != selection)
  {
    setChecked(false);
    setPathFiltering(false);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void DataArrayPathSelectionWidget::selectionWidgetUnlocked(QToolButton* selection)
{
  DataArrayPathSelectionWidget* pathSelection = dynamic_cast<DataArrayPathSelectionWidget*>(selection);
  if(pathSelection == m_LockedSelection)
  {
    m_LockedSelection = nullptr;
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void DataArrayPathSelectionWidget::emitRequirements()
{
  m_FilteringPassed = true;

  switch(m_DataType)
  {
  case DataArrayPathHelper::DataType::DataContainer:
    Q_EMIT viewPathsMatchingReqs(m_DataContainerReqs);
    break;
  case DataArrayPathHelper::DataType::AttributeMatrix:
    Q_EMIT viewPathsMatchingReqs(m_AttrMatrixReqs);
    break;
  case DataArrayPathHelper::DataType::DataArray:
    Q_EMIT viewPathsMatchingReqs(m_DataArrayReqs);
    break;
  case DataArrayPathHelper::DataType::None:
    break;
  }

  setState(State::Active);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void DataArrayPathSelectionWidget::enterEvent(QEvent* event)
{
  if(m_LockedSelection != nullptr)
  {
    return;
  }

  emitRequirements();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void DataArrayPathSelectionWidget::leaveEvent(QEvent* event)
{
  // Do not end filtering if the button state is checked
  if(m_LockedSelection != nullptr)
  {
    return;
  }

  m_FilteringPassed = false;
  Q_EMIT endViewPaths();
  if(checkCurrentPath())
  {
    setState(State::Normal);
  }
  else
  {
    setState(State::NotFound);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void DataArrayPathSelectionWidget::dragEnterEvent(QDragEnterEvent* event)
{
  if(!event->mimeData()->hasFormat(SIMPLView::DragAndDrop::DataArrayPath))
  {
    return;
  }
  if(this == event->source())
  {
    return;
  }

  const QMimeData* mime = event->mimeData();
  QByteArray data = mime->data(SIMPLView::DragAndDrop::DataArrayPath);
  QString dataStr = QString::fromUtf8(data);

  DataArrayPath path = DataArrayPath::Deserialize(dataStr, "|");

  if(checkPathReqs(path))
  {
    setState(State::DragEnabled);
    event->accept();
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void DataArrayPathSelectionWidget::dragLeaveEvent(QDragLeaveEvent* event)
{
  QToolButton::dragLeaveEvent(event);

  if(isChecked())
  {
    return;
  }

  resetStyle();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void DataArrayPathSelectionWidget::dropEvent(QDropEvent* event)
{
  const QMimeData* mime = event->mimeData();
  QByteArray data = mime->data(SIMPLView::DragAndDrop::DataArrayPath);
  QString dataStr = QString::fromUtf8(data);

  setDataArrayPath(DataArrayPath::Deserialize(dataStr, "|"));

  setChecked(false);

  event->accept();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void DataArrayPathSelectionWidget::mousePressEvent(QMouseEvent* event)
{
  if(event->button() == Qt::LeftButton)
  {
    m_StartPos = event->pos();
  }

  QToolButton::mousePressEvent(event);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void DataArrayPathSelectionWidget::mouseMoveEvent(QMouseEvent* event)
{
  if(event->buttons() & Qt::LeftButton)
  {
    int distance = (event->pos() - m_StartPos).manhattanLength();
    if(distance >= QApplication::startDragDistance())
    {
      performDrag();
    }
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void DataArrayPathSelectionWidget::performDrag()
{
  setChecked(true);
  DataArrayPath path = getDataArrayPath();
  Q_EMIT filterPath(path);

  // MimeData stores the current path AND marks itself as a SelectionWidget drag
  // for connecting to the DataStructureWidget.
  QMimeData* mimeData = new QMimeData;
  mimeData->setData(SIMPLView::DragAndDrop::DataArrayPath, path.serialize().toUtf8());
  mimeData->setData(SIMPLView::DragAndDrop::SelectionWidget, path.serialize().toUtf8());

  QDrag* drag = new QDrag(this);
  drag->setMimeData(mimeData);
  drag->setPixmap(CreateDragIcon(getPropertyName(), path.getDataType()));
  drag->exec(Qt::CopyAction);

  // drag->exec is a blocking method
  resetStyle();
  Q_EMIT endViewPaths();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void DataArrayPathSelectionWidget::setFilter(AbstractFilter* filter)
{
  m_Filter = filter;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void DataArrayPathSelectionWidget::beforePreflight()
{
  //  m_SelectionMenu = createSelectionMenu();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void DataArrayPathSelectionWidget::afterPreflight()
{
  if(nullptr == m_Filter)
  {
    return;
  }

  if(nullptr != m_Filter && m_Filter->getDataContainerArray())
  {
    return;
  }

  DataContainerArray::Pointer dca = m_Filter->getDataContainerArray();
  bool found = false;
  switch(getDataType())
  {
  case DataArrayPathHelper::DataType::DataContainer:
    if(dca->getDataContainer(getDataArrayPath()))
    {
      found = true;
    }
    break;
  case DataArrayPathHelper::DataType::AttributeMatrix:
    if(dca->getAttributeMatrix(getDataArrayPath()))
    {
      found = true;
    }
    break;
  case DataArrayPathHelper::DataType::DataArray: {
    AttributeMatrix::Pointer am = dca->getAttributeMatrix(getDataArrayPath());
    if(am && am->getAttributeArray(getDataArrayPath().getDataArrayName()))
    {
      found = true;
    }
  }
  break;
  case DataArrayPathHelper::DataType::None:
    break;
  }

  // Update StyleSheet
  if(found)
  {
    setState(State::Normal);
  }
  else
  {
    setState(State::NotFound);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void DataArrayPathSelectionWidget::updateCheckState(bool checked)
{
  if(checked)
  {
    Q_EMIT dataArrayPathSelectionLocked(this);
    emitRequirements();
  }
  else
  {
    Q_EMIT dataArrayPathSelectionUnlocked(this);
    m_FilteringPassed = false;

    if(checkCurrentPath())
    {
      setState(State::Normal);
    }
    else
    {
      setState(State::NotFound);
    }
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void DataArrayPathSelectionWidget::setPathFiltering(bool active)
{
  setChecked(active);
  updateCheckState(active);
  m_FilteringPassed = false;

  if(!active && !underMouse())
  {
    if(checkCurrentPath())
    {
      setState(State::Normal);
    }
    else
    {
      setState(State::NotFound);
    }
  }

  repaint();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void DataArrayPathSelectionWidget::resetStyle()
{
  setPathFiltering(false);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
const QColor DataArrayPathSelectionWidget::getBorderColor(State state) const
{
  SVStyle* style = SVStyle::Instance();
  QColor color = "#000000";

  switch(state)
  {
  case State::Normal:
    color = style->getDataArrayPath_border_normal();
    break;
  case State::Active:
    color = GetActiveColor(m_DataType);
    break;
  case State::NotFound:
    color = style->getDataArrayPath_border_not_found();
    break;
  case State::DragEnabled:
    color = style->getDataArrayPath_border_drag_enabled();
    break;
  case State::DragDisabled:
    color = style->getDataArrayPath_border_drag_disabled();
    break;
  }

  return color;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
DataArrayPathSelectionWidget::State DataArrayPathSelectionWidget::getState() const
{
  return m_State;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void DataArrayPathSelectionWidget::setState(State styleType)
{
  m_State = styleType;

  // Force update the styling
  style()->unpolish(this);
  style()->polish(this);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int DataArrayPathSelectionWidget::getXMargin() const
{
  ensurePolished();

  // This roundabout calculation is to solve the problem where methods designed to return the margins or content rect
  // after the stylesheet has been applied are not returning the correct result.
  int xMargin;
  {
    QStyleOptionButton option;
    option.initFrom(this);

    QRect contentsRect1 = style()->subElementRect(QStyle::SE_PushButtonLayoutItem, &option, this);
    QRect contentsRect2 = style()->subElementRect(QStyle::SE_PushButtonContents, &option, this);

    int width1 = contentsRect1.width();
    int width2 = contentsRect2.width();
    xMargin = (width1 - width2) / 2;
  }

  return xMargin;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int DataArrayPathSelectionWidget::getYMargin() const
{
  ensurePolished();

  // This roundabout calculation is to solve the problem where methods designed to return the margins or content rect
  // after the stylesheet has been applied are not returning the correct result.
  int yMargin;
  {
    QStyleOptionButton option;
    option.initFrom(this);

    QRect contentsRect1 = style()->subElementRect(QStyle::SE_PushButtonLayoutItem, &option, this);
    QRect contentsRect2 = style()->subElementRect(QStyle::SE_PushButtonContents, &option, this);

    int height1 = contentsRect1.height();
    int height2 = contentsRect2.height();
    yMargin = (height1 - height2) / 2;
  }

  return yMargin;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QRect DataArrayPathSelectionWidget::getStyledContentsRect() const
{
  ensurePolished();

  QStyleOptionButton option;
  option.initFrom(this);

  return style()->subElementRect(QStyle::SE_PushButtonContents, &option, this);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QRect DataArrayPathSelectionWidget::getStyledBorderRect() const
{
  ensurePolished();

  QStyleOptionButton option;
  option.initFrom(this);

  QRect styledRect = style()->subElementRect(QStyle::SE_PushButtonLayoutItem, &option, this);
  if(styledRect.isValid())
  {
    return styledRect;
  }

  // return the standard rect if the styled rect is not valid
  return rect();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void DataArrayPathSelectionWidget::paintEvent(QPaintEvent* event)
{
  QToolButton::paintEvent(event);

  if(!(isEnabled() && (isChecked() || m_FilteringPassed)))
  {
    return;
  }

  ensurePolished();

  int yMargin = getYMargin();
  if(yMargin < 0)
  {
    yMargin = 0;
  }

  // Use yMargin because no method designed to return margins or content rect after the stylesheet has been applied
  // returns the correct value.
  int rectWidth = getStyledBorderRect().height() + 1;
  int penWidth = 1;
  int radius = 4;
  QRect rect(width() - rectWidth, yMargin / 2, rectWidth, rectWidth - yMargin);
  QColor penColor(getBorderColor(State::Active));
  QColor fillColor(getBorderColor(State::Active));
  if(getState() == State::NotFound)
  {
    penColor = getBorderColor(State::NotFound);
  }
  if(!isEnabled())
  {
    fillColor = QColor(221, 221, 221);
  }

  QPen pen;
  penColor.setAlpha(50);
  pen.setColor(penColor);
  pen.setWidth(penWidth);
  QPainter painter{this};

#if 0
  QPainterPath ppath;
  ppath.addRoundedRect(rect, radius, radius);
  painter.setPen(pen);
  painter.setBrush(fillColor);
  painter.fillPath(ppath, fillColor);
#else
  painter.setPen(pen);
  painter.setBrush(fillColor);
  painter.drawRoundedRect(rect, radius, radius);
  painter.drawRect(rect.x(), rect.y(), radius, rect.height());
#endif
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QSize DataArrayPathSelectionWidget::updatedSizeHint(QSize sizeHint) const
{
  int xMargin = getXMargin();

  // Check Width
  QFontMetrics fm(font());
  int textPadding = xMargin;
  int textWidth = fm.QFONTMETRICS_WIDTH(text()) + textPadding;
  int minWidth = textWidth + xMargin + (2 * sizeHint.height());
  if(sizeHint.width() < minWidth)
  {
    sizeHint.setWidth(minWidth);
  }

  return sizeHint;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QSize DataArrayPathSelectionWidget::minimumSizeHint() const
{
  ensurePolished();
  QSize minHint = updatedSizeHint(QToolButton::minimumSizeHint());

  return minHint;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QSize DataArrayPathSelectionWidget::sizeHint() const
{
  ensurePolished();
  QSize hint = updatedSizeHint(QToolButton::sizeHint());

  return hint;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString DataArrayPathSelectionWidget::getPropertyName() const
{
  return m_PropName;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void DataArrayPathSelectionWidget::setPropertyName(QString propName)
{
  m_PropName = propName;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
bool DataArrayPathSelectionWidget::isCreatedPath(const DataArrayPath& path) const
{
  if(nullptr == m_Filter)
  {
    return true;
  }

  std::list<DataArrayPath> createdPaths = m_Filter->getCreatedPaths();
  return std::find(createdPaths.begin(), createdPaths.end(), path) != createdPaths.end();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void DataArrayPathSelectionWidget::showContextMenu(const QPoint& pos)
{
  // Do not create the menu if the widget is disabled
  if(!isEnabled())
  {
    return;
  }

  if(nullptr == m_SelectionMenu)
  {
    m_SelectionMenu = createSelectionMenu();
  }

  m_SelectionMenu->exec(mapToGlobal(pos));
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QMenu* DataArrayPathSelectionWidget::createSelectionMenu()
{
  if(nullptr == m_Filter || DataArrayPathHelper::DataType::None == m_DataType)
  {
    return nullptr;
  }

  DataContainerArray::Pointer dca = m_Filter->getDataContainerArray();
  if(nullptr == dca.get())
  {
    return nullptr;
  }

  QMenu* menu = new QMenu(this);
  DataArrayPath path;

  // Populate Menu with DataContainers
  DataContainerArray::Container containers = dca->getDataContainers();
  for(const auto& dc : containers)
  {
    path.setDataContainerName(dc->getName());
    if(isCreatedPath(path))
    {
      continue;
    }

    if(DataArrayPathHelper::DataType::DataContainer == m_DataType)
    {
      QAction* action = menu->addAction(dc->getName());
      action->setEnabled(checkPathReqs(path));
      connect(action, &QAction::triggered, this, [=] { setDataArrayPath(path); });
    }
    else
    {
      QMenu* dcMenu = new QMenu(dc->getName());
      menu->addMenu(dcMenu);
      bool dcMenuEnabled = false;

      for(const AttributeMatrix::Pointer& am : dc->getChildren())
      {
        path.setAttributeMatrixName(am->getName());
        if(isCreatedPath(path))
        {
          continue;
        }

        // Populate DataContainer menu
        if(DataArrayPathHelper::DataType::AttributeMatrix == m_DataType)
        {
          QAction* action = dcMenu->addAction(am->getName());
          action->setEnabled(checkPathReqs(path));
          connect(action, &QAction::triggered, this, [=] { setDataArrayPath(path); });

          if(checkPathReqs(path))
          {
            dcMenuEnabled = true;
          }
        }
        else
        {
          QMenu* amMenu = new QMenu(am->getName());
          dcMenu->addMenu(amMenu);
          bool amMenuEnabled = false;

          // Populate AttributeMatrix menu
          QList<QString> attrArrayNames = am->getAttributeArrayNames();
          for(const auto& daName : attrArrayNames)
          {
            path.setDataArrayName(daName);
            if(isCreatedPath(path))
            {
              continue;
            }

            QAction* action = amMenu->addAction(daName);
            action->setEnabled(checkPathReqs(path));
            connect(action, &QAction::triggered, this, [=] { setDataArrayPath(path); });

            if(checkPathReqs(path))
            {
              amMenuEnabled = true;
            }
          } // End DataArray

          amMenu->setEnabled(amMenuEnabled);

          if(amMenuEnabled)
          {
            dcMenuEnabled = true;
          }
        }
      } // End AttributeMatrix

      dcMenu->setEnabled(dcMenuEnabled);
    }
  } // End DataContainer

  return menu;
}
