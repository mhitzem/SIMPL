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

#include "FilterInputWidget.h"

#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QVBoxLayout>

#include <QtCore/QDebug>
#include <QtCore/QTextStream>

#include "SIMPLib/Common/DocRequestManager.h"
#include "SIMPLib/FilterParameters/DataContainerReaderFilterParameter.h"
#include "SIMPLib/FilterParameters/InputFileFilterParameter.h"
#include "SIMPLib/FilterParameters/InputPathFilterParameter.h"
#include "SIMPLib/FilterParameters/LinkedBooleanFilterParameter.h"
#include "SIMPLib/FilterParameters/LinkedChoicesFilterParameter.h"
#include "SIMPLib/FilterParameters/LinkedDataContainerSelectionFilterParameter.h"
#include "SIMPLib/Plugin/ISIMPLibPlugin.h"
#include "SIMPLib/Utilities/SIMPLDataPathValidator.h"

#include "SVWidgetsLib/Widgets/SVStyle.h"

#include "SVWidgetsLib/FilterParameterWidgets/ChoiceWidget.h"
#include "SVWidgetsLib/FilterParameterWidgets/LinkedBooleanWidget.h"
#include "SVWidgetsLib/FilterParameterWidgets/LinkedDataContainerSelectionWidget.h"

#include "SVWidgetsLib/Core/FilterWidgetManager.h"
#include "SVWidgetsLib/Core/SVWidgetsLibConstants.h"
#include "SVWidgetsLib/Widgets/DataContainerArrayWidget.h"

#if 0
#include "ctkCollapsibleGroupBox.h"
#define QGroupBox ctkCollapsibleGroupBox
#endif

// Initialize private static member variable
QString FilterInputWidget::m_OpenDialogLastFilePath = "";

enum TabIndices
{
  BASIC_TAB,
  CURRENT_STRUCTURE_TAB,
  HELP_TAB
};

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
FilterInputWidget::FilterInputWidget(AbstractFilter::Pointer filter, QWidget* parent)
: QWidget(parent)
, m_Ui(new Ui::FilterInputWidget)
, m_FilterClassName(filter->getNameOfClass())
, m_AdvFadedOut(false)
{
  m_Ui->setupUi(this);
  setupGui();

  if(m_OpenDialogLastFilePath.isEmpty())
  {
    m_OpenDialogLastFilePath = QDir::homePath();
  }

  layoutWidgets(filter.get());
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
FilterInputWidget::~FilterInputWidget()
{
  delete m_VariablesVerticalLayout;
  delete m_VariablesWidget;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
bool FilterInputWidget::eventFilter(QObject* o, QEvent* e)
{
  if(e->type() == QEvent::Resize && (qobject_cast<QLabel*>(o) != nullptr) && m_Ui->brandingLabel == o)
  {
    QFontMetrics metrics(m_Ui->brandingLabel->font());
    QString elidedText = metrics.elidedText(m_BrandingLabel, Qt::ElideMiddle, m_Ui->brandingLabel->width());
    m_Ui->brandingLabel->setText(elidedText);
    return true;
  }
  return QWidget::eventFilter(o, e);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void FilterInputWidget::setupGui()
{
  //  QFont humanLabelFont = SVStyle::Instance()->GetHumanLabelFont();
  //  QFont brandingFont = SVStyle::Instance()->GetBrandingLabelFont();

  //  m_Ui->filterHumanLabel->setFont(humanLabelFont);
  //  m_Ui->filterIndex->setFont(humanLabelFont);

  QString releaseType = QString::fromLatin1(SIMPLViewProj_RELEASE_TYPE);
  if(releaseType.compare("Official") == 0)
  {
    m_Ui->brandingLabel->hide();
  }
  else
  {
    // m_Ui->brandingLabel->setFont(brandingFont);
    m_Ui->brandingLabel->installEventFilter(this);
  }

  connect(m_Ui->informationBtn, SIGNAL(clicked()), this, SLOT(showHelp()));
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void FilterInputWidget::setFilterIndex(const QString& index)
{
  m_Ui->filterIndex->setText(index);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void FilterInputWidget::toRunningState()
{
  QMapIterator<QString, QWidget*> iter(m_PropertyToWidget);
  while(iter.hasNext())
  {
    iter.next();
    iter.value()->setDisabled(true);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void FilterInputWidget::toIdleState()
{
  QMapIterator<QString, QWidget*> iter(m_PropertyToWidget);
  while(iter.hasNext())
  {
    iter.next();
    iter.value()->setDisabled(false);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void FilterInputWidget::layoutWidgets(AbstractFilter* filter)
{
  // If the filter is valid then instantiate all the FilterParameterWidgets
  // Create the Widget that will be placed into the Variables Scroll Area
  m_VariablesWidget = new QWidget(this);
  m_VariablesWidget->setObjectName("fiwVariablesWidget");
  m_VariablesWidget->setGeometry(QRect(0, 0, 250, 267));

  m_VariablesVerticalLayout = new QVBoxLayout(m_VariablesWidget);
  m_VariablesVerticalLayout->setSpacing(30);
  m_VariablesVerticalLayout->setMargin(0);

  QString groupBoxStyle;
  QTextStream ss(&groupBoxStyle);
  //  ss << "QGroupBox {";
  //  ss << "    font-weight: bold;";
  //  ss << "}";
  //  ss << "QGroupBox::title {";
  //  ss << "    subcontrol-origin: margin;";
  //  ss << "    subcontrol-position: top left;";
  //  ss << "    padding: 0 5px;";
  //  ss << "    font-weight: bold;";
  //  ss << "}";

  QGroupBox* parametersGroupBox = new QGroupBox("Parameters", this);
  QVBoxLayout* pLayout = new QVBoxLayout(parametersGroupBox);
  pLayout->setContentsMargins(0, 0, 0, 0);
  pLayout->setSpacing(0);
  // parametersGroupBox->setStyleSheet(groupBoxStyle);

  QGroupBox* requiredGroupBox = new QGroupBox("Required Objects", this);
  QVBoxLayout* rLayout = new QVBoxLayout(requiredGroupBox);
  rLayout->setContentsMargins(0, 0, 0, 0);
  rLayout->setSpacing(0);
  // requiredGroupBox->setStyleSheet(groupBoxStyle);

  QGroupBox* createdGroupBox = new QGroupBox("Created Objects", this);
  QVBoxLayout* cLayout = new QVBoxLayout(createdGroupBox);
  cLayout->setContentsMargins(0, 0, 0, 0);
  cLayout->setSpacing(0);
  // createdGroupBox->setStyleSheet(groupBoxStyle);

  QGroupBox* noCategoryGroupBox = new QGroupBox("Uncategorized", this);
  QVBoxLayout* nLayout = new QVBoxLayout(noCategoryGroupBox);
  nLayout->setContentsMargins(0, 0, 0, 0);
  nLayout->setSpacing(0);
  // noCategoryGroupBox->setStyleSheet(groupBoxStyle);

  // Get the FilterWidgetManagere instance so we can instantiate new FilterParameterWidgets
  FilterWidgetManager* fwm = FilterWidgetManager::Instance();
  // Get a list of all the filterParameters from the filter.
  FilterParameterVectorType filterParameters = filter->getFilterParameters();
  // Create all the FilterParameterWidget objects that can be displayed where ever the developer needs
  bool addSpacer = true;

  int pCount = 0, rCount = 0, cCount = 0;
  for(FilterParameterVectorType::iterator iter = filterParameters.begin(); iter != filterParameters.end(); ++iter)
  {
    FilterParameter* parameter = (*iter).get();

    QWidget* filterParameterWidget = fwm->createWidget(parameter, filter, this);
    m_PropertyToWidget.insert(parameter->getPropertyName(), filterParameterWidget); // Update our Map of Filter Parameter Properties to the Widget
    // Alert to DataArrayPath requirements
    connect(filterParameterWidget, SIGNAL(viewPathsMatchingReqs(DataContainerSelectionFilterParameter::RequirementType)), this,
            SLOT(getEmittedPathReqs(DataContainerSelectionFilterParameter::RequirementType)));
    connect(filterParameterWidget, SIGNAL(viewPathsMatchingReqs(AttributeMatrixSelectionFilterParameter::RequirementType)), this,
            SLOT(getEmittedPathReqs(AttributeMatrixSelectionFilterParameter::RequirementType)));
    connect(filterParameterWidget, SIGNAL(viewPathsMatchingReqs(DataArraySelectionFilterParameter::RequirementType)), this,
            SLOT(getEmittedPathReqs(DataArraySelectionFilterParameter::RequirementType)));
    connect(filterParameterWidget, SIGNAL(endViewPaths()), this, SIGNAL(endViewPaths()));
    // Alert to DataArrayPaths from the DataStructureWidget
    connect(this, SIGNAL(filterPath(DataArrayPath)), filterParameterWidget, SIGNAL(filterPathInput(DataArrayPath)));
    connect(this, SIGNAL(endDataStructureFiltering()), filterParameterWidget, SIGNAL(endDataStructureRequirements()));
    connect(this, SIGNAL(applyPathToFilteringParameter(DataArrayPath)), filterParameterWidget, SIGNAL(applyPathToFilteringParameter(DataArrayPath)));
    connect(this, SIGNAL(endPathFiltering()), filterParameterWidget, SIGNAL(endViewPathRequirements()));
    // Alert to DataArrayPaths from other FilterParameters
    connect(filterParameterWidget, SIGNAL(filterPath(DataArrayPath)), this, SLOT(emitFilterPath(DataArrayPath)));
    connect(filterParameterWidget, SIGNAL(endViewPaths()), this, SIGNAL(endPathFiltering()));

    // Return any lock/unlock signals from FilterParameterWidgets to all stored FilterParameterWidgets
    connect(filterParameterWidget, SIGNAL(dataArrayPathSelectionLocked(QToolButton*)), this, SIGNAL(dataArrayPathSelectionLocked(QToolButton*)));
    connect(this, SIGNAL(dataArrayPathSelectionLocked(QToolButton*)), filterParameterWidget, SIGNAL(lockDataArrayPathSelection(QToolButton*)));
    connect(filterParameterWidget, SIGNAL(dataArrayPathSelectionUnlocked(QToolButton*)), this, SIGNAL(dataArrayPathSelectionUnlocked(QToolButton*)));
    connect(this, SIGNAL(dataArrayPathSelectionUnlocked(QToolButton*)), filterParameterWidget, SIGNAL(unlockDataArrayPathSelection(QToolButton*)));

    if(nullptr == filterParameterWidget)
    {
      continue;
    }

    // Determine which group box to add the widget into
    if(parameter->getCategory() == FilterParameter::Category::Parameter)
    {
      filterParameterWidget->setParent(m_VariablesWidget);
      pLayout->addWidget(filterParameterWidget);
      pCount++;
    }
    else if(parameter->getCategory() == FilterParameter::Category::RequiredArray)
    {
      filterParameterWidget->setParent(m_VariablesWidget);
      rLayout->addWidget(filterParameterWidget);
      rCount++;
    }
    else if(parameter->getCategory() == FilterParameter::Category::CreatedArray)
    {
      filterParameterWidget->setParent(m_VariablesWidget);
      cLayout->addWidget(filterParameterWidget);
      cCount++;
    }
    else
    {
      filterParameterWidget->setParent(m_VariablesWidget);
      nLayout->addWidget(filterParameterWidget);
    }

    FilterParameterWidget* fpwPtr = qobject_cast<FilterParameterWidget*>(filterParameterWidget);
    if(nullptr != fpwPtr)
    {
      if(fpwPtr->isWidgetExpanding())
      {
        addSpacer = false;
      }
    }

    // Connect up some signals and slots
    connect(filterParameterWidget, SIGNAL(parametersChanged(bool)), this, SIGNAL(filterParametersChanged(bool)));

    connect(filterParameterWidget, SIGNAL(errorSettingFilterParameter(const QString&)), this, SIGNAL(errorSettingFilterParameter(const QString&)));
  }

  // Now link any boolean widgets to any conditional Widgets that they might control.
  linkConditionalWidgets(filterParameters);

  // If there are widgets in the parameters group box, add it to the overall layout.  If not, remove the group box.
  if(!pLayout->isEmpty() || pCount > 0)
  {
    m_VariablesVerticalLayout->addWidget(parametersGroupBox);
  }
  else
  {
    delete parametersGroupBox;
  }

  // If there are widgets in the required arrays group box, add it to the overall layout.  If not, remove the group box.
  if(!rLayout->isEmpty() || rCount > 0)
  {
    m_VariablesVerticalLayout->addWidget(requiredGroupBox);
  }
  else
  {
    delete requiredGroupBox;
  }

  // If there are widgets in the created arrays group box, add it to the overall layout.  If not, remove the group box.
  if(!cLayout->isEmpty() || cCount > 0)
  {
    m_VariablesVerticalLayout->addWidget(createdGroupBox);
  }
  else
  {
    delete createdGroupBox;
  }

  // If there are widgets in the uncategorized group box, add it to the overall layout.  If not, remove the group box.
  if(!nLayout->isEmpty())
  {
    m_VariablesVerticalLayout->addWidget(noCategoryGroupBox);
  }
  else
  {
    delete noCategoryGroupBox;
  }

  if(!addSpacer)
  {
    m_Ui->scrollAreaVertSpacer->removeItem(m_Ui->inputVertSpacer);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void FilterInputWidget::linkConditionalWidgets(FilterParameterVectorType& filterParameters)
{
  // Figure out if we have any "Linked Boolean Widgets" to hook up to other widgets
  for(FilterParameterVectorType::iterator iter = filterParameters.begin(); iter != filterParameters.end(); ++iter)
  {
    FilterParameter::Pointer filterParameter = (*iter);
    LinkedBooleanFilterParameter::Pointer filterParameterPtr = std::dynamic_pointer_cast<LinkedBooleanFilterParameter>(filterParameter);

    if(nullptr != filterParameterPtr)
    {
      std::vector<QString> linkedProps = filterParameterPtr->getConditionalProperties();

      QWidget* checkboxSource = m_PropertyToWidget[filterParameterPtr->getPropertyName()];

      for(const QString& propName : linkedProps)
      {
        QWidget* w = nullptr;
        if(m_PropertyToWidget.contains(propName))
        {
          w = m_PropertyToWidget[propName];
          connect(checkboxSource, SIGNAL(conditionalPropertyChanged(int)), w, SLOT(setLinkedConditionalState(int)));
          LinkedBooleanWidget* lbw = qobject_cast<LinkedBooleanWidget*>(checkboxSource);
          if((lbw != nullptr) && lbw->getLinkedState() != Qt::Checked)
          {
            w->hide();
          }
        }
      }

      LinkedBooleanWidget* boolWidget = qobject_cast<LinkedBooleanWidget*>(checkboxSource);
      if(nullptr != boolWidget)
      {
        boolWidget->updateLinkedWidgets();
      }
    }

    // Figure out if we have any Linked ComboBox Widgets to hook up to other widgets
    LinkedChoicesFilterParameter::Pointer optionPtr2 = std::dynamic_pointer_cast<LinkedChoicesFilterParameter>(filterParameter);

    if(nullptr != optionPtr2.get())
    {
      std::vector<QString> linkedProps = optionPtr2->getLinkedProperties();

      QWidget* checkboxSource = m_PropertyToWidget[optionPtr2->getPropertyName()];

      for(const QString& propName : linkedProps)
      {
        QWidget* w = nullptr;
        if(m_PropertyToWidget.contains(propName))
        {
          w = m_PropertyToWidget[propName];
        }

        if(nullptr != w)
        {
          // qDebug() << "Connecting: " << optionPtr2->getPropertyName() << " to " << propName;
          connect(checkboxSource, SIGNAL(conditionalPropertyChanged(int)), w, SLOT(setLinkedComboBoxState(int)));

          ChoiceWidget* choiceWidget = qobject_cast<ChoiceWidget*>(checkboxSource);
          if(choiceWidget != nullptr)
          {
            choiceWidget->widgetChanged(choiceWidget->getCurrentIndex(), false);
          }
        }
      }
    }

    LinkedDataContainerSelectionFilterParameter::Pointer optionPtr3 = std::dynamic_pointer_cast<LinkedDataContainerSelectionFilterParameter>(filterParameter);

    if(nullptr != optionPtr3)
    {
      std::vector<QString> linkedProps = optionPtr3->getLinkedProperties();

      QWidget* checkboxSource = m_PropertyToWidget[optionPtr3->getPropertyName()];

      for(const QString& propName : linkedProps)
      {
        QWidget* w = nullptr;
        if(m_PropertyToWidget.contains(propName))
        {
          w = m_PropertyToWidget[propName];
        }
        if(nullptr != w)
        {
          // qDebug() << "Connecting: " << optionPtr2->getPropertyName() << " to " << propName;
          connect(checkboxSource, SIGNAL(conditionalPropertyChanged(int)), w, SLOT(setLinkedComboBoxState(int)));

          LinkedDataContainerSelectionWidget* linkedDataContainerSelectionWidget = qobject_cast<LinkedDataContainerSelectionWidget*>(checkboxSource);
          if(linkedDataContainerSelectionWidget != nullptr)
          {
            linkedDataContainerSelectionWidget->widgetChanged();
          }
        }
      }
    }
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void FilterInputWidget::showHelp()
{
  DocRequestManager* docRequester = DocRequestManager::Instance();
  docRequester->requestFilterDocs(m_FilterClassName);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void FilterInputWidget::clearInputWidgets()
{
  // Remove any existing input widgets
  QLayoutItem* item = m_Ui->variablesGrid->itemAt(0);
  if(nullptr != item)
  {
    QWidget* w = item->widget();
    if(w != nullptr)
    {
      w->setVisible(false);
      m_Ui->variablesGrid->removeWidget(w);
    }
  }

  m_Ui->filterHumanLabel->setText("No Filter Selected");
  m_Ui->brandingLabel->clear();
  m_Ui->filterIndex->hide();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void FilterInputWidget::displayFilterParameters(AbstractFilter::Pointer filter)
{
  clearInputWidgets();

  if(m_VariablesWidget != nullptr)
  {
    m_Ui->variablesGrid->addWidget(m_VariablesWidget);
    m_VariablesWidget->setVisible(true);
  }

  ISIMPLibPlugin* plug = filter->getPluginInstance();
  if(nullptr != plug)
  {
    m_BrandingLabel = QString("Plugin: %1 (%2) Filter Name: %3").arg(plug->getPluginDisplayName()).arg(plug->getVersion()).arg(filter->getNameOfClass());
  }
  else
  {
    m_BrandingLabel = QString("Plugin: Unknown Plugin. Filter Name: %1").arg(filter->getNameOfClass());
  }
  m_Ui->brandingLabel->setText(m_BrandingLabel);

  // Add a label at the top of the Inputs Tabs to show what filter we are working on
  m_Ui->filterHumanLabel->setText(filter->getHumanLabel());
  m_Ui->filterIndex->setText(QString::number(filter->getPipelineIndex() + 1));
  m_Ui->filterIndex->show();
  // m_Ui->filterIndex->clear();
  QString style;

  QString filterGroup;
  QTextStream groupStream(&filterGroup);
  groupStream << "Group: " << filter->getGroupName() << "\n";
  groupStream << "Subgroup: " << filter->getSubGroupName();
  m_Ui->filterHumanLabel->setToolTip(filterGroup);

  //  QColor bgColor =  w->getGroupColor();
  //  QColor borderColor = QColor::fromHsv(bgColor.hue(), 100, 120);

  QTextStream styleStream(&style);
  styleStream << "QFrame#" << m_Ui->labelFrame->objectName() << "{";
  styleStream << "border-bottom: 0px solid;";
  //  styleStream << "border-bottom-color: " << borderColor.name() << ";";
  // styleStream << "background-color: " << bgColor.name() << ";";
  // styleStream << "border-radius: 0 0 0 0px;";
  styleStream << "}";

#if 0
  int index = -1;
  if(f.get()) {
    index = f->getPipelineIndex() + 1;
  }
  m_Ui->filterIndex->setText(QString::number(index));


  styleStream << "QLabel#" << m_Ui->filterIndex->objectName() << "{";
  styleStream << "background-color: rgb(48, 48, 48);";
  styleStream << "color: rgb(242, 242, 242);"; // Always have a white'ish font
 // styleStream << "border-radius: 3px;";
  styleStream << "padding: 1 5 1 5px;";
  styleStream << "}";
#endif

  m_Ui->labelFrame->setStyleSheet(style);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void FilterInputWidget::fadeInWidget(QWidget* widget)
{
  if(nullptr != m_FaderWidget)
  {
    m_FaderWidget->close();
  }
  m_FaderWidget = new QtSFaderWidget(widget);
  m_FaderWidget->setStartColor(SIMPLView::Defaults::AdvancedColor);
  m_FaderWidget->start();
  m_AdvFadedOut = false;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void FilterInputWidget::fadeOutWidget(QWidget* widget)
{

  if(nullptr != m_FaderWidget)
  {
    m_FaderWidget->close();
  }
  m_FaderWidget = new QtSFaderWidget(widget);
  m_FaderWidget->setFadeOut();
  m_FaderWidget->setStartColor(SIMPLView::Defaults::AdvancedColor);
  connect(m_FaderWidget, SIGNAL(animationComplete()), widget, SLOT(hide()));
  m_FaderWidget->start();
  m_AdvFadedOut = true;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void FilterInputWidget::getEmittedPathReqs(DataContainerSelectionFilterParameter::RequirementType dcReqs)
{
  Q_EMIT viewPathsMatchingReqs(dcReqs);

  // QObject* obj = this->sender();
  // for(QWidget* widget : m_PropertyToWidget)
  //{
  //  FilterParameterWidget* fpWidget = dynamic_cast<FilterParameterWidget*>(widget);
  //  if(fpWidget && fpWidget != obj)
  //  {
  //    Q_EMIT fpWidget->endViewPathRequirements();
  //  }
  //}
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void FilterInputWidget::getEmittedPathReqs(AttributeMatrixSelectionFilterParameter::RequirementType amReqs)
{
  Q_EMIT viewPathsMatchingReqs(amReqs);

  // QObject* obj = this->sender();
  // for(QWidget* widget : m_PropertyToWidget)
  //{
  //  FilterParameterWidget* fpWidget = dynamic_cast<FilterParameterWidget*>(widget);
  //  if(fpWidget && fpWidget != obj)
  //  {
  //    Q_EMIT fpWidget->endViewPathRequirements();
  //  }
  //}
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void FilterInputWidget::getEmittedPathReqs(DataArraySelectionFilterParameter::RequirementType daReqs)
{
  Q_EMIT viewPathsMatchingReqs(daReqs);

  // QObject* obj = this->sender();
  // for(QWidget* widget : m_PropertyToWidget)
  //{
  //  FilterParameterWidget* fpWidget = dynamic_cast<FilterParameterWidget*>(widget);
  //  if(fpWidget && fpWidget != obj)
  //  {
  //    Q_EMIT fpWidget->endViewPathRequirements();
  //  }
  //}
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void FilterInputWidget::emitFilterPath(DataArrayPath path)
{
  QObject* obj = this->sender();
  for(QWidget* widget : m_PropertyToWidget)
  {
    FilterParameterWidget* fpWidget = dynamic_cast<FilterParameterWidget*>(widget);
    if((fpWidget != nullptr) && fpWidget != obj)
    {
      Q_EMIT fpWidget->filterPathInput(path);
    }
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QWidget* FilterInputWidget::getVariablesTabContentsWidget()
{
  return m_Ui->variablesTabContents;
}
