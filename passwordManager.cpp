#include "include/sqlite3.h"
#include <fstream>
#include <stdio.h>
#include <iostream>
#include <string.h>

sqlite3 *MDPdb;
sqlite3 *CheckDB;
bool newUser = true;
typedef struct nodelist
{
    char *textValue;
    nodelist *next;
} list;

void openMDPDB(char *filename, sqlite3 *database);
void createMDPTable(sqlite3 *database);
bool CheckIdentity(char *filename, sqlite3 *database);
bool databaseExists(char *filename);
void CreateCheckDB(char *filename, sqlite3 *database);
void HandleUserInputs();
void CreationMode();
void AccesMode();
bool IsInsideList(char *value, list *headNode);

int main(int argc, char *argv[])
{
    char *filename = (char *)"mdp.db";
    openMDPDB(filename, MDPdb);
    char *pFilename = (char *)malloc(sizeof(char) * 10);
    if (argc < 2)
    {
        std::cout << "Enter the filename of the database where your user mdp is or will be stored:";
        std::cin >> pFilename;
    }
    else
        pFilename = argv[1];
    if (!newUser)
    {
        if (CheckIdentity(pFilename, CheckDB))
            createMDPTable(MDPdb);
        else
        {
            std::cout << ("Wrong password exiting the program\n");
            return 1;
        }
    }
    else
        CreateCheckDB(pFilename, CheckDB);
    HandleUserInputs();
    return 0;
}

bool CheckIdentity(char *filename, sqlite3 *database)
{
    bool returnValue;
    int opened = sqlite3_open(filename, &database);
    if (opened)
    { // check if opening the database is successful
        std::cout << "Database could not be opened " << sqlite3_errmsg(database) << "\n";
    }
    else
    {
        std::cout << "opened database successfuly \n";
        CheckDB = database;
    }

    sqlite3_stmt *stmt = NULL;
    char *errorMsg;
    char *value = (char *)malloc(sizeof(char) * 10);
    char *userValue = (char *)malloc(sizeof(char) * 10);

    const char *query = "SELECT mdp FROM USER";
    if (sqlite3_prepare_v2(CheckDB, query, -1, &stmt, 0) == SQLITE_OK)
    {
        if (sqlite3_step(stmt) == SQLITE_ROW)
        {
            // Extract the first column of the first row
            value = (char *)sqlite3_column_text(stmt, 0);
        }

        std::cout << "Your password : ";
        std::cin >> userValue;
        std::cout << "\n";
        returnValue = strcmp(userValue, value) == 0;
        sqlite3_finalize(stmt);
    }
    free(errorMsg);
    free(value);
    free(userValue);
    return returnValue;
}

// open the database
void openMDPDB(char *filename, sqlite3 *database)
{
    newUser = !databaseExists(filename);
    int opened = sqlite3_open(filename, &database);
    if (opened)
    { // check if opening the database is successful
        std::cout << "Database could not be opened " << sqlite3_errmsg(database) << "\n";
    }
    else
    {
        std::cout << "opened database successfuly \n";
        MDPdb = database;
    }
}

void createMDPTable(sqlite3 *database)
{
    std::cout << "CREATE MDP TABLE\n";
    char *errorMsg;
    const char *sqlCreate = "CREATE TABLE IF NOT EXISTS MDP("
                            "AppSite TEXT NOT NULL,"
                            "username TEXT NOT NULL,"
                            "mdp TEXT NOT NULL)";

    int rc = sqlite3_exec(database, sqlCreate, NULL, 0, &errorMsg);
    if (rc != SQLITE_OK)
    {
        std::cout << "Error in executing SQL: " << errorMsg << "\n";
        // free the error message
        sqlite3_free(errorMsg);
    }
    else
    {
        std::cout << "table MDP made successfully \n";
    }
}

void HandleUserInputs()
{
    char *userMode = (char *)malloc(sizeof(char) * 5);
    std::cout << "You're now connected to your password database\n";
    std::cout << "Do you want to store a new password or see your already stored password ?\n ";
    while (strcmp(userMode, "n") != 0 && strcmp(userMode, "new") != 0 && strcmp(userMode, "s") != 0 && strcmp(userMode, "stored") != 0)
    {
        std::cout << "Enter 'n' or 'new' for a new password and 's' or 'stored' to access your passwords : ";
        std::cin >> userMode;
    }
    if (strcmp(userMode, "n") == 0 || strcmp(userMode, "new") == 0)
    {
        std::cout << "You've entered the creation mode !\n";
        CreationMode();
    }
    else
    {
        std::cout << "You've entered the access mode !\n";
        AccesMode();
    }

    free(userMode);
}

void CreationMode()
{
    char *continueToCreate = (char *)malloc(sizeof(char) * 5);
    while (strcmp(continueToCreate, "yes") != 0 && strcmp(continueToCreate, "y") != 0 && strcmp(continueToCreate, "Y"))
    {
        const char *creationQuery = "INSERT INTO MDP(AppSite, username, mdp) VALUES(?, ?, ?)";
        sqlite3_stmt *stmt = NULL;
        if (sqlite3_prepare_v2(MDPdb, creationQuery, -1, &stmt, 0) == SQLITE_OK)
        {
            char *appSite = (char *)malloc(sizeof(char) * 25);
            char *username = (char *)malloc(sizeof(char) * 25);
            char *mdp = (char *)malloc(sizeof(char) * 25);
            std::cout << "Enter the place where this password is used: ";
            std::cin >> appSite;
            std::cout << "Enter the username: ";
            std::cin >> username;
            std::cout << "Enter the password: ";
            std::cin >> mdp;
            sqlite3_bind_text(stmt, 1, appSite, -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 2, username, -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 3, mdp, -1, SQLITE_TRANSIENT);
            sqlite3_step(stmt);
            sqlite3_finalize(stmt);
            std::cout << "---------------------------------------------------------\n";
            std::cout << "You've just added your password: " << mdp << " at the username: " << username << " for: " << appSite << "\n";
            std::cout << "---------------------------------------------------------\n";
        }
        std::cout << "Do you want to quit the creation mode ? \n  If not enter anything else than 'y', 'Y' or 'yes': ";
        std::cin >> continueToCreate;
    }
    char *enterAccesMode = (char *)malloc(sizeof(char) * 2);
    std::cout << "Do you want to acces your passwords ? \nEnter 'y' , 'Y' or 'yes' if so:";
    std::cin >> enterAccesMode;
    if (strcmp(enterAccesMode, "y") == 0 || strcmp(enterAccesMode, "y") == 0 || strcmp(enterAccesMode, "yes") == 0)
        AccesMode();
    free(enterAccesMode);
    free(continueToCreate);
}
void AccesMode()
{
    char *continueToAcces = (char *)malloc(sizeof(char) * 5);
    while (strcmp(continueToAcces, "yes") != 0 && strcmp(continueToAcces, "y") != 0 && strcmp(continueToAcces, "Y"))
    {
        list *appSiteList = (list *)malloc(sizeof(list));
        list *newNode = appSiteList;

        appSiteList->next = NULL;
        appSiteList->textValue = NULL;
        std::cout << "Here's a list of all the places where you have stored a password:\n";
        const char *appQuery = "SELECT AppSite FROM MDP";
        sqlite3_stmt *stmt = NULL;
        if (sqlite3_prepare_v2(MDPdb, appQuery, -1, &stmt, 0) == SQLITE_OK)
        {
            while (sqlite3_step(stmt) == SQLITE_ROW)
            {
                while (newNode->next != NULL)
                {
                    newNode = newNode->next;
                }
                newNode->textValue = (char *)sqlite3_column_text(stmt, 0);
                std::cout << newNode->textValue << "\n";
            }
        }

        std::cout << "----------------------------------\n";
        const char *specificAppQuery = "SELECT * FROM MDP WHERE AppSite = ?";
        stmt = NULL;
        if (sqlite3_prepare_v2(MDPdb, specificAppQuery, -1, &stmt, 0) == SQLITE_OK)
        {
            std::cout << "here\n";
            char *userApp = (char *)malloc(sizeof(char) * 10);
            bool hasEntered = false;
            while (!IsInsideList(userApp, appSiteList) || !hasEntered)
            {
                // hasEntered = true;
                std::cout << "From which app you want to see your password and username ?:";
                std::cin >> userApp;
            }
            sqlite3_bind_text(stmt, 1, userApp, -1, SQLITE_TRANSIENT);
            while (sqlite3_step(stmt) == SQLITE_ROW)
            {
                std::cout << "---------------------------------\n";
                for (int i = 0; i < sqlite3_column_count(stmt); i++)
                {
                    std::cout << (char *)sqlite3_column_name(stmt, i) << ": " << (char *)sqlite3_column_text(stmt, i) << "\n";
                }
            }
            sqlite3_finalize(stmt);
            free(userApp);
        }
        std::cout << "---------------------------------\n";
        std::cout << "Do you want to quit the acces mode ? \n  If not enter anything else than 'y', 'Y' or 'yes': ";
        std::cin >> continueToAcces;
    }
    // Check if the user want to create new password
    char *enterCreationMode = (char *)malloc(sizeof(char) * 2);
    std::cout << "Do you want to create new passwords ? \nEnter 'y' , 'Y' or 'yes' if so:";
    std::cin >> enterCreationMode;
    if (strcmp(enterCreationMode, "y") == 0 || strcmp(enterCreationMode, "y") == 0 || strcmp(enterCreationMode, "yes") == 0)
        CreationMode();

    free(enterCreationMode);
    free(continueToAcces);
}
void CreateCheckDB(char *filename, sqlite3 *database)
{
    bool opened = sqlite3_open(filename, &database);
    if (opened)
    { // check if opening the database is successful
        std::cout << "Database could not be opened " << sqlite3_errmsg(database) << "\n";
    }
    else
    {
        std::cout << "opened database successfuly \n";
        CheckDB = database;
    }

    char *errorMsg;
    const char *sqlCreate = "CREATE TABLE IF NOT EXISTS USER("
                            "mdp TEXT NOT NULL)";

    int rc = sqlite3_exec(database, sqlCreate, NULL, 0, &errorMsg);
    if (rc != SQLITE_OK)
    {
        std::cout << "Error in executing SQL: " << errorMsg << "\n";
        // free the error message
        sqlite3_free(errorMsg);
    }
    else
    {
        std::cout << "table USER made successfully \n";
    }

    const char *mdpQuery = "INSERT INTO USER(mdp) VALUES(?)";
    sqlite3_stmt *stmt = NULL;
    if (sqlite3_prepare_v2(CheckDB, mdpQuery, -1, &stmt, 0) == SQLITE_OK)
    {
        char *userMDP = (char *)malloc(sizeof(char) * 10);
        std::cout << " Create your password here: ";
        std::cin >> userMDP;
        std::cout << "\n"
                  << userMDP << "\n";
        sqlite3_bind_text(stmt, 1, userMDP, -1, SQLITE_TRANSIENT);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }
    free(errorMsg);
}

bool databaseExists(char *filename)
{
    std::ifstream file(filename);
    return file.good();
}

bool IsInsideList(char *value, list *headNode)
{
    list *searchNode = headNode;
    std::cout << searchNode->next->textValue << "\n";
    while (strcmp(searchNode->textValue, value) != 0)
    {
        if (strcmp(searchNode->textValue, value) == 0)
        {
            std::cout << "here\n";
            return true;
        }
        std::cout << "before" << searchNode->textValue;
        searchNode = searchNode->next;
        std::cout << "after" << searchNode->textValue;
    }
    std::cout << "outside?\n";
    return false;
}