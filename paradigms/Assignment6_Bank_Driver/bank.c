#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>

#include "error.h"
#include "bank.h"
#include "branch.h"
#include "account.h"
#include "report.h"

/*
 * allocate the bank structure and initialize the branches.
 */
Bank*
Bank_Init(int numBranches, int numAccounts, AccountAmount initalAmount,
          AccountAmount reportingAmount,
          int numWorkers)
{
  Bank *bank = malloc(sizeof(*bank));
  if (bank == NULL) {
    return NULL;
  }

  bank->remainingWorkers = numWorkers;

  if (sem_init(&bank->check, 0, 1) < 0 ||
      sem_init(&bank->nextDay, 0, 0) < 0 ||
      sem_init(&bank->lock, 0, 1) < 0) {
    free(bank);
    return NULL;
  }

  Branch_Init(bank, numBranches, numAccounts, initalAmount);
  Report_Init(bank, reportingAmount, numWorkers);

  return bank;
}

void
Bank_Dispose(Bank *bank)
{
  if (bank == NULL) {
    return;
  }

  sem_destroy(&bank->check);
  sem_destroy(&bank->nextDay);
  sem_destroy(&bank->lock);

  for (int i = 0; i < bank->numberBranches; i++) {
    Branch *branch = &bank->branches[i];
    sem_destroy(&branch->lock);

    for (int j = 0; j < branch->numberAccounts; j++) {
      sem_destroy(&branch->accounts[j].lock);
    }

    free(branch->accounts);
  }

  free(bank->report);
  free(bank->branches);
  free(bank);
}

/*
 * get the balance of the entire bank by adding up all the balances in
 * each branch.
 */
int
Bank_Balance(Bank *bank, AccountAmount *balance)
{
  assert(bank->branches);

  for (int i = 0; i < bank->numberBranches; i++) {
    sem_wait(&bank->branches[i].lock);
  }

  AccountAmount sum = 0;
  int status = 0;

  for (unsigned int i = 0; i < bank->numberBranches; i++) {
    AccountAmount branchBalance = 0;
    status = Branch_Balance(bank, bank->branches[i].branchID,
                            &branchBalance);
    if (status < 0) {
      break;
    }
    sum += branchBalance;
  }

  if (status == 0) {
    *balance = sum;
  }

  for (int i = 0; i < bank->numberBranches; i++) {
    sem_post(&bank->branches[i].lock);
  }

  return status;
}

/*
 * tranverse and validate each branch.
 */
int
Bank_Validate(Bank *bank)
{
  assert(bank->branches);

  for (unsigned int i = 0; i < bank->numberBranches; i++) {
    int rc = Branch_Validate(bank, bank->branches[i].branchID);
    if (rc < 0) {
      return rc;
    }
  }

  return 0;
}

/*
 * compare the data inside two banks and see they are exactly the same;
 * it is called in BankTest.
 */
int
Bank_Compare(Bank *bank1, Bank *bank2)
{
  if (bank1->numberBranches != bank2->numberBranches) {
    fprintf(stderr, "Bank num branches mismatch\n");
    return -1;
  }

  for (unsigned int i = 0; i < bank1->numberBranches; i++) {
    if (Branch_Compare(&bank1->branches[i],
                       &bank2->branches[i]) < 0) {
      return -1;
    }
  }

  return Report_Compare(bank1, bank2);
}
