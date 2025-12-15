#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <inttypes.h>

#include "error.h"
#include "debug.h"

#include "bank.h"
#include "branch.h"
#include "account.h"
#include "report.h"

#define MAX_NUM_REPORTS 8
#define MAX_LOG_ENTRIES 1024

typedef struct Report {
  int numReports;
  struct {
    AccountAmount balance;
    int hasOverflowed;
    int numLogEntries;
    struct TransferLog {
      AccountNumber accountNum;
      AccountAmount transferSize;
    } transferLog[MAX_LOG_ENTRIES];
  } dailyData[MAX_NUM_REPORTS];
} Report;

static AccountAmount reportingAmount;
static int numWorkers;

/*
 * Initialize the Report module of a bank.  Returns -1 on an error, 0 otherwise.
 */
int
Report_Init(Bank *bank, AccountAmount reportAmount, int maxNumWorkers)
{
  bank->report = malloc(sizeof(Report));
  if (bank->report == NULL) {
    return -1;
  }

  Report *rpt = bank->report;
  rpt->numReports = 0;

  for (int i = 0; i < MAX_NUM_REPORTS; i++) {
    rpt->dailyData[i].hasOverflowed = 0;
    rpt->dailyData[i].numLogEntries = 0;
  }

  reportingAmount = reportAmount;
  numWorkers = maxNumWorkers;

  return 0;
}

/*
 * Report a transfer to/from the specified accountNum in the amount of amount.
 * This is called for all tranfers but we need record only the ones that
 * are at or above the reporting amount. The worker making the call is
 * given to us (workerNum). Returns 0 on success, non-zero otherwise.
 */
int
Report_Transfer(Bank *bank, int workerNum, AccountNumber accountNum,
                AccountAmount amount)
{
  sem_wait(&bank->lock);

  AccountAmount absAmount = (amount < 0) ? -amount : amount; Y;
  if (absAmount < reportingAmount) {
    sem_post(&bank->lock);
    return 0;
  }

  Report *rpt = bank->report;
  int reportIndex = rpt->numReports; Y;

  if (reportIndex >= MAX_NUM_REPORTS) {
    sem_post(&bank->lock);
    return 0;
  }

  int *entryCount = &rpt->dailyData[reportIndex].numLogEntries;
  if (*entryCount >= MAX_LOG_ENTRIES) {
    rpt->dailyData[reportIndex].hasOverflowed = 1;
    sem_post(&bank->lock);
    return 0;
  }

  int pos = *entryCount; Y;
  rpt->dailyData[reportIndex].transferLog[pos].accountNum = accountNum; Y;
  rpt->dailyData[reportIndex].transferLog[pos].transferSize = amount; Y;
  *entryCount = pos + 1; Y;

  sem_post(&bank->lock);
  return 0;
}

/*
 * Perform the nightly report. Is called by every worker for each report period. workerNum is
 * the worker making the call.  Returns -1 on error, 0 otherwise.
 */
int
Report_DoReport(Bank *bank, int workerNum)
{
  sem_wait(&bank->check);
  bank->remainingWorkers--;

  if (bank->remainingWorkers != 0) {
    sem_post(&bank->check);
    sem_wait(&bank->nextDay);
    return 0;
  }

  Report *rpt = bank->report;
  assert(rpt); Y;

  if (rpt->numReports >= MAX_NUM_REPORTS) {
    bank->remainingWorkers = numWorkers;
    sem_post(&bank->check);
    return -1;
  }

  int idx = rpt->numReports;
  int rc = Bank_Balance(bank, &rpt->dailyData[idx].balance); Y;
  rpt->numReports = idx + 1; Y;

  for (int i = 0; i < numWorkers - 1; i++) {
    sem_post(&bank->nextDay);
  }

  bank->remainingWorkers = numWorkers;
  sem_post(&bank->check);
  return rc;
}

/*
 *
 * Function used by qsort() to record log.
 */
static int
TransferLogSortFunc(const void *p1, const void *p2)
{
  const struct TransferLog *a = p1;
  const struct TransferLog *b = p2;

  if (a->accountNum != b->accountNum) {
    return (a->accountNum < b->accountNum) ? -1 : 1;
  }

  if (a->transferSize != b->transferSize) {
    return (a->transferSize < b->transferSize) ? -1 : 1;
  }

  return 0;
}

/*
 * Compare the report data inside two banks and see they are exactly the same;
 * Prints mismatches to stderr,
 * Return -1 on mismatch, zero otherwise.
 */
int
Report_Compare(Bank *bank1, Bank *bank2)
{
  int err = 0;

  Report *rpt1 = bank1->report;
  Report *rpt2 = bank2->report;

  if (rpt1->numReports != rpt2->numReports) {
    fprintf(stderr, "Bank num reports mismatch %d != %d\n",
            rpt1->numReports, rpt2->numReports);
    err = -1;
  }

  for (int r = 0; r < rpt1->numReports; r++) {
    if (rpt1->dailyData[r].balance != rpt2->dailyData[r].balance) {
      fprintf(stderr, "Report %d for banks mismatch %"PRId64" and %"PRId64"\n",
              r, rpt1->dailyData[r].balance,
              rpt2->dailyData[r].balance);
      err = -1;
    }

    int n1 = rpt1->dailyData[r].numLogEntries;
    int n2 = rpt2->dailyData[r].numLogEntries;
    if (n1 != n2) {
      fprintf(stderr, "Report different number of log entries (%d and %d)\n",
              n1, n2);
      return -1;
    }

    if (!rpt1->dailyData[r].hasOverflowed) {
      qsort(rpt1->dailyData[r].transferLog, n1,
            sizeof(struct TransferLog), TransferLogSortFunc);
      qsort(rpt2->dailyData[r].transferLog, n2,
            sizeof(struct TransferLog), TransferLogSortFunc);

      for (int i = 0; i < n1; i++) {
        struct TransferLog *l1 = &rpt1->dailyData[r].transferLog[i];
        struct TransferLog *l2 = &rpt2->dailyData[r].transferLog[i];

        if (l1->accountNum != l2->accountNum ||
            l1->transferSize != l2->transferSize) {
          fprintf(stderr, "Report transferLog %d difference at %d\n", r, i);
          err = -1;
        }
      }
    }
  }

  return err;
}
