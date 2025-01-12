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

#include "ComparisonSelectionAdvancedWidget.h"

#include "SIMPLib/Utilities/FilterCompatibility.hpp"

#include "SVWidgetsLib/Core/SVWidgetsLibConstants.h"
#include "SVWidgetsLib/Widgets/SVStyle.h"

#include "FilterParameterWidgetUtils.hpp"
#include "FilterParameterWidgetsDialogs.h"

#include "SIMPLib/DataContainers/DataContainerArray.h"
#include "SIMPLib/Filtering/ComparisonValue.h"

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
ComparisonSelectionAdvancedWidget::ComparisonSelectionAdvancedWidget(FilterParameter* parameter, AbstractFilter* filter, QWidget* parent)
: FilterParameterWidget(parameter, filter, parent)
, m_DidCausePreflight(false)
{
  m_FilterParameter = SIMPL_FILTER_PARAMETER_COMPATIBILITY_CHECK(filter, parameter, ComparisonSelectionAdvancedWidget, ComparisonSelectionAdvancedFilterParameter);

  QString filterName = getFilter()->getNameOfClass();

  setupUi(this);
  setupGui();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
ComparisonSelectionAdvancedWidget::~ComparisonSelectionAdvancedWidget() = default;

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
ComparisonInputsAdvanced ComparisonSelectionAdvancedWidget::getComparisonInputs()
{
  ComparisonInputsAdvanced comps;
  if(comparisonSetWidget == nullptr)
  {
    return comps;
  }

  DataArrayPath amPath = DataArrayPath::Deserialize(m_SelectedAttributeMatrixPath->text(), Detail::Delimiter);
  comps.setDataContainerName(amPath.getDataContainerName());
  comps.setAttributeMatrixName(amPath.getAttributeMatrixName());

  QString dcName = comps.getDataContainerName();
  QString amName = comps.getAttributeMatrixName();

  comps.setInvert(comparisonSetWidget->getComparisonSet()->getInvertComparison());

  QVector<AbstractComparison::Pointer> comparisons = comparisonSetWidget->getComparisons();

  for(int i = 0; i < comparisons.size(); ++i)
  {
    comps.addInput(comparisons[i]);
  }
  return comps;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ComparisonSelectionAdvancedWidget::setupGui()
{
  if(getFilter() == nullptr)
  {
    return;
  }
  if(getFilterParameter() == nullptr)
  {
    return;
  }

  AttributeMatrixSelectionFilterParameter::RequirementType reqs;
  m_SelectedAttributeMatrixPath->setAttrMatrixRequirements(reqs);
  m_SelectedAttributeMatrixPath->setFilter(getFilter());

  // Catch when the filter is about to execute the preflight
  connect(getFilter(), &AbstractFilter::preflightAboutToExecute, this, &ComparisonSelectionAdvancedWidget::beforePreflight);

  // Catch when the filter is finished running the preflight
  connect(getFilter(), &AbstractFilter::preflightExecuted, this, &ComparisonSelectionAdvancedWidget::afterPreflight);

  // Catch when the filter wants its values updated
  connect(getFilter(), &AbstractFilter::updateFilterParameters, this, &ComparisonSelectionAdvancedWidget::filterNeedsInputParameters);

  // If the DataArrayPath is updated in the filter, update the widget
  connect(getFilter(), SIGNAL(dataArrayPathUpdated(QString, DataArrayPath::RenameType)), this, SLOT(updateDataArrayPath(QString, DataArrayPath::RenameType)));

  connect(this, SIGNAL(filterPathInput(DataArrayPath)), m_SelectedAttributeMatrixPath, SLOT(checkDragPath(DataArrayPath)));
  connect(this, SIGNAL(endViewPathRequirements()), m_SelectedAttributeMatrixPath, SLOT(clearPathFiltering()));
  connect(this, SIGNAL(endDataStructureRequirements()), m_SelectedAttributeMatrixPath, SLOT(endExternalFiltering()));
  connect(this, SIGNAL(applyPathToFilteringParameter(DataArrayPath)), m_SelectedAttributeMatrixPath, SLOT(setFilteredDataArrayPath(DataArrayPath)));

  connect(m_SelectedAttributeMatrixPath, SIGNAL(viewPathsMatchingReqs(AttributeMatrixSelectionFilterParameter::RequirementType)), this,
          SIGNAL(viewPathsMatchingReqs(AttributeMatrixSelectionFilterParameter::RequirementType)));
  connect(m_SelectedAttributeMatrixPath, SIGNAL(endViewPaths()), this, SIGNAL(endViewPaths()));
  // connect(m_SelectedAttributeMatrixPath, SIGNAL(pathChanged()), this, SIGNAL(parametersChanged()));
  connect(m_SelectedAttributeMatrixPath, SIGNAL(filterPath(DataArrayPath)), this, SIGNAL(filterPath(DataArrayPath)));
  connect(m_SelectedAttributeMatrixPath, SIGNAL(pathChanged()), this, SLOT(attributeMatrixUpdated()));

  connect(m_SelectedAttributeMatrixPath, SIGNAL(dataArrayPathSelectionLocked(QToolButton*)), this, SIGNAL(dataArrayPathSelectionLocked(QToolButton*)));
  connect(this, SIGNAL(lockDataArrayPathSelection(QToolButton*)), m_SelectedAttributeMatrixPath, SLOT(selectionWidgetLocked(QToolButton*)));
  connect(m_SelectedAttributeMatrixPath, SIGNAL(dataArrayPathSelectionUnlocked(QToolButton*)), this, SIGNAL(dataArrayPathSelectionUnlocked(QToolButton*)));
  connect(this, SIGNAL(unlockDataArrayPathSelection(QToolButton*)), m_SelectedAttributeMatrixPath, SLOT(selectionWidgetUnlocked(QToolButton*)));

  // Create the Comparison Set
  comparisonSetWidget->setComparisonSet(ComparisonSet::New());
  connect(comparisonSetWidget, SIGNAL(comparisonChanged()), this, SIGNAL(parametersChanged()));

  // Copy the data into the Comparison Set
  ComparisonInputsAdvanced comps = dynamic_cast<ComparisonSelectionAdvancedFilterParameter*>(getFilterParameter())->getGetterCallback()();

  DataArrayPath defaultPath = getFilter()->property(PROPERTY_NAME_AS_CHAR).value<DataArrayPath>();
  m_SelectedAttributeMatrixPath->setText(defaultPath.serialize(Detail::Delimiter));
  QString matrixPropertyName = getFilterParameter()->getHumanLabel();
  matrixPropertyName = matrixPropertyName.replace("Arrays", "Matrix");
  matrixPropertyName = matrixPropertyName.replace("arrays", "matrix");
  m_SelectedAttributeMatrixPath->setPropertyName(matrixPropertyName);

  DataArrayPath currentPath;
  currentPath.setDataContainerName(comps.getDataContainerName());
  currentPath.setAttributeMatrixName(comps.getAttributeMatrixName());
  if(!currentPath.isEmpty())
  {
    QStringList names = generateAttributeArrayList(currentPath.getDataContainerName(), currentPath.getAttributeMatrixName());
    presetAttributeMatrix(currentPath);
  }
  comparisonSetWidget->setComparisons(comps.getInputs());

#if 0
  // is the filter parameter tied to a boolean property of the Filter Instance, if it is then we need to make the check box visible
  if (getFilterParameter()->isConditional() == true)
  {
    bool boolProp = getFilter()->property(getFilterParameter()->getConditionalProperty().toLatin1().constData()).toBool();
    conditionalCB->setChecked(boolProp);
    conditionalCB->setText(getFilterParameter()->getConditionalLabel());
    dataContainerList->setEnabled(boolProp);
    attributeMatrixList->setEnabled(boolProp);
    attributeArrayList->setEnabled(boolProp);
  }
  else
  {
    widgetLayout->removeWidget(conditionalCB);
    conditionalCB->deleteLater();
    widgetLayout->removeWidget(linkLeft);
    linkLeft->deleteLater();
    widgetLayout->removeWidget(linkRight);
    linkRight->deleteLater();
  }
#endif
}

#if 0
// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ComparisonSelectionAdvancedWidget::on_conditionalCB_stateChanged(int state)
{
  bool boolProp = conditionalCB->isChecked();
  dataContainerList->setEnabled(boolProp);
  dataContainerList->setVisible(boolProp);
  attributeMatrixList->setEnabled(boolProp);
  attributeMatrixList->setVisible(boolProp);
  attributeArrayList->setEnabled(boolProp);
  attributeArrayList->setVisible(boolProp);

  label->setVisible(boolProp);
  linkLeft->setVisible(boolProp);
  linkRight->setVisible(boolProp);
  m_DidCausePreflight = true;
  Q_EMIT parametersChanged();
  m_DidCausePreflight = false;

}
#endif

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QStringList ComparisonSelectionAdvancedWidget::generateAttributeArrayList(const QString& currentDCName, const QString& currentAttrMatName)
{
  DataArrayPath path(currentDCName, currentAttrMatName, "");
  AttributeMatrix::Pointer am = getFilter()->getDataContainerArray()->getAttributeMatrix(path);

  if(nullptr == am)
  {
    return QStringList();
  }

  return am->getAttributeArrayNames();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString ComparisonSelectionAdvancedWidget::checkStringValues(QString curDcName, QString filtDcName)
{
  if(curDcName.isEmpty() && !filtDcName.isEmpty())
  {
    return filtDcName;
  }
  if(!curDcName.isEmpty() && filtDcName.isEmpty())
  {
    return curDcName;
  }
  if(!curDcName.isEmpty() && !filtDcName.isEmpty() && m_DidCausePreflight)
  {
    return curDcName;
  }

  return filtDcName;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ComparisonSelectionAdvancedWidget::widgetChanged(const QString& text)
{
  Q_UNUSED(text)
  m_DidCausePreflight = true;
  Q_EMIT parametersChanged();
  m_DidCausePreflight = false;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ComparisonSelectionAdvancedWidget::setComparisons(QVector<AbstractComparison::Pointer> comparisons)
{
  qint32 count = comparisons.size();

  for(int i = 0; i < count; ++i)
  {
    comparisonSetWidget->addComparison(comparisons[i]);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ComparisonSelectionAdvancedWidget::filterNeedsInputParameters(AbstractFilter* filter)
{
  Q_UNUSED(filter)
  ComparisonSelectionAdvancedFilterParameter::SetterCallbackType setter = m_FilterParameter->getSetterCallback();
  if(setter)
  {
    setter(getComparisonInputs());
  }
  else
  {
    getFilter()->notifyMissingProperty(getFilterParameter());
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ComparisonSelectionAdvancedWidget::beforePreflight()
{
  if(nullptr == getFilter())
  {
    return;
  }
  if(m_DidCausePreflight)
  {
    // std::cout << "***  ComparisonSelectionAdvancedWidget already caused a preflight, just returning" << std::endl;
    return;
  }

  if(m_SelectedAttributeMatrixPath->text().isEmpty())
  {
    populateButtonText();
  }

  m_SelectedAttributeMatrixPath->beforePreflight();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ComparisonSelectionAdvancedWidget::afterPreflight()
{
  DataContainerArray::Pointer dca = getFilter()->getDataContainerArray();
  if(nullptr == dca)
  {
    return;
  }

  if(dca->doesAttributeMatrixExist(m_SelectedAttributeMatrixPath->getDataArrayPath()))
  {
    AttributeMatrix::Pointer am = dca->getAttributeMatrix(m_SelectedAttributeMatrixPath->getDataArrayPath());
    if(nullptr != am.get())
    {
      DataArrayPath path = m_SelectedAttributeMatrixPath->getDataArrayPath();
      QStringList arrayNames = generateAttributeArrayList(path.getDataContainerName(), path.getAttributeMatrixName());

      comparisonSetWidget->setArrayNames(arrayNames);

      if(nullptr == comparisonSetWidget->getAttributeMatrix())
      {
        ComparisonInputsAdvanced comps = m_FilterParameter->getGetterCallback()();
        comparisonSetWidget->setAttributeMatrix(am);
        comparisonSetWidget->setComparisons(comps.getInputs());
      }
    }
  }

  m_SelectedAttributeMatrixPath->afterPreflight();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ComparisonSelectionAdvancedWidget::populateButtonText()
{
  //  std::cout << "ComparisonSelectionWidget::populateComboBoxes()" << std::endl;

  // Now get the DataContainerArray from the Filter instance
  // We are going to use this to get all the current DataContainers
  DataContainerArray::Pointer dca = getFilter()->getDataContainerArray();
  if(nullptr == dca)
  {
    return;
  }

  // Check to see if we have any DataContainers to actually populate drop downs with.
  if(dca->getDataContainers().empty())
  {
    return;
  }

  // Cache the DataContainerArray Structure for our use during all the selections
  m_DcaProxy = DataContainerArrayProxy(dca.get());

  // Grab what is currently selected
  QString curDcName = "";
  QString curAmName = "";

  // Get what is in the filter
  ComparisonInputsAdvanced comps = m_FilterParameter->getGetterCallback()();

  QString filtDcName = comps.getDataContainerName();
  QString filtAmName = comps.getAttributeMatrixName();

  QString dcName = checkStringValues(curDcName, filtDcName);
  QString amName = checkStringValues(curAmName, filtAmName);

  m_SelectedAttributeMatrixPath->setText(dcName + Detail::Delimiter + amName);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
bool ComparisonSelectionAdvancedWidget::eventFilter(QObject* obj, QEvent* event)
{
  if(event->type() == QEvent::Show && obj == m_SelectedAttributeMatrixPath->menu())
  {
    QPoint pos = adjustedMenuPosition(m_SelectedAttributeMatrixPath);
    m_SelectedAttributeMatrixPath->menu()->move(pos);
    return true;
  }
  return false;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ComparisonSelectionAdvancedWidget::attributeMatrixUpdated()
{
  DataArrayPath path = m_SelectedAttributeMatrixPath->getDataArrayPath();
  attributeMatrixSelected(path.serialize(Detail::Delimiter));
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ComparisonSelectionAdvancedWidget::attributeMatrixSelected(QString path)
{
  setSelectedPath(path);

  m_DidCausePreflight = true;
  Q_EMIT parametersChanged();
  m_DidCausePreflight = false;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ComparisonSelectionAdvancedWidget::setSelectedPath(QString path)
{
  DataArrayPath amPath = DataArrayPath::Deserialize(path, Detail::Delimiter);
  setSelectedPath(amPath);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ComparisonSelectionAdvancedWidget::setSelectedPath(DataArrayPath amPath)
{
  if(amPath.isEmpty())
  {
    return;
  }

  DataContainerArray::Pointer dca = getFilter()->getDataContainerArray();
  if(nullptr == dca)
  {
    return;
  }

  if(dca->doesAttributeMatrixExist(amPath))
  {
    AttributeMatrix::Pointer am = dca->getAttributeMatrix(amPath);

    if(nullptr != comparisonSetWidget->getComparisonSet())
    {
      comparisonSetWidget->setAttributeMatrix(am);
      comparisonSetWidget->setComparisonSet(ComparisonSet::New());
    }
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ComparisonSelectionAdvancedWidget::presetAttributeMatrix(DataArrayPath amPath)
{
  m_presetPath = amPath;
  m_SelectedAttributeMatrixPath->setText(amPath.serialize(Detail::Delimiter));
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ComparisonSelectionAdvancedWidget::updateDataArrayPath(QString propertyName, const DataArrayPath::RenameType& renamePath)
{
  if(propertyName == getFilterParameter()->getPropertyName())
  {
    DataArrayPath oldPath;
    DataArrayPath newPath;
    std::tie(oldPath, newPath) = renamePath;

    ComparisonInputsAdvanced inputs = m_FilterParameter->getGetterCallback()();
    DataArrayPath amPath = inputs.getAttributeMatrixPath();
    AbstractComparison::Pointer input = inputs[0];

    blockSignals(true);
    // Update the AttributeMatrix path
    {
      DataContainerArray::Pointer dca = getFilter()->getDataContainerArray();
      if(nullptr == dca.get())
      {
        return;
      }

      m_SelectedAttributeMatrixPath->setText(amPath.serialize(Detail::Delimiter));
    }

    // Update the DataArray choices
    if(!oldPath.getDataArrayName().isEmpty() && oldPath.getDataArrayName() != newPath.getDataArrayName())
    {
      comparisonSetWidget->renameDataArrayPath(renamePath);
    }
    blockSignals(false);
  }
}
