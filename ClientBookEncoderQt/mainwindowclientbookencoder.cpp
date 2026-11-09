#include "mainwindowclientbookencoder.h"
#include "ui_mainwindowclientbookencoder.h"
#include "unistd.h"
#include <QInputDialog>
#include <QMessageBox>
#include <iostream>
#include <QLineEdit>
#include <QByteArray>
#include <cstring>
#include <cstdlib>
#include <QStringList>

extern "C" {
    #include "sockets.h"
    #include "entities.h"
    #include "obep.h"
    #include "obep_transport.h"
}

using namespace std;

static bool copierChamp(char* destination, size_t capacite, const string& valeur)
{
    if (valeur.empty() || valeur.size() >= capacite)
        return false;

    std::memcpy(destination, valeur.data(), valeur.size());
    return true;
}


static int envoyerAjoutObep(
    int socket,
    obep_rqst_type_t type,
    void* donnees,
    const char* commande)
{
    obep_rqst_t requete{};
    requete.rqst_type = type;
    requete.msg_type = obep_req;
    requete.rqst_rslt = obep_norslt;
    requete.argc = 1;
    requete.arg = donnees;

    obep_msg_t* message = obep_get_msg(&requete);

    if (message == nullptr)
        return -1;

    if (message->msg == nullptr)
    {
        std::free(message);
        return -1;
    }

    bool envoye = obep_send_frame(socket, message) == 0;

    std::free(message->msg);
    std::free(message);

    if (!envoye)
        return -1;

    obep_msg_t reponse{};

    if (obep_receive_frame(socket, &reponse) != 0)
    {
        std::free(reponse.msg);
        return -1;
    }

    int id = -1;

    bool succes =
        std::memcmp(reponse.header + 5, commande, 7) == 0 &&
        std::memcmp(reponse.header + 12, "|AN|SUCCESS#", 12) == 0;

    if (succes && reponse.msg != nullptr)
    {
        obep_rqst_t* resultat = obep_get_rqst(&reponse);

        if (resultat != nullptr)
        {
            if (resultat->rqst_type == type &&
                resultat->msg_type == obep_ans &&
                resultat->rqst_rslt == obep_success &&
                resultat->argc == 1 &&
                resultat->arg != nullptr)
            {
                id = *static_cast<int*>(resultat->arg);
            }

            std::free(resultat->arg);
            std::free(resultat);
        }
    }

    std::free(reponse.msg);

    return id > 0 ? id : -1;
}


MainWindowClientBookEncoder::MainWindowClientBookEncoder(string& IP, int PORT, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindowClientBookEncoder),
    ip{IP}, port{PORT}
{
    ui->setupUi(this);
    //this->setFixedSize(1068, 301);

    // Configuration de la table des employes (Personnel Garage)
    ui->tableWidgetEncodedBooks->setColumnCount(9);
    ui->tableWidgetEncodedBooks->setRowCount(0);
    QStringList labelsTableEmployes;
    labelsTableEmployes << "Id" << "Titre" << "Auteur" << "Sujet" << "ISBN" << "Pages" << "Année" << "Prix" << "Stock";
    ui->tableWidgetEncodedBooks->setHorizontalHeaderLabels(labelsTableEmployes);
    ui->tableWidgetEncodedBooks->horizontalHeader()->setVisible(true);
    ui->tableWidgetEncodedBooks->horizontalHeader()->setStretchLastSection(true);
    ui->tableWidgetEncodedBooks->verticalHeader()->setVisible(false);
    ui->tableWidgetEncodedBooks->horizontalHeader()->setStyleSheet("background-color: lightyellow");
    int columnWidths[] = {35, 250, 200, 200, 150, 50, 50, 50, 40};
    for (int col = 0; col < 9; ++col)
        ui->tableWidgetEncodedBooks->setColumnWidth(col, columnWidths[col]);

    this->logoutOk();
}

MainWindowClientBookEncoder::~MainWindowClientBookEncoder()
{
    if (socketServeur >= 0)
    {
        ::close(socketServeur);
        socketServeur = -1;
    }

    delete ui;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// Fonctions utiles Table des livres encodés (ne pas modifier) ////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindowClientBookEncoder::addTupleTableBooks(int id,
                                                     string title,
                                                     string author,
                                                     string subject,
                                                     string isbn,
                                                     int pageCount,
                                                     int publishYear,
                                                     float price,
                                                     int stockQuantity)
{
    int nb = ui->tableWidgetEncodedBooks->rowCount();
    nb++;
    ui->tableWidgetEncodedBooks->setRowCount(nb);
    ui->tableWidgetEncodedBooks->setRowHeight(nb-1,10);

    // id
    QTableWidgetItem *item = new QTableWidgetItem;
    item->setFlags(item->flags() & ~Qt::ItemIsSelectable);
    item->setTextAlignment(Qt::AlignCenter);
    item->setText(QString::number(id));
    ui->tableWidgetEncodedBooks->setItem(nb-1,0,item);

    // title
    item = new QTableWidgetItem;
    item->setFlags(item->flags() & ~Qt::ItemIsSelectable);
    item->setText(QString::fromStdString(title));
    ui->tableWidgetEncodedBooks->setItem(nb-1,1,item);

    // author
    item = new QTableWidgetItem;
    item->setFlags(item->flags() & ~Qt::ItemIsSelectable);
    item->setTextAlignment(Qt::AlignCenter);
    item->setText(QString::fromStdString(author));
    ui->tableWidgetEncodedBooks->setItem(nb-1,2,item);

    // subject
    item = new QTableWidgetItem;
    item->setFlags(item->flags() & ~Qt::ItemIsSelectable);
    item->setTextAlignment(Qt::AlignCenter);
    item->setText(QString::fromStdString(subject));
    ui->tableWidgetEncodedBooks->setItem(nb-1,3,item);

    // isbn
    item = new QTableWidgetItem;
    item->setFlags(item->flags() & ~Qt::ItemIsSelectable);
    item->setTextAlignment(Qt::AlignCenter);
    item->setText(QString::fromStdString(isbn));
    ui->tableWidgetEncodedBooks->setItem(nb-1,4,item);

    // pageCount
    item = new QTableWidgetItem;
    item->setFlags(item->flags() & ~Qt::ItemIsSelectable);
    item->setTextAlignment(Qt::AlignCenter);
    item->setText(QString::number(pageCount));
    ui->tableWidgetEncodedBooks->setItem(nb-1,5,item);

    // publishYear
    item = new QTableWidgetItem;
    item->setFlags(item->flags() & ~Qt::ItemIsSelectable);
    item->setTextAlignment(Qt::AlignCenter);
    item->setText(QString::number(publishYear));
    ui->tableWidgetEncodedBooks->setItem(nb-1,6,item);

    // price
    item = new QTableWidgetItem;
    item->setFlags(item->flags() & ~Qt::ItemIsSelectable);
    item->setTextAlignment(Qt::AlignCenter);
    item->setText(QString::number(price));
    ui->tableWidgetEncodedBooks->setItem(nb-1,7,item);

    // stockQuantity
    item = new QTableWidgetItem;
    item->setFlags(item->flags() & ~Qt::ItemIsSelectable);
    item->setTextAlignment(Qt::AlignCenter);
    item->setText(QString::number(stockQuantity));
    ui->tableWidgetEncodedBooks->setItem(nb-1,8,item);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindowClientBookEncoder::clearTableBooks() {
    ui->tableWidgetEncodedBooks->setRowCount(0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// Fonctions utiles des comboboxes (ne pas modifier) //////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindowClientBookEncoder::addComboBoxAuthors(string author){
    ui->comboBoxAuthors->addItem(QString::fromStdString(author));
}

string MainWindowClientBookEncoder::getSelectionAuthor() const {
    return ui->comboBoxAuthors->currentText().toStdString();
}

void MainWindowClientBookEncoder::clearComboBoxAuthors() {
    ui->comboBoxAuthors->clear();
}

void MainWindowClientBookEncoder::addComboBoxSubjects(string subject){
    ui->comboBoxSubjects->addItem(QString::fromStdString(subject));
}

string MainWindowClientBookEncoder::getSelectionSubject() const {
    return ui->comboBoxSubjects->currentText().toStdString();
}

void MainWindowClientBookEncoder::clearComboBoxSubjects() {
    ui->comboBoxSubjects->clear();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// Fonction utiles de la fenêtre (ne pas modifier) ////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
string MainWindowClientBookEncoder::getTitle() const {
    return ui->lineEditTitle->text().toStdString();
}

string MainWindowClientBookEncoder::getIsbn() const {
    return ui->lineEditIsbn->text().toStdString();
}

int MainWindowClientBookEncoder::getPageCount() const {
    return ui->spinBoxPageCount->value();
}

float MainWindowClientBookEncoder::getPrice() const {
    return ui->doubleSpinBoxPrice->value();
}

int MainWindowClientBookEncoder::getPublishYear() const {
    return ui->spinBoxPublishYear->value();
}

int MainWindowClientBookEncoder::getStockQuantity() const {
    return ui->spinBoxStockQuantity->value();
}

void MainWindowClientBookEncoder::loginOk() {
    ui->pushButtonClear->setEnabled(true);
    ui->pushButtonAddBook->setEnabled(true);
    ui->pushButtonAddAuthor->setEnabled(true);
    ui->pushButtonAddSubject->setEnabled(true);
    ui->actionLogin->setEnabled(false);
    ui->actionLogout->setEnabled(true);
}

void MainWindowClientBookEncoder::logoutOk() {
    ui->pushButtonClear->setEnabled(false);
    ui->pushButtonAddBook->setEnabled(false);
    ui->pushButtonAddAuthor->setEnabled(false);
    ui->pushButtonAddSubject->setEnabled(false);
    ui->actionLogin->setEnabled(true);
    ui->actionLogout->setEnabled(false);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// Fonctions permettant d'afficher des boites de dialogue (ne pas modifier) ///////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindowClientBookEncoder::dialogMessage(const string& title,const string& message) {
   QMessageBox::information(this,QString::fromStdString(title),QString::fromStdString(message));
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindowClientBookEncoder::dialogError(const string& title,const string& message) {
   QMessageBox::critical(this,QString::fromStdString(title),QString::fromStdString(message));
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
string MainWindowClientBookEncoder::dialogInputText(const string& title,const string& question) {
    return QInputDialog::getText(this,QString::fromStdString(title),QString::fromStdString(question)).toStdString();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
int MainWindowClientBookEncoder::dialogInputInt(const string& title,const string& question) {
    return QInputDialog::getInt(this,QString::fromStdString(title),QString::fromStdString(question));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// Fonctions gestion des boutons et items de menu (TO DO) /////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindowClientBookEncoder::on_pushButtonAddAuthor_clicked()
{
    author_t auteur{};

    string nom = this->dialogInputText("Nouvel auteur", "Nom ?");
    string prenom = this->dialogInputText("Nouvel auteur", "Prenom ?");
    string naissance = this->dialogInputText(
        "Nouvel auteur", "Date de naissance (yyyy-mm-dd) ?"
    );

    if (!copierChamp(auteur.last_name, sizeof(auteur.last_name), nom) ||
        !copierChamp(auteur.first_name, sizeof(auteur.first_name), prenom) ||
        !copierChamp(auteur.birth_date, sizeof(auteur.birth_date), naissance))
    {
        this->dialogError("Auteur", "Champs vides ou trop longs.");
        return;
    }

    int idAuteur = envoyerAjoutObep(
        socketServeur, obep_add_auth, &auteur, "ADDAUTH"
    );

    if (idAuteur > 0)   
    {
        this->dialogMessage("Auteur", "Auteur ajoute avec succes !");
        this->chargerListes();
    }
    else
    {
        this->dialogError("Auteur", "Ajout refuse ou erreur de communication.");
    }
}

void MainWindowClientBookEncoder::on_pushButtonAddSubject_clicked()
{
    subject_t sujet{};

    string nom = this->dialogInputText("Nouveau sujet", "Nom ?");

    if (!copierChamp(sujet.name, sizeof(sujet.name), nom))
    {
        this->dialogError("Sujet", "Nom vide ou trop long.");
        return;
    }

    int idSujet = envoyerAjoutObep(
        socketServeur, obep_add_subj, &sujet, "ADDSUBJ"
    );

    if (idSujet > 0)
    {
        this->dialogMessage("Sujet", "Sujet ajoute avec succes !");
        this->chargerListes();
    }
    else
    {
        this->dialogError("Sujet", "Ajout refuse ou erreur de communication.");
    }
}

void MainWindowClientBookEncoder::on_pushButtonAddBook_clicked()
{
    book_t livre{};

    livre.processed = 0;
    livre.auth.auth_id = ui->comboBoxAuthors->currentData().toInt();
    livre.subj.subj_id = ui->comboBoxSubjects->currentData().toInt();

    livre.page_count = this->getPageCount();
    livre.stock_quantity = this->getStockQuantity();
    livre.price = this->getPrice();
    livre.publish_year = this->getPublishYear();

    if (livre.auth.auth_id <= 0 || livre.subj.subj_id <= 0)
    {
        this->dialogError("Livre", "Selectionne un auteur et un sujet.");
        return;
    }

    if (!copierChamp(livre.title, sizeof(livre.title), this->getTitle()) ||
        !copierChamp(livre.isbn, sizeof(livre.isbn), this->getIsbn()))
    {
        this->dialogError("Livre", "Titre ou ISBN vide ou trop long.");
        return;
    }

    int idLivre = envoyerAjoutObep(
        socketServeur, obep_add_book, &livre, "ADDBOOK"
    );

    if (idLivre > 0)
    {
        this->addTupleTableBooks(
            -1,
            this->getTitle(),
            this->getSelectionAuthor(),
            this->getSelectionSubject(),
            this->getIsbn(),
            this->getPageCount(),
            this->getPublishYear(),
            this->getPrice(),
            this->getStockQuantity()
        );

        this->dialogMessage("Livre", "Livre ajoute avec succes !");
        this->on_pushButtonClear_clicked();
    }
    else
    {
        this->dialogError("Livre", "Ajout refuse ou erreur de communication.");
    }
}

void MainWindowClientBookEncoder::on_pushButtonClear_clicked() {
    ui->lineEditTitle->clear();
    ui->lineEditIsbn->clear();
    ui->spinBoxPageCount->setValue(0);
    ui->doubleSpinBoxPrice->setValue(0);
    ui->spinBoxPublishYear->setValue(0);
    ui->spinBoxStockQuantity->setValue(0);
}

void MainWindowClientBookEncoder::on_actionLogin_triggered()
{
    if (socketServeur >= 0)
    {
        this->dialogMessage("Connexion", "Une connexion TCP existe deja.");
        return;
    }

    bool confirme = false;

    QString login = QInputDialog::getText(
        this,
        "Entrée en session",
        "Login ?",
        QLineEdit::Normal,
        "",
        &confirme
    );

    if (!confirme || login.isEmpty())
        return;

    QString password = QInputDialog::getText(
        this,
        "Entrée en session",
        "Mot de passe ?",
        QLineEdit::Password,
        "",
        &confirme
    );

    if (!confirme || password.isEmpty())
        return;


        // Conversion des QString de Qt en chaînes UTF-8
    QByteArray loginBytes = login.toUtf8();
    QByteArray passwordBytes = password.toUtf8();

    // Création de l'employé attendu par OBEP
    employee_t employe = {0};

    // Vérification des tailles avant la copie
    if (loginBytes.size() >= static_cast<qsizetype>(sizeof(employe.login)) ||
        passwordBytes.size() >= static_cast<qsizetype>(sizeof(employe.pswd)))
    {
        this->dialogError("Login", "Identifiant ou mot de passe trop long.");
        return;
    }

    // Copie des identifiants dans la structure
    std::memcpy(employe.login, loginBytes.constData(), loginBytes.size());
    std::memcpy(employe.pswd, passwordBytes.constData(), passwordBytes.size());

    // Construction de la requête OBEP
    obep_rqst_t requete{};
    requete.rqst_type = obep_login;
    requete.msg_type = obep_req;
    requete.rqst_rslt = obep_norslt;
    requete.arg = &employe;

    socketServeur = cslib_clientSocket(ip.c_str(), port);

    if (socketServeur < 0)
    {
        this->dialogError("Connexion impossible",
                          cslib_socketerror_str(socketServeur));

        socketServeur = -1;
        return;
    }

    if (cslib_setTimeouts(socketServeur, 5) == -1)
    {
        this->dialogError("Connexion", "Impossible de configurer les timeouts.");

        ::close(socketServeur);
        socketServeur = -1;
        return;
    }

    // Transformer la requête LOGIN en message OBEP
    obep_msg_t* message = obep_get_msg(&requete);

    // Vérifier que la sérialisation a fonctionné
    if (message == nullptr || message->msg == nullptr)
    {
        this->dialogError("LOGIN", "Erreur de serialisation OBEP.");

        if (message != nullptr)
        {
            std::free(message->msg);
            std::free(message);
        }

        ::close(socketServeur);
        socketServeur = -1;
        return;
    }

    // Envoyer le message OBEP sur la connexion TCP
    int resultatEnvoi = obep_send_frame(socketServeur, message);

    // Le message sérialisé n'est plus nécessaire après l'envoi
    std::free(message->msg);
    std::free(message);

    // Vérifier le résultat de l'envoi
    if (resultatEnvoi == -1)
    {
        this->dialogError("LOGIN", "Erreur lors de l'envoi OBEP.");

        ::close(socketServeur);
        socketServeur = -1;
        return;
    }

    // Recevoir la réponse OBEP envoyée par le serveur
    obep_msg_t reponse{};

    if (obep_receive_frame(socketServeur, &reponse) == -1)
    {
        std::free(reponse.msg);

        ::close(socketServeur);
        socketServeur = -1;

        this->dialogError("LOGIN",
                        "Erreur reseau : reponse OBEP non recue.");
        return;
    }

    // Vérifier les deux réponses possibles de notre serveur simulé
    bool succes = std::memcmp(
        reponse.header,
        "0000|LOGIN  |AN|SUCCESS#",
        OBEP__HEADER_LENGTH
    ) == 0;

    bool refus = std::memcmp(
        reponse.header,
        "0000|LOGIN  |AN|FAILURE#",
        OBEP__HEADER_LENGTH
    ) == 0;

    // Libérer le contenu éventuellement alloué à la réception
    std::free(reponse.msg);

    // Traiter le résultat de l'authentification
    if (succes)
    {
        this->loginOk();

        if (this->chargerListes())
        {
            this->dialogMessage(
                "Login",
                "Authentification acceptee !"
            );
        }
        return;
    }

    // En cas de refus ou de réponse incorrecte,
    // la connexion n'est pas conservée.
    ::close(socketServeur);
    socketServeur = -1;

    if (refus)
    {
        this->dialogError("LOGIN",
                        "Identifiants refuses par le serveur.");
    }
    else
    {
        this->dialogError("LOGIN",
                        "Reponse OBEP inattendue ou incorrecte.");
    }
}

void MainWindowClientBookEncoder::on_actionLogout_triggered()
{
    if (socketServeur >= 0)
    {
        obep_rqst_t requete{};
        requete.rqst_type = obep_logout;
        requete.msg_type = obep_req;
        requete.rqst_rslt = obep_norslt;

        obep_msg_t* message = obep_get_msg(&requete);

        if (message != nullptr)
        {
            if (obep_send_frame(socketServeur, message) == 0)
            {
                obep_msg_t reponse{};

                if (obep_receive_frame(socketServeur, &reponse) == 0)
                {
                    if (std::memcmp(
                            reponse.header,
                            "0000|LOGOUT |AN|SUCCESS#",
                            OBEP__HEADER_LENGTH) != 0)
                    {
                        this->dialogError(
                            "Logout",
                            "Reponse LOGOUT inattendue ou refusee."
                        );
                    }
                }
                else
                {
                    this->dialogError(
                        "Logout",
                        "Aucune reponse OBEP recue."
                    );
                }

                std::free(reponse.msg);
            }
            else
            {
                this->dialogError(
                    "Logout",
                    "Impossible d'envoyer la requete LOGOUT."
                );
            }

            std::free(message->msg);
            std::free(message);
        }

        ::close(socketServeur);
        socketServeur = -1;
    }

    this->logoutOk();
}

void MainWindowClientBookEncoder::on_actionQuitter_triggered(){

    QApplication::exit(0);
}


bool MainWindowClientBookEncoder::chargerListes()
{
    this->clearComboBoxAuthors();
    this->clearComboBoxSubjects();

    const obep_rqst_type_t types[] = {
        obep_get_auth,
        obep_get_subj
    };

    const char* commandes[] = {
        "GETAUTH",
        "GETSUBJ"
    };

    for (int i = 0; i < 2; ++i)
    {
        // Préparer la requête OBEP
        obep_rqst_t requete{};
        requete.rqst_type = types[i];
        requete.msg_type = obep_req;
        requete.rqst_rslt = obep_norslt;

        obep_msg_t* message = obep_get_msg(&requete);

        if (message == nullptr)
            break;

        // Envoyer la requête
        bool ok = obep_send_frame(socketServeur, message) == 0;

        std::free(message->msg);
        std::free(message);

        if (!ok)
            break;

        // Recevoir la réponse
        obep_msg_t reponse{};

        if (obep_receive_frame(socketServeur, &reponse) != 0)
        {
            std::free(reponse.msg);
            break;
        }

        // Vérifier que la réponse correspond à la requête
        ok =
            std::memcmp(reponse.header + 5, commandes[i], 7) == 0 &&
            std::memcmp(reponse.header + 13, "AN", 2) == 0 &&
            std::memcmp(reponse.header + 16, "SUCCESS", 7) == 0;

        if (!ok)
        {
            std::free(reponse.msg);
            break;
        }

        // Une liste vide est possible
        if (reponse.msg != nullptr && reponse.msg[0] != '\0')
        {
            // Désérialisation par le module de Lionel
            obep_rqst_t* resultat = obep_get_rqst(&reponse);

            std::free(reponse.msg);

            if (resultat == nullptr)
                break;

            ok =
                resultat->rqst_type == types[i] &&
                resultat->msg_type == obep_ans &&
                resultat->rqst_rslt == obep_success &&
                resultat->argc >= 0 &&
                (resultat->argc == 0 || resultat->arg != nullptr);

            if (ok && i == 0)
            {
                // Liste des auteurs
                author_t* auteurs =
                    static_cast<author_t*>(resultat->arg);

                for (int j = 0; j < resultat->argc; ++j)
                {
                    QString nom =
                        QString::fromUtf8(auteurs[j].first_name) +
                        " " +
                        QString::fromUtf8(auteurs[j].last_name);

                    ui->comboBoxAuthors->addItem(nom, auteurs[j].id);
                }
            }
            else if (ok && i == 1)
            {
                // Liste des sujets
                subject_t* sujets =
                    static_cast<subject_t*>(resultat->arg);

                for (int j = 0; j < resultat->argc; ++j)
                {
                    ui->comboBoxSubjects->addItem(
                        QString::fromUtf8(sujets[j].name),
                        sujets[j].id
                    );
                }
            }

            std::free(resultat->arg);
            std::free(resultat);

            if (!ok)
                break;
        }
        else
        {
            std::free(reponse.msg);
        }
        if (i == 1)
            return true;
    }

        // Une des deux requêtes a échoué
    this->clearComboBoxAuthors();
    this->clearComboBoxSubjects();

    if (socketServeur >= 0)
    {
        ::close(socketServeur);
        socketServeur = -1;
    }

    this->logoutOk();

    this->dialogError(
        "OBEP",
        "Impossible de recuperer les auteurs ou les sujets."
    );

    return false;
}
