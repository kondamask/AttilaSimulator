/**************************************************************************
 *
 * Copyright (c) 2002 - 2011 by Computer Architecture Department,
 * Universitat Politecnica de Catalunya.
 * All rights reserved.
 *
 * The contents of this file may not be disclosed to third parties,
 * copied or duplicated in any form, in whole or in part, without the
 * prior permission of the authors, Computer Architecture Department
 * and Universitat Politecnica de Catalunya.
 *
 * $RCSfile: FetchCache64.h,v $
 * $Revision: 1.7 $
 * $Author: vmoya $
 * $Date: 2006-06-12 08:07:58 $
 *
 * Fetch Cache class definition file.  Version for 64 bit addresses.
 *
 */

 /**
  *
  *  @file FetchCache64.h
  *
  *  Defines the Fetch Cache class.  This class defines a type of
  *  cache supporting fetch access to request cache lines before
  *  using them to memory.  Version for 64 bit addresses.
  *
  */


#ifndef _FETCHCACHE64_

#define _FETCHCACHE64_

#include "GPUTypes.h"
#include "Cache64.h"
#include "StatisticsManager.h"
#include "DynamicObject.h"

namespace gpu3d
{

	/**
	 *
	 *  Defines an entry of the memory request queue.
	 *
	 */
	struct Cache64Request
	{
		u64bit inAddress;   /**<  Address of the first byte of the line requested into the cache.  */
		u64bit outAddress;  /**<  Address of the first byte of the line requested out of the cache.  */
		u32bit line;        /**<  Fetch cache line for the line.  */
		u32bit way;         /**<  Fetch cache way for the line.  */
		bool spill;         /**<  If the line is to be written back to memory.  */
		bool fill;          /**<  If the line is going to be read from memory.  */
		bool masked;        /**<  If it will use a masked write.  */
		bool free;          /**<  If the entry is free.  */
		DynamicObject *source;  /**<  Pointer to a Dynamic Object that is the source of the cache request.  */
	};


	/**
	 *
	 *  This class describes and implements the behaviour of a cache
	 *  that admits to fetch (or request) cache lines before using
	 *  them.  Version for 64 bit addresses.
	 *
	 *  The fetch cache can be used for caches that can be benefited
	 *  from this fetch feature.  A texture cache, a Z cache and a
	 *  color cache are among those where this kind of cache would
	 *  be useful.
	 *
	 *  This class inherits from the basic Cache class.
	 *
	 */

	class FetchCache64 : private Cache64
	{

	private:

		/*  Constants.  */
		enum {
			MAX_LRU = 4     /**<  Defines the number of accesses remembered by the pseudo LRU replacement policy. */
		};


		/*  Fetch cache parameters.  */
		u32bit requestQueueSize;/**<  Size of the memory request queue.  */
		char *name;             /**<  Name/prefix/postfix for the fetch cache.  */

		/* Fetch cache structures.  */
		u32bit **reserve;       /**<  Fetch cache line reserve counter.  */
		u32bit **victim;        /**<  Victim lists per line.  */
		bool **replaceLine;     /**<  The line is being replaced.  */
		bool **dirty;           /**<  Line dirty bit.  Stores if the line has been modified.  */
		bool **masked;          /**<  Masked line.  Stores if the line uses the line write mask.  */
		bool ***writeMask;      /**<  Write mask for the bytes in a line.  */
		u32bit maxLRU;          /**<  Number of entries used in the pseudo LRU victim list (takes into account number of ways).   */
		u32bit firstWay;        /**<  First way from which to start searching a victim.  */

#if KONDAMASK
		u64bit cycle;

		struct cache_line_cycle_info
		{
			u64bit insert;
			u64bit lastHit;
			u64bit lastOn;
		};
		cache_line_cycle_info** accessCycles;
#endif

		/*  Memory request queue.  */
		Cache64Request *requestQueue;   /**<  Memory request queue.  */
		u32bit freeRequests;    /**<  Number of free requests.  */
		u32bit activeRequests;  /**<  Number of active requests entries.  */
		u32bit nextFreeRequest; /**<  Pointer to the next free request.  */
		u32bit nextRequest;     /**<  Pointer to the next memory request.  */
		u32bit *freeRequestList;/**<  List of free memory request entries.  */
		u32bit *activeList;     /**<  List of active memory request entries.  */

		//  Debug.
		bool debugMode;         /**<  Flag that stores if debug output is enabled.  */

		/*  Statistics.  */
		GPUStatistics::Statistic *fetchMisses;      /**<  Misses produced by fetch operations.  */
		GPUStatistics::Statistic *fetchHits;        /**<  Hits produced by fetch operations.  */
		GPUStatistics::Statistic *fetchMissOK;      /**<  Misses produced by fetch operations that were started/queued when produced.  */
		GPUStatistics::Statistic *fetchMissFail;    /**<  Misses produced by fetch operations that were not started/queued when produced.  */
		GPUStatistics::Statistic *fetchMissFailReqQ;/**<  Misses produced by fetch operations that were not started/queued when produced because the cache request queue was full.  */
		GPUStatistics::Statistic *fetchMissFailRes; /**<  Misses produced by fetch operations that were not started/queued when produced because there was no available cache line.  */
		GPUStatistics::Statistic *fetchMissFailMiss;/**<  Misses produced by fetch operations that were not started/queued when produced because no misses were allowed.  */
		GPUStatistics::Statistic *allocMisses;      /**<  Misses produced by alloc operations.  */
		GPUStatistics::Statistic *allocHits;        /**<  Hits produced by alloc operations.  */
		GPUStatistics::Statistic *allocMissOK;      /**<  Misses produced by alloc operations that were started/queued when produced.  */
		GPUStatistics::Statistic *allocMissFail;    /**<  Misses produced by alloc operations that were not started/queued when produced.  */
		GPUStatistics::Statistic *allocMissFailReqQ;/**<  Misses produced by alloc operations that were not started/queued when produced because the cache request queue was full.  */
		GPUStatistics::Statistic *allocMissFailRes; /**<  Misses produced by alloc operations that were not started/queued when produced because there was no available cache line.  */
		GPUStatistics::Statistic *allocMissFailMiss; /**<  Misses produced by alloc operations that were not started/queued when produced because no misses were allowed.  */
		GPUStatistics::Statistic *readOK;           /**<  Succesful read operations.  */
		GPUStatistics::Statistic *readFail;         /**<  Failed read operations.  */
		GPUStatistics::Statistic *writeOK;          /**<  Succesful write operations.  */
		GPUStatistics::Statistic *writeFail;        /**<  Failed write operations.  */
		GPUStatistics::Statistic *readBytes;        /**<  Total bytes read.  */
		GPUStatistics::Statistic *writeBytes;       /**<  Total bytes written.  */
		GPUStatistics::Statistic *unreserves;       /**<  Unreserve operations.  */

		/**
		 *
		 *  Updates the victim selection policy with a new access.
		 *
		 *  @param way Way accessed.
		 *  @param line Line accessed.
		 *
		 */

		void access(u32bit way, u32bit line);

		/**
		 *
		 *  Returns the next victim selected by the victim policy.
		 *
		 *  @return The next victim way for a given line.
		 *
		 */

		u32bit nextVictim(u32bit line);


	public:

		/**
		 *
		 *  Fetch Cache constructor.
		 *
		 *  Creates and initializes a Fetch Cache object.
		 *
		 *  @param numWays Number of ways in the cache.
		 *  @param numLines Number of lines in the cache.
		 *  @param lineBytes Number of bytes per cache line.
		 *  @param reqQueueSize Fetch cache memory request queue size.
		 *  @param name Name/prefix/postfix of the fetch cache.
		 *
		 *  @return A new initialized fetch cache object.
		 *
		 */

		FetchCache64(u32bit numWays, u32bit numLines, u32bit lineBytes, u32bit reqQueueSize,
			char *name = NULL);

		/**
		 *
		 *  Reserves and requests to memory (if the line is not already
		 *  available) the cache line for the requested address.
		 *
		 *  @param address Address inside a cache line to fetch.
		 *  @param way Reference to variable where to store the
		 *  way where the fetched line will be found.
		 *  @param line Reference to a variable where to store
		 *  the line where the fetched line will be found.
		 *  @param source Pointer to a Dynamic Object that is the source of the request.
		 *
		 *  @return If the line for the address could be reserved and
		 *  fetched returns TRUE, otherwise returns FALSE
		 *  (ex. all lines are already reserved).
		 *
		 */

		bool fetch(u64bit address, u32bit &way, u32bit &line, DynamicObject *source);

		/**
		 *
		 *  Reserves and requests to memory (if the line is not already available) the cache line
		 *  for the requested address.
		 *
		 *  @param address Address inside a cache line to fetch.
		 *  @param way Reference to variable where to store the way where the fetched line will be found.
		 *  @param line Reference to a variable where to store the line where the fetched line will be found.
		 *  @param miss Reference to a variable storing if the Fetch Cache is allowed to fetch
		 *  a line on a miss.  If the value is TRUE no fetch on miss is allowed.  The variable is
		 *  then used to return if the fetch produced a miss.
		 *  @param source Pointer to a Dynamic Object that is the source of the request.
		 *
		 *  @return If the line for the address could be reserved and fetched returns TRUE, otherwise returns FALSE
		 *  (ex. all lines are already reserved).
		 *
		 */

		bool fetch(u64bit address, u32bit &way, u32bit &line, bool &miss, DynamicObject *source);

		/**
		 *
		 *  Reserves and requests to memory (if the line is not already available) the cache line
		 *  for the requested address.  This version returns the line tag and a flag telling if
		 *  the data is already available in the cache for out of order read.
		 *
		 *  @param address Address inside a cache line to fetch.
		 *  @param way Reference to variable where to store the way where the fetched line will be found.
		 *  @param line Reference to a variable where to store the line where the fetched line will be found.
		 *  @param miss Reference to a variable storing if the Fetch Cache is allowed to fetch
		 *  a line on a miss.  If the value is TRUE no fetch on miss is allowed.  The variable is
		 *  then used to return if the fetch produced a miss.
		 *  @param ready Returns if the data for the address is ready to be read.
		 *  @param source Pointer to a Dynamic Object that is the source of the request.
		 *
		 *  @return If the line for the address could be reserved and fetched returns TRUE, otherwise returns FALSE
		 *  (ex. all lines are already reserved).
		 *
		 */

		bool fetch(u64bit address, u32bit &way, u32bit &line, bool &miss, bool &ready, DynamicObject *source);

		/**
		 *
		 *  Reserves and allocates a cache line for the requested address.
		 *  If the cache line had valid data a request to memory to
		 *  copy back the cache line data is added to the Memory Request
		 *  Queue.  Used to implement a write buffer mode.
		 *
		 *  @param address Address inside a cache line to fetch.
		 *  @param way Reference to variable where to store the
		 *  way where the fetched line will be found.
		 *  @param line Reference to a variable where to store
		 *  the line where the fetched line will be found.
		 *  @param source Pointer to a Dynamic Object that is the source of the request.
		 *
		 *  @return If the line for the address could be reserved and
		 *  allocated returns TRUE, otherwise returns FALSE
		 *  (ex. all lines are already reserved).
		 *
		 */

		bool allocate(u64bit address, u32bit &way, u32bit &line, DynamicObject *source);

		/**
		 *
		 *  Reads a given amount of data (not larger than a cache line)
		 *  data from the fetch cache.
		 *
		 *  The line associated with the requested address
		 *  must have been previously fetched, if not an error
		 *  is produced and the program exits.
		 *
		 *  @param address Address of the data to read.
		 *  @param way Line way which is going to be read.
		 *  @param line Line inside the way which is going to be read.
		 *  @param size Amount of data to read from the requested cache line.
		 *  Must be a multiple of 4 bytes.
		 *  @param data Pointer to an array where to store the read
		 *  data.
		 *
		 *  @return If the read could be performed returns TRUE,
		 *  in any other case returns FALSE (ex. line not yet received from
		 *  video memory).
		 *
		 */

		bool read(u64bit address, u32bit way, u32bit line, u32bit size, u8bit *data);

		/**
		 *
		 *  Writes a given amount of data to the fetch cache and unreserves
		 *  (if it was still reserved) the associated fetch cache line.
		 *
		 *  @param address Address of the data to write.
		 *  @param way Line way which is going to be read.
		 *  @param line Line inside the way which is going to be read.
		 *  @param size Amount of data to write (not larger than a cache
		 *  line).  Must be a multiple of 4 bytes.
		 *  @param data Pointer to a buffer where the data to write is stored.
		 *
		 *  @return If the write could be performed returns TRUE,
		 *  otherwise returns FALSE (ex. resource conflict using the write bus).
		 *
		 */

		bool write(u64bit address, u32bit way, u32bit line, u32bit size, u8bit *data);

		/**
		 *
		 *  Reads a full cache line.
		 *
		 *  @param way Way from which to read.
		 *  @param line Line to read.
		 *  @param linedata Pointer to the buffer where to store the line.
		 *
		 *  @return If the line could be read returns TRUE, if the
		 *  cache write port is busy return FALSE.
		 *
		 */

		bool readLine(u32bit way, u32bit line, u8bit *linedata);

		/**
		 *
		 *  Reads the cache line mask.
		 *
		 *  @param way Way from which to get the line mask.
		 *  @param line Line from which to get the line mask.
		 *  @param mask Pointer to a buffer where to store the mask.
		 *
		 */

		void readMask(u32bit way, u32bit line, u32bit *mask);

		/**
		 *
		 *  Reset the cache line mask.
		 *
		 *  @param way Way for which to reset the line mask.
		 *  @param line Line for which to reset the line mask.
		 *
		 */

		void resetMask(u32bit way, u32bit line);

		/**
		 *
		 *  Writes and replaces a full cache line.
		 *
		 *  @param way Way from which to read.
		 *  @param line Line to read.
		 *  @param linedata Pointer to the buffer where to store the line.
		 *  @param tag Reference to a variable where to store the tag of the written line.
		 *
		 *  @return If the line could be written returns TRUE, if the
		 *  cache read port is busy return FALSE.
		 *
		 */

		bool writeLine(u32bit way, u32bit line, u8bit *linedata, u64bit &tag);

		/**
		 *
		 *  Masked data write to the fetch cache. Writes the data (using
		 *  the mask) and liberates (if it was still reserved) the
		 *  associated fetch cache line.
		 *
		 *  @param address Address of data to write.
		 *  @param way Line way which is going to be read.
		 *  @param line Line inside the way which is going to be read.
		 *  @param size Amount of data to write (not larger than a cache
		 *  line).
		 *  @param data Pointer to a buffer where the data to write is stored.
		 *  @param mask Write mask (per byte).
		 *
		 *  @return If the write could be performed returns TRUE,
		 *  otherwise returns FALSE (ex. resource conflict using the write bus).
		 *
		 */

		bool write(u64bit address, u32bit way, u32bit line, u32bit size,
			u8bit *data, bool *mask);

		/**
		 *
		 *  Liberate a fetch cache line.
		 *
		 *  @param way Line way that is going to be liberated/unreserved.
		 *  @param line Line inside the way that is going to be liberated/unreserved.
		 *
		 *
		 */

		void unreserve(u32bit way, u32bit line);

		/**
		 *
		 *  Resets the Fetch Cache structures.
		 *
		 *  WARNING:  REDEFINED NON VIRTUAL!!!.
		 *
		 */

		void reset();

		/**
		 *
		 *  Flushes the valid fetch cache lines back to memory.
		 *
		*  @return If all the valid lines have been written
		 *  to memory returns TRUE.  If not returns FALSE.
		 *
		 */

		bool flush();

		/**
		 *
		 *  Gets the next cache request in the queue.
		 *
		 *  @param requestID The cache request identifier.
		 *
		 *  @return A pointer to the next cache request entry.  If there
		 *  are no requests returns NULL;
		 *
		 */

		Cache64Request *getRequest(u32bit &requestID);

		/**
		 *
		 *  Liberates a cache request from the request queue.
		 *
		 *  @param request Request entry number.
		 *  @param freeSpill If the spill request is liberated.
		 *  @param freeFill If the fill request is liberated.
		 *
		 */

		void freeRequest(u32bit request, bool freeSpill, bool freeFill);

		/**
		 *
		 *  Enables or disables debug output for the texture cache.
		 *
		 *  @param enable Boolean variable used to enable/disable debug output for the TextureCache.
		 *
		 */

		void setDebug(bool enable);

#if KONDAMASK
		void decay(u64bit cycleIn, u32bit decayCycles)
		{
			cycle = cycleIn;
			for (u32bit w = 0; w < numWays; w++)
			{
				for (u32bit l = 0; l < numLines; l++)
				{
					if (valid[w][l])
					{
						accessCycles[w][l].lastOn = cycle;
						if ((cycle - accessCycles[w][l].lastHit) > decayCycles)
						{
							printf("DECAY %s way %d set %d\n", name, w, l);
							gpu3d::GPUStatistics::StatisticsManager::instance().LogCacheAccess(
								this->name, line2address(w, l), GPUStatistics::StatisticsManager::CACHE_DECAY, l, w, cycle);
							// What exacty should I reset here
							valid[w][l] = FALSE;
							dirty[w][l] = FALSE;
							//reserve[w][l] = 0;
							tags[w][l] = 0;
						}
					}
				}
			}
		}
#endif

		};

	} // namespace gpu3d

#endif
