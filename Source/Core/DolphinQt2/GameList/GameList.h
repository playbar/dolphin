// Copyright 2015 Dolphin Emulator Project
// Licensed under GPLv2+
// Refer to the license.txt file included.

#pragma once

#include <QLabel>
#include <QListView>
#include <QSortFilterProxyModel>
#include <QStackedWidget>
#include <QTableView>

#include "DolphinQt2/GameList/GameFile.h"
#include "DolphinQt2/GameList/GameListModel.h"

class GameList final : public QStackedWidget
{
  Q_OBJECT

public:
  explicit GameList(QWidget* parent = nullptr);
  QString GetSelectedGame() const;

public slots:
  void SetTableView() { SetPreferredView(true); }
  void SetListView() { SetPreferredView(false); }
  void SetViewColumn(int col, bool view) { m_table->setColumnHidden(col, !view); }
  void OnColumnVisibilityToggled(const QString& row, bool visible);

private slots:
  void ShowContextMenu(const QPoint&);
  void OpenContainingFolder();
  void OpenProperties();
  void OpenSaveFolder();
  void OpenWiki();
  void SetDefaultISO();
  void DeleteFile();
  void InstallWAD();
  void UninstallWAD();
  void ExportWiiSave();
  void CompressISO();
  void OnHeaderViewChanged();

signals:
  void GameSelected();
  void EmulationStarted();
  void EmulationStopped();

private:
  void MakeTableView();
  void MakeListView();
  void MakeEmptyView();
  // We only have two views, just use a bool to distinguish.
  void SetPreferredView(bool table);
  void ConsiderViewChange();

  GameListModel* m_model;
  QSortFilterProxyModel* m_table_proxy;
  QSortFilterProxyModel* m_list_proxy;

  QListView* m_list;
  QTableView* m_table;
  QLabel* m_empty;
  bool m_prefer_table;

protected:
  void keyReleaseEvent(QKeyEvent* event) override;
};
