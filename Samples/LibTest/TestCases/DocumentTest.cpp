#include "../TestCase.h"


struct MyCustomObject1 : public DKResource
{
	float a;
	float b;
	DKVector3 vector;

	DKObject<DKSerializer> Serializer(void)
	{
		DKObject<DKSerializer> ser = DKObject<DKSerializer>::New();
		ser->SetResourceClass(L"MyCustomObject1");
		ser->Bind(L"super", DKResource::Serializer(), NULL);
		ser->Bind(L"a",
			DKFunction([this](DKVariant& v) {v = (DKVariant::VFloat)a;}),
			DKFunction([this](DKVariant& v) {a = v.Float();}),
			DKFunction([](const DKVariant& v)-> bool {return v.ValueType() == DKVariant::TypeFloat;}),
			NULL);
		ser->Bind(L"b",
			DKFunction([this](DKVariant& v) {v = (DKVariant::VFloat)b;}),
			DKFunction([this](DKVariant& v) {b = v.Float();}),
			DKFunction([](const DKVariant& v)-> bool {return v.ValueType() == DKVariant::TypeFloat;}),
			NULL);
		ser->SetCallback(DKFunction([](DKSerializer::State s) {
			switch (s)
			{
			case DKSerializer::StateSerializeBegin:
				DKLog("[MyCustomObject1] SerializeBegin\n"); break;
			case DKSerializer::StateSerializeSucceed:
				DKLog("[MyCustomObject1] SerializeSucceed\n"); break;
			case DKSerializer::StateSerializeFailed:
				DKLog("[MyCustomObject1] SerializeFailed\n"); break;
			case DKSerializer::StateDeserializeBegin:
				DKLog("[MyCustomObject1] DeserializeBegin\n"); break;
			case DKSerializer::StateDeserializeSucceed:
				DKLog("[MyCustomObject1] DeserializeSucceed\n"); break;
			case DKSerializer::StateDeserializeFailed:
				DKLog("[MyCustomObject1] DeserializeFailed\n"); break;
			default:
				DKLog("[MyCustomObject1] Unknown Serializer state\n"); break;
			}
		}));

		return ser;
	}
};

struct MyCustomObject2 : public DKResource
{
	int value1;
	int value2;
	DKString value3;
	DKObject<MyCustomObject1> object1;
	DKObject<MyCustomObject1> object2;

	DKObject<DKSerializer> Serializer(void)
	{
		DKObject<DKSerializer> ser = DKObject<DKSerializer>::New();
		ser->SetResourceClass(L"MyCustomObject2");
		ser->Bind(L"super", DKResource::Serializer(), NULL);
		ser->Bind(L"value1",
			DKFunction([this](DKVariant& v) {v = (DKVariant::VInteger)value1;}),
			DKFunction([this](DKVariant& v) {value1 = v.Integer();}),
			DKFunction([](const DKVariant& v)-> bool {return v.ValueType() == DKVariant::TypeInteger;}),
			NULL);
		ser->Bind(L"value2",
			DKFunction([this](DKVariant& v) {v = (DKVariant::VInteger)value2;}),
			DKFunction([this](DKVariant& v) {value2 = v.Integer();}),
			DKFunction([](const DKVariant& v)-> bool {return v.ValueType() == DKVariant::TypeInteger;}),
			NULL);
		ser->Bind(L"value3",
			DKFunction([this](DKVariant& v) {v = (DKVariant::VString)value3;}),
			DKFunction([this](DKVariant& v) {value3 = v.String();}),
			DKFunction([](const DKVariant& v)-> bool {return v.ValueType() == DKVariant::TypeString;}),
			NULL);
		ser->Bind(L"object1",
			DKFunction([this](DKObject<DKResource>& r) {r = object1.SafeCast<DKResource>();}),
			DKFunction([this](DKObject<DKResource>& r) {object1 = r.SafeCast<MyCustomObject1>();}),
			DKFunction([](const DKObject<DKResource>& r)-> bool {return r.SafeCast<MyCustomObject1>() != NULL;}),
			DKSerializer::ExternalResourceReferenceIfPossible,
			DKFunction([this] {object1 = NULL;})->Invocation().SafeCast<DKOperation>());
		ser->SetCallback(DKFunction([](DKSerializer::State s) {
			switch (s)
			{
			case DKSerializer::StateSerializeBegin:
				DKLog("[MyCustomObject2] SerializeBegin\n"); break;
			case DKSerializer::StateSerializeSucceed:
				DKLog("[MyCustomObject2] SerializeSucceed\n"); break;
			case DKSerializer::StateSerializeFailed:
				DKLog("[MyCustomObject2] SerializeFailed\n"); break;
			case DKSerializer::StateDeserializeBegin:
				DKLog("[MyCustomObject2] DeserializeBegin\n"); break;
			case DKSerializer::StateDeserializeSucceed:
				DKLog("[MyCustomObject2] DeserializeSucceed\n"); break;
			case DKSerializer::StateDeserializeFailed:
				DKLog("[MyCustomObject2] DeserializeFailed\n"); break;
			default:
				DKLog("[MyCustomObject2] Unknown Serializer state\n"); break;
			}
		}));

		return ser;
	}
};

struct MyCustomObject3 : public MyCustomObject2
{
	float value1;
	float value2;

	typedef DKArray<DKObject<MyCustomObject1>> Obj1Array;
	typedef DKMap<int, DKObject<MyCustomObject1>> Obj1Map;
	Obj1Array obj1Array;
	Obj1Map obj1Map;
	

	DKObject<DKSerializer> Serializer(void)
	{
		DKObject<DKSerializer> ser = DKObject<DKSerializer>::New();
		ser->SetResourceClass(L"MyCustomObject3");
		ser->Bind(L"super", MyCustomObject2::Serializer(), NULL);
		ser->Bind(L"value1",
			DKFunction([this](DKVariant& v) {v = (DKVariant::VFloat)value1;}),
			DKFunction([this](DKVariant& v) {value1 = v.Float();}),
			DKFunction([](const DKVariant& v)-> bool {return v.ValueType() == DKVariant::TypeFloat;}),
			NULL);
		ser->Bind(L"value2",
			DKFunction([this](DKVariant& v) {v = (DKVariant::VFloat)value2;}),
			DKFunction([this](DKVariant& v) {value2 = v.Float();}),
			DKFunction([](const DKVariant& v)-> bool {return v.ValueType() == DKVariant::TypeFloat;}),
			NULL);
		ser->Bind(L"obj1Array",
			DKFunction([this](DKSerializer::ExternalArrayType& v)
			{
				v.Reserve(obj1Array.Count());
				for (size_t i = 0; i < obj1Array.Count(); ++i)
					v.Add(obj1Array.Value(i).SafeCast<DKResource>());
			}),
			DKFunction([this](DKSerializer::ExternalArrayType& t)
			{
				obj1Array.Clear();
				obj1Array.Reserve(t.Count());
				for (size_t i = 0; i < t.Count(); ++i)
				{
					MyCustomObject1 *p = t.Value(i).SafeCast<MyCustomObject1>();
					if (p)
						obj1Array.Add(p);
				}
			}),
			DKFunction([this](const DKSerializer::ExternalArrayType&)->bool {return true;}),
			DKSerializer::ExternalResourceInclude,
			NULL);
		ser->Bind(L"obj1Map",
			DKFunction([this](DKSerializer::ExternalMapType& v)
			{
				obj1Map.EnumerateForward([&v](MyCustomObject3::Obj1Map::Pair& p)
				{
					v.Insert(DKString::Format("%u", p.key), p.value.SafeCast<DKResource>());
				});
			}),
			DKFunction([this](DKSerializer::ExternalMapType& v)
			{
				obj1Map.Clear();
				v.EnumerateForward([this](DKSerializer::ExternalMapType::Pair& p)
				{
					int key = p.key.ToUnsignedInteger();
					MyCustomObject1* val = p.value.SafeCast<MyCustomObject1>();
					if (val)
						obj1Map.Insert(key, val);
				});
			}),
			DKFunction([this](const DKSerializer::ExternalMapType&)->bool {return true;}),
			DKSerializer::ExternalResourceInclude,
			NULL);
		ser->SetCallback(DKFunction([](DKSerializer::State s) {
			switch (s)
			{
			case DKSerializer::StateSerializeBegin:
				DKLog("[MyCustomObject3] SerializeBegin\n"); break;
			case DKSerializer::StateSerializeSucceed:
				DKLog("[MyCustomObject3] SerializeSucceed\n"); break;
			case DKSerializer::StateSerializeFailed:
				DKLog("[MyCustomObject3] SerializeFailed\n"); break;
			case DKSerializer::StateDeserializeBegin:
				DKLog("[MyCustomObject3] DeserializeBegin\n"); break;
			case DKSerializer::StateDeserializeSucceed:
				DKLog("[MyCustomObject3] DeserializeSucceed\n"); break;
			case DKSerializer::StateDeserializeFailed:
				DKLog("[MyCustomObject3] DeserializeFailed\n"); break;
			default:
				DKLog("[MyCustomObject3] Unknown Serializer state\n"); break;
			}
		}));

		return ser;
	}
};


struct DocumentTest : public TestCase
{
	void RunTest()
	{
		//DKAllocator& allocator = DKAllocator::DefaultAllocator();
		bool init1 = DKResource::RegisterAllocator(L"", L"MyCustomObject1", DKFunction(DKResourceAlloc<MyCustomObject1>));
		bool init2 = DKResource::RegisterAllocator(L"", L"MyCustomObject2", DKFunction(DKResourceAlloc<MyCustomObject2>));
		bool init3 = DKResource::RegisterAllocator(L"", L"MyCustomObject3", DKFunction(DKResourceAlloc<MyCustomObject3>));

		DKLog("Document test begin!\n");

		DKResourcePool pool;

		DKObject<MyCustomObject1> obj1 = DKObject<MyCustomObject1>::New();
		obj1->a = 111.0;
		obj1->b = 222.0;

		//obj1->SetName(L"testObject1");
		//pool.AddResource(obj1->Name(), obj1);

		//DKObject<MyCustomObject2> obj2 = DKObject<MyCustomObject2>::New();
		//obj2->value1 = 10.0;
		//obj2->value2 = 20.0;
		//obj2->value3 = L"value3";
		//obj2->object1 = obj1;

		DKObject<MyCustomObject3> obj3 = DKObject<MyCustomObject3>::New();
		obj3.SafeCast<MyCustomObject2>()->value1 = 10.0;
		obj3.SafeCast<MyCustomObject2>()->value2 = 20.0;
		obj3.SafeCast<MyCustomObject2>()->value3 = L"value3";
		obj3.SafeCast<MyCustomObject2>()->object1 = obj1;
		obj3->value1 = 100;
		obj3->value2 = 200;

		for (int i = 0; i < 3; ++i)
		{
			DKObject<MyCustomObject1> p = DKObject<MyCustomObject1>::New();
			p->SetName(DKString::Format("Obj1Array_item:%u", i));
			p->a = float(i);
			p->b = float(i) * 10;
			obj3->obj1Array.Add(p);
		}
		for (int i = 0; i < 4; ++i)
		{
			DKObject<MyCustomObject1> p = DKObject<MyCustomObject1>::New();
			p->SetName(DKString::Format("Obj1Map_item:%u", i));
			p->a = float(i) * 100;
			p->b = float(i) * 1000;
			obj3->obj1Map.Insert(i, p);
		}

		DKSerializer::SerializeForm sf[4] = {
			DKSerializer::SerializeFormXML,
			DKSerializer::SerializeFormBinXML,
			DKSerializer::SerializeFormBinary,
			DKSerializer::SerializeFormCompressedBinary
		};
		const char* sfstr[4] = {
			"SerializeFormXML",
			"SerializeFormBinXML",
			"SerializeFormBinary",
			"SerializeFormCompressedBinary"
		};

		// DKSerializer 이용
		for (size_t i = 0; i < 4; ++i)
		{
			DKObject<DKSerializer> serializer = obj3->Serializer();
			if (serializer)
			{
				DKObject<DKData> data = serializer->Serialize(sf[i]);
				if (data)
				{
					DKLog("serializer->Serialize(%s) %llu bytes\n", sfstr[i], (unsigned long long)data->Length());
					obj3.SafeCast<MyCustomObject2>()->value1 = 1.0;
					obj3.SafeCast<MyCustomObject2>()->value2 = 2.0;
					obj3.SafeCast<MyCustomObject2>()->value3 = L"v3";
					obj3->value1 = 10;
					obj3->value2 = 20;
					obj1->a = 1;
					obj1->b = 1;
					obj3->obj1Array.Clear();
					obj3->obj1Map.Clear();
					if (serializer->Deserialize(data, &pool) == false)
						DKLog("serializer->Deserialize failed!\n");
				}
				else
				{
					DKLog("serializer->Serialize(%s) failed.\n", sfstr[i]);
				}
			}
		}
		// Serialize/Deserialize 멤버 함수 이용
		for (size_t i = 0; i < 4; ++i)
		{
			DKObject<DKData> data = obj3->Serialize(sf[i]);
			if (data)
			{
				DKLog("obj3->Serialize(%s) %llu bytes\n", sfstr[i], (unsigned long long)data->Length());
				obj3.SafeCast<MyCustomObject2>()->value1 = 1.0;
				obj3.SafeCast<MyCustomObject2>()->value2 = 2.0;
				obj3.SafeCast<MyCustomObject2>()->value3 = L"v3";
				obj3->value1 = 10;
				obj3->value2 = 20;
				obj1->a = 1;
				obj1->b = 1;
				obj3->obj1Array.Clear();
				obj3->obj1Map.Clear();
				if (obj3->Deserialize(data, &pool) == false)
					DKLog("obj3->Deserialize failed!\n");
			}
			else
			{
				DKLog("obj3->Serialize(%s) failed.\n", sfstr[i]);
			}
		}


		DKLog("obj3->obj1Array has %d items\n", obj3->obj1Array.Count());
		for (int i = 0; i < obj3->obj1Array.Count(); ++i)
		{
			MyCustomObject1* p = obj3->obj1Array.Value(i);
			DKLog("obj3->obj1Array[%d] name:%ls, a:%f, b:%f\n", i, (const wchar_t*)p->Name(), p->a, p->b);
		}
		DKLog("obj3->obj1Map has %d items\n", obj3->obj1Map.Count());
		obj3->obj1Map.EnumerateForward([](const MyCustomObject3::Obj1Map::Pair& pair)
		{
			int key = pair.key;
			const MyCustomObject1* p = pair.value;
			DKLog("obj3->obj1Map[%u] name:%ls, a:%f, b:%f\n", key, (const wchar_t*)p->Name(),  p->a, p->b);
		});


		DKLog("Document test end.\n");

		DKResource::UnregisterAllocator(L"", L"MyCustomObject3");
		DKResource::UnregisterAllocator(L"", L"MyCustomObject2");
		DKResource::UnregisterAllocator(L"", L"MyCustomObject1");
	}
} documentTest;

