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

#include "InputPathWidget.h"

#include <QtWidgets/QFileDialog>

#include "SIMPLib/Utilities/FilterCompatibility.hpp"

#include "SVWidgetsLib/QtSupport/QtSFileCompleter.h"

#include "SVWidgetsLib/Core/SVWidgetsLibConstants.h"

#include "FilterParameterWidgetsDialogs.h"

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
InputPathWidget::InputPathWidget(FilterParameter* parameter, AbstractFilter* filter, QWidget* parent)
: AbstractIOFileWidget(parameter, filter, parent)
{
  m_FilterParameter = SIMPL_FILTER_PARAMETER_COMPATIBILITY_CHECK(filter, parameter, InputPathWidget, InputPathFilterParameter);

  setupGui();

  if(filter != nullptr)
  {
    QString currentPath = m_FilterParameter->getGetterCallback()();
    if(!currentPath.isEmpty())
    {
      currentPath = QDir::toNativeSeparators(currentPath);
      setValue(currentPath);
    }
    else
    {
      setValue(QDir::homePath());
    }
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
InputPathWidget::~InputPathWidget() = default;

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void InputPathWidget::setupGui()
{
  connect(selectBtn, SIGNAL(clicked()), this, SLOT(selectInputPath()));

  m_LineEdit->setPlaceholderText("Enter Input Folder Path");
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void InputPathWidget::selectInputPath()
{
  QString currentPath = m_FilterParameter->getGetterCallback()();
  if(currentPath.isEmpty())
  {
    currentPath = getValue();
  }
  QString file = QFileDialog::getExistingDirectory(this, tr("Select Input Folder"), currentPath, QFileDialog::ShowDirsOnly);

  if(file.isEmpty())
  {
    return;
  }
  file = QDir::toNativeSeparators(file);
  // Store the last used directory into the private instance variable
  QFileInfo fi(file);
  setValue(fi.filePath());

  m_LineEdit->setText(file);
  on_m_LineEdit_editingFinished();
}
