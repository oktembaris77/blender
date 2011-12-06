/*
 * Copyright 2011, Blender Foundation.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * Contributor: 
 *		Jeroen Bakker 
 *		Monique Dewanchand
 */

class MemoryBuffer;

#ifndef _COM_MemoryBuffer_h_
#define _COM_MemoryBuffer_h_

#include "COM_ExecutionGroup.h"
#include "BLI_rect.h"
#include "COM_MemoryProxy.h"
extern "C" {
    #include "BLI_threads.h"
}
#include <vector>

/**
  * @brief state of a memory buffer
  * @ingroup Memory
  */
typedef enum MemoryBufferState {
	/** @brief memory has been allocated on creator device and CPU machine, but kernel has not been executed */
	COM_MB_ALLOCATED = 1,
	/** @brief memory has been allocated, and the kernel has been executed. CopyBack has not been performed.*/
	COM_MB_CREATED = 2,
	/** @brief memory has been allocated, and the kernel has been executed. CopyBack has been performed.*/
	COM_MB_AVAILABLE = 3,
	/** @brief CPU memory has been freed, chunk is available on disk. state is a substate of available.*/
	COM_MB_STORED = 4,
	/** @brief chunk has been freed (not usable anymore)*/
	COM_MB_FREE = 5,
	/** @brief chunk is consolidated from other chunks. special state.*/
	COM_MB_TEMPORARILY = 6
} MemoryBufferState;

class MemoryProxy;

/**
  * @brief a MemoryBuffer contains access to the data of a chunk
  */
class MemoryBuffer {
private:
    /**
	  * @brief proxy of the memory (same for all chunks in the same buffer)
      */
	MemoryProxy * memoryProxy;

    /**
	  * @brief the type of buffer COM_DT_VALUE, COM_DT_VECTOR, COM_DT_COLOR
      */
	DataType datatype;


    /**
	  * @brief region of this buffer inside reative to the MemoryProxy
      */
    rcti rect;

    /**
	  * brief refers to the chunknumber within the executiongroup where related to the MemoryProxy
	  * @see memoryProxy
      */
	unsigned int chunkNumber;

	/**
	  * @brief width of the chunk
	  */
    unsigned int chunkWidth;

    /**
	  * @brief state of the buffer
      */
	MemoryBufferState state;

    /**
	  * @brief the actual float buffer/data
      */
    float* buffer;

    /**
	  * @brief filename used for reading buffer back to memory
      */
    string filename;

	/**
	  * @brief mutex of this MemoryBuffer
	  */
    ThreadMutex mutex;

	/**
	  * @brief total number of users are currently using this MemoryBuffer
	  */
	unsigned int numberOfUsers;

private:
	/**
	  * @brief read this MemoryBuffer back into memory
	  */
    void readFromDisc();


	/**
	  * @brief is this MemoryBuffer available
	  */
	bool isAvailable();

	/**
	  * @brief generate the filename to save this tile to.
	  */
	bool determineFilename();


public:
	/**
	  * @brief construct new MemoryBuffer for a chunk
	  */
	MemoryBuffer(MemoryProxy *memoryProxy, unsigned int chunkNumber, rcti* rect);

	/**
	  * @brief construct new temporarily MemoryBuffer for an area
	  */
	MemoryBuffer(MemoryProxy *memoryProxy, rcti* rect);

	/**
	  * @brief destructor
	  */
    ~MemoryBuffer();

	/**
	  * @brief save this MemoryBuffer to disk and free the memory
	  */
	bool saveToDisc();

	/**
	  * @brief make sure this MemoryBuffer is loaded in memory
	  * @param addUser increase the number of users
	  */
    bool makeAvailable(bool addUser);

	/**
	  * @brief read the ChunkNumber of this MemoryBuffer
	  */
	unsigned int getChunkNumber() {return this->chunkNumber;}

	/**
	  * @brief get the data of this MemoryBuffer
	  * @note buffer should already be available in memory
	  */
    float* getBuffer() {return this->buffer;}

	/**
	  * @brief after execution the state will be set to available by calling this method
	  */
    void setCreatedState() {
        this->state = COM_MB_AVAILABLE;
    }

    void read(float* result, int x, int y);
    void readCubic(float* result, float x, float y);

	/**
	  * @brief is this MemoryBuffer a temporarily buffer (based on an area, not on a chunk)
	  */
	inline const bool isTemporarily() const {return this->state == COM_MB_TEMPORARILY;}

	/**
	  * @brief add the content from otherBuffer to this MemoryBuffer
	  * @param otherBuffer source buffer
	  */
    void copyContentFrom(MemoryBuffer* otherBuffer);

	/**
	  * @brief how many memory is allocated by this MemoryBuffer
	  */
    long getAllocatedMemorySize();



	/**
	  * @brief increase the number of users of this MemoryBuffer
	  */
    void addUser();
	/**
	  * @brief decrease the number of users of this MemoryBuffer
	  */
	void removeUser();

	/**
	  * @brief get the rect of this MemoryBuffer
	  */
    rcti* getRect() {return &this->rect;}

	/**
	  * @brief get the width of this MemoryBuffer
	  */
	int getWidth() const;

	/**
	  * @brief get the height of this MemoryBuffer
	  */
	int getHeight() const;

private:
    unsigned int determineBufferSize();
};

#endif
