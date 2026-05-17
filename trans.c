
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_ACCOUNTS 100
#define LAST_NAME_MAX 14
#define FIRST_NAME_MAX 9
#define NEGATIVE_BALANCE_LIMIT (-1000.0)

struct clientData
{
    unsigned int acctNum; // account number
    char lastName[15];    // account last name (incl. null terminator)
    char firstName[10];   // account first name (incl. null terminator)
    double balance;       // account balance
};

static unsigned int enter_choice(void);
static void write_accounts_txt(FILE *read_ptr);
static void update_record(FILE *file_ptr);
static void create_record(FILE *file_ptr);
static void delete_record(FILE *file_ptr);
static void list_accounts(FILE *file_ptr);

static int is_valid_acct_num(unsigned int acct_num);
static int is_valid_balance(double balance);
static int read_account_at(FILE *file_ptr, unsigned int acct_num, struct clientData *out);
static int write_account_at(FILE *file_ptr, unsigned int acct_num, const struct clientData *in);
static void sort_by_acct_num(struct clientData *arr, size_t n);

int main(void)
{
    FILE *credit_file_ptr;
    unsigned int choice;

    credit_file_ptr = fopen("credit.dat", "rb+");
    if (credit_file_ptr == NULL)
    {
        puts("trans.c: credit.dat could not be opened.");
        return -1;
    }

    while ((choice = enter_choice()) != 6)
    {

        switch (choice)
        {
        case 1:
            write_accounts_txt(credit_file_ptr);
            break;
        case 2:
            update_record(credit_file_ptr);
            break;
        case 3:
            create_record(credit_file_ptr);
            break;
        case 4:
            delete_record(credit_file_ptr);
            break;
        case 5:
            list_accounts(credit_file_ptr);
            break;
        default:
            puts("Incorrect choice");
            break;
        }
    }

    fclose(credit_file_ptr);
    return 0;
}

static unsigned int enter_choice(void)
{
    unsigned int menu_choice;

    printf("\nEnter your choice\n"
           "1 - store a formatted text file of accounts called\n"
           "    \"accounts.txt\" for printing\n"
           "2 - update an account\n"
           "3 - add a new account\n"
           "4 - delete an account\n"
           "5 - list all accounts\n"
           "6 - end program\n? ");

    if (scanf("%u", &menu_choice) != 1)
        return 6;

    return menu_choice;
}

static int is_valid_acct_num(unsigned int acct_num)
{
    return acct_num >= 1 && acct_num <= MAX_ACCOUNTS;
}

static int is_valid_balance(double balance)
{
 
    return balance >= NEGATIVE_BALANCE_LIMIT;
}

static int read_account_at(FILE *file_ptr, unsigned int acct_num, struct clientData *out)
{
    long offset;

    if (!is_valid_acct_num(acct_num))
        return 0;

    offset = (long)(acct_num - 1) * (long)sizeof(struct clientData);
    if (fseek(file_ptr, offset, SEEK_SET) != 0)
        return 0;

    return fread(out, sizeof(struct clientData), 1, file_ptr) == 1;
}

static int write_account_at(FILE *file_ptr, unsigned int acct_num, const struct clientData *in)
{
    long offset;

    if (!is_valid_acct_num(acct_num))
        return 0;

    offset = (long)(acct_num - 1) * (long)sizeof(struct clientData);
    if (fseek(file_ptr, offset, SEEK_SET) != 0)
        return 0;

    return fwrite(in, sizeof(struct clientData), 1, file_ptr) == 1;
}

static void write_accounts_txt(FILE *read_ptr)
{
    FILE *write_ptr;
    struct clientData client;
    int ok;

    write_ptr = fopen("accounts.txt", "w");
    if (write_ptr == NULL)
    {
        puts("File could not be opened for writing: accounts.txt");
        return;
    }

    rewind(read_ptr);
    fprintf(write_ptr, "%-6s%-16s%-11s%10s\n", "Acct", "Last Name", "First Name", "Balance");

    while ((ok = (fread(&client, sizeof(struct clientData), 1, read_ptr) == 1)))
    {
        if (client.acctNum != 0)
        {
            fprintf(write_ptr, "%-6u%-16s%-11s%10.2f\n",
                    client.acctNum, client.lastName, client.firstName, client.balance);
        }
    }

    fclose(write_ptr);
    puts("accounts.txt generated successfully.");
}

static void update_record(FILE *file_ptr)
{
    unsigned int account;
    double transaction;
    struct clientData client;

    printf("%s", "Enter account to update ( 1 - 100 ): ");
    if (scanf("%u", &account) != 1 || !is_valid_acct_num(account))
    {
        puts("Invalid account number.");
        return;
    }

    if (!read_account_at(file_ptr, account, &client))
    {
        puts("Unable to read account.");
        return;
    }

    if (client.acctNum == 0)
    {
        printf("Account #%u has no information.\n", account);
        return;
    }

    printf("%-6u%-16s%-11s%10.2f\n\n",
           client.acctNum, client.lastName, client.firstName, client.balance);

    printf("%s", "Enter charge ( + ) or payment ( - ): ");
    if (scanf("%lf", &transaction) != 1)
    {
        puts("Invalid transaction amount.");
        return;
    }

    client.balance += transaction;
    if (!is_valid_balance(client.balance))
    {
        puts("Transaction rejected: balance would be below allowed minimum.");
        return;
    }

    printf("%-6u%-16s%-11s%10.2f\n",
           client.acctNum, client.lastName, client.firstName, client.balance);

    if (!write_account_at(file_ptr, account, &client))
        puts("Error: could not write updated record.");
}

static void delete_record(FILE *file_ptr)
{
    unsigned int account_num;
    struct clientData client;
    struct clientData blank_client;

    blank_client.acctNum = 0;
    blank_client.lastName[0] = '\0';
    blank_client.firstName[0] = '\0';
    blank_client.balance = 0.0;

    printf("%s", "Enter account number to delete ( 1 - 100 ): ");
    if (scanf("%u", &account_num) != 1 || !is_valid_acct_num(account_num))
    {
        puts("Invalid account number.");
        return;
    }

    if (!read_account_at(file_ptr, account_num, &client))
    {
        puts("Unable to read account.");
        return;
    }

    if (client.acctNum == 0)
    {
        printf("Account %u does not exist.\n", account_num);
        return;
    }

    if (!write_account_at(file_ptr, account_num, &blank_client))
        puts("Error: could not delete record.");
    else
        printf("Account %u deleted.\n", account_num);
}

static void create_record(FILE *file_ptr)
{
    unsigned int account_num;
    struct clientData client;

    memset(&client, 0, sizeof(client));
    client.balance = 0.0;

    printf("%s", "Enter new account number ( 1 - 100 ): ");
    if (scanf("%u", &account_num) != 1 || !is_valid_acct_num(account_num))
    {
        puts("Invalid account number.");
        return;
    }

    if (!read_account_at(file_ptr, account_num, &client))
    {
        puts("Unable to read account slot.");
        return;
    }

    if (client.acctNum != 0)
    {
        printf("Account #%u already contains information.\n", client.acctNum);
        return;
    }

    printf("%s", "Enter lastname, firstname, balance\n? ");
    if (scanf("%14s%9s%lf", client.lastName, client.firstName, &client.balance) != 3)
    {
        puts("Invalid input.");
        return;
    }

    if (!is_valid_balance(client.balance))
    {
        puts("Invalid initial balance.");
        return;
    }

    client.acctNum = account_num;

    if (!write_account_at(file_ptr, account_num, &client))
        puts("Error: could not create record.");
    else
        printf("Account %u created.\n", account_num);
}

static void sort_by_acct_num(struct clientData *arr, size_t n)
{
    // Simple selection sort (n <= 100)
    for (size_t i = 0; i < n; ++i)
    {
        size_t min_idx = i;
        for (size_t j = i + 1; j < n; ++j)
        {
            if (arr[j].acctNum < arr[min_idx].acctNum)
                min_idx = j;
        }
        if (min_idx != i)
        {
            struct clientData tmp = arr[i];
            arr[i] = arr[min_idx];
            arr[min_idx] = tmp;
        }
    }
}

static void list_accounts(FILE *file_ptr)
{
    struct clientData all[MAX_ACCOUNTS];
    size_t count = 0;
    struct clientData client;

    rewind(file_ptr);

    while (count < MAX_ACCOUNTS && fread(&client, sizeof(struct clientData), 1, file_ptr) == 1)
    {
        if (client.acctNum != 0)
            all[count++] = client;
    }

    sort_by_acct_num(all, count);

    printf("\n%-6s%-16s%-11s%10s\n", "Acct", "Last Name", "First Name", "Balance");
    for (size_t i = 0; i < count; ++i)
    {
        printf("%-6u%-16s%-11s%10.2f\n",
               all[i].acctNum, all[i].lastName, all[i].firstName, all[i].balance);
    }

    if (count == 0)
        puts("No accounts to display.");
}

