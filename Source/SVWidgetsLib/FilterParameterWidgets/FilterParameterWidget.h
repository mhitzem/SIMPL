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

#pragma once

#include <QtCore/QPointer>

#include <QtWidgets/QFrame>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QWidget>

#include "SIMPLib/SIMPLib.h"
#include "SIMPLib/FilterParameters/AttributeMatrixSelectionFilterParameter.h"
#include "SIMPLib/FilterParameters/DataArraySelectionFilterParameter.h"
#include "SIMPLib/FilterParameters/DataContainerSelectionFilterParameter.h"

#include "SVWidgetsLib/FilterParameterWidgets/FilterParameterWidget.h"
#include "SVWidgetsLib/QtSupport/QtSFaderWidget.h"
#include "SVWidgetsLib/SVWidgetsLib.h"
#include "SVWidgetsLib/Widgets/SVStyle.h"

namespace Detail
{
const QString Delimiter(" / ");
}

class AbstractFilter;
class FilterParameter;
class QPropertyAnimation;
class QGraphicsOpacityEffect;
class QTimer;
class QLineEdit;

/**
 * @brief The FilterParameterWidget class
 */
class SVWidgetsLib_EXPORT FilterParameterWidget : public QFrame
{
  Q_OBJECT
public:
  FilterParameterWidget(QWidget* parent = nullptr);
  FilterParameterWidget(FilterParameter* parameter, AbstractFilter* filter = nullptr, QWidget* parent = nullptr);

  ~FilterParameterWidget() override;

  using EnumType = uint32_t;

  enum class Style : EnumType
  {
    FS_STANDARD_STYLE = 0,
    FS_DRAGGING_STYLE = 1,
    FS_DOESNOTEXIST_STYLE = 2,
    FS_WARNING_STYLE = 3
  };

  /**
   * @brief Setter property for Filter
   */
  virtual void setFilter(AbstractFilter* value);
  /**
   * @brief Getter property for Filter
   * @return Value of Filter
   */
  virtual AbstractFilter* getFilter() const;

  /**
   * @brief Setter property for FilterParameter
   */
  virtual void setFilterParameter(FilterParameter* value);
  /**
   * @brief Getter property for FilterParameter
   * @return Value of FilterParameter
   */
  virtual FilterParameter* getFilterParameter() const;

  /**
   * @brief Setter property for WidgetIsExpanding
   */
  virtual void setWidgetIsExpanding(bool value);
  virtual bool isWidgetExpanding() const;

  void setValidFilePath(const QString& filePath);

  void fadeInWidget(QWidget* widget);

  QPointer<QtSFaderWidget> getFaderWidget() const;
  void setFaderWidget(QPointer<QtSFaderWidget> faderWidget);

  virtual void setupGui();

  QString wrapStringInHtml(const QString& message);

  virtual void changeStyleSheet(Style style);

Q_SIGNALS:
  void filterPath(DataArrayPath path);
  void viewPathsMatchingReqs(DataContainerSelectionFilterParameter::RequirementType dcReqs);
  void viewPathsMatchingReqs(AttributeMatrixSelectionFilterParameter::RequirementType amReqs);
  void viewPathsMatchingReqs(DataArraySelectionFilterParameter::RequirementType daReqs);
  void endViewPaths();
  void filterPathInput(DataArrayPath);
  void endViewPathRequirements();
  void endDataStructureRequirements();
  void applyPathToFilteringParameter(DataArrayPath path);
  void dataArrayPathSelectionLocked(QToolButton* selection);
  void dataArrayPathSelectionUnlocked(QToolButton* button);
  void lockDataArrayPathSelection(QToolButton* selection);
  void unlockDataArrayPathSelection(QToolButton* button);
  void errorSettingFilterParameter(const QString& msg);
  void parametersChanged(bool preflight = true);

public Q_SLOTS:

  void setLinkedConditionalState(int state);
  void setLinkedComboBoxState(int groupId);

  void fadeWidget(QWidget* widget, bool in);
  void animationFinished();

  /**
   * @brief showFileInFileSystem
   */
  virtual void showFileInFileSystem();

  /**
   * @brief loadData
   */
  virtual void loadData();

protected:
  /**
   * @brief adjustedMenuPosition
   * @param pushButton
   * @return
   */
  QPoint adjustedMenuPosition(QToolButton* pushButton);

protected Q_SLOTS:
  void showBorder();
  void hideBorder();

private:
  bool m_WidgetIsExpanding = {};

  AbstractFilter* m_Filter = nullptr;
  FilterParameter* m_FilterParameter = nullptr;

  QPointer<QtSFaderWidget> m_FaderWidget;

  float startValue;
  float endValue;
  bool fadeIn;

  QTimer* m_Timer = nullptr;
  QPropertyAnimation* animation;
  QGraphicsOpacityEffect* effect;
  QString m_CurrentlyValidPath;

public:
  FilterParameterWidget(const FilterParameterWidget&) = delete;            // Copy Constructor Not Implemented
  FilterParameterWidget(FilterParameterWidget&&) = delete;                 // Move Constructor Not Implemented
  FilterParameterWidget& operator=(const FilterParameterWidget&) = delete; // Copy Assignment Not Implemented
  FilterParameterWidget& operator=(FilterParameterWidget&&) = delete;      // Move Assignment Not Implemented
};

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
/* These macros are mainly used by the 2nd, 3rd and 4rth Order Polynomial Widgets
 * We put them here so all three can use these macros and we do not duplicate
 * the code into 3 different locations
 */

#define FOPW_SETUP_WIDGET(cell)                                                                                                                                                                        \
  connect(cell, SIGNAL(textChanged(const QString&)), this, SLOT(widgetChanged(const QString&)));                                                                                                       \
  QDoubleValidator* cell##Val = new QDoubleValidator(cell);                                                                                                                                            \
  cell##Val->setLocale(loc);                                                                                                                                                                           \
  cell->setValidator(cell##Val);

#define FOPW_CHECK_LINEEDIT(cell)                                                                                                                                                                      \
  {                                                                                                                                                                                                    \
    if(sender() == cell && cell->text().isEmpty())                                                                                                                                                     \
    {                                                                                                                                                                                                  \
      SVStyle::Instance()->LineEditErrorStyle(cell);                                                                                                                                                   \
      SVStyle::Instance()->SetErrorColor("QLabel", errorLabel);                                                                                                                                        \
      errorLabel->setText("No value entered. Filter will use default value of " + getFilterParameter()->getDefaultValue().toString());                                                                 \
      errorLabel->show();                                                                                                                                                                              \
    }                                                                                                                                                                                                  \
  }

#define FOPW_EXTRACT_VALUE(cell)                                                                                                                                                                       \
  data.cell = loc.toFloat(cell->text(), &ok);                                                                                                                                                          \
  if(!ok)                                                                                                                                                                                              \
  {                                                                                                                                                                                                    \
    if(errorLabel)                                                                                                                                                                                     \
    {                                                                                                                                                                                                  \
      SVStyle::Instance()->SetErrorColor("QLabel", errorLabel);                                                                                                                                        \
      errorLabel->setText("Value entered is beyond the representable range for the value type.\nThe filter will use the default value of " + getFilterParameter()->getDefaultValue().toString());      \
      errorLabel->show();                                                                                                                                                                              \
    }                                                                                                                                                                                                  \
    SVStyle::Instance()->LineEditErrorStyle(cell);                                                                                                                                                     \
    data.cell = defValue.cell;                                                                                                                                                                         \
  }
