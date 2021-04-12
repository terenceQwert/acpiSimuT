#ifdef __cplusplus
extern "C"
{
#endif
#include <wdm.h>
#ifdef __cplusplus
}
#endif

VOID * __cdecl operator new(size_t size, POOL_TYPE PoolType = PagedPool)
{
  KdPrint(("global operator new \n"));
  KdPrint(("Allocate sizes:%d\n", size));
  return ExAllocatePoolWithTag(PoolType, size, '1111');
}

VOID __cdecl operator delete(void*pointer)
{
  KdPrint(("Global delete operator\n"));
  ExFreePoolWithTag(pointer, '1111');
}

class TestClass
{
public:
  TestClass() {
    KdPrint(("TestClass:TestClass\n"));

  }
  ~TestClass()
  {
    KdPrint(("TestClass::~TestClass\n"));
  }

  VOID * operator new(size_t size, POOL_TYPE PoolType = PagedPool)
  {
    KdPrint(("TestClass::new \n"));
    KdPrint(("Allocate size :%d\n", size));
    return ExAllocatePoolWithTag(PoolType, size, '1111');
  }

  VOID operator delete(void*pointer)
  {
    KdPrint(("TestClass::delete\n"));
    ExFreePoolWithTag(pointer, '1111');
  }

private:
  char buffer[1024];
};

void TestNewOperator()
{
#if 0
  TestClass * pTestClass = new TestClass();
  delete pTestClass;

  pTestClass = new (NonPagedPool)TestClass();
  delete pTestClass;

  char *pBuffer = new(PagedPool)char[100];
  delete[]pBuffer;
  pBuffer = new (NonPagedPool)char[100];
  delete[]pBuffer;
#endif
}