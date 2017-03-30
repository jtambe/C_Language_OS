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
		// checking if process is active
		if(q[proctmp].active)
		{
			int minTimestamp = INT_MAX;
			pc = q[proctmp].pc; // get program counter for process
			page = pc/PAGESIZE; // page for program counter
			int nextpage = ((page+1)%20);

			timestamps[proctmp][page] = tick;
			timestamps[proctmp][nextpage] = tick;


			int pageinResult = pagein(proctmp,page);
			if((!q[proctmp].pages[page]) && (pageinResult == 0))
			{
				for(pagetmp = 0; pagetmp < q[proctmp].npages; ++pagetmp)
				{
					if(q[proctmp].pages[pagetmp] == 1) // if allocated
					{
						// get the lowest timestamp to identify least used page
						if(timestamps[proctmp][pagetmp] < minTimestamp)
						{
							oldpage = pagetmp;
							minTimestamp = timestamps[proctmp][pagetmp];
						}
					}
				}
				if(oldpage != page ) // if oldpage is not needed
				{
					pageout(proctmp,oldpage); // remove oldpage
				}
			}

			int nextpageinResult = pagein(proctmp,nextpage);
			minTimestamp = INT_MAX;
			//{nextpageinResult ? printf("true \n"):printf("false \n"); }
			if( (!q[proctmp].pages[nextpage]) && (nextpageinResult == 0))
			{
				for(pagetmp = 0; pagetmp < q[proctmp].npages; ++pagetmp)
				{
					if(q[proctmp].pages[pagetmp] == 1) // if allocated
					{
						if(timestamps[proctmp][pagetmp] < minTimestamp)
						{
							oldpage = pagetmp;
							minTimestamp = timestamps[proctmp][pagetmp];
						}
					}
				}

				if(oldpage != nextpage) // if oldpage is not needed
				{
					pageout(proctmp,oldpage); // remove oldpage
				}
			}



		}
	}

	/* advance time for next pageit iteration */
	tick++;
}
