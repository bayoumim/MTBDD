////////////////////////////////////////////////////////////////////////////////////////////////////
// this file contains all Standard header file utilized in this package
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _BDD_UTIL_FILE
#define _BDD_UTIL_FILE

// defined constant
#define TERMINAL_LEVEL                                  0	// define the level of the terminal nodes (0,1)
#define ONE_ID                                          0	// define the id of node 'one'
#define ONE_LEVEL                                       0       // define the level for one node
#define FORWARD_VALUE                                   0XA	// define the special value for forwarded nodes (if this value was found in the else pointer, this means that this node is forwarded)
#define HASH_NO_OF_LOOK_UP_TABLES                       31	// define the max. no of loop up tables (use in resizing hash table)
#define PERMENANT_NODE                                  1	// define the value that indicate wether this node is permenant or not (used in incremental Mark and some other GC techniques)
#define INITIAL_MARK_FOR_ORDINARY_MARK_GC               0
#define INITIAL_MARK_FOR_INCREMENTAL_MARK_GC            1


// ------------------------------------------------------------------------------------------------------------------------------------------------------------
// these constants are used in BDDPointer class
// ------------------------------------------------------------------------------------------------------------------------------------------------------------
#define MASK_FOR_LOCAL_ID_IN_BDD_POINTER 		0X1F	// this used to extract the first 5 bits in the BDDPointer class (corresponding to local id) 
#define MASK_FOR_UPPER_FLAGS_IN_BDD_POINTER 		0XC0	// this used to extract the last 2 bits in the BDDPointer class (corresponding to inverted and temporary flags) 
#define MASK_FOR_INVERTED_BIT_IN_BDD_POINTER 		0X80	// this used to extract the inversion bit in the the BDDPointer class
#define MASK_FOR_REMOVING_INVERTED_BIT_IN_BDD_POINTER 	0X7F	// this used to REMOVE the inversion bit in the the BDDPointer class
#define MASK_FOR_TEMPORARY_BIT_IN_BDD_POINTER 		0X40	// this used to extract the Temporary bit in the the BDDPointer class
#define MASK_FOR_REMOVING_TEMPORARY_BIT_IN_BDD_POINTER 	0XBF	// this used to REMOVE the temporary bit in the the BDDPointer class
#define MASK_FOR_REMOVING_UNUSED_BIT_IN_BDD_POINTER 	0XDF	// this used to REMOVE the unused bit in the the BDDPointer class
#define SHIFT_FOR_ID_IN_BDD_POINTER                     32	// SHIFT used in getting long id

// ------------------------------------------------------------------------------------------------------------------------------------------------------------
// these constants are used in BDD NODE class
// ------------------------------------------------------------------------------------------------------------------------------------------------------------
#define        MASK_FOR_LEVEL_IN_BDD_NODE                                     0x1FFF                  // mask for extracting level in 'level' field
#define        MASK_FOR_REMOVING_LEVEL_IN_BDD_NODE                            0XE000                  // mask for removing full flag in 'data group 2; field
#define        MASK_FOR_INVERTED_FLAG_IN_BDD_NODE                             0x8000                  // mask for extracting INVERSION FLAG in 'level' field
#define        MASK_FOR_REMOVING_INVERTED_FLAG_IN_BDD_NODE                    0X7FFF                  // mask for removing INVERSION FLAG flag in 'data group 2; field
#define        MASK_FOR_FORWARDED_FLAG_IN_BDD_NODE                            0x4000                  // mask for extracting FORWARDED FLAG in 'level' field
#define        MASK_FOR_REMOVING_FORWARDED_FLAG_IN_BDD_NODE                   0XBFFF                  // mask for removing FORWARDED FLAG flag in 'data group 2; field

// ------------------------------------------------------------------------------------------------------------------------------------------------------------
// these constants are used for bdd bucket
// ------------------------------------------------------------------------------------------------------------------------------------------------------------
// length of local id
#define         LOCAL_ID_LENGHT                                     5	// the length of local id (within the bucket) is 5 bits
// then constant
#define 	MASK_FOR_THEN_ID_IN_BDD_BUCKET                      0x3E0			// define the mask of the local id for the 'then' pointer
#define 	SHIFT_FOR_THEN_ID_IN_BDD_BUCKET                     LOCAL_ID_LENGHT		// define the amount of shift for 'then' local id in bdd BUCKET
#define		MASK_FOR_REMOVE_THEN_ID_IN_BDD_BUCKET               0xFFFFFC1F			// mask used to remove 'then' id from the 'group data' field
// else constant
#define 	MASK_FOR_ELSE_ID_IN_BDD_BUCKET                      0x7C00			// define the mask of the local id for the 'ELSE' pointer
#define 	SHIFT_FOR_ELSE_ID_IN_BDD_BUCKET                     LOCAL_ID_LENGHT<<1		// define the amount of shift for 'ELSE' local id in bdd BUCKET (2*local id length)
#define		MASK_FOR_REMOVE_ELSE_ID_IN_BDD_BUCKET               0xFFFF83FF                  // mask used to remove 'ELSE' id from the 'group data' field
#define		MASK_FOR_ELSE_INVERTED_EDGE_IN_BDD_BUCKET           0x8000                      // mask used to extract the inverted edge in bdd bucket
#define		MASK_FOR_REMOVING_ELSE_INVERTED_EDGE_IN_BDD_BUCKET  0xFFFF7FFF                  // mask used to remove the inverted edge in bdd bucket

// local BUCKET id
#define 	MASK_FOR_LOCAL_ID_IN_BDD_BUCKET 		MASK_FOR_LOCAL_ID_IN_BDD_POINTER	// define the mask of the BUCKET id as the mask of the bbd pointer
#define 	MASK_FOR_REMOVE_LOCAL_ID_IN_BDD_BUCKET	0xFFFFFFE0				// mask to remove the local id BUCKET from 'group data' field
// inverted bit for else pointer
#define		MASK_FOR_INVERTED_EDGE_IN_BDD_BUCKET 	0x8000					// mask used to extract the inverted edge for the else pointer in bdd
#define		SHIFT_FOR_INVERTED_EDGE_IN_BDD_BUCKET	LOCAL_ID_LENGHT<<1                      // shift for the else pointer (2* local id length)
#define		MASK_FOR_INVERTED_BIT_IN_BDD_BUCKET	MASK_FOR_INVERTED_BIT_IN_BDD_POINTER	// mask used to extract the inverted edge from a bdd pointer in order to store it in bdd bucket (used in setElse in bucket class)

// full flag in bucket

// level
#define 	MASK_FOR_GETTING_LEVEL_IN_BDD_BUCKET	0x3FFE0000				// mask used to extract level from the 'group data' field in bdd bucket
#define		SHIFT_FOR_LEVEL_IN_BDD_BUCKET		((3 * LOCAL_ID_LENGHT) + 2)			// amount of shift for level (3 local id + else edge + empty)
#define		MASK_FOR_REMOVING_LEVEL_IN_BDD_BUCKET	0xC001FFFF				// mask used to remove level from the 'group data' field in bdd
#define		MAX_NO_OF_LEVELS_IN_BDD_BUCKET		0X1FFF					// max. no. of levels (2^13-1)
// full , swap and reserved flags
#define 	MASK_FOR_FULL_FLAG_IN_BDD_BUCKET                0x10000				// mask used for extracting full flag from bdd bucket
#define		MASK_FOR_REMOVING_FULL_FLAG_IN_BDD_BUCKET	0xFFFEFFFF			// mask used to remove full flag from bdd bucket
#define		MASK_FOR_SWAP_IN_BDD_BUCKET                     0x40000000			// mask for extracting swap flag
#define		MASK_FOR_RESERVED_IN_BDD_BUCKET                 0x80000000			// mask for extracting reserved flag
#define		MASK_FOR_REMOVING_SWAP_IN_BDD_BUCKET            0xBFFFFFFF			// mask for removing swap in bdd bucket
#define		MASK_FOR_REMOVING_RESERVED_IN_BDD_BUCKET        0x7FFFFFFF			// mask for removing reserved in bdd bucket
#define		MASK_FOR_ALL_FLAGS_IN_BDD_BUCKET                0xC0010000			// mask for extracting all flags in bdd bucket
#define		MASK_FOR_REMOVING_ALL_FLAGS_IN_BDD_BUCKET	0x3FFEFFFF			// mask for removing ALL flags in bdd bucket


// ------------------------------------------------------------------------------------------------------------------------------------------------------------
// these constants are used for hash class (unique table)
// ------------------------------------------------------------------------------------------------------------------------------------------------------------
#define 	MASK_ALL_BITS_IN_HOP_INFO_IN_HASH_TABLE		0xFFFFFFFF				// mask all bits in hop infos

// ------------------------------------------------------------------------------------------------------------------------------------------------------------
// these constants are used in Request class
// ------------------------------------------------------------------------------------------------------------------------------------------------------------
// constants for flags
#define        MASK_FOR_FULL_FLAG_IN_REQUEST_BUCKET                                     0x1                     // mask for extracting full flag in 'data group 2. field
#define        MASK_FOR_REMOVING_FULL_FLAG_IN_REQUEST_BUCKET                            0XFFFFFFFE              // mask for removing full flag in 'data group 2; field
#define        MASK_FOR_DATED_FLAG_IN_REQUEST_BUCKET                                    0x2                     // mask for extracting dated flag in 'data group 2. field
#define        MASK_FOR_REMOVING_DATED_FLAG_IN_REQUEST_BUCKET                           0XFFFFFFFD              // mask for removing dated flag in 'data group 2; field
#define        MASK_FOR_THEN_PROCESSED_IN_APPLY_FLAG_IN_REQUEST_BUCKET                  0x4                     // mask for extracting 'THEN PROCESSED IN APPLY' flag in 'data group 2. field
#define        MASK_FOR_REMOVING_THEN_PROCESSED_IN_APPLY_FLAG_IN_REQUEST_BUCKET         0xFFFFFFFB              // mask for removing 'THEN PROCESSED IN APPLY' flag in 'data group 2; field
#define        MASK_FOR_ELSE_PROCESSED_IN_APPLY_FLAG_IN_REQUEST_BUCKET                  0x8                     // mask for extracting 'ELSE PROCESSED IN APPLY' flag in 'data group 2. field
#define        MASK_FOR_REMOVING_ELSE_PROCESSED_IN_APPLY_FLAG_IN_REQUEST_BUCKET         0xFFFFFFF7              // mask for removing 'ELSE PROCESSED IN APPLY' flag in 'data group 2; field
#define        MASK_FOR_THEN_PROCESSED_IN_REDUCE_FLAG_IN_REQUEST_BUCKET                 0x10                    // mask for extracting 'THEN PROCESSED IN REDUCE' flag in 'data group 2. field
#define        MASK_FOR_REMOVING_THEN_PROCESSED_IN_REDUCE_FLAG_IN_REQUEST_BUCKET        0xFFFFFFEF              // mask for removing 'THEN PROCESSED IN REDUCE' flag in 'data group 2; field
#define        MASK_FOR_ELSE_PROCESSED_IN_REDUCE_FLAG_IN_REQUEST_BUCKET                 0x20                    // mask for extracting 'ELSE PROCESSED IN REDUCE' flag in 'data group 2. field
#define        MASK_FOR_REMOVING_ELSE_PROCESSED_IN_REDUCE_FLAG_IN_REQUEST_BUCKET        0xFFFFFFDF              // mask for removing 'ELSE PROCESSED IN REDUCE' flag in 'data group 2; field
#define        MASK_FOR_THEN_TEMPORARY_FLAG_IN_REQUEST_BUCKET                           0x40                    // mask for extracting 'THEN TEMPORARY' flag in 'data group 2. field
#define        MASK_FOR_REMOVING_THEN_TEMPORARY_FLAG_IN_REQUEST_BUCKET                  0xFFFFFFBF              // mask for removing 'THEN TEMPORARY' flag in 'data group 2; field
#define        MASK_FOR_ELSE_TEMPORARY_FLAG_IN_REQUEST_BUCKET                           0x80                    // mask for extracting 'ELSE TEMPORARY' flag in 'data group 2. field
#define        MASK_FOR_REMOVING_ELSE_TEMPORARY_FLAG_IN_REQUEST_BUCKET                  0xFFFFFF7F              // mask for removing 'ELSE TEMPORARY' flag in 'data group 2; field
#define        MASK_FOR_R_NODE_INVERSION_IN_REQUEST_BUCKET                              0x100                   // mask for extracting 'NODE INVERSION BIT' flag in 'data group 2. field
#define        MASK_FOR_REMOVING_R_NODE_INVERSION_FLAG_IN_REQUEST_BUCKET                0xFFFFFEFF              // mask for removing 'NODE INVERSION BIT' flag in 'data group 2; field
#define        MASK_FOR_F_INVERTED_EDGE_IN_REQUEST_BUCKET                               0x200                   // mask for extracting 'F INVERTED EDGE' flag in 'data group 2. field
#define        MASK_FOR_REMOVING_F_INVERTED_EDGE_FLAG_IN_REQUEST_BUCKET                 0xFFFFFDFF              // mask for removing 'F INVERTED EDGE' flag in 'data group 2; field
#define        MASK_FOR_G_INVERTED_EDGE_IN_REQUEST_BUCKET                               0x400                   // mask for extracting 'G INVERTED EDGE' flag in 'data group 2. field
#define        MASK_FOR_REMOVING_G_INVERTED_EDGE_FLAG_IN_REQUEST_BUCKET                 0xFFFFFBFF              // mask for removing 'G INVERTED EDGE' flag in 'data group 2; field
#define        MASK_FOR_H_INVERTED_EDGE_IN_REQUEST_BUCKET                               0x800                   // mask for extracting 'H INVERTED EDGE' flag in 'data group 2. field
#define        MASK_FOR_REMOVING_H_INVERTED_EDGE_FLAG_IN_REQUEST_BUCKET                 0xFFFFF7FF              // mask for removing 'H INVERTED EDGE' flag in 'data group 2; field
#define        MASK_THEN_POINTER_INVERTED_EDGE_IN_REQUEST_BUCKET                        0x1000                  // mask for extracting 'THEN POINTER INVERTED EDGE' flag in 'data group 2. field
#define        MASK_FOR_REMOVING_THEN_POINTER_INVERTED_EDGE_IN_REQUEST_BUCKET           0xFFFFEFFF              // mask for removing 'THEN POINTER INVERTED EDGE' flag in 'data group 2; field
#define        MASK_ELSE_POINTER_INVERTED_EDGE_IN_REQUEST_BUCKET                        0x2000                  // mask for extracting 'ELSE POINTER INVERTED EDGE' flag in 'data group 2. field
#define        MASK_FOR_REMOVING_ELSE_POINTER_INVERTED_EDGE_IN_REQUEST_BUCKET           0xFFFFDFFF              // mask for removing 'ELSE POINTER INVERTED EDGE' flag in 'data group 2; field
#define        MASK_FORWARDED_NODE_IN_REQUEST_BUCKET                                    0x4000                  // mask for extracting 'forwarded' flag in 'data group 2. field
#define        MASK_FOR_REMOVIN_FORWARDED_NODE_IN_REQUEST_BUCKET                        0xFFFFBFFF              // mask for removing 'forwarded' flag in 'data group 2; field
// these flags are stored in 'data group'1
#define        MASK_FOR_RESERVED_FLAG_IN_REQUEST_BUCKET                                 0x40000000              // mask for extracting reserved flag in 'data group 1. field
#define        MASK_FOR_REMOVING_RESERVED_FLAG_IN_REQUEST_BUCKET                        0xBFFFFFFF              // mask for removing reserved flag in 'data group 1; field
#define        MASK_FOR_SWAPPED_FLAG_IN_REQUEST_BUCKET                                  0x80000000              // mask for extracting SWAPPED flag in 'data group 1. field
#define        MASK_FOR_REMOVING_SWAPPED_FLAG_IN_REQUEST_BUCKET                         0x7FFFFFFF              // mask for removing SWAPPED flag in 'data group 1; field
// flags to aquire all flags
#define        MASK_FOR_ALL_FLAGS_IN_REQUEST_BUCKET                                     0x7FFF                  // mask for extracting 'ALL FLAGS' flag in 'data group 2. field
#define        MASK_FOR_REMOVING_ALL_FLAGS_IN_REQUEST_BUCKET                            0xFFFF8000              // mask for removing 'ALL FLAGS' flag in 'data group 2; field
// constants for level
#define        SHIFT_FOR_LEVEL_REQUEST_BUCKET                                           19                      // shift for level
#define        MASK_FOR_REMOVING_LEVEL_IN_REQUEST_BUCKET                                0x7FFFF                 // mask for removing 'level' in 'data group 2; field
// constants for ids
#define        MASK_FOR_ID_IN_REQUEST_BUCKET                                            0x1F                    // mask for extracting local id for node R (data group 1)
#define        MASK_FOR_REMOVING_R_NODE_ID_IN_REQUEST_BUCKET                            0xFFFFFFE0              // mask for REMOVING local id for node R (data group 1)
#define        MASK_FOR_REMOVING_F_ID_IN_REQUEST_BUCKET                                 0xFFFFFC1F              // mask for REMOVING local id for F (data group 1)
#define        MASK_FOR_REMOVING_G_ID_IN_REQUEST_BUCKET                                 0xFFFF83FF              // mask for REMOVING local id for G(data group 1)
#define        MASK_FOR_REMOVING_H_ID_IN_REQUEST_BUCKET                                 0xFFF07FFF              // mask for REMOVING local id for H(data group 1)
#define        MASK_FOR_REMOVING_R_THEN_ID_IN_REQUEST_BUCKET                            0xFE0FFFFF              // mask for REMOVING local id for R Then(data group 1)
#define        MASK_FOR_REMOVING_R_ELSE_ID_IN_REQUEST_BUCKET                            0xC1FFFFFF              // mask for REMOVING local id for R Else(data group 1)
#define        SHIFT_FOR_F_IN_REQUEST_BUCKET                                            5                       // shift for F id
#define        SHIFT_FOR_G_IN_REQUEST_BUCKET                                            10                      // shift for G id
#define        SHIFT_FOR_H_IN_REQUEST_BUCKET                                            15                      // shift for H id
#define        SHIFT_FOR_R_THEN_IN_REQUEST_BUCKET                                       20                      // shift for H id
#define        SHIFT_FOR_R_ELSE_IN_REQUEST_BUCKET                                       25                      // shift for H id



#include "pthread.h" 
#include "mutex.h"

#include <math.h>
#include <iostream>
#include <ctype.h>
#include <strings.h> // or <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <iomanip>
#include <fstream>
#include <cstring>
#include <string> 
#include <sys/time.h>
#include "circuit.hh"
#include <sstream>

// for console arguments
#include <unistd.h>
#include <getopt.h>



#include <vector> 
using std::vector;
#include <vector>
using std::vector;
#include <string>
using std::string;

using namespace std;
using std::cerr;
using std::cin;
using std::cout;
// for string
using std::string;

// for formating the ouput
using std::endl;
using std::hex;
using std::oct;
using std::showbase;
using std::noshowbase;
using std::setbase;

// for file stream
using std::ofstream; // output file stream
using std::ios;
// String stream
using std::stringstream;

// defined options of integrity checker
enum
{
   IC_NO_THING,         				/* 0 */
	// validate hash table 
   IC_DUMP_HASH_TABLE,					/* 1 */
   IC_VALIDATE_HASH,					/* 2 */
	// validate queue (requests)
   IC_DUMP_QUEUE,					/* 3 */
   IC_VALIDATE_QUEUE,					/* 4 */
	// validate queue after apply
   IC_DUMP_QUEUE_AFTER_APPLY,				/* 5 */
   IC_VALIDATE_QUEUE_AFTER_APPLY,			/* 6 */
	// validate queue after reduce
   IC_DUMP_QUEUE_AFTER_REDUCE,				/* 7 */
   IC_VALIDATE_QUEUE_AFTER_REDUCE,			/* 8 */
	// validate hash after GC
// check wether you are going to remove a live node (which is wrong)
   IC_DUMP_HASH_AFTER_GC,				/* 9 */
   IC_VALIDATE_HASH_DURING_GC,				/* 10 */
	// validate with simulation
   IC_VALIDATE_CIRCUIT_OUTPUTS_WITH_SIMULATION,		/* 11 */
	// validate with isomorphic comparision with CUDD structure
   IC_VALIDATE_CIRCUIT_WITH_CUDD,			/* 12 */
        // Validate with depreciated hash
   IC_VALIDATE_WITH_DEPRECIATED_HASH,                   /* 13 */
        // validate with depreciated queue
   IC_VALIDATE_WITH_DEPRECIATED_QUEUE,                  /* 14 */
        // validate with depreciated queue after apply
   IC_VALIDATE_WITH_DEPRECIATED_QUEUE_AFTER_APPLY,      /* 15 */
        // validate with depreciated queue after reduce
   IC_VALIDATE_WITH_DEPRECIATED_QUEUE_AFTER_REDUCE,     /* 16 */
        // Validate hash table after reduce.
   IC_DUMP_HASH_TABLE_AFTER_REDUCE,			/* 17 */
   IC_VALIDATE_HASH_TABLE_AFTER_REDUCE			/* 18 */
};
// Define options for GC 
enum
{
    H_NO_GC,         			/* 0 */
    H_GC_RF_ENABLED,        		/* 1 */
    H_GC_MARK_ENABLED,        		/* 2 */
    H_GC_INCREMENTAL_MARK_ENABLED       /* 3 */
};
// Define options for Variable reordering 
enum
{
   VR_NO_VARIABLE_REORDERING,         		/* 0 */
   VR_WEIGHTED_HEURISTIC        		/* 1 */
};
// type of resizing 
enum
{
    H_RESIZE_TYPE_SWAP,         				/* 0 */
    H_RESIZE_TYPE_INCREMENTAL         			/* 1 */
};

#endif 
