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
#include "DynamicTableWidget.h"

#include <QtWidgets/QMessageBox>

#include <QtCore/QDebug>

#include "SIMPLib/FilterParameters/DynamicTableData.h"
#include "SIMPLib/FilterParameters/DynamicTableFilterParameter.h"
#include "SIMPLib/FilterParameters/FilterParameter.h"
#include "SIMPLib/Plugin/ISIMPLibPlugin.h"
#include "SIMPLib/Utilities/FilterCompatibility.hpp"

#include "FilterParameterWidgetsDialogs.h"

#include "SVWidgetsLib/Core/SVWidgetsLibConstants.h"
#include "SVWidgetsLib/FilterParameterWidgets/DynamicTableItemDelegate.h"
#include "SVWidgetsLib/Widgets/SVStyle.h"

const QString addRowTT = "Adds a row to the table.";
const QString addColTT = "Adds a column to the table.";
const QString deleteRowTT = "Removes the currently selected row from the table.";
const QString deleteColTT = "Removes the currently selected column from the table.";

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
DynamicTableWidget::DynamicTableWidget(FilterParameter* parameter, AbstractFilter* filter, QWidget* parent)
: FilterParameterWidget(parameter, filter, parent)
{
  m_FilterParameter = SIMPL_FILTER_PARAMETER_COMPATIBILITY_CHECK(filter, parameter, DynamicTableWidget, DynamicTableFilterParameter);

  setupUi(this);
  setupGui();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
DynamicTableWidget::~DynamicTableWidget()
{
  delete m_ItemDelegate;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void DynamicTableWidget::initializeWidget(FilterParameter* parameter, AbstractFilter* filter)
{
  setFilter(filter);
  setFilterParameter(parameter);
  setupGui();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void DynamicTableWidget::setFilterParameter(FilterParameter* value)
{
  m_FilterParameter = dynamic_cast<DynamicTableFilterParameter*>(value);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
FilterParameter* DynamicTableWidget::getFilterParameter() const
{
  return m_FilterParameter;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void DynamicTableWidget::setupGui()
{
  // Catch when the filter is about to execute the preflight
  connect(getFilter(), &AbstractFilter::preflightAboutToExecute, this, &DynamicTableWidget::beforePreflight);

  // Catch when the filter is finished running the preflight
  connect(getFilter(), &AbstractFilter::preflightExecuted, this, &DynamicTableWidget::afterPreflight);

  // Catch when the filter wants its values updated
  connect(getFilter(), &AbstractFilter::updateFilterParameters, this, &DynamicTableWidget::filterNeedsInputParameters);

  tableLabel->setText(m_FilterParameter->getHumanLabel());

  dynamicTable->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
  dynamicTable->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

  // Set the item delegate so that we can only enter 'double' values into the table
  m_ItemDelegate = new DynamicTableItemDelegate;
  dynamicTable->setItemDelegate(m_ItemDelegate);

  // Set button tooltips
  addRowBtn->setToolTip(addRowTT);
  addColBtn->setToolTip(addColTT);
  deleteRowBtn->setToolTip(deleteRowTT);
  deleteColBtn->setToolTip(deleteColTT);

  // Populate the table
  populateTable();

  dynamicTable->resizeRowsToContents();

  updateButtonStyles();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void DynamicTableWidget::updateButtonStyles()
{
  // Set Style
}
// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void DynamicTableWidget::on_dynamicTable_cellChanged(int /* row */, int /* col */)
{
  m_DidCausePreflight = true;
  Q_EMIT parametersChanged();
  m_DidCausePreflight = false;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void DynamicTableWidget::filterNeedsInputParameters(AbstractFilter* filter)
{
  QStringList rHeaders, cHeaders;
  for(int i = 0; i < dynamicTable->rowCount(); i++)
  {
    QTableWidgetItem* vItem = dynamicTable->verticalHeaderItem(i);
    if(nullptr != vItem)
    {
      QString vName = vItem->data(Qt::DisplayRole).toString();
      rHeaders << vName;
    }
  }
  for(int i = 0; i < dynamicTable->columnCount(); i++)
  {
    QTableWidgetItem* cItem = dynamicTable->horizontalHeaderItem(i);
    if(nullptr != cItem)
    {
      QString cName = cItem->data(Qt::DisplayRole).toString();
      cHeaders << cName;
    }
  }

  DynamicTableData data = m_FilterParameter->getGetterCallback()();
  data.setTableData(getData());
  data.setRowHeaders(rHeaders);
  data.setColHeaders(cHeaders);

  Q_UNUSED(filter)
  DynamicTableFilterParameter::SetterCallbackType setter = m_FilterParameter->getSetterCallback();
  if(setter)
  {
    setter(data);
  }
  else
  {
    getFilter()->notifyMissingProperty(getFilterParameter());
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
std::vector<std::vector<double>> DynamicTableWidget::getData()
{
  int rCount = dynamicTable->rowCount(), cCount = dynamicTable->columnCount();
  std::vector<std::vector<double>> data(rCount, std::vector<double>(cCount));

  for(int row = 0; row < rCount; row++)
  {
    for(int col = 0; col < cCount; col++)
    {
      bool ok = false;
      QTableWidgetItem* item = dynamicTable->item(row, col);
      if(nullptr == item)
      {
        return std::vector<std::vector<double>>();
      }
      data[row][col] = item->data(Qt::DisplayRole).toDouble(&ok);

      if(!ok)
      {
        qDebug() << "Could not set the model data into the DynamicTableData object.";
        data.clear();
        return data;
      }
    }
  }

  return data;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void DynamicTableWidget::beforePreflight()
{
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void DynamicTableWidget::afterPreflight()
{
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void DynamicTableWidget::on_addRowBtn_clicked()
{
  int row = dynamicTable->rowCount();

  // If we are adding the first row, add the first column too.
  if(row <= 0 && dynamicTable->columnCount() <= 0)
  {
    dynamicTable->insertColumn(0);
    dynamicTable->setHorizontalHeaderItem(0, new QTableWidgetItem("0"));
  }

  dynamicTable->insertRow(row);
  dynamicTable->setVerticalHeaderItem(row, new QTableWidgetItem(QString::number(dynamicTable->rowCount() - 1)));

  dynamicTable->blockSignals(true);
  for(int col = 0; col < dynamicTable->columnCount(); col++)
  {
    if(col + 1 == dynamicTable->columnCount())
    {
      // Only fire the signal after the last new item has been created
      dynamicTable->blockSignals(false);
    }

    QTableWidgetItem* item = new QTableWidgetItem("0");
    dynamicTable->setItem(row, col, item);
  }

  // Update buttons
  updateDynamicButtons();

  // Renumber dynamic headers
  renumberDynamicHeaders();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void DynamicTableWidget::on_addColBtn_clicked()
{
  int col = dynamicTable->columnCount();

  // If we are adding the first column, add the first row too.
  if(col <= 0 && dynamicTable->rowCount() <= 0)
  {
    dynamicTable->insertRow(0);
    dynamicTable->setVerticalHeaderItem(0, new QTableWidgetItem("0"));
  }

  dynamicTable->insertColumn(col);
  dynamicTable->setHorizontalHeaderItem(col, new QTableWidgetItem(QString::number(dynamicTable->columnCount() - 1)));

  dynamicTable->blockSignals(true);
  for(int row = 0; row < dynamicTable->rowCount(); row++)
  {
    if(row + 1 == dynamicTable->rowCount())
    {
      // Only fire the signal after the last new item has been created
      dynamicTable->blockSignals(false);
    }

    QTableWidgetItem* item = new QTableWidgetItem("0");
    dynamicTable->setItem(row, col, item);
  }

  // Update buttons
  updateDynamicButtons();

  // Renumber dynamic headers
  renumberDynamicHeaders();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void DynamicTableWidget::on_deleteRowBtn_clicked()
{
  DynamicTableData data = m_FilterParameter->getGetterCallback()();

  dynamicTable->removeRow(dynamicTable->currentRow());

  if(dynamicTable->rowCount() <= 0 && data.getDynamicCols())
  {
    while(dynamicTable->columnCount() > 0)
    {
      dynamicTable->removeColumn(0);
    }
  }

  // Update buttons
  updateDynamicButtons();

  // Renumber dynamic headers
  renumberDynamicHeaders();

  // Cause a preflight
  on_dynamicTable_cellChanged(0, 0);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void DynamicTableWidget::on_deleteColBtn_clicked()
{
  DynamicTableData data = m_FilterParameter->getGetterCallback()();

  dynamicTable->removeColumn(dynamicTable->currentColumn());

  if(dynamicTable->columnCount() <= 0 && data.getDynamicRows())
  {
    while(dynamicTable->rowCount() > 0)
    {
      dynamicTable->removeRow(0);
    }
  }

  // Update buttons
  updateDynamicButtons();

  // Renumber dynamic headers
  renumberDynamicHeaders();

  // Cause a preflight
  on_dynamicTable_cellChanged(0, 0);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void DynamicTableWidget::populateTable()
{
  // Get what is in the filter
  DynamicTableData data = m_FilterParameter->getGetterCallback()();

  if(m_FilterParameter != nullptr)
  {
    // If the filter parameter generated an error, empty the data object and display the error.
    if(m_FilterParameter->getErrorCondition() < 0)
    {
      QString errorMessage = m_FilterParameter->getErrorMessage();
      QString pluginName = getFilter()->getPluginInstance()->getPluginBaseName();
      QString filterName = getFilter()->getHumanLabel();
      QString filterParameterName = m_FilterParameter->getPropertyName();
      QString vendorName = getFilter()->getPluginInstance()->getVendor();

      // Use HTML code in the error message, because this will be displayed in RichText format.
      errorMessage.prepend("The filter parameter '" + filterParameterName + "' in filter '" + filterName + "' has an error:<blockquote><b>");
      errorMessage.append("</b></blockquote>'" + filterName + "' is a part of the " + pluginName + " plugin.  ");
      errorMessage.append("Please contact " + vendorName + ", the developer of the " + pluginName + " plugin.");

      QMessageBox errorBox;
      errorBox.setIcon(QMessageBox::Critical);
      errorBox.setWindowTitle("'" + filterName + "' Error");
      errorBox.setText(errorMessage);
      errorBox.setStandardButtons(QMessageBox::Ok);
      errorBox.setDefaultButton(QMessageBox::Ok);
      errorBox.setTextFormat(Qt::RichText);
      errorBox.exec();
      return;
    }

    if(data.isEmpty()) // If there was nothing in the filter, use the defaults
    {
      data.setTableData(m_FilterParameter->getDefaultTableData().getTableData());
      data.setRowHeaders(m_FilterParameter->getDefaultTableData().getRowHeaders());
      data.setColHeaders(m_FilterParameter->getDefaultTableData().getColHeaders());
    }

    std::vector<std::vector<double>> tableData = data.getTableData();

    // Populate table with filter values
    int numRows = data.getNumRows();
    int numCols = data.getNumCols();
    if(data.getDefaultRowCount() > numRows)
    {
      numRows = data.getDefaultRowCount();
      if(data.getMinRows() > numRows)
      {
        numRows = data.getMinRows();
      }
    }
    if(data.getDefaultColCount() > numCols)
    {
      numCols = data.getDefaultColCount();
      if(data.getMinCols() > numCols)
      {
        numCols = data.getMinCols();
      }
    }
    for(int row = 0; row < numRows; row++)
    {
      dynamicTable->insertRow(row);
      for(int col = 0; col < numCols; col++)
      {
        if(dynamicTable->columnCount() == col)
        {
          dynamicTable->insertColumn(col);
        }

        QTableWidgetItem* item;
        if(row >= tableData.size() || col >= tableData[row].size())
        {
          item = new QTableWidgetItem("0");
        }
        else
        {
          item = new QTableWidgetItem(QString::number(tableData[row][col]));
        }
        dynamicTable->setItem(row, col, item);
      }
    }

    // Populate row and column headers
    populateHeaders();

    // Update the state of the Add/Remove buttons
    updateDynamicButtons();
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void DynamicTableWidget::populateHeaders()
{
  DynamicTableData data = m_FilterParameter->getGetterCallback()();

  if(!data.getDynamicRows())
  {
    dynamicTable->setVerticalHeaderLabels(data.getRowHeaders());
  }
  if(!data.getDynamicCols())
  {
    dynamicTable->setHorizontalHeaderLabels(data.getColHeaders());
  }

  renumberDynamicHeaders(); // Use this function to populate the dynamic headers for the first time
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void DynamicTableWidget::renumberDynamicHeaders()
{
  DynamicTableData data = m_FilterParameter->getGetterCallback()();

  if(data.getDynamicRows())
  {
    QStringList rHeaders;
    for(int i = 0; i < dynamicTable->rowCount(); i++)
    {
      rHeaders << QString::number(i);
    }
    dynamicTable->setVerticalHeaderLabels(rHeaders);
  }

  if(data.getDynamicCols())
  {
    QStringList cHeaders;
    for(int i = 0; i < dynamicTable->columnCount(); i++)
    {
      cHeaders << QString::number(i);
    }
    dynamicTable->setHorizontalHeaderLabels(cHeaders);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void DynamicTableWidget::updateDynamicButtons()
{
  DynamicTableData data = m_FilterParameter->getGetterCallback()();

  // Hide add/remove row buttons if row count is not dynamic
  if(!data.getDynamicRows())
  {
    addRowBtn->setHidden(true);
    deleteRowBtn->setHidden(true);
  }

  // Hide add/remove column buttons if column count is not dynamic
  if(!data.getDynamicCols())
  {
    addColBtn->setHidden(true);
    deleteColBtn->setHidden(true);
  }

  // Enable/Disable delete row button
  if(dynamicTable->rowCount() <= data.getMinRows())
  {
    deleteRowBtn->setDisabled(true);
    QString toolTip = "'" + m_FilterParameter->getHumanLabel() + "' must have at least " + QString::number(data.getMinRows()) + " rows.";
    if(data.getMinRows() == 1)
    {
      // Fix the grammar in the tooltip when there's only 1 row.
      toolTip.chop(2);
      toolTip.append('.');
    }
    deleteRowBtn->setToolTip(toolTip);
  }
  else
  {
    deleteRowBtn->setEnabled(true);
    deleteRowBtn->setToolTip(deleteRowTT);
  }

  // Enable/Disable delete column button
  if(dynamicTable->columnCount() <= data.getMinCols())
  {
    deleteColBtn->setDisabled(true);
    QString toolTip = "'" + m_FilterParameter->getHumanLabel() + "' must have at least " + QString::number(data.getMinCols()) + " columns.";
    if(data.getMinCols() == 1)
    {
      // Fix the grammar in the tooltip when there's only 1 column.
      toolTip.chop(2);
      toolTip.append('.');
    }
    deleteColBtn->setToolTip(toolTip);
  }
  else
  {
    deleteColBtn->setEnabled(true);
    deleteColBtn->setToolTip(deleteColTT);
  }
  updateButtonStyles();
}
