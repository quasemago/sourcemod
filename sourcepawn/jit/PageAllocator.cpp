#include <stdlib.h>
#include "PageAllocator.h"
#include <sm_platform.h>

using namespace SourcePawn;

PageAllocator::PageAllocator() : m_pFreePages(NULL), m_NumBlocks(0), m_Blocks(NULL)
{
#if defined PLATFORM_WINDOWS
	SYSTEM_INFO info;
	GetSystemInfo(&info);
	m_PageSize = info.dwPageSize;
	m_PageGranularity = info.dwAllocationGranularity;
#else
#error oh
#endif
}

PageAllocator::~PageAllocator()
{
	if (m_NumBlocks)
	{
		for (unsigned int i = 0; i < m_NumBlocks; i++)
		{
			VirtualFree(m_Blocks[i], 0, MEM_RELEASE);
		}
		free(m_Blocks);
	}
	else
	{
		Page *next;

		while (m_pFreePages != NULL)
		{
			next = m_pFreePages->next;
			VirtualFree(m_pFreePages, 0, MEM_RELEASE);
			m_pFreePages = next;
		}
	}
}

Page *PageAllocator::AllocPage()
{
	/* Check if we need a new page */
	if (m_pFreePages != NULL)
	{
		Page *p;

		p = m_pFreePages;
		m_pFreePages = m_pFreePages->next;
		p->next = NULL;

		return p;
	}
	else
	{
		char *p;
		Page *pPage, *pLast;
		
		pLast = NULL;
		p = (char *)VirtualAlloc(NULL, m_PageGranularity, MEM_COMMIT|MEM_RESERVE, PAGE_EXECUTE_READWRITE);

		if (m_PageSize != m_PageGranularity)
		{
			m_Blocks = (void **)realloc(m_Blocks, sizeof(void *) * (m_NumBlocks + 1));
			m_Blocks[m_NumBlocks] = p;
			m_NumBlocks++;
		}

		for (unsigned int i = 1; i <= (m_PageGranularity / m_PageSize); i++)
		{
			pPage = (Page *)p;
			pPage->data = (uint8_t *)p + sizeof(Page);
			pPage->next = pLast;
			pLast = pPage;
			p += m_PageSize;
		}

		m_pFreePages = pLast->next;
		pLast->next = NULL;

		return pLast;
	}
}

void PageAllocator::FreePage(Page *p)
{
	p->next = m_pFreePages;
	m_pFreePages = p;
}

unsigned int PageAllocator::GetPageSize()
{
	return m_PageSize;
}
