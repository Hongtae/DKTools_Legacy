#include "AppStore.h"

#ifdef _WIN32
using namespace DKFoundation;
using namespace DKFramework;
using namespace DKUtils;

AppStore::AppStore(void)
: context(NULL)
{
}

AppStore::~AppStore(void)
{
}

const AppStore::Transaction* AppStore::FindTransaction(const DKFoundation::DKString& tid) const
{
	return NULL;
}

void AppStore::FinishTransaction(const DKFoundation::DKString& tid)
{
}

DKArray<DKString> AppStore::IncompleteTransactions(void) const
{
	return DKArray<DKString>();
}

bool AppStore::BeginTransaction(const DKString& productId, int quantity)
{
	return false;
}

bool AppStore::IsStoreEnabled(void) const
{
	return false;
}

bool AppStore::QueryProductInfo(const DKString* pids, size_t numPids, ProductCallback* callback, DKRunLoop* runLoop)
{
	return false;
}

bool AppStore::QueryProductInfoFromPListDictFile(const DKFoundation::DKString& plist, const char* arrayKey, ProductCallback* callback, DKRunLoop* runLoop)
{
	return false;
}

bool AppStore::QueryProductInfoFromPListDictURL(const DKFoundation::DKString& url, const char* arrayKey, ProductCallback* callback, DKRunLoop* runLoop)
{
	return false;
}

bool AppStore::QueryProductInfoFromPListArrayFile(const DKFoundation::DKString& plist, ProductCallback* callback, DKRunLoop* runLoop)
{
	return false;
}

bool AppStore::QueryProductInfoFromPListArrayURL(const DKFoundation::DKString& url, ProductCallback* callback, DKRunLoop* runLoop)
{
	return false;
}

void AppStore::SetCallback(DKFunctionSignature<TransactionCallback>* tc,
						   DKFunctionSignature<TransactionErrorCallback>* ec,
						   DKFunctionSignature<RestoreCallback>* rn,
						   DKRunLoop* runLoop, void* context)
{
	DKCriticalSection<DKSpinLock> guard(lock);
	transactionCallback.SetCallback(tc, runLoop, context);
	transactionErrorCallback.SetCallback(ec, runLoop, context);
	restoreNotificationCallback.SetCallback(rn, runLoop, context);
}

void AppStore::RemoveCallback(void* context)
{
	DKCriticalSection<DKSpinLock> guard(lock);
	transactionCallback.Remove(context);
	transactionErrorCallback.Remove(context);
	restoreNotificationCallback.Remove(context);
}

void AppStore::RestorePreviousTransactions(void)
{
}

#pragma mark - Internal Callback
void AppStore::InternalProductProc(void* ptr)
{	
}

void AppStore::InternalTransactionProc(void* p)
{
}

void AppStore::InternalRestoreProc(bool s, void* err)
{	
}


#endif
