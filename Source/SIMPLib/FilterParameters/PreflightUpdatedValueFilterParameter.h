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

#include <memory>

#include "SIMPLib/FilterParameters/FilterParameter.h"

/**
 * @brief SIMPL_NEW_PREFLIGHTUPDATEDVALUE_FP This macro is a short-form way of instantiating an instance of
 * PreflightUpdatedValueFilterParameter. There are 4 required parameters and 1 optional parameter
 * that are always passed to this macro in the following order: HumanLabel, PropertyName, Category,
 * FilterName (class name), GroupIndex (optional).
 *
 * Therefore, the macro should be written like this (this is a concrete example):
 * SIMPL_NEW_PREFLIGHTUPDATEDVALUE_FP("HumanLabel", PropertyName, Category, FilterName, GroupIndex)
 *
 * Example 1 (instantiated within a filter called [GenericExample](@ref genericexample), with optional GroupIndex parameter):
 * SIMPL_NEW_PREFLIGHTUPDATEDVALUE_FP("Estimated Primary Features", EstimatedPrimaryFeatures, FilterParameter::Category::Parameter, GenericExample);
 */
#define SIMPL_NEW_PREFLIGHTUPDATEDVALUE_FP(...)                                                                                                                                                        \
  SIMPL_EXPAND(_FP_GET_PREFLIGHTUPDATEDVALUE_OVERRIDE(__VA_ARGS__, SIMPL_NEW_PREFLIGHTUPDATEDVALUE_FP_5, SIMPL_NEW_PREFLIGHTUPDATEDVALUE_FP_4)(PreflightUpdatedValueFilterParameter, __VA_ARGS__))

/**
 * @brief The PreflightUpdatedValueFilterParameter class is used by filters to instantiate an PreflightUpdatedValueWidget.  By instantiating an instance of
 * this class in a filter's setupFilterParameters() method, a PreflightUpdatedValueWidget will appear in the filter's "filter input" section in the DREAM3D GUI.
 */
class SIMPLib_EXPORT PreflightUpdatedValueFilterParameter : public FilterParameter
{
  // Start Python bindings declarations
  // clang-format off
  PYB11_BEGIN_BINDINGS(PreflightUpdatedValueFilterParameter SUPERCLASS FilterParameter)
  PYB11_SHARED_POINTERS(PreflightUpdatedValueFilterParameter)
  PYB11_STATIC_CREATION(Create)
  PYB11_END_BINDINGS()
  // clang-format on
  // End Python bindings declarations
public:
  using Self = PreflightUpdatedValueFilterParameter;
  using Pointer = std::shared_ptr<Self>;
  using ConstPointer = std::shared_ptr<const Self>;
  using WeakPointer = std::weak_ptr<Self>;
  using ConstWeakPointer = std::weak_ptr<const Self>;
  static Pointer NullPointer();

  static Pointer New();

  /**
   * @brief Returns the name of the class for PreflightUpdatedValueFilterParameter
   */
  QString getNameOfClass() const override;
  /**
   * @brief Returns the name of the class for PreflightUpdatedValueFilterParameter
   */
  static QString ClassName();

  using SetterCallbackType = std::function<void(QString)>;
  using GetterCallbackType = std::function<QString(void)>;

  /**
   * @brief New This function instantiates an instance of the PreflightUpdatedValueFilterParameter. Although this function is available to be used,
   * the preferable way to instantiate an instance of this class is to use the SIMPL_NEW_PREFLIGHTUPDATEDVALUE_FP(...) macro at the top of this file.

   * @param humanLabel The name that the users of DREAM.3D see for this filter parameter
   * @param propertyName The internal property name for this filter parameter.
   * @param defaultValue The value that this filter parameter will be initialized to by default.
   * @param category The category for the filter parameter in the DREAM.3D user interface.  There
   * are three categories: Parameter, Required Arrays, and Created Arrays.
   * @param getterCallback The method in the AbstractFilter subclass that <i>gets</i> the value of the property
  * that this FilterParameter subclass represents.
   * @param groupIndex Integer that specifies the group that this filter parameter will be placed in.
   * @return
   */
  static Pointer Create(const QString& humanLabel, const QString& propertyName, const QString& defaultValue, Category category, GetterCallbackType getterCallback, int groupIndex = -1);

  ~PreflightUpdatedValueFilterParameter() override;

  /**
   * @brief getWidgetType Returns the type of widget that displays and controls
   * this FilterParameter subclass
   * @return
   */
  QString getWidgetType() const override;

  /**
   * @param GetterCallback The method in the AbstractFilter subclass that <i>gets</i> the value of the property
   * that this FilterParameter subclass represents.
   * @return The GetterCallback
   */
  /**
   * @brief Setter property for GetterCallback
   */
  void setGetterCallback(const PreflightUpdatedValueFilterParameter::GetterCallbackType& value);
  /**
   * @brief Getter property for GetterCallback
   * @return Value of GetterCallback
   */
  PreflightUpdatedValueFilterParameter::GetterCallbackType getGetterCallback() const;

  /**
   * @brief readJson
   * @return
   */
  void readJson(const QJsonObject& json) override;

  /**
   * @brief writeJson
   * @return
   */
  void writeJson(QJsonObject& json) const override;

protected:
  /**
   * @brief PreflightUpdatedValueFilterParameter The default constructor.  It is protected because this
   * filter parameter should only be instantiated using its New(...) function or short-form macro.
   */
  PreflightUpdatedValueFilterParameter();

public:
  PreflightUpdatedValueFilterParameter(const PreflightUpdatedValueFilterParameter&) = delete;            // Copy Constructor Not Implemented
  PreflightUpdatedValueFilterParameter(PreflightUpdatedValueFilterParameter&&) = delete;                 // Move Constructor Not Implemented
  PreflightUpdatedValueFilterParameter& operator=(const PreflightUpdatedValueFilterParameter&) = delete; // Copy Assignment Not Implemented
  PreflightUpdatedValueFilterParameter& operator=(PreflightUpdatedValueFilterParameter&&) = delete;      // Move Assignment Not Implemented

private:
  PreflightUpdatedValueFilterParameter::GetterCallbackType m_GetterCallback = {};
};
