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

#include "DoubleWidget.h"

#include "SIMPLib/Utilities/FilterCompatibility.hpp"

#include "SVWidgetsLib/Widgets/SVStyle.h"

#include "SVWidgetsLib/Core/SVWidgetsLibConstants.h"

#include "FilterParameterWidgetsDialogs.h"

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
DoubleWidget::DoubleWidget(FilterParameter* parameter, AbstractFilter* filter, QWidget* parent)
: FilterParameterWidget(parameter, filter, parent)
{
  m_FilterParameter = SIMPL_FILTER_PARAMETER_COMPATIBILITY_CHECK(filter, parameter, DoubleWidget, DoubleFilterParameter);

  setupUi(this);
  setupGui();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
DoubleWidget::~DoubleWidget() = default;

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void DoubleWidget::setupGui()
{
  // Catch when the filter is about to execute the preflight
  connect(getFilter(), &AbstractFilter::preflightAboutToExecute, this, &DoubleWidget::beforePreflight);

  // Catch when the filter is finished running the preflight
  connect(getFilter(), &AbstractFilter::preflightExecuted, this, &DoubleWidget::afterPreflight);

  // Catch when the filter wants its values updated
  connect(getFilter(), &AbstractFilter::updateFilterParameters, this, &DoubleWidget::filterNeedsInputParameters);

  connect(value, SIGNAL(textChanged(const QString&)), this, SLOT(widgetChanged(const QString&)));

  QLocale loc = QLocale::system();
  loc.setNumberOptions(QLocale::NumberOption::RejectGroupSeparator | QLocale::NumberOption::OmitGroupSeparator);

  QDoubleValidator* xVal = new QDoubleValidator(value);
  xVal->setLocale(loc);
  value->setValidator(xVal);

  if(getFilterParameter() != nullptr)
  {
    label->setText(getFilterParameter()->getHumanLabel());
    QString str = loc.toString(m_FilterParameter->getGetterCallback()());
    value->setText(str);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void DoubleWidget::widgetChanged(const QString& text)
{
  Q_EMIT parametersChanged();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void DoubleWidget::filterNeedsInputParameters(AbstractFilter* filter)
{
  bool ok = true;
  double defValue = getFilterParameter()->getDefaultValue().toDouble();
  double i = defValue;
  // Next make sure there is something in the
  if(!value->text().isEmpty())
  {
    i = value->locale().toDouble(value->text(), &ok);
    // i = value->text().toDouble(&ok);
    //  make sure we can convert the entered value to a 32 bit signed int
    if(!ok)
    {
      SVStyle::Instance()->LineEditBackgroundErrorStyle(value);
      SVStyle::Instance()->SetErrorColor("QLabel", errorLabel);
      errorLabel->setText("Value entered is beyond the representable range for a double.\nThe filter will use the default value of " + getFilterParameter()->getDefaultValue().toString());
      errorLabel->show();
      i = defValue;
    }
    else
    {
      errorLabel->hide();
      SVStyle::Instance()->LineEditClearStyle(value);
    }
  }
  else
  {
    SVStyle::Instance()->LineEditErrorStyle(value);
    SVStyle::Instance()->SetErrorColor("QLabel", errorLabel);
    errorLabel->setText("No value entered. Filter will use default value of " + getFilterParameter()->getDefaultValue().toString());
    errorLabel->show();
  }
  Q_UNUSED(filter)
  DoubleFilterParameter::SetterCallbackType setter = m_FilterParameter->getSetterCallback();
  if(setter)
  {
    setter(i);
  }
  else
  {
    getFilter()->notifyMissingProperty(getFilterParameter());
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void DoubleWidget::beforePreflight()
{
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void DoubleWidget::afterPreflight()
{
}
