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

#include "LinkedBooleanWidget.h"

#include "SIMPLib/Utilities/FilterCompatibility.hpp"

#include "SVWidgetsLib/Core/SVWidgetsLibConstants.h"

#include "FilterParameterWidgetsDialogs.h"

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
LinkedBooleanWidget::LinkedBooleanWidget(FilterParameter* parameter, AbstractFilter* filter, QWidget* parent)
: FilterParameterWidget(parameter, filter, parent)
{
  m_FilterParameter = SIMPL_FILTER_PARAMETER_COMPATIBILITY_CHECK(filter, parameter, LinkedBooleanWidget, LinkedBooleanFilterParameter);

  setupUi(this);
  setupGui();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
LinkedBooleanWidget::~LinkedBooleanWidget() = default;

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void LinkedBooleanWidget::setupGui()
{

  // Catch when the filter is about to execute the preflight
  connect(getFilter(), &AbstractFilter::preflightAboutToExecute, this, &LinkedBooleanWidget::beforePreflight);

  // Catch when the filter is finished running the preflight
  connect(getFilter(), &AbstractFilter::preflightExecuted, this, &LinkedBooleanWidget::afterPreflight);

  // Catch when the filter wants its values updated
  connect(getFilter(), &AbstractFilter::updateFilterParameters, this, &LinkedBooleanWidget::filterNeedsInputParameters);

  connect(value, SIGNAL(stateChanged(int)), this, SLOT(widgetChanged(int)));

  if(getFilterParameter() != nullptr)
  {
    label->setText(getFilterParameter()->getHumanLabel());
    value->setChecked(m_FilterParameter->getGetterCallback()());
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void LinkedBooleanWidget::updateLinkedWidgets()
{
  int state = value->checkState();
  Q_EMIT conditionalPropertyChanged(state);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int LinkedBooleanWidget::getLinkedState()
{
  return value->checkState();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void LinkedBooleanWidget::widgetChanged(int state)
{
  // Let the other widgets that this widget is linked to know that the state changed.
  Q_EMIT conditionalPropertyChanged(state);
  Q_EMIT parametersChanged();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void LinkedBooleanWidget::filterNeedsInputParameters(AbstractFilter* filter)
{
  Q_UNUSED(filter)
  LinkedBooleanFilterParameter::SetterCallbackType setter = m_FilterParameter->getSetterCallback();
  if(setter)
  {
    setter(value->isChecked());
  }
  else
  {
    getFilter()->notifyMissingProperty(getFilterParameter());
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void LinkedBooleanWidget::beforePreflight()
{
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void LinkedBooleanWidget::afterPreflight()
{
}
