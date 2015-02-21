#if defined(__APPLE__) && defined(__MACH__)

#import <TargetConditionals.h>
#import <StoreKit/StoreKit.h>

#include "AppStore.h"
using namespace DKFoundation;
using namespace DKFramework;
using namespace DKUtils;

typedef void (AppStore::*ProductProc)(void*);
typedef void (AppStore::*TransactionProc)(void*);
typedef void (AppStore::*RestoreProc)(bool, void*);

////////////////////////////////////////////////////////////////////////////////
// 제품 정보 요청하는 객체
// 한번 생성되서 제품정보 요청이 완료되면 파기됨.
#pragma mark - SKProductRequestDelegate
@interface AppStoreProductRequestDelegate : NSObject<SKProductsRequestDelegate, SKRequestDelegate>
{
	AppStore* storeInterface;
	ProductProc productProc;
	NSArray* productData;
	NSError* requestError;
}
@property (nonatomic, assign) AppStore* storeInterface;
@property (nonatomic, assign) ProductProc productProc;
@property (nonatomic, retain) NSArray* productData;
@property (nonatomic, retain) NSError* requestError;
- (id)initWithStoreInterface:(AppStore*)store withProductProc:(ProductProc)proc;
@end
@implementation AppStoreProductRequestDelegate
@synthesize storeInterface;
@synthesize productProc;
@synthesize productData;
@synthesize requestError;

- (id)initWithStoreInterface:(AppStore*)store withProductProc:(ProductProc)proc
{
	self = [super init];
	if (self)
	{
		self.storeInterface = store;
		self.productProc = proc;
	}
	return self;
}

- (void)dealloc
{
	self.productData = nil;
	self.requestError = nil;
	[super dealloc];
}

- (void)requestProductInfo:(NSSet*)products
{
	SKProductsRequest *request = [[SKProductsRequest alloc] initWithProductIdentifiers:products];
	request.delegate = self;
	[request start];
}

- (void)productsRequest:(SKProductsRequest *)request didReceiveResponse:(SKProductsResponse *)response
{
	self.productData = response.products;
	[request autorelease];	
}

#pragma mark - SKRequestDelegate
- (void)requestDidFinish:(SKRequest *)request
{
	self.requestError = nil;
	((*storeInterface).*productProc)(self);
}
- (void)request:(SKRequest *)request didFailWithError:(NSError *)error
{
	self.requestError = error;
	((*storeInterface).*productProc)(self);
}

@end

////////////////////////////////////////////////////////////////////////////////
// 트랜젝션 옵저버 객체 (계속해서 트랜젝션 모니터링 함)
#pragma mark - AppStorePaymentQueueDelegate
@interface AppStorePaymentQueueDelegate : NSObject <SKPaymentTransactionObserver>
{
	AppStore* storeInterface;
	TransactionProc transactionProc;
	RestoreProc restoreProc;
}
@property (nonatomic, assign) AppStore* storeInterface;
@property (nonatomic, assign) TransactionProc transactionProc;
@property (nonatomic, assign) RestoreProc restoreProc;
- (id)initWithStoreInterface:(AppStore*)store withTransactionProc:(TransactionProc)transaction withRestoreProc:(RestoreProc)restore;
@end

@implementation AppStorePaymentQueueDelegate
@synthesize storeInterface;
@synthesize transactionProc;
@synthesize restoreProc;

- (id)initWithStoreInterface:(AppStore*)store withTransactionProc:(TransactionProc)transaction withRestoreProc:(RestoreProc)restore
{
	self = [super init];
	if (self)
	{
		self.storeInterface = store;
		self.transactionProc = transaction;
		self.restoreProc = restore;
	}
	return self;
}

#pragma mark - SKPaymentTransactionObserver
- (void)paymentQueue:(SKPaymentQueue *)queue updatedTransactions:(NSArray *)transactions
{
	for (SKPaymentTransaction* transaction in transactions)
	{
		((*storeInterface).*transactionProc)((void*)transaction);
	}
}

- (void)paymentQueueRestoreCompletedTransactionsFinished:(SKPaymentQueue *)queue
{
	((*storeInterface).*restoreProc)(true, nil);
}

- (void)paymentQueue:(SKPaymentQueue *)queue restoreCompletedTransactionsFailedWithError:(NSError *)error
{
	((*storeInterface).*restoreProc)(false, error);
}

@end

#pragma mark - AppStore class implementation

AppStore::AppStore(void)
: context(NULL)
{
	AppStorePaymentQueueDelegate* store = [[AppStorePaymentQueueDelegate alloc] initWithStoreInterface:this
																				   withTransactionProc:&AppStore::InternalTransactionProc
																					   withRestoreProc:&AppStore::InternalRestoreProc];
	if (store == nil)
	{
		DKERROR_THROW("AppStorePaymentQueueDelegate allocation failed!\n");
	}
	
	[[SKPaymentQueue defaultQueue] addTransactionObserver:store];
	context = reinterpret_cast<void*>(store);
}

AppStore::~AppStore(void)
{
	AppStorePaymentQueueDelegate* store = reinterpret_cast<AppStorePaymentQueueDelegate*>(context);	
	[[SKPaymentQueue defaultQueue] removeTransactionObserver:store];	
	[store release];
	context = NULL;
}

const AppStore::Transaction* AppStore::FindTransaction(const DKFoundation::DKString& tid) const
{
	DKCriticalSection<DKSpinLock> guard(lock);
	const TransactionMap::Pair* p = transactionMap.Find(tid);
	if (p)
	{
		return &p->value;
	}	
	return NULL;
}

void AppStore::FinishTransaction(const DKFoundation::DKString& tid)
{
	DKCriticalSection<DKSpinLock> guard(lock);
	
	TransactionMap::Pair* p = transactionMap.Find(tid);
	if (p)
	{
		if (p->value.completed == false)
		{
			[[SKPaymentQueue defaultQueue] finishTransaction:(SKPaymentTransaction*)p->value.internalContext];
			p->value.completed = true;
		}
		
		if (p->value.originalTransaction && p->value.originalTransaction->completed)
		{
			transactionMap.Remove(p->value.originalTransaction->transactionId);
		}
	}
	transactionMap.Remove(tid);
}

DKArray<DKString> AppStore::IncompleteTransactions(void) const
{
	DKCriticalSection<DKSpinLock> guard(lock);
	DKArray<DKString> keys;
	keys.Reserve(transactionMap.Count());
	transactionMap.EnumerateForward([&keys](const TransactionMap::Pair& pair)
									{
										keys.Add(pair.key);
									});
	return keys;
}

bool AppStore::BeginTransaction(const DKString& productId, int quantity)
{
	DKCriticalSection<DKSpinLock> guard(lock);
	if ([SKPaymentQueue canMakePayments])
	{
		SKMutablePayment* payment = [[SKMutablePayment alloc] init];
		payment.productIdentifier = [NSString stringWithUTF8String:(const char*)DKStringU8(productId)];
		payment.quantity = quantity;
		payment.requestData = nil;

#ifdef DKLIB_DEBUG_ENABLED
		[[SKPaymentQueue defaultQueue] addPayment:payment];
		[payment release];
		return true;
#else		
		@try
		{
			[[SKPaymentQueue defaultQueue] addPayment:payment];
			[payment release];
			return true;
		}
		@catch (NSException* e)
		{
			NSLog(@"ERROR: SKPaymentQueue addPayment failed: (%@)\n", [e reason]);			
		}
		@catch (NSString* s)
		{
			NSLog(@"ERROR: SKPaymentQueue addPayment failed: (%@)\n", s);
		}
#endif
	}
	return false;
}

bool AppStore::IsStoreEnabled(void) const
{
	return [SKPaymentQueue canMakePayments] == YES;
}

bool AppStore::QueryProductInfo(const DKString* pids, size_t numPids, ProductCallback* callback, DKRunLoop* runLoop)
{
	if (pids == NULL || numPids == 0 || callback == NULL)
		return false;

	NSMutableSet* pset = [[NSMutableSet alloc] initWithCapacity:numPids];
	for (size_t i = 0; i < numPids; ++i)
	{
		NSString* productId = [NSString stringWithUTF8String:(const char*)DKStringU8(pids[i])];
		if (productId.length > 0)
			[pset addObject:productId];
	}
	
	if (pset.count > 0)
	{
		AppStoreProductRequestDelegate* request = [[AppStoreProductRequestDelegate alloc] initWithStoreInterface:this
																								 withProductProc:&AppStore::InternalProductProc];
		if (request)
		{
			ProductCallbackInfo callbackInfo = {callback, runLoop};
			
			DKCriticalSection<DKSpinLock> guard(lock);		
			if (productCallbackInfoMap.Insert((void*)request, callbackInfo))
			{
				[request requestProductInfo:pset];
				return true;
			}
		}
		else
		{
			DKLog("ERROR: ProductRequest object allocation failed.\n");
		}
	}
	return false;
}

bool AppStore::QueryProductInfoFromPListDictFile(const DKFoundation::DKString& plist, const char* arrayKey, ProductCallback* callback, DKRunLoop* runLoop)
{
	NSDictionary* dict = [NSDictionary dictionaryWithContentsOfFile:[NSString stringWithUTF8String:(const char*)DKStringU8(plist)]];
	if (dict)
	{
		NSArray* items = [dict objectForKey:[NSString stringWithUTF8String:arrayKey]];
		if ([items isKindOfClass:[NSArray class]] && items.count > 0)
		{
			NSSet* pset = [NSSet setWithArray:items];
			if (pset.count > 0)
			{
				AppStoreProductRequestDelegate* request = [[AppStoreProductRequestDelegate alloc] initWithStoreInterface:this
																										 withProductProc:&AppStore::InternalProductProc];
				if (request)
				{
					ProductCallbackInfo callbackInfo = {callback, runLoop};
					
					DKCriticalSection<DKSpinLock> guard(lock);		
					if (productCallbackInfoMap.Insert((void*)request, callbackInfo))
					{
						[request requestProductInfo:pset];
						return true;
					}
				}
				else
				{
					DKLog("ERROR: ProductRequest object allocation failed.\n");
				}
			}
		}
	}
	return false;
}

bool AppStore::QueryProductInfoFromPListDictURL(const DKFoundation::DKString& url, const char* arrayKey, ProductCallback* callback, DKRunLoop* runLoop)
{
	NSDictionary* dict = [NSDictionary dictionaryWithContentsOfURL:[NSURL URLWithString:[NSString stringWithUTF8String:(const char*)DKStringU8(url)]]];
	if (dict)
	{
		NSArray* items = [dict objectForKey:[NSString stringWithUTF8String:arrayKey]];
		if ([items isKindOfClass:[NSArray class]] && items.count > 0)
		{
			NSSet* pset = [NSSet setWithArray:items];
			if (pset.count > 0)
			{
				AppStoreProductRequestDelegate* request = [[AppStoreProductRequestDelegate alloc] initWithStoreInterface:this
																										 withProductProc:&AppStore::InternalProductProc];
				if (request)
				{
					ProductCallbackInfo callbackInfo = {callback, runLoop};
					
					DKCriticalSection<DKSpinLock> guard(lock);		
					if (productCallbackInfoMap.Insert((void*)request, callbackInfo))
					{
						[request requestProductInfo:pset];
						return true;
					}
				}
				else
				{
					DKLog("ERROR: ProductRequest object allocation failed.\n");
				}
			}	
		}
	}
	return false;
}

bool AppStore::QueryProductInfoFromPListArrayFile(const DKFoundation::DKString& plist, ProductCallback* callback, DKRunLoop* runLoop)
{
	NSArray* items = [NSArray arrayWithContentsOfFile:[NSString stringWithUTF8String:(const char*)DKStringU8(plist)]];
	if (items.count > 0)
	{
		NSSet* pset = [NSSet setWithArray:items];
		if (pset.count > 0)
		{
			AppStoreProductRequestDelegate* request = [[AppStoreProductRequestDelegate alloc] initWithStoreInterface:this
																									 withProductProc:&AppStore::InternalProductProc];
			if (request)
			{
				ProductCallbackInfo callbackInfo = {callback, runLoop};
				
				DKCriticalSection<DKSpinLock> guard(lock);		
				if (productCallbackInfoMap.Insert((void*)request, callbackInfo))
				{
					[request requestProductInfo:pset];
					return true;
				}
			}
			else
			{
				DKLog("ERROR: ProductRequest object allocation failed.\n");
			}
		}
	}
	return false;
}

bool AppStore::QueryProductInfoFromPListArrayURL(const DKFoundation::DKString& url, ProductCallback* callback, DKRunLoop* runLoop)
{
	NSArray* items = [NSArray arrayWithContentsOfURL:[NSURL URLWithString:[NSString stringWithUTF8String:(const char*)DKStringU8(url)]]];
	if (items.count > 0)
	{
		NSSet* pset = [NSSet setWithArray:items];
		if (pset.count > 0)
		{
			AppStoreProductRequestDelegate* request = [[AppStoreProductRequestDelegate alloc] initWithStoreInterface:this
																									 withProductProc:&AppStore::InternalProductProc];
			if (request)
			{
				ProductCallbackInfo callbackInfo = {callback, runLoop};
				
				DKCriticalSection<DKSpinLock> guard(lock);		
				if (productCallbackInfoMap.Insert((void*)request, callbackInfo))
				{
					[request requestProductInfo:pset];
					return true;
				}
			}
			else
			{
				DKLog("ERROR: ProductRequest object allocation failed.\n");
			}
		}	
	}
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
	DKCriticalSection<DKSpinLock> guard(lock);
	[[SKPaymentQueue defaultQueue] restoreCompletedTransactions];
}

#pragma mark - Internal Callback
void AppStore::InternalProductProc(void* ptr)
{	
	AppStoreProductRequestDelegate* request = (AppStoreProductRequestDelegate*)ptr;
	DKCriticalSection<DKSpinLock> guard(lock);	
	ProductCallbackInfoMap::Pair* p = productCallbackInfoMap.Find(ptr);
	if (p)
	{
		ProductCallbackInfo& callback = p->value;
		if (callback.function)
		{			
			NSArray* products = request.productData;
			DKArray<Product> productList;
			productList.Reserve(products.count);
			
			NSNumberFormatter *numberFormatter = [[[NSNumberFormatter alloc] init] autorelease];
			[numberFormatter setFormatterBehavior:NSNumberFormatterBehavior10_4];
			[numberFormatter setNumberStyle:NSNumberFormatterCurrencyStyle];
			
			for (SKProduct* product in products)
			{	
				[numberFormatter setLocale:product.priceLocale];
				NSString *formattedPrice = [numberFormatter stringFromNumber:product.price];
				
				Product p;
				p.productId = DKString((const DKUniChar8*)[product.productIdentifier UTF8String]);
				p.title = DKString((const DKUniChar8*)[product.localizedTitle UTF8String]);
				p.description = DKString((const DKUniChar8*)[product.localizedDescription UTF8String]);
				p.localizedPrice = DKString((const DKUniChar8*)[formattedPrice UTF8String]);
				p.price = [product.price doubleValue];
				
				productList.Add(p);
			}

			DKObject<DKOperation> op = callback.function->Invocation(productList).SafeCast<DKOperation>();
			if (callback.runLoop)
				callback.runLoop->PostOperation(op,0);
			else
				op->Perform();
		}
	}
	productCallbackInfoMap.Remove(p);
	[request autorelease];
}

void AppStore::InternalTransactionProc(void* p)
{
	SKPaymentTransaction* transaction = (SKPaymentTransaction*)p;
	
	switch (transaction.transactionState)
	{
		case SKPaymentTransactionStateRestored:				// 이전에 구매한 기록
			if (true)
			{
				lock.Lock();
				
				Transaction trans;
				trans.state = Transaction::StateRestored;
				trans.transactionId = DKString((const DKUniChar8*)[transaction.transactionIdentifier UTF8String]);
				trans.productId = DKString((const DKUniChar8*)[transaction.payment.productIdentifier UTF8String]);
				trans.productQuantity = transaction.payment.quantity;
				trans.receipt = NULL;
				trans.date = DKDateTime((double)[transaction.transactionDate timeIntervalSince1970]);
				trans.internalContext = (void*)transaction;
				trans.completed = false;
				
				if (transaction.originalTransaction && transaction.originalTransaction.transactionState == SKPaymentTransactionStatePurchased)
				{
					DKString tid = DKString((const DKUniChar8*)[transaction.originalTransaction.transactionIdentifier UTF8String]);
					TransactionMap::Pair* p = transactionMap.Find(tid);
					if (p)
					{
						trans.originalTransaction = &p->value;							
					}
					else
					{					
						Transaction origTrans;
						origTrans.state = Transaction::StatePurchased;
						origTrans.transactionId = tid;
						origTrans.productId = DKString((const DKUniChar8*)[transaction.originalTransaction.payment.productIdentifier UTF8String]);
						origTrans.productQuantity = transaction.originalTransaction.payment.quantity;
						origTrans.receipt = NULL;
						origTrans.date = DKDateTime((double)[transaction.originalTransaction.transactionDate timeIntervalSince1970]);
						origTrans.originalTransaction = NULL;
						origTrans.internalContext = (void*)transaction.originalTransaction;
						origTrans.completed = true;
						
						if ([transaction.originalTransaction respondsToSelector:@selector(transactionReceipt)])
						{
							NSData* receipt = [transaction.originalTransaction transactionReceipt];
							if (receipt && [receipt length] > 0)
								origTrans.receipt = DKBuffer::Create((const void*)[receipt bytes], (size_t)[receipt length]).SafeCast<DKData>();
						}
						
						transactionMap.Insert(tid, origTrans);
						trans.originalTransaction = &transactionMap.Find(tid)->value;
					}
				}
				else
				{
					trans.originalTransaction = NULL;					
				}
				
				transactionMap.Update(trans.transactionId, trans);
				lock.Unlock();
				
				transactionCallback.PostInvocation(trans.transactionId);	
			}
			break;
		case SKPaymentTransactionStatePurchased:			// 구매 완료
			if (true )
			{
				Transaction trans;
				trans.state = Transaction::StatePurchased;
				trans.transactionId = DKString((const DKUniChar8*)[transaction.transactionIdentifier UTF8String]);
				trans.productId = DKString((const DKUniChar8*)[transaction.payment.productIdentifier UTF8String]);
				trans.productQuantity = transaction.payment.quantity;
				trans.receipt = NULL;
				trans.date = DKDateTime((double)[transaction.transactionDate timeIntervalSince1970]);
				trans.originalTransaction = NULL;
				trans.internalContext = (void*)transaction;
				trans.completed = false;

				if ([transaction respondsToSelector:@selector(transactionReceipt)])
				{
					NSData* receipt = [transaction transactionReceipt];
					if (receipt && [receipt length] > 0)
						trans.receipt = DKBuffer::Create((const void*)[receipt bytes], (size_t)[receipt length]).SafeCast<DKData>();
				}

				if (true)
				{
					DKCriticalSection<DKSpinLock> guard(lock);
					transactionMap.Update(trans.transactionId, trans);
				}
				transactionCallback.PostInvocation(trans.transactionId);
			}
			break;
		case SKPaymentTransactionStateFailed:				// 구매 실패
			if (true)
			{
				TransactionError transactionError;
				switch (transaction.error.code)
				{
					case SKErrorClientInvalid:       // client is not allowed to issue the request, etc.
						transactionError.code = TransactionError::ErrorClientInvalid;
						break;
					case SKErrorPaymentCancelled:    // user cancelled the request, etc.
						transactionError.code = TransactionError::ErrorPaymentCancelled;
						break;
					case SKErrorPaymentInvalid:      // purchase identifier was invalid, etc.
						transactionError.code = TransactionError::ErrorPaymentInvalid;
						break;
					case SKErrorPaymentNotAllowed:    // this machine is not allowed to make the payment		
						transactionError.code = TransactionError::ErrorPaymentNotAllowed;
						break;
					default:
						transactionError.code = TransactionError::ErrorUnknown;
						break;
				}
				
				DKLog("transaction error:%s\n", (const char*)[[transaction.error localizedDescription] UTF8String]);
				
				transactionError.productId = DKString((const DKUniChar8*)[transaction.payment.productIdentifier UTF8String]);
				transactionError.error = DKObject<Error>::New();
				transactionError.error->description = DKString((const DKUniChar8*)[[transaction.error localizedDescription] UTF8String]);
				transactionError.error->reason = DKString((const DKUniChar8*)[[transaction.error localizedFailureReason] UTF8String]);
				transactionError.error->suggestion = DKString((const DKUniChar8*)[[transaction.error localizedRecoverySuggestion] UTF8String]);
				
				transactionErrorCallback.PostInvocation(transactionError);		
			}
			
			this->lock.Lock();				
			[[SKPaymentQueue defaultQueue] finishTransaction:transaction];
			this->lock.Unlock();
			break;
		default:
			break;
	}
}

void AppStore::InternalRestoreProc(bool s, void* err)
{
	RestoreNotification restoreResult;
	
	if (err)
	{
		NSError* error = (NSError*)err;
		
		restoreResult.error = DKObject<Error>::New();
		restoreResult.error->description = DKString((const DKUniChar8*)[[error localizedDescription] UTF8String]);
		restoreResult.error->reason = DKString((const DKUniChar8*)[[error localizedFailureReason] UTF8String]);
		restoreResult.error->suggestion = DKString((const DKUniChar8*)[[error localizedRecoverySuggestion] UTF8String]);
	}

	if (s)
		restoreResult.result = RestoreNotification::RestoreCompleted;
	else
		restoreResult.result = RestoreNotification::RestoreCompletedWithError;

	restoreNotificationCallback.PostInvocation(restoreResult);
}


#endif
