/* Copyright (c) 1997-2005 Janne Huttunen
   database updater v2.99.5.10100                */

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