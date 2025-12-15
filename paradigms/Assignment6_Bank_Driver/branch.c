#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <inttypes.h>

#include "teller.h"
#include "account.h"
#include "error.h"
#include "debug.h"

#include "branch.h"

/*
 * allocate and initialize each branch.
 */
int
Branch_Init(Bank *bank, int numBranches, int numAccounts,
            AccountAmount initialAmount)
{
  bank->numberBranches = numBranches;
  bank->branches = malloc(sizeof(Branch) * numBranches);
  if (bank->branches == NULL) {
    return -1;
  }

  int accountsPerBranch = numAccounts / numBranches;

  for (int i = 0; i < numBranches; i++) {
    Branch *branch = bank->branches + i;
    sem_init(&branch->lock, 0, 1);
    pthread_mutex_init(&branch->pLock, NULL);

    branch->branchID = i;
    branch->numberAccounts = accountsPerBranch;
    branch->balance = 0;

    branch->accounts = malloc(sizeof(Account) * accountsPerBranch);
    if (branch->accounts == NULL) {
      return -1;
    }

    for (int j = 0; j < accountsPerBranch; j++) {
      Account *acct = &branch->accounts[j];
      Account_Init(bank, acct, j, i, initialAmount);
      branch->balance += acct->balance;
    }
  }

  return 0;
}

/*
 * update the balance of a branch.
 */
int
Branch_UpdateBalance(Bank *bank, BranchID branchID, AccountAmount change)
{
  assert(bank->branches); Y;

  if (branchID >= bank->numberBranches) {
    return -1;
  }

  Branch *branch = &bank->branches[branchID];
  pthread_mutex_lock(&branch->pLock);
  AccountAmount current = branch->balance; Y;
  branch->balance = current + change; Y;

  pthread_mutex_unlock(&branch->pLock);
  return 0;
}

/*
 * get the balance of the branch
 */
int
Branch_Balance(Bank *bank, BranchID branchID, AccountAmount *balance)
{
  assert(bank->branches);

  if (branchID >= bank->numberBranches) {
    return -1;
  }

  Branch *branch = &bank->branches[branchID];

  pthread_mutex_lock(&branch->pLock);

  *balance = branch->balance; Y;

  /* It should be the case that the balance of a branch matches the sum 
   * of all the accounts in the branch.  The following routine validates 
   * this assumption but is far too expense to run in normal operation. 
   */
  /* assert(Branch_Validate(bank, branchID) == 0);  */
  pthread_mutex_unlock(&branch->pLock);
  return 0;
}

/*
 * validate the branch by checking its branchID and making sure that
 * its balance equals the sum of balances of all accounts inside
 * the branch.
 */
int
Branch_Validate(Bank *bank, BranchID branchID)
{
  assert(bank->branches);

  if (branchID >= bank->numberBranches) {
    return -1;
  }

  Branch *branch = &bank->branches[branchID];
  AccountAmount computed = 0;

  for (int i = 0; i < branch->numberAccounts; i++) {
    computed += branch->accounts[i].balance;
  }

  if (computed != branch->balance) {
    fprintf(stderr, "Branch balance mismatch. "
            "Computer value is %"PRId64", but stored value is %"PRId64"\n",
            computed, branch->balance);
    return -1;
  }

  return 0;
}

/*
 * Compare all data inside two branches to see if they are exactly the same.
 */
int
Branch_Compare(Branch *branch1, Branch *branch2)
{
  int err = 0;

  BranchID id1 = branch1->branchID;
  BranchID id2 = branch2->branchID;

  if (branch1->numberAccounts != branch2->numberAccounts) {
    fprintf(stderr, "Branches %"PRIu64" and %"PRIu64" mismatch in numberAccounts "
            "(%d and %d, respectively).\n",
            id1, id2,
            branch1->numberAccounts,
            branch2->numberAccounts);
    err = -1;
  }

  if (branch1->balance != branch2->balance) {
    fprintf(stderr, "Branches %"PRIu64" and %"PRIu64" mismatch in balance "
            "(%"PRId64" and %"PRId64", respectively).\n",
            id1, id2,
            branch1->balance,
            branch2->balance);
    err = -1;
  }

  int count = branch1->numberAccounts;
  for (int i = 0; i < count; i++) {
    Account *a1 = &branch1->accounts[i];
    Account *a2 = &branch2->accounts[i];

    assert(a1->accountNumber == a2->accountNumber);

    if (a1->balance != a2->balance) {
      fprintf(stderr,
              "Branch %"PRIu64" and %"PRIu64" mismatch in account 0x%"PRIx64" balance "
              "(%"PRId64" and %"PRId64", respectively).\n",
              id1, id2,
              a1->accountNumber,
              a1->balance,
              a2->balance);
      err = -1;
    }
  }

  return err;
}
