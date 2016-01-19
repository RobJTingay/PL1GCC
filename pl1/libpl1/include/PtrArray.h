#ifndef PtrArray_H
#define PtrArray_H 1

/*

  This particular code is part of the PL/I preprocessor, intended to be
  used in the PL/I front end for GCC.  

  Copyright 2006  Michael J. Garvin
  Copyright 2011  Henrik Sorensen

  Original author:

    Michael J. Garvin
    http://www.magma.ca/~mgarvin/
    mgarvin@magma.ca
    mgarvin@nortel.com
 
*/

/*

  This file is part of GCC PL/I.

     GCC PL/I is free software; you can redistribute it and/or modify it under
     the terms of the GNU General Public License as published by the Free
     Software Foundation; version 3.

     GCC PL/I is distributed in the hope that it will be useful, but WITHOUT ANY
     WARRANTY; without even the implied warranty of MERCHANTABILITY or
     FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
     for more details.

     You should have received a copy of the GNU General Public License
     along with GCC PL/I; see the file COPYING3.  If not see
     <http://www.gnu.org/licenses/>.

*/

/* system includes */

#include <stdlib.h>
#include <string.h>

/*
   A fast pointer array, this version of an array is 
   intended to be used for shallow sequences of small
   objects. To keep things simple the vector works with
   void * pointers.  You will have to cast things going
   in/comming out. This should be generic enough to work
   with any light weight objects you have, but also not
   burden you with a lot of macro stuff or heavy call 
   signatures to try and "template"ize things.
 
   The capacity is automatic and will double (power's of 2)
   whenever it needs to grow.

   See below for operations you can perform, basically 
   most if not all of std::vector.

*/

typedef struct {

  /* the number of times we have doubled up */

  int level;

  /* the array of pointers to the objects */

  void **ptrs;

  /* the maximum capacity of ptrs */

  int maxCapacity;

  /* number of items */

  int nitems;

} PtrArray;

/* default constructor make an empty array */

static inline void PtrArray_default_ctor(PtrArray *obj) {

  if(obj==NULL) {
    return ;
  }

  obj->level = 0;

  obj->maxCapacity = 64;

  obj->ptrs = (void**)malloc(obj->maxCapacity*sizeof(void*));

  obj->nitems = 0;

  memset(obj->ptrs, '\0', sizeof(void*)*(obj->maxCapacity));
}

/* copy constructor, make an array from another one */

static inline void PtrArray_copy_ctor(PtrArray *this, PtrArray *other) {

  if(this==NULL) {
    return ;
  }

  this->level = other->level;

  this->maxCapacity = other->maxCapacity;

  this->ptrs = (void**)malloc(this->maxCapacity*sizeof(void*));

  this->nitems = other->nitems;

  int i=0;
  for(i=0; i<this->nitems; i++) {
    this->ptrs[i] = other->ptrs[i];
  }
}

/* clear out the object pointers */

static inline void PtrArray_clear(PtrArray *this) {

  if(this==NULL) {
    return ;
  }

  /* bulk clear */

  memset(this->ptrs, '\0', sizeof(void*)*(this->maxCapacity));

  this->nitems=0;

  /* capacity is still the same */
}

/* 
   assign an arry to this one, this one must already be
   constructed.
*/

static inline void PtrArray_copy(PtrArray *this, PtrArray *other) {

  if((this==NULL)||(other==NULL)) {
    return ;
  }

  /* get rid of the old values */

  PtrArray_clear(this);

  free(this->ptrs);

  this->level = other->level;

  /* put in the new values */

  this->maxCapacity = other->maxCapacity;

  this->ptrs = (void*)malloc(this->maxCapacity*sizeof(void*));
      
  this->nitems = other->nitems;

  /* copy the pointers over */

  int i=0;
  for(i=0; i<this->nitems; i++) {

    this->ptrs[i] = other->ptrs[i];

  }
}

/* fetch attributes */

static inline int PtrArray_capacity(PtrArray *this) {
  if(this==NULL) {
    return 0;
  }
  return this->maxCapacity;
}

static inline int PtrArray_size(PtrArray *this) {
  if(this==NULL) {
    return 0;
  }
  return this->nitems;
}

static inline int PtrArray_entries(PtrArray *this) {
  if(this==NULL) {
    return 0;
  }
  return this->nitems;
}

/* fetch an item, no bounds checking */

static inline void *PtrArray_index(PtrArray *this, size_t i) {
  return this->ptrs[i];
}
static inline void PtrArray_set(PtrArray *this, size_t i, void *value) {
  this->ptrs[i] = value;
}

/* fetch an item with bounds checking */

static inline void *PtrArray_at(PtrArray *this, size_t i) {

 if(this==NULL) {
    return NULL;
 }
 if(i>=(size_t)(this->nitems)) {
   return NULL;
 }

 return this->ptrs[i];
}

/* fetch the item at the back (no bounds checking) */

static inline void *PtrArray_back(PtrArray *this) {
  return this->ptrs[this->nitems-1];
}

/* fetch the item at the front (no bounds checking) */
 
static inline void *PtrArray_front(PtrArray *this) {
  return this->ptrs[0];
}

/* make the array bigger if needed */

static int PtrArray_resize(PtrArray *this) {

  if(this==NULL) {
    return 0;
  }

  /* really necessary? */

  if(this->nitems<PtrArray_capacity(this)) {
    return 1;
  }

  /* make the new pointer array */

  int newCapacity = this->maxCapacity * 2;
     
  void **ptrs2 = NULL;

  ptrs2 = (void**)malloc(newCapacity*sizeof(void*));
    
  /* copy over the old pointers */

  memcpy(ptrs2, this->ptrs, sizeof(void*)*(this->maxCapacity));

  /* get rid of the old pointer array */

  free((void*)this->ptrs);
  
  this->ptrs = ptrs2;

  /* nitems is the same */

  this->maxCapacity = newCapacity;

  this->level++;

  return 1;
}

/* insert an item (at the end), expand array if needed */

static inline int PtrArray_push_back(PtrArray *this, void *item) {

  if(this==NULL) {
    return 0;
  }

  /* make sure there is room */

  if(this->nitems>=PtrArray_capacity(this)) {
    if(!PtrArray_resize(this)) {
      return 0;
    }
  }

  this->ptrs[this->nitems++] = item;

  return 1;
}

/* erase a particular item */

static inline void PtrArray_erase(PtrArray *this, int index) {

  if(this==NULL) {
    return ;
  }

  if(index<0) {
    return ;
  }

  if(index>=this->nitems) {
    return ;
  }

  /* copy down the remainder */

  int i=0;
  for(i=index+1; i<this->nitems; i++) {
    this->ptrs[i-1]=this->ptrs[i];
  }

  this->nitems--;
}

/* erase the trailing item */

static inline void PtrArray_pop_back(PtrArray *this) {
  PtrArray_erase(this, PtrArray_size(this)-1);
}

/* standard destructor */

static inline void PtrArray_dtor(PtrArray *this) {

  if(this==NULL) {
    return ;
  }

  /* get rid of the pointer array */

  free((void*)this->ptrs);

  this->level = 0;
  this->maxCapacity = 0;
  this->ptrs = NULL;
  this->nitems = 0;
}

static inline PtrArray *PtrArray_new(void) {

  PtrArray *tmp = (PtrArray*)(malloc(sizeof(PtrArray)));
  memset(tmp, '\0', sizeof(PtrArray));

  return tmp;
}
static inline void PtrArray_delete(PtrArray *this) {
  if(this==NULL) {
    return ;
  }
  free(this);
}

static inline PtrArray *MakePtrArray(void) {

  PtrArray *this = PtrArray_new();
  PtrArray_default_ctor(this);

  return this;
}

static inline void DisposePtrArray(PtrArray *this) {

  if(this==NULL) {
    return ;
  }

  PtrArray_dtor(this);
  PtrArray_delete(this);
}

#endif

