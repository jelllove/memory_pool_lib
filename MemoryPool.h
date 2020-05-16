#ifndef MEM_POOL_H_FILE_INCLUDE
#define MEM_POOL_H_FILE_INCLUDE

#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <stdlib.h>
#include "IMemoryPool.h"
#include "MemoryBlock.h"
#include "ThreadLock.h"


class CMemoryPool : public IMemoryPool
{
public:
	CMemoryPool(unsigned int nSize, unsigned int nNum);

	~CMemoryPool();

	static CMemoryPool* GetInstance(unsigned int nSize, unsigned int nNum);

	virtual char* malloc(unsigned int nSize);
	virtual char* realloc(char *pOldPtr, unsigned int nNewSize);
	virtual bool free(char *pPtr);
	
	//free all memory
	virtual void destory();
	
	//if failed to malloc, realloc, free, return the error msg;
	virtual char* errormsg();


private:

	static CMemoryPool* m_pInstance;

	//���ö���
	CMemoryBlock *m_pMemoryBlockFree;
	CMemoryBlock *m_pMemoryBlockFreeLast;

	//�����ö���
	CMemoryBlock *m_pMemoryBlockUse;
	CMemoryBlock *m_pMemoryBlockUseLast;

	//������Ϣ
	char m_chError[256];

	//�߳���
	CThreadLock m_ThreadLock;
};

#endif


