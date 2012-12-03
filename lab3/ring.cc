// ring.cc
//	Routines to implement a ring buffer for producer and consumer 
//      problem.
//	
// Copyright (c) 1995 The Regents of the University of Southern Queensland.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

extern "C" {
#include <stdio.h>
extern int exit(int st);
}

#include "ring.h"

//----------------------------------------------------------------------
// slot::slot
// 	The constructor for the slot class.  
//----------------------------------------------------------------------

slot::slot(int id, int number)
{
    thread_id = id;
    value = number;
}



//----------------------------------------------------------------------
// Ring::Ring
// 	The constructor for the Ring class.  Note that it doesn't have a
// 	return type.
//
// 	"sz" -- maximum number of elements in the ring buffer at any time
//----------------------------------------------------------------------

Ring::Ring(int sz)
{
    if (sz < 1) {
	fprintf(stderr, "Error: Ring: size %d too small\n", sz);
	exit(1);
    }

    // Initialize the data members of the ring object.
    size = sz;
    in = 0;
    out = 0;
    buffer = new slot[size]; //allocate an array of slots.
	notFull = new Condition("The buffer is not full");
	notEmpty = new Condition("The buffer is not empty");
	mutex = new Lock("The monitor's lock");
}

//----------------------------------------------------------------------
// Ring::~Ring
// 	The destructor for the Ring class.  Just get rid of the array we
// 	allocated in the constructor.
//----------------------------------------------------------------------

Ring::~Ring()
{
    // Some compilers and books tell you to write this as:
    //     delete [size] stack;
    // but apparently G++ doesn't like that.

    delete [] buffer;
	delete notFull;
	delete notEmpty;
	delete mutex;
}

//----------------------------------------------------------------------
// Ring::Put
// 	Put a message into the next available empty slot. We assume the
//       caller has done necesaary synchronization.
//
//	"message" -- the message to be put in the buffer
//----------------------------------------------------------------------

void
Ring::Put(slot *message)
{
	mutex->Acquire();
	while (Full())
		notFull->Wait(mutex);

    buffer[in].thread_id = message->thread_id;
    buffer[in].value = message->value;
    in = (in + 1) % size;

	notEmpty->Signal(mutex);
	mutex->Release();
}

//----------------------------------------------------------------------
// Ring::Get
// 	Get a message from the next full slot. We assume the
//       caller has done necesaary synchronization.
//
//	"message" -- the message from  the buffer
//----------------------------------------------------------------------

void
Ring::Get(slot *message)
{
	mutex->Acquire();
	while (Empty())
		notEmpty->Wait(mutex);
	
    message->thread_id = buffer[out].thread_id;
    message->value = buffer[out].value;
    out = (out + 1) % size;

	notFull->Signal(mutex);
	mutex->Release();
}

int
Ring::Empty()
{
	return in == out;
}


int
Ring::Full()
{
	return (in-out-1) == size;
}


