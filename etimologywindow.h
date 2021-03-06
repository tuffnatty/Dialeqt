#ifndef ETIMOLOGYWINDOW_H
#define ETIMOLOGYWINDOW_H

#include <QWidget>
#include <QDialog>
#include <QSqlDatabase>
#include <QSqlQueryModel>
#include "dictglobalattributes.h"

namespace Ui {
  class EtimologyWindow;
}

class EtimologyWindow : public QDialog
{
  Q_OBJECT
  QVariant wordid;
  QVariant wordtranscription;
  QVariant tag;
  QSqlDatabase db;
  QSqlQueryModel *model;
  QSqlQueryModel *wordsmodel;
  QSet<DictGlobalAttributes> * dictsOpened;
  QSet<QString> connectionNamesForDicts;
  QList<QPair<QString, QVariant>> wordsToConnect;
  QString wordsModelQuery;
public:
  explicit EtimologyWindow(QVariant _wordid, QVariant _word_transcription, QVariant _etimology_tag, QSqlDatabase _currentDb, QSet<DictGlobalAttributes> * _dictsOpened, QWidget *parent = 0);
  ~EtimologyWindow();

signals:
  bool goToWordInDict(QString dict, QVariant id);

private slots:
  bool findWords();
  bool openDbAndAddConnection();
  bool unlink();
  bool goToSelected();

private:
  Ui::EtimologyWindow *ui;
  void setupModel();
  bool checkConnectedDatabases();
  bool prepareForConnection();
};

#endif // ETIMOLOGYWINDOW_H
