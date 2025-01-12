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

#include <QtCore/QJsonArray>
#include <QtCore/QMap>
#include <QtCore/QMapIterator>
#include <QtCore/QString>
#include <QtCore/QUuid>

#include "SIMPLib/SIMPLib.h"
#include "SIMPLib/Filtering/IFilterFactory.hpp"

#ifdef SIMPL_EMBED_PYTHON
#include <QtCore/QSet>
#endif

/**
 * @brief The FilterManager class manages instances of filters and is mainly used to instantiate
 * an instance of a filter given its human label or class name. This class uses the Factory design
 * pattern.
 */
class SIMPLib_EXPORT FilterManager
{
public:
  /**
   * @brief Returns the name of the class for FilterManager
   */
  QString getNameOfClass() const;
  /**
   * @brief Returns the name of the class for FilterManager
   */
  static QString ClassName();

  virtual ~FilterManager();

  typedef QMap<QString, IFilterFactory::Pointer> Collection;
  typedef QMapIterator<QString, IFilterFactory::Pointer> CollectionIterator;

  typedef QMap<QUuid, IFilterFactory::Pointer> UuidCollection;
  typedef QMapIterator<QUuid, IFilterFactory::Pointer> UuidCollectionIterator;

  /**
   * @brief Static instance to retrieve the global instance of this class
   * @return
   */
  static FilterManager* Instance();

  /**
   * @brief Registers a QFilterFactory instance for a give name
   * @param name The name of the filter
   * @param factory An instance of the factory
   */
  static void RegisterFilterFactory(const QString& name, IFilterFactory::Pointer factory);

  /**
   * @brief RegisterKnownFilters This filter registers a factory for each filter that is included
   * in the core of DREAM3DLib. This method is called when the singleton instance of this class is
   * first created.
   */
  static void RegisterKnownFilters(FilterManager* fm);

  /**
   * @brief FilterManager::printFactoryNames
   */
  void printFactoryNames() const;

  /**
   * @brief Returns the mapping of names to Factory instances for all the
   * factories that are registered.
   * @return
   */
  Collection getFactories() const;

  /**
   * @brief Returns the mapping of names to the Factory instances for a given filter group
   * @param groupName The name of the group.
   * @return
   */
  Collection getFactories(const QString& groupName);

  /**
   * @brief Returns the mapping of names to the Factory instances for a given filter subgroup
   * @param subGroupName The name of the subgroup.
   * @return
   */
  Collection getFactories(const QString& groupName, const QString& subGroupName);

  /**
   * @brief Returns true if it contains a filter factory with the given UUID
   * @param uuid
   * @return
   */
  bool contains(const QUuid& uuid) const;

  /**
   * @brief Adds a Factory that creates QFilters
   * @param name
   * @param factory
   */
  void addFilterFactory(const QString& name, IFilterFactory::Pointer factory);

  /**
   * @brief Removes the given filter factory by UUID. Returns true if successful
   * @param uuid
   * @return
   */
  bool removeFilterFactory(const QUuid& uuid);

  /**
   * @brief getGroupNames Returns the uniqe set of group names for all the filters
   * @return
   */
  QSet<QString> getGroupNames();

  /**
   * @brief getSubGroupNames For a given group, returns all the subgroups
   * @param groupName The name of the Filter group
   * @return
   */
  QSet<QString> getSubGroupNames(const QString& groupName);

  /**
   * @brief getFactoryFromClassName Returns a FilterFactory for a given filter
   * @param filterName
   * @return
   */
  IFilterFactory::Pointer getFactoryFromClassName(const QString& filterName) const;

  /**
   * @brief getFactoryFromClassName Returns a FilterFactory for a given filter
   * @param filterName
   * @return
   */
  IFilterFactory::Pointer getFactoryFromUuid(const QUuid& uuid) const;

  /**
   * @brief getFactoryFromClassNameHumanName For a given human label, the FilterFactory is given
   * @param humanName
   * @return
   */
  IFilterFactory::Pointer getFactoryFromHumanName(const QString& humanName);

  /**
   * @brief This will return a QJsonArray object that contains information about
   * all available filters
   * @return
   */
  QJsonArray toJsonArray() const;

#ifdef SIMPL_EMBED_PYTHON
  /**
   * @brief Adds a factory that creates Python filters
   * @param name
   * @param factory
   */
  void addPythonFilterFactory(const QString& name, IFilterFactory::Pointer factory);

  /**
   * @brief Returns set of Python filter uuids
   * @return
   */
  QSet<QUuid> pythonFilterUuids() const;

  /**
   * @brief Returns true if the given uuid is for a Python filter
   * @param uuid
   * @return
   */
  bool isPythonFilter(const QUuid& uuid) const;

  /**
   * @brief Clears all Python filters
   */
  void clearPythonFilterFactories();
#endif

protected:
  FilterManager();

private:
  Collection m_Factories;
  UuidCollection m_UuidFactories;

#ifdef SIMPL_EMBED_PYTHON
  QSet<QUuid> m_PythonUuids;
#endif

  static FilterManager* s_Self;

public:
  FilterManager(const FilterManager&) = delete;            // Copy Constructor Not Implemented
  FilterManager(FilterManager&&) = delete;                 // Move Constructor Not Implemented
  FilterManager& operator=(const FilterManager&) = delete; // Copy Assignment Not Implemented
  FilterManager& operator=(FilterManager&&) = delete;      // Move Assignment Not Implemented
};
