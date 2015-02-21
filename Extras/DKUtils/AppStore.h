#pragma once
#include <DK.h>

////////////////////////////////////////////////////////////////////////////////
//
// AppStore
//
// 맥, iOS 플랫폼에서 In-app purchase 해주는 클래스
//
// UpdateProduct 로 제품 정보를 가져온 후에, RestorePreviousTransactions 또는 BeginTransaction 를 해야한다.
// RestorePreviousTransactions 는 기존의 구매내역 가져오는 함수
// BeginTransaction 는 새로 구매함
////////////////////////////////////////////////////////////////////////////////

namespace DKUtils
{
	class AppStore : public DKFoundation::DKSharedInstance<AppStore>
	{
	public:
		~AppStore(void);
		
		// 제품 정보 (제품 정보는 UpdateProduct.. 를 호출했을때 받으며, 이 객체에 저장되지 않음)
		struct Product
		{
			DKFoundation::DKString productId;
			DKFoundation::DKString title;
			DKFoundation::DKString description;
			DKFoundation::DKString localizedPrice;		// 화면 출력용 가격 스트링
			double price;
		};
		// 트랜젝션 (미완료된 트랜젝션이 이 객체에 저장됨, 완료된 트랜젝션은 저장되지 않는다.)
		struct Transaction
		{
			enum State
			{
				StatePurchased = 0,
				StateRestored,
			};
			State state;
			DKFoundation::DKString transactionId;
			DKFoundation::DKDateTime date;
			DKFoundation::DKString productId;
			DKFoundation::DKObject<DKFoundation::DKData> receipt;		// 영수증 데이터 (2011-12-03: 현재 iOS 에서만 존재함)
			long productQuantity;
			Transaction* originalTransaction;	// StateRestored 일때 원본의 transactionId
			void* internalContext;
			bool completed;
		};
		struct Error
		{
			DKFoundation::DKString description;
			DKFoundation::DKString reason;
			DKFoundation::DKString suggestion;			
		};
		// 트랜젝션 실패 (제품 아이디와 실패 사유 및 에러 포함)
		struct TransactionError
		{
			enum ErrorCode
			{
				ErrorUnknown = 0,				// 알수 없는 에러
				ErrorClientInvalid,				// 유효하지 않은 클라이언트
				ErrorPaymentCancelled,			// 사용자가 취소함
				ErrorPaymentInvalid,			// 결재정보가 유효하지 않음
				ErrorPaymentNotAllowed,			// 승인 불가능
			};
			ErrorCode code;
			DKFoundation::DKString productId;
			DKFoundation::DKObject<Error> error;
		};
		// 트랜젝션 복구 결과
		struct RestoreNotification
		{
			enum Result
			{
				RestoreCompleted = 0,
				RestoreCompletedWithError,
			};
			Result result;
			DKFoundation::DKObject<Error> error;
		};

		typedef void TransactionCallback(DKFoundation::DKString);		// transactionId
		typedef void TransactionErrorCallback(TransactionError);
		typedef void RestoreCallback(RestoreNotification);

		// 이전 구매 기록 복구함
		void RestorePreviousTransactions(void);
		// 트랜젝션 검색 (복구하거나 구매해서 기록이 생기면 조회 가능함 - FinishTransaction 을 호출하면 지워진다!)
		const Transaction* FindTransaction(const DKFoundation::DKString& tid) const;
		// 구매 트랜젝션을 완료한다. (앱 내에서 처리 후에 호출함)
		void FinishTransaction(const DKFoundation::DKString& tid);
		// 미완성 트랜젝션 조회. (이전에 처리 완료전에 종료됐다거나.. 하는 상황)
		DKFoundation::DKArray<DKFoundation::DKString> IncompleteTransactions(void) const;
		// 새 구매 트랜젝션 시작함
		bool BeginTransaction(const DKFoundation::DKString& productId, int quantity);
		// 구매나 복구시 콜백 등록
		void SetCallback(DKFoundation::DKFunctionSignature<TransactionCallback>* tc,
						 DKFoundation::DKFunctionSignature<TransactionErrorCallback>* ec,
						 DKFoundation::DKFunctionSignature<RestoreCallback>* rn,
						 DKFoundation::DKRunLoop* runLoop, void* context);
		void RemoveCallback(void* context);
						 
		bool IsStoreEnabled(void) const;
		
		typedef DKFoundation::DKFunctionSignature<void (DKFoundation::DKArray<Product>)> ProductCallback;

		bool QueryProductInfo(const DKFoundation::DKString* pids, size_t numPids, ProductCallback* callback, DKFoundation::DKRunLoop* runLoop);
		bool QueryProductInfoFromPListDictFile(const DKFoundation::DKString& plist, const char* arrayKey, ProductCallback* callback, DKFoundation::DKRunLoop* runLoop);
		bool QueryProductInfoFromPListDictURL(const DKFoundation::DKString& plist, const char* arrayKey, ProductCallback* callback, DKFoundation::DKRunLoop* runLoop);
		bool QueryProductInfoFromPListArrayFile(const DKFoundation::DKString& plist, ProductCallback* callback, DKFoundation::DKRunLoop* runLoop);
		bool QueryProductInfoFromPListArrayURL(const DKFoundation::DKString& plist, ProductCallback* callback, DKFoundation::DKRunLoop* runLoop);
			
	private:
		void InternalProductProc(void*);
		void InternalTransactionProc(void*);
		void InternalRestoreProc(bool, void*);

		struct ProductCallbackInfo
		{
			DKFoundation::DKObject<ProductCallback> function;
			DKFoundation::DKRunLoop* runLoop;
		};
		
		typedef DKFoundation::DKMap<void*, ProductCallbackInfo> ProductCallbackInfoMap;
		ProductCallbackInfoMap productCallbackInfoMap;
		
		typedef DKFoundation::DKMap<DKFoundation::DKString, Transaction> TransactionMap;
		TransactionMap transactionMap;
		
		DKFoundation::DKCallback<TransactionCallback, void*> transactionCallback;
		DKFoundation::DKCallback<TransactionErrorCallback, void*> transactionErrorCallback;
		DKFoundation::DKCallback<RestoreCallback, void*> restoreNotificationCallback;
		
		AppStore(void);	
		void* context;
		DKFoundation::DKSpinLock lock;
		friend class DKFoundation::DKObject<AppStore>;
		friend class DKFoundation::DKSharedInstance<AppStore>;
	};
}
