/* Copyright (c) 1997-2006 Janne Huttunen
   database updater v2.99.6.8050                */

#if !defined(DATABASEMODIFIER_H)
#define DATABASEMODIFIER_H

#if _MSC_VER >= 1000
#pragma once
#endif


class CDatabaseModifiers
{
public:
	enum ActionType{
		Update,
		Delete,
		NewDirectory,
		NewFile
	};

	struct ACTION {
		ActionType nType;
		WCHAR* pPath;

	};
    			

public:
	CDatabaseModifiers();
	~CDatabaseModifiers();
};



#endif