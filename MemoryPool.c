#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include "MemoryPool.h"

CMemoryPool* CMemoryPool::m_pInstance = NULL;

CMemoryPool::CMemoryPool(unsigned int nSize, unsigned int nNum)
{

	printf("1\n");
	m_pMemoryBlockFree = NULL;
	m_pMemoryBlockUse = NULL;

	m_pMemoryBlockFreeLast = NULL;
	m_pMemoryBlockUseLast = NULL;
	
	m_chError[0] = '\0';
	
	for (unsigned int i = 0; i < nNum; i++)
	{
		CMemoryBlock *pBlock = new CMemoryBlock(nSize);
		
		if (NULL != pBlock)
		{
			if (NULL == m_pMemoryBlockFree)
			{
				
				m_pMemoryBlockFree = pBlock;
				m_pMemoryBlockFreeLast = pBlock;

			}
			else
			{
				pBlock->m_pPre = m_pMemoryBlockFreeLast;
				m_pMemoryBlockFreeLast->m_pNext = pBlock;
				m_pMemoryBlockFreeLast = pBlock;
			}
		}
	}
}

CMemoryPool::~CMemoryPool()
{
	while (NULL != m_pMemoryBlockFree)
	{
		CMemoryBlock *pBlock = m_pMemoryBlockFree;
		m_pMemoryBlockFree = m_pMemoryBlockFree->m_pNext;

		pBlock->Free();
		delete pBlock;
	}

	m_pMemoryBlockFreeLast = NULL;

	//----------------------------------------------------
	while (NULL != m_pMemoryBlockUse)
	{
		CMemoryBlock *pBlock = m_pMemoryBlockUse;
		m_pMemoryBlockUse = m_pMemoryBlockUse->m_pNext;
		
		pBlock->Free();
		delete pBlock;
	}
	m_pMemoryBlockUseLast = NULL;
}

char* CMemoryPool::malloc(unsigned int nSize)
{
	//上锁
	CAutoLock autoLock(&m_ThreadLock);

	//如果已经没有可用了，直接NEW出一个来
	if (NULL == m_pMemoryBlockFree)
	{
		CMemoryBlock *pTmp = new CMemoryBlock(nSize);

		if (pTmp == NULL)
		{
			//如果连类都创建失败
			strcpy(m_chError, "failed to create CMemoryBlock");
			return NULL;
		}
		else if (pTmp->GetSize() < nSize)
		{
			//如果只有类，但是内容不够的话
			pTmp->Free();
			delete pTmp;
			strcpy(m_chError, "failed to create CMemoryBlock's memory");
			return NULL;
		}
		else
		{
			//一切顺序的话
			m_pMemoryBlockUseLast->m_pNext = pTmp;
			m_pMemoryBlockUseLast = pTmp;
			return pTmp->GetMemory();
		}
	}

	/*
		找最理想的原则是这样的：
		1.如果GetSize() - nSize 如果是正数，那比负数优先，因为至少不需要再realloc
		2.如果GetSize() - nSize 如果是正数，之前的也是正数，取小的那个正数有优先，因为可能尽可能利用空间
		2.如果GetSize() - nSize 如果是负数，绝对绝小的那个优先，因为realloc时要memmov的空间要小

	*/
	//最理想的一个内存块
	CMemoryBlock *pPerfect = m_pMemoryBlockFree;

	//一开始取负的最小值
	int nMiniSize = ~0;

	//从可以用的队列中找到一个大小最适合的
	CMemoryBlock *pCur = m_pMemoryBlockFree;

	while (NULL != pCur)
	{
		int nTmp = pCur->GetSize() - (int)nSize;

		if (nMiniSize < 0)
		{
			if (nTmp >= 0)
			{
				nMiniSize = nTmp;
				pPerfect = pCur;
			}
			else if (nMiniSize < nTmp)
			{
				nMiniSize = nTmp;
				pPerfect = pCur;
			}
		}
		else
		{
			if (nTmp >= 0)
			{
				if (nTmp < nMiniSize)
				{
					nMiniSize = nTmp;
					pPerfect = pCur;
				}
			}
		}

		pCur = pCur->m_pNext;
	}


	//判断是否有相要的
	if (pPerfect->GetSize() < nSize)
	{
		if (!pPerfect->Realloc(nSize))
		{
			strcpy(m_chError, "failed to realloc the memory");
			return NULL;
		}
	}

	//Pop出来
	if (pPerfect->m_pNext == NULL && pPerfect->m_pPre == NULL)
	{
		m_pMemoryBlockFree = NULL;
		m_pMemoryBlockFreeLast = NULL;
	}
	else if (pPerfect->m_pNext == NULL)
	{
		m_pMemoryBlockFreeLast = pPerfect->m_pPre;
		m_pMemoryBlockFreeLast->m_pNext = NULL;
	}
	else if (pPerfect->m_pPre == NULL)
	{
		m_pMemoryBlockFree = m_pMemoryBlockFree->m_pNext;
		m_pMemoryBlockFree->m_pPre = NULL;
	}
	else
	{
		(pPerfect->m_pPre)->m_pNext = pPerfect->m_pNext;
		(pPerfect->m_pNext)->m_pPre = pPerfect->m_pPre;
	}

	//PUSH
	if (m_pMemoryBlockUseLast == NULL)
	{
		pPerfect->m_pNext = NULL;
		pPerfect->m_pPre = NULL;
		m_pMemoryBlockUse = pPerfect;
		m_pMemoryBlockUseLast = pPerfect;
	}
	else
	{
		m_pMemoryBlockUseLast->m_pNext = pPerfect;
		pPerfect->m_pNext = NULL;
		pPerfect->m_pPre = m_pMemoryBlockUseLast;
		m_pMemoryBlockUseLast = pPerfect;
	}

	return pPerfect->GetMemory();
}


char* CMemoryPool::realloc(char *pOldPtr, unsigned int nNewSize)
{
	//上锁
	CAutoLock autoLock(&m_ThreadLock);

	CMemoryBlock *pCur = m_pMemoryBlockUse;

	while (NULL == pCur)
	{
		if (pCur->GetMemory() == pOldPtr)
		{
			break;
		}
		pCur = pCur->m_pNext;
	}

	if (NULL == pCur)
	{
		strcpy(m_chError, "error memory address");
		return NULL;
	}

	if (!pCur->Realloc(nNewSize))
	{
		strcpy(m_chError, "failed to realloc");
		return NULL;
	}

	return pCur->GetMemory();
}

bool CMemoryPool::free(char *pPtr)
{
	//上锁
	CAutoLock autoLock(&m_ThreadLock);

	CMemoryBlock *pCur = m_pMemoryBlockUse;
	
	while (NULL == pCur)
	{
		if (pCur->GetMemory() == pPtr)
		{
			break;
		}
		pCur = pCur->m_pNext;
	}
	
	if (NULL == pCur)
	{
		strcpy(m_chError, "error memory address");
		return false;
	}


	//Pop出来
	if (pCur->m_pNext == NULL && pCur->m_pPre == NULL)
	{
		m_pMemoryBlockUse = NULL;
		m_pMemoryBlockUseLast = NULL;
	}
	else if (pCur->m_pNext == NULL)
	{
		m_pMemoryBlockUseLast = pCur->m_pPre;
		m_pMemoryBlockUseLast->m_pNext = NULL;
	}
	else if (pCur->m_pPre == NULL)
	{
		m_pMemoryBlockUse = m_pMemoryBlockUse->m_pNext;
		m_pMemoryBlockUse->m_pPre = NULL;
	}
	else
	{
		(pCur->m_pPre)->m_pNext = pCur->m_pNext;
		(pCur->m_pNext)->m_pPre = pCur->m_pPre;
	}
	
	//PUSH
	if (m_pMemoryBlockFreeLast == NULL)
	{
		pCur->m_pNext = NULL;
		pCur->m_pPre = NULL;
		m_pMemoryBlockFree = pCur;
		m_pMemoryBlockFreeLast = pCur;
	}
	else
	{
		m_pMemoryBlockFreeLast->m_pNext = pCur;
		pCur->m_pNext = NULL;
		pCur->m_pPre = m_pMemoryBlockFreeLast;
		m_pMemoryBlockFreeLast = pCur;
	}

	return true;
}

//free all memory
void CMemoryPool::destory()
{
	//上锁
	CAutoLock autoLock(&m_ThreadLock);

	delete this;
}

//if failed to malloc, realloc, free, return the error msg;
char* CMemoryPool::errormsg()
{
	//上锁
	CAutoLock autoLock(&m_ThreadLock);

	return m_chError;
}

CMemoryPool* CMemoryPool::GetInstance(unsigned int nSize, unsigned int nNum)
{
	if (m_pInstance == NULL)
	{
		m_pInstance = new CMemoryPool(nSize, nNum);
	}
	
	return m_pInstance;
}


IMemoryPool * CreateMemoryPool(unsigned int nSize, unsigned int nNum)
{
	return CMemoryPool::GetInstance(nSize, nNum);
}









