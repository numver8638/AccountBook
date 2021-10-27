#pragma once

#include "Database.h"
#include "Status.h"

typedef Status (*MenuHandler)(DatabaseRef);

Status ShowAll(DatabaseRef);
Status ShowIncomes(DatabaseRef);
Status ShowWithdraws(DatabaseRef);
Status AddIncome(DatabaseRef);
Status AddWithdraw(DatabaseRef);
Status Search(DatabaseRef);
Status Update(DatabaseRef);
Status Remove(DatabaseRef);