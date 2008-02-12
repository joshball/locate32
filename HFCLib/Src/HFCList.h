////////////////////////////////////////////////////////////////////
// HFC Library - Copyright (C) 1999-2008 Janne Huttunen
////////////////////////////////////////////////////////////////////


#ifndef HFCLIST_H
#define HFCLIST_H

template<class TYPE>
class CList : public CExceptionObject
{
protected:
	struct CNode
	{
		CNode* pNext;
		CNode* pPrev;
		TYPE data;
	};
public:
	CList();
	~CList();

	int GetCount() const;
	int GetSizeOfType() const { return sizeof(TYPE); }

	BOOL IsEmpty() const;
	
	TYPE& GetHead();
	TYPE GetHead() const;
	TYPE& GetTail();
	TYPE GetTail() const;

	TYPE RemoveHead();
	TYPE RemoveTail();

	POSITION AddHead(TYPE newElement);
	POSITION AddTail(TYPE newElement);

	void AddHead(CList* pNewList);
	void AddTail(CList* pNewList);

	void RemoveAll();

	POSITION GetHeadPosition() const;
	POSITION GetTailPosition() const;
	static POSITION GetNextPosition(POSITION& rPosition);
	static POSITION GetPrevPosition(POSITION& rPosition);

	TYPE& GetNext(POSITION& rPosition); 
	TYPE GetNext(POSITION& rPosition) const;
	TYPE& GetPrev(POSITION& rPosition); 
	TYPE GetPrev(POSITION& rPosition) const; 

	static TYPE& GetAtRef(POSITION position);
	static TYPE* GetAtPtr(POSITION position);
	static TYPE GetAt(POSITION position);
	static void SetAt(POSITION pos,TYPE newElement);
	void RemoveAt(POSITION position);

	POSITION InsertBefore(POSITION position,TYPE newElement);
	POSITION InsertAfter(POSITION position,TYPE newElement);

	POSITION Find(TYPE searchValue,POSITION startAfter=NULL) const;
	POSITION FindIndex(int nIndex) const;
	
	void Swap(CList<TYPE>& lst);

protected:
	CNode* m_pNodeHead;
	CNode* m_pNodeTail;
	int m_nCount;
};

#ifdef DEF_RESOURCES
class CMDIChildWndList
{
protected:
	struct CNode
	{
		CNode* pNext;
		CNode* pPrev;
		CMDIChildWnd* pWnd;
	};
public:
	CMDIChildWndList();
	~CMDIChildWndList();
	
	int GetCount() const { return m_nCount; }
	BOOL IsEmpty() const { return (m_pNodeHead==NULL); }
	
	POSITION AddWindow(CMDIChildWnd* pWnd);
	BYTE RemoveWindow(POSITION pPos);
	BYTE RemoveAllWindows();
	static CMDIChildWnd* GetWindow(POSITION pPos) { return ((CNode*)pPos)->pWnd; }
	
	POSITION FindWindow(CMDIChildWnd* pWnd) const;
	POSITION FindWindow(HWND hWnd) const;

	POSITION GetFirstPosition() const { return (POSITION)m_pNodeHead; }
	POSITION GetLastPosition() const { return (POSITION)m_pNodeTail; }
	static POSITION GetNextPosition(POSITION pPos) { return (POSITION)(((CNode*)pPos)->pNext); }
	static POSITION GetPrevPosition(POSITION pPos) { return (POSITION)(((CNode*)pPos)->pPrev); }

protected:
	CNode* m_pNodeHead;
	CNode* m_pNodeTail;
	int m_nCount;
};
#endif

template<class TYPE>
CList<TYPE>::CList()
{
	m_pNodeHead=NULL;
	m_pNodeTail=NULL;
	m_nCount=0;
}

template<class TYPE>
CList<TYPE>::~CList()
{
	RemoveAll();
}

template<class TYPE>
inline int CList<TYPE>::GetCount() const
{ return m_nCount; }

template<class TYPE>
inline BOOL CList<TYPE>::IsEmpty() const
{ return (m_pNodeHead==NULL); }

template<class TYPE>
TYPE& CList<TYPE>::GetHead()
{ 
	if (m_pNodeHead!=NULL)
		return m_pNodeHead->data;
	else
	{
		TYPE *temp=NULL;
		return *temp;
	}
}

template<class TYPE>
TYPE CList<TYPE>::GetHead() const
{ 
	if (m_pNodeHead!=NULL)
		return m_pNodeHead->data;
	else
		return 0;
}

template<class TYPE>
TYPE& CList<TYPE>::GetTail()
{
	if (m_pNodeTail!=NULL)
		return m_pNodeTail->data;
	else
	{
		TYPE *temp=NULL;
		return *temp;
	}
}

template<class TYPE>
TYPE CList<TYPE>::GetTail() const
{
	if (m_pNodeTail!=NULL)
		return m_pNodeTail->data;
	else
		return 0;
}

template<class TYPE>
TYPE CList<TYPE>::RemoveHead()
{
	if (m_pNodeHead!=NULL)
	{
		TYPE ret;
		m_nCount--;
		if (m_pNodeHead==m_pNodeTail)
		{
			ret=m_pNodeHead->data;
			delete m_pNodeHead;
			m_pNodeHead=m_pNodeTail=NULL;
			return ret;
		}
		CNode* tmp=m_pNodeHead;
		m_pNodeHead=m_pNodeHead->pNext;
		m_pNodeHead->pPrev=NULL;
		ret=tmp->data;
		delete tmp;
		return ret;
	}
	return 0;
}

template<class TYPE>
TYPE CList<TYPE>::RemoveTail()
{
	if (m_pNodeHead!=NULL)
	{
		TYPE ret;
		m_nCount--;
		if (m_pNodeHead==m_pNodeTail)
		{
			ret=m_pNodeHead->data;
			delete m_pNodeHead;
			m_pNodeHead=m_pNodeTail=NULL;
			return ret;
		}
		CNode* tmp=m_pNodeTail;
		m_pNodeTail=m_pNodeTail->pPrev;
		m_pNodeTail->pNext=NULL;
		ret=tmp->data;
		delete tmp;
		return ret;
	}
	return 0;
}


template<class TYPE>
POSITION CList<TYPE>::AddHead(TYPE newElement)
{
	CNode* tmp=new CNode;
	if (tmp==NULL)
	{
		if (m_bThrow)
			throw CException(CException::cannotAllocate);
		else
			SetHFCError(HFC_CANNOTALLOC);
		return NULL;
	}
	tmp->data=newElement;
	tmp->pPrev=NULL;
	tmp->pNext=m_pNodeHead;
	if (m_pNodeHead!=NULL)
		m_pNodeHead->pPrev=tmp;
	else
		m_pNodeTail=tmp;
	m_pNodeHead=tmp;
	m_nCount++;
	return (POSITION) tmp;
}

template<class TYPE>
POSITION CList<TYPE>::AddTail(TYPE newElement)
{
	CNode* tmp=new CNode;
	if (tmp==NULL)
	{
		if (m_bThrow)
			throw CException(CException::cannotAllocate);
		else
			SetHFCError(HFC_CANNOTALLOC);
		return NULL;
	}
	tmp->data=newElement;
	tmp->pNext=NULL;
	tmp->pPrev=m_pNodeTail;
	if (m_pNodeTail!=NULL)
		m_pNodeTail->pNext=tmp;
	else
		m_pNodeHead=tmp;
	m_pNodeTail=tmp;
	m_nCount++;
	return (POSITION) tmp;
}

template<class TYPE>
void CList<TYPE>::AddHead(CList* pNewList)
{
	if (pNewList==NULL)
		return;
	if (pNewList.m_pNodeHead==NULL)
		return;	
	if (m_pNodeHead==NULL)
	{
		m_pNodeHead=pNewList->m_pNodeHead;
		m_pNodeTail=pNewList->m_pNodeTail;
		m_nCount=pNewList->m_nCount;
		return;
	}
	pNewList->m_pNodeTail->pNext=m_pNodeHead;
	m_pNodeHead->pPrev=pNewList->m_pNodeTail;
	m_pNodeHead=pNewList;
	m_nCount+=pNewList->m_nCount;	
}

template<class TYPE>
void CList<TYPE>::AddTail(CList* pNewList)
{
	if (pNewList==NULL)
		return;
	if (pNewList.m_pNodeHead==NULL)
		return;	
	if (m_pNodeHead==NULL)
	{
		m_pNodeHead=pNewList->m_pNodeHead;
		m_pNodeTail=pNewList->m_pNodeTail;
		m_nCount=pNewList->m_nCount;
		return;
	}
	pNewList->m_pNodeHead->pPrev=m_pNodeTail;
	m_pNodeTail->pNext=pNewList->m_pNodeHead;
	m_pNodeTail=pNewList->m_pNodeTail;
	m_nCount+=pNewList->m_nCount;
}

template<class TYPE>
void CList<TYPE>::RemoveAll()
{
	CNode* tmp;
	while(m_pNodeHead!=NULL)
	{
		tmp=m_pNodeHead;
		m_pNodeHead=tmp->pNext;
		delete tmp;
	}
	m_nCount=0;
	m_pNodeTail=NULL;
}

template<class TYPE>
inline POSITION CList<TYPE>::GetHeadPosition() const
{ return (POSITION) m_pNodeHead; }

template<class TYPE>
inline POSITION CList<TYPE>::GetTailPosition() const
{ return (POSITION) m_pNodeTail; }

template<class TYPE>
inline POSITION CList<TYPE>::GetNextPosition(POSITION& rPosition)
{   return (POSITION)(((CNode*)rPosition)->pNext); }

template<class TYPE>
inline POSITION CList<TYPE>::GetPrevPosition(POSITION& rPosition)
{   return (POSITION)(((CNode*)rPosition)->pPrev); }

template<class TYPE>
TYPE& CList<TYPE>::GetNext(POSITION& rPosition)
{
	rPosition=(POSITION)(((CNode*)rPosition)->pNext);
	TYPE temp=0;
	return (rPosition==NULL?temp:((CNode*)rPosition)->data);
}

template<class TYPE>
TYPE CList<TYPE>::GetNext(POSITION& rPosition) const
{
	rPosition=(POSITION)(((CNode*)rPosition)->pNext);
	return (rPosition==NULL?NULL:((CNode*)rPosition)->data);
}

template<class TYPE>
TYPE& CList<TYPE>::GetPrev(POSITION& rPosition)
{
	rPosition=(POSITION)(((CNode*)rPosition)->pPrev);
	TYPE temp=0;
	return (rPosition==NULL?temp:((CNode*)rPosition)->data);
}

template<class TYPE>
TYPE CList<TYPE>::GetPrev(POSITION& rPosition) const
{
	rPosition=(POSITION)(((CNode*)rPosition)->pPrev);
	return (rPosition==NULL?NULL:((CNode*)rPosition)->data);
}

template<class TYPE>
inline TYPE& CList<TYPE>::GetAtRef(POSITION position)
{ return ((CNode*)position)->data; }

template<class TYPE>
inline TYPE* CList<TYPE>::GetAtPtr(POSITION position)
{ return &((CNode*)position)->data; }

template<class TYPE>
inline TYPE CList<TYPE>::GetAt(POSITION position)
{ return ((CNode*)position)->data; }

template<class TYPE>
inline void CList<TYPE>::SetAt(POSITION pos,TYPE newElement)
{ ((CNode*)pos)->data=newElement; }

template<class TYPE>
void CList<TYPE>::RemoveAt(POSITION position)
{
	CNode* node=(CNode*)position;
	if (node->pNext!=NULL)
	{
		if (node->pPrev!=NULL)
		{
			node->pPrev->pNext=node->pNext;
			node->pNext->pPrev=node->pPrev;
		}
		else
		{
			m_pNodeHead=node->pNext;	
			node->pNext->pPrev=NULL;
		}
	}
	else
	{
		if (node->pPrev!=NULL)
		{
			m_pNodeTail=node->pPrev;
			node->pPrev->pNext=NULL;
		}
		else
		{
			m_pNodeHead=NULL;
			m_pNodeTail=NULL;
		}
	}
	m_nCount--;
	delete node;
}

template<class TYPE>
POSITION CList<TYPE>::InsertBefore(POSITION position,TYPE newElement)
{
	CNode* tmp=new CNode;
	if (tmp==NULL)
	{
		if (m_bThrow)
			throw CException(CException::cannotAllocate);
		else
			SetHFCError(HFC_CANNOTALLOC);
		return NULL;
	}
	tmp->data=newElement;
	tmp->pPrev=((CNode*)position)->pPrev;
	if (tmp->pPrev!=NULL)
		tmp->pPrev->pNext=tmp;
	else
		m_pNodeHead=tmp;
	tmp->pNext=((CNode*)position);
	((CNode*)position)->pPrev=tmp;
	m_nCount++;
	return (POSITION) tmp;
}

template<class TYPE>
POSITION CList<TYPE>::InsertAfter(POSITION position,TYPE newElement)
{
	CNode* tmp=new CNode;
	if (tmp==NULL)
	{
		if (m_bThrow)
			throw CException(CException::cannotAllocate);
		else
			SetHFCError(HFC_CANNOTALLOC);
		return NULL;
	}
	tmp->data=newElement;
	tmp->pNext=((CNode*)position)->pNext;
	if (tmp->pNext!=NULL)
		tmp->pNext->pPrev=tmp;
	else
		m_pNodeTail=tmp;
	tmp->pPrev=((CNode*)position);
	((CNode*)position)->pNext=tmp;
	m_nCount++;
	return (POSITION) tmp;
}

template<class TYPE>
POSITION CList<TYPE>::Find(TYPE searchValue,POSITION startAfter) const
{
	CNode* tmp;
	if ((CNode*)startAfter==NULL)
		tmp=m_pNodeHead;
	else
		tmp=((CNode*)startAfter)->pNext;
	while (tmp!=NULL)
	{
		if (tmp->data==searchValue)
			return (POSITION) tmp;
		tmp=tmp->pNext;
	}
	return NULL;
}

template<class TYPE>
POSITION CList<TYPE>::FindIndex(int nIndex) const
{
	if (nIndex>=m_nCount)
		return NULL;
	CNode *ptr=m_pNodeHead;
	for (int i=0;i<nIndex;i++)
		ptr=ptr->pNext;
	return (POSITION) ptr;
}

template<class TYPE>
void CList<TYPE>::Swap(CList<TYPE>& lst)
{
	union {
		CNode* pTmp;
		int nTmp;
	} tmp;
	tmp.pTmp=m_pNodeHead;
	m_pNodeHead=lst.m_pNodeHead;
	lst.m_pNodeHead=tmp.pTmp;

	tmp.pTmp=m_pNodeTail;
	m_pNodeTail=lst.m_pNodeTail;
	lst.m_pNodeTail=tmp.pTmp;

	tmp.nTmp=m_nCount;
	m_nCount=lst.m_nCount;
	lst.m_nCount=tmp.nTmp;
}

template<class TYPE>
class CListFP : public CList<TYPE> // Uses delete to free pointers
{
public:
	~CListFP();

	void RemoveHead();
	void RemoveTail();

	void RemoveAll();
	void RemoveAt(POSITION position);

};

template<class TYPE>
CListFP<TYPE>::~CListFP()
{
	RemoveAll();
}

template<class TYPE>
void CListFP<TYPE>::RemoveHead()
{
	if (m_pNodeHead!=NULL)
	{
		m_nCount--;
		if (m_pNodeHead==m_pNodeTail)
		{
			delete m_pNodeHead->data;
			delete m_pNodeHead;
			m_pNodeHead=m_pNodeTail=NULL;
			return;
		}
		CNode* tmp=m_pNodeHead;
		m_pNodeHead=m_pNodeHead->pNext;
		m_pNodeHead->pPrev=NULL;
		delete tmp->data;
		delete tmp;
	}
}

template<class TYPE>
void CListFP<TYPE>::RemoveTail()
{
	if (m_pNodeHead!=NULL)
	{
		m_nCount--;
		if (m_pNodeHead==m_pNodeTail)
		{
			delete m_pNodeHead->data;
			delete m_pNodeHead;
			m_pNodeHead=m_pNodeTail=NULL;
			return;
		}
		CNode* tmp=m_pNodeTail;
		m_pNodeTail=m_pNodeTail->pPrev;
		m_pNodeTail->pNext=NULL;
		delete tmp->data;
		delete tmp;
	}
	return 0;
}

template<class TYPE>
void CListFP<TYPE>::RemoveAll()
{
	CNode* tmp;
	while(m_pNodeHead!=NULL)
	{
		tmp=m_pNodeHead;
		m_pNodeHead=tmp->pNext;
		delete tmp->data;
		delete tmp;
	}
	m_nCount=0;
	m_pNodeTail=NULL;
}


template<class TYPE>
void CListFP<TYPE>::RemoveAt(POSITION position)
{
	CNode* node=(CNode*)position;
	if (node->pNext!=NULL)
	{
		if (node->pPrev!=NULL)
		{
			node->pPrev->pNext=node->pNext;
			node->pNext->pPrev=node->pPrev;
		}
		else
		{
			m_pNodeHead=node->pNext;	
			node->pNext->pPrev=NULL;
		}
	}
	else
	{
		if (node->pPrev!=NULL)
		{
			m_pNodeTail=node->pPrev;
			node->pPrev->pNext=NULL;
		}
		else
		{
			m_pNodeHead=NULL;
			m_pNodeTail=NULL;
		}
	}
	m_nCount--;
	delete node->data;
	delete node;
}

template<class TYPE>
class CListFAP : public CList<TYPE> // Uses delete[] to free pointers
{
public:
	~CListFAP();

	void RemoveHead();
	void RemoveTail();

	void RemoveAll();
	void RemoveAt(POSITION position);

};

template<class TYPE>
CListFAP<TYPE>::~CListFAP()
{
	RemoveAll();
}

template<class TYPE>
void CListFAP<TYPE>::RemoveHead()
{
	if (m_pNodeHead!=NULL)
	{
		m_nCount--;
		if (m_pNodeHead==m_pNodeTail)
		{
			delete[] m_pNodeHead->data;
			delete m_pNodeHead;
			m_pNodeHead=m_pNodeTail=NULL;
			return;
		}
		CNode* tmp=m_pNodeHead;
		m_pNodeHead=m_pNodeHead->pNext;
		m_pNodeHead->pPrev=NULL;
		delete[] tmp->data;
		delete tmp;
	}
}

template<class TYPE>
void CListFAP<TYPE>::RemoveTail()
{
	if (m_pNodeHead!=NULL)
	{
		m_nCount--;
		if (m_pNodeHead==m_pNodeTail)
		{
			delete[] m_pNodeHead->data;
			delete m_pNodeHead;
			m_pNodeHead=m_pNodeTail=NULL;
			return;
		}
		CNode* tmp=m_pNodeTail;
		m_pNodeTail=m_pNodeTail->pPrev;
		m_pNodeTail->pNext=NULL;
		delete[] tmp->data;
		delete tmp;
	}
	return 0;
}

template<class TYPE>
void CListFAP<TYPE>::RemoveAll()
{
	CNode* tmp;
	while(m_pNodeHead!=NULL)
	{
		tmp=m_pNodeHead;
		m_pNodeHead=tmp->pNext;
		delete[] tmp->data;
		delete tmp;
	}
	m_nCount=0;
	m_pNodeTail=NULL;
}


template<class TYPE>
void CListFAP<TYPE>::RemoveAt(POSITION position)
{
	CNode* node=(CNode*)position;
	if (node->pNext!=NULL)
	{
		if (node->pPrev!=NULL)
		{
			node->pPrev->pNext=node->pNext;
			node->pNext->pPrev=node->pPrev;
		}
		else
		{
			m_pNodeHead=node->pNext;	
			node->pNext->pPrev=NULL;
		}
	}
	else
	{
		if (node->pPrev!=NULL)
		{
			m_pNodeTail=node->pPrev;
			node->pPrev->pNext=NULL;
		}
		else
		{
			m_pNodeHead=NULL;
			m_pNodeTail=NULL;
		}
	}
	m_nCount--;
	delete[] node->data;
	delete node;
}

class CObList			: public CList <CObject> {};
class CStringList		: public CList <CString> {};

#endif

