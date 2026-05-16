/*
 * =========================================================
 *       BANKPRO - Professional Bank Account Manager
 *         Random-Access File I/O  |  ANSI Terminal UI
 * =========================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

/* ---- ANSI colour codes ---- */
#define RESET    "\033[0m"
#define BOLD     "\033[1m"
#define DIM      "\033[2m"
#define RED      "\033[31m"
#define GREEN    "\033[32m"
#define YELLOW   "\033[33m"
#define CYAN     "\033[36m"
#define WHITE    "\033[97m"

/* ---- Constants ---- */
#define MAX_ACCOUNTS 100
#define DB_FILE      "credit.dat"
#define REPORT_FILE  "accounts.txt"
#define DIV_WIDTH    60

/* ---- Data structure ---- */
typedef struct {
    unsigned int acctNum;
    char         lastName[15];
    char         firstName[10];
    double       balance;
} ClientData;

/* ---- Prototypes ---- */
unsigned int enterChoice(void);
void printDivider(char ch, int width);
void printBanner(void);
void printHeader(void);
void printRecord(ClientData *c);
void printSuccess(const char *msg);
void printError(const char *msg);
void printInfo(const char *msg);
void clearInputBuffer(void);
int  validAccount(int acc);
void textFile(FILE *readPtr);
void updateRecord(FILE *fPtr);
void newRecord(FILE *fPtr);
void deleteRecord(FILE *fPtr);
void searchRecord(FILE *fPtr);
void showSummary(FILE *fPtr);

/* ========================================================= */
int main(void)
{
    FILE        *cfPtr;
    unsigned int choice;

    printBanner();

    cfPtr = fopen(DB_FILE, "rb+");
    if (cfPtr == NULL) {
        cfPtr = fopen(DB_FILE, "wb+");
        if (cfPtr == NULL) {
            printError("FATAL: Cannot open or create database file.");
            exit(EXIT_FAILURE);
        }
        ClientData blank = {0, "", "", 0.0};
        int i;
        for (i = 0; i < MAX_ACCOUNTS; i++)
            fwrite(&blank, sizeof(ClientData), 1, cfPtr);
        printInfo("New database created with 100 empty slots.");
    } else {
        printInfo("Database loaded successfully.");
    }

    while ((choice = enterChoice()) != 7) {
        switch (choice) {
            case 1: textFile(cfPtr);     break;
            case 2: updateRecord(cfPtr); break;
            case 3: newRecord(cfPtr);    break;
            case 4: deleteRecord(cfPtr); break;
            case 5: searchRecord(cfPtr); break;
            case 6: showSummary(cfPtr);  break;
            default:
                printError("Invalid choice. Please select 1-7.");
                break;
        }
    }

    fclose(cfPtr);
    printf("\n" CYAN "  Thank you for using BankPro. Goodbye!" RESET "\n\n");
    return 0;
}

/* ========================================================= */
void printDivider(char ch, int width)
{
    int i;
    printf("  " CYAN);
    for (i = 0; i < width; i++) putchar(ch);
    printf(RESET "\n");
}

void printBanner(void)
{
    printf("\n");
    printDivider('=', DIV_WIDTH);
    printf(CYAN "  |" BOLD WHITE
           "        BANKPRO  --  Account Management System        "
           RESET CYAN "|\n" RESET);
    printf(CYAN "  |" DIM
           "              Random-Access File Edition               "
           RESET CYAN "|\n" RESET);
    printDivider('=', DIV_WIDTH);
    printf("\n");
}

void printHeader(void)
{
    printf("\n  " BOLD CYAN "%-6s  %-15s  %-10s  %12s\n" RESET,
           "Acct#", "Last Name", "First Name", "Balance");
    printDivider('-', 50);
}

void printRecord(ClientData *c)
{
    const char *col = (c->balance < 0.0) ? RED : GREEN;
    printf("  %-6u  %-15s  %-10s  %s%12.2f" RESET "\n",
           c->acctNum, c->lastName, c->firstName, col, c->balance);
}

void printSuccess(const char *msg) { printf("\n  " GREEN "[OK]  %s" RESET "\n", msg); }
void printError  (const char *msg) { printf("\n  " RED   "[ERR] %s" RESET "\n", msg); }
void printInfo   (const char *msg) { printf("  "  CYAN   "[i]   %s" RESET "\n", msg); }

void clearInputBuffer(void) { int c; while ((c = getchar()) != '\n' && c != EOF); }
int  validAccount(int acc)  { return (acc >= 1 && acc <= MAX_ACCOUNTS); }

/* ========================================================= */
unsigned int enterChoice(void)
{
    unsigned int choice;
    printf("\n");
    printDivider('-', DIV_WIDTH);
    printf(BOLD "  MENU\n" RESET);
    printDivider('-', DIV_WIDTH);
    printf("  " YELLOW "1" RESET "  ->  Export formatted report  (accounts.txt)\n");
    printf("  " YELLOW "2" RESET "  ->  Update account balance\n");
    printf("  " YELLOW "3" RESET "  ->  Add new account\n");
    printf("  " YELLOW "4" RESET "  ->  Delete account\n");
    printf("  " YELLOW "5" RESET "  ->  Search account by number\n");
    printf("  " YELLOW "6" RESET "  ->  Show summary statistics\n");
    printf("  " YELLOW "7" RESET "  ->  Exit\n");
    printDivider('-', DIV_WIDTH);
    printf(BOLD "  Your choice: " RESET);

    if (scanf("%u", &choice) != 1) { clearInputBuffer(); return 0; }
    clearInputBuffer();
    return choice;
}

/* ========================================================= */
void textFile(FILE *readPtr)
{
    FILE       *writePtr;
    ClientData  client;
    int         result, count = 0;
    double      total = 0.0;
    time_t      now;
    char        timebuf[64];

    if ((writePtr = fopen(REPORT_FILE, "w")) == NULL) {
        printError("Could not open accounts.txt for writing.");
        return;
    }

    now = time(NULL);
    strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H:%M:%S", localtime(&now));

    fprintf(writePtr, "==========================================================\n");
    fprintf(writePtr, "         BankPro -- Account Statement Report\n");
    fprintf(writePtr, "  Generated : %s\n", timebuf);
    fprintf(writePtr, "==========================================================\n\n");
    fprintf(writePtr, "%-6s  %-15s  %-10s  %12s\n",
            "Acct#", "Last Name", "First Name", "Balance");
    fprintf(writePtr, "----------------------------------------------------------\n");

    rewind(readPtr);
    while (!feof(readPtr)) {
        result = fread(&client, sizeof(ClientData), 1, readPtr);
        if (result == 1 && client.acctNum != 0) {
            fprintf(writePtr, "%-6u  %-15s  %-10s  %12.2f\n",
                    client.acctNum, client.lastName,
                    client.firstName, client.balance);
            total += client.balance;
            count++;
        }
    }

    fprintf(writePtr, "----------------------------------------------------------\n");
    fprintf(writePtr, "Total Accounts : %d\n",       count);
    fprintf(writePtr, "Total Balance  : %12.2f\n",   total);
    fprintf(writePtr, "==========================================================\n");
    fclose(writePtr);

    {
        char msg[80];
        snprintf(msg, sizeof(msg),
                 "Report saved to '%s'  (%d records).", REPORT_FILE, count);
        printSuccess(msg);
    }
}

/* ========================================================= */
void updateRecord(FILE *fPtr)
{
    int        account;
    double     transaction;
    ClientData client = {0, "", "", 0.0};

    printf("\n  Enter account number to update (1-%d): ", MAX_ACCOUNTS);
    if (scanf("%d", &account) != 1) { clearInputBuffer(); printError("Invalid input."); return; }
    clearInputBuffer();
    if (!validAccount(account)) { printError("Account number out of range."); return; }

    fseek(fPtr, (long)(account - 1) * (long)sizeof(ClientData), SEEK_SET);
    fread(&client, sizeof(ClientData), 1, fPtr);

    if (client.acctNum == 0) {
        printError("Account does not exist. Use option 3 to create it.");
        return;
    }

    printf("\n  " BOLD "Current Record:\n" RESET);
    printHeader(); printRecord(&client);

    printf("\n  Enter amount (+charge / -payment): ");
    if (scanf("%lf", &transaction) != 1) { clearInputBuffer(); printError("Invalid amount."); return; }
    clearInputBuffer();

    client.balance += transaction;
    fseek(fPtr, (long)(account - 1) * (long)sizeof(ClientData), SEEK_SET);
    fwrite(&client, sizeof(ClientData), 1, fPtr);

    printf("\n  " BOLD "Updated Record:\n" RESET);
    printHeader(); printRecord(&client);
    printSuccess("Balance updated successfully.");
}

/* ========================================================= */
void newRecord(FILE *fPtr)
{
    ClientData client = {0, "", "", 0.0};
    int        accountNum;

    printf("\n  Enter new account number (1-%d): ", MAX_ACCOUNTS);
    if (scanf("%d", &accountNum) != 1) { clearInputBuffer(); printError("Invalid input."); return; }
    clearInputBuffer();
    if (!validAccount(accountNum)) { printError("Account number out of range."); return; }

    fseek(fPtr, (long)(accountNum - 1) * (long)sizeof(ClientData), SEEK_SET);
    fread(&client, sizeof(ClientData), 1, fPtr);

    if (client.acctNum != 0) {
        char msg[60];
        snprintf(msg, sizeof(msg), "Account #%u already exists.", client.acctNum);
        printError(msg);
        return;
    }

    printf("  Last name   : "); if (scanf("%14s", client.lastName)  != 1) { clearInputBuffer(); printError("Invalid input."); return; }
    printf("  First name  : "); if (scanf("%9s",  client.firstName) != 1) { clearInputBuffer(); printError("Invalid input."); return; }
    printf("  Balance     : "); if (scanf("%lf",  &client.balance)  != 1) { clearInputBuffer(); printError("Invalid balance."); return; }
    clearInputBuffer();

    client.acctNum = (unsigned int)accountNum;
    fseek(fPtr, (long)(accountNum - 1) * (long)sizeof(ClientData), SEEK_SET);
    fwrite(&client, sizeof(ClientData), 1, fPtr);

    printf("\n  " BOLD "New Record Created:\n" RESET);
    printHeader(); printRecord(&client);
    printSuccess("Account created successfully.");
}

/* ========================================================= */
void deleteRecord(FILE *fPtr)
{
    ClientData client;
    ClientData blank = {0, "", "", 0.0};
    int        accountNum;
    char       confirm;

    printf("\n  Enter account number to delete (1-%d): ", MAX_ACCOUNTS);
    if (scanf("%d", &accountNum) != 1) { clearInputBuffer(); printError("Invalid input."); return; }
    clearInputBuffer();
    if (!validAccount(accountNum)) { printError("Account number out of range."); return; }

    fseek(fPtr, (long)(accountNum - 1) * (long)sizeof(ClientData), SEEK_SET);
    fread(&client, sizeof(ClientData), 1, fPtr);

    if (client.acctNum == 0) { printError("Account does not exist."); return; }

    printf("\n  " BOLD "Record to be deleted:\n" RESET);
    printHeader(); printRecord(&client);

    printf("\n  " RED BOLD "Confirm deletion? (y/N): " RESET);
    if (scanf(" %c", &confirm) != 1) { clearInputBuffer(); return; }
    clearInputBuffer();

    if (tolower((unsigned char)confirm) != 'y') { printInfo("Deletion cancelled."); return; }

    fseek(fPtr, (long)(accountNum - 1) * (long)sizeof(ClientData), SEEK_SET);
    fwrite(&blank, sizeof(ClientData), 1, fPtr);
    printSuccess("Account deleted successfully.");
}

/* ========================================================= */
void searchRecord(FILE *fPtr)
{
    ClientData client = {0, "", "", 0.0};
    int        accountNum;

    printf("\n  Enter account number to search (1-%d): ", MAX_ACCOUNTS);
    if (scanf("%d", &accountNum) != 1) { clearInputBuffer(); printError("Invalid input."); return; }
    clearInputBuffer();
    if (!validAccount(accountNum)) { printError("Account number out of range."); return; }

    fseek(fPtr, (long)(accountNum - 1) * (long)sizeof(ClientData), SEEK_SET);
    fread(&client, sizeof(ClientData), 1, fPtr);

    if (client.acctNum == 0) { printError("Account not found."); return; }

    printf("\n  " BOLD "Account Details:\n" RESET);
    printHeader(); printRecord(&client);
}

/* ========================================================= */
void showSummary(FILE *fPtr)
{
    ClientData client;
    int    total = 0, positive = 0, negative = 0;
    double totalBal = 0.0, highest = -1e18, lowest = 1e18;
    char   highName[26] = "N/A";
    char   lowName[26]  = "N/A";

    rewind(fPtr);
    while (fread(&client, sizeof(ClientData), 1, fPtr) == 1) {
        if (client.acctNum == 0) continue;
        total++;
        totalBal += client.balance;
        if (client.balance >= 0.0) positive++; else negative++;
        if (client.balance > highest) {
            highest = client.balance;
            snprintf(highName, sizeof(highName), "%s %s", client.firstName, client.lastName);
        }
        if (client.balance < lowest) {
            lowest = client.balance;
            snprintf(lowName, sizeof(lowName), "%s %s", client.firstName, client.lastName);
        }
    }

    printf("\n");
    printDivider('=', DIV_WIDTH);
    printf(BOLD "  SUMMARY STATISTICS\n" RESET);
    printDivider('=', DIV_WIDTH);
    printf("  %-28s " WHITE  "%d"    RESET "\n", "Total Accounts:",      total);
    printf("  %-28s " GREEN  "%d"    RESET "\n", "Accounts in Credit:",  positive);
    printf("  %-28s " RED    "%d"    RESET "\n", "Accounts in Debit:",   negative);
    printf("  %-28s %s%.2f"          RESET "\n", "Total Balance:",
           (totalBal >= 0.0 ? GREEN : RED), totalBal);
    if (total > 0) {
        printf("  %-28s " YELLOW "%.2f  (%s)" RESET "\n", "Highest Balance:", highest, highName);
        printf("  %-28s " YELLOW "%.2f  (%s)" RESET "\n", "Lowest Balance:",  lowest,  lowName);
        printf("  %-28s " CYAN   "%.2f"        RESET "\n", "Average Balance:", totalBal / total);
    }
    printDivider('=', DIV_WIDTH);
}
