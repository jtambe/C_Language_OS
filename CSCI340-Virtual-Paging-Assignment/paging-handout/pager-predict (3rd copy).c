/*
 * File: pager-predict.c
 * Author:       Andy Sayler
 *               http://www.andysayler.com
 * Adopted From: Dr. Alva Couch
 *               http://www.cs.tufts.edu/~couch/
 *
 * Project: CSCI 3753 Programming Assignment 4
 * Create Date: Unknown
 * Modify Date: 2012/04/03
 * Description:
 * 	This file contains a predictive pageit
 *      implmentation.
 */

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include "simulator.h"

void pageit(Pentry q[MAXPROCESSES]) {

	/* This file contains the stub for a predictive pager */
	/* You may need to add/remove/modify any part of this file */

	/* Static vars */
	static int initialized = 0;
	static int tick = 1; // artificial time
	static int timestamps[MAXPROCESSES][MAXPROCPAGES];

	/* Local vars */
	int proctmp;
	int pagetmp;
	int oldpage; // Least recently used page to evict
	int pc, page; // used for pc and program counter page

	/* initialize static vars on first run */
	if (!initialized) {
		/* Init complex static vars here */

		initialized = 1;
	}

	/* TODO: Implement Predictive Paging */
	//fprintf(stderr, "pager-predict not yet implemented. Exiting...\n");
	//exit(EXIT_FAILURE);
	for(proctmp = 0 ; proctmp < MAXPROCESSES ; ++proctmp)
	{
		if(!q[proctmp].active)
		{
			for(pagetmp = 0; pagetmp < q[proctmp].npages; ++pagetmp)
			{
				pageout(proctmp,pagetmp); // remove not needed pages
			}
		}
	}

	for(proctmp = 0 ; proctmp < MAXPROCESSES ; ++proctmp)
	{
		if(q[proctmp].active)
		{
			pc = q[proctmp].pc; // get program counter for process
			page = pc/PAGESIZE; // page for program counter
			int nextpage = ((page+1)%20);

			for(pagetmp = 0; pagetmp < q[proctmp].npages; ++pagetmp)
			{
				if(page == 3)
				{
					if(pagetmp != page || pagetmp != 0 || pagetmp != 10 || pagetmp != 11 || pagetmp != nextpage )
					{
						pageout(proctmp,pagetmp); // remove not needed pages
					}
				}
				if(page == 2)
				{
					if(pagetmp != page || pagetmp != 10  || pagetmp != nextpage )
					{
						pageout(proctmp,pagetmp); // remove not needed pages
					}
				}
				if(page == 8)
				{
					if(pagetmp != page || pagetmp != 0 || pagetmp != nextpage )
					{
						pageout(proctmp,pagetmp); // remove not needed pages
					}
				}
				if(page == 10)
				{
					if(pagetmp != page || pagetmp != 0 || pagetmp != nextpage )
					{
						pageout(proctmp,pagetmp); // remove not needed pages
					}
				}
				if(page == 11)
				{
					if(pagetmp != page || pagetmp != 0 || pagetmp != nextpage )
					{
						pageout(proctmp,pagetmp); // remove not needed pages
					}
				}
				if(page == 12)
				{
					if(pagetmp != page || pagetmp != 9 || pagetmp != 0 || pagetmp != nextpage )
					{
						pageout(proctmp,pagetmp); // remove not needed pages
					}
				}
				if(page == 13)
				{
					if(pagetmp != page || pagetmp != 9 || pagetmp != 0 ||pagetmp != nextpage )
					{
						pageout(proctmp,pagetmp); // remove not needed pages
					}
				}
				if(page == 14)
				{
					if(pagetmp != page || pagetmp != 0 ||pagetmp != nextpage )
					{
						pageout(proctmp,pagetmp); // remove not needed pages
					}
				}

			}
		}
	}


	for(proctmp = 0 ; proctmp < MAXPROCESSES ; ++proctmp)
	{
		// checking if process is active
		if(q[proctmp].active)
		{
			int minTimestamp = INT_MAX;
			pc = q[proctmp].pc; // get program counter for process
			page = pc/PAGESIZE; // page for program counter
			int nextpage = ((page+1)%20);

			int pageinResult = pagein(proctmp,page);
			int nextpageinResult = pagein(proctmp,nextpage);

			if(page == 3)
			{
				pagein(proctmp,0);
				pagein(proctmp,10);
				pagein(proctmp,11);
			}
			if(page == 2)
			{
				pagein(proctmp,10);
			}
			if(page == 8)
			{
				pagein(proctmp,0);
			}
			if(page == 10)
			{
				pagein(proctmp,0);
			}
			if(page == 11)
			{
				pagein(proctmp,10);
			}
			if(page == 12)
			{
				pagein(proctmp,9);
				pagein(proctmp,0);
			}
			if(page == 13)
			{
				pagein(proctmp,9);
				pagein(proctmp,0);
			}
			if(page == 14)
			{
				pagein(proctmp,9);
			}



		}
	}

	/* advance time for next pageit iteration */
	tick++;
}
