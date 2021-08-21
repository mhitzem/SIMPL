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

#include "ThirdOrderPolynomialWidget.h"

#include "SIMPLib/FilterParameters/ThirdOrderPolynomialFilterParameter.h"
#include "SIMPLib/Utilities/FilterCompatibility.hpp"
#include "SVWidgetsLib/Widgets/SVStyle.h"

#include "SVWidgetsLib/Core/SVWidgetsLibConstants.h"

#include "FilterParameterWidgetsDialogs.h"

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
ThirdOrderPolynomialWidget::ThirdOrderPolynomialWidget(FilterParameter* parameter, AbstractFilter* filter, QWidget* parent)
: FilterParameterWidget(parameter, filter, parent)
{
  m_FilterParameter = SIMPL_FILTER_PARAMETER_COMPATIBILITY_CHECK(filter, parameter, ThirdOrderPolynomialWidget, ThirdOrderPolynomialFilterParameter);

  setupUi(this);
  setupGui();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
ThirdOrderPolynomialWidget::~ThirdOrderPolynomialWidget() = default;

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ThirdOrderPolynomialWidget::setupGui()
{
  // Catch when the filter is about to execute the preflight
  connect(getFilter(), &AbstractFilter::preflightAboutToExecute, this, &ThirdOrderPolynomialWidget::beforePreflight);

  // Catch when the filter is finished running the preflight
  connect(getFilter(), &AbstractFilter::preflightExecuted, this, &ThirdOrderPolynomialWidget::afterPreflight);

  // Catch when the filter wants its values updated
  connect(getFilter(), &AbstractFilter::updateFilterParameters, this, &ThirdOrderPolynomialWidget::filterNeedsInputParameters);

  QLocale loc = QLocale::system();

  FOPW_SETUP_WIDGET(c30)
  FOPW_SETUP_WIDGET(c03)
  FOPW_SETUP_WIDGET(c21)
  FOPW_SETUP_WIDGET(c12)
  FOPW_SETUP_WIDGET(c20)
  FOPW_SETUP_WIDGET(c02)
  FOPW_SETUP_WIDGET(c11)
  FOPW_SETUP_WIDGET(c10)
  FOPW_SETUP_WIDGET(c01)
  FOPW_SETUP_WIDGET(c00)

  if(getFilterParameter() != nullptr)
  {
    label->setText(getFilterParameter()->getHumanLabel());

    Float3rdOrderPoly_t data = SafeFilterParameterGetter(m_FilterParameter, getFilter());
    c30->setText(loc.toString(data.c30));
    c03->setText(loc.toString(data.c03));
    c21->setText(loc.toString(data.c21));
    c12->setText(loc.toString(data.c12));
    c20->setText(loc.toString(data.c20));
    c02->setText(loc.toString(data.c02));
    c11->setText(loc.toString(data.c11));
    c10->setText(loc.toString(data.c10));
    c01->setText(loc.toString(data.c01));
    c00->setText(loc.toString(data.c00));
  }
  errorLabel->hide();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ThirdOrderPolynomialWidget::widgetChanged(const QString& text)
{
  Q_UNUSED(text)
  errorLabel->hide();

  FOPW_CHECK_LINEEDIT(c30)
  FOPW_CHECK_LINEEDIT(c03)
  FOPW_CHECK_LINEEDIT(c21)
  FOPW_CHECK_LINEEDIT(c12)
  FOPW_CHECK_LINEEDIT(c20)
  FOPW_CHECK_LINEEDIT(c02)
  FOPW_CHECK_LINEEDIT(c11)
  FOPW_CHECK_LINEEDIT(c10)
  FOPW_CHECK_LINEEDIT(c01)
  FOPW_CHECK_LINEEDIT(c00)

  Q_EMIT parametersChanged();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ThirdOrderPolynomialWidget::filterNeedsInputParameters(AbstractFilter* filter)
{
  bool ok = false;
  Float3rdOrderPoly_t data;
  Float3rdOrderPoly_t defValue = m_FilterParameter->getDefaultValue().value<Float3rdOrderPoly_t>();

  QLocale loc;

  FOPW_EXTRACT_VALUE(c30)
  FOPW_EXTRACT_VALUE(c03)
  FOPW_EXTRACT_VALUE(c21)
  FOPW_EXTRACT_VALUE(c12)
  FOPW_EXTRACT_VALUE(c20)
  FOPW_EXTRACT_VALUE(c02)
  FOPW_EXTRACT_VALUE(c11)
  FOPW_EXTRACT_VALUE(c10)
  FOPW_EXTRACT_VALUE(c01)
  FOPW_EXTRACT_VALUE(c00)

  Q_UNUSED(filter)
  SafeFilterParameterSetter(m_FilterParameter, data, getFilter());
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ThirdOrderPolynomialWidget::beforePreflight()
{
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ThirdOrderPolynomialWidget::afterPreflight()
{
}
