/**
 * ipc_forwards:
 *
 * Forward declarations of all ipc subsystem structs.
 * Internal to the ipc subsystem so do not depend on this file!
 */

#ifndef __IPC_FORWARDS_H
#define __IPC_FORWARDS_H

struct ipc_fullmsg;
struct ipc_io;
struct ipc_addr;
union ipc_msg;
enum ipc_msg_id;

typedef struct ipc_fullmsg ipc_fullmsg_t;
typedef struct ipc_io ipc_io_t;
typedef struct ipc_addr ipc_addr_t;
typedef union ipc_msg ipc_msg_t;
typedef enum ipc_msg_id ipc_msg_id_t;


#endif /* __IPC_FORWARDS_H */
