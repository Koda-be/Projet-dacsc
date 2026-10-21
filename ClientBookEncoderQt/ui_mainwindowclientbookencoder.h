/********************************************************************************
** Form generated from reading UI file 'mainwindowclientbookencoder.ui'
**
** Created by: Qt User Interface Compiler version 5.12.5
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOWCLIENTBOOKENCODER_H
#define UI_MAINWINDOWCLIENTBOOKENCODER_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindowClientBookEncoder
{
public:
    QAction *actionLogin;
    QAction *actionLogout;
    QAction *actionQuitter;
    QWidget *centralwidget;
    QTableWidget *tableWidgetEncodedBooks;
    QLabel *label;
    QLineEdit *lineEditTitle;
    QLabel *label_2;
    QComboBox *comboBoxAuthors;
    QPushButton *pushButtonAddAuthor;
    QLabel *label_3;
    QComboBox *comboBoxSubjects;
    QPushButton *pushButtonAddSubject;
    QLineEdit *lineEditIsbn;
    QLabel *label_4;
    QSpinBox *spinBoxPageCount;
    QLabel *label_5;
    QLabel *label_6;
    QSpinBox *spinBoxPublishYear;
    QDoubleSpinBox *doubleSpinBoxPrice;
    QLabel *label_7;
    QSpinBox *spinBoxStockQuantity;
    QLabel *label_8;
    QPushButton *pushButtonClear;
    QLabel *label_9;
    QPushButton *pushButtonAddBook;
    QMenuBar *menubar;
    QMenu *menuSession;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *MainWindowClientBookEncoder)
    {
        if (MainWindowClientBookEncoder->objectName().isEmpty())
            MainWindowClientBookEncoder->setObjectName(QString::fromUtf8("MainWindowClientBookEncoder"));
        MainWindowClientBookEncoder->resize(1068, 326);
        actionLogin = new QAction(MainWindowClientBookEncoder);
        actionLogin->setObjectName(QString::fromUtf8("actionLogin"));
        actionLogout = new QAction(MainWindowClientBookEncoder);
        actionLogout->setObjectName(QString::fromUtf8("actionLogout"));
        actionQuitter = new QAction(MainWindowClientBookEncoder);
        actionQuitter->setObjectName(QString::fromUtf8("actionQuitter"));
        centralwidget = new QWidget(MainWindowClientBookEncoder);
        centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
        tableWidgetEncodedBooks = new QTableWidget(centralwidget);
        tableWidgetEncodedBooks->setObjectName(QString::fromUtf8("tableWidgetEncodedBooks"));
        tableWidgetEncodedBooks->setGeometry(QRect(10, 180, 1051, 101));
        label = new QLabel(centralwidget);
        label->setObjectName(QString::fromUtf8("label"));
        label->setGeometry(QRect(10, 10, 64, 21));
        lineEditTitle = new QLineEdit(centralwidget);
        lineEditTitle->setObjectName(QString::fromUtf8("lineEditTitle"));
        lineEditTitle->setGeometry(QRect(70, 10, 461, 25));
        label_2 = new QLabel(centralwidget);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setGeometry(QRect(550, 10, 64, 21));
        comboBoxAuthors = new QComboBox(centralwidget);
        comboBoxAuthors->setObjectName(QString::fromUtf8("comboBoxAuthors"));
        comboBoxAuthors->setGeometry(QRect(620, 10, 291, 25));
        pushButtonAddAuthor = new QPushButton(centralwidget);
        pushButtonAddAuthor->setObjectName(QString::fromUtf8("pushButtonAddAuthor"));
        pushButtonAddAuthor->setGeometry(QRect(930, 10, 131, 25));
        pushButtonAddAuthor->setStyleSheet(QString::fromUtf8("background-color: lightblue"));
        label_3 = new QLabel(centralwidget);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        label_3->setGeometry(QRect(550, 50, 64, 21));
        comboBoxSubjects = new QComboBox(centralwidget);
        comboBoxSubjects->setObjectName(QString::fromUtf8("comboBoxSubjects"));
        comboBoxSubjects->setGeometry(QRect(620, 50, 291, 25));
        pushButtonAddSubject = new QPushButton(centralwidget);
        pushButtonAddSubject->setObjectName(QString::fromUtf8("pushButtonAddSubject"));
        pushButtonAddSubject->setGeometry(QRect(930, 50, 131, 25));
        pushButtonAddSubject->setStyleSheet(QString::fromUtf8("background-color: lightblue"));
        lineEditIsbn = new QLineEdit(centralwidget);
        lineEditIsbn->setObjectName(QString::fromUtf8("lineEditIsbn"));
        lineEditIsbn->setGeometry(QRect(70, 40, 331, 25));
        label_4 = new QLabel(centralwidget);
        label_4->setObjectName(QString::fromUtf8("label_4"));
        label_4->setGeometry(QRect(10, 40, 64, 21));
        spinBoxPageCount = new QSpinBox(centralwidget);
        spinBoxPageCount->setObjectName(QString::fromUtf8("spinBoxPageCount"));
        spinBoxPageCount->setGeometry(QRect(70, 70, 81, 26));
        spinBoxPageCount->setMaximum(2000);
        spinBoxPageCount->setSingleStep(10);
        spinBoxPageCount->setValue(0);
        label_5 = new QLabel(centralwidget);
        label_5->setObjectName(QString::fromUtf8("label_5"));
        label_5->setGeometry(QRect(10, 70, 64, 21));
        label_6 = new QLabel(centralwidget);
        label_6->setObjectName(QString::fromUtf8("label_6"));
        label_6->setGeometry(QRect(170, 70, 151, 21));
        spinBoxPublishYear = new QSpinBox(centralwidget);
        spinBoxPublishYear->setObjectName(QString::fromUtf8("spinBoxPublishYear"));
        spinBoxPublishYear->setGeometry(QRect(320, 70, 81, 26));
        spinBoxPublishYear->setMinimum(1900);
        spinBoxPublishYear->setMaximum(2100);
        spinBoxPublishYear->setValue(1900);
        doubleSpinBoxPrice = new QDoubleSpinBox(centralwidget);
        doubleSpinBoxPrice->setObjectName(QString::fromUtf8("doubleSpinBoxPrice"));
        doubleSpinBoxPrice->setGeometry(QRect(70, 100, 81, 26));
        doubleSpinBoxPrice->setMaximum(999.990000000000009);
        doubleSpinBoxPrice->setSingleStep(0.500000000000000);
        label_7 = new QLabel(centralwidget);
        label_7->setObjectName(QString::fromUtf8("label_7"));
        label_7->setGeometry(QRect(10, 100, 64, 21));
        spinBoxStockQuantity = new QSpinBox(centralwidget);
        spinBoxStockQuantity->setObjectName(QString::fromUtf8("spinBoxStockQuantity"));
        spinBoxStockQuantity->setGeometry(QRect(320, 100, 81, 26));
        spinBoxStockQuantity->setMaximum(1000);
        label_8 = new QLabel(centralwidget);
        label_8->setObjectName(QString::fromUtf8("label_8"));
        label_8->setGeometry(QRect(270, 100, 51, 21));
        pushButtonClear = new QPushButton(centralwidget);
        pushButtonClear->setObjectName(QString::fromUtf8("pushButtonClear"));
        pushButtonClear->setGeometry(QRect(410, 40, 121, 81));
        pushButtonClear->setStyleSheet(QString::fromUtf8("background-color: orange"));
        label_9 = new QLabel(centralwidget);
        label_9->setObjectName(QString::fromUtf8("label_9"));
        label_9->setGeometry(QRect(10, 150, 521, 21));
        pushButtonAddBook = new QPushButton(centralwidget);
        pushButtonAddBook->setObjectName(QString::fromUtf8("pushButtonAddBook"));
        pushButtonAddBook->setGeometry(QRect(550, 90, 511, 31));
        pushButtonAddBook->setStyleSheet(QString::fromUtf8("background-color: lightgreen;\n"
"font-size: 16px;"));
        MainWindowClientBookEncoder->setCentralWidget(centralwidget);
        menubar = new QMenuBar(MainWindowClientBookEncoder);
        menubar->setObjectName(QString::fromUtf8("menubar"));
        menubar->setGeometry(QRect(0, 0, 1068, 22));
        menuSession = new QMenu(menubar);
        menuSession->setObjectName(QString::fromUtf8("menuSession"));
        MainWindowClientBookEncoder->setMenuBar(menubar);
        statusbar = new QStatusBar(MainWindowClientBookEncoder);
        statusbar->setObjectName(QString::fromUtf8("statusbar"));
        MainWindowClientBookEncoder->setStatusBar(statusbar);

        menubar->addAction(menuSession->menuAction());
        menuSession->addAction(actionLogin);
        menuSession->addAction(actionLogout);
        menuSession->addSeparator();
        menuSession->addAction(actionQuitter);

        retranslateUi(MainWindowClientBookEncoder);

        QMetaObject::connectSlotsByName(MainWindowClientBookEncoder);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindowClientBookEncoder)
    {
        MainWindowClientBookEncoder->setWindowTitle(QApplication::translate("MainWindowClientBookEncoder", "Application Encodage", nullptr));
        actionLogin->setText(QApplication::translate("MainWindowClientBookEncoder", "Login", nullptr));
        actionLogout->setText(QApplication::translate("MainWindowClientBookEncoder", "Logout", nullptr));
        actionQuitter->setText(QApplication::translate("MainWindowClientBookEncoder", "Quitter", nullptr));
        label->setText(QApplication::translate("MainWindowClientBookEncoder", "Titre : ", nullptr));
        label_2->setText(QApplication::translate("MainWindowClientBookEncoder", "Auteur :", nullptr));
        pushButtonAddAuthor->setText(QApplication::translate("MainWindowClientBookEncoder", "Ajouter", nullptr));
        label_3->setText(QApplication::translate("MainWindowClientBookEncoder", "Sujet :", nullptr));
        pushButtonAddSubject->setText(QApplication::translate("MainWindowClientBookEncoder", "Ajouter", nullptr));
        label_4->setText(QApplication::translate("MainWindowClientBookEncoder", "ISBN :", nullptr));
        label_5->setText(QApplication::translate("MainWindowClientBookEncoder", "Pages :", nullptr));
        label_6->setText(QApplication::translate("MainWindowClientBookEncoder", "Ann\303\251e du publication :", nullptr));
        label_7->setText(QApplication::translate("MainWindowClientBookEncoder", "Prix :", nullptr));
        label_8->setText(QApplication::translate("MainWindowClientBookEncoder", "Stock :", nullptr));
        pushButtonClear->setText(QApplication::translate("MainWindowClientBookEncoder", "Vider les champs", nullptr));
        label_9->setText(QApplication::translate("MainWindowClientBookEncoder", "<html><head/><body><p><span style=\" font-size:14pt; font-weight:600; color:#204a87;\">Liste des livres encod\303\251s lors de cette session :</span></p></body></html>", nullptr));
        pushButtonAddBook->setText(QApplication::translate("MainWindowClientBookEncoder", "Ajouter nouveau livre", nullptr));
        menuSession->setTitle(QApplication::translate("MainWindowClientBookEncoder", "Session", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindowClientBookEncoder: public Ui_MainWindowClientBookEncoder {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOWCLIENTBOOKENCODER_H
