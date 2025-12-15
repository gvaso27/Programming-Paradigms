#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>

#include "teller.h"
#include "account.h"
#include "error.h"
#include "debug.h"

#include "branch.h"
#include "report.h"

/*
 * initialize the account based on the passed-in information.
 */
void
Account_Init(Bank *bank, Account *account, int id, int branch,
             AccountAmount initialAmount)
{
  extern int testfailurecode;

  account->accountNumber = Account_MakeAccountNum(branch, id);
  account->balance = initialAmount;
  sem_init(&account->lock, 0, 1);

  if (testfailurecode && ((id & 0x3) == 0)) {
    // To test failures, we initialize every 4th account with a negative value
    account->balance = -1;
  }
}

/*
 * get the ID of the branch which the account is in.
 */
BranchID
GetBranchID(AccountNumber accountNum)
{
  Y;
  BranchID id = (BranchID)(accountNum >> 32);
  return id;
}

/*
 * get the branch-wide subaccount number of the account.
 */
int
AcountNum_Subaccount(AccountNumber accountNum)
{
  Y;
  return (int)(accountNum & 0x7ffffff);
}

/*
 * find the account address based on the accountNum.
 */
Account *
Account_LookupByNumber(Bank *bank, AccountNumber accountNum)
{
  BranchID bid = GetBranchID(accountNum);
  int index = AcountNum_Subaccount(accountNum);
  Branch *branch = &bank->branches[bid];
  return &branch->accounts[index];
}

/*
 * adjust the balance of the account.
 */
void
Account_Adjust(Bank *bank, Account *account, AccountAmount amount,
               int updateBranch)
{
  AccountAmount current = Account_Balance(account);
  account->balance = current + amount;

  if (updateBranch) {
    Branch_UpdateBalance(bank,
                         GetBranchID(account->accountNumber),
                         amount);
  }
  Y;
}

/*
 * return the balance of the account.
 */
AccountAmount
Account_Balance(Account *account)
{
  AccountAmount value = account->balance; Y;
  return value;
}

/*
 * make the account number based on the branch number and
 * the branch-wise subaccount number.
 */
AccountNumber
Account_MakeAccountNum(int branch, int subaccount)
{
  AccountNumber num = (AccountNumber)subaccount;
  num |= ((uint64_t)branch << 32); Y;
  return num;
}

/*
 * Test to see if two accounts are in the same branch.
 */
int
Account_IsSameBranch(AccountNumber accountNum1, AccountNumber accountNum2)
{
  BranchID b1 = GetBranchID(accountNum1);
  BranchID b2 = GetBranchID(accountNum2);
  return b1 == b2;
}
