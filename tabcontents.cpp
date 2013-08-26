#include <QDebug>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlQueryModel>
#include <QSqlTableModel>
#include <QSqlRelationalTableModel>
#include <QSqlRelationalDelegate>
#include <QFileDialog>

#include "tabcontents.h"
#include "customquerydiagnostics.h"
#include "legendwindow.h"
#include "etimologywindow.h"
#include "phonologywindow.h"
#include "taleswindow.h"
#include "paradigmwindow.h"
#include "playwindow.h"

#include "ui_tabcontents.h"


TabContents::TabContents(DictGlobalAttributes _dictAttrs, QWidget *parent) :
  dictAttrs(_dictAttrs),
  QWidget(parent),
  db(QSqlDatabase::database(dictAttrs.getFilename())),
  ui(new Ui::TabContents),
  soundChosen(false),
  praatChosen(false)
{
  ui->setupUi(this);

  if (!db.isValid()) {
      qDebug() << "Error during tab creation: Database is " << db.isOpen() << " and it's because " << db.isOpenError();
      return;
  }

  
  connect(ui->soundButton, SIGNAL(clicked()), this, SLOT(chooseSound()));
  connect(ui->praatButton, SIGNAL(clicked()), this, SLOT(choosePraat()));

  connect(ui->submitButton , SIGNAL(clicked()), this, SLOT(submitWord()));
  connect(ui->legendButton , SIGNAL(clicked()), this, SLOT(showLegend()));
  connect(ui->etimologyButton , SIGNAL(clicked()), this, SLOT(showEtimology()));
  connect(ui->paradigmButton , SIGNAL(clicked()), this, SLOT(showParadigm()));
  connect(ui->playButton , SIGNAL(clicked()), this, SLOT(showPlay()));
  connect(ui->phonologyButton , SIGNAL(clicked()), this, SLOT(showPhonology()));
  connect(ui->talesButton , SIGNAL(clicked()), this, SLOT(showTales()));

  dictModel = new QSqlTableModel(this, db);
  soundsModel = new QSqlTableModel(this, db);
  praatModel = new QSqlTableModel(this, db);

  initializePraatModel(praatModel);
  initializeSoundsModel(soundsModel);
  initializeDictModel(dictModel);

  ui->dictionaryTable->setModel(dictModel);
  ui->dictionaryTable->setColumnHidden(0,true);
  ui->dictionaryTable->setColumnHidden(2,true);
  ui->dictionaryTable->setColumnHidden(5,true);
  ui->dictionaryTable->setColumnHidden(6,true);
  ui->dictionaryTable->setColumnHidden(7,true);
  ui->dictionaryTable->setColumnHidden(8,true);
  ui->dictionaryTable->setSortingEnabled(true);
  ui->dictionaryTable->setColumnWidth(1, 100 );
  ui->dictionaryTable->setColumnWidth(3, 200);
  ui->dictionaryTable->setColumnWidth(4, 350);

  ui->soundsList->setModel(soundsModel);
  ui->soundsList->setModelColumn(2);

  ui->praatList->setModel(praatModel);
  ui->praatList->setModelColumn(2);

  connect(ui->dictionaryTable , SIGNAL(clicked(QModelIndex)), this, SLOT(filterBlobs()));

}

bool TabContents::chooseSound() {
  if (!this->soundChosen) {
      this->soundFilename = QFileDialog::getOpenFileName(this, tr("Выберите звуковой файл для вставки в словарь"), "", tr("Звуковые файлы (*.mp3 *.wav)"));
      qDebug() << "Sound file selected: " << this->soundFilename;
      if (!this->soundFilename.isEmpty()) {
        ui->soundButton->setText("Сбросить звук?");
        this->soundChosen = true;
      }
  } else {
      this->soundFilename.clear();
      ui->soundButton->setText("Выберите звуковой файл");
      this->soundChosen = false;
  }
  return true;
}

bool TabContents::choosePraat() {
  if (!this->praatChosen) {
      this->praatFilenameMarkup =  QDir::toNativeSeparators (QFileDialog::getOpenFileName(this,
                                                                                          tr("Выберите файл разметки Praat для вставки в словарь"),
                                                                                          "",
                                                                                          tr("Файлы разметки Praat (*.TextGrid)")));
      QFileInfo fileInfo(this->praatFilenameMarkup);
      this->praatFilenameSound = QDir::toNativeSeparators (fileInfo.canonicalPath().append(fileInfo.completeBaseName().append(".wav").prepend("/")));
      qDebug() << "Praat markup is selected: " << this->praatFilenameMarkup;
      qDebug() << "Praat sound is selected: " << this->praatFilenameSound;
      if (!this->praatFilenameMarkup.isEmpty()) {
        ui->praatButton->setText("Сбросить выбор?");
        this->praatChosen = true;
      }
  } else {
      this->praatFilenameMarkup.clear();
      this->praatFilenameSound.clear();
      ui->praatButton->setText("Выберите файл разметки");
      this->praatChosen = false;
  }
  return true;
}

bool TabContents::showLegend() {
  LegendWindow legend(this);
  legend.exec();
  return true;
}

bool TabContents::showEtimology() {
  EtimologyWindow etimology(this);
  etimology.exec();
  return true;
}

bool TabContents::showParadigm() {
  ParadigmWindow paradigm(this);
  paradigm.exec();
  return true;
}

bool TabContents::showPlay() {
  QStringList playlist;
  playlist.append("C:/Users/al/QTProjects/01. Air - Playground Love.mp3");
  PlayWindow play(playlist,this);
  play.exec();
  return true;
}

bool TabContents::showPhonology() {
  PhonologyWindow phonology(this);
  phonology.exec();
  return true;
}

bool TabContents::showTales() {
  TalesWindow tales(this);
  tales.exec();
  return true;
}


TabContents::~TabContents()
{
  delete dictModel;
  delete soundsModel;
  delete praatModel;
  delete ui;
}

void TabContents::initializeDictModel(QSqlTableModel *model) {
  model->setTable("dictionary");
  model->setEditStrategy(QSqlTableModel::OnFieldChange);

  model->setHeaderData(0, Qt::Horizontal, QObject::tr("Номер"));
  model->setHeaderData(1, Qt::Horizontal, QObject::tr("Слово"));
  model->setHeaderData(2, Qt::Horizontal, QObject::tr("Регулярная форма"));
  model->setHeaderData(3, Qt::Horizontal, QObject::tr("Транскрипция"));
  model->setHeaderData(4, Qt::Horizontal, QObject::tr("Перевод"));

//  model->setRelation(0, QSqlRelation("dict_blobs_description", "name", "wordid"));
//  model->setFilter("relTblAl_0.'wordid' = dictionary.'id'");
//  model->setRelation(6, QSqlRelation("dict_blobs_description", "wordid", "description"));
  qDebug() << model->query().lastQuery();
  model->select();
  qDebug() << model->query().lastQuery();
  qDebug() << model->query().result();
}

bool TabContents::filterBlobs() {
  qDebug() << "filtering blobs";
  QString filterSoundsQuery = "dict_blobs_description.type = 1 and dict_blobs_description.wordid = ";
  QString filterPraatQuery = "dict_blobs_description.type = 2 and dict_blobs_description.wordid = ";

  QModelIndexList model_index = ui->dictionaryTable->selectionModel()->selection().indexes();
  int row = model_index.at(0).row();
  QString param = this->dictModel->data(this->dictModel->index(row,0)).toString();
  filterSoundsQuery.append(param);
  filterPraatQuery.append(param);
  const QString filtersounds(filterSoundsQuery);
  const QString filterpraat(filterPraatQuery);
  soundsModel->setFilter(filtersounds);
  praatModel->setFilter(filterpraat);
  qDebug() << soundsModel->query().lastQuery();
  qDebug() << praatModel->query().lastQuery();
  qDebug() << soundsModel->query().result();
  qDebug() << praatModel->query().result();

  return true;
}

void TabContents::initializeSoundsModel(QSqlTableModel *model) {
  model->setTable("dict_blobs_description");
  model->setEditStrategy(QSqlTableModel::OnFieldChange);

  model->setHeaderData(0, Qt::Horizontal, QObject::tr("Номер"));
  model->setHeaderData(1, Qt::Horizontal, QObject::tr("Тип"));
  model->setHeaderData(2, Qt::Horizontal, QObject::tr("Имя"));
  model->setHeaderData(3, Qt::Horizontal, QObject::tr("Описание"));
  model->setHeaderData(4, Qt::Horizontal, QObject::tr("Ссылка на слово"));
  model->setHeaderData(5, Qt::Horizontal, QObject::tr("Ссылка на блоб"));

  model->setFilter("dict_blobs_description.type = 1");

  model->select();
}

void TabContents::initializePraatModel(QSqlTableModel *model) {
  model->setTable("dict_blobs_description");
  model->setEditStrategy(QSqlTableModel::OnFieldChange);

  model->setHeaderData(0, Qt::Horizontal, QObject::tr("Номер"));
  model->setHeaderData(1, Qt::Horizontal, QObject::tr("Тип"));
  model->setHeaderData(2, Qt::Horizontal, QObject::tr("Имя"));
  model->setHeaderData(3, Qt::Horizontal, QObject::tr("Описание"));
  model->setHeaderData(4, Qt::Horizontal, QObject::tr("Ссылка на слово"));
  model->setHeaderData(5, Qt::Horizontal, QObject::tr("Ссылка на блоб"));

  model->setFilter("dict_blobs_description.type = 2");

  model->select();

}


void errorMsg (const QString& error_msg){
  QMessageBox errormsg;
  errormsg.setText(error_msg);
  errormsg.setIcon(QMessageBox::Critical);
  errormsg.setDefaultButton(QMessageBox::Ok);
  errormsg.exec();
  return;
}

bool TabContents::isValidInput(void) {
  // 1 -- text inputs are blank
  if (ui->wordForm->text().isEmpty() && ui->transcritptionForm->text().isEmpty() && ui->translationForm->text().isEmpty()) {
      errorMsg("Вы должны заполнить хотя бы одно из текстовых полей для создания слова");
      return false;
  }
  // 2 -- sound is selected, but no description
  if (!this->soundFilename.isEmpty() && ui->soundDescription->text().isEmpty()) {
      errorMsg("Вы выбрали звук, но не написали описание: так нельзя");
      return false;
  }
  // 3 -- sound is not selected, but description is ready
  if (this->soundFilename.isEmpty() && !ui->soundDescription->text().isEmpty()) {
      errorMsg("Вы описали аудиозапись, но не выбрали её: так нельзя");
      return false;
  }
  // 4 -- sound file doesn't exist
  if (!this->soundFilename.isEmpty() && !QFile(this->soundFilename).exists()) {
      errorMsg("Кажется, аудиофайл изчез, пока вы заполняли остальные поля. Сбросьте выбор и попробуйте ещё раз");
      return false;
  }
  // 5 -- praat is selected, but no description
  if ( !(this->praatFilenameSound.isEmpty() || this->praatFilenameMarkup.isEmpty()) && ui->praatDescription->text().isEmpty()) {
      errorMsg("Вы выбрали файлы для Praat, но не описали их; так нельзя");
      return false;
  }
  // 6 -- praat is not selected, but description is ready
  if ((this->praatFilenameSound.isEmpty() || this->praatFilenameMarkup.isEmpty()) && !ui->praatDescription->text().isEmpty()) {
      errorMsg("Вы описали файлы для Praat, но не выбрали их, это нужно сделать");
      return false;
  }
  // 7 -- praat markup doesn't exist
  if (!(this->praatFilenameMarkup.isEmpty()) && !QFile(this->praatFilenameMarkup).exists()) {
      qDebug() << !(this->praatFilenameMarkup.isEmpty());
      qDebug() << QFile(this->praatFilenameMarkup).exists();
      errorMsg("Похоже, к файлу разметки Praat нет файла звука. Проверьте, что они называются одинаково (за исключением расширений .wav и .praat) и что оба файла лежат в одной и той же папке");
      return false;
  }
  // 8 -- praat sound doesn't exist
  if (!(this->praatFilenameSound.isEmpty()) && !QFile(this->praatFilenameSound).exists()) {
      errorMsg("Похоже, что вы выбрали файл звука, но парного файла разметки к нему не существует. Проверьте, что они называются одинаково (за исключением расширений .wav и .praat) и что оба файла лежат в одной и той же папке");
      return false;
  }

  // 0 -- everything is ok
  return true;
}

DictEntry TabContents::readFields()
{
  QString word(ui->wordForm->text());
  QString transcription(ui->transcritptionForm->text());
  QString translation(ui->translationForm->text());

  QString soundDescription;
  QString praatDescription;

  if (this->soundChosen) {
      soundDescription = (ui->soundDescription->text());
  } else {
      this->soundFilename = "";
  }

  if (this->praatChosen) {
      praatDescription = (ui->praatDescription->text());
  } else {
      this->praatFilenameMarkup = "";
      this->praatFilenameSound = "";
      praatDescription = "";
  }

  return DictEntry(this,
               word,
               transcription,
               translation,
               this->soundFilename,
               soundDescription,
               this->praatFilenameMarkup,
               this->praatFilenameSound,
               praatDescription);

}

bool TabContents::submitWord()
{
  /* check that we have at least one filled field */
  if (!this->isValidInput()) {
      return false;
  }

  DictEntry entry(readFields());
  qDebug() << "Now we should get an entry to sqlite";


  QSqlRecord record = dictModel->record();
  record.setValue("word", entry.getWord());
  record.setValue("regular_form" , entry.getTranscription());
  record.setValue("transcription" , entry.getTranscription());
  record.setValue("translation", entry.getTranslation());
  record.setValue("is_a_regular_form" , true);
  record.setValue("has_paradigm", false);
  dictModel->insertRecord(-1, record);

  dictModel->submitAll();
  qDebug() << "after set data: " << dictModel->query().lastQuery();

  record.clear();

  QVariant wordid = dictModel->query().lastInsertId();

  if (this->soundChosen || this->praatChosen) {
    QVariant soundblobid;

    // here we need to push sound to blobs table, get id of blob and make a record in blobs_description_table
    if (this->soundChosen) {
        QSqlQuery soundBlobQuery(db);
        soundBlobQuery.prepare("INSERT INTO blobs(mainblob) values (:soundblob)");
        soundBlobQuery.bindValue(":soundblob", *(entry.getSoundPointer()));
        if (!soundBlobQuery.exec()) {
           //TODO: Cleanup
            qDebug() << "Can't insert sound blob";
            qDebug() << "Query was the following: " << getLastExecutedQuery(soundBlobQuery);
            qDebug() << db.lastError().text();
            this->clearForms();
            return false;
        }
        soundblobid = soundBlobQuery.lastInsertId();
        soundBlobQuery.clear();

        QSqlQuery soundDescriptionQuery(db);
        soundDescriptionQuery.prepare("INSERT INTO dict_blobs_description(type,name,description,wordid,blobid)"
                                       "VALUES (1,:name,:description,:wordid,:blobid)");
        soundDescriptionQuery.bindValue(":name", entry.getSoundFilename());
        soundDescriptionQuery.bindValue(":description", entry.getSoundDescription());
        soundDescriptionQuery.bindValue(":wordid", wordid);
        soundDescriptionQuery.bindValue(":blobid", soundblobid);

        if (!soundDescriptionQuery.exec()) {
           //TODO: Cleanup
            qDebug() << "Can't insert sound blob description";
            qDebug() << "Query was the following: " << getLastExecutedQuery(soundDescriptionQuery);
            qDebug() << db.lastError().text();
            this->clearForms();
            return false;
        }
        qDebug() << "Successfully submitted sound";
    }
  }

  if (this->praatChosen) {
      QVariant praatblobid;

      QSqlQuery praatBlobQuery(db);
      praatBlobQuery.prepare("INSERT INTO blobs(mainblob,secblob) values (:praatmarkupblob,:praatsoundblob)");
      praatBlobQuery.bindValue(":praatmarkupblob", *(entry.getPraatMarkupPointer()));
      praatBlobQuery.bindValue(":praatsoundblob", *(entry.getPraatSoundPointer()));
      if (!praatBlobQuery.exec()) {
         //TODO: Cleanup
          qDebug() << "Can't insert praat blob";
          qDebug() << "Query was the following: " << getLastExecutedQuery(praatBlobQuery);
          qDebug() << db.lastError().text();
          this->clearForms();
          return false;
      }
      praatblobid = praatBlobQuery.lastInsertId();
      praatBlobQuery.clear();

      QSqlQuery praatDescriptionQuery(db);
      praatDescriptionQuery.prepare("INSERT INTO dict_blobs_description(type,name,description,wordid,blobid)"
                                     "VALUES (2,:name,:description,:wordid,:blobid)");
      praatDescriptionQuery.bindValue(":name", entry.getBasePraatMarkupFilename());
      praatDescriptionQuery.bindValue(":description", entry.getPraatDescription());
      praatDescriptionQuery.bindValue(":wordid", wordid);
      praatDescriptionQuery.bindValue(":blobid", praatblobid);

      if (!praatDescriptionQuery.exec()) {
         //TODO: Cleanup
          qDebug() << "Can't insert sound blob description";
          qDebug() << "Query was the following: " << getLastExecutedQuery(praatDescriptionQuery);
          qDebug() << db.lastError().text();
          this->clearForms();
          return false;
      }
      qDebug() << "Successfully submitted praat blob";
  }

//  dictModel->database().commit();
  qDebug() << "Executed the query on commit: " << dictModel->query().lastQuery();
  dictModel->select();



  this->clearForms();
  return true;
}

void TabContents::clearForms() {
  ui->wordForm->clear();
  ui->transcritptionForm->clear();
  ui->translationForm->clear();
  ui->soundDescription->clear();
  ui->praatDescription->clear();
  this->praatChosen = false;
  ui->praatButton->setText("Выберите файл разметки");
  this->soundChosen = false;
  ui->soundButton->setText("Выберите звуковой файл");
  this->soundFilename.clear();
  this->praatFilenameMarkup.clear();
  this->praatFilenameSound.clear();
}
