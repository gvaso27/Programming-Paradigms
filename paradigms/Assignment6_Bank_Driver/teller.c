#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <inttypes.h>

#include "teller.h"
#include "branch.h"
#include "account.h"
#include "error.h"
#include "debug.h"

/*
 * deposit money into an account
 */
int
Teller_DoDeposit(Bank *bank, AccountNumber accountNum, AccountAmount amount)
{
  assert(amount >= 0);

  DPRINTF('t', ("Teller_DoDeposit(account 0x%"PRIx64" amount %"PRId64")\n",
                accountNum, amount));

  Account *account = Account_LookupByNumber(bank, accountNum);
  if (account == NULL) {
    return ERROR_ACCOUNT_NOT_FOUND;
  }

  BranchID branchID = GetBranchID(accountNum);

  sem_wait(&bank->branches[branchID].lock);
  sem_wait(&account->lock);

  Account_Adjust(bank, account, amount, 1);

  sem_post(&account->lock);
  sem_post(&bank->branches[branchID].lock);

  return ERROR_SUCCESS;
}

/*
 * withdraw money from an account
 */
int
Teller_DoWithdraw(Bank *bank, AccountNumber accountNum, AccountAmount amount)
{
  assert(amount >= 0);

  DPRINTF('t', ("Teller_DoWithdraw(account 0x%"PRIx64" amount %"PRId64")\n",
                accountNum, amount));

  Account *account = Account_LookupByNumber(bank, accountNum);
  if (account == NULL) {
    return ERROR_ACCOUNT_NOT_FOUND;
  }

  BranchID branchID = GetBranchID(accountNum);

  sem_wait(&bank->branches[branchID].lock);
  sem_wait(&account->lock);

  if (amount > Account_Balance(account)) {
    sem_post(&account->lock);
    sem_post(&bank->branches[branchID].lock);
    return ERROR_INSUFFICIENT_FUNDS;
  }

  Account_Adjust(bank, account, -amount, 1);

  sem_post(&account->lock);
  sem_post(&bank->branches[branchID].lock);

  return ERROR_SUCCESS;
}

/*
 * do a tranfer from one account to another account
 */
int
Teller_DoTransfer(Bank *bank, AccountNumber srcAccountNum,
                  AccountNumber dstAccountNum,
                  AccountAmount amount)
{
  assert(amount >= 0);

  DPRINTF('t', ("Teller_DoTransfer(src 0x%"PRIx64", dst 0x%"PRIx64
                ", amount %"PRId64")\n",
                srcAccountNum, dstAccountNum, amount));

  if (srcAccountNum == dstAccountNum) {
    return ERROR_SUCCESS;
  }

  Account *srcAccount = Account_LookupByNumber(bank, srcAccountNum);
  Account *dstAccount = Account_LookupByNumber(bank, dstAccountNum);

  if (srcAccount == NULL || dstAccount == NULL) {
    return ERROR_ACCOUNT_NOT_FOUND;
  }

  BranchID srcBranchID = GetBranchID(srcAccountNum);
  BranchID dstBranchID = GetBranchID(dstAccountNum);

  int updateBranch = !Account_IsSameBranch(srcAccountNum, dstAccountNum);

  if (updateBranch) {
    BranchID first = srcBranchID < dstBranchID ? srcBranchID : dstBranchID;
    BranchID second = srcBranchID < dstBranchID ? dstBranchID : srcBranchID;

    sem_wait(&bank->branches[first].lock);
    sem_wait(&bank->branches[second].lock);
  }

  Account *firstAcc = srcAccount->accountNumber < dstAccount->accountNumber
                      ? srcAccount : dstAccount;
  Account *secondAcc = (firstAcc == srcAccount) ? dstAccount : srcAccount;

  sem_wait(&firstAcc->lock);
  sem_wait(&secondAcc->lock);

  if (amount > Account_Balance(srcAccount)) {
    sem_post(&secondAcc->lock);
    sem_post(&firstAcc->lock);

    if (updateBranch) {
      sem_post(&bank->branches[srcBranchID].lock);
      sem_post(&bank->branches[dstBranchID].lock);
    }

    return ERROR_INSUFFICIENT_FUNDS;
  }

  Account_Adjust(bank, srcAccount, -amount, updateBranch);
  Account_Adjust(bank, dstAccount, amount, updateBranch);

  sem_post(&secondAcc->lock);
  sem_post(&firstAcc->lock);

  if (updateBranch) {
    sem_post(&bank->branches[srcBranchID].lock);
    sem_post(&bank->branches[dstBranchID].lock);
  }

  return ERROR_SUCCESS;
}
