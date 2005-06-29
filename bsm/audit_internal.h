/*
 * @APPLE_LICENSE_HEADER_START@
 *
 * Copyright (c) 1999-2004 Apple Computer, Inc.
 * Copyright (c) 2005 SPARTA, Inc.
 * All Rights Reserved.
 *
 * This code was developed in part by Robert N. M. Watson, Senior Principal
 * Scientist, SPARTA, Inc.
 *
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 *
 * @APPLE_LICENSE_HEADER_END@
 */

#ifndef _LIBBSM_INTERNAL_H
#define	_LIBBSM_INTERNAL_H

/*
 * audit_internal.h contains private interfaces that are shared by user space
 * and the kernel for the purposes of assembling audit records.  Applications
 * should not include this file or use the APIs found within, or it may be
 * broken with future releases of OpenBSM, which may delete, modify, or
 * otherwise break these interfaces or the assumptions they rely on.
 */

/* We could determined the header and trailer sizes by
 * defining appropriate structures. We hold off that approach
 * till we have a consistant way of using structures for all tokens.
 * This is not straightforward since these token structures may
 * contain pointers of whose contents we dont know the size
 * (e.g text tokens)
 */
#define	BSM_HEADER_SIZE		18
#define	BSM_TRAILER_SIZE	7

/*
 * BSM token streams store fields in big endian byte order, so as to be
 * portable; when encoding and decoding, we must convert byte orders for
 * typed values.
 */
#define	ADD_U_CHAR(loc, val)						\
	do {								\
		*loc = val;						\
		loc += sizeof(u_char);					\
	} while(0)


#define	ADD_U_INT16(loc, val)						\
	do {								\
		be16enc(loc, val);					\
		loc += sizeof(u_int16_t);				\
	} while(0)

#define	ADD_U_INT32(loc, val)						\
	do {								\
		be32enc(loc, val);					\
		loc += sizeof(u_int32_t);				\
	} while(0)

#define	ADD_U_INT64(loc, val)						\
	do {								\
		be64enc(loc, val);					\
		loc += sizeof(u_int64_t); 				\
	} while(0)

#define	ADD_MEM(loc, data, size)					\
	do {								\
		memcpy(loc, data, size);				\
		loc += size;						\
	} while(0)

#define	ADD_STRING(loc, data, size)	ADD_MEM(loc, data, size)

#endif /* !_LIBBSM_INTERNAL_H_ */
