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

#include <functional>
#include <memory>

#include <QtCore/QString>
#include <QtCore/QVariant>

#include "SIMPLib/SIMPLib.h"
#include "SIMPLib/DataContainers/DataArrayPath.h"

class AbstractFilter;

/**
 * @class FilterParameter FilterParameter.h DREAM3DLib/FilterParameters/FilterParameter.h
 * @brief This class holds the various properties that an input parameter to a
 * filter needs to describe itself.
 *
 * @date Jan 17, 2012
 * @version 1.0
 */
class SIMPLib_EXPORT FilterParameter
{

  // clang-format off
  // Start Python bindings declarations
  PYB11_BEGIN_BINDINGS(FilterParameter)
  PYB11_SHARED_POINTERS(FilterParameter)

  PYB11_ENUMERATION(Category)
  PYB11_END_BINDINGS()
  // clang-format on
  // End Python bindings declarations
public:
  using Self = FilterParameter;
  using Pointer = std::shared_ptr<Self>;
  using ConstPointer = std::shared_ptr<const Self>;
  using WeakPointer = std::weak_ptr<Self>;
  using ConstWeakPointer = std::weak_ptr<const Self>;
  static Pointer NullPointer();

  /**
   * @brief Returns the name of the class for FilterParameter
   */
  virtual QString getNameOfClass() const;
  /**
   * @brief Returns the name of the class for FilterParameter
   */
  static QString ClassName();

  using EnumType = uint32_t;

  enum class Category : EnumType
  {
    Parameter = 0,
    RequiredArray = 1,
    CreatedArray = 2,
    Uncategorized = 3
  };

  virtual ~FilterParameter();

  /**
   * @brief Setter property for HumanLabel
   */
  void setHumanLabel(const QString& value);
  /**
   * @brief Getter property for HumanLabel
   * @return Value of HumanLabel
   */
  QString getHumanLabel() const;

  /**
   * @brief Setter property for PropertyName
   */
  void setPropertyName(const QString& value);
  /**
   * @brief Getter property for PropertyName
   * @return Value of PropertyName
   */
  QString getPropertyName() const;

  /**
   * @brief This sets an alternate or legacy property name that was used in past versions of a filter.
   *
   * This can be useful if the filter changed the name of the property and to keep old pipeline files working
   * @param value
   */
  void setLegacyPropertyName(const QString& value);

  /**
   * @brief Returns the legacy property name
   * @return
   */
  QString getLegacyPropertyName() const;

  /**
   * @brief getWidgetType This is a pure virtual function. All subclasses need
   * to implement this function.
   * @return
   */
  virtual QString getWidgetType() const = 0;

  /**
   * @brief Setter property for DefaultValue
   */
  virtual void setDefaultValue(const QVariant& value);

  /**
   * @brief getDefaultValue
   * @return
   */
  virtual QVariant getDefaultValue() const;

  /**
   * @brief Setter property for Category
   */
  void setCategory(const FilterParameter::Category& value);
  /**
   * @brief Getter property for Category
   * @return Value of Category
   */
  FilterParameter::Category getCategory() const;

  /**
   * @brief Setter property for ReadOnly
   */
  void setReadOnly(bool value);
  /**
   * @brief Getter property for ReadOnly
   * @return Value of ReadOnly
   */
  bool getReadOnly() const;

  /**
   * @brief Setter property for GroupIndex
   */
  void setGroupIndex(int value);
  /**
   * @brief Getter property for GroupIndex
   * @return Value of GroupIndex
   */
  int getGroupIndex() const;

  /**
   * @brief readJson
   * @return
   */
  virtual void readJson(const QJsonObject& json);

  /**
   * @brief writeJson
   * @return
   */
  virtual void writeJson(QJsonObject& json) const;

  /**
   * @brief Handle DataArrayPath changes if necessary
   * @param filter
   * @param renamePath
   */
  virtual void dataArrayPathRenamed(AbstractFilter* filter, const DataArrayPath::RenameType& renamePath);

protected:
  FilterParameter();

public:
  FilterParameter(const FilterParameter&) = delete;            // Copy Constructor Not Implemented
  FilterParameter(FilterParameter&&) = delete;                 // Move Constructor Not Implemented
  FilterParameter& operator=(const FilterParameter&) = delete; // Copy Assignment Not Implemented
  FilterParameter& operator=(FilterParameter&&) = delete;      // Move Assignment Not Implemented

private:
  QVariant m_DefaultValue = {};

  QString m_HumanLabel = {};
  QString m_PropertyName = {};
  QString m_LegacyPropertyName = {};
  FilterParameter::Category m_Category = {};
  bool m_ReadOnly = {};
  int m_GroupIndex = {};
};

using FilterParameterVectorType = std::vector<FilterParameter::Pointer>;

// -----------------------------------------------------------------------------
// This section of Macros allows each FilterParameter subclass to create a macro
// or set of macros that can lessen the amout of code that needs to be written
// in order to create an instantiation of the subclass. The technique used here
// is the 'paired, sliding list' of macro parameters that also makes use of
// __VA__ARGS__
// -----------------------------------------------------------------------------

#define SIMPL_BIND_SETTER(CLASS, PTR, PROP) std::bind(&CLASS::set##PROP, PTR, std::placeholders::_1)

#define SIMPL_BIND_GETTER(CLASS, PTR, PROP) std::bind(&CLASS::get##PROP, PTR)

// Define overrides that can be used by the expansion of our main macro.
// Each subclass can define a macro that takes up to nine (9) arguments
// to the constructor. These macros support a minimum of 4 arguments.

#define SIMPL_NEW_FP_9(Class, Desc, Prop, Category, Filter, Index, A, B, C, D)                                                                                                                         \
  Class::Create(Desc, #Prop, get##Prop(), Category, SIMPL_BIND_SETTER(Filter, this, Prop), SIMPL_BIND_GETTER(Filter, this, Prop), Index, A, B, C, D)

#define SIMPL_NEW_FP_8(Class, Desc, Prop, Category, Filter, Index, A, B, C)                                                                                                                            \
  Class::Create(Desc, #Prop, get##Prop(), Category, SIMPL_BIND_SETTER(Filter, this, Prop), SIMPL_BIND_GETTER(Filter, this, Prop), Index, A, B, C)

#define SIMPL_NEW_FP_7(Class, Desc, Prop, Category, Filter, Index, A, B)                                                                                                                               \
  Class::Create(Desc, #Prop, get##Prop(), Category, SIMPL_BIND_SETTER(Filter, this, Prop), SIMPL_BIND_GETTER(Filter, this, Prop), Index, A, B)

#define SIMPL_NEW_FP_6(Class, Desc, Prop, Category, Filter, Index, A)                                                                                                                                  \
  Class::Create(Desc, #Prop, get##Prop(), Category, SIMPL_BIND_SETTER(Filter, this, Prop), SIMPL_BIND_GETTER(Filter, this, Prop), Index, A)

#define SIMPL_NEW_FP_5(Class, Desc, Prop, Category, Filter, Index)                                                                                                                                     \
  Class::Create(Desc, #Prop, get##Prop(), Category, SIMPL_BIND_SETTER(Filter, this, Prop), SIMPL_BIND_GETTER(Filter, this, Prop), Index)

#define SIMPL_NEW_FP_4(Class, Desc, Prop, Category, Filter) Class::Create(Desc, #Prop, get##Prop(), Category, SIMPL_BIND_SETTER(Filter, this, Prop), SIMPL_BIND_GETTER(Filter, this, Prop))

#define SIMPL_NEW_PREFLIGHTUPDATEDVALUE_FP_5(Class, Desc, Prop, Category, Filter, Index) Class::Create(Desc, #Prop, get##Prop(), Category, SIMPL_BIND_GETTER(Filter, this, Prop), Index)

#define SIMPL_NEW_PREFLIGHTUPDATEDVALUE_FP_4(Class, Desc, Prop, Category, Filter) Class::Create(Desc, #Prop, get##Prop(), Category, SIMPL_BIND_GETTER(Filter, this, Prop))

/**
 * @brief This macro is needed for Visual Studio due to differences of VAR_ARGS when
 * passed to another macro that results in a new macro that needs expansion.
 */
#define SIMPL_EXPAND(x) x

// -----------------------------------------------------------------------------
// Define a macro that uses the "paired, sliding arg list"
// technique to select the appropriate override.
#define _FP_GET_OVERRIDE(A, B, C, D, E, F, G, H, I, NAME, ...) NAME
#define _FP_GET_PREFLIGHTUPDATEDVALUE_OVERRIDE(A, B, C, D, E, NAME, ...) NAME
