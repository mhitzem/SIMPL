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

#include "AbstractIOFileWidget.h"

#include <QtCore/QDir>
#include <QtCore/QFileInfo>

#include <QtGui/QKeyEvent>
#include <QtGui/QPainter>

#include <QtWidgets/QMenu>

#include "SIMPLib/Utilities/FilterCompatibility.hpp"
#include "SIMPLib/Utilities/SIMPLDataPathValidator.h"

#include "SVWidgetsLib/Core/SVWidgetsLibConstants.h"
#include "SVWidgetsLib/QtSupport/QtSFileCompleter.h"
#include "SVWidgetsLib/QtSupport/QtSFileUtils.h"
#include "SVWidgetsLib/Widgets/SVStyle.h"

#include "FilterParameterWidgetsDialogs.h"

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
AbstractIOFileWidget::AbstractIOFileWidget(FilterParameter* parameter, AbstractFilter* filter, QWidget* parent)
: FilterParameterWidget(parameter, filter, parent)
{
  m_FilterParameter = SIMPL_FILTER_PARAMETER_COMPATIBILITY_CHECK(filter, parameter, AbstractIOFileWidget, AbstractIOFilterParameter);

  setupUi(this);
  setupGui();

  QString currentPath = m_FilterParameter->getGetterCallback()();
  if(!currentPath.isEmpty())
  {
    currentPath = QDir::toNativeSeparators(currentPath);
    // Store the last used directory into the private instance variable
    QFileInfo fi(currentPath);
    m_LineEdit->setText(fi.filePath());
    setValidFilePath(m_LineEdit->text());
  }
  else
  {
    m_LineEdit->setText(QDir::homePath());
    setValidFilePath(m_LineEdit->text());
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
AbstractIOFileWidget::~AbstractIOFileWidget() = default;

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void AbstractIOFileWidget::setIcon(const QPixmap& path)
{
  m_Icon = path;
  setupMenuField();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QPixmap AbstractIOFileWidget::getIcon()
{
  return m_Icon;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void AbstractIOFileWidget::setupGui()
{
  // Catch when the filter is about to execute the preflight
  connect(getFilter(), &AbstractFilter::preflightAboutToExecute, this, &AbstractIOFileWidget::beforePreflight);

  // Catch when the filter is finished running the preflight
  connect(getFilter(), &AbstractFilter::preflightExecuted, this, &AbstractIOFileWidget::afterPreflight);

  // Catch when the filter wants its m_LineEdits updated
  connect(getFilter(), &AbstractFilter::updateFilterParameters, this, &AbstractIOFileWidget::filterNeedsInputParameters);

  QtSFileCompleter* com = new QtSFileCompleter(this, false);
  m_LineEdit->setCompleter(com);
  QObject::connect(com, SIGNAL(activated(const QString&)), this, SLOT(on_m_LineEdit_textChanged(const QString&)));

  setupMenuField();

  absPathLabel->hide();
  // absPathNameLabel->hide();

  // Update the widget when the data directory changes
  SIMPLDataPathValidator* validator = SIMPLDataPathValidator::Instance();
  connect(validator, &SIMPLDataPathValidator::dataDirectoryChanged, [=] {
    blockSignals(true);
    on_m_LineEdit_textChanged(m_LineEdit->text());
    on_m_LineEdit_fileDropped(m_LineEdit->text());
    on_m_LineEdit_returnPressed();
    blockSignals(false);

    Q_EMIT parametersChanged();
  });

  if(getFilterParameter() != nullptr)
  {
    label->setText(getFilterParameter()->getHumanLabel());

    QString currentPath = m_FilterParameter->getGetterCallback()();
    m_LineEdit->setText(currentPath);
    setValidFilePath(m_LineEdit->text());
    on_m_LineEdit_fileDropped(currentPath);
    on_m_LineEdit_returnPressed();
  }

  m_CurrentText = m_LineEdit->text();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void AbstractIOFileWidget::keyPressEvent(QKeyEvent* event)
{
  if(event->key() == Qt::Key_Escape)
  {
    SVStyle* style = SVStyle::Instance();
    m_LineEdit->setText(m_CurrentText);
    setValidFilePath(m_LineEdit->text());
    style->LineEditClearStyle(m_LineEdit);
    m_LineEdit->setToolTip("");
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void AbstractIOFileWidget::setupMenuField()
{
  QFileInfo fi(m_LineEdit->text());

  QMenu* lineEditMenu = new QMenu(m_LineEdit);
  m_LineEdit->setButtonMenu(QtSLineEdit::Left, lineEditMenu);

  m_LineEdit->setButtonVisible(QtSLineEdit::Left, true);

  QPixmap pixmap(8, 8);
  pixmap.fill(Qt::transparent);
  QPainter painter(&pixmap);
  painter.drawPixmap(0, (pixmap.height() - m_Icon.height()) / 2, m_Icon);
  m_LineEdit->setButtonPixmap(QtSLineEdit::Left, pixmap);

  {
    m_ShowFileAction = new QAction(lineEditMenu);
    m_ShowFileAction->setObjectName(QString::fromUtf8("showFileAction"));
#if defined(Q_OS_WIN)
    m_ShowFileAction->setText("Show in Windows Explorer");
#elif defined(Q_OS_MAC)
    m_ShowFileAction->setText("Show in Finder");
#else
    m_ShowFileAction->setText("Show in File System");
#endif
    lineEditMenu->addAction(m_ShowFileAction);
    connect(m_ShowFileAction, SIGNAL(triggered()), this, SLOT(showFileInFileSystem()));
  }

  if(!m_LineEdit->text().isEmpty() && fi.exists())
  {
    m_ShowFileAction->setEnabled(true);
  }
  else
  {
    m_ShowFileAction->setDisabled(true);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void AbstractIOFileWidget::on_m_LineEdit_editingFinished()
{
  SIMPLDataPathValidator* validator = SIMPLDataPathValidator::Instance();
  QString path = validator->convertToAbsolutePath(m_LineEdit->text());

  QFileInfo fi(m_LineEdit->text());
  if(fi.isRelative())
  {
    absPathLabel->setText(path);
    absPathLabel->show();
  }
  else
  {
    absPathLabel->hide();
  }

  QtSFileUtils::VerifyPathExists(path, m_LineEdit);
  if(m_LineEdit->text() != m_CurrentText)
  {
    Q_EMIT parametersChanged(); // This should force the preflight to run because we are emitting a signal
  }
  m_CurrentText = m_LineEdit->text();
  setValidFilePath(m_LineEdit->text());
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void AbstractIOFileWidget::on_m_LineEdit_returnPressed()
{
  on_m_LineEdit_editingFinished();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void AbstractIOFileWidget::on_m_LineEdit_textChanged(const QString& text)
{
  SIMPLDataPathValidator* validator = SIMPLDataPathValidator::Instance();
  QString inputPath = validator->convertToAbsolutePath(text);

  QFileInfo fi(text);
  if(fi.isRelative())
  {
    absPathLabel->setText(inputPath);
  }

  if(QtSFileUtils::HasValidFilePath(inputPath))
  {
    m_ShowFileAction->setEnabled(true);
    setValidFilePath(m_LineEdit->text());
  }
  else
  {
    m_ShowFileAction->setDisabled(true);
  }

  SVStyle* style = SVStyle::Instance();

  if(text != m_CurrentText)
  {
    style->LineEditBackgroundErrorStyle(m_LineEdit);
    m_LineEdit->setToolTip("Press the 'Return' key to apply your changes");
  }
  else
  {
    style->LineEditClearStyle(m_LineEdit);
    m_LineEdit->setToolTip("");
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void AbstractIOFileWidget::on_m_LineEdit_fileDropped(const QString& text)
{
  SIMPLDataPathValidator* validator = SIMPLDataPathValidator::Instance();
  QString inputPath = validator->convertToAbsolutePath(text);

  m_LineEdit->setText(text);
  setValidFilePath(m_LineEdit->text());
  // Set/Remove the red outline if the file does exist
  QtSFileUtils::VerifyPathExists(inputPath, m_LineEdit);

  Q_EMIT parametersChanged(); // This should force the preflight to run because we are emitting a signal
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void AbstractIOFileWidget::filterNeedsInputParameters(AbstractFilter* filter)
{
  QString text = m_LineEdit->text();

  SIMPLDataPathValidator* validator = SIMPLDataPathValidator::Instance();
  text = validator->convertToAbsolutePath(text);

  auto setter = m_FilterParameter->getSetterCallback();
  if(setter)
  {
    setter(text);
  }
  else
  {
    getFilter()->notifyMissingProperty(getFilterParameter());
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void AbstractIOFileWidget::beforePreflight()
{
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void AbstractIOFileWidget::afterPreflight()
{
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void AbstractIOFileWidget::setValue(const QString& val)
{
  m_LineEdit->setText(val);
  setValidFilePath(m_LineEdit->text());
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString AbstractIOFileWidget::getValue()
{
  if(m_LineEdit->text().isEmpty())
  {
    return QDir::homePath();
  }
  return m_LineEdit->text();
}
