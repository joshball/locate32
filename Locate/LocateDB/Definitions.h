/* Copyright (c) 1997-2006 Janne Huttunen
   database updater v2.98.6.8050                */

#if !defined(DBDEFINITIONS_H)
#define DBDEFINITIONS_H

#if _MSC_VER >= 1000
#pragma once
#endif


enum CallingReason {
	Initializing=0,
	ClassShouldDelete=1,
	StartedDatabase=2,
	FinishedDatabase=3,
	FinishedUpdating=4,
	FinishedLocating=4,
	BeginningDatabase=5, 
	ErrorOccured=6,
	RootChanged=7,
	SearchingStarted=8, // Only for CLocater
	SearchingEnded=9, // Only for CLocater
	RootInformationAvail=10 // Only for CLocater
};

enum UpdateError {
	ueSuccess = 0,
	ueError = 1,
	ueCreate = 2,
	ueWrite = 3,
	ueOpen = 4,
	ueRead = 5,
	ueAlloc = 6,
	ueUnknown = 7,
	ueCannotCreateThread = 8,
	ueStopped = 9,
	ueFolderUnavailable = 10,
	ueInvalidDatabase = 11,
	ueLimitReached = 12, // Only for CLocater
	ueCannotIncrement = 13,
	ueWrongCharset = 14,
	ueStillWorking = 0xF000
};

#endif