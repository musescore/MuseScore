#ifdef __cplusplus
extern "C" {
#endif

#ifndef	_child_port_server_
#define	_child_port_server_

/* Module child_port */

#include <string.h>
#include <mach/ndr.h>
#include <mach/boolean.h>
#include <mach/kern_return.h>
#include <mach/notify.h>
#include <mach/mach_types.h>
#include <mach/message.h>
#include <mach/mig_errors.h>
#include <mach/port.h>
	
/* BEGIN VOUCHER CODE */

#ifndef KERNEL
#if defined(__has_include)
#if __has_include(<mach/mig_voucher_support.h>)
#ifndef USING_VOUCHERS
#define USING_VOUCHERS
#endif
#ifndef __VOUCHER_FORWARD_TYPE_DECLS__
#define __VOUCHER_FORWARD_TYPE_DECLS__
#ifdef __cplusplus
extern "C" {
#endif
	extern boolean_t voucher_mach_msg_set(mach_msg_header_t *msg) __attribute__((weak_import));
#ifdef __cplusplus
}
#endif
#endif // __VOUCHER_FORWARD_TYPE_DECLS__
#endif // __has_include(<mach/mach_voucher_types.h>)
#endif // __has_include
#endif // !KERNEL
	
/* END VOUCHER CODE */

	
/* BEGIN MIG_STRNCPY_ZEROFILL CODE */

#if defined(__has_include)
#if __has_include(<mach/mig_strncpy_zerofill_support.h>)
#ifndef USING_MIG_STRNCPY_ZEROFILL
#define USING_MIG_STRNCPY_ZEROFILL
#endif
#ifndef __MIG_STRNCPY_ZEROFILL_FORWARD_TYPE_DECLS__
#define __MIG_STRNCPY_ZEROFILL_FORWARD_TYPE_DECLS__
#ifdef __cplusplus
extern "C" {
#endif
	extern int mig_strncpy_zerofill(char *dest, const char *src, int len) __attribute__((weak_import));
#ifdef __cplusplus
}
#endif
#endif /* __MIG_STRNCPY_ZEROFILL_FORWARD_TYPE_DECLS__ */
#endif /* __has_include(<mach/mig_strncpy_zerofill_support.h>) */
#endif /* __has_include */
	
/* END MIG_STRNCPY_ZEROFILL CODE */


#ifdef AUTOTEST
#ifndef FUNCTION_PTR_T
#define FUNCTION_PTR_T
typedef void (*function_ptr_t)(mach_port_t, char *, mach_msg_type_number_t);
typedef struct {
        char            *name;
        function_ptr_t  function;
} function_table_entry;
typedef function_table_entry   *function_table_t;
#endif /* FUNCTION_PTR_T */
#endif /* AUTOTEST */

#ifndef	child_port_MSG_COUNT
#define	child_port_MSG_COUNT	1
#endif	/* child_port_MSG_COUNT */

#include <mach/std_types.h>
#include <mach/mig.h>
#include <mach/mig.h>
#include <mach/mach_types.h>
#include "util/mach/child_port_types.h"

#ifdef __BeforeMigServerHeader
__BeforeMigServerHeader
#endif /* __BeforeMigServerHeader */

#ifndef MIG_SERVER_ROUTINE
#define MIG_SERVER_ROUTINE
#endif


/* SimpleRoutine child_port_check_in */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
MIG_SERVER_ROUTINE
kern_return_t handle_child_port_check_in
(
	child_port_server_t server,
	child_port_token_t token,
	mach_port_t port,
	mach_msg_type_name_t portPoly
);

#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
boolean_t child_port_server(
		mach_msg_header_t *InHeadP,
		mach_msg_header_t *OutHeadP);

#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
mig_routine_t child_port_server_routine(
		mach_msg_header_t *InHeadP);


/* Description of this subsystem, for use in direct RPC */
extern const struct handle_child_port_subsystem {
	mig_server_routine_t	server;	/* Server routine */
	mach_msg_id_t	start;	/* Min routine number */
	mach_msg_id_t	end;	/* Max routine number + 1 */
	unsigned int	maxsize;	/* Max msg size */
	vm_address_t	reserved;	/* Reserved */
	struct routine_descriptor	/*Array of routine descriptors */
		routine[1];
} handle_child_port_subsystem;

/* typedefs for all requests */

#ifndef __Request__child_port_subsystem__defined
#define __Request__child_port_subsystem__defined

#ifdef  __MigPackStructs
#pragma pack(push, 4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		mach_msg_port_descriptor_t port;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		child_port_token_t token;
	} __Request__child_port_check_in_t __attribute__((unused));
#ifdef  __MigPackStructs
#pragma pack(pop)
#endif
#endif /* !__Request__child_port_subsystem__defined */


/* union of all requests */

#ifndef __RequestUnion__handle_child_port_subsystem__defined
#define __RequestUnion__handle_child_port_subsystem__defined
union __RequestUnion__handle_child_port_subsystem {
	__Request__child_port_check_in_t Request_child_port_check_in;
};
#endif /* __RequestUnion__handle_child_port_subsystem__defined */
/* typedefs for all replies */

#ifndef __Reply__child_port_subsystem__defined
#define __Reply__child_port_subsystem__defined

#ifdef  __MigPackStructs
#pragma pack(push, 4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		NDR_record_t NDR;
		kern_return_t RetCode;
	} __Reply__child_port_check_in_t __attribute__((unused));
#ifdef  __MigPackStructs
#pragma pack(pop)
#endif
#endif /* !__Reply__child_port_subsystem__defined */


/* union of all replies */

#ifndef __ReplyUnion__handle_child_port_subsystem__defined
#define __ReplyUnion__handle_child_port_subsystem__defined
union __ReplyUnion__handle_child_port_subsystem {
	__Reply__child_port_check_in_t Reply_child_port_check_in;
};
#endif /* __ReplyUnion__handle_child_port_subsystem__defined */

#ifndef subsystem_to_name_map_child_port
#define subsystem_to_name_map_child_port \
    { "child_port_check_in", 10011 }
#endif

#ifdef __AfterMigServerHeader
__AfterMigServerHeader
#endif /* __AfterMigServerHeader */

#endif	 /* _child_port_server_ */

#ifdef mig_external
mig_external
#else
extern
#endif
kern_return_t __MIG_check__Request__child_port_check_in_t(__Request__child_port_check_in_t *In0P);

#ifdef __cplusplus
}
#endif
