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

#include <QtCore/QSettings>
#include <QtCore/QString>
#include <QtWidgets/QTreeWidgetItem>

#include "SVWidgetsLib/QtSupport/QtSFileDragMessageBox.h"
#include "SVWidgetsLib/QtSupport/QtSSettings.h"

#include "SIMPLib/Common/Constants.h"

#include "SVWidgetsLib/SVWidgetsLib.h"

#include "ui_BookmarksToolboxWidget.h"

class QListWidget;
class QTreeWidgetItem;
class FilterListToolboxWidget;
class FilterLibraryTreeWidget;
class QSettings;
class QAction;

/**
 * @brief The BookmarksToolboxWidget class
 */
class SVWidgetsLib_EXPORT BookmarksToolboxWidget : public QWidget, private Ui::BookmarksToolboxWidget
{

  Q_OBJECT
public:
  /**
   * @brief BookmarksToolboxWidget
   * @param parent
   */
  BookmarksToolboxWidget(QWidget* parent = nullptr);
  ~BookmarksToolboxWidget() override;

  /**
   * @brief setupGui
   */
  virtual void setupGui();

  /**
   * @brief connectFilterList
   * @param filterListWidget
   */
  void connectFilterList(FilterListToolboxWidget* filterListWidget);

  /**
     @brief Delete a directory along with all of its contents.
     @param dirName Path of directory to remove.
     @return true on success; false on error.
  */
  static bool removeDir(const QString& dirName);

  /**
   * @brief getBookmarksTreeView
   * @return
   */
  BookmarksTreeView* getBookmarksTreeView();

  /**
   * @brief Reads the preferences from the users pref file
   */
  void readSettings(QtSSettings* prefs);

  /**
   * @brief Writes the preferences to the users pref file
   */
  void writeSettings(QtSSettings* prefs);

  /**
   * @brief writeSettings
   */
  void writeSettings();

  virtual QDir findV4FavoritesDirectory();

protected:
  QStringList generateFilterListFromPipelineFile(QString path);
  QString generateHtmlFilterListFromPipelineFile(QString path);

  void populateFilterList(QStringList filterNames);
  QString writeNewFavoriteFilePath(QString newFavoriteTitle, QString favoritePath, QTreeWidgetItem* item);

protected Q_SLOTS:

  //// Slots to catch signals from the QTreeWidget
  void on_bookmarksTreeView_clicked(const QModelIndex& index);
  void on_bookmarksTreeView_doubleClicked(const QModelIndex& index);

  void listenLocateBookmarkTriggered();

Q_SIGNALS:

  void fireWriteSettings();

  /**
   * @brief pipelineNeedsToBeSaved
   * @param path The absolute path to the pipeline file
   * @param name The name that the favorite will show up as in the GUI
   */
  void pipelineNeedsToBeSaved(const QString& path, const QString& name);

  /**
   * @brief filterListGenerated
   * @param filterList
   */
  void filterListGenerated(const QStringList& filterList, bool sort);

  /**
   * @brief This signal is emitted when a new SIMPLView instance is needed.  A filePath is used to populate the new
   * SIMPLView instance with a pipeline, if necessary.
   * @param filePath The absolute path to the pipeline file.  If empty, the instance will not have a pipeline in it.
   * @param execute A boolean that decides whether to execute the pipeline or not
   */
  void bookmarkActivated(const QString& filePath, bool execute = false);

  void raiseBookmarksDockWidget();

  void updateStatusBar(const QString& msg);

private:
  QString m_OpenDialogLastFilePath;

  /**
   * @brief serializeTreePath
   * @param treePath
   */
  QList<QString> deserializeTreePath(QString treePath);

  /**
   * @brief locateBookmark
   * @return
   */
  bool locateBookmark();

public:
  BookmarksToolboxWidget(const BookmarksToolboxWidget&) = delete;            // Copy Constructor Not Implemented
  BookmarksToolboxWidget(BookmarksToolboxWidget&&) = delete;                 // Move Constructor Not Implemented
  BookmarksToolboxWidget& operator=(const BookmarksToolboxWidget&) = delete; // Copy Assignment Not Implemented
  BookmarksToolboxWidget& operator=(BookmarksToolboxWidget&&) = delete;      // Move Assignment Not Implemented
};
